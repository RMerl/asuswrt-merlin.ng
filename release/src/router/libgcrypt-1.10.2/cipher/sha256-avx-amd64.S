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
; This code is described in an Intel White-Paper:
; "Fast SHA-256 Implementations on Intel Architecture Processors"
;
; To find it, surf to http://www.intel.com/p/en_US/embedded
; and search for that title.
; The paper is expected to be released roughly at the end of April, 2012
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This code schedules 1 blocks at a time, with 4 lanes per block
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*/
/*
 * Conversion to GAS assembly and integration to libgcrypt
 *  by Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * Note: Based on the SSSE3 implementation.
 */

#ifdef __x86_64
#include <config.h>
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && \
    defined(HAVE_INTEL_SYNTAX_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_AVX) && defined(USE_SHA256)

#include "asm-common-amd64.h"

.intel_syntax noprefix

#define	VMOVDQ vmovdqu /* assume buffers not aligned */

#define ROR(p1, p2) \
	/* shld is faster than ror on Intel Sandybridge */ \
	shld	p1, p1, (32 - p2);

/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Define Macros*/

/* addm [mem], reg
 * Add reg to mem using reg-mem add and store */
#define addm(p1, p2) \
	add	p2, p1; \
	mov	p1, p2;

/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

/* COPY_XMM_AND_BSWAP xmm, [mem], byte_flip_mask
 * Load xmm with mem and byte swap each dword */
#define COPY_XMM_AND_BSWAP(p1, p2, p3) \
	VMOVDQ p1, p2; \
	vpshufb p1, p1, p3;

/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

#define X0 xmm4
#define X1 xmm5
#define X2 xmm6
#define X3 xmm7

#define XTMP0 xmm0
#define XTMP1 xmm1
#define XTMP2 xmm2
#define XTMP3 xmm3
#define XTMP4 xmm8
#define XFER xmm9

#define SHUF_00BA xmm10 /* shuffle xBxA -> 00BA */
#define SHUF_DC00 xmm11 /* shuffle xDxC -> DC00 */
#define BYTE_FLIP_MASK xmm12

#define NUM_BLKS rdx	/* 3rd arg */
#define CTX rsi	/* 2nd arg */
#define INP rdi	/* 1st arg */

#define SRND rdi	/* clobbers INP */
#define c ecx
#define d r8d
#define e edx

#define TBL rbp
#define a eax
#define b ebx

#define f r9d
#define g r10d
#define h r11d

#define y0 r13d
#define y1 r14d
#define y2 r15d



#define _INP_END_SIZE	8
#define _INP_SIZE	8
#define _XFER_SIZE	8
#define _XMM_SAVE_SIZE	0
/* STACK_SIZE plus pushes must be an odd multiple of 8 */
#define _ALIGN_SIZE	8

#define _INP_END	0
#define _INP		(_INP_END  + _INP_END_SIZE)
#define _XFER		(_INP      + _INP_SIZE)
#define _XMM_SAVE	(_XFER     + _XFER_SIZE + _ALIGN_SIZE)
#define STACK_SIZE	(_XMM_SAVE + _XMM_SAVE_SIZE)


#define FOUR_ROUNDS_AND_SCHED_0(X0, X1, X2, X3, a, b, c, d, e, f, g, h) \
		/* compute s0 four at a time and s1 two at a time */; \
		/* compute W[-16] + W[-7] 4 at a time */; \
	mov	y0, e		/* y0 = e */; \
	ROR(	y0, (25-11))	/* y0 = e >> (25-11) */; \
	mov	y1, a		/* y1 = a */; \
		vpalignr	XTMP0, X3, X2, 4	/* XTMP0 = W[-7] */; \
	ROR(	y1, (22-13))	/* y1 = a >> (22-13) */; \
	xor	y0, e		/* y0 = e ^ (e >> (25-11)) */; \
	mov	y2, f		/* y2 = f */; \
	ROR(	y0, (11-6))	/* y0 = (e >> (11-6)) ^ (e >> (25-6)) */; \
	xor	y1, a		/* y1 = a ^ (a >> (22-13) */; \
	xor	y2, g		/* y2 = f^g */; \
		vpaddd	XTMP0, XTMP0, X0	/* XTMP0 = W[-7] + W[-16] */; \
	xor	y0, e		/* y0 = e ^ (e >> (11-6)) ^ (e >> (25-6)) */; \
	and	y2, e		/* y2 = (f^g)&e */; \
	ROR(	y1, (13-2))	/* y1 = (a >> (13-2)) ^ (a >> (22-2)) */; \
		/* compute s0 */; \
		vpalignr	XTMP1, X1, X0, 4	/* XTMP1 = W[-15] */; \
	xor	y1, a		/* y1 = a ^ (a >> (13-2)) ^ (a >> (22-2)) */; \
	ROR(	y0, 6)		/* y0 = S1 = (e>>6) & (e>>11) ^ (e>>25) */; \
	xor	y2, g		/* y2 = CH = ((f^g)&e)^g */; \
	ROR(	y1, 2)		/* y1 = S0 = (a>>2) ^ (a>>13) ^ (a>>22) */; \
	add	y2, y0		/* y2 = S1 + CH */; \
	add	y2, [rsp + _XFER + 0*4]	/* y2 = k + w + S1 + CH */; \
	mov	y0, a		/* y0 = a */; \
	add	h, y2		/* h = h + S1 + CH + k + w */; \
	mov	y2, a		/* y2 = a */; \
		vpslld	XTMP2, XTMP1, (32-7); \
	or	y0, c		/* y0 = a|c */; \
	add	d, h		/* d = d + h + S1 + CH + k + w */; \
	and	y2, c		/* y2 = a&c */; \
		vpsrld	XTMP3, XTMP1, 7; \
	and	y0, b		/* y0 = (a|c)&b */; \
	add	h, y1		/* h = h + S1 + CH + k + w + S0 */; \
		vpor	XTMP3, XTMP3, XTMP2	/* XTMP1 = W[-15] ror 7 */; \
	or	y0, y2		/* y0 = MAJ = (a|c)&b)|(a&c) */; \
	lea	h, [h + y0]	/* h = h + S1 + CH + k + w + S0 + MAJ */

#define FOUR_ROUNDS_AND_SCHED_1(X0, X1, X2, X3, a, b, c, d, e, f, g, h) \
	mov	y0, e		/* y0 = e */; \
	mov	y1, a		/* y1 = a */; \
	ROR(	y0, (25-11))	/* y0 = e >> (25-11) */; \
	xor	y0, e		/* y0 = e ^ (e >> (25-11)) */; \
	mov	y2, f		/* y2 = f */; \
	ROR(	y1, (22-13))	/* y1 = a >> (22-13) */; \
		vpslld	XTMP2, XTMP1, (32-18); \
	xor	y1, a		/* y1 = a ^ (a >> (22-13) */; \
	ROR(	y0, (11-6))	/* y0 = (e >> (11-6)) ^ (e >> (25-6)) */; \
	xor	y2, g		/* y2 = f^g */; \
		vpsrld	XTMP4, XTMP1, 18; \
	ROR(	y1, (13-2))	/* y1 = (a >> (13-2)) ^ (a >> (22-2)) */; \
	xor	y0, e		/* y0 = e ^ (e >> (11-6)) ^ (e >> (25-6)) */; \
	and	y2, e		/* y2 = (f^g)&e */; \
	ROR(	y0, 6)		/* y0 = S1 = (e>>6) & (e>>11) ^ (e>>25) */; \
		vpxor	XTMP4, XTMP4, XTMP3; \
	xor	y1, a		/* y1 = a ^ (a >> (13-2)) ^ (a >> (22-2)) */; \
	xor	y2, g		/* y2 = CH = ((f^g)&e)^g */; \
		vpsrld	XTMP1, XTMP1, 3	/* XTMP4 = W[-15] >> 3 */; \
	add	y2, y0		/* y2 = S1 + CH */; \
	add	y2, [rsp + _XFER + 1*4]	/* y2 = k + w + S1 + CH */; \
	ROR(	y1, 2)		/* y1 = S0 = (a>>2) ^ (a>>13) ^ (a>>22) */; \
		vpxor	XTMP1, XTMP1, XTMP2	/* XTMP1 = W[-15] ror 7 ^ W[-15] ror 18 */; \
	mov	y0, a		/* y0 = a */; \
	add	h, y2		/* h = h + S1 + CH + k + w */; \
	mov	y2, a		/* y2 = a */; \
		vpxor	XTMP1, XTMP1, XTMP4	/* XTMP1 = s0 */; \
	or	y0, c		/* y0 = a|c */; \
	add	d, h		/* d = d + h + S1 + CH + k + w */; \
	and	y2, c		/* y2 = a&c */; \
		/* compute low s1 */; \
		vpshufd	XTMP2, X3, 0b11111010	/* XTMP2 = W[-2] {BBAA} */; \
	and	y0, b		/* y0 = (a|c)&b */; \
	add	h, y1		/* h = h + S1 + CH + k + w + S0 */; \
		vpaddd	XTMP0, XTMP0, XTMP1	/* XTMP0 = W[-16] + W[-7] + s0 */; \
	or	y0, y2		/* y0 = MAJ = (a|c)&b)|(a&c) */; \
	lea	h, [h + y0]	/* h = h + S1 + CH + k + w + S0 + MAJ */

#define FOUR_ROUNDS_AND_SCHED_2(X0, X1, X2, X3, a, b, c, d, e, f, g, h) \
	mov	y0, e		/* y0 = e */; \
	mov	y1, a		/* y1 = a */; \
	ROR(	y0, (25-11))	/* y0 = e >> (25-11) */; \
	xor	y0, e		/* y0 = e ^ (e >> (25-11)) */; \
	ROR(	y1, (22-13))	/* y1 = a >> (22-13) */; \
	mov	y2, f		/* y2 = f */; \
	xor	y1, a		/* y1 = a ^ (a >> (22-13) */; \
	ROR(	y0, (11-6))	/* y0 = (e >> (11-6)) ^ (e >> (25-6)) */; \
		vpsrlq	XTMP3, XTMP2, 17	/* XTMP2 = W[-2] ror 17 {xBxA} */; \
	xor	y2, g		/* y2 = f^g */; \
		vpsrlq	XTMP4, XTMP2, 19	/* XTMP3 = W[-2] ror 19 {xBxA} */; \
	xor	y0, e		/* y0 = e ^ (e >> (11-6)) ^ (e >> (25-6)) */; \
	and	y2, e		/* y2 = (f^g)&e */; \
		vpsrld	XTMP2, XTMP2, 10	/* XTMP4 = W[-2] >> 10 {BBAA} */; \
	ROR(	y1, (13-2))	/* y1 = (a >> (13-2)) ^ (a >> (22-2)) */; \
	xor	y1, a		/* y1 = a ^ (a >> (13-2)) ^ (a >> (22-2)) */; \
	xor	y2, g		/* y2 = CH = ((f^g)&e)^g */; \
	ROR(	y0, 6)		/* y0 = S1 = (e>>6) & (e>>11) ^ (e>>25) */; \
		vpxor	XTMP2, XTMP2, XTMP3; \
	add	y2, y0		/* y2 = S1 + CH */; \
	ROR(	y1, 2)		/* y1 = S0 = (a>>2) ^ (a>>13) ^ (a>>22) */; \
	add	y2, [rsp + _XFER + 2*4]	/* y2 = k + w + S1 + CH */; \
		vpxor	XTMP4, XTMP4, XTMP2	/* XTMP4 = s1 {xBxA} */; \
	mov	y0, a		/* y0 = a */; \
	add	h, y2		/* h = h + S1 + CH + k + w */; \
	mov	y2, a		/* y2 = a */; \
		vpshufb	XTMP4, XTMP4, SHUF_00BA	/* XTMP4 = s1 {00BA} */; \
	or	y0, c		/* y0 = a|c */; \
	add	d, h		/* d = d + h + S1 + CH + k + w */; \
	and	y2, c		/* y2 = a&c */; \
		vpaddd	XTMP0, XTMP0, XTMP4	/* XTMP0 = {..., ..., W[1], W[0]} */; \
	and	y0, b		/* y0 = (a|c)&b */; \
	add	h, y1		/* h = h + S1 + CH + k + w + S0 */; \
		/* compute high s1 */; \
		vpshufd	XTMP2, XTMP0, 0b01010000 /* XTMP2 = W[-2] {DDCC} */; \
	or	y0, y2		/* y0 = MAJ = (a|c)&b)|(a&c) */; \
	lea	h, [h + y0]	/* h = h + S1 + CH + k + w + S0 + MAJ */

#define FOUR_ROUNDS_AND_SCHED_3(X0, X1, X2, X3, a, b, c, d, e, f, g, h) \
	mov	y0, e		/* y0 = e */; \
	ROR(	y0, (25-11))	/* y0 = e >> (25-11) */; \
	mov	y1, a		/* y1 = a */; \
	ROR(	y1, (22-13))	/* y1 = a >> (22-13) */; \
	xor	y0, e		/* y0 = e ^ (e >> (25-11)) */; \
	mov	y2, f		/* y2 = f */; \
	ROR(	y0, (11-6))	/* y0 = (e >> (11-6)) ^ (e >> (25-6)) */; \
		vpsrlq	XTMP3, XTMP2, 17	/* XTMP2 = W[-2] ror 17 {xDxC} */; \
	xor	y1, a		/* y1 = a ^ (a >> (22-13) */; \
	xor	y2, g		/* y2 = f^g */; \
		vpsrlq	X0, XTMP2, 19	/* XTMP3 = W[-2] ror 19 {xDxC} */; \
	xor	y0, e		/* y0 = e ^ (e >> (11-6)) ^ (e >> (25-6)) */; \
	and	y2, e		/* y2 = (f^g)&e */; \
	ROR(	y1, (13-2))	/* y1 = (a >> (13-2)) ^ (a >> (22-2)) */; \
		vpsrld	XTMP2, XTMP2,    10	/* X0 = W[-2] >> 10 {DDCC} */; \
	xor	y1, a		/* y1 = a ^ (a >> (13-2)) ^ (a >> (22-2)) */; \
	ROR(	y0, 6)		/* y0 = S1 = (e>>6) & (e>>11) ^ (e>>25) */; \
	xor	y2, g		/* y2 = CH = ((f^g)&e)^g */; \
		vpxor	XTMP2, XTMP2, XTMP3; \
	ROR(	y1, 2)		/* y1 = S0 = (a>>2) ^ (a>>13) ^ (a>>22) */; \
	add	y2, y0		/* y2 = S1 + CH */; \
	add	y2, [rsp + _XFER + 3*4]	/* y2 = k + w + S1 + CH */; \
		vpxor	X0, X0, XTMP2	/* X0 = s1 {xDxC} */; \
	mov	y0, a		/* y0 = a */; \
	add	h, y2		/* h = h + S1 + CH + k + w */; \
	mov	y2, a		/* y2 = a */; \
		vpshufb	X0, X0, SHUF_DC00	/* X0 = s1 {DC00} */; \
	or	y0, c		/* y0 = a|c */; \
	add	d, h		/* d = d + h + S1 + CH + k + w */; \
	and	y2, c		/* y2 = a&c */; \
		vpaddd	X0, X0, XTMP0	/* X0 = {W[3], W[2], W[1], W[0]} */; \
	and	y0, b		/* y0 = (a|c)&b */; \
	add	h, y1		/* h = h + S1 + CH + k + w + S0 */; \
	or	y0, y2		/* y0 = MAJ = (a|c)&b)|(a&c) */; \
	lea	h, [h + y0]	/* h = h + S1 + CH + k + w + S0 + MAJ */

#define FOUR_ROUNDS_AND_SCHED(X0, X1, X2, X3, a, b, c, d, e, f, g, h) \
	FOUR_ROUNDS_AND_SCHED_0(X0, X1, X2, X3, a, b, c, d, e, f, g, h); \
	FOUR_ROUNDS_AND_SCHED_1(X0, X1, X2, X3, h, a, b, c, d, e, f, g); \
	FOUR_ROUNDS_AND_SCHED_2(X0, X1, X2, X3, g, h, a, b, c, d, e, f); \
	FOUR_ROUNDS_AND_SCHED_3(X0, X1, X2, X3, f, g, h, a, b, c, d, e);

/* input is [rsp + _XFER + %1 * 4] */
#define DO_ROUND(i1, a, b, c, d, e, f, g, h) \
	mov	y0, e		/* y0 = e */; \
	ROR(	y0, (25-11))	/* y0 = e >> (25-11) */; \
	mov	y1, a		/* y1 = a */; \
	xor	y0, e		/* y0 = e ^ (e >> (25-11)) */; \
	ROR(	y1, (22-13))	/* y1 = a >> (22-13) */; \
	mov	y2, f		/* y2 = f */; \
	xor	y1, a		/* y1 = a ^ (a >> (22-13) */; \
	ROR(	y0, (11-6))	/* y0 = (e >> (11-6)) ^ (e >> (25-6)) */; \
	xor	y2, g		/* y2 = f^g */; \
	xor	y0, e		/* y0 = e ^ (e >> (11-6)) ^ (e >> (25-6)) */; \
	ROR(	y1, (13-2))	/* y1 = (a >> (13-2)) ^ (a >> (22-2)) */; \
	and	y2, e		/* y2 = (f^g)&e */; \
	xor	y1, a		/* y1 = a ^ (a >> (13-2)) ^ (a >> (22-2)) */; \
	ROR(	y0, 6)		/* y0 = S1 = (e>>6) & (e>>11) ^ (e>>25) */; \
	xor	y2, g		/* y2 = CH = ((f^g)&e)^g */; \
	add	y2, y0		/* y2 = S1 + CH */; \
	ROR(	y1, 2)		/* y1 = S0 = (a>>2) ^ (a>>13) ^ (a>>22) */; \
	add	y2, [rsp + _XFER + i1 * 4]	/* y2 = k + w + S1 + CH */; \
	mov	y0, a		/* y0 = a */; \
	add	h, y2		/* h = h + S1 + CH + k + w */; \
	mov	y2, a		/* y2 = a */; \
	or	y0, c		/* y0 = a|c */; \
	add	d, h		/* d = d + h + S1 + CH + k + w */; \
	and	y2, c		/* y2 = a&c */; \
	and	y0, b		/* y0 = (a|c)&b */; \
	add	h, y1		/* h = h + S1 + CH + k + w + S0 */; \
	or	y0, y2		/* y0 = MAJ = (a|c)&b)|(a&c) */; \
	lea	h, [h + y0]	/* h = h + S1 + CH + k + w + S0 + MAJ */

/*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; void sha256_avx(void *input_data, UINT32 digest[8], UINT64 num_blks)
;; arg 1 : pointer to input data
;; arg 2 : pointer to digest
;; arg 3 : Num blocks
*/
.text
.globl _gcry_sha256_transform_amd64_avx
ELF(.type  _gcry_sha256_transform_amd64_avx,@function;)
.align 16
_gcry_sha256_transform_amd64_avx:
	CFI_STARTPROC()
	vzeroupper

	push	rbx
	CFI_PUSH(rbx)
	push	rbp
	CFI_PUSH(rbp)
	push	r13
	CFI_PUSH(r13)
	push	r14
	CFI_PUSH(r14)
	push	r15
	CFI_PUSH(r15)

	sub	rsp, STACK_SIZE
	CFI_ADJUST_CFA_OFFSET(STACK_SIZE);

	shl	NUM_BLKS, 6	/* convert to bytes */
	jz	.Ldone_hash
	add	NUM_BLKS, INP	/* pointer to end of data */
	mov	[rsp + _INP_END], NUM_BLKS

	/* load initial digest */
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

.Loop0:
	lea	TBL, [.LK256 ADD_RIP]

	/* byte swap first 16 dwords */
	COPY_XMM_AND_BSWAP(X0, [INP + 0*16], BYTE_FLIP_MASK)
	COPY_XMM_AND_BSWAP(X1, [INP + 1*16], BYTE_FLIP_MASK)
	COPY_XMM_AND_BSWAP(X2, [INP + 2*16], BYTE_FLIP_MASK)
	COPY_XMM_AND_BSWAP(X3, [INP + 3*16], BYTE_FLIP_MASK)

	mov	[rsp + _INP], INP

	/* schedule 48 input dwords, by doing 3 rounds of 16 each */
	mov	SRND, 3
.align 16
.Loop1:
	vpaddd	XFER, X0, [TBL + 0*16]
	vmovdqa	[rsp + _XFER], XFER
	FOUR_ROUNDS_AND_SCHED(X0, X1, X2, X3, a, b, c, d, e, f, g, h)

	vpaddd	XFER, X1, [TBL + 1*16]
	vmovdqa	[rsp + _XFER], XFER
	FOUR_ROUNDS_AND_SCHED(X1, X2, X3, X0, e, f, g, h, a, b, c, d)

	vpaddd	XFER, X2, [TBL + 2*16]
	vmovdqa	[rsp + _XFER], XFER
	FOUR_ROUNDS_AND_SCHED(X2, X3, X0, X1, a, b, c, d, e, f, g, h)

	vpaddd	XFER, X3, [TBL + 3*16]
	vmovdqa	[rsp + _XFER], XFER
	add	TBL, 4*16
	FOUR_ROUNDS_AND_SCHED(X3, X0, X1, X2, e, f, g, h, a, b, c, d)

	sub	SRND, 1
	jne	.Loop1

	mov	SRND, 2
.Loop2:
	vpaddd	X0, X0, [TBL + 0*16]
	vmovdqa	[rsp + _XFER], X0
	DO_ROUND(0, a, b, c, d, e, f, g, h)
	DO_ROUND(1, h, a, b, c, d, e, f, g)
	DO_ROUND(2, g, h, a, b, c, d, e, f)
	DO_ROUND(3, f, g, h, a, b, c, d, e)
	vpaddd	X1, X1, [TBL + 1*16]
	vmovdqa	[rsp + _XFER], X1
	add	TBL, 2*16
	DO_ROUND(0, e, f, g, h, a, b, c, d)
	DO_ROUND(1, d, e, f, g, h, a, b, c)
	DO_ROUND(2, c, d, e, f, g, h, a, b)
	DO_ROUND(3, b, c, d, e, f, g, h, a)

	vmovdqa	X0, X2
	vmovdqa	X1, X3

	sub	SRND, 1
	jne	.Loop2

	addm([4*0 + CTX],a)
	addm([4*1 + CTX],b)
	addm([4*2 + CTX],c)
	addm([4*3 + CTX],d)
	addm([4*4 + CTX],e)
	addm([4*5 + CTX],f)
	addm([4*6 + CTX],g)
	addm([4*7 + CTX],h)

	mov	INP, [rsp + _INP]
	add	INP, 64
	cmp	INP, [rsp + _INP_END]
	jne	.Loop0

.Ldone_hash:
	vzeroall

	vmovdqa	[rsp + _XFER], XFER
	xor     eax, eax

	add	rsp, STACK_SIZE
	CFI_ADJUST_CFA_OFFSET(-STACK_SIZE);

	pop	r15
	CFI_POP(r15)
	pop	r14
	CFI_POP(r14)
	pop	r13
	CFI_POP(r13)
	pop	rbp
	CFI_POP(rbp)
	pop	rbx
	CFI_POP(rbx)

	ret_spec_stop
	CFI_ENDPROC()


.align 16
.LK256:
	.long	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5
	.long	0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5
	.long	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3
	.long	0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174
	.long	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc
	.long	0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da
	.long	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7
	.long	0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967
	.long	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13
	.long	0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85
	.long	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3
	.long	0xd192e819,0xd6990624,0xf40e3585,0x106aa070
	.long	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5
	.long	0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3
	.long	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208
	.long	0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2

.LPSHUFFLE_BYTE_FLIP_MASK: .octa 0x0c0d0e0f08090a0b0405060700010203

/* shuffle xBxA -> 00BA */
.L_SHUF_00BA:              .octa 0xFFFFFFFFFFFFFFFF0b0a090803020100

/* shuffle xDxC -> DC00 */
.L_SHUF_DC00:              .octa 0x0b0a090803020100FFFFFFFFFFFFFFFF

#endif
#endif
