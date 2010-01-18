/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* All rights reserved.                                                        *
*                                                                             *
* SHA256 Implementation Copyright (C) 2010 Samuel Rodriguez-Sevilla           *
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
	@file sha256_kernel.cu
	
	@brief CUDA implementation of SHA256
 */

//Textures
texture<int, 1, cudaReadModeElementType> texCharset;

#define InitGuess(num, a,b,c,d)				\
		{										\
			/* Calculate the four indices */	\
			LstartInit(lstart3, a);				\
			LstartInit(lstart2, b);				\
			LstartInit(lstart1, c);				\
			LstartInit(lstart0, d);				\
			/* Pack four elements into the int (if we exceed length, padding will overwrite the garbage) */	\
			w##num = 							\
			(charset[lstart3] << 24) | 			\
			(charset[lstart2] << 16) | 			\
			(charset[lstart1] << 8) |  			\
			 charset[lstart0];					\
		}
		 
#define LstartInit(ls, num)								\
		{												\
			/* Get initial value and apply carry-in */	\
			ls = carry + start[num]; 					\
			/* Rightmost value? Bump by index */		\
			if(num == lm1) 								\
				ls += index;							\
			/* Carry out */								\
			if(ls >= base && num<len)					\
			{											\
				/* Calculate carry */					\
				carry = ls / base;						\
				/* Update this digit */					\
				ls %= base;								\
			}											\
			else										\
				carry = 0;								\
		}


/*!
	@brief CUDA implementation of SHA256	
	Thread-per-block requirement: minimum 64
	
	@param gtarget Target value (five ints, little endian)
	@param gstart Start index in charset (array of 32 ints, data is left aligned, unused values are at right)
	@param gsalt Salt (not used)
	@param status Set to true by a thread which succeeds in cracking the hash
	@param output Set to the collision by a thread which succeeds in cracking the hash
	@param base Length of the character set (passed in texCharset texture)
	@param len Length of valid data in gstart
	@param saltlen Length of salt (not used)
	@param hashcount Number of hashes being tested (not used)
 */
extern "C" __global__ void sha256Kernel(int* gtarget, int* gstart, char* gsalt, char* status, char* output, int base, int len, int saltlen, int hashcount)
{
	//Get our position in the grid
	int index = (blockDim.x * blockIdx.x) + threadIdx.x;
	
	//Cache charset in shmem
	__shared__ char charset[256];
	if(threadIdx.x < ceil((float)base / 4))
	{
		int* ccs = (int*)&charset[0];
		ccs[threadIdx.x] = tex1Dfetch(texCharset, threadIdx.x);
	}
	
	//Cache start value
	__shared__ int start[32];
	if(threadIdx.x < len)
		start[threadIdx.x] = gstart[threadIdx.x];
	
	//Cache target value (do byte-swap here)
	__shared__ int target[8];
	if(threadIdx.x < 8)
		target[threadIdx.x] = bswap(gtarget[threadIdx.x]);
	
	//Wait for all cache filling to finish
	__syncthreads();
	
	//Core round functions
	#include "sha256_kernel_core.h"
		
	//Test the output
	int* pt = (int*)target;
	if( (pt[0] == a) && (pt[1] == b) && (pt[2] == c) && (pt[3] == d) && (pt[4] == e) && (pt[5] == f) && (pt[6] == g) && (pt[7] == h))
	{
		//If we get here, we must be a match! Save the result and quit
		*status = 1;
		unsigned int lo4 = len/4;
		
		unsigned int lstart0=0, lstart1=0, lstart2=0, lstart3=0;
		unsigned int lm1 = len-1;
		unsigned int carry = 0;		//no initial carry-in
		switch(lo4)
		{
		case 7:
			InitGuess(7, 31,30,29,28);
		case 6:
			InitGuess(6, 27,26,25,24);
		case 5:
			InitGuess(5, 23,22,21,20);
		case 4:
			InitGuess(4, 19,18,17,16);
		case 3:
			InitGuess(3, 15,14,13,12);
		case 2:
			InitGuess(2, 11,10,9,8);
		case 1:
			InitGuess(1, 7,6,5,4);
		case 0:
		default:
			InitGuess(0, 3,2,1,0);
		}
		
		switch(lo4)
		{
		SaveOutput(7);
		SaveOutput(6);
		SaveOutput(5);
		SaveOutput(4);
		SaveOutput(3);	
		SaveOutput(2);
		SaveOutput(1);
		SaveOutput(0);
		}
	}
}
