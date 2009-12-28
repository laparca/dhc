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
	@file md4_kernel.cu
	
	@brief CUDA implementation of MD4
 */

//Textures
texture<int, 1, cudaReadModeElementType> texCharset;

//Fake-array macros
#define AddPadding(num) case num: \
		buf##num = (buf##num & ~paddmask) | padding; \
		break
#define InitGuess(num, a,b,c,d)				\
		{										\
			/* Calculate the four indices */	\
			LstartInit(lstart3, a);				\
			LstartInit(lstart2, b);				\
			LstartInit(lstart1, c);				\
			LstartInit(lstart0, d);				\
			/* Pack four elements into the int (if we exceed length, padding will overwrite the garbage) */	\
			buf##num = 							\
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
		
#define SaveOutput(num) case num:					\
		po[num] = buf##num;

/*!
	@brief CUDA implementation of MD4
	
	Thread-per-block requirement: minimum 64
	
	@param gtarget Target value (four ints, little endian)
	@param gstart Start index in charset (array of 32 ints, data is left aligned, unused values are at right)
	@param gsalt Salt (not used)
	@param status Set to true by a thread which succeeds in cracking the hash
	@param output Set to the collision by a thread which succeeds in cracking the hash
	@param base Length of the character set (passed in texCharset texture)
	@param len Length of valid data in gstart
	@param saltlen Length of salt (not used)
	@param hashcount Number of hashes being tested (not used)
 */
extern "C" __global__ void md4Kernel(int* gtarget, int* gstart, char* gsalt, char* status, char* output, int base, int len, int saltlen, int hashcount)
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
	
	//Cache target value
	__shared__ int target[4];
	if(threadIdx.x < 4)
		target[threadIdx.x] = gtarget[threadIdx.x];
	
	//Wait for all cache filling to finish
	__syncthreads();
	
	//Guess generation and round structure
	#include "md4ntlm_kernel_core.h"
	
	//Check results
	if(target[0] == a && target[1] == b && target[2] == c && target[3] == d)
	{
		*status = 1;
		
		int* po = (int*)output;
		switch(len / 4)
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

/*!
	@brief CUDA implementation of MD4 with meet-in-the-middle attack
	
	Thread-per-block requirement: minimum 64
	
	@param gtarget Target value (four ints, little endian). The last seven sub-rounds have been reversed by this point.
	@param gstart Start index in charset (array of 32 ints, data is left aligned, unused values are at right)
	@param gsalt Salt (not used)
	@param status Set to true by a thread which succeeds in cracking the hash
	@param output Set to the collision by a thread which succeeds in cracking the hash
	@param base Length of the character set (passed in texCharset texture)
	@param len Length of valid data in gstart
	@param saltlen Length of salt (not used)
	@param hashcount Number of hashes being tested (not used)
 */
extern "C" __global__ void md4_fastKernel(int* gtarget, int* gstart, char* gsalt, char* status, char* output, int base, int len, int saltlen, int hashcount)
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
	
	//Cache target value
	__shared__ int target[4];
	if(threadIdx.x < 4)
		target[threadIdx.x] = gtarget[threadIdx.x];
	
	//Wait for all cache filling to finish
	__syncthreads();
	
	//Guess generation and round structure
	#define MD4_MEET_IN_THE_MIDDLE
	#include "md4ntlm_kernel_core.h"
	
	//Check results
	if(target[0] == a && target[1] == b && target[2] == c && target[3] == d)
	{
		*status = 1;
		
		int* po = (int*)output;
		switch(len / 4)
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

/*!
	@brief CUDA implementation of MD4 with batch cracking support
	
	Thread-per-block requirement: minimum 64
	
	@param gtarget Target value (four ints, little endian)
	@param gstart Start index in charset (array of 32 ints, data is left aligned, unused values are at right)
	@param gsalt Salt (not used)
	@param status Set to true by a thread which succeeds in cracking the hash
	@param output Set to the collision by a thread which succeeds in cracking the hash
	@param base Length of the character set (passed in texCharset texture)
	@param len Length of valid data in gstart
	@param saltlen Length of salt (not used)
	@param hashcount Number of hashes being tested
 */
extern "C" __global__ void md4BatchKernel(int* gtarget, int* gstart, char* gsalt, char* status, char* output, int base, int len, int saltlen, int hashcount)
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
	
	//Cache target value
	__shared__ int target[4 * 128];
	if(threadIdx.x < 64)
	{
		int td = threadIdx.x;
		if(td < hashcount)
		{
			for(int i=0; i<4; i++)
				target[4*td + i] = gtarget[4*td + i];
		}
		if(td > 64)
		{
			td -= 64;
			if(td < hashcount)
			{
				for(int i=0; i<4; i++)
					target[4*td + i] = gtarget[4*td + i];
			}
		}
	}
	
	//Wait for all cache filling to finish
	__syncthreads();
	
	//Guess generation and round structure
	#include "md4ntlm_kernel_core.h"
	
	//Check results
	for(int i=0; i<hashcount; i++)
	{
		int* xtarget = target + (4*i);
		
		//Check results
		if(xtarget[0] == a && xtarget[1] == b && xtarget[2] == c && xtarget[3] == d)
		{
			status[i] = 1;
			
			int* po = (int*)output + (i*8);
			switch(len / 4)
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
}


/*!
	@brief CUDA implementation of MD4 with batch cracking support and cryptanalysis
	
	Thread-per-block requirement: minimum 64
	
	@param gtarget Target value (four ints, little endian)
	@param gstart Start index in charset (array of 32 ints, data is left aligned, unused values are at right)
	@param gsalt Salt (not used)
	@param status Set to true by a thread which succeeds in cracking the hash
	@param output Set to the collision by a thread which succeeds in cracking the hash
	@param base Length of the character set (passed in texCharset texture)
	@param len Length of valid data in gstart
	@param saltlen Length of salt (not used)
	@param hashcount Number of hashes being tested
 */
extern "C" __global__ void md4_fastBatchKernel(int* gtarget, int* gstart, char* gsalt, char* status, char* output, int base, int len, int saltlen, int hashcount)
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
	
	//Cache target value
	__shared__ int target[4 * 128];
	if(threadIdx.x < 64)
	{
		int td = threadIdx.x;
		if(td < hashcount)
		{
			for(int i=0; i<4; i++)
				target[4*td + i] = gtarget[4*td + i];
		}
		if(td > 64)
		{
			td -= 64;
			if(td < hashcount)
			{
				for(int i=0; i<4; i++)
					target[4*td + i] = gtarget[4*td + i];
			}
		}
	}
	
	//Wait for all cache filling to finish
	__syncthreads();
	
	//Guess generation and round structure
	#define MD4_MEET_IN_THE_MIDDLE
	#include "md4ntlm_kernel_core.h"
	
	//Check results
	for(int i=0; i<hashcount; i++)
	{
		int* xtarget = target + (4*i);
		
		//Check results
		if(xtarget[0] == a && xtarget[1] == b && xtarget[2] == c && xtarget[3] == d)
		{
			status[i] = 1;
			
			int* po = (int*)output + (i*8);
			switch(len / 4)
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
}


