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
	@file WorkUnit.h
	
	@brief Declaration of WorkUnit class
 */

#ifndef WorkUnit_h
#define WorkUnit_h

/*!
	@brief A work unit
 */
class WorkUnit
{
public:
	////////////////////////////////////////////////////////////////////////////
	//Global values
	
	/*!
		@brief Work unit ID (opaque handle, used by controller only)
	 */
	std::string m_id;
	
	////////////////////////////////////////////////////////////////////////////
	//Inputs (recieved from controller)
	
	/*!
		@brief Hash value(s) (raw binary)
	 */
	std::vector<unsigned char*> m_hashvalues;
	
	/*!
		@brief Salt(s) (raw binary)
		
		TODO: add salt length here
	 */
	std::vector<unsigned char*> m_salts;
	
	/*!
		@brief Character set
	 */
	std::string m_charset;
	
	/*!
		@brief Hash algorithm
	 */
	std::string m_algorithm;
	
	/*!
		@brief Start guess
	 */
	std::string m_start;
	
	/*!
		@brief End guess
	 */
	std::string m_end;

	/*!
		@brief Length of each hash, in bytes
	 */
	unsigned int m_hashlen;
		
	
	////////////////////////////////////////////////////////////////////////////
	//Outputs (send back to controller when we finish)
	
	/*!
		@brief Set to true if we got cracked
	 */
	std::vector<bool> m_bCracked;

	/*!
		@brief Current hash IDs
	 */
	std::vector<unsigned int> m_hashids;
	
	/*!
		@brief Cracked value (element i is valid only if m_bCracked[i] is true)
	 */
	std::vector<std::string> m_collisions;
	
	/*!
		@brief Time spent on this work unit
	 */
	std::string m_dt;
	
	/*!
		@brief Crack speed (millions of hashes per second, string format)
	 */
	std::string m_speed;
};

#endif
