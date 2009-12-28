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
	@file md5crypt_kernel.cu
	
	@brief CUDA implementation of MD5crypt
 */

//Textures
texture<int, 1, cudaReadModeElementType> texCharset;

//Left rotate
#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

//Core function
#define md5round_core(a,b,c,d,k,s,t,f) a = ROTL(a + f(b,c,d) + buf[k] + t,s) + b;
#define fastmd5round_core(a,b,c,d,k,s,t,f) a = ROTL(a + f(b,c,d) + k + t,s) + b;

//Round ops
#define Ff(b,c,d) (((b) & (c)) | (~(b) & (d)))
#define Fg(b,c,d) (((b) & (d)) | (~(d) & (c)))
#define Fh(b,c,d) ((b) ^ (c) ^ (d))
#define Fi(b,c,d) ((c) ^ ((b) | ~(d)))

//Rounds
#define md5round_f(a,b,c,d,i,n,t) md5round_core(a,b,c,d,i,n,t,Ff)
#define md5round_g(a,b,c,d,i,n,t) md5round_core(a,b,c,d,i,n,t,Fg)
#define md5round_h(a,b,c,d,i,n,t) md5round_core(a,b,c,d,i,n,t,Fh)
#define md5round_i(a,b,c,d,i,n,t) md5round_core(a,b,c,d,i,n,t,Fi)

#define fastmd5round_f(a,b,c,d,i,n,t) fastmd5round_core(a,b,c,d,i,n,t,Ff)
#define fastmd5round_g(a,b,c,d,i,n,t) fastmd5round_core(a,b,c,d,i,n,t,Fg)
#define fastmd5round_h(a,b,c,d,i,n,t) fastmd5round_core(a,b,c,d,i,n,t,Fh)
#define fastmd5round_i(a,b,c,d,i,n,t) fastmd5round_core(a,b,c,d,i,n,t,Fi)

void __device__ DoMD5Hash(unsigned int* pin, unsigned int* pout, int length)
{
	unsigned char* out = reinterpret_cast<unsigned char*>(pout);
	
	//////////////////////////////////////////////////////////////////////////////
	//STEP 1: Copy buffer and append padding bits
	//Generate the string
	unsigned int buf[16];								//Our computation buffer
	for(int j=floor(length/4.0f); j<16; j++)			//Zero the end
		buf[j] = 0;
	for(int j=0; j<ceilf(length/4.0f); j++)				//Copy string
		buf[j] = pin[j];
	unsigned char* p=(unsigned char*)&buf[0];			//Byte ptr to buffer
	p[length]=0x80;										//Add padding
	for(int i=length+1; i%4 != 0; i++)
		p[i] = 0;
	
	//////////////////////////////////////////////////////////////////////////////
	//STEP 2: Append Length
	buf[14]=length * 8;

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
	
	unsigned int xb[4] = {a, b, c, d};
	for(int i=0; i<4; i++)
	{
		int base=i*4;
		out[base] = xb[i] & 0xff; xb[i] >>= 8;
		out[base+1] = xb[i] & 0xff; xb[i] >>= 8;
		out[base+2] = xb[i] & 0xff; xb[i] >>= 8;
		out[base+3] = xb[i] & 0xff; xb[i] >>= 8;
	}
}

//Do an MD5 hash, with the following contraints:
//	pin is writable
//	pin != pout
//	pin, pout are never cast to char* (compiler bug)
void __device__ DoMD5HashFast(unsigned int* buf, unsigned int* pout, int length)
{
	//////////////////////////////////////////////////////////////////////////////
	//STEP 1: Copy buffer and append padding bits
	//Generate the string
	for(int j=ceil(length/4.0f); j<16; j++)				//Zero the end
		buf[j] = 0;
	unsigned char* p=(unsigned char*)buf;				//Byte ptr to buffer
	p[length]=0x80;										//Add padding
	for(int i=length+1; i%4 != 0; i++)
		p[i] = 0;
	
	//////////////////////////////////////////////////////////////////////////////
	//STEP 2: Append Length
	unsigned int xlen = length * 8;

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
	fastmd5round_f(a,b,c,d,buf[0],7,0xd76aa478);
		fastmd5round_f(d,a,b,c,buf[1],12,0xe8c7b756);
		fastmd5round_f(c,d,a,b,buf[2],17,0x242070db);
		fastmd5round_f(b,c,d,a,buf[3],22,0xc1bdceee);
	fastmd5round_f(a,b,c,d,buf[4],7,0xf57c0faf);
		fastmd5round_f(d,a,b,c,buf[5],12,0x4787c62a);
		fastmd5round_f(c,d,a,b,buf[6],17,0xa8304613);
		fastmd5round_f(b,c,d,a,buf[7],22,0xfd469501);
	fastmd5round_f(a,b,c,d,buf[8],7,0x698098d8);
		fastmd5round_f(d,a,b,c,buf[9],12,0x8b44f7af);
		fastmd5round_f(c,d,a,b,buf[10],17,0xffff5bb1);
		fastmd5round_f(b,c,d,a,buf[11],22,0x895cd7be);
	fastmd5round_f(a,b,c,d,buf[12],7,0x6b901122);
		fastmd5round_f(d,a,b,c,buf[13],12,0xfd987193);
		fastmd5round_f(c,d,a,b,xlen,17,0xa679438e);
		fastmd5round_f(b,c,d,a,0,22,0x49b40821);

	fastmd5round_g(a,b,c,d,buf[1],5,0xf61e2562);
		fastmd5round_g(d,a,b,c,buf[6],9,0xc040b340);
		fastmd5round_g(c,d,a,b,buf[11],14,0x265e5a51);
		fastmd5round_g(b,c,d,a,buf[0],20,0xe9b6c7aa);
	fastmd5round_g(a,b,c,d,buf[5],5,0xd62f105d);
		fastmd5round_g(d,a,b,c,buf[10],9,0x02441453);
		fastmd5round_g(c,d,a,b,0,14,0xd8a1e681);
		fastmd5round_g(b,c,d,a,buf[4],20,0xe7d3fbc8);
	fastmd5round_g(a,b,c,d,buf[9],5,0x21e1cde6);
		fastmd5round_g(d,a,b,c,xlen,9,0xc33707d6);
		fastmd5round_g(c,d,a,b,buf[3],14,0xf4d50d87);
		fastmd5round_g(b,c,d,a,buf[8],20,0x455a14ed);
	fastmd5round_g(a,b,c,d,buf[13],5,0xa9e3e905);
		fastmd5round_g(d,a,b,c,buf[2],9,0xfcefa3f8);
		fastmd5round_g(c,d,a,b,buf[7],14,0x676f02d9);
		fastmd5round_g(b,c,d,a,buf[12],20,0x8d2a4c8a);

	fastmd5round_h(a,b,c,d,buf[5],4,0xfffa3942);
		fastmd5round_h(d,a,b,c,buf[8],11,0x8771f681);
		fastmd5round_h(c,d,a,b,buf[11],16,0x6d9d6122);
		fastmd5round_h(b,c,d,a,xlen,23,0xfde5380c);
	fastmd5round_h(a,b,c,d,buf[1],4,0xa4beea44);
		fastmd5round_h(d,a,b,c,buf[4],11,0x4bdecfa9);
		fastmd5round_h(c,d,a,b,buf[7],16,0xf6bb4b60);
		fastmd5round_h(b,c,d,a,buf[10],23,0xbebfbc70);
	fastmd5round_h(a,b,c,d,buf[13],4,0x289b7ec6);
		fastmd5round_h(d,a,b,c,buf[0],11,0xeaa127fa);
		fastmd5round_h(c,d,a,b,buf[3],16,0xd4ef3085);
		fastmd5round_h(b,c,d,a,buf[6],23,0x04881d05);
	fastmd5round_h(a,b,c,d,buf[9],4,0xd9d4d039);
		fastmd5round_h(d,a,b,c,buf[12],11,0xe6db99e5);
		fastmd5round_h(c,d,a,b,0,16,0x1fa27cf8);
		fastmd5round_h(b,c,d,a,buf[2],23,0xc4ac5665);

	fastmd5round_i(a,b,c,d,buf[0],6,0xf4292244);
		fastmd5round_i(d,a,b,c,buf[7],10,0x432aff97);
		fastmd5round_i(c,d,a,b,xlen,15,0xab9423a7);
		fastmd5round_i(b,c,d,a,buf[5],21,0xfc93a039);
	fastmd5round_i(a,b,c,d,buf[12],6,0x655b59c3);
		fastmd5round_i(d,a,b,c,buf[3],10,0x8f0ccc92);
		fastmd5round_i(c,d,a,b,buf[10],15,0xffeff47d);
		fastmd5round_i(b,c,d,a,buf[1],21,0x85845dd1);
	fastmd5round_i(a,b,c,d,buf[8],6,0x6fa87e4f);
		fastmd5round_i(d,a,b,c,0,10,0xfe2ce6e0);
		fastmd5round_i(c,d,a,b,buf[6],15,0xa3014314);
		fastmd5round_i(b,c,d,a,buf[13],21,0x4e0811a1);
	fastmd5round_i(a,b,c,d,buf[4],6,0xf7537e82);
		fastmd5round_i(d,a,b,c,buf[11],10,0xbd3af235);
		fastmd5round_i(c,d,a,b,buf[2],15,0x2ad7d2bb);
		fastmd5round_i(b,c,d,a,buf[9],21,0xeb86d391);
	
	//////////////////////////////////////////////////////////////////////////////
	//STEP 5: Output
	a += 0x67452301;
	b += 0xefcdab89;
	c += 0x98badcfe;
	d += 0x10325476;
	
	pout[0] = a;
	pout[1] = b;
	pout[2] = c;
	pout[3] = d;
}

int __device__ istrcat(unsigned int* dest, int destlen, unsigned int* src, int srclen)
{
	//Concatenate the strings (sliding window processing one source dword at a time)
	int base = floor(destlen/4.0f);
	int n = (destlen % 4) * 8;
	int fmn = 32 - n;
	unsigned int keepmask = 0xffffffff >> fmn;
	unsigned int rmask = 0xffffffff >> n;
	for(int i=0; i<=ceilf(srclen / 4.0f); i++)
	{
		int d = dest[base];
		int s = src[i];
		d &= keepmask;
		d |= ((s & rmask) << n);
		dest[base] = d;
		dest[base+1] = s >> fmn;
		base++;
	}
	
	//Return new length
	return destlen + srclen;
}

/*!
	@brief CUDA implementation of md5crypt	
	Thread-per-block requirement: minimum 64
	
	@param gtarget Target value (four ints, little endian)
	@param gstart Start index in charset (array of 32 ints, data is left aligned, unused values are at right)
	@param gsalt Salt
	@param status Set to true by a thread which succeeds in cracking the hash
	@param output Set to the collision by a thread which succeeds in cracking the hash
	@param base Length of the character set (passed in texCharset texture)
	@param len Length of valid data in gstart
	@param saltlen Length of salt
	@param hashcount Number of hashes being tested (not used)
 */
extern "C" __global__ void md5cryptKernel(int* gtarget, int* gstart, char* gsalt, char* status, char* output, int base, int len, int saltlen, int hashcount)
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
		
	//Cache salt
	__shared__ unsigned int isalt[3];
	if(saltlen > 12)			//md5crypt is defined as having salt up to 64 bits max
		return;
	unsigned char* salt = reinterpret_cast<unsigned char*>(isalt);
	if(threadIdx.x < saltlen)
		salt[threadIdx.x] = gsalt[threadIdx.x];
	
	//Cache target value
	__shared__ int target[4];
	if(threadIdx.x < 4)
		target[threadIdx.x] = gtarget[threadIdx.x];
	
	//Wait for all cache filling to finish
	__syncthreads();
	
	//Calculate the value we're hashing
	int pos[20];
	int guessend = len-1;
	for(int i=0; i<guessend; i++)
		pos[i] = start[i];
	pos[guessend] = start[guessend] + index;
	for(int i=guessend; i>=0; i--)
	{
		//No carry? Quit
		if(pos[i] < base)
			break;
		
		//Split carry from digit
		int digit = pos[i] % base;
		int carry = pos[i] - digit;
		
		//Save digit, push carry over
		pos[i] = digit;
		if(i >= 1)
			pos[i-1] += carry/base;
	}
	unsigned int iin[8];
	unsigned char* in = reinterpret_cast<unsigned char*>(iin);
	{
		for(int i=0; i<len; i++)
			in[i] = charset[pos[i]];
	}
	
	//|| is defined as concatenation
	//foo := MD5(password || salt || password);
	unsigned int ifoo[16];
	unsigned char* foo = reinterpret_cast<unsigned char*>(ifoo);
	int foolen=0;
	{
		//Copy password		
		int base = floor(len/4.0f);
		for(int i=0; i<=base; i++)
			ifoo[i] = iin[i];
		foolen = len;
		
		//Append the salt and password
		foolen = istrcat(ifoo, foolen, isalt, saltlen);
		foolen = istrcat(ifoo, foolen, iin, len);
		
		//Do the hash
		DoMD5Hash(ifoo,ifoo,foolen);
	}
	
	// extend(str, len) is str repeated to fill len bytes.
	//  bar := password || "$1$" || salt || extend(foo, length(password));
	unsigned int ibar[16];
	unsigned char* bar = reinterpret_cast<unsigned char*>(ibar);
	int barlen = 0;
	for(int i=0; i<ceilf(len); i++)	//copy in dword blocks, extra (if any) will be overwritten by the $1$ text
		ibar[i] = iin[i];
	barlen += len;
	bar[barlen++]='$';
	bar[barlen++]='1';
	bar[barlen++]='$';
	barlen = istrcat(ibar, barlen, isalt, saltlen);
	for(int i=0; i<len; i++)		//extend(foo, inlen)
		bar[barlen+i] = foo[i%foolen];
	barlen += len;
	
	//Pad with alternating nulls and initial characters
	int i = len;	
	while(i > 0)
	{
		if(i % 2)
			bar[barlen++] = '\0';	//append a null, not an ascii '0' or nothing at all!
		else
			bar[barlen++] = in[0];
		i >>= 1;
	}
	
	//TODO: Cache some strings, but don't use more than 32 registers.
	//Since we can run up to 4x 128 threads on a computer 1.3 device with 16K of shmem, we cannot
	//use more than 4K of shmem per block.
		
	//baz := MD5(bar);
	unsigned int ibaz[16];
	DoMD5Hash(ibar,ibaz,barlen);
	
	//ifnz(i, s) is defined as (i==0) ? "" : s
	unsigned int itbaz[16];
	itbaz[0] = ibaz[0];
	itbaz[1] = ibaz[1];
	itbaz[2] = ibaz[2];
	itbaz[3] = ibaz[3];
	i=0;
	int lo4 = ceilf(len/4.0f);
	while(i<1000)
	{
		//baz := MD5(baz || ifnz(i mod 3, salt) || ifnz(i mod 7, password) || password);
		int tbazlen = 16;
		if(i%3 != 0)
		{
			//Append salt, knowing that itbaz is aligned nicely so no shifting is needed
			itbaz[4] = isalt[0];
			itbaz[5] = isalt[1];
			itbaz[6] = isalt[2];
			tbazlen = 16 + saltlen;
			
			//Append password
			if(i%7 != 0)
				tbazlen = istrcat(itbaz, tbazlen, iin, len);
		}
		else if(i%7 != 0)
		{
			//Append password, knowing that itbaz is aligned nicely so no shifting is needed
			//istrcat(itbaz, tbazlen, iin, len);
			itbaz[4] = iin[0];
			itbaz[5] = iin[1];
			itbaz[6] = iin[2];
			itbaz[7] = iin[3];
			itbaz[8] = iin[4];
			itbaz[9] = iin[5];
			itbaz[10] = iin[6];
			tbazlen = 16 + len;
		}
		tbazlen = istrcat(itbaz, tbazlen, iin, len);
		DoMD5HashFast(itbaz,ibaz,tbazlen);
		
		i++;
		
		//baz := MD5(password || ifnz(i mod 3, salt) || ifnz(i mod 7, password) || baz);
		for(int j=0; j<lo4; j++)
			itbaz[j] = iin[j];
		tbazlen = len;
		if(i%3 != 0)
			tbazlen = istrcat(itbaz, tbazlen, isalt, saltlen);
		if(i%7 != 0)
			tbazlen = istrcat(itbaz, tbazlen, iin, len);
		tbazlen = istrcat(itbaz, tbazlen, ibaz, 16);
		DoMD5HashFast(itbaz,itbaz,tbazlen);
		
		i++;
	}
	
	//Test result
	if(itbaz[0] != target[0])
		return;
	if(itbaz[1] != target[1])
		return;
	if(itbaz[2] != target[2])
		return;
	if(itbaz[3] != target[3])
		return;
	
	//Match if we got to here. Save result.
	*status = 1;
	for(int j=0; j<len; j++)
		output[j] = in[j];
}
