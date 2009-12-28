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
	@file md4nltm_kernel_core.h
	
	@brief Common round functions shared by MD4 / NTLM kernels
 */
 
//Left rotate
#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

//Round ops
//F and H are same as MD5, G is different
#define Ff(b,c,d) (((b) & (c)) | (~(b) & (d)))
#define Fg(b,c,d) (((b) & (c)) | ((b) & (d)) | ((c) & (d)) )
#define Fh(b,c,d) ((b) ^ (c) ^ (d))

//A single round of MD4. Similar to MD5 but adds a fixed constant Q before shifting,
//instead of adding B after the shift.
#define md4round_core(a,b,c,d,k,s,f,q) a = ROTL(a + f(b,c,d) + buf##k + q,s);

//Wrappers for md4round_core with appropriate round functions and constants
#define md4round_f(a,b,c,d,k,s) md4round_core(a,b,c,d,k,s,Ff,0x00000000)
#define md4round_g(a,b,c,d,k,s) md4round_core(a,b,c,d,k,s,Fg,0x5A827999)
#define md4round_h(a,b,c,d,k,s) md4round_core(a,b,c,d,k,s,Fh,0x6ED9EBA1)
 
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
#ifdef NTLM
unsigned int buf14 = len * 16;
#else
unsigned int buf14 = len * 8;
#endif
unsigned int buf15 = 0;

#ifdef NTLM
	//Message data (deliberate fall-through)
	//Assume message is a multiple of 4, we will zero out extra bytes in the padding stage.
	//Ripple carry, etc is done in the InitGuess macros.
	int efflen = len * 2;										//length of actual data being hashed
	unsigned int lstart0=0, lstart1=0;
	unsigned int lm1 = len-1;
	unsigned int lo4 = efflen/4;
	unsigned int carry = 0;		//no initial carry-in
	switch(lo4)
	{
	case 10:
		InitGuess(10, 21,20);
	case 9:
		InitGuess(9, 19,18);
	case 8:
		InitGuess(8, 17,16);
	case 7:
		InitGuess(7, 15,14);
	case 6:
		InitGuess(6, 13,12);
	case 5:
		InitGuess(5, 11,10);
	case 4:
		InitGuess(4, 9,8);
	case 3:
		InitGuess(3, 7,6);
	case 2:
		InitGuess(2, 5,4);
	case 1:
		InitGuess(1, 3,2);
	case 0:
	default:
		InitGuess(0, 1,0);
	}

	//////////////////////////////////////////////////////////////////////////////
	//STEP 2: Append Padding Bits
	unsigned int padding = 0x80 << ( (efflen & 0x3) << 3);
	unsigned int paddmask = 0xFFFFFFFF << ( (efflen & 0x3) << 3);
	switch(len / 2)
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
		AddPadding(9);
		AddPadding(10);
		AddPadding(11);
		//Skip other values since guess length is limited
	}
#else
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
#endif
	
//////////////////////////////////////////////////////////////////////////////
//STEP 3: Initialize Constants
//Variable declarations
unsigned int a,b,c,d;								//Temp variables for MD5 computation
a = 0x67452301;
b = 0xefcdab89;
c = 0x98badcfe;
d = 0x10325476;

//////////////////////////////////////////////////////////////////////////////
//STEP 4: MD4 Rounds
//Round 1
md4round_f(a,b,c,d,0,3);
	md4round_f(d,a,b,c,1,7);
	md4round_f(c,d,a,b,2,11);
	md4round_f(b,c,d,a,3,19);
md4round_f(a,b,c,d,4,3);
	md4round_f(d,a,b,c,5,7);
	md4round_f(c,d,a,b,6,11);
	md4round_f(b,c,d,a,7,19);
md4round_f(a,b,c,d,8,3);
	md4round_f(d,a,b,c,9,7);
	md4round_f(c,d,a,b,10,11);
	md4round_f(b,c,d,a,11,19);
md4round_f(a,b,c,d,12,3);
	md4round_f(d,a,b,c,13,7);
	md4round_f(c,d,a,b,14,11);
	md4round_f(b,c,d,a,15,19);

//Round 2
md4round_g(a,b,c,d,0,3);
	md4round_g(d,a,b,c,4,5);
	md4round_g(c,d,a,b,8,9);
	md4round_g(b,c,d,a,12,13);
md4round_g(a,b,c,d,1,3);
	md4round_g(d,a,b,c,5,5);
	md4round_g(c,d,a,b,9,9);
	md4round_g(b,c,d,a,13,13);
md4round_g(a,b,c,d,2,3);
	md4round_g(d,a,b,c,6,5);
	md4round_g(c,d,a,b,10,9);
	md4round_g(b,c,d,a,14,13);
md4round_g(a,b,c,d,3,3);
	md4round_g(d,a,b,c,7,5);
	md4round_g(c,d,a,b,11,9);
	md4round_g(b,c,d,a,15,13);

//Round 3
md4round_h(a,b,c,d,0,3);
	md4round_h(d,a,b,c,8,9);
	md4round_h(c,d,a,b,4,11);
	md4round_h(b,c,d,a,12,15);
md4round_h(a,b,c,d,2,3);
	md4round_h(d,a,b,c,10,9);
	md4round_h(c,d,a,b,6,11);
	md4round_h(b,c,d,a,14,15);
md4round_h(a,b,c,d,1,3);

//Last 7 sub-rounds and final addition can be skipped due to meet-in-the-middle attack
#ifndef MD4_MEET_IN_THE_MIDDLE
	md4round_h(d,a,b,c,9,9);
	md4round_h(c,d,a,b,5,11);
	md4round_h(b,c,d,a,13,15);
md4round_h(a,b,c,d,3,3);
	md4round_h(d,a,b,c,11,9);
	md4round_h(c,d,a,b,7,11);
	md4round_h(b,c,d,a,15,15);
	
//////////////////////////////////////////////////////////////////////////////
//STEP 5: Output

a += 0x67452301;
b += 0xefcdab89;
c += 0x98badcfe;
d += 0x10325476;
#endif
