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
#include "sha256.h"
#include "ExecutorFactory.h"
#include "debug.h"

string sha256::GetName()
{
	return string("sha256");
}

int sha256::HashLength()
{
	return 32;
}

int  sha256::InputLength()
{
	return 32;
}

void sha256::ExecuteCPU(WorkUnit& wu, int nCore)
{
	
}

#ifdef CUDA_ENABLED
void sha256::ExecuteGPU(WorkUnit& wu, Device* pDevice, CudaContext* pContext)
{
	DO_ENTER("sha256", "ExecuteGPU");
		
	Module *hashmod;
	CudaKernel *hashker;
	unsigned int nTargetHashes = wu.m_hashvalues.size();
		
	/* Loads the ptx code into memory and creates the module */
	hashmod = new Module("sha256", ReadPtx("sha256"), *pContext);
		
	/* Identify the function to use */
	string func = "sha256";
		
	if(wu.m_hashvalues.size() > 1)
		func = func + "BatchKernel";
	else
		func = func + "Kernel";
		
	/* Load the function */
	hashker = hashmod->GetKernel(func.c_str());

	executor_parameters parameters;
	parameters["hashmod"] = &hashmod;
	parameters["hashker"] = &hashker;
		
	Executor *exec = ExecutorFactory::Get("BasicExecutor");
	exec->Execute(this, wu, pDevice, pContext, parameters);
		
	delete hashker;
	delete hashmod;		
}
#endif

bool sha256::IsGPUCapable()
{
#ifdef CUDA_ENABLED
	return true;
#else
	return false;
#endif
}

bool sha256::IsCPUCapable()
{
	return false;
}

BEGIN_PLUGIN("Samuel Rodriguez Sevilla")
	ADD_ALGORITHM(sha256, "sha256", 1)
END_PLUGIN()