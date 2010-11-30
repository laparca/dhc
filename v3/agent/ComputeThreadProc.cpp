/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* Copyright (C) 2010 Samuel Rodriguez Sevilla
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
#include "Algorithm.h"
#include "AlgorithmFactory.h"
#include <time.h>
#include <stdarg.h>
using namespace std;

extern double g_tBaseN;
extern bool g_bTesting;

//Delay for the specificed number of milliseconds.
//Defined only for non-Windows platforms as it is included in the Windows API.
#ifndef WINDOWS
#define Sleep(ms) usleep(1000 * ms);
#endif

/*!
 *	@class SelectAlgorithms
 *	@brief Selects the algorithm using the GPU capability or the
 *	GPU capability.
 */
class SelectAlgorithms
{
private:
	bool with_GPU;
	bool with_CPU;

public:
	SelectAlgorithms(bool wg, bool wc) : with_GPU(wg), with_CPU(wc)
	{}
	
	bool operator()(Algorithm *alg)
	{
		if(alg->IsGPUCapable() && with_GPU)
			return true;
		else if(alg->IsCPUCapable() && with_CPU)
			return true;
		return false;
	}
};
/*!
 *	@def SELECT_WITH_GPU
 *	@brief This define is used as parameter for the SelectAlgorithms
 *	class and indicates that it has select algorithms that use GPU.
 */
#define SELECT_WITH_GPU true, false
/*!
 *	@def SELECT_WITH_CPU
 *	@brief This define is used as parameter for the SelectAlgorithms
 *	class and indicates that it has select algorithms that use CPU.
 */
#define SELECT_WITH_CPU false, true
/*!
 *	@def SELECT_WITH_BOTH
 *	@brief This define is used as parameter for the SelectAlgorithms
 *	class and indicates that it has select algorithms that use GPU or CPU.
 */
#define SELECT_WITH_BOTH true, true

/*!
 *	@brief Transform an algorithm list into a algorithm name list.
 *	@param An algorithm list.
 *	@return Al algorithm name list.
 */
vector<string> GetAlgorithmNames(const vector<Algorithm *>& vAlgorithm)
{
	vector<string> names;
	vector<Algorithm *>::const_iterator end = vAlgorithm.end();
	for(vector<Algorithm *>::const_iterator it = vAlgorithm.begin(); it != end; it++)
		names.push_back((*it)->GetName());
	return names;
}

/*!
 *	@brief Wrapper for cuda kernel functions.
 *
 *	CudaFunction generates a wrapper for the cuda kernel functions and
 *	this is very useful for simplify calling those methods.
 */
class CudaFunction
{
private:
	Stream *m_stream;
	CudaKernel *m_hashker;
	int m_block_x;
	int m_block_y;
	int m_threads_x;
	int m_threads_y;
	int m_threads_z;
public:
	CudaFunction(Stream *stream, CudaKernel *hashker, int blocks_x, int blocks_y, int threads_x, int threads_y, int threads_z) :
		m_stream(stream),
		m_hashker(hashker),
		m_block_x(blocks_x),
		m_block_y(blocks_y),
		m_threads_x(threads_x),
		m_threads_y(threads_y),
		m_threads_z(threads_z)
	{}

	/*!
	 *	@brief Runs the kernel function.
	 *	This operator runs the kernel function with the specificed parameters and
	 *	emultates the aspect of a normal function. The parameters are an array
	 *	of KernelParamBase. That array cannot be dynamic (allocated with malloc
	 *	or new) in other case the template will not determine the array size.
	 *	@param params An array with the params for the kernel function.
	 */
	template<int Size>
	void operator()(KernelParamBase *(&params)[Size])
	{
		m_stream->AddKernelCall(*m_hashker, m_block_x, m_block_y, m_threads_x, m_threads_y, m_threads_z, params);
	}
};
/*!
	@brief Processes events for a single compute device.
	
	@param pData A ComputeDevice* pointer, casted to void*, identifying the device to use
 */
void ComputeThreadProc(void* pData)
{
	long sleep_time = 5000;
	long waiting_time = -1;
	
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
			vector<Algorithm *> vAlg = AlgorithmFactory::GetAlgorithms(SelectAlgorithms(SELECT_WITH_GPU));
			algs = GetAlgorithmNames(vAlg);
		}
#if defined(LINUX) && defined(AMD64)
		else if(LINUX==1 && AMD64==1)			//64 bit Linux
		{
			vector<Algorithm *> vAlg = AlgorithmFactory::GetAlgorithms(SelectAlgorithms(SELECT_WITH_CPU));
			algs = GetAlgorithmNames(vAlg);
		}
#endif
		//Try to get a work unit
		/* TODO
		 * Tomar la diferencia de tiempo entre que se envia y se recive la
		 * respuesta (tiempo de respuesta del servidor). Dependiendo de cuanto
		 * sea este tiempo se debera esperar mas o menos para realizar la
		 * siguiente peticion. El objetivo de esto es evitar la saturacion
		 * del controlador en caso de que haya demasiados agentes ociosos.
		 *
		 * Si TRS actual == TRS anterior => mismo tiempo de espera
		 * Si TRS actual > TRS anterior => subimos el tiempo de espera (5 seg.)
		 * Si TRS actual < TRS anterior => reducimos el tiempo de espera (1 seg.)
		 */
		WorkUnit wu;
		long t0 = time(NULL);
		bool has_wu = link.GetWorkUnit(wu, algs);
		long t1 = time(NULL);
		if(waiting_time != -1)
		{
			long wt_now = t1 - t0;
			if(waiting_time > wt_now && sleep_time > 5000)
				sleep_time -= 1000;
			else if(waiting_time < wt_now)
				sleep_time += 5000;
		}
		
		waiting_time = t1 - t0;
		cout << "wait time = " << waiting_time << endl;
		
		if(!has_wu)
		{
			//Not cracking, wait 5 seconds and try again
			cout << "No WU, waiting " << sleep_time << " milliseconds\n";
			Sleep(sleep_time);
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
	//** {{ CODIGO PARA LA PRECARGA DEL ALGORITMO
	Algorithm *alg = AlgorithmFactory::GetAlgorithm(wu.m_algorithm);
	//alg->Prepare(pDevice, pContext, wu);
	alg->ExecuteGPU(wu, pDevice, pContext);
	//** }}
}
#endif
