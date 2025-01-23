/*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (c) 2012, Intel Corporation
;
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are
; met:
;
; * Redistributions of source code must retain the above copyright
;   notice, this list of conditions and the following disclaimer.
;
; * Redistributions in binary form must reproduce the above copyright
;   notice, this list of conditions and the following disclaimer in the
;   documentation and/or other materials provided with the
;   distribution.
;
; * Neither the name of the Intel Corporation nor the names of its
;   contributors may be used to endorse or promote products derived from
;   this software without specific prior written permission.
;
;
; THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION "AS IS" AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
; PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL CORPORATION OR
; CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
; EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
; PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
; LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
; NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; This code is described in an Intel White-Paper:
; "Fast SHA-256 Implementations on Intel Architecture Processors"
;
; To find it, surf to http://www.intel.com/p/en_US/embedded
; and search for that title.
; The paper is expected to be released roughly at the end of April, 2012
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This code schedules 2 blocks at a time, with 4 lanes per block
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*/
/*
 * Conversion to GAS assembly and integration to libgcrypt
 *  by Jussi Kivilinna <jussi.kivilinna@iki.fi>
 */

#ifdef __x86_64
#include <config.h>
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && \
    defined(HAVE_INTEL_SYNTAX_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_AVX2) && defined(HAVE_GCC_INLINE_ASM_BMI2) && \
    defined(USE_SHA256)

#include "asm-common-amd64.h"

.intel_syntax noprefix

#define	VMOVDQ vmovdqu /* ; assume buffers not aligned  */

/* ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Define Macros */

/*  addm [mem], reg */
/*  Add reg to mem using reg-mem add and store */
#define addm(p1, p2) \
	add	p2, p1; \
	mov	p1, p2;

/* ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; */

#define X0 ymm4
#define X1 ymm5
#define X2 ymm6
#define X3 ymm7

/*  XMM versions of above */
#define XWORD0 xmm4
#define XWORD1 xmm5
#define XWORD2 xmm6
#define XWORD3 xmm7

#define XTMP0 ymm0
#define XTMP1 ymm1
#define XTMP2 ymm2
#define XTMP3 ymm3
#define XTMP4 ymm8
#define XFER ymm9
#define XTMP5 ymm11

#define SHUF_00BA ymm10 /*  shuffle xBxA -> 00BA */
#define SHUF_DC00 ymm12 /*  shuffle xDxC -> DC00 */
#define BYTE_FLIP_MASK ymm13

#define X_BYTE_FLIP_MASK xmm13 /*  XMM version of BYTE_FLIP_MASK */

#define NUM_BLKS rdx /*  3rd arg */
#define CTX rsi      /*  2nd arg */
#define INP rdi      /*  1st arg */
#define c ecx
#define d r8d
#define e edx        /*  clobbers NUM_BLKS */
#define y3 edi       /*  clobbers INP */

#define TBL rbp
#define SRND CTX     /*  SRND is same register as CTX */

#define a eax
#define b ebx
#define f r9d
#define g r10d
#define h r11d
#define old_h r11d

#define T1 r12d
#define y0 r13d
#define y1 r14d
#define y2 r15d


#define _XFER_SIZE 2*64*4	/*  2 blocks, 64 rounds, 4 bytes/round */
#define _XMM_SAVE_SIZE 0
#define _INP_END_SIZE 8
#define _INP_SIZE 8
#define _CTX_SIZE 8
#define _RSP_SIZE 8

#define _XFER 0
#define _XMM_SAVE  _XFER     + _XFER_SIZE
#define _INP_END   _XMM_SAVE + _XMM_SAVE_SIZE
#define _INP       _INP_END  + _INP_END_SIZE
#define _CTX       _INP      + _INP_SIZE
#define _RSP       _CTX      + _CTX_SIZE
#define STACK_SIZE _RSP      + _RSP_SIZE

#define ONE_ROUND_PART1(XFERIN, a, b, c, d, e, f, g, h) \
	/* h += Sum1 (e) + Ch (e, f, g) + (k[t] + w[0]); */ \
	/* d += h; */ \
	/* h += Sum0 (a) + Maj (a, b, c); */ \
	\
	/* Ch(x, y, z) => ((x & y) + (~x & z)) */ \
	/* Maj(x, y, z) => ((x & y) + (z & (x ^ y))) */ \
	\
	mov y3, e; \
	add h, [XFERIN]; \
	and y3, f; \
	rorx y0, e, 25; \
	rorx y1, e, 11; \
	lea h, [h + y3]; \
	andn y3, e, g; \
	rorx T1, a, 13; \
	xor y0, y1; \
	lea h, [h + y3]

#define ONE_ROUND_PART2(a, b, c, d, e, f, g, h) \
	rorx y2, a, 22; \
	rorx y1, e, 6; \
	mov y3, a; \
	xor T1, y2; \
	xor y0, y1; \
	xor y3, b; \
	lea h, [h + y0]; \
	mov y0, a; \
	rorx y2, a, 2; \
	add d, h; \
	and y3, c; \
	xor T1, y2; \
	lea h, [h + y3]; \
	lea h, [h + T1]; \
	and y0, b; \
	lea h, [h + y0]

#define ONE_ROUND(XFER, a, b, c, d, e, f, g, h) \
	ONE_ROUND_PART1(XFER, a, b, c, d, e, f, g, h); \
	ONE_ROUND_PART2(a, b, c, d, e, f, g, h)

#define FOUR_ROUNDS_AND_SCHED(XFERIN, XFEROUT, X0, X1, X2, X3, a, b, c, d, e, f, g, h) \
	/* ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; RND N + 0 ;;;;;;;;;;;;;;;;;;;;;;;;;;;; */; \
		vpalignr	XTMP0, X3, X2, 4	/*  XTMP0 = W[-7] */; \
		vpaddd	XTMP0, XTMP0, X0	/*  XTMP0 = W[-7] + W[-16]; y1 = (e >> 6); S1 */; \
		vpalignr	XTMP1, X1, X0, 4	/*  XTMP1 = W[-15] */; \
		vpsrld	XTMP2, XTMP1, 7; \
		vpslld	XTMP3, XTMP1, (32-7); \
		vpor	XTMP3, XTMP3, XTMP2	/*  XTMP3 = W[-15] ror 7 */; \
		vpsrld	XTMP2, XTMP1,18; \
	\
	ONE_ROUND(0*4+XFERIN, a, b, c, d, e, f, g, h); \
	\
	/* ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; RND N + 1 ;;;;;;;;;;;;;;;;;;;;;;;;;;;; */; \
		vpsrld	XTMP4, XTMP1, 3	/*  XTMP4 = W[-15] >> 3 */; \
		vpslld	XTMP1, XTMP1, (32-18); \
		vpxor	XTMP3, XTMP3, XTMP1; \
		vpxor	XTMP3, XTMP3, XTMP2	/*  XTMP3 = W[-15] ror 7 ^ W[-15] ror 18 */; \
		vpxor	XTMP1, XTMP3, XTMP4	/*  XTMP1 = s0 */; \
		vpshufd	XTMP2, X3, 0b11111010	/*  XTMP2 = W[-2] {BBAA} */; \
		vpaddd	XTMP0, XTMP0, XTMP1	/*  XTMP0 = W[-16] + W[-7] + s0 */; \
		vpsrld	XTMP4, XTMP2, 10	/*  XTMP4 = W[-2] >> 10 {BBAA} */; \
	\
	ONE_ROUND(1*4+XFERIN, h, a, b, c, d, e, f, g); \
	\
	/* ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; RND N + 2 ;;;;;;;;;;;;;;;;;;;;;;;;;;;; */; \
		vpsrlq	XTMP3, XTMP2, 19	/*  XTMP3 = W[-2] ror 19 {xBxA} */; \
		vpsrlq	XTMP2, XTMP2, 17	/*  XTMP2 = W[-2] ror 17 {xBxA} */; \
		vpxor	XTMP2, XTMP2, XTMP3; \
		vpxor	XTMP4, XTMP4, XTMP2	/*  XTMP4 = s1 {xBxA} */; \
		vpshufb	XTMP4, XTMP4, SHUF_00BA	/*  XTMP4 = s1 {00BA} */; \
		vpaddd	XTMP0, XTMP0, XTMP4	/*  XTMP0 = {..., ..., W[1], W[0]} */; \
		vpshufd	XTMP2, XTMP0, 0b1010000	/*  XTMP2 = W[-2] {DDCC} */; \
	\
	ONE_ROUND(2*4+XFERIN, g, h, a, b, c, d, e, f); \
	\
	/* ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; RND N + 3 ;;;;;;;;;;;;;;;;;;;;;;;;;;;; */; \
		vpsrld	XTMP5, XTMP2,   10	/*  XTMP5 = W[-2] >> 10 {DDCC} */; \
		vpsrlq	XTMP3, XTMP2, 19	/*  XTMP3 = W[-2] ror 19 {xDxC} */; \
		vpsrlq	XTMP2, XTMP2, 17	/*  XTMP2 = W[-2] ror 17 {xDxC} */; \
		vpxor	XTMP2, XTMP2, XTMP3; \
		vpxor	XTMP5, XTMP5, XTMP2	/*  XTMP5 = s1 {xDxC} */; \
		vpshufb	XTMP5, XTMP5, SHUF_DC00	/*  XTMP5 = s1 {DC00} */; \
		vpaddd	X0, XTMP5, XTMP0	/*  X0 = {W[3], W[2], W[1], W[0]} */; \
		vpaddd	XFER, X0, [TBL + XFEROUT]; \
	\
	ONE_ROUND_PART1(3*4+XFERIN, f, g, h, a, b, c, d, e); \
		vmovdqa [rsp + _XFER + XFEROUT], XFER; \
	ONE_ROUND_PART2(f, g, h, a, b, c, d, e);

#define DO_4ROUNDS(XFERIN, a, b, c, d, e, f, g, h) \
	ONE_ROUND(0*4+XFERIN, a, b, c, d, e, f, g, h); \
	ONE_ROUND(1*4+XFERIN, h, a, b, c, d, e, f, g); \
	ONE_ROUND(2*4+XFERIN, g, h, a, b, c, d, e, f); \
	ONE_ROUND(3*4+XFERIN, f, g, h, a, b, c, d, e)

/*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; void sha256_rorx(void *input_data, UINT32 digest[8], UINT64 num_blks)
;; arg 1 : pointer to input data
;; arg 2 : pointer to digest
;; arg 3 : Num blocks
*/
.text
.globl _gcry_sha256_transform_amd64_avx2
ELF(.type _gcry_sha256_transform_amd64_avx2,@function)
.align 32
_gcry_sha256_transform_amd64_avx2:
	CFI_STARTPROC()
	xor eax, eax

	cmp rdx, 0
	je .Lnowork

	push	rbx
	CFI_PUSH(rbx)
	push	rbp
	CFI_PUSH(rbp)
	push	r12
	CFI_PUSH(r12)
	push	r13
	CFI_PUSH(r13)
	push	r14
	CFI_PUSH(r14)
	push	r15
	CFI_PUSH(r15)

	vzeroupper

	vmovdqa	BYTE_FLIP_MASK, [.LPSHUFFLE_BYTE_FLIP_MASK ADD_RIP]
	vmovdqa	SHUF_00BA, [.L_SHUF_00BA ADD_RIP]
	vmovdqa	SHUF_DC00, [.L_SHUF_DC00 ADD_RIP]

	mov	rax, rsp
	CFI_DEF_CFA_REGISTER(rax);
	sub	rsp, STACK_SIZE
	and	rsp, ~63
	mov	[rsp + _RSP], rax
	CFI_CFA_ON_STACK(_RSP, 6 * 8)

	shl	NUM_BLKS, 6	/*  convert to bytes */
	lea	NUM_BLKS, [NUM_BLKS + INP - 64] /*  pointer to last block */
	mov	[rsp + _INP_END], NUM_BLKS

	/* Check if only one block of input. Note: Loading initial digest
	 * only uses 'mov' instruction and does not change condition
	 * flags. */
	cmp	NUM_BLKS, INP

	/* ; load initial digest */
	mov	a,[4*0 + CTX]
	mov	b,[4*1 + CTX]
	mov	c,[4*2 + CTX]
	mov	d,[4*3 + CTX]
	mov	e,[4*4 + CTX]
	mov	f,[4*5 + CTX]
	mov	g,[4*6 + CTX]
	mov	h,[4*7 + CTX]

	mov	[rsp + _CTX], CTX

	je	.Ldo_last_block

.Loop0:
	lea	TBL, [.LK256 ADD_RIP]

	/* ; Load first 16 dwords from two blocks */
	VMOVDQ	XTMP0, [INP + 0*32]
	VMOVDQ	XTMP1, [INP + 1*32]
	VMOVDQ	XTMP2, [INP + 2*32]
	VMOVDQ	XTMP3, [INP + 3*32]

	/* ; byte swap data */
	vpshufb	XTMP0, XTMP0, BYTE_FLIP_MASK
	vpshufb	XTMP1, XTMP1, BYTE_FLIP_MASK
	vpshufb	XTMP2, XTMP2, BYTE_FLIP_MASK
	vpshufb	XTMP3, XTMP3, BYTE_FLIP_MASK

	/* ; transpose data into high/low halves */
	vperm2i128	X0, XTMP0, XTMP2, 0x20
	vperm2i128	X1, XTMP0, XTMP2, 0x31
	vperm2i128	X2, XTMP1, XTMP3, 0x20
	vperm2i128	X3, XTMP1, XTMP3, 0x31

.Last_block_enter:
	add	INP, 64
	mov	[rsp + _INP], INP

	/* ; schedule 48 input dwords, by doing 3 rounds of 12 each */
	xor	SRND, SRND

	vpaddd	XFER, X0, [TBL + 0*32]
	vmovdqa [rsp + _XFER + 0*32], XFER
	vpaddd	XFER, X1, [TBL + 1*32]
	vmovdqa [rsp + _XFER + 1*32], XFER
	vpaddd	XFER, X2, [TBL + 2*32]
	vmovdqa [rsp + _XFER + 2*32], XFER
	vpaddd	XFER, X3, [TBL + 3*32]
	vmovdqa [rsp + _XFER + 3*32], XFER

.align 16
.Loop1:
	FOUR_ROUNDS_AND_SCHED(rsp + _XFER + SRND + 0*32, SRND + 4*32, X0, X1, X2, X3, a, b, c, d, e, f, g, h)
	FOUR_ROUNDS_AND_SCHED(rsp + _XFER + SRND + 1*32, SRND + 5*32, X1, X2, X3, X0, e, f, g, h, a, b, c, d)
	FOUR_ROUNDS_AND_SCHED(rsp + _XFER + SRND + 2*32, SRND + 6*32, X2, X3, X0, X1, a, b, c, d, e, f, g, h)
	FOUR_ROUNDS_AND_SCHED(rsp + _XFER + SRND + 3*32, SRND + 7*32, X3, X0, X1, X2, e, f, g, h, a, b, c, d)

	add	SRND, 4*32
	cmp	SRND, 3 * 4*32
	jb	.Loop1

	/* ; Do last 16 rounds with no scheduling */
	DO_4ROUNDS(rsp + _XFER + (3*4*32 + 0*32), a, b, c, d, e, f, g, h)
	DO_4ROUNDS(rsp + _XFER + (3*4*32 + 1*32), e, f, g, h, a, b, c, d)
	DO_4ROUNDS(rsp + _XFER + (3*4*32 + 2*32), a, b, c, d, e, f, g, h)
	DO_4ROUNDS(rsp + _XFER + (3*4*32 + 3*32), e, f, g, h, a, b, c, d)

	mov	CTX, [rsp + _CTX]
	mov	INP, [rsp + _INP]

	addm([4*0 + CTX],a)
	addm([4*1 + CTX],b)
	addm([4*2 + CTX],c)
	addm([4*3 + CTX],d)
	addm([4*4 + CTX],e)
	addm([4*5 + CTX],f)
	addm([4*6 + CTX],g)
	addm([4*7 + CTX],h)

	cmp	INP, [rsp + _INP_END]
	ja	.Ldone_hash

	/* ;;; Do second block using previously scheduled results */
	xor	SRND, SRND
.align 16
.Loop3:
	DO_4ROUNDS(rsp + _XFER + SRND + 0*32 + 16, a, b, c, d, e, f, g, h)
	DO_4ROUNDS(rsp + _XFER + SRND + 1*32 + 16, e, f, g, h, a, b, c, d)
	add	SRND, 2*32
	cmp	SRND, 4 * 4*32
	jb .Loop3

	mov	CTX, [rsp + _CTX]
	mov	INP, [rsp + _INP]
	add	INP, 64

	addm([4*0 + CTX],a)
	addm([4*1 + CTX],b)
	addm([4*2 + CTX],c)
	addm([4*3 + CTX],d)
	addm([4*4 + CTX],e)
	addm([4*5 + CTX],f)
	addm([4*6 + CTX],g)
	addm([4*7 + CTX],h)

	cmp	INP, [rsp + _INP_END]
	jb	.Loop0
	ja	.Ldone_hash

.Ldo_last_block:
	/* ;;; do last block */
	lea	TBL, [.LK256 ADD_RIP]

	VMOVDQ	XWORD0, [INP + 0*16]
	VMOVDQ	XWORD1, [INP + 1*16]
	VMOVDQ	XWORD2, [INP + 2*16]
	VMOVDQ	XWORD3, [INP + 3*16]

	vpshufb	XWORD0, XWORD0, X_BYTE_FLIP_MASK
	vpshufb	XWORD1, XWORD1, X_BYTE_FLIP_MASK
	vpshufb	XWORD2, XWORD2, X_BYTE_FLIP_MASK
	vpshufb	XWORD3, XWORD3, X_BYTE_FLIP_MASK

	jmp	.Last_block_enter

.Lonly_one_block:

	/* ; load initial digest */
	mov	a,[4*0 + CTX]
	mov	b,[4*1 + CTX]
	mov	c,[4*2 + CTX]
	mov	d,[4*3 + CTX]
	mov	e,[4*4 + CTX]
	mov	f,[4*5 + CTX]
	mov	g,[4*6 + CTX]
	mov	h,[4*7 + CTX]

	vmovdqa	BYTE_FLIP_MASK, [.LPSHUFFLE_BYTE_FLIP_MASK ADD_RIP]
	vmovdqa	SHUF_00BA, [.L_SHUF_00BA ADD_RIP]
	vmovdqa	SHUF_DC00, [.L_SHUF_DC00 ADD_RIP]

	mov	[rsp + _CTX], CTX
	jmp	.Ldo_last_block

.Ldone_hash:
	vzeroall

	/* burn stack */
	vmovdqa [rsp + _XFER + 0 * 32], ymm0
	vmovdqa [rsp + _XFER + 1 * 32], ymm0
	vmovdqa [rsp + _XFER + 2 * 32], ymm0
	vmovdqa [rsp + _XFER + 3 * 32], ymm0
	vmovdqa [rsp + _XFER + 4 * 32], ymm0
	vmovdqa [rsp + _XFER + 5 * 32], ymm0
	vmovdqa [rsp + _XFER + 6 * 32], ymm0
	vmovdqa [rsp + _XFER + 7 * 32], ymm0
	vmovdqa [rsp + _XFER + 8 * 32], ymm0
	vmovdqa [rsp + _XFER + 9 * 32], ymm0
	vmovdqa [rsp + _XFER + 10 * 32], ymm0
	vmovdqa [rsp + _XFER + 11 * 32], ymm0
	vmovdqa [rsp + _XFER + 12 * 32], ymm0
	vmovdqa [rsp + _XFER + 13 * 32], ymm0
	vmovdqa [rsp + _XFER + 14 * 32], ymm0
	vmovdqa [rsp + _XFER + 15 * 32], ymm0
	xor     eax, eax

	mov	rsp, [rsp + _RSP]
	CFI_DEF_CFA_REGISTER(rsp)

	pop	r15
	CFI_POP(r15)
	pop	r14
	CFI_POP(r14)
	pop	r13
	CFI_POP(r13)
	pop	r12
	CFI_POP(r12)
	pop	rbp
	CFI_POP(rbp)
	pop	rbx
	CFI_POP(rbx)

.Lnowork:
	ret_spec_stop
	CFI_ENDPROC()

.align 64
.LK256:
	.long	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5
	.long	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5
	.long	0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5
	.long	0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5
	.long	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3
	.long	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3
	.long	0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174
	.long	0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174
	.long	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc
	.long	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc
	.long	0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da
	.long	0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da
	.long	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7
	.long	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7
	.long	0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967
	.long	0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967
	.long	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13
	.long	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13
	.long	0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85
	.long	0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85
	.long	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3
	.long	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3
	.long	0xd192e819,0xd6990624,0xf40e3585,0x106aa070
	.long	0xd192e819,0xd6990624,0xf40e3585,0x106aa070
	.long	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5
	.long	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5
	.long	0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3
	.long	0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3
	.long	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208
	.long	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208
	.long	0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
	.long	0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2

.LPSHUFFLE_BYTE_FLIP_MASK:
	.octa 0x0c0d0e0f08090a0b0405060700010203,0x0c0d0e0f08090a0b0405060700010203

/*  shuffle xBxA -> 00BA */
.L_SHUF_00BA:
	.octa 0xFFFFFFFFFFFFFFFF0b0a090803020100,0xFFFFFFFFFFFFFFFF0b0a090803020100

/*  shuffle xDxC -> DC00 */
.L_SHUF_DC00:
	.octa 0x0b0a090803020100FFFFFFFFFFFFFFFF,0x0b0a090803020100FFFFFFFFFFFFFFFF

#endif
#endif
