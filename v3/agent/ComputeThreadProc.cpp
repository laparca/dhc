/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* All rights reserved.                                                        *
*                                                                             *
* Redistribution and use in source and binary forms, with or without modifi-  *
* cation, are permitted provided that the following conditions are met:       *
*                                                                             *
*    * Redistributions of source code must retain the above copyright notice  *
*      this list of conditions and the following disclaimer.                  *
*                                                                             *
*    * Redistributions in binary form must reproduce the above copyright      *
*      notice, this list of conditions and the following disclaimer in the    *
*      documentation and/or other materials provided with the distribution.   *
*                                                                             *
*    * Neither the name of RPISEC nor the names of its contributors may be    *
*      used to endorse or promote products derived from this software without *
*      specific prior written permission.                                     *
*                                                                             *
* THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY EXPRESS OR IMPLIED      *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN     *
* NO EVENT SHALL RPISEC BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      *
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        *
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                *
*                                                                             *
*******************************************************************************/

/*!
	@file ComputeThreadProc.cpp
	
	@brief Core cracking logic
 */

#include "agent.h"

using namespace std;

extern double g_tBaseN;
extern bool g_bTesting;

//Delay for the specificed number of milliseconds.
//Defined only for non-Windows platforms as it is included in the Windows API.
#ifndef WINDOWS
#define Sleep(ms) usleep(1000 * ms);
#endif

/*!
	@brief Processes events for a single compute device.
	
	@param pData A ComputeDevice* pointer, casted to void*, identifying the device to use
 */
void ComputeThreadProc(void* pData)
{
	//Set up the device
	ComputeDevice* pDev = reinterpret_cast<ComputeDevice*>(pData);
	
	#ifdef CUDA_ENABLED
		//Initialize and print some info
		Device* pDevice = NULL;
		CudaContext* pContext = NULL;
		if(pDev->bGPU)
		{
			//Create CUDA stuff
			pDevice = new Device(pDev->index);
			pContext = new CudaContext(*pDevice, CudaContext::Yield);
		}
	#endif
	
	//Set up our controller link
	//(starts a persistent connection if required by the protocol, otherwise does nothing)
	ControllerLink link(pDev);
	
	//Loop forever
	while(true)
	{
		//TODO: get algorithm list from somewhere else rather than hard coding it
		vector<string> algs;
		if(pDev->bGPU)							//CUDA, regardless of CPU architecture
		{
			algs.push_back("md4");
			algs.push_back("md5");
			algs.push_back("md5crypt");
			algs.push_back("ntlm");
			algs.push_back("sha1");
		}
		else if(LINUX==1 && AMD64==1)			//64 bit Linux
			algs.push_back("md5");
		
		//Try to get a work unit
		WorkUnit wu;
		if(!link.GetWorkUnit(wu, algs))
		{
			//Not cracking, wait 5 seconds and try again
			cout << "No WU, waiting\n";
			Sleep(5000);
			continue;
		}
		
		//Dispatch it
		#ifdef CUDA_ENABLED
		if(pDev->bGPU)
			DoWorkUnitOnGPU(wu, pDevice, pContext);
		else
		#endif
			DoWorkUnitOnCPU(wu, pDev->index);
			
		//Submit results
		link.SubmitResults(wu);
		
		//Free hash memory
		for(unsigned int i=0; i<wu.m_hashvalues.size(); i++)
			delete[] wu.m_hashvalues[i];
		for(unsigned int i=0; i<wu.m_salts.size(); i++)
			delete[] wu.m_salts[i];
	}
	
	//TODO: free resources when we are ctrl-Ced
}

/*!
	@brief Processes a work unit on a CPU core
	
	@param wu The work unit to process
	@param nCore Core ID (currently just a thread ID, no affinity masks are used)
 */
void DoWorkUnitOnCPU(WorkUnit& wu, int nCore)
{
	//Look up the start and end values
	SSE_ALIGN(int rcharset[256]);
	GenerateReverseCharset(rcharset, wu.m_charset);
	SSE_ALIGN(int start[MAX_BASEN_LENGTH]);
	SSE_ALIGN(int end[MAX_BASEN_LENGTH]);
	BaseNParse(start, wu.m_start, rcharset);
	BaseNParse(end, wu.m_end, rcharset);
	unsigned int len = wu.m_start.length();
	unsigned int base = wu.m_charset.size();
	const char* charset = wu.m_charset.c_str();
	
	//TODO: Sanity check values
	
	//Look up hash function
	//TODO: Use some kind of plugin system
	HASHPROC pHash = NULL;
	if(wu.m_algorithm == "md5")
	{
		//Temporary until we get 32 bit hashes
		#if (AMD64 == 1)
			pHash = MD5Hash;
		#endif
	}
	else	
	{
		ThrowCustomError("Unknown hash function");
	}
	
	//Record performance info
	double tstart = GetTime();
	unsigned long nHashes = 0;
	
	int nHashesToCrack = wu.m_hashvalues.size();
	int hashlen = wu.m_hashlen;
		
	//Loop through charset
	char guesses[4 * MAX_BASEN_LENGTH + 1] = {0};	//Everything not in a valid guess is already nulled out
	char outputs[512 * 4 + 1] = {0};
	while(BaseNLess(start, end, len) && (nHashesToCrack > 0))
	{
		//Build four guesses
		for(int j=0; j<4; j++)
		{
			for(unsigned int i=0; i<len; i++)
				guesses[j*MAX_BASEN_LENGTH + i] = charset[start[i]];
				
			BaseNAdd1(start, base, len);
		}
	
		//Hash them
		nHashes += 4;
		pHash(guesses, outputs, len);		
		
		//Test results
		for(int i=0; i<4; i++)
		{
			//Check each hash against every target
			for(unsigned int j=0; j<wu.m_hashvalues.size(); j++)
			{
				if(memcmp(outputs + hashlen*i, wu.m_hashvalues[j], hashlen) == 0)
				{
					char str[MAX_BASEN_LENGTH] = {0};
					memcpy(str, guesses + (i*MAX_BASEN_LENGTH), len);
					wu.m_collisions[j] = str;
					nHashesToCrack--;
				}
			}
		}
	}
	
	//Print stats
	double dt = GetTime() - tstart;
	float speed = static_cast<double>(nHashes) / (dt * 1000000);
	if(!g_bTesting)
	{
		printf("Work unit completed (by core %d): %lu hashes in %.2f sec, avg speed %.2f MHz\n",
			nCore,
			nHashes,
			dt, 
			speed
			);
	}
		
	//Format output
	char sbuf[128];
	sprintf(sbuf, "%.3f", (float)dt);
	wu.m_dt = sbuf;
	sprintf(sbuf, "%.3f", speed);
	wu.m_speed = sbuf;
}

#ifdef CUDA_ENABLED
/*!
	@brief Processes a work unit on GPU
	
	@param wu The work unit to process
	@param pDevice The device to work on
	@param pContext Context being used
 */
void DoWorkUnitOnGPU(WorkUnit& wu, Device* pDevice, CudaContext* pContext)
{	
	//Look up the start and end values
	int rcharset[256];
	GenerateReverseCharset(rcharset, wu.m_charset);
	int start[MAX_BASEN_LENGTH];
	int end[MAX_BASEN_LENGTH];
	BaseNParse(start, wu.m_start, rcharset);
	BaseNParse(end, wu.m_end, rcharset);
	unsigned int len = wu.m_start.length();
	unsigned int base = wu.m_charset.size();
	unsigned int nTargetHashes = wu.m_hashvalues.size();
	
	//Start and end lengths must match
	if(len != wu.m_end.length())
		ThrowError("Invalid work unit");	
	if(len > 30)
		ThrowError("Input too big");
	if(nTargetHashes > 128)
		ThrowError("Max 128 hashes per block\n");
	
	//TODO: Sanity check charset (duplicate check, etc)?
	
	//Load hash module and kernel from PTX
	string fname = string("ptx/") + wu.m_algorithm + ".ptx";
	Module hashmod(fname, *pContext);
	if(wu.m_algorithm == "md4" && len <= 12)						//If we are cracking a weak algorithm, switch to
	{																// the cryptanalytic attack when possible
		wu.m_algorithm = "md4_fast";
	}
	string func;
	if(nTargetHashes > 1)
		func = wu.m_algorithm + "BatchKernel";
	else
		func = wu.m_algorithm + "Kernel";
	CudaKernel* hashker = hashmod.GetKernel(func.c_str());
	
	//Perform cryptanalytic attacks on weak algorithms
	if(wu.m_algorithm == "md4_fast")
	{
		for(unsigned int i=0; i<nTargetHashes; i++)
			MD4MeetInTheMiddlePreprocessing(wu.m_hashvalues[i]);
	}
	
	/*
	//DEBUG: Set length
	len = 3;
	start[0] = 0;
	start[1] = 0;
	start[2] = 0;
	*/
	
	//A few settings
	//TODO: Calculate on a per-algorithm basis and store in XML
	int threadcount = 256;
	int xblockcount = 16384;
	if(pDevice->GetMajorVersion() > 1 || pDevice->GetMinorVersion() >= 3)
	{
		//Compute 1.3 or better - use lots of blocks
		xblockcount = 65535;
	}
	if(wu.m_algorithm == "md5crypt")	//md5crypt is slow!
	{
		xblockcount /= 1024;
		
		//Kernel currently requires >64 threads
		if(pDevice->GetMajorVersion() > 1 || pDevice->GetMinorVersion() >= 3)
			threadcount = 128;
		else
			threadcount = 64;
	}
	unsigned long hashcount = xblockcount * threadcount;
	
	//Record performance info
	double tstart = GetTime();
	g_tBaseN = 0;
	unsigned long nHashes = 0;
	
	Stream mystream[2];

	//Copy charset to device
	HostMemoryBuffer hcharset(256);
	memcpy(hcharset.GetData(), wu.m_charset.c_str(), base);
	DeviceMemoryBuffer dcharset(256, *pContext);
	mystream[0].AddHtoDMemcpy(dcharset, hcharset);
	
	//Allocate host memory buffers for target hash and salt.
	//They are appended end-to-end.
	unsigned int hashlen = wu.m_hashlen;
	unsigned int saltlen = 1;
	if(wu.m_salts.size() > 0)
		saltlen = strlen(reinterpret_cast<const char*>(wu.m_salts[0]));
	if(saltlen == 0)		//No salt? Use one byte as padding
		saltlen = 1;
	HostMemoryBuffer hhash(hashlen * nTargetHashes);
	HostMemoryBuffer hsalt(saltlen * nTargetHashes);
	unsigned char* hash = reinterpret_cast<unsigned char*>(hhash.GetData());
	unsigned char* salt = reinterpret_cast<unsigned char*>(hsalt.GetData());
	for(unsigned int i=0; i<wu.m_hashvalues.size(); i++)
	{
		unsigned int base = hashlen*i;
		for(unsigned int j=0; j<hashlen; j++)
			hash[base + j] = wu.m_hashvalues[i][j];
	}
	for(unsigned int i=0; i<wu.m_salts.size(); i++)
	{
		unsigned int base = saltlen*i;
		for(unsigned int j=0; j<saltlen; j++)
			salt[base + j] = wu.m_salts[i][j];	
	}

	//Copy target hash to device
	DeviceMemoryBuffer dhash(hhash.GetSize(), *pContext);
	mystream[0].AddHtoDMemcpy(dhash, hhash);
	
	//Copy salt to device
	DeviceMemoryBuffer dsalt(hsalt.GetSize(), *pContext);
	mystream[0].AddHtoDMemcpy(dsalt, hsalt);	
	
	//Allocate memory for start guess and output
	HostMemoryBuffer hstart(len * sizeof(int));
	DeviceMemoryBuffer dstart1(hstart.GetSize(), *pContext);
	DeviceMemoryBuffer dstart2(hstart.GetSize(), *pContext);
	HostMemoryBuffer houtput(32 * nTargetHashes);
	unsigned char* hout = reinterpret_cast<unsigned char*>(houtput.GetData());
	memset(hout, '-', houtput.GetSize());
	DeviceMemoryBuffer doutput1(houtput.GetSize(), *pContext);
	DeviceMemoryBuffer doutput2(houtput.GetSize(), *pContext);
	mystream[0].AddHtoDMemcpy(doutput1, houtput);
	mystream[1].AddHtoDMemcpy(doutput2, houtput);
	
	//Initial 'cracked' flags are 0
	HostMemoryBuffer hstatus(nTargetHashes);
	char* phstatus = reinterpret_cast<char*>(hstatus.GetData());
	for(unsigned int i=0; i<nTargetHashes; i++)
		phstatus[i] = 0;
	DeviceMemoryBuffer dstatus1(nTargetHashes, *pContext);
	DeviceMemoryBuffer dstatus2(nTargetHashes, *pContext);
	mystream[0].AddHtoDMemcpy(dstatus1, hstatus);
	mystream[1].AddHtoDMemcpy(dstatus2, hstatus);
	
	//Bind charset texture
	CUtexref texCharset;
	CUresult result;
	if(CUDA_SUCCESS != (result = cuModuleGetTexRef(&texCharset, hashmod.GetModule(), "texCharset")))
		ThrowCudaLLError("Failed to get reference to texCharset", result);
	if(CUDA_SUCCESS != (result = cuTexRefSetAddress(
		NULL,
		texCharset,
		static_cast<CUdeviceptr>(reinterpret_cast<intptr_t>(dcharset.GetData())),
		dcharset.GetSize())))
		ThrowCudaLLError("Failed to set address of texCharset", result);
	if(CUDA_SUCCESS != (result =  cuTexRefSetFlags(texCharset, CU_TRSF_READ_AS_INTEGER)))
		ThrowCudaLLError("Failed to set texture flags", result);

	//Loop through charset
	int nStream = 0;
	#ifdef CUDA_PROFILING_ENABLED
	mystream[0].StartProfiling();
	mystream[1].StartProfiling();
	#endif
	while(BaseNLess(start, end, len))
	{
		//Copy start value
		memcpy(hstart.GetData(), start, len*sizeof(int));
		mystream[nStream].AddHtoDMemcpy(nStream == 0 ? dstart1 : dstart2, hstart);
		
		//DEBUG: Print results
		//char str[32] = {0};
		//for(int i=0; i<len; i++)
		//	str[i] = wu.m_charset[start[i]];
		//printf("Start: %s\n", str);
		
		//Run the kernel
		KernelParam<DeviceMemoryBuffer> ptarget(dhash);
		KernelParam<DeviceMemoryBuffer> pstart(nStream == 0 ? dstart1 : dstart2);
		KernelParam<DeviceMemoryBuffer> psalt(dsalt);
		KernelParam<DeviceMemoryBuffer> pstatus(nStream == 0 ? dstatus1 : dstatus2);
		KernelParam<DeviceMemoryBuffer> poutput(nStream == 0 ? doutput1 : doutput2);
		KernelParam<int> pbase(base);
		KernelParam<int> plen(len);
		KernelParam<int> psaltlen(saltlen);
		KernelParam<int> targets(nTargetHashes);
		KernelParamBase* params[] = 
		{
			&ptarget,
			&pstart,
			&psalt,
			&pstatus,
			&poutput,
			&pbase,
			&plen,
			&psaltlen,
			&targets,
			NULL
		};
		cuParamSetTexRef(hashker->GetFunction(), CU_PARAM_TR_DEFAULT, texCharset);
		mystream[nStream].AddKernelCall(
			*hashker,
			xblockcount, 1,
			threadcount, 1, 1,
			params);
			
		//Don't copy results back
		//We want raw throughput, no need to test until end of WU
		
		//Bump our start value
		BaseNAdd(start, base, len, hashcount);
		nHashes += hashcount;
	
		//Swap streams
		nStream = 1 - nStream;
	}

	for(nStream = 0; nStream <= 1; nStream++)
	{
		//Copy results back
		mystream[nStream].AddDtoHMemcpy(hstatus, nStream == 0 ? dstatus1 : dstatus2);
		mystream[nStream].AddDtoHMemcpy(houtput, nStream == 0 ? doutput1 : doutput2);
		mystream[nStream].Barrier();
		
		//Check results
		for(unsigned int i=0; i<nTargetHashes; i++)
		{
			if(phstatus[i])
			{
				if(!phstatus[i])
					continue;
					
				//printf("Cracked\n");
					
				//Extract the appropriate substring
				char* strRaw = reinterpret_cast<char*>(houtput.GetData());
				char str[32] = {0};
				int base = 32*i;
				for(unsigned int j=0; j<len; j++)
					str[j] = strRaw[j+base];
				
				/*	
				printf("Cracked %d\n", i);
				for(int q=0; q<32*nTargetHashes; q++)
					printf("%c", strRaw[q]);*/
			
				//if(wu.m_algorithm == "md5crypt")
				//{
				//	//DEBUG: Print results as hex ints
				//	printf("\n");
				//	int* istr = reinterpret_cast<int*>(houtput.GetData());
				//	for(int i=0; i<4; i++)
				//		printf("%08x ", istr[i]);
				//	printf("\n");
				//	
				//	//DEBUG: Print results as hex
				//	for(unsigned int i=0; i<hashlen; i++)
				//		printf("%02x", static_cast<int>(str[i] & 0xFF));		
				//		
				//	printf("\n");
				//	exit(-1);
				//}

				//DEBUG: Print and quit
				//printf("%s", str);
				//printf("\n");
				//exit(0);
					
				wu.m_collisions[i] = str;
				wu.m_bCracked[i] = true;
			}
		}
	}
	
	#ifdef CUDA_PROFILING_ENABLED
	mystream[0].StopProfiling();
	mystream[1].StopProfiling();
	#endif
	
	//Clean up
	delete hashker;
	
	#ifdef PROFILING_ENABLED
	printf("Base-N time: %.3f sec\n", g_tBaseN);
	#endif
	
	//Print stats
	double dt = GetTime() - tstart;
	float speed = static_cast<double>(nHashes) / (dt * 1000000);
	if(!g_bTesting)
	{
		printf("Work unit completed (by GPU %d): %lu hashes in %.2f sec, avg speed %.2f MHz\n",
			pDevice->GetDeviceID(),
			nHashes,
			dt, 
			speed
			);
	}
		
	//Format output
	char sbuf[128];
	sprintf(sbuf, "%.3f", (float)dt);
	wu.m_dt = sbuf;
	sprintf(sbuf, "%.3f", speed);
	wu.m_speed = sbuf;
}
#endif
