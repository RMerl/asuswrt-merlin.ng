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
*/
/*
 * Conversion to GAS assembly and integration to libgcrypt
 *  by Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * Note: original implementation was named as SHA512-SSE4. However, only SSSE3
 *       is required.
 */

#ifdef __x86_64
#include <config.h>
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && \
    defined(HAVE_INTEL_SYNTAX_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_SSSE3) && defined(USE_SHA512)

#include "asm-common-amd64.h"

.intel_syntax noprefix

.text

/* Virtual Registers */
#define msg rdi /* ARG1 */
#define digest rsi /* ARG2 */
#define msglen rdx /* ARG3 */
#define T1 rcx
#define T2 r8
#define a_64 r9
#define b_64 r10
#define c_64 r11
#define d_64 r12
#define e_64 r13
#define f_64 r14
#define g_64 r15
#define h_64 rbx
#define tmp0 rax

/*
; Local variables (stack frame)
; Note: frame_size must be an odd multiple of 8 bytes to XMM align RSP
*/
#define frame_W 0 /* Message Schedule */
#define frame_W_size (80 * 8)
#define frame_WK ((frame_W) + (frame_W_size)) /* W[t] + K[t] | W[t+1] + K[t+1] */
#define frame_WK_size (2 * 8)
#define frame_GPRSAVE ((frame_WK) + (frame_WK_size))
#define frame_GPRSAVE_size (5 * 8)
#define frame_size ((frame_GPRSAVE) + (frame_GPRSAVE_size))


/* Useful QWORD "arrays" for simpler memory references */
#define MSG(i)    msg    + 8*(i)               /* Input message (arg1) */
#define DIGEST(i) digest + 8*(i)               /* Output Digest (arg2) */
#define K_t(i)    .LK512   + 8*(i) ADD_RIP     /* SHA Constants (static mem) */
#define W_t(i)    rsp + frame_W  + 8*(i)       /* Message Schedule (stack frame) */
#define WK_2(i)   rsp + frame_WK + 8*((i) % 2) /* W[t]+K[t] (stack frame) */
/* MSG, DIGEST, K_t, W_t are arrays */
/* WK_2(t) points to 1 of 2 qwords at frame.WK depdending on t being odd/even */

#define SHA512_Round(t, a, b, c, d, e, f, g, h) \
	/* Compute Round %%t */; \
	mov	T1,   f        /* T1 = f */; \
	mov	tmp0, e        /* tmp = e */; \
	xor	T1,   g        /* T1 = f ^ g */; \
	ror	tmp0, 23 /* 41     ; tmp = e ror 23 */; \
	and	T1,   e        /* T1 = (f ^ g) & e */; \
	xor	tmp0, e        /* tmp = (e ror 23) ^ e */; \
	xor	T1,   g        /* T1 = ((f ^ g) & e) ^ g = CH(e,f,g) */; \
	add	T1,   [WK_2(t)] /* W[t] + K[t] from message scheduler */; \
	ror	tmp0, 4 /* 18      ; tmp = ((e ror 23) ^ e) ror 4 */; \
	xor	tmp0, e        /* tmp = (((e ror 23) ^ e) ror 4) ^ e */; \
	mov	T2,   a        /* T2 = a */; \
	add	T1,   h        /* T1 = CH(e,f,g) + W[t] + K[t] + h */; \
	ror	tmp0, 14 /* 14     ; tmp = ((((e ror23)^e)ror4)^e)ror14 = S1(e) */; \
	add	T1,   tmp0     /* T1 = CH(e,f,g) + W[t] + K[t] + S1(e) */; \
	mov	tmp0, a        /* tmp = a */; \
	xor	T2,   c        /* T2 = a ^ c */; \
	and	tmp0, c        /* tmp = a & c */; \
	and	T2,   b        /* T2 = (a ^ c) & b */; \
	xor	T2,   tmp0     /* T2 = ((a ^ c) & b) ^ (a & c) = Maj(a,b,c) */; \
	mov	tmp0, a        /* tmp = a */; \
	ror	tmp0, 5 /* 39      ; tmp = a ror 5 */; \
	xor	tmp0, a        /* tmp = (a ror 5) ^ a */; \
	add	d, T1          /* e(next_state) = d + T1  */; \
	ror	tmp0, 6 /* 34      ; tmp = ((a ror 5) ^ a) ror 6 */; \
	xor	tmp0, a        /* tmp = (((a ror 5) ^ a) ror 6) ^ a */; \
	lea	h, [T1 + T2]   /* a(next_state) = T1 + Maj(a,b,c) */; \
	ror	tmp0, 28 /* 28     ; tmp = ((((a ror5)^a)ror6)^a)ror28 = S0(a) */; \
	add	h, tmp0        /* a(next_state) = T1 + Maj(a,b,c) S0(a) */

#define SHA512_2Sched_2Round_sse_PART1(t, a, b, c, d, e, f, g, h) \
	/* \
	; Compute rounds %%t-2 and %%t-1 \
	; Compute message schedule QWORDS %%t and %%t+1 \
	; \
	;   Two rounds are computed based on the values for K[t-2]+W[t-2] and \
	; K[t-1]+W[t-1] which were previously stored at WK_2 by the message \
	; scheduler. \
	;   The two new schedule QWORDS are stored at [W_t(%%t)] and [W_t(%%t+1)]. \
	; They are then added to their respective SHA512 constants at \
	; [K_t(%%t)] and [K_t(%%t+1)] and stored at dqword [WK_2(%%t)] \
	;   For brievity, the comments following vectored instructions only refer to \
	; the first of a pair of QWORDS. \
	; Eg. XMM2=W[t-2] really means XMM2={W[t-2]|W[t-1]} \
	;   The computation of the message schedule and the rounds are tightly \
	; stitched to take advantage of instruction-level parallelism. \
	; For clarity, integer instructions (for the rounds calculation) are indented \
	; by one tab. Vectored instructions (for the message scheduler) are indented \
	; by two tabs. \
	*/ \
	\
	mov	T1, f; \
		movdqa	xmm2, [W_t(t-2)]  /* XMM2 = W[t-2] */; \
	xor	T1,   g; \
	and	T1,   e; \
		movdqa	xmm0, xmm2          /* XMM0 = W[t-2] */; \
	xor	T1,   g; \
	add	T1,   [WK_2(t)]; \
		movdqu	xmm5, [W_t(t-15)] /* XMM5 = W[t-15] */; \
	mov	tmp0, e; \
	ror	tmp0, 23 /* 41 */; \
		movdqa	xmm3, xmm5          /* XMM3 = W[t-15] */; \
	xor	tmp0, e; \
	ror	tmp0, 4 /* 18 */; \
		psrlq	xmm0, 61 - 19       /* XMM0 = W[t-2] >> 42 */; \
	xor	tmp0, e; \
	ror	tmp0, 14 /* 14 */; \
		psrlq	xmm3, (8 - 7)       /* XMM3 = W[t-15] >> 1 */; \
	add	T1,   tmp0; \
	add	T1,   h; \
		pxor	xmm0, xmm2          /* XMM0 = (W[t-2] >> 42) ^ W[t-2] */; \
	mov	T2,   a; \
	xor	T2,   c; \
		pxor	xmm3, xmm5          /* XMM3 = (W[t-15] >> 1) ^ W[t-15] */; \
	and	T2,   b; \
	mov	tmp0, a; \
		psrlq	xmm0, 19 - 6        /* XMM0 = ((W[t-2]>>42)^W[t-2])>>13 */; \
	and	tmp0, c; \
	xor	T2,   tmp0; \
		psrlq	xmm3, (7 - 1)       /* XMM3 = ((W[t-15]>>1)^W[t-15])>>6 */; \
	mov	tmp0, a; \
	ror	tmp0, 5 /* 39 */; \
		pxor	xmm0, xmm2          /* XMM0 = (((W[t-2]>>42)^W[t-2])>>13)^W[t-2] */; \
	xor	tmp0, a; \
	ror	tmp0, 6 /* 34 */; \
		pxor	xmm3, xmm5          /* XMM3 = (((W[t-15]>>1)^W[t-15])>>6)^W[t-15] */; \
	xor	tmp0, a; \
	ror	tmp0, 28 /* 28 */; \
		psrlq	xmm0, 6             /* XMM0 = ((((W[t-2]>>42)^W[t-2])>>13)^W[t-2])>>6 */; \
	add	T2,   tmp0; \
	add	d, T1; \
		psrlq	xmm3, 1             /* XMM3 = (((W[t-15]>>1)^W[t-15])>>6)^W[t-15]>>1 */; \
	lea	h, [T1 + T2]

#define SHA512_2Sched_2Round_sse_PART2(t, a, b, c, d, e, f, g, h) \
		movdqa	xmm1, xmm2          /* XMM1 = W[t-2] */; \
	mov	T1,   f; \
	xor	T1,   g; \
		movdqa	xmm4, xmm5          /* XMM4 = W[t-15] */; \
	and	T1,   e; \
	xor	T1,   g; \
		psllq	xmm1, (64 - 19) - (64 - 61) /* XMM1 = W[t-2] << 42 */; \
	add	T1,   [WK_2(t+1)]; \
	mov	tmp0, e; \
		psllq	xmm4, (64 - 1) - (64 - 8) /* XMM4 = W[t-15] << 7 */; \
	ror	tmp0, 23 /* 41 */; \
	xor	tmp0, e; \
		pxor	xmm1, xmm2          /* XMM1 = (W[t-2] << 42)^W[t-2] */; \
	ror	tmp0, 4 /* 18 */; \
	xor	tmp0, e; \
		pxor	xmm4, xmm5          /* XMM4 = (W[t-15]<<7)^W[t-15] */; \
	ror	tmp0, 14 /* 14 */; \
	add	T1,   tmp0; \
		psllq	xmm1, (64 - 61)     /* XMM1 = ((W[t-2] << 42)^W[t-2])<<3 */; \
	add	T1,   h; \
	mov	T2,   a; \
		psllq	xmm4, (64 - 8)      /* XMM4 = ((W[t-15]<<7)^W[t-15])<<56 */; \
	xor	T2,   c; \
	and	T2,   b; \
		pxor	xmm0, xmm1          /* XMM0 = s1(W[t-2]) */; \
	mov	tmp0, a; \
	and	tmp0, c; \
		movdqu	xmm1, [W_t(t- 7)] /* XMM1 = W[t-7] */; \
	xor	T2,   tmp0; \
		pxor	xmm3, xmm4          /* XMM3 = s0(W[t-15]) */; \
	mov	tmp0, a; \
		paddq	xmm0, xmm3          /* XMM0 = s1(W[t-2]) + s0(W[t-15]) */; \
	ror	tmp0, 5 /* 39 */; \
		paddq	xmm0, [W_t(t-16)] /* XMM0 = s1(W[t-2]) + s0(W[t-15]) + W[t-16] */; \
	xor	tmp0, a; \
		paddq	xmm0, xmm1          /* XMM0 = s1(W[t-2]) + W[t-7] + s0(W[t-15]) + W[t-16] */; \
	ror	tmp0, 6 /* 34 */; \
		movdqa	[W_t(t)], xmm0     /* Store scheduled qwords */; \
	xor	tmp0, a; \
		paddq	xmm0, [K_t(t)]      /* Compute W[t]+K[t] */; \
	ror	tmp0, 28 /* 28 */; \
		movdqa	[WK_2(t)], xmm0     /* Store W[t]+K[t] for next rounds */; \
	add	T2,   tmp0; \
	add	d, T1; \
	lea	h, [T1 + T2]

#define SHA512_2Sched_2Round_sse(t, a, b, c, d, e, f, g, h) \
	SHA512_2Sched_2Round_sse_PART1(t, a, b, c, d, e, f, g, h); \
	SHA512_2Sched_2Round_sse_PART2(t, h, a, b, c, d, e, f, g)

/*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; void sha512_sse4(const void* M, void* D, uint64_t L);
; Purpose: Updates the SHA512 digest stored at D with the message stored in M.
; The size of the message pointed to by M must be an integer multiple of SHA512
;   message blocks.
; L is the message length in SHA512 blocks.
*/
.globl _gcry_sha512_transform_amd64_ssse3
ELF(.type _gcry_sha512_transform_amd64_ssse3,@function;)
.align 16
_gcry_sha512_transform_amd64_ssse3:
	CFI_STARTPROC()
	xor eax, eax

	cmp msglen, 0
	je .Lnowork

	/* Allocate Stack Space */
	sub	rsp, frame_size
	CFI_ADJUST_CFA_OFFSET(frame_size);

	/* Save GPRs */
	mov	[rsp + frame_GPRSAVE + 8 * 0], rbx
	mov	[rsp + frame_GPRSAVE + 8 * 1], r12
	mov	[rsp + frame_GPRSAVE + 8 * 2], r13
	mov	[rsp + frame_GPRSAVE + 8 * 3], r14
	mov	[rsp + frame_GPRSAVE + 8 * 4], r15
	CFI_REL_OFFSET(rbx, frame_GPRSAVE + 8 * 0);
	CFI_REL_OFFSET(r12, frame_GPRSAVE + 8 * 1);
	CFI_REL_OFFSET(r13, frame_GPRSAVE + 8 * 2);
	CFI_REL_OFFSET(r14, frame_GPRSAVE + 8 * 3);
	CFI_REL_OFFSET(r15, frame_GPRSAVE + 8 * 4);

.Lupdateblock:

	/* Load state variables */
	mov	a_64, [DIGEST(0)]
	mov	b_64, [DIGEST(1)]
	mov	c_64, [DIGEST(2)]
	mov	d_64, [DIGEST(3)]
	mov	e_64, [DIGEST(4)]
	mov	f_64, [DIGEST(5)]
	mov	g_64, [DIGEST(6)]
	mov	h_64, [DIGEST(7)]

	/* BSWAP 2 QWORDS */
	movdqa	xmm1, [.LXMM_QWORD_BSWAP ADD_RIP]
	movdqu	xmm0, [MSG(0)]
	pshufb	xmm0, xmm1      /* BSWAP */
	movdqa	[W_t(0)], xmm0  /* Store Scheduled Pair */
	paddq	xmm0, [K_t(0)]  /* Compute W[t]+K[t] */
	movdqa	[WK_2(0)], xmm0 /* Store into WK for rounds */

	#define T_2_14(t, a, b, c, d, e, f, g, h) \
		/* BSWAP 2 QWORDS; Compute 2 Rounds */; \
		movdqu	xmm0, [MSG(t)]; \
		pshufb	xmm0, xmm1      /* BSWAP */; \
		SHA512_Round(((t) - 2), a##_64, b##_64, c##_64, d##_64, \
				        e##_64, f##_64, g##_64, h##_64); \
		movdqa	[W_t(t)], xmm0  /* Store Scheduled Pair */; \
		paddq	xmm0, [K_t(t)]  /* Compute W[t]+K[t] */; \
		SHA512_Round(((t) - 1), h##_64, a##_64, b##_64, c##_64, \
				        d##_64, e##_64, f##_64, g##_64); \
		movdqa	[WK_2(t)], xmm0 /* Store W[t]+K[t] into WK */

	#define T_16_78(t, a, b, c, d, e, f, g, h) \
		SHA512_2Sched_2Round_sse((t), a##_64, b##_64, c##_64, d##_64, \
					      e##_64, f##_64, g##_64, h##_64)

	#define T_80(t, a, b, c, d, e, f, g, h) \
		/* Compute 2 Rounds */; \
		SHA512_Round((t - 2), a##_64, b##_64, c##_64, d##_64, \
				      e##_64, f##_64, g##_64, h##_64); \
		SHA512_Round((t - 1), h##_64, a##_64, b##_64, c##_64, \
				      d##_64, e##_64, f##_64, g##_64)

	T_2_14(2, a, b, c, d, e, f, g, h)
	T_2_14(4, g, h, a, b, c, d, e, f)
	T_2_14(6, e, f, g, h, a, b, c, d)
	T_2_14(8, c, d, e, f, g, h, a, b)
	T_2_14(10, a, b, c, d, e, f, g, h)
	T_2_14(12, g, h, a, b, c, d, e, f)
	T_2_14(14, e, f, g, h, a, b, c, d)
	T_16_78(16, c, d, e, f, g, h, a, b)
	T_16_78(18, a, b, c, d, e, f, g, h)
	T_16_78(20, g, h, a, b, c, d, e, f)
	T_16_78(22, e, f, g, h, a, b, c, d)
	T_16_78(24, c, d, e, f, g, h, a, b)
	T_16_78(26, a, b, c, d, e, f, g, h)
	T_16_78(28, g, h, a, b, c, d, e, f)
	T_16_78(30, e, f, g, h, a, b, c, d)
	T_16_78(32, c, d, e, f, g, h, a, b)
	T_16_78(34, a, b, c, d, e, f, g, h)
	T_16_78(36, g, h, a, b, c, d, e, f)
	T_16_78(38, e, f, g, h, a, b, c, d)
	T_16_78(40, c, d, e, f, g, h, a, b)
	T_16_78(42, a, b, c, d, e, f, g, h)
	T_16_78(44, g, h, a, b, c, d, e, f)
	T_16_78(46, e, f, g, h, a, b, c, d)
	T_16_78(48, c, d, e, f, g, h, a, b)
	T_16_78(50, a, b, c, d, e, f, g, h)
	T_16_78(52, g, h, a, b, c, d, e, f)
	T_16_78(54, e, f, g, h, a, b, c, d)
	T_16_78(56, c, d, e, f, g, h, a, b)
	T_16_78(58, a, b, c, d, e, f, g, h)
	T_16_78(60, g, h, a, b, c, d, e, f)
	T_16_78(62, e, f, g, h, a, b, c, d)
	T_16_78(64, c, d, e, f, g, h, a, b)
	T_16_78(66, a, b, c, d, e, f, g, h)
	T_16_78(68, g, h, a, b, c, d, e, f)
	T_16_78(70, e, f, g, h, a, b, c, d)
	T_16_78(72, c, d, e, f, g, h, a, b)
	T_16_78(74, a, b, c, d, e, f, g, h)
	T_16_78(76, g, h, a, b, c, d, e, f)
	T_16_78(78, e, f, g, h, a, b, c, d)
	T_80(80, c, d, e, f, g, h, a, b)

	/* Update digest */
	add	[DIGEST(0)], a_64
	add	[DIGEST(1)], b_64
	add	[DIGEST(2)], c_64
	add	[DIGEST(3)], d_64
	add	[DIGEST(4)], e_64
	add	[DIGEST(5)], f_64
	add	[DIGEST(6)], g_64
	add	[DIGEST(7)], h_64

	/* Advance to next message block */
	add	msg, 16*8
	dec	msglen
	jnz	.Lupdateblock

	/* Restore GPRs */
	mov	rbx, [rsp + frame_GPRSAVE + 8 * 0]
	mov	r12, [rsp + frame_GPRSAVE + 8 * 1]
	mov	r13, [rsp + frame_GPRSAVE + 8 * 2]
	mov	r14, [rsp + frame_GPRSAVE + 8 * 3]
	mov	r15, [rsp + frame_GPRSAVE + 8 * 4]
	CFI_RESTORE(rbx)
	CFI_RESTORE(r12)
	CFI_RESTORE(r13)
	CFI_RESTORE(r14)
	CFI_RESTORE(r15)

	pxor	xmm0, xmm0
	pxor	xmm1, xmm1
	pxor	xmm2, xmm2
	pxor	xmm3, xmm3
	pxor	xmm4, xmm4
	pxor	xmm5, xmm5

	/* Burn stack */
	mov eax, 0
.Lerase_stack:
	movdqu [rsp + rax], xmm0
	add eax, 16
	cmp eax, frame_W_size
	jne .Lerase_stack
	movdqu [rsp + frame_WK], xmm0
	xor     eax, eax

	/* Restore Stack Pointer */
	add	rsp, frame_size
	CFI_ADJUST_CFA_OFFSET(-frame_size);

.Lnowork:
	ret_spec_stop
	CFI_ENDPROC()

/*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Binary Data
*/

.align 16

/* Mask for byte-swapping a couple of qwords in an XMM register using (v)pshufb. */
.LXMM_QWORD_BSWAP:
	.octa 0x08090a0b0c0d0e0f0001020304050607

/* K[t] used in SHA512 hashing */
.LK512:
	.quad 0x428a2f98d728ae22,0x7137449123ef65cd
	.quad 0xb5c0fbcfec4d3b2f,0xe9b5dba58189dbbc
	.quad 0x3956c25bf348b538,0x59f111f1b605d019
	.quad 0x923f82a4af194f9b,0xab1c5ed5da6d8118
	.quad 0xd807aa98a3030242,0x12835b0145706fbe
	.quad 0x243185be4ee4b28c,0x550c7dc3d5ffb4e2
	.quad 0x72be5d74f27b896f,0x80deb1fe3b1696b1
	.quad 0x9bdc06a725c71235,0xc19bf174cf692694
	.quad 0xe49b69c19ef14ad2,0xefbe4786384f25e3
	.quad 0x0fc19dc68b8cd5b5,0x240ca1cc77ac9c65
	.quad 0x2de92c6f592b0275,0x4a7484aa6ea6e483
	.quad 0x5cb0a9dcbd41fbd4,0x76f988da831153b5
	.quad 0x983e5152ee66dfab,0xa831c66d2db43210
	.quad 0xb00327c898fb213f,0xbf597fc7beef0ee4
	.quad 0xc6e00bf33da88fc2,0xd5a79147930aa725
	.quad 0x06ca6351e003826f,0x142929670a0e6e70
	.quad 0x27b70a8546d22ffc,0x2e1b21385c26c926
	.quad 0x4d2c6dfc5ac42aed,0x53380d139d95b3df
	.quad 0x650a73548baf63de,0x766a0abb3c77b2a8
	.quad 0x81c2c92e47edaee6,0x92722c851482353b
	.quad 0xa2bfe8a14cf10364,0xa81a664bbc423001
	.quad 0xc24b8b70d0f89791,0xc76c51a30654be30
	.quad 0xd192e819d6ef5218,0xd69906245565a910
	.quad 0xf40e35855771202a,0x106aa07032bbd1b8
	.quad 0x19a4c116b8d2d0c8,0x1e376c085141ab53
	.quad 0x2748774cdf8eeb99,0x34b0bcb5e19b48a8
	.quad 0x391c0cb3c5c95a63,0x4ed8aa4ae3418acb
	.quad 0x5b9cca4f7763e373,0x682e6ff3d6b2b8a3
	.quad 0x748f82ee5defb2fc,0x78a5636f43172f60
	.quad 0x84c87814a1f0ab72,0x8cc702081a6439ec
	.quad 0x90befffa23631e28,0xa4506cebde82bde9
	.quad 0xbef9a3f7b2c67915,0xc67178f2e372532b
	.quad 0xca273eceea26619c,0xd186b8c721c0c207
	.quad 0xeada7dd6cde0eb1e,0xf57d4f7fee6ed178
	.quad 0x06f067aa72176fba,0x0a637dc5a2c898a6
	.quad 0x113f9804bef90dae,0x1b710b35131c471b
	.quad 0x28db77f523047d84,0x32caab7b40c72493
	.quad 0x3c9ebe0a15c9bebc,0x431d67c49c100d4c
	.quad 0x4cc5d4becb3e42b6,0x597f299cfc657e2a
	.quad 0x5fcb6fab3ad6faec,0x6c44198c4a475817

#endif
#endif
