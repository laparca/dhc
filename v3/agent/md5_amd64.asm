;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                             ;
; Distributed Hash Cracker v3.0                                               ;
;                                                                             ;
; Copyright (c) 2009 RPISEC.                                                  ;
; All rights reserved.                                                        ;
;                                                                             ;
; Redistribution and use in source and binary forms, with or without modifi-  ;
; cation, are permitted provided that the following conditions are met:       ;
;                                                                             ;
;    * Redistributions of source code must retain the above copyright notice  ;
;      this list of conditions and the following disclaimer.                  ;
;                                                                             ;
;    * Redistributions in binary form must reproduce the above copyright      ;
;      notice, this list of conditions and the following disclaimer in the    ;
;      documentation and/or other materials provided with the distribution.   ;
;                                                                             ;
;    * Neither the name of RPISEC nor the names of its contributors may be    ;
;      used to endorse or promote products derived from this software without ;
;      specific prior written permission.                                     ;
;                                                                             ;
; THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY EXPRESS OR IMPLIED      ;
; WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        ;
; MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN     ;
; NO EVENT SHALL RPISEC BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  ;
; SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    ;
; TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      ;
; PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      ;
; LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        ;
; NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          ;
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                ;
;                                                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;	@file md5_amd64.asm
;	@brief 64-bit SSE md5

BITS 64

SECTION .text 

;Use RIP-relative addressing
DEFAULT REL

;Constants (in code space, repeated 4x for SSE)
;Initialization vectors
align 16
md5init_a:	times 4 dd 0x67452301

align 16
md5init_b:	times 4 dd 0xefcdab89

align 16
md5init_c:	times 4 dd 0x98badcfe

align 16
md5init_d:	times 4 dd 0x10325476

;All ones 
align 16
md5_ones:	times 4 dd 0xFFFFFFFF

align 16
md5_tbuf:
			;Round 1 constants
			times 4 dd 0xd76aa478
			times 4 dd 0xe8c7b756
			times 4 dd 0x242070db
			times 4 dd 0xc1bdceee
			times 4 dd 0xf57c0faf
			times 4 dd 0x4787c62a
			times 4 dd 0xa8304613
			times 4 dd 0xfd469501
			times 4 dd 0x698098d8
			times 4 dd 0x8b44f7af
			times 4 dd 0xffff5bb1
			times 4 dd 0x895cd7be
			times 4 dd 0x6b901122
			times 4 dd 0xfd987193
			times 4 dd 0xa679438e ;times 4 dd 0xa679438e
			times 4 dd 0x49b40821
			
			;Round 2 constants
			times 4 dd 0xf61e2562
			times 4 dd 0xc040b340
			times 4 dd 0x265e5a51
			times 4 dd 0xe9b6c7aa
			times 4 dd 0xd62f105d
			times 4 dd 0x02441453
			times 4 dd 0xd8a1e681
			times 4 dd 0xe7d3fbc8
			times 4 dd 0x21e1cde6
			times 4 dd 0xc33707d6
			times 4 dd 0xf4d50d87
			times 4 dd 0x455a14ed
			times 4 dd 0xa9e3e905
			times 4 dd 0xfcefa3f8
			times 4 dd 0x676f02d9
			times 4 dd 0x8d2a4c8a
			
			;Round 3 constants
			times 4 dd 0xfffa3942
			times 4 dd 0x8771f681
			times 4 dd 0x6d9d6122
			times 4 dd 0xfde5380c
			times 4 dd 0xa4beea44
			times 4 dd 0x4bdecfa9
			times 4 dd 0xf6bb4b60
			times 4 dd 0xbebfbc70
			times 4 dd 0x289b7ec6
			times 4 dd 0xeaa127fa
			times 4 dd 0xd4ef3085
			times 4 dd 0x04881d05
			times 4 dd 0xd9d4d039
			times 4 dd 0xe6db99e5
			times 4 dd 0x1fa27cf8
			times 4 dd 0xc4ac5665
			
			;Round 4 constants
			times 4 dd 0xf4292244
			times 4 dd 0x432aff97
			times 4 dd 0xab9423a7
			times 4 dd 0xfc93a039
			times 4 dd 0x655b59c3
			times 4 dd 0x8f0ccc92
			times 4 dd 0xffeff47d
			times 4 dd 0x85845dd1
			times 4 dd 0x6fa87e4f
			times 4 dd 0xfe2ce6e0
			times 4 dd 0xa3014314
			times 4 dd 0x4e0811a1
			times 4 dd 0xf7537e82
			times 4 dd 0xbd3af235
			times 4 dd 0x2ad7d2bb
			times 4 dd 0xeb86d391
			
;//#define F(b,c,d) (((b) & (c)) | (~(b) & (d)))
%macro		sse_md5round_f 7
	movaps	xmm4,		%2					; xmm4 = b
	andps	xmm4,		%3					; xmm4 = b & c
	movaps	xmm5,		%2					; xmm5 = b
	andnps	xmm5,		%4					; xmm5 = ~b & d
	orps	xmm4,		xmm5				; xmm4 = F
 
	movaps	xmm5,		[rax + 16*%5]		; prefetch x[i] 
	paddd	%1,			xmm4				; a = a + F
	paddd	%1,			[md5_tbuf + 16*%7]	; a = a + F + T[i]*
	paddd	%1,			xmm5				; a = a + F + T[i] + x[i]*
 
	movaps	xmm4,		%1					; rotate a left shamt bits
	pslld	%1,			%6		        								
	psrld	xmm4,		(32-%6)									
	orps	%1,			xmm4										
 
	paddd	%1,			%2					; a = ROTL(~~~,N) + b) 
 
%endmacro

;//#define G(b,c,d) (((b) & (d)) | (~(d) & (c)))
%macro sse_md5round_g 7
	movaps	xmm4,		%2				    ; xmm4 = b
	andps	xmm4,		%4					; xmm4 = b & d
	movaps	xmm5,		%4					; xmm5 = d
	andnps	xmm5,		%3					; xmm5 = ~d & c
	orps	xmm4,		xmm5				; xmm4 = G
	movaps	xmm5,		[rax + 16*%5]		; prefetch x[i] 
	paddd	%1,			xmm4				; a = a + F
	paddd	%1,			[md5_tbuf + 16*%7]	; a = a + F + T[i]*
	paddd	%1,			xmm5				; a = a + F + T[i] + x[i]*
 
	movaps	xmm4,		%1					; rotate a left shamt bits
	pslld	%1,			%6		        								
	psrld	xmm4,		(32-%6)									
	orps	%1,			xmm4										
 
	paddd	%1,			%2					; a = ROTL(~~~,N) + b) 
%endmacro

;//#define H(b,c,d) ((b) ^ (c) ^ (d))
;(a,b,c,d,index,shamt,stepnum) __asm							
%macro sse_md5round_h 7
	movaps	xmm4,		%2					;  xmm4 = b
	xorps	xmm4,		%3					;	xmm4 = b ^ c
	xorps	xmm4,		%4					;	xmm4 = H
	movaps	xmm5,		[rax + 16*%5];		; prefetch x[i] 
	paddd	%1,			xmm4				; a = a + F
	paddd	%1,			[md5_tbuf + 16*%7]	; a = a + F + T[i]*
	paddd	%1,			xmm5				; a = a + F + T[i] + x[i]*
	 
	movaps	xmm4,		%1					; rotate a left shamt bits
	pslld	%1,			%6		        								
	psrld	xmm4,		(32-%6)									
	orps	%1,			xmm4										
	 
	paddd	%1,			%2					; a = ROTL(~~~,N) + b) 
%endmacro
 
;//#define I(b,c,d) ((c) ^ ((b) | ~(d)))
;(a,b,c,d,index,shamt,stepnum) 
%macro sse_md5round_i 7
	movaps	xmm4,		%4					;xmm4 = d
	xorps	xmm4,		[md5_ones]			;xmm4 = ~d
	orps	xmm4,		%2					;xmm4 = b | ~d
	xorps	xmm4,		%3					;xmm4 = I
	movaps	xmm5,		[rax + 16*%5]		; prefetch x[i] 
	paddd	%1,			xmm4				; a = a + F
	paddd	%1,			[md5_tbuf + 16*%7]	; a = a + F + T[i]*
	paddd	%1,			xmm5				; a = a + F + T[i] + x[i]*
	 
	movaps	xmm4,		%1					; rotate a left shamt bits
	pslld	%1,			%6		        								
	psrld	xmm4,		(32-%6)									
	orps	%1,			xmm4										
	 
	paddd	%1,			%2					; a = ROTL(~~~,N) + b) 
%endmacro

;void MD5Hash(char* in, char* out, unsigned long len)
;rdi			in
;rsi			out
;rdx			len

;[rbp-256]		xbuf
;rax			xbuf (from .length through beginning of .output)

;xmm0			A
;xmm1			B
;xmm2			C
;xmm3			D
global MD5Hash:function
MD5Hash:
	push	rbp								;Standard entry sequence
	mov		rbp, rsp			
	sub		rsp, 256						;Reserve 256 bytes of space for our X buffer
											;16 * sizeof dword = 64 bytes per hash
											;64 * 4 hashes = 256 bytes total
	push	rbx								;Save registers
											;TODO: save r12-r16 if we use them
		
.padding:							
	;Step 1: Append Padding Bits
	;NOTE: This overwrites one byte after the end of the actual input buffer.
	mov		byte [rdi + rdx], 80h
	mov		byte [rdi + rdx + 32], 80h
	mov		byte [rdi + rdx + 64], 80h
	mov		byte [rdi + rdx + 96], 80h
	
	;Step 0: Copy data, interleaving as needed
	xor		rax, rax						;Zero counter
	mov		rcx, rdx						;Get size
	inc		rcx								;Add one (padding byte)
	or		rcx, 3h							;Round up to a dword boundary (there are nulls after our data)
	lea		rbx, [rbp-256]					;Get pointer to xbuf
.dwordloop:
	mov		r8d, dword [rdi + rax]			;Read from input
	mov		r9d, dword [rdi + rax + 32]
	mov		r10d, dword [rdi + rax + 64]	
	mov		r11d, dword [rdi + rax + 96]	
	
	mov		dword [rbx], r8d				;Copy to buffer
	mov		dword [rbx+4], r9d
	mov		dword [rbx+8], r10d
	mov		dword [rbx+12], r11d

	add		rbx, 16							;Bump pointer
	
	add		rax, 4							;Bump counter
	cmp		rax, rcx						;Time to stop?
	jl		.dwordloop						;No, keep going
	
	;Zero the rest of the buffer
	xorps	xmm0, xmm0						;Four dwords of zeros
.zeroloop:
	movaps	[rbx], xmm0						;Store a block of zeros
	
	add		rbx, 16							;Bump pointer
	
	add		rax, 4							;Bump counter
	cmp		rax, 64							;Time to stop?
	jl		.zeroloop						;No, keep going
		
.length:	
	;Step 2: Append Length
	lea		rax, [rbp-256]					;rax = xbuf
	lea		rbx, [rdx * 8]					;rbx = len in BITS
	mov		dword [rax + 56*4], ebx			;Copy length (assumed to be < 2^32)
	mov		dword [rax + 57*4], ebx
	mov		dword [rax + 58*4], ebx
	mov		dword [rax+ 59*4], ebx
	
.constants:
	;Step 3: Initialize Constants
	movaps	xmm0, [md5init_a]
	movaps	xmm1, [md5init_b]
	movaps	xmm2, [md5init_c]
	movaps	xmm3, [md5init_d]
	
.rounds:
	;Step 4: MD5 rounds
	
	;Round 1
	sse_md5round_f xmm0,xmm1,xmm2,xmm3,0,7,0
	sse_md5round_f xmm3,xmm0,xmm1,xmm2,1,12,1 
	sse_md5round_f xmm2,xmm3,xmm0,xmm1,2,17,2 
	sse_md5round_f xmm1,xmm2,xmm3,xmm0,3,22,3 
	sse_md5round_f xmm0,xmm1,xmm2,xmm3,4,7,4 
	sse_md5round_f xmm3,xmm0,xmm1,xmm2,5,12,5 
	sse_md5round_f xmm2,xmm3,xmm0,xmm1,6,17,6 
	sse_md5round_f xmm1,xmm2,xmm3,xmm0,7,22,7 
	sse_md5round_f xmm0,xmm1,xmm2,xmm3,8,7,8 
	sse_md5round_f xmm3,xmm0,xmm1,xmm2,9,12,9 
	sse_md5round_f xmm2,xmm3,xmm0,xmm1,10,17,10 
	sse_md5round_f xmm1,xmm2,xmm3,xmm0,11,22,11 
	sse_md5round_f xmm0,xmm1,xmm2,xmm3,12,7,12 
	sse_md5round_f xmm3,xmm0,xmm1,xmm2,13,12,13 
	sse_md5round_f xmm2,xmm3,xmm0,xmm1,14,17,14 
	sse_md5round_f xmm1,xmm2,xmm3,xmm0,15,22,15 
	
	;Round 2
	sse_md5round_g xmm0,xmm1,xmm2,xmm3,1,5,16 
	sse_md5round_g xmm3,xmm0,xmm1,xmm2,6,9,17 
	sse_md5round_g xmm2,xmm3,xmm0,xmm1,11,14,18 
	sse_md5round_g xmm1,xmm2,xmm3,xmm0,0,20,19 
	sse_md5round_g xmm0,xmm1,xmm2,xmm3,5,5,20 
	sse_md5round_g xmm3,xmm0,xmm1,xmm2,10,9,21 
	sse_md5round_g xmm2,xmm3,xmm0,xmm1,15,14,22 
	sse_md5round_g xmm1,xmm2,xmm3,xmm0,4,20,23 
	sse_md5round_g xmm0,xmm1,xmm2,xmm3,9,5,24 
	sse_md5round_g xmm3,xmm0,xmm1,xmm2,14,9,25 
	sse_md5round_g xmm2,xmm3,xmm0,xmm1,3,14,26 
	sse_md5round_g xmm1,xmm2,xmm3,xmm0,8,20,27 
	sse_md5round_g xmm0,xmm1,xmm2,xmm3,13,5,28 
	sse_md5round_g xmm3,xmm0,xmm1,xmm2,2,9,29 
	sse_md5round_g xmm2,xmm3,xmm0,xmm1,7,14,30 
	sse_md5round_g xmm1,xmm2,xmm3,xmm0,12,20,31 

	; Round 3
	sse_md5round_h xmm0,xmm1,xmm2,xmm3,5,4,32 
	sse_md5round_h xmm3,xmm0,xmm1,xmm2,8,11,33 
	sse_md5round_h xmm2,xmm3,xmm0,xmm1,11,16,34 
	sse_md5round_h xmm1,xmm2,xmm3,xmm0,14,23,35 
	sse_md5round_h xmm0,xmm1,xmm2,xmm3,1,4,36 
	sse_md5round_h xmm3,xmm0,xmm1,xmm2,4,11,37 
	sse_md5round_h xmm2,xmm3,xmm0,xmm1,7,16,38 
	sse_md5round_h xmm1,xmm2,xmm3,xmm0,10,23,39 
	sse_md5round_h xmm0,xmm1,xmm2,xmm3,13,4,40 
	sse_md5round_h xmm3,xmm0,xmm1,xmm2,0,11,41 
	sse_md5round_h xmm2,xmm3,xmm0,xmm1,3,16,42 
	sse_md5round_h xmm1,xmm2,xmm3,xmm0,6,23,43 
	sse_md5round_h xmm0,xmm1,xmm2,xmm3,9,4,44 
	sse_md5round_h xmm3,xmm0,xmm1,xmm2,12,11,45 
	sse_md5round_h xmm2,xmm3,xmm0,xmm1,15,16,46 
	sse_md5round_h xmm1,xmm2,xmm3,xmm0,2,23,47 
	
	; Round 4
	sse_md5round_i xmm0,xmm1,xmm2,xmm3,0,6,48 
	sse_md5round_i xmm3,xmm0,xmm1,xmm2,7,10,49 
	sse_md5round_i xmm2,xmm3,xmm0,xmm1,14,15,50 
	sse_md5round_i xmm1,xmm2,xmm3,xmm0,5,21,51 
	sse_md5round_i xmm0,xmm1,xmm2,xmm3,12,6,52 
	sse_md5round_i xmm3,xmm0,xmm1,xmm2,3,10,53 
	sse_md5round_i xmm2,xmm3,xmm0,xmm1,10,15,54 
	sse_md5round_i xmm1,xmm2,xmm3,xmm0,1,21,55 
	sse_md5round_i xmm0,xmm1,xmm2,xmm3,8,6,56 
	sse_md5round_i xmm3,xmm0,xmm1,xmm2,15,10,57 
	sse_md5round_i xmm2,xmm3,xmm0,xmm1,6,15,58 
	sse_md5round_i xmm1,xmm2,xmm3,xmm0,13,21,59 
	sse_md5round_i xmm0,xmm1,xmm2,xmm3,4,6,60 
	sse_md5round_i xmm3,xmm0,xmm1,xmm2,11,10,61 
	sse_md5round_i xmm2,xmm3,xmm0,xmm1,2,15,62 
	sse_md5round_i xmm1,xmm2,xmm3,xmm0,9,21,63 
	
.output:
	;Step 5: Output
	
	;Add initial values back to registers
	paddd	xmm0, [md5init_a]
	paddd	xmm1, [md5init_b]
	paddd	xmm2, [md5init_c]
	paddd	xmm3, [md5init_d]
	
	;and move to output
	movups	[rsp-256], xmm0					;Copy block 1
	mov		eax, [rsp-256]		
	mov		[rsi], eax
	mov		eax, [rsp-252]		
	mov		[rsi + 16], eax
	mov		eax, [rsp-248]		
	mov		[rsi + 32], eax
	mov		eax, [rsp-244]		
	mov		[rsi + 48], eax
	
	movups	[rsp-256], xmm1					;Copy block 2
	mov		eax, [rsp-256]		
	mov		[rsi + 4], eax
	mov		eax, [rsp-252]		
	mov		[rsi + 4 + 16], eax
	mov		eax, [rsp-248]		
	mov		[rsi + 4 + 32], eax
	mov		eax, [rsp-244]		
	mov		[rsi + 4 + 48], eax
	
	movups	[rsp-256], xmm2					;Copy block 3
	mov		eax, [rsp-256]		
	mov		[rsi + 8], eax
	mov		eax, [rsp-252]		
	mov		[rsi + 8 + 16], eax
	mov		eax, [rsp-248]		
	mov		[rsi + 8 + 32], eax
	mov		eax, [rsp-244]		
	mov		[rsi + 8 + 48], eax
	
	movups	[rsp-256], xmm3					;Copy block 4
	mov		eax, [rsp-256]		
	mov		[rsi + 12], eax
	mov		eax, [rsp-252]		
	mov		[rsi + 12 + 16], eax
	mov		eax, [rsp-248]		
	mov		[rsi + 12 + 32], eax
	mov		eax, [rsp-244]		
	mov		[rsi + 12 + 48], eax
	
.hashdone:
	pop		rbx								;Restore registers
	mov		rsp, rbp						;Standard exit sequence
	pop		rbp
	
	ret										;and bail

