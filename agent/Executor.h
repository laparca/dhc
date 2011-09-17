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
#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "Algorithm.h"
#include <map>
#include <string>
using namespace std;

/*!
 *	@def GET_EXECUTOR_PARAM
 *	@brief This define is used as parameter for the SelectAlgorithms
 *	class and indicates that it has select algorithms that use GPU.
 */
#define GET_EXECUTOR_PARAM(type, name)                           \
type name;                                                       \
do                                                               \
{                                                                \
   executor_parameters_iterator value = parameters.find(#name);  \
   if(value != parameters.end())                                 \
   {                                                             \
      name = *(static_cast<type*>(value->second));               \
   }                                                             \
   else                                                          \
      throw "Parameter is not defined";                          \
}while(0)


typedef map<string, void*> executor_parameters;
typedef map<string, void*>::iterator executor_parameters_iterator;

/*!
 * @class Executor
 * @brief Executor prepares the GPU for run Algorithms using a known method.
 *
 * When an Algorithm must be executed in a GPU this algorithm needs its values
 * loaded in the GPU memory. An executor represents a mechanism to initialize
 * the GPU for an algorithm.
 */
class Executor
{
public:
#ifdef CUDA_ENABLED
	/*!
	 * @brief Executes an Algorithm into the GPU
	 */
	virtual void Execute(Algorithm *alg, WorkUnit& wu, Device* pDevice, CudaContext* pContext, executor_parameters& parameters) = 0;
	void operator()(Algorithm *alg, WorkUnit& wu, Device* pDevice, CudaContext* pContext, executor_parameters& parameters)
	{
		Execute(alg, wu, pDevice, pContext, parameters);
	}
#endif
	/*!
	 * @brief Returns the Executor name
	 * @return The name of the Executor
	 */
	virtual string GetName() = 0;
};


#endif
