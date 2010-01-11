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
unsigned int w16=0;

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

