#include "BasicExecutor.h"

extern double g_tBaseN;

string BasicExecutor::GetName()
{
	return string("BasicExecutor");
}

void BasicExecutor::Execute(Algorithm *alg, WorkUnit& wu, Device* pDevice, CudaContext* pContext)
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

/*	{{ OLD
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
}} */

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
	else if(wu.m_algorithm == "sha256") // SHA256 requires a lot of registers
	{
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
	
	/** TODO
	* Tengo que leer el hashmod desde el algoritmo.
	*/
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

	CudaFunction hashStream[2] = {
		CudaFunction(&(mystream[0]), hashker, xblockcount, 1, threadcount, 1, 1),
		CudaFunction(&(mystream[1]), hashker, xblockcount, 1, threadcount, 1, 1)
	};

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
		KernelParamBase* params[9] = 
		{
			&ptarget,
			&pstart,
			&psalt,
			&pstatus,
			&poutput,
			&pbase,
			&plen,
			&psaltlen,
			&targets
		};
		cuParamSetTexRef(hashker->GetFunction(), CU_PARAM_TR_DEFAULT, texCharset);
		hashStream[nStream](params);
		/*mystream[nStream].AddKernelCall(
			*hashker,
			xblockcount, 1,
			threadcount, 1, 1,
			params);*/

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