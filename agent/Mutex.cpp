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
	@file Mutex.cpp
	
	@brief Implementation of Mutex class
 */

#include "agent.h"

/*!
	@brief Initializes a mutex.
 */
Mutex::Mutex(void)
{
#if WINDOWS
	InitializeCriticalSection(&m_mutex);
#elif UNIX
	pthread_mutex_init(&m_mutex,NULL);
#endif
}

/*!
	@brief Destroys a mutex.
 */
Mutex::~Mutex(void)
{
#if WINDOWS
	DeleteCriticalSection(&m_mutex);
#endif
}

/*!
	@brief Locks a mutex.
 */
void Mutex::Lock()
{
#if WINDOWS
	EnterCriticalSection(&m_mutex);
#elif UNIX
	pthread_mutex_lock(&m_mutex);
#endif
}

/*!
	@brief Unlocks a mutex.
 */
void Mutex::Unlock()
{
#if WINDOWS
	LeaveCriticalSection(&m_mutex);
#elif UNIX
	pthread_mutex_unlock(&m_mutex);
#endif
}
