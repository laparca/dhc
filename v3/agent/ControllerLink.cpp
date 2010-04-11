/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* Copyright (C) 2010 Samuel Rodriguez Sevilla
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

/*!
	@file ControllerLink.cpp
	
	@brief Implementation of ControllerLink
 */

#include "agent.h"
#include "XmlParser.h"
#include "Algorithm.h"

using namespace std;

extern string g_server;
extern string g_hostname;

/* This initialization is mandatory */
INIT_ALGORITHMS();

size_t curlwritecallback(void *ptr, size_t size, size_t nmemb, void *stream);

/*!
	@brief Initializes a controller link.
	
	This function may or may not initiate a persistent connection to the controller. (The current implementation does not,
	but this may change in a future version.)
	
	@param pDev The device which will process work units from this link (used for server side stats)
 */
ControllerLink::ControllerLink(ComputeDevice* pDev)
{
	//Libcurl init
	m_pCurl = curl_easy_init();
	if(NULL == m_pCurl)
		ThrowError("libcurl init failed");
		
	//Set up description
	if(pDev->bGPU)
		m_type="GPU";
	else
		m_type="core";
	char num[64];
	sprintf(num, "%d", pDev->index);
	m_num = num;
}

/*!
	@brief Destroys a controller link and frees resources.
	
	If a persistent connection was created, it will be disconnected at this time.
 */
ControllerLink::~ControllerLink()
{
	curl_easy_cleanup(m_pCurl);
}

/*!
	@brief Retrieves a work unit from the controller.
	
	@param wu Object to store the work unit in
	@param alglist List of algorithms supported
	
	@return True if a work unit was successfully retrieved, false if no work is available
 */
bool ControllerLink::GetWorkUnit(WorkUnit& wu, const vector<string>& alglist)
{
	//Initialize default values
	wu.m_bCracked.clear();
	wu.m_hashvalues.clear();
	wu.m_hashids.clear();
	wu.m_collisions.clear();
	
	//Generate algorithm list
	string algs = "";
	for(unsigned int i=0; i<alglist.size(); i++)
	{
		if(i != 0)
			algs += ",";
		algs += alglist[i];
	}
	
	//Format our request
	string request = g_server + "?action=getwu&version=3.2&hostname=" + g_hostname + "&type=" + m_type + "&num=" + m_num + "&accept-algorithms=" + algs;
		
	//Send to the server
	string recvdata;
	curl_easy_reset(m_pCurl);
	curl_easy_setopt(m_pCurl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, curlwritecallback);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &recvdata);
	curl_easy_setopt(m_pCurl, CURLOPT_URL, request.c_str());
	curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0);		//TODO: control this by command line arg
	if(0 != curl_easy_perform(m_pCurl))
		ThrowError("libcurl error");
		
	//Parse XML
	XmlParser* parser = NULL;
	try
	{
		parser = new XmlParser(recvdata);
	}
	catch(std::string err)
	{
		cerr << "Failed to parse work unit! Expected well formed XML, got:" << endl;
		string str = recvdata.substr(0, 256);
		cerr << str.c_str() << endl;
		return false;
	}
	
	//Process it
	const XmlNode* pRoot = parser->GetRoot();
	if(pRoot == NULL)
	{
		cerr << "Failed to parse work (no root node):" << endl;
		string str = recvdata.substr(0, 256);
		cerr << str.c_str() << endl;
		return false;
	}
	if(pRoot->GetType() == "workunit")
	{
		//Work unit, process it
		//TODO: Add more sanity checking (e.g. can only have one of each tag other than <hash> per work unit)
		for(unsigned int i=0; i<pRoot->GetChildCount(); i++)
		{
			//Skip the node if it's not a tag
			const XmlNode* pNode = pRoot->GetChildNode(i);
			if(pNode->GetNodeType() != XmlNode::NODETYPE_TAG)
				continue;
				
			//Must have one and only one child
			if(pNode->GetChildCount() != 1)
				continue;
				
			//Get the item
			string type = pNode->GetType();
			
			//Get the child and validate it
			const XmlNode* pData = pNode->GetChildNode(0);
			if(pData->GetNodeType() != XmlNode::NODETYPE_TEXT)
				continue;
				
			//Get the value
			string txt = pData->GetBody();
			
			//Process it
			if(type == "id")
				wu.m_id = txt;
			else if(type == "algorithm")
				wu.m_algorithm = txt;
			else if(type == "charset")
			{
				//Decode the character set
				wu.m_charset = "";
				for(unsigned int j=0; j<txt.length(); j++)
				{
					char code = txt[j];
					switch(code)
					{
					case 'a':
						wu.m_charset += "abcdefghijklmnopqrstuvwxyz";
						break;
					case 'A':
						wu.m_charset += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
						break;
					case '1':
						wu.m_charset += "1234567890";
						break;
					case '!':
						wu.m_charset += "`~!@#$%^&*()-=_+[]\\{}|;':\",./<>?";
						break;
					case 's':
						wu.m_charset += ' ';
						break;
					case 'n':
						wu.m_charset += '\n';
						break;
					default:
						ThrowCustomError("Unrecognized charset abbreviation in work unit");
					}
				}
			}
			else if(type == "hash")
			{			
				//Save hash ID
				for(unsigned int i=0; i<pNode->GetAttributeCount(); i++)
				{
					if(pNode->GetAttributeName(i) == "id")
					{
						unsigned int id;
						sscanf(pNode->GetAttributeValue(i).c_str(), "%u", &id);
						wu.m_hashids.push_back(id);
						wu.m_collisions.push_back("");
					}
				}

				//Decode (algorithm must come first)
				//TODO: Verify this is indeed the case
				unsigned char* salt = new unsigned char[16];
				unsigned char* hash = new unsigned char[256];
				DecodeHash(wu.m_algorithm, txt, wu.m_hashlen, hash, salt);
				wu.m_hashvalues.push_back(hash);
				wu.m_salts.push_back(salt);
				wu.m_bCracked.push_back(false);
			}
			else if(type == "start")
			{
				wu.m_start = txt;
				if(txt.length() >= MAX_BASEN_LENGTH - 1)
					ThrowError("Start guess too long");
			}
			else if(type == "end")
			{
				wu.m_end = txt;
				if(txt.length() >= MAX_BASEN_LENGTH - 1)
					ThrowError("End guess too long");
			}
			else
				ThrowCustomError("Unknown tag type in work unit");
		}		
		delete parser;
		return true;
	}
	else if(pRoot->GetType() == "nowork")
	{
		//Look up attributes, see if they have a reason
		for(unsigned int i=0; i<pRoot->GetAttributeCount(); i++)
		{
			if(pRoot->GetAttributeName(i) == "reason")
			{
				string s = pRoot->GetAttributeValue(i);
				if(s == "idle")
				{
					//No action needed
				}
				else if(s == "version")
				{
					cerr << "Warning: no work available because controller is running a different protocol version" << endl;
				}
				break;
			}
		}
				
		delete parser;
		return false;
	}
	else
	{
		cerr << "Warning: Invalid work unit (expected <nowork> or <workunit>, found <" << pRoot->GetType().c_str() << ">)" << endl;
		return false;
	}
}

/*!
	@brief Submits the results of processing a work unit.
	
	@param wu The work unit to submit
 */
void ControllerLink::SubmitResults(WorkUnit& wu)
{
	//Format our request
	//TODO: URLencode collision
	//TODO: See if we should use POST data for this
	string request = g_server + "?action=submitwu&version=3.2&wuid=" + wu.m_id + "&dt=" + wu.m_dt + "&speed=" + wu.m_speed;
	
	//Add collisions
	char sbuf[512];
	unsigned int hits=0;
	for(unsigned int i=0; i<wu.m_collisions.size(); i++)
	{
		if(wu.m_collisions[i] != "")
		{
			//Add collision to URL
			sprintf(sbuf, "&hash%d=%u&cleartext%d=%s", hits, wu.m_hashids[i], hits, wu.m_collisions[i].c_str());
			request += sbuf;
			
			//Bump hit count
			hits++;
		}
	}
	sprintf(sbuf, "&collisions=%d", hits);
	request += sbuf;
	
	//Send to the server
	string recvdata;
	curl_easy_reset(m_pCurl);
	curl_easy_setopt(m_pCurl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, curlwritecallback);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &recvdata);
	curl_easy_setopt(m_pCurl, CURLOPT_URL, request.c_str());
	curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0);		//TODO: control this by command line arg
	if(0 != curl_easy_perform(m_pCurl))
		ThrowError("libcurl error");
		
	if(recvdata != "ok")
	{
		//Fix bug #142: this should not be a fatal error
		cout << "Error submitting results to server : " << recvdata.c_str() << endl;
	}
}

void ControllerLink::DecodeHash(
	const std::string& algorithm,
	const std::string& text,
	unsigned int& hashlen,
	unsigned char* hash,
	unsigned char* salt
	)
{
	Algorithm *alg = Algorithm::GetAlgorithm(algorithm);
	if(alg != NULL)
		hashlen = alg->HashLength();
	else
		ThrowCustomError("Unknown hash function");
/*
	if(algorithm == "md4" || 
		algorithm == "md4_fast" || 
		algorithm == "md5" ||
		algorithm == "md5crypt" ||
		algorithm == "ntlm")
	{
		hashlen = 16;
	}
	else if(algorithm == "sha1")
		hashlen = 20;
	else if(algorithm == "sha256")
		hashlen = 32;
	else	
		ThrowCustomError("Unknown hash function");
*/

	if(algorithm != "md5crypt" && hashlen*2 != text.length())
		ThrowError("Invalid hash length");
		
	//Sanity check
	if(hashlen > 256)
		ThrowError("Hash length too long");
	if(text.length() == 0)
		ThrowError("Empty hash");
	salt[0] = '\0';
	
	//Passwd hash of some sort
	if(text[0] == '$')
	{
		//Make sure it's md5crypt (DES, etc. not supported atm)
		if(text[1] != '1' || text[2] != '$')
			ThrowError("Unsupported hash format (expected $1$)");
		
		//Start reading the salt	
		unsigned int i=3;
		for(; i<19 && i<text.size() && text[i] != '$'; i++)
			salt[i-3] = text[i];
		salt[i-3] = '\0';
				
		//Generate reverse charset
		unsigned char b64charset[] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		int rcharset[256] = {0};
		for(int i=0; i<64; i++)
			rcharset[b64charset[i]] = i;

		//TODO: validate DES
		//if((text.size()-i) != 20)
		//	ThrowError("Invalid hash length");

		//Decode the hash
		unsigned int base = i+1;
		unsigned int iOut = 0;
		int scramble[16] = {12, 6, 0, 13, 7, 1, 14, 8, 2, 15, 9, 3, 5, 10, 4, 11};
		//cout << endl << text.c_str()+base << endl;
		for(i=0; (i+base) < text.size(); i+=4)
		{
			//Get the four input values
			unsigned char num[4] = 
			{
				rcharset[static_cast<int>(text[i  +base])],
				rcharset[static_cast<int>(text[i+1+base])],
				rcharset[static_cast<int>(text[i+2+base])],
				rcharset[static_cast<int>(text[i+3+base])]
			};
			
			//Merge down to 3
			unsigned char pch[3] = 
			{
				 num[0]        |  ( (num[1] & 0x3) << 6),
				(num[1] >> 2)  |  (  num[2]        << 4 ),
				(num[2] >> 4)  |  (  num[3]        << 2)
			};

			//Copy it (including the scramble pattern)
			for(int j=0; j<3 && iOut < 16; j++)
				hash[scramble[iOut++]] = pch[j];
		}
	}
	
	//Hex encoding
	else
	{
		if(text.length() != hashlen*2)
			ThrowError("Invalid hash length");
		
		for(unsigned int i=0; i<hashlen; i++)
		{
			char test[3]=
			{
				text[i*2],
				text[i*2 + 1],
				'\0'
			};
			int digit;
			sscanf(test, "%x", &digit);
			hash[i] = digit & 0xFF;
		}
	}
}

/*!
	@brief Write callback for libcurl
	
	This function expects to be called with an std::string* in stream.
	
	See libcurl docs for more details.
	
	@param ptr Temporary buffer to read from
	@param size Size of an element of ptr, in bytes
	@param nmemb Number of elements in ptr
	@param stream Buffer to write to
	@return Number of bytes processed
 */
size_t curlwritecallback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	//Get the data size
	size_t sz = size*nmemb;
	
	//Null terminate it
	char* buf = new char[sz+1];
	memcpy(buf, ptr, sz);
	buf[sz]='\0';
	
	//Append to our string
	string& str = *reinterpret_cast<string*>(stream);
	str += buf;
	
	//Clean up our copy
	delete[] buf;
	
	//Done
	return sz;
}
