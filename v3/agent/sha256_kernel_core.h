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
	@file sha256_kernel_core.h
	
	@brief Shared stuff used by sha256Kernel() and sha256batchKernel()
 */

/////////////////////////////////////////////////////////////////////////////
//STEP 1: Copy buffer

//Main computation buffer
unsigned int w0=0;
unsigned int w1=0;
unsigned int w2=0;
unsigned int w3=0;
unsigned int w4=0;
unsigned int w5=0;
unsigned int w6=0;
unsigned int w7=0;
unsigned int w8=0;
unsigned int w9=0;
unsigned int w10=0;
unsigned int w11=0;
unsigned int w12=0;
unsigned int w13=0;
unsigned int w14=0;
unsigned int w15=len*8;
unsigned int w16=0, w17=0, w18=0, w19=0, w20=0, w21=0, w22=0, w23=0,
             w24=0, w25=0, w26=0, w27=0, w28=0, w29=0, w30=0, w31=0,
             w32=0, w33=0, w34=0, w35=0, w36=0, w37=0, w38=0, w39=0,
             w40=0, w41=0, w42=0, w43=0, w44=0, w45=0, w46=0, w47=0,
             w48=0, w49=0, w50=0, w51=0, w52=0, w53=0, w54=0, w55=0,
             w56=0, w57=0, w58=0, w59=0, w60=0, w61=0, w62=0, w63=0;


unsigned int k0  = 0x428a2f98;
unsigned int k1  = 0x71374491;
unsigned int k2  = 0xb5c0fbcf;
unsigned int k3  = 0xe9b5dba5;
unsigned int k4  = 0x3956c25b;
unsigned int k5  = 0x59f111f1;
unsigned int k6  = 0x923f82a4;
unsigned int k7  = 0xab1c5ed5;
unsigned int k8  = 0xd807aa98;
unsigned int k9  = 0x12835b01;
unsigned int k10 = 0x243185be;
unsigned int k11 = 0x550c7dc3;
unsigned int k12 = 0x72be5d74;
unsigned int k13 = 0x80deb1fe;
unsigned int k14 = 0x9bdc06a7;
unsigned int k15 = 0xc19bf174;
unsigned int k16 = 0xe49b69c1;
unsigned int k17 = 0xefbe4786;
unsigned int k18 = 0x0fc19dc6;
unsigned int k19 = 0x240ca1cc;
unsigned int k20 = 0x2de92c6f;
unsigned int k21 = 0x4a7484aa;
unsigned int k22 = 0x5cb0a9dc;
unsigned int k23 = 0x76f988da;
unsigned int k24 = 0x983e5152;
unsigned int k25 = 0xa831c66d;
unsigned int k26 = 0xb00327c8;
unsigned int k27 = 0xbf597fc7;
unsigned int k28 = 0xc6e00bf3;
unsigned int k29 = 0xd5a79147;
unsigned int k30 = 0x06ca6351;
unsigned int k31 = 0x14292967;
unsigned int k32 = 0x27b70a85;
unsigned int k33 = 0x2e1b2138;
unsigned int k34 = 0x4d2c6dfc;
unsigned int k35 = 0x53380d13;
unsigned int k36 = 0x650a7354;
unsigned int k37 = 0x766a0abb;
unsigned int k38 = 0x81c2c92e;
unsigned int k39 = 0x92722c85;
unsigned int k40 = 0xa2bfe8a1;
unsigned int k41 = 0xa81a664b;
unsigned int k42 = 0xc24b8b70;
unsigned int k43 = 0xc76c51a3;
unsigned int k44 = 0xd192e819;
unsigned int k45 = 0xd6990624;
unsigned int k46 = 0xf40e3585;
unsigned int k47 = 0x106aa070;
unsigned int k48 = 0x19a4c116;
unsigned int k49 = 0x1e376c08;
unsigned int k50 = 0x2748774c;
unsigned int k51 = 0x34b0bcb5;
unsigned int k52 = 0x391c0cb3;
unsigned int k53 = 0x4ed8aa4a;
unsigned int k54 = 0x5b9cca4f;
unsigned int k55 = 0x682e6ff3;
unsigned int k56 = 0x748f82ee;
unsigned int k57 = 0x78a5636f;
unsigned int k58 = 0x84c87814;
unsigned int k59 = 0x8cc70208;
unsigned int k60 = 0x90befffa;
unsigned int k61 = 0xa4506ceb;
unsigned int k62 = 0xbef9a3f7;
unsigned int k63 = 0xc67178f2;

//Scope temporary variables locally
{
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
}

//Convert to big endian
w0  = bswap(w0);
w1  = bswap(w1);
w2  = bswap(w2);
w3  = bswap(w3);
w4  = bswap(w4);
w5  = bswap(w5);
w6  = bswap(w6);
w7  = bswap(w7);
w8  = bswap(w8);
w9  = bswap(w9);
w10 = bswap(w10);
w11 = bswap(w11);
w12 = bswap(w12);
w13 = bswap(w13);
w14 = bswap(w14);
//Don't bswap w15 (length) or w16 (not set yet)

//////////////////////////////////////////////////////////////////////////////
//STEP 3: Initialize Constants
unsigned int
	a = 0x6a09e667,
	b = 0xbb67ae85,
	c = 0x3c6ef372,
	d = 0xa54ff53a,
	e = 0x510e527f,
	f = 0x9b05688c,
	g = 0x1f83d9ab,
	h = 0x5be0cd19;


//////////////////////////////////////////////////////////////////////////////
//STEP 4: SHA-256 Rounds
{
	unsigned int s0,s1;
	unsigned int maj, t1, t2, ch;

	// Extend the sixteen 32-bit words into sixty-four 32-bit words:
	Extend(16,  1, 14,  0,  9);
	Extend(17,  2, 15,  1, 10);
	Extend(18,  3, 16,  2, 11);
	Extend(19,  4, 17,  3, 12);
	Extend(20,  5, 18,  4, 13);
	Extend(21,  6, 19,  5, 14);
	Extend(22,  7, 20,  6, 15);
	Extend(23,  8, 21,  7, 16);
	Extend(24,  9, 22,  8, 17);
	Extend(25, 10, 23,  9, 18);
	Extend(26, 11, 24, 10, 19);
	Extend(27, 12, 25, 11, 20);
	Extend(28, 13, 26, 12, 21);
	Extend(29, 14, 27, 13, 22);
	Extend(30, 15, 28, 14, 23);
	Extend(31, 16, 29, 15, 24);
	Extend(32, 17, 30, 16, 25);
	Extend(33, 18, 31, 17, 26);
	Extend(34, 19, 32, 18, 27);
	Extend(35, 20, 33, 19, 28);
	Extend(36, 21, 34, 20, 29);
	Extend(37, 22, 35, 21, 30);
	Extend(38, 23, 36, 22, 31);
	Extend(39, 24, 37, 23, 32);
	Extend(40, 25, 38, 24, 33);
	Extend(41, 26, 39, 25, 34);
	Extend(42, 27, 40, 26, 35);
	Extend(43, 28, 41, 27, 36);
	Extend(44, 29, 42, 28, 37);
	Extend(45, 30, 43, 29, 38);
	Extend(46, 31, 44, 30, 39);
	Extend(47, 32, 45, 31, 40);
	Extend(48, 33, 46, 32, 41);
	Extend(49, 34, 47, 33, 42);
	Extend(50, 35, 48, 34, 43);
	Extend(51, 36, 49, 35, 44);
	Extend(52, 37, 50, 36, 45);
	Extend(53, 38, 51, 37, 46);
	Extend(54, 39, 52, 38, 47);
	Extend(55, 40, 53, 39, 48);
	Extend(56, 41, 54, 40, 49);
	Extend(57, 42, 55, 41, 50);
	Extend(58, 43, 56, 42, 51);
	Extend(59, 44, 57, 43, 52);
	Extend(60, 45, 58, 44, 53);
	Extend(61, 46, 59, 45, 54);
	Extend(62, 47, 60, 46, 55);
	Extend(63, 48, 61, 47, 56);


	Round( 0); Round( 1); Round( 2); Round( 3); Round( 4); Round( 5); Round( 6); Round( 7);
	Round( 8); Round( 9); Round(10); Round(11); Round(12); Round(13); Round(14); Round(15);
	Round(16); Round(17); Round(18); Round(19); Round(20); Round(21); Round(22); Round(23);
	Round(24); Round(25); Round(26); Round(27); Round(28); Round(29); Round(30); Round(31);
	Round(32); Round(33); Round(34); Round(35); Round(36); Round(37); Round(38); Round(39);
	Round(40); Round(41); Round(42); Round(43); Round(44); Round(45); Round(46); Round(47);
	Round(48); Round(49); Round(50); Round(51); Round(52); Round(53); Round(54); Round(55);
	Round(56); Round(57); Round(58); Round(59); Round(60); Round(61); Round(62); Round(63);

	a += 0x6a09e667;
	b += 0xbb67ae85;
	c += 0x3c6ef372;
	d += 0xa54ff53a;
	e += 0x510e527f;
	f += 0x9b05688c;
	g += 0x1f83d9ab;
	h += 0x5be0cd19;
}
