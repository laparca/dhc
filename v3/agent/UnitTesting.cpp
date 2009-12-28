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
	@file UnitTesting.cpp
	
	@brief Unit tests
 */
 
#include "agent.h"
#include "XmlParser.h"

using namespace std;

struct TestVector
{
	string desc;			//Test description
	string alg;				//Algorithm code	
	string hash;			//The hash
	bool bCrackable;		//Should it be crackable?
	string plaintext;		//If crackable, the plaintext
};

/*
bool CpuTests()
{
	#if (LINUX && AMD64)
	
	cout << "CPU hashes..." << endl;
	
	//Default work unit configuration: 4 char letters only
	WorkUnit wu;
	wu.m_charset = "abcdefghijklmnopqrstuvwxyz";
	wu.m_start = "aaaa";
	wu.m_end = "zzzz";
	
	//Set up our test vectors
	const int testcount = 2;
	TestVector vectors[testcount]=
	{
		{"* MD5      (1)     ", "md5", "098f6bcd4621d373cade4e832627b4f6", true,  "test"},
		{"* MD5      (2)     ", "md5", "d8578edf8458ce06fbc5bb76a58c5ca4", false, ""}
	};
	
	//Run tests
	bool ret = true;
	for(int i=0; i<testcount; i++)
	{
		//Setup
		TestVector& vec = vectors[i];
		cout << vec.desc.c_str();
		wu.m_hash = vec.hash;
		wu.m_algorithm = vec.alg;
		wu.m_bCracked = false;
		
		//Try cracking
		DoWorkUnitOnCPU(wu, 0);
		
		//See what happened
		bool bPass = vec.bCrackable == wu.m_bCracked;
		if(vec.bCrackable)
			bPass &= (vec.plaintext == wu.m_collision);
			
		if(bPass)
			cout << PASS << " (speed = " << wu.m_speed.c_str() << " MHz)" << endl;
		else
		{
			cout << FAIL << endl;
			if(wu.m_bCracked)
				cout << "  Found unexpected collision " << wu.m_collision << endl;
			else
				cout << "  Failed to find collision " << vec.plaintext << endl;
			ret = false;
		}
	}
	
	cout << endl;
	#endif	
	return ret;
}*/

bool DoUnitTests(bool bTestGPU)
{
	#ifdef CUDA_ENABLED
	
	//Initialize (first device by default, this should be configurable in the future)
	Device* pDevice = new Device(0);
	CudaContext* pContext = new CudaContext(*pDevice, CudaContext::Yield);
	
	//Show header
	cout << "Using device " << pDevice->GetName() << " for GPU hashes" << endl;
	
	#endif
	
	//Load test vector file
	XmlParser parser("agent/testvectors.xml");
	
	//Process each test suite
	bool ret = true;
	const XmlNode* pRoot = parser.GetRoot();
	int iTestSuite = 0;
	for(size_t i=0; i<pRoot->GetChildCount(); i++)
	{
		//Grab the node and make sure it's a test suite
		const XmlNode* pSuite = pRoot->GetChildNode(i);
		if(pSuite->GetNodeType() != XmlNode::NODETYPE_TAG)
			continue;
		if(pSuite->GetType() != "suite")
		{
			cout << "Warning: skipping unrecognized tag in testvectors.xml" << endl;
			continue;
		}
		
		//Configure global settings for the test suite
		WorkUnit wuInit;
		bool bGpuTest = false;
		for(size_t j=0; j<pSuite->GetAttributeCount(); j++)
		{
			if(pSuite->GetAttributeName(j) == "device")
			{
				if(pSuite->GetAttributeValue(j) == "gpu")
					bGpuTest = true;
			}
			else if(pSuite->GetAttributeName(j) == "charset")
				wuInit.m_charset = pSuite->GetAttributeValue(j);
			else if(pSuite->GetAttributeName(j) == "start")
				wuInit.m_start = pSuite->GetAttributeValue(j);
			else if(pSuite->GetAttributeName(j) == "end")
				wuInit.m_end = pSuite->GetAttributeValue(j);
			else
			{
				cout << "Warning: skipping unrecognized value in testvectors.xml" << endl;
				continue;
			}
		}
		
		//TODO: Sanity check inputs
		
		//No CUDA? Skip GPU tests
		#ifndef CUDA_ENABLED
			if(bGpuTest)
				continue;
		#endif
		
		//Print test suite parameters
		iTestSuite ++;
		cout << endl
			<< "Test suite " << iTestSuite << ": "
			<< "device = " << (bGpuTest? "GPU" : "CPU")
			<< ", length = "<< wuInit.m_start.length()
			<< ", charset = " << wuInit.m_charset.c_str()
			<< endl;
			
		//Iterate through each test
		for(size_t j=0; j<pSuite->GetChildCount(); j++)
		{
			WorkUnit wu = wuInit;
			
			const XmlNode* pTest = pSuite->GetChildNode(j);
			if(pTest->GetNodeType() != XmlNode::NODETYPE_TAG)
				continue;
			if(pTest->GetType() != "test")
			{
				cout << "Warning: skipping unrecognized tag in testvectors.xml" << endl;
				continue;
			}
			
			//Configure the test
			string cleartext;
			string desc;
			for(size_t k=0; k<pTest->GetAttributeCount(); k++)
			{
				if(pTest->GetAttributeName(k) == "alg")
					wu.m_algorithm = pTest->GetAttributeValue(k);
				else if(pTest->GetAttributeName(k) == "hash")
				{
					unsigned char* salt = new unsigned char[16];
					unsigned char* hash = new unsigned char[256];
					wu.m_hashvalues.clear();
					ControllerLink::DecodeHash(wu.m_algorithm, pTest->GetAttributeValue(k), wu.m_hashlen, hash, salt);
					wu.m_hashvalues.push_back(hash);
					wu.m_salts.push_back(salt);
					wu.m_collisions.push_back("");
				}
				else if(pTest->GetAttributeName(k) == "cleartext")
					cleartext = pTest->GetAttributeValue(k);
				else if(pTest->GetAttributeName(k) == "desc")
					desc = pTest->GetAttributeValue(k);
				else
				{
					cout << "Warning: skipping unrecognized value in testvectors.xml" << endl;
					continue;
				}
			}
			
			//Run it
			printf("* %-10s    ", desc.c_str());
			if(bGpuTest)
			{
				#ifdef CUDA_ENABLED
					DoWorkUnitOnGPU(wu, pDevice, pContext);
				#endif
			}
			else
			{
				#if (LINUX && AMD64)
					DoWorkUnitOnCPU(wu, 0);
				#endif
			}
			
			//See what comes out in the wash!
			if(wu.m_collisions[0] == cleartext)
			{
				cout << PASS << " (speed = " << wu.m_speed.c_str() << " MHz)" << endl;
			}
			else
			{
				cout << FAIL << endl;
				if(wu.m_bCracked[0])
					cout << "  Found unexpected collision " << wu.m_collisions[0] << endl;
				else
					cout << "  Failed to find collision " << cleartext.c_str() << endl;
				ret = false;
			}
			
			//Clean up
			for(unsigned int i=0; i<wu.m_hashvalues.size(); i++)
				delete[] wu.m_hashvalues[i];
			for(unsigned int i=0; i<wu.m_salts.size(); i++)
				delete[] wu.m_salts[i];
		}
	}
	
	cout << endl;
	return ret;
}
