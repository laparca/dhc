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
	@file MD4MeetInTheMiddleAttack.cpp
	
	@brief Preprocessing for the meet-in-the-middle attack on MD4
 */

#include "agent.h"

#define H(X,Y,Z) ((X) ^ (Y) ^ (Z))
#define ROTR(a,shamt) (((a) >> (shamt)) | ((a) << (32 - (shamt))))
#define reverse_md4round_core(a,b,c,d,k,s,f,q, x)  a = ROTR(a, s); a -= (f(b,c,d) + x + q);
#define reverse_md4round_h(a,b,c,d,k,s, x) reverse_md4round_core(a,b,c,d,k,s,H,0x6ED9EBA1, x)

/**
	@brief Reverses the last 7 sub-rounds of an MD4 hash.
	
	The cleartext must be <= 12 characters for this attack to be successful.
	
	@param hash The hash to reverse
 */
void MD4MeetInTheMiddlePreprocessing(unsigned char* hash)
{
	//Grab output variables
	unsigned int* pout = reinterpret_cast<unsigned int*>(hash);
	unsigned int& a = pout[0];
	unsigned int& b = pout[1];
	unsigned int& c = pout[2];
	unsigned int& d = pout[3];
	
	//Start reversing by subtracting the output state
	a -= 0x67452301;
	b -= 0xefcdab89;
	c -= 0x98badcfe;
	d -= 0x10325476;
	
	//Reverse the last 7 sub-rounds
		reverse_md4round_h(b,c,d,a,15,15, 0);
		reverse_md4round_h(c,d,a,b,7,11, 0);
		reverse_md4round_h(d,a,b,c,11,9, 0);
	reverse_md4round_h(a,b,c,d,3,3, 0);	
		reverse_md4round_h(b,c,d,a,13,15, 0);
		reverse_md4round_h(c,d,a,b,5,11, 0);
	reverse_md4round_h(d,a,b,c,9,9, 0);
}
