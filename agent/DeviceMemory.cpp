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
	@file DeviceMemory.cpp
	
	@brief Implementation of DeviceMemoryBuffer
 */

#include "agent.h"
#include "DeviceMemory.h"
#include "Mutex.h"

using namespace std;

#ifdef CUDA_ENABLED

/*!
	@brief Allocates a new device memory buffer.
	
	If insufficient memory is present, an error is thrown.
	
	@param size Size of the buffer, in bytes
	@param context The thread context we are to be associated with (must be the current thread's context)
 */
DeviceMemoryBuffer::DeviceMemoryBuffer(size_t size, DeviceThreadContext& context)
: m_context(&context)
, m_size(size)
, m_data(NULL)
{
	if(m_size == 0)
		ThrowCustomError("Invalid size 0 in DeviceMemoryBuffer::DeviceMemoryBuffer()");
	
	//Allocate the block	
	Allocate();
}

/*!
	@brief Frees this buffer.
 */
DeviceMemoryBuffer::~DeviceMemoryBuffer()
{
	//GPU cleanup
	if(m_data != NULL)
	{					
		//Free GPU memory
		cuMemFree(*reinterpret_cast<CUdeviceptr*>(&m_data));	
		m_data = NULL;
	}
		
	//Make us obviously invalid to catch use-after-free situations
	m_size = 1024;
	m_context = NULL;
}

/*!
	@brief Gets a pointer to GPU memory.
			
	@return Pointer to GPU memory, only valid in the current thread. Note that this pointer is NOT mapped to the virtual address
	space of the host; attempts to use it as a conventional pointer will fail.
 */
void* DeviceMemoryBuffer::GetData()
{
	return m_data;
}

/*!
	@brief Allocates GPU memory
	
	If insufficient memory is present, an error is thrown.
 */
void DeviceMemoryBuffer::Allocate()
{
	//Try to allocate the buffer, failing if we get an unknown error
	CUresult err = CUDA_SUCCESS;
	if(CUDA_SUCCESS == (err = cuMemAlloc(reinterpret_cast<CUdeviceptr*>(&m_data), m_size)) )
		return;
	ThrowCudaLLError("Device memory allocation failed",err);	
}

#endif //#ifdef CUDA_ENABLED
