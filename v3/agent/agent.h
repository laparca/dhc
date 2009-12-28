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
	@file agent.h
	
	@brief Main header file for the agent
 */

#ifndef agent_h
#define agent_h

//Configuration
#include "config.h"

//STL/libc
#include <errno.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>

//Generic UNIX stuff (not mac/linux/bsd specific)
#ifdef UNIX
#include <unistd.h>
#include <signal.h>
#endif

//CUDA headers
#ifdef CUDA_ENABLED
#include <cuda.h>
#endif

//libcurl stuff
#include <curl/curl.h>

//Error handling
#define ThrowError(err) DoThrowError(err, strerror(errno), __FILE__, __LINE__)
#define ThrowCustomError(err) DoThrowError(err, "", __FILE__, __LINE__)
#ifdef CUDA_ENABLED
#define ThrowCudaLLError(err,code) DoThrowError(err, LLGetErrorString(code), __FILE__, __LINE__)
const char* LLGetErrorString(CUresult code);
#endif
void DoThrowError(const char* err, const char* sys_err, const char* file, int line);

//CUDA helpers
#include "DeviceMemory.h"
#include "LowLevel.h"

//Multithreading
#include "Thread.h"
#include "Mutex.h"

/*!
	@brief A compute device (GPU / cpu core)
 */
struct ComputeDevice
{
	/*!
		@brief Indicates if this device is a GPU or not.
	 */
	bool bGPU;
	
	/*!
		@brief Index of this device (core number / GPU id)
	 */
	int index;
};

//Internal classes
#include "WorkUnit.h"
#include "BaseN.h"
#include "ControllerLink.h"

//Signal handlers
void OnCtrlC(int sig);

//Informational helpers
double GetTime();
int GetCpuCores();
std::string GetHostname();

//Threads
THREAD_PROTOTYPE(ComputeThread, _pData);
void ComputeThreadProc(void* pData);

//Work units
void DoWorkUnitOnCPU(WorkUnit& wu, int nCore);

#ifdef CUDA_ENABLED
void DoWorkUnitOnGPU(WorkUnit& wu, Device* pDevice, CudaContext* pContext);
#endif

//Unit testing
#define FAIL "\033[1;31mFAIL\033[0m"
#define PASS "\033[32mPASS\033[0m"
bool DoUnitTests(bool bTestGPU);

//Hash function prototype
typedef void (*HASHPROC)(char*, char*, unsigned long);

//Hash functions (TODO: move to plugins)
extern "C" void MD5Hash(char* in, char* out, unsigned long len);

//Cryptanalysis
void MD4MeetInTheMiddlePreprocessing(unsigned char* hash);

//Alignment
#ifdef UNIX
#define SSE_ALIGN(x) x __attribute__((aligned(16)))
#else
#define SSE_ALIGN(x) __declspec(align(16)) x
#endif

//Turn on to profile stuff
//#define PROFILING_ENABLED

#ifdef PROFILING_ENABLED
//Turn on for full CUDA traces (slow)
#define CUDA_PROFILING_ENABLED
#endif

#endif
