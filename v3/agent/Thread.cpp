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
******************************************************************************/

/*!
	@file Thread.h
	
	@brief Declaration of Thread class
 */

#include "agent.h"
#include "Thread.h"
#include <string>
using namespace std;

/*!
	@brief Spawns a thread
	
	@param proc Thread procedure
	@param param Parameter to pass to the thread
 */
Thread::Thread(ZTHREADPROC proc,void* param)
{
#if WINDOWS
	m_hThread = CreateThread(
		NULL,
		0,
		proc,
		param,
		0,
		NULL);
	if(m_hThread==NULL)
		throw string("Failed to create thread");
#elif UNIX
	pthread_attr_t attr; 
	pthread_attr_init(&attr); 
	pthread_create(&m_hThread,&attr,proc,param);
	//TODO: error check
#endif
}

/*!
	@brief Closes a handle to a thread.
	
	This does NOT cause the thread to terminate. If forcible termination is required, a pthread_kill() / TerminateThread()
	wrapper will be added in the future.
 */
Thread::~Thread(void)
{
#if WINDOWS
	CloseHandle(m_hThread);
#elif UNIX
	//Nothing here yet
#endif
}

/*!
	@brief Blocks the calling thread until the spawned thread has finished.
 */
void Thread::WaitUntilTermination()
{
#if WINDOWS
	WaitForSingleObject(m_hThread,INFINITE);
#elif UNIX
	pthread_join(m_hThread,NULL);
#endif
}
