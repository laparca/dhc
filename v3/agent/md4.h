/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* Copyright (C) 2010 Samuel Rodríguez Sevilla
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
#ifndef MD4_H
#define MD4_H

#include "Algorithm.h"
#include "ExecutorFactory.h"

class md4: public Algorithm {
public:
	string GetName()
	{
		return string("md4");
	}
	int HashLength()
	{
		return 16;
	}
	int  InputLength()
	{
		return 16;
	}
	void ExecuteCPU() {}
	void ExecuteGPU(WorkUnit& wu, Device* pDevice, CudaContext* pContext)
	{
		Module *hashmod;
		CudaKernel *haskher;

		/* Loads the ptx code into memory and creates the module */
		hashmod = new Module(ReadPtx("md4"), pContext);
		
		/* Identify the function to use */
		string func = "md4";

		if(wu.m_start.length() <= 12)						//If we are cracking a weak algorithm, switch to
		{																// the cryptanalytic attack when possible
			func = "md4_fast";
		}
		
		if(wu.m_hashvalues.size() > 1)
			func = func + "BatchKernel";
		else
			func = func + "Kernel";
		
		/* Load the function */
		hashker = hashmod->GetKernel(func.c_str());

		//Perform cryptanalytic attacks on weak algorithms
		if(wu.m_algorithm == "md4_fast")
		{
			for(unsigned int i=0; i<nTargetHashes; i++)
				MD4MeetInTheMiddlePreprocessing(wu.m_hashvalues[i]);
		}		

		Executor *exec = ExecutorFactory::Get("BasicExecutor");
		exec->Execute(this, wu, pDevice, pContext);
	}

	virtual bool IsGPUCapable()
	{
#ifdef CUDA_ENABLED
		return true;
#else
		return false;
#endif
	}
	virtual bool IsCPUCapable()
	{
		return false;
	}
};

string ReadPtx(string name)
{
	// Directorios a probar
	string dirs[] = { string("./"), string("ptx/"), string("/usr/lib/cracker/ptx/") };
	string code;
	
	for(int i = 0; i < sizeof(dirs)/sizeof(string); i++)
	{
		string path = dirs[i] + name + ".ptx";
		ifstream myfile(path.c_str());
		if(!myfile) continue;
		
		char line[1024];
		while(!myfile.eof())
		{
			myfile.getline(line, 1024);
			code += line;
			code += "\n";
		}
		
		return string(code);
	}
	
	string strErr = string("Failed to open CUDA module file ") + fname;
	ThrowCustomError(strErr.c_str());
}

#endif
