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
	@file BaseN.cpp
	
	@brief Implementation of arbitrary base arithmetic routines
 */

#include "agent.h"

using namespace std;

/*!
	@brief Total time spent in base-N operations.
	
	Valid only if PROFILING_ENABLED is defined.
 */
double g_tBaseN = 0;

/*!
	@brief Generates a reverse-lookup table from a character set.
	
	A reverse charset is a lookup table which can be used to convert ASCII values from a character set into index values.
	
	@param rcharset Reverse charset
	@param charset Input character set
 */
void GenerateReverseCharset(int* rcharset, const string& charset)
{
	//Clear it
	memset(rcharset, 0, 256*sizeof(int));
	
	//Generate reverse charset
	for(unsigned int i=0; i<charset.size(); i++)
		rcharset[static_cast<int>(charset[i])] = i;
}

/*!
	@brief Parses a string into a base-N integer.
	
	@param num Output buffer
	@param str Input string
	@param rcharset Reverse charset
 */
void BaseNParse(int* num, const string& str, int* rcharset)
{
	//Sanity check
	if(str.length() >= MAX_BASEN_LENGTH)
		ThrowError("input string too long");
	
	//Read digits	
	int len = str.length();
	for(int i=0; i<len; i++)
		num[i] = rcharset[static_cast<int>(str[i])];
		
	//Pad end to a known value so that we can do vector comparisons
	for(int i=len; i<MAX_BASEN_LENGTH; i++)
		num[i] = -1;
}

/*!
	@brief Compares two base-N integers
	
	@param a Value 1
	@param b Value 2
	@param len Length of the numbers (must be same length)
	
	@return True if a is strictly less than b
 */
bool BaseNLess(int* a, int* b, int len)
{
	#ifdef PROFILING_ENABLED
	double start = GetTime();
	#endif
	
	bool ret = false;
	for(int i=0; i<len; i++)
	{
		if(a[i] > b[i])
			break;
		if(a[i] < b[i])
		{
			ret=true;
			break;
		}
	}
	
	#ifdef PROFILING_ENABLED
	g_tBaseN += GetTime() - start;
	#endif
	
	return ret;
}

/*!
	@brief Adds 1 to a base-N integer
	
	@param a Value to increment
	@param base Base of a
	@param len Length of A
 */
void BaseNAdd1(int* a, int base, int len)
{
	#ifdef PROFILING_ENABLED
	double start = GetTime();
	#endif
	
	//Bump the rightmost digit
	a[len-1] ++;
	
	//Propagate carries
	for(int i=len-1; i>=0; i--)
	{
		//No overflow? Done
		if(a[i] < base)
			break;
			
		//Overflow m-s digit? Saturate and quit
		if(i == 0)
		{
			for(int j=0; j<len; j++)
				a[j] = base-1;
			break;
		}
		
		//Nope, carry
		a[i] -= base;
		a[i-1] ++;
	}
	
	#ifdef PROFILING_ENABLED
	g_tBaseN += GetTime() - start;
	#endif
}

/*!
	@brief Adds a value to a base-N integer
	
	@param a Value to increment
	@param base Base of a
	@param len Length of A
	@param val Value to add
 */
void BaseNAdd(int* a, int base, int len, int val)
{
	#ifdef PROFILING_ENABLED
	double start = GetTime();
	#endif
	
	//Bump the rightmost digit
	a[len-1] += val;

	//Propagate carries
	for(int i=len-1; i>=0; i--)
	{
		//No overflow? Done
		if(a[i] < base)
			break;
			
		//Overflow m-s digit? Saturate and quit
		if(i == 0)
		{
			for(int j=0; j<len; j++)
				a[j] = base-1;
			break;
		}
		
		//Calculate carry
		int carry = a[i] / base;
		
		//Calculate our digit
		a[i] %= base;
		
		//Apply carry
		a[i-1] += carry;
	}
	
	#ifdef PROFILING_ENABLED
	g_tBaseN += GetTime() - start;
	#endif
}

