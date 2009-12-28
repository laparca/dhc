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
*******************************************************************************/

/*!
	@file ControllerLink.h
	
	@brief Declaration of ControllerLink
 */

#ifndef ControllerLink_h
#define ControllerLink_h

/*!
	@brief Interface for talking with the controller.
 */
class ControllerLink
{
public:
	ControllerLink(ComputeDevice* pDev);
	~ControllerLink();

	//Gets a WU, if no crack is in progress returns false
	bool GetWorkUnit(WorkUnit& wu, const std::vector<std::string>& alglist);
	
	//Submits results from a work unit
	void SubmitResults(WorkUnit& wu);
	
	//Decodes a hash
	static void DecodeHash(
		const std::string& algorithm,
		const std::string& text,
		unsigned int& hashlen,
		unsigned char* hash,
		unsigned char* salt
		);
	
protected:

	/*!
		@brief Libcurl handle for HTTP requests
	 */
	CURL* m_pCurl;
	
	/*!
		@brief String representation of our device type (e.g. "GPU" / "core")
	*/
	std::string m_type;
	
	/*!
		@brief String representation of our device ID
	*/
	std::string m_num;
};

#endif
