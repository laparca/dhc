/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* Copyright (C) 2010 Samuel Rodriguez Sevilla                                 *
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

#ifndef CUDA_FUNCTION_H
#define CUDA_FUNCTION_H

#include "LowLevel.h"

#ifdef CUDA_ENABLED
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
#endif

#endif