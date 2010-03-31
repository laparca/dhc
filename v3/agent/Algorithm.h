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


/*!
	@file Algorithm.h
	
	@brief Declararation of an algorithm functionality.
 */

#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <string>
#include <vector>
#include "WorkUnit.h"

using namespace std;

/*!
	@brief
 */
class Algorithm
{
public:
	/*!
		@brief Returns the name of the algorithm
	 */
	virtual string GetName() = 0;
	virtual int  HashLength() = 0;
	virtual void ExecuteCPU() = 0;
	void ExecuteGPU(Device *pDevice);
	virtual bool IsGPUCapable() = 0;
	virtual bool IsCPUCapable() = 0;
	
	/*!
		@brief Prepares information to accelerate hash attack or to perform better the functionality
		@param wo
	 */
	virtual void Prepare(WorkUnit & wo) = 0;


private:
	static vector<Algorithm *> vAlgorithms;
public:
	static void RegisterAlgorithm(Algorithm *pAlgorithm)
	{
		vAlgorithms.push_back(pAlgorithm);
	}
	static Algorithm *GetAlgorithm(string name)
	{
		for(vector<Algorithm *>::iterator it = vAlgorithms.begin(); it != vAlgorithms.end(); it++)
		{
			if(*it->GetName() == name)
				return *it;
		}
		return NULL;
	}
};

#endif