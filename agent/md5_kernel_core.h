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
	@file md5_kernel_core.h
	
	@brief Shared stuff used by md5Kernel() and md5batchKernel()
 */

//////////////////////////////////////////////////////////////////////////////
//STEP 1: Copy buffer
unsigned int buf0 = 0;
unsigned int buf1 = 0;
unsigned int buf2 = 0;
unsigned int buf3 = 0;
unsigned int buf4 = 0;
unsigned int buf5 = 0;
unsigned int buf6 = 0;
unsigned int buf7 = 0;
unsigned int buf8 = 0;
unsigned int buf9 = 0;
unsigned int buf10 = 0;
unsigned int buf11 = 0;
unsigned int buf12 = 0;
unsigned int buf13 = 0;
unsigned int buf14 = len * 8;
unsigned int buf15 = 0;

//Message data (deliberate fall-through)
//Assume message is a multiple of 4, we will zero out extra bytes in the padding stage.
//Ripple carry, etc is done in the InitGuess macros.
unsigned int lstart0=0, lstart1=0, lstart2=0, lstart3=0;
unsigned int lm1 = len-1;
unsigned int lo4 = len/4;
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

//Skip other values since guess length is limited to 32

//////////////////////////////////////////////////////////////////////////////
//STEP 2: Append Padding Bits
unsigned int padding = 0x80 << ( (len & 0x3) << 3);
unsigned int paddmask = 0xFFFFFFFF << ( (len & 0x3) << 3);
switch(len / 4)
{
	AddPadding(0);
	AddPadding(1);
	AddPadding(2);
	AddPadding(3);
	AddPadding(4);
	AddPadding(5);
	AddPadding(6);
	AddPadding(7);
	AddPadding(8);
	//Skip other values since guess length is limited to 32
}
	
/*
//DEBUG: Report status
char* p0 = (char*)&buf0;
if(len==6 &&
	(charset[start[0]] == 'e' && charset[start[1]] == 'm' &&
	charset[start[2]] == 'z' && charset[start[3]] == 'e' &&
	charset[start[4]] == 'y' && charset[start[5]] == '7' )

	&& index == 0
	)
{
	*status = 1;
	int* pi = (int*) output;
	
	pi[0] = buf0;
	pi[1] = buf1;
	pi[2] = buf2;
	pi[3] = lo4;
}
return;
*/

//////////////////////////////////////////////////////////////////////////////
//STEP 3: Initialize Constants
//Variable declarations
unsigned int a,b,c,d;								//Temp variables for MD5 computation
a = 0x67452301;
b = 0xefcdab89;
c = 0x98badcfe;
d = 0x10325476;

//////////////////////////////////////////////////////////////////////////////
//STEP 4: MD5 Rounds
md5round_f(a,b,c,d,0,7,0xd76aa478);
	md5round_f(d,a,b,c,1,12,0xe8c7b756);
	md5round_f(c,d,a,b,2,17,0x242070db);
	md5round_f(b,c,d,a,3,22,0xc1bdceee);
md5round_f(a,b,c,d,4,7,0xf57c0faf);
	md5round_f(d,a,b,c,5,12,0x4787c62a);
	md5round_f(c,d,a,b,6,17,0xa8304613);
	md5round_f(b,c,d,a,7,22,0xfd469501);
md5round_f(a,b,c,d,8,7,0x698098d8);
	md5round_f(d,a,b,c,9,12,0x8b44f7af);
	md5round_f(c,d,a,b,10,17,0xffff5bb1);
	md5round_f(b,c,d,a,11,22,0x895cd7be);
md5round_f(a,b,c,d,12,7,0x6b901122);
	md5round_f(d,a,b,c,13,12,0xfd987193);
	md5round_f(c,d,a,b,14,17,0xa679438e);
	md5round_f(b,c,d,a,15,22,0x49b40821);

md5round_g(a,b,c,d,1,5,0xf61e2562);
	md5round_g(d,a,b,c,6,9,0xc040b340);
	md5round_g(c,d,a,b,11,14,0x265e5a51);
	md5round_g(b,c,d,a,0,20,0xe9b6c7aa);
md5round_g(a,b,c,d,5,5,0xd62f105d);
	md5round_g(d,a,b,c,10,9,0x02441453);
	md5round_g(c,d,a,b,15,14,0xd8a1e681);
	md5round_g(b,c,d,a,4,20,0xe7d3fbc8);
md5round_g(a,b,c,d,9,5,0x21e1cde6);
	md5round_g(d,a,b,c,14,9,0xc33707d6);
	md5round_g(c,d,a,b,3,14,0xf4d50d87);
	md5round_g(b,c,d,a,8,20,0x455a14ed);
md5round_g(a,b,c,d,13,5,0xa9e3e905);
	md5round_g(d,a,b,c,2,9,0xfcefa3f8);
	md5round_g(c,d,a,b,7,14,0x676f02d9);
	md5round_g(b,c,d,a,12,20,0x8d2a4c8a);

md5round_h(a,b,c,d,5,4,0xfffa3942);
	md5round_h(d,a,b,c,8,11,0x8771f681);
	md5round_h(c,d,a,b,11,16,0x6d9d6122);
	md5round_h(b,c,d,a,14,23,0xfde5380c);
md5round_h(a,b,c,d,1,4,0xa4beea44);
	md5round_h(d,a,b,c,4,11,0x4bdecfa9);
	md5round_h(c,d,a,b,7,16,0xf6bb4b60);
	md5round_h(b,c,d,a,10,23,0xbebfbc70);
md5round_h(a,b,c,d,13,4,0x289b7ec6);
	md5round_h(d,a,b,c,0,11,0xeaa127fa);
	md5round_h(c,d,a,b,3,16,0xd4ef3085);
	md5round_h(b,c,d,a,6,23,0x04881d05);
md5round_h(a,b,c,d,9,4,0xd9d4d039);
	md5round_h(d,a,b,c,12,11,0xe6db99e5);
	md5round_h(c,d,a,b,15,16,0x1fa27cf8);
	md5round_h(b,c,d,a,2,23,0xc4ac5665);

md5round_i(a,b,c,d,0,6,0xf4292244);
	md5round_i(d,a,b,c,7,10,0x432aff97);
	md5round_i(c,d,a,b,14,15,0xab9423a7);
	md5round_i(b,c,d,a,5,21,0xfc93a039);
md5round_i(a,b,c,d,12,6,0x655b59c3);
	md5round_i(d,a,b,c,3,10,0x8f0ccc92);
	md5round_i(c,d,a,b,10,15,0xffeff47d);
	md5round_i(b,c,d,a,1,21,0x85845dd1);
md5round_i(a,b,c,d,8,6,0x6fa87e4f);
	md5round_i(d,a,b,c,15,10,0xfe2ce6e0);
	md5round_i(c,d,a,b,6,15,0xa3014314);
	md5round_i(b,c,d,a,13,21,0x4e0811a1);
md5round_i(a,b,c,d,4,6,0xf7537e82);
	md5round_i(d,a,b,c,11,10,0xbd3af235);
	md5round_i(c,d,a,b,2,15,0x2ad7d2bb);
	md5round_i(b,c,d,a,9,21,0xeb86d391);

//////////////////////////////////////////////////////////////////////////////
//STEP 5: Output

a += 0x67452301;
b += 0xefcdab89;
c += 0x98badcfe;
d += 0x10325476;
