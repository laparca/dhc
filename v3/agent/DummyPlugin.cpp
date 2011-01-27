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
******************************************************************************/
#include "DummyPlugin.h"

/*!
 *	@brief Returns the name of the algorithm
 *	@return The algorithm name
 */
string DummyAlgorithm::GetName()
{
	return string("DummyPlug");
}
/*!
 *	@brief Returns the length of a hash generated by the algorithm
 *	@return The generated hash length
 */
int  DummyAlgorithm::HashLength()
{
	return 0;
}
/*!
 * 	@brief Each algorithm requires an input (normally a hash) with
 *	a concrete length. This method returns the length of this input.
 *	@return The algorithm input length
 */
int  DummyAlgorithm::InputLength()
{
	return 0;
}
/*!
 *	@brief Executes the CPU version of the algorithm. It is useful
 *	if the system is not GPU accelerated.
 */
void DummyAlgorithm::ExecuteCPU()
{
	
}
#ifdef CUDA_ENABLED
/*!
 *	@brief Executes the GPU version of the algorithm.
 *	@param pDevice Information about the GPU device
 */
void DummyAlgorithm::ExecuteGPU(WorkUnit& wu, Device* pDevice, CudaContext* pContext)
{
	
}
#endif
/*!
 *	@brief Returns if the algorithm can use GPU capabilities
 *	@return Tue if the Algorithm can use a GPU
 */
bool DummyAlgorithm::IsGPUCapable()
{
	return true;
}
/*!
 *	@brief Returns if the algorithm cas use GPU capabilites
 *	@return True if the algorithm can use a CPU
 */
bool DummyAlgorithm::IsCPUCapable()
{
	return true;
}
/*!
 *	@brief Prepares information to accelerate hash attack or to perform better the functionality
 *	@param wo The workunit with the information to analize
 */
//virtual void DummyAlgorithm::Prepare(WorkUnit & wo) = 0;
#ifdef CUDA_ENABLED
/*!
 *  @brief Prepared information to accelerate hash attack or to improve the performance. It is
 *         indicated for CUDA devices.
 *  @param pDevice
 *  @param pContext
 *  @param wu
 */
//	virtual void DummyAlgorithm::Prepare(Device *pDevice, CudaContext *pContext, WorkUnit *wu) = 0;
#endif
/*!
 *  @brief This method frees the internal memory used during the execution process.
 */
//	virtual void DummyAlgorithm::End() = 0;
/*!
 *	@brief Number of registers used by the GPU version of the algorithm.
 *	This value can be obtained using the cubin compilation of the GPU unit. If it is
 *	unknown it returns 0.
 *	@return The number of registers used
 */
int DummyAlgorithm::RegistersUsed()
{
	return 0;
}
/*!
 *	@brief Amount of constant memory used by the GPU version of the algorithm.
 *	This value can be obtained using the cubin compilation of the GPU unit. If it is
 *	unknown it return 0.
 *	@return The amount of constant memory used
 */
int DummyAlgorithm::ConstantMemoryUsed()
{
	return 0;
}
/*!
 *	@brief Amount of shared memory used by the GPU version of the algorithm.
 *	This value can be obtained using the cubin compilation of the GPU unit. If it is
 *	unknown it return 0.
 *	@return The amount of shared memory used
 */
int DummyAlgorithm::SharedMemoryUsed()
{
	return 0;
}


BEGIN_PLUGIN("Samuel Rodriguez Sevilla")
	ADD_ALGORITHM(DummyAlgorithm, "Dummy", 1)
END_PLUGIN()