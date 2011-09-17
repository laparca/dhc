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
	@file DeviceMemory.h
	
	@brief Declaration of DeviceMemoryBuffer
 */

#ifndef DEVICE_MEMORY_H
#define DEVICE_MEMORY_H

#include <list>
#include <map>

#ifdef CUDA_ENABLED

class DeviceMemoryBuffer;

/*!
	@brief Thread-level context structure used for paging
	
	Do not use this class directly - use CudaContext instead.
 */
class DeviceThreadContext
{
public:
	virtual ~DeviceThreadContext()
	{}
};

/*!
	@brief Smart pointer to a block of GPU memory.
	
	A paging scheme, using the LRU algorithm, is supported to aid functioning in low-memory conditions: upon failure of a memory
	allocation, older blocks will be paged out to CPU memory. The paging scheme is not yet fully integrated with streaming,
	however; using a DeviceMemoryBuffer object in a Stream automatically page-locks the buffer and does not unlock when the
	stream is flushed.
	
	Objects of this class are not thread safe due to the underlying CUDA API - each object may only be used in the host thread
	from which it was created.
 */
class DeviceMemoryBuffer
{
public:
	DeviceMemoryBuffer(size_t size, DeviceThreadContext& context);
	~DeviceMemoryBuffer();

	void* GetData();

	/*!
		@brief Gets the size of this buffer
		@return Buffer size, in bytes
	 */
	size_t GetSize()
	{ return m_size; }

	friend class DeviceThreadContext;
	
protected:
	/*!
		@brief The device context associated with this buffer
	 */
	DeviceThreadContext* m_context;
	
	/*!
		@brief Size of this buffer, in bytes
	 */
	size_t m_size;
	
	/*!
		@brief Pointer to the current memory block on the GPU
		
		Only valid if we are an active buffer (not paged out).
	 */
	void* m_data;
	
	void Allocate();
	
private:
	//copying disallowed
	DeviceMemoryBuffer(const DeviceMemoryBuffer& unused)
	{ ThrowError("call of unsupported function"); }
	
	DeviceMemoryBuffer& operator=(const DeviceMemoryBuffer& unused)
	{ ThrowError("call of unsupported function"); return *this; }
};

#endif //#ifdef CUDA_ENABLED

#endif

