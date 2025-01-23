/* chacha20-amd64-ssse3.S  -  SSSE3 implementation of ChaCha20 cipher
 *
 * Copyright (C) 2017-2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Based on D. J. Bernstein reference implementation at
 * http://cr.yp.to/chacha.html:
 *
 * chacha-regs.c version 20080118
 * D. J. Bernstein
 * Public domain.
 */

#ifdef __x86_64
#include <config.h>
#if defined(HAVE_GCC_INLINE_ASM_SSSE3) && \
   (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
    defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))

.text

#include "asm-common-amd64.h"
#include "asm-poly1305-amd64.h"

/* register macros */
#define INPUT %rdi
#define DST   %rsi
#define SRC   %rdx
#define NBLKS %rcx
#define ROUND %eax

/* stack structure */
#define STACK_VEC_X12 (16)
#define STACK_VEC_X13 (16 + STACK_VEC_X12)
#define STACK_TMP     (16 + STACK_VEC_X13)
#define STACK_TMP1    (16 + STACK_TMP)
#define STACK_TMP2    (16 + STACK_TMP1)

#define STACK_MAX     (16 + STACK_TMP2)

/* vector registers */
#define X0 %xmm0
#define X1 %xmm1
#define X2 %xmm2
#define X3 %xmm3
#define X4 %xmm4
#define X5 %xmm5
#define X6 %xmm6
#define X7 %xmm7
#define X8 %xmm8
#define X9 %xmm9
#define X10 %xmm10
#define X11 %xmm11
#define X12 %xmm12
#define X13 %xmm13
#define X14 %xmm14
#define X15 %xmm15

/**********************************************************************
  helper macros
 **********************************************************************/

/* 4x4 32-bit integer matrix transpose */
#define transpose_4x4(x0, x1, x2, x3, t1, t2, t3) \
	movdqa    x0, t2; \
	punpckhdq x1, t2; \
	punpckldq x1, x0; \
	\
	movdqa    x2, t1; \
	punpckldq x3, t1; \
	punpckhdq x3, x2; \
	\
	movdqa     x0, x1; \
	punpckhqdq t1, x1; \
	punpcklqdq t1, x0; \
	\
	movdqa     t2, x3; \
	punpckhqdq x2, x3; \
	punpcklqdq x2, t2; \
	movdqa     t2, x2;

/* fill xmm register with 32-bit value from memory */
#define pbroadcastd(mem32, xreg) \
	movd mem32, xreg; \
	pshufd $0, xreg, xreg;

/* xor with unaligned memory operand */
#define pxor_u(umem128, xreg, t) \
	movdqu umem128, t; \
	pxor t, xreg;

/* xor register with unaligned src and save to unaligned dst */
#define xor_src_dst(dst, src, offset, xreg, t) \
	pxor_u(offset(src), xreg, t); \
	movdqu xreg, offset(dst);

#define clear(x) pxor x,x;

/**********************************************************************
  4-way chacha20
 **********************************************************************/

#define ROTATE2(v1,v2,c,tmp1,tmp2)	\
	movdqa v1, tmp1; 		\
	movdqa v2, tmp2; 		\
	psrld $(32 - (c)), v1;		\
	pslld $(c), tmp1;		\
	paddb tmp1, v1;			\
	psrld $(32 - (c)), v2;		\
	pslld $(c), tmp2;		\
	paddb tmp2, v2;

#define ROTATE_SHUF_2(v1,v2,shuf)	\
	pshufb shuf, v1;		\
	pshufb shuf, v2;

#define XOR(ds,s) \
	pxor s, ds;

#define PLUS(ds,s) \
	paddd s, ds;

#define QUARTERROUND2(a1,b1,c1,d1,a2,b2,c2,d2,ign,tmp1,tmp2,\
		      interleave_op1,interleave_op2)		\
	movdqa .Lshuf_rol16 rRIP, tmp1;				\
		interleave_op1;					\
	PLUS(a1,b1); PLUS(a2,b2); XOR(d1,a1); XOR(d2,a2);	\
	    ROTATE_SHUF_2(d1, d2, tmp1);			\
	PLUS(c1,d1); PLUS(c2,d2); XOR(b1,c1); XOR(b2,c2);	\
	    ROTATE2(b1, b2, 12, tmp1, tmp2);			\
	movdqa .Lshuf_rol8 rRIP, tmp1;				\
		interleave_op2;					\
	PLUS(a1,b1); PLUS(a2,b2); XOR(d1,a1); XOR(d2,a2);	\
	    ROTATE_SHUF_2(d1, d2, tmp1);			\
	PLUS(c1,d1); PLUS(c2,d2); XOR(b1,c1); XOR(b2,c2);	\
	    ROTATE2(b1, b2,  7, tmp1, tmp2);

chacha20_data:
.align 16
.Lshuf_rol16:
	.byte 2,3,0,1,6,7,4,5,10,11,8,9,14,15,12,13
.Lshuf_rol8:
	.byte 3,0,1,2,7,4,5,6,11,8,9,10,15,12,13,14
.Lcounter1:
	.long 1,0,0,0
.Linc_counter:
	.long 0,1,2,3
.Lunsigned_cmp:
	.long 0x80000000,0x80000000,0x80000000,0x80000000

.align 8
.globl _gcry_chacha20_amd64_ssse3_blocks4
ELF(.type _gcry_chacha20_amd64_ssse3_blocks4,@function;)

_gcry_chacha20_amd64_ssse3_blocks4:
	/* input:
	 *	%rdi: input
	 *	%rsi: dst
	 *	%rdx: src
	 *	%rcx: nblks (multiple of 4)
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	subq $STACK_MAX, %rsp;
	andq $~15, %rsp;

.Loop4:
	mov $20, ROUND;

	/* Construct counter vectors X12 and X13 */
	movdqa .Linc_counter rRIP, X0;
	movdqa .Lunsigned_cmp rRIP, X2;
	pbroadcastd((12 * 4)(INPUT), X12);
	pbroadcastd((13 * 4)(INPUT), X13);
	paddd X0, X12;
	movdqa X12, X1;
	pxor X2, X0;
	pxor X2, X1;
	pcmpgtd X1, X0;
	psubd X0, X13;
	movdqa X12, (STACK_VEC_X12)(%rsp);
	movdqa X13, (STACK_VEC_X13)(%rsp);

	/* Load vectors */
	pbroadcastd((0 * 4)(INPUT), X0);
	pbroadcastd((1 * 4)(INPUT), X1);
	pbroadcastd((2 * 4)(INPUT), X2);
	pbroadcastd((3 * 4)(INPUT), X3);
	pbroadcastd((4 * 4)(INPUT), X4);
	pbroadcastd((5 * 4)(INPUT), X5);
	pbroadcastd((6 * 4)(INPUT), X6);
	pbroadcastd((7 * 4)(INPUT), X7);
	pbroadcastd((8 * 4)(INPUT), X8);
	pbroadcastd((9 * 4)(INPUT), X9);
	pbroadcastd((10 * 4)(INPUT), X10);
	pbroadcastd((11 * 4)(INPUT), X11);
	pbroadcastd((14 * 4)(INPUT), X14);
	pbroadcastd((15 * 4)(INPUT), X15);
	movdqa X11, (STACK_TMP)(%rsp);
	movdqa X15, (STACK_TMP1)(%rsp);

.Lround2_4:
	QUARTERROUND2(X0, X4,  X8, X12,   X1, X5,  X9, X13, tmp:=,X11,X15,,)
	movdqa (STACK_TMP)(%rsp), X11;
	movdqa (STACK_TMP1)(%rsp), X15;
	movdqa X8, (STACK_TMP)(%rsp);
	movdqa X9, (STACK_TMP1)(%rsp);
	QUARTERROUND2(X2, X6, X10, X14,   X3, X7, X11, X15, tmp:=,X8,X9,,)
	QUARTERROUND2(X0, X5, X10, X15,   X1, X6, X11, X12, tmp:=,X8,X9,,)
	movdqa (STACK_TMP)(%rsp), X8;
	movdqa (STACK_TMP1)(%rsp), X9;
	movdqa X11, (STACK_TMP)(%rsp);
	movdqa X15, (STACK_TMP1)(%rsp);
	QUARTERROUND2(X2, X7,  X8, X13,   X3, X4,  X9, X14, tmp:=,X11,X15,,)
	sub $2, ROUND;
	jnz .Lround2_4;

	/* tmp := X15 */
	movdqa (STACK_TMP)(%rsp), X11;
	pbroadcastd((0 * 4)(INPUT), X15);
	PLUS(X0, X15);
	pbroadcastd((1 * 4)(INPUT), X15);
	PLUS(X1, X15);
	pbroadcastd((2 * 4)(INPUT), X15);
	PLUS(X2, X15);
	pbroadcastd((3 * 4)(INPUT), X15);
	PLUS(X3, X15);
	pbroadcastd((4 * 4)(INPUT), X15);
	PLUS(X4, X15);
	pbroadcastd((5 * 4)(INPUT), X15);
	PLUS(X5, X15);
	pbroadcastd((6 * 4)(INPUT), X15);
	PLUS(X6, X15);
	pbroadcastd((7 * 4)(INPUT), X15);
	PLUS(X7, X15);
	pbroadcastd((8 * 4)(INPUT), X15);
	PLUS(X8, X15);
	pbroadcastd((9 * 4)(INPUT), X15);
	PLUS(X9, X15);
	pbroadcastd((10 * 4)(INPUT), X15);
	PLUS(X10, X15);
	pbroadcastd((11 * 4)(INPUT), X15);
	PLUS(X11, X15);
	movdqa (STACK_VEC_X12)(%rsp), X15;
	PLUS(X12, X15);
	movdqa (STACK_VEC_X13)(%rsp), X15;
	PLUS(X13, X15);
	movdqa X13, (STACK_TMP)(%rsp);
	pbroadcastd((14 * 4)(INPUT), X15);
	PLUS(X14, X15);
	movdqa (STACK_TMP1)(%rsp), X15;
	movdqa X14, (STACK_TMP1)(%rsp);
	pbroadcastd((15 * 4)(INPUT), X13);
	PLUS(X15, X13);
	movdqa X15, (STACK_TMP2)(%rsp);

	/* Update counter */
	addq $4, (12 * 4)(INPUT);

	transpose_4x4(X0, X1, X2, X3, X13, X14, X15);
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 0), X0, X15);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 0), X1, X15);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 0), X2, X15);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 0), X3, X15);
	transpose_4x4(X4, X5, X6, X7, X0, X1, X2);
	movdqa (STACK_TMP)(%rsp), X13;
	movdqa (STACK_TMP1)(%rsp), X14;
	movdqa (STACK_TMP2)(%rsp), X15;
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 1), X4, X0);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 1), X5, X0);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 1), X6, X0);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 1), X7, X0);
	transpose_4x4(X8, X9, X10, X11, X0, X1, X2);
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 2), X8, X0);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 2), X9, X0);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 2), X10, X0);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 2), X11, X0);
	transpose_4x4(X12, X13, X14, X15, X0, X1, X2);
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 3), X12, X0);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 3), X13, X0);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 3), X14, X0);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 3), X15, X0);

	sub $4, NBLKS;
	lea (4 * 64)(DST), DST;
	lea (4 * 64)(SRC), SRC;
	jnz .Loop4;

	/* clear the used vector registers and stack */
	clear(X0);
	movdqa X0, (STACK_VEC_X12)(%rsp);
	movdqa X0, (STACK_VEC_X13)(%rsp);
	movdqa X0, (STACK_TMP)(%rsp);
	movdqa X0, (STACK_TMP1)(%rsp);
	movdqa X0, (STACK_TMP2)(%rsp);
	clear(X1);
	clear(X2);
	clear(X3);
	clear(X4);
	clear(X5);
	clear(X6);
	clear(X7);
	clear(X8);
	clear(X9);
	clear(X10);
	clear(X11);
	clear(X12);
	clear(X13);
	clear(X14);
	clear(X15);

	/* eax zeroed by round loop. */
	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_chacha20_amd64_ssse3_blocks4,
	  .-_gcry_chacha20_amd64_ssse3_blocks4;)

/**********************************************************************
  2-way && 1-way chacha20
 **********************************************************************/

#define ROTATE_SHUF(v1,shuf)		\
	pshufb shuf, v1;

#define ROTATE(v1,c,tmp1)		\
	movdqa v1, tmp1; 		\
	psrld $(32 - (c)), v1;		\
	pslld $(c), tmp1;		\
	paddb tmp1, v1;

#define WORD_SHUF(v1,shuf)		\
	pshufd $shuf, v1, v1;

#define QUARTERROUND4(x0,x1,x2,x3,shuf_rol8,shuf_rol16,tmp1,shuf_x1,\
		      shuf_x2,shuf_x3) \
	PLUS(x0, x1); XOR(x3, x0); ROTATE_SHUF(x3, shuf_rol16); \
	PLUS(x2, x3); XOR(x1, x2); ROTATE(x1, 12, tmp1); \
	PLUS(x0, x1); XOR(x3, x0); ROTATE_SHUF(x3, shuf_rol8); \
	PLUS(x2, x3); \
	  WORD_SHUF(x3, shuf_x3); \
		      XOR(x1, x2); \
	  WORD_SHUF(x2, shuf_x2); \
				   ROTATE(x1, 7, tmp1); \
	  WORD_SHUF(x1, shuf_x1);

.align 8
.globl _gcry_chacha20_amd64_ssse3_blocks1
ELF(.type _gcry_chacha20_amd64_ssse3_blocks1,@function;)

_gcry_chacha20_amd64_ssse3_blocks1:
	/* input:
	 *	%rdi: input
	 *	%rsi: dst
	 *	%rdx: src
	 *	%rcx: nblks
	 */
	CFI_STARTPROC();

	/* Load constants */
	movdqa .Lcounter1 rRIP, X4;
	movdqa .Lshuf_rol8 rRIP, X5;
	movdqa .Lshuf_rol16 rRIP, X6;

	/* Load state */
	movdqu (0 * 4)(INPUT), X10;
	movdqu (4 * 4)(INPUT), X11;
	movdqu (8 * 4)(INPUT), X12;
	movdqu (12 * 4)(INPUT), X13;

	cmp $2, NBLKS;
	jb .Loop1;

	mov $20, ROUND;

	movdqa X10, X0;
	movdqa X11, X1;
	movdqa X12, X2;
	movdqa X13, X3;

	movdqa X10, X8;
	movdqa X11, X9;
	movdqa X12, X14;
	movdqa X13, X15;
	paddq X4, X15;

.Lround2_2:
	QUARTERROUND4(X0, X1, X2,  X3,  X5, X6, X7, 0x39, 0x4e, 0x93);
	QUARTERROUND4(X8, X9, X14, X15, X5, X6, X7, 0x39, 0x4e, 0x93);
	QUARTERROUND4(X0, X1, X2,  X3,  X5, X6, X7, 0x93, 0x4e, 0x39);
	QUARTERROUND4(X8, X9, X14, X15, X5, X6, X7, 0x93, 0x4e, 0x39);
	sub $2, ROUND;
	jnz .Lround2_2;

	PLUS(X0, X10);
	PLUS(X1, X11);
	PLUS(X2, X12);
	PLUS(X3, X13);

	/* Update counter */
	paddq X4, X13;

	PLUS(X8, X10);
	PLUS(X9, X11);
	PLUS(X14, X12);
	PLUS(X15, X13);

	/* Update counter */
	paddq X4, X13;

	xor_src_dst(DST, SRC, 0 * 4, X0, X7);
	xor_src_dst(DST, SRC, 4 * 4, X1, X7);
	xor_src_dst(DST, SRC, 8 * 4, X2, X7);
	xor_src_dst(DST, SRC, 12 * 4, X3, X7);
	xor_src_dst(DST, SRC, 16 * 4, X8, X7);
	xor_src_dst(DST, SRC, 20 * 4, X9, X7);
	xor_src_dst(DST, SRC, 24 * 4, X14, X7);
	xor_src_dst(DST, SRC, 28 * 4, X15, X7);

	lea (2 * 64)(DST), DST;
	lea (2 * 64)(SRC), SRC;

	clear(X8);
	clear(X9);
	clear(X14);
	clear(X15);

	sub $2, NBLKS;
	jz .Ldone1;

.Loop1:
	mov $20, ROUND;

	movdqa X10, X0;
	movdqa X11, X1;
	movdqa X12, X2;
	movdqa X13, X3;

.Lround2_1:
	QUARTERROUND4(X0, X1, X2, X3, X5, X6, X7, 0x39, 0x4e, 0x93);
	QUARTERROUND4(X0, X1, X2, X3, X5, X6, X7, 0x93, 0x4e, 0x39);
	sub $2, ROUND;
	jnz .Lround2_1;

	PLUS(X0, X10);
	PLUS(X1, X11);
	PLUS(X2, X12);
	PLUS(X3, X13);

	/* Update counter */
	paddq X4, X13;

	xor_src_dst(DST, SRC, 0 * 4, X0, X7);
	xor_src_dst(DST, SRC, 4 * 4, X1, X7);
	xor_src_dst(DST, SRC, 8 * 4, X2, X7);
	xor_src_dst(DST, SRC, 12 * 4, X3, X7);

	lea (64)(DST), DST;
	lea (64)(SRC), SRC;

	sub $1, NBLKS;
	jnz .Loop1;

.Ldone1:
	/* Store counter */
	movdqu X13, (12 * 4)(INPUT);

	/* clear the used vector registers */
	clear(X0);
	clear(X1);
	clear(X2);
	clear(X3);
	clear(X4);
	clear(X5);
	clear(X6);
	clear(X7);
	clear(X10);
	clear(X11);
	clear(X12);
	clear(X13);

	/* eax zeroed by round loop. */
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_chacha20_amd64_ssse3_blocks1,
	  .-_gcry_chacha20_amd64_ssse3_blocks1;)

/**********************************************************************
  4-way stitched chacha20-poly1305
 **********************************************************************/

#define _ /*_*/

.align 8
.globl _gcry_chacha20_poly1305_amd64_ssse3_blocks4
ELF(.type _gcry_chacha20_poly1305_amd64_ssse3_blocks4,@function;)

_gcry_chacha20_poly1305_amd64_ssse3_blocks4:
	/* input:
	 *	%rdi: input
	 *	%rsi: dst
	 *	%rdx: src
	 *	%rcx: nblks (multiple of 4)
	 *	%r9: poly1305-state
	 *	%r8: poly1305-src
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	subq $(9 * 8) + STACK_MAX + 16, %rsp;
	andq $~15, %rsp;

	movq %rbx, (STACK_MAX + 0 * 8)(%rsp);
	movq %r12, (STACK_MAX + 1 * 8)(%rsp);
	movq %r13, (STACK_MAX + 2 * 8)(%rsp);
	movq %r14, (STACK_MAX + 3 * 8)(%rsp);
	movq %r15, (STACK_MAX + 4 * 8)(%rsp);
	CFI_REG_ON_STACK(rbx, STACK_MAX + 0 * 8);
	CFI_REG_ON_STACK(r12, STACK_MAX + 1 * 8);
	CFI_REG_ON_STACK(r13, STACK_MAX + 2 * 8);
	CFI_REG_ON_STACK(r14, STACK_MAX + 3 * 8);
	CFI_REG_ON_STACK(r15, STACK_MAX + 4 * 8);

	movq %rdx, (STACK_MAX + 5 * 8)(%rsp); # SRC
	movq %rsi, (STACK_MAX + 6 * 8)(%rsp); # DST
	movq %rcx, (STACK_MAX + 7 * 8)(%rsp); # NBLKS

	/* Load state */
	POLY1305_LOAD_STATE();

.Loop_poly4:

	/* Construct counter vectors X12 and X13 */
	movdqa .Linc_counter rRIP, X0;
	movdqa .Lunsigned_cmp rRIP, X2;
	pbroadcastd((12 * 4)(INPUT), X12);
	pbroadcastd((13 * 4)(INPUT), X13);
	paddd X0, X12;
	movdqa X12, X1;
	pxor X2, X0;
	pxor X2, X1;
	pcmpgtd X1, X0;
	psubd X0, X13;
	movdqa X12, (STACK_VEC_X12)(%rsp);
	movdqa X13, (STACK_VEC_X13)(%rsp);

	/* Load vectors */
	pbroadcastd((0 * 4)(INPUT), X0);
	pbroadcastd((1 * 4)(INPUT), X1);
	pbroadcastd((2 * 4)(INPUT), X2);
	pbroadcastd((3 * 4)(INPUT), X3);
	pbroadcastd((4 * 4)(INPUT), X4);
	pbroadcastd((5 * 4)(INPUT), X5);
	pbroadcastd((6 * 4)(INPUT), X6);
	pbroadcastd((7 * 4)(INPUT), X7);
	pbroadcastd((8 * 4)(INPUT), X8);
	pbroadcastd((9 * 4)(INPUT), X9);
	pbroadcastd((10 * 4)(INPUT), X10);
	pbroadcastd((11 * 4)(INPUT), X11);
	pbroadcastd((14 * 4)(INPUT), X14);
	pbroadcastd((15 * 4)(INPUT), X15);
	movdqa X11, (STACK_TMP)(%rsp);
	movdqa X15, (STACK_TMP1)(%rsp);

	/* Process four ChaCha20 blocks and sixteen Poly1305 blocks. */

	movl $20, (STACK_MAX + 8 * 8 + 4)(%rsp);
.Lround4_with_poly1305_outer:
	movl $6, (STACK_MAX + 8 * 8)(%rsp);
.Lround4_with_poly1305_inner1:
	/* rounds 0-5 & 10-15 */
		      POLY1305_BLOCK_PART1(0 * 16)
	QUARTERROUND2(X0, X4,  X8, X12,   X1, X5,  X9, X13, tmp:=,X11,X15,
		      POLY1305_BLOCK_PART2(),
		      POLY1305_BLOCK_PART3())
	movdqa (STACK_TMP)(%rsp), X11;
	movdqa (STACK_TMP1)(%rsp), X15;
	movdqa X8, (STACK_TMP)(%rsp);
	movdqa X9, (STACK_TMP1)(%rsp);
	QUARTERROUND2(X2, X6, X10, X14,   X3, X7, X11, X15, tmp:=,X8,X9,
		      POLY1305_BLOCK_PART4(),
		      POLY1305_BLOCK_PART5())
		      POLY1305_BLOCK_PART1(1 * 16)
		      lea (2 * 16)(POLY_RSRC), POLY_RSRC;
	QUARTERROUND2(X0, X5, X10, X15,   X1, X6, X11, X12, tmp:=,X8,X9,
		      POLY1305_BLOCK_PART2(),
		      POLY1305_BLOCK_PART3())
	movdqa (STACK_TMP)(%rsp), X8;
	movdqa (STACK_TMP1)(%rsp), X9;
	movdqa X11, (STACK_TMP)(%rsp);
	movdqa X15, (STACK_TMP1)(%rsp);
	QUARTERROUND2(X2, X7,  X8, X13,   X3, X4,  X9, X14, tmp:=,X11,X15,
		      POLY1305_BLOCK_PART4(),
		      POLY1305_BLOCK_PART5())

	subl $2, (STACK_MAX + 8 * 8)(%rsp);
	jnz .Lround4_with_poly1305_inner1;

	movl $4, (STACK_MAX + 8 * 8)(%rsp);
.Lround4_with_poly1305_inner2:
	/* rounds 6-9 & 16-19 */
		      POLY1305_BLOCK_PART1(0 * 16)
		      lea (1 * 16)(POLY_RSRC), POLY_RSRC;
	QUARTERROUND2(X0, X4,  X8, X12,   X1, X5,  X9, X13, tmp:=,X11,X15,
		      POLY1305_BLOCK_PART2(),
		      _)
	movdqa (STACK_TMP)(%rsp), X11;
	movdqa (STACK_TMP1)(%rsp), X15;
	movdqa X8, (STACK_TMP)(%rsp);
	movdqa X9, (STACK_TMP1)(%rsp);
	QUARTERROUND2(X2, X6, X10, X14,   X3, X7, X11, X15, tmp:=,X8,X9,
		      POLY1305_BLOCK_PART3(),
		      _)
	QUARTERROUND2(X0, X5, X10, X15,   X1, X6, X11, X12, tmp:=,X8,X9,
		      POLY1305_BLOCK_PART4(),
		      _)
	movdqa (STACK_TMP)(%rsp), X8;
	movdqa (STACK_TMP1)(%rsp), X9;
	movdqa X11, (STACK_TMP)(%rsp);
	movdqa X15, (STACK_TMP1)(%rsp);
	QUARTERROUND2(X2, X7,  X8, X13,   X3, X4,  X9, X14, tmp:=,X11,X15,
		      POLY1305_BLOCK_PART5(),
		      _)

	subl $2, (STACK_MAX + 8 * 8)(%rsp);
	jnz .Lround4_with_poly1305_inner2;

	subl $10, (STACK_MAX + 8 * 8 + 4)(%rsp);
	jnz .Lround4_with_poly1305_outer;

	/* tmp := X15 */
	movdqa (STACK_TMP)(%rsp), X11;
	pbroadcastd((0 * 4)(INPUT), X15);
	PLUS(X0, X15);
	pbroadcastd((1 * 4)(INPUT), X15);
	PLUS(X1, X15);
	pbroadcastd((2 * 4)(INPUT), X15);
	PLUS(X2, X15);
	pbroadcastd((3 * 4)(INPUT), X15);
	PLUS(X3, X15);
	pbroadcastd((4 * 4)(INPUT), X15);
	PLUS(X4, X15);
	pbroadcastd((5 * 4)(INPUT), X15);
	PLUS(X5, X15);
	pbroadcastd((6 * 4)(INPUT), X15);
	PLUS(X6, X15);
	pbroadcastd((7 * 4)(INPUT), X15);
	PLUS(X7, X15);
	pbroadcastd((8 * 4)(INPUT), X15);
	PLUS(X8, X15);
	pbroadcastd((9 * 4)(INPUT), X15);
	PLUS(X9, X15);
	pbroadcastd((10 * 4)(INPUT), X15);
	PLUS(X10, X15);
	pbroadcastd((11 * 4)(INPUT), X15);
	PLUS(X11, X15);
	movdqa (STACK_VEC_X12)(%rsp), X15;
	PLUS(X12, X15);
	movdqa (STACK_VEC_X13)(%rsp), X15;
	PLUS(X13, X15);
	movdqa X13, (STACK_TMP)(%rsp);
	pbroadcastd((14 * 4)(INPUT), X15);
	PLUS(X14, X15);
	movdqa (STACK_TMP1)(%rsp), X15;
	movdqa X14, (STACK_TMP1)(%rsp);
	pbroadcastd((15 * 4)(INPUT), X13);
	PLUS(X15, X13);
	movdqa X15, (STACK_TMP2)(%rsp);

	/* Update counter */
	addq $4, (12 * 4)(INPUT);

	movq (STACK_MAX + 5 * 8)(%rsp), SRC;
	movq (STACK_MAX + 6 * 8)(%rsp), DST;

	transpose_4x4(X0, X1, X2, X3, X13, X14, X15);
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 0), X0, X15);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 0), X1, X15);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 0), X2, X15);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 0), X3, X15);
	transpose_4x4(X4, X5, X6, X7, X0, X1, X2);
	movdqa (STACK_TMP)(%rsp), X13;
	movdqa (STACK_TMP1)(%rsp), X14;
	movdqa (STACK_TMP2)(%rsp), X15;
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 1), X4, X0);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 1), X5, X0);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 1), X6, X0);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 1), X7, X0);
	transpose_4x4(X8, X9, X10, X11, X0, X1, X2);
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 2), X8, X0);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 2), X9, X0);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 2), X10, X0);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 2), X11, X0);
	transpose_4x4(X12, X13, X14, X15, X0, X1, X2);
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 3), X12, X0);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 3), X13, X0);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 3), X14, X0);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 3), X15, X0);

	subq $4, (STACK_MAX + 7 * 8)(%rsp); # NBLKS

	lea (4 * 64)(DST), DST;
	lea (4 * 64)(SRC), SRC;
	movq SRC, (STACK_MAX + 5 * 8)(%rsp);
	movq DST, (STACK_MAX + 6 * 8)(%rsp);

	jnz .Loop_poly4;

	/* Store state */
	POLY1305_STORE_STATE();

	/* clear the used vector registers and stack */
	clear(X0);
	movdqa X0, (STACK_VEC_X12)(%rsp);
	movdqa X0, (STACK_VEC_X13)(%rsp);
	movdqa X0, (STACK_TMP)(%rsp);
	movdqa X0, (STACK_TMP1)(%rsp);
	movdqa X0, (STACK_TMP2)(%rsp);
	clear(X1);
	clear(X2);
	clear(X3);
	clear(X4);
	clear(X5);
	clear(X6);
	clear(X7);
	clear(X8);
	clear(X9);
	clear(X10);
	clear(X11);
	clear(X12);
	clear(X13);
	clear(X14);
	clear(X15);

	movq (STACK_MAX + 0 * 8)(%rsp), %rbx;
	movq (STACK_MAX + 1 * 8)(%rsp), %r12;
	movq (STACK_MAX + 2 * 8)(%rsp), %r13;
	movq (STACK_MAX + 3 * 8)(%rsp), %r14;
	movq (STACK_MAX + 4 * 8)(%rsp), %r15;
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);
	CFI_RESTORE(%r14);
	CFI_RESTORE(%r15);

	xorl %eax, %eax;
	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_chacha20_poly1305_amd64_ssse3_blocks4,
	  .-_gcry_chacha20_poly1305_amd64_ssse3_blocks4;)

/**********************************************************************
  2-way && 1-way stitched chacha20-poly1305
 **********************************************************************/

.align 8
.globl _gcry_chacha20_poly1305_amd64_ssse3_blocks1
ELF(.type _gcry_chacha20_poly1305_amd64_ssse3_blocks1,@function;)

_gcry_chacha20_poly1305_amd64_ssse3_blocks1:
	/* input:
	 *	%rdi: chacha20-state
	 *	%rsi: dst
	 *	%rdx: src
	 *	%rcx: nblks
	 *	%r9: poly1305-state
	 *	%r8: poly1305-src
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	subq $(9 * 8), %rsp;
	movq %rbx, (0 * 8)(%rsp);
	movq %r12, (1 * 8)(%rsp);
	movq %r13, (2 * 8)(%rsp);
	movq %r14, (3 * 8)(%rsp);
	movq %r15, (4 * 8)(%rsp);
	CFI_REG_ON_STACK(rbx, 0 * 8);
	CFI_REG_ON_STACK(r12, 1 * 8);
	CFI_REG_ON_STACK(r13, 2 * 8);
	CFI_REG_ON_STACK(r14, 3 * 8);
	CFI_REG_ON_STACK(r15, 4 * 8);

	movq %rdx, (5 * 8)(%rsp); # SRC
	movq %rsi, (6 * 8)(%rsp); # DST
	movq %rcx, (7 * 8)(%rsp); # NBLKS

	/* Load constants */
	movdqa .Lcounter1 rRIP, X4;
	movdqa .Lshuf_rol8 rRIP, X5;
	movdqa .Lshuf_rol16 rRIP, X6;

	/* Load state */
	movdqu (0 * 4)(INPUT), X10;
	movdqu (4 * 4)(INPUT), X11;
	movdqu (8 * 4)(INPUT), X12;
	movdqu (12 * 4)(INPUT), X13;

	POLY1305_LOAD_STATE();

	cmpq $2, (7 * 8)(%rsp); #NBLKS
	jb .Loop_poly1;

	movdqa X10, X0;
	movdqa X11, X1;
	movdqa X12, X2;
	movdqa X13, X3;

	movdqa X10, X8;
	movdqa X11, X9;
	movdqa X12, X14;
	movdqa X13, X15;
	paddq X4, X15;

	/* Process two ChaCha20 blocks and eight Poly1305 blocks. */

	movl $20, (8 * 8 + 4)(%rsp);
.Lround2_with_poly1305_outer:
	movl $8, (8 * 8)(%rsp);
.Lround2_with_poly1305_inner:
	POLY1305_BLOCK_PART1(0 * 16);
	  QUARTERROUND4(X0, X1, X2,  X3,  X5, X6, X7, 0x39, 0x4e, 0x93);
	lea (1 * 16)(POLY_RSRC), POLY_RSRC;
	POLY1305_BLOCK_PART2();
	  QUARTERROUND4(X8, X9, X14, X15, X5, X6, X7, 0x39, 0x4e, 0x93);
	POLY1305_BLOCK_PART3();
	  QUARTERROUND4(X0, X1, X2,  X3,  X5, X6, X7, 0x93, 0x4e, 0x39);
	POLY1305_BLOCK_PART4();
	  QUARTERROUND4(X8, X9, X14, X15, X5, X6, X7, 0x93, 0x4e, 0x39);
	POLY1305_BLOCK_PART5();

	subl $2, (8 * 8)(%rsp);
	jnz .Lround2_with_poly1305_inner;

	  QUARTERROUND4(X0, X1, X2,  X3,  X5, X6, X7, 0x39, 0x4e, 0x93);
	  QUARTERROUND4(X8, X9, X14, X15, X5, X6, X7, 0x39, 0x4e, 0x93);
	  QUARTERROUND4(X0, X1, X2,  X3,  X5, X6, X7, 0x93, 0x4e, 0x39);
	  QUARTERROUND4(X8, X9, X14, X15, X5, X6, X7, 0x93, 0x4e, 0x39);

	subl $10, (8 * 8 + 4)(%rsp);
	jnz .Lround2_with_poly1305_outer;

	movq (5 * 8)(%rsp), SRC;
	movq (6 * 8)(%rsp), DST;

	PLUS(X0, X10);
	PLUS(X1, X11);
	PLUS(X2, X12);
	PLUS(X3, X13);

	/* Update counter */
	paddq X4, X13;

	PLUS(X8, X10);
	PLUS(X9, X11);
	PLUS(X14, X12);
	PLUS(X15, X13);

	/* Update counter */
	paddq X4, X13;

	xor_src_dst(DST, SRC, 0 * 4, X0, X7);
	xor_src_dst(DST, SRC, 4 * 4, X1, X7);
	xor_src_dst(DST, SRC, 8 * 4, X2, X7);
	xor_src_dst(DST, SRC, 12 * 4, X3, X7);
	xor_src_dst(DST, SRC, 16 * 4, X8, X7);
	xor_src_dst(DST, SRC, 20 * 4, X9, X7);
	xor_src_dst(DST, SRC, 24 * 4, X14, X7);
	xor_src_dst(DST, SRC, 28 * 4, X15, X7);

	clear(X8);
	clear(X9);
	clear(X14);
	clear(X15);

	subq $2, (7 * 8)(%rsp); # NBLKS
	lea (2 * 64)(SRC), SRC;
	lea (2 * 64)(DST), DST;
	movq SRC, (5 * 8)(%rsp);
	movq DST, (6 * 8)(%rsp);
	jz .Ldone_poly1;

.Loop_poly1:
	movdqa X10, X0;
	movdqa X11, X1;
	movdqa X12, X2;
	movdqa X13, X3;

	/* Process one ChaCha20 block and four Poly1305 blocks. */

	movl $20, (8 * 8 + 4)(%rsp);
.Lround1_with_poly1305_outer:
	movl $8, (8 * 8)(%rsp);
.Lround1_with_poly1305_inner:
	POLY1305_BLOCK_PART1(0 * 16);
	  QUARTERROUND4(X0, X1, X2, X3, X5, X6, X7, 0x39, 0x4e, 0x93);
	POLY1305_BLOCK_PART2();
	  QUARTERROUND4(X0, X1, X2, X3, X5, X6, X7, 0x93, 0x4e, 0x39);
	lea (1 * 16)(POLY_RSRC), POLY_RSRC;

	POLY1305_BLOCK_PART3();
	  QUARTERROUND4(X0, X1, X2, X3, X5, X6, X7, 0x39, 0x4e, 0x93);
	POLY1305_BLOCK_PART4();
	  QUARTERROUND4(X0, X1, X2, X3, X5, X6, X7, 0x93, 0x4e, 0x39);
	POLY1305_BLOCK_PART5();

	subl $4, (8 * 8)(%rsp);
	jnz .Lround1_with_poly1305_inner;

	  QUARTERROUND4(X0, X1, X2, X3, X5, X6, X7, 0x39, 0x4e, 0x93);
	  QUARTERROUND4(X0, X1, X2, X3, X5, X6, X7, 0x93, 0x4e, 0x39);

	subl $10, (8 * 8 + 4)(%rsp);
	jnz .Lround1_with_poly1305_outer;

	movq (5 * 8)(%rsp), SRC;
	movq (6 * 8)(%rsp), DST;

	PLUS(X0, X10);
	PLUS(X1, X11);
	PLUS(X2, X12);
	PLUS(X3, X13);

	/* Update counter */
	paddq X4, X13;

	xor_src_dst(DST, SRC, 0 * 4, X0, X7);
	xor_src_dst(DST, SRC, 4 * 4, X1, X7);
	xor_src_dst(DST, SRC, 8 * 4, X2, X7);
	xor_src_dst(DST, SRC, 12 * 4, X3, X7);

	subq $1, (7 * 8)(%rsp); # NBLKS
	lea (64)(SRC), SRC;
	lea (64)(DST), DST;
	movq SRC, (5 * 8)(%rsp);
	movq DST, (6 * 8)(%rsp);

	jnz .Loop_poly1;

.Ldone_poly1:
	/* Store state */
	POLY1305_STORE_STATE();

	movdqu X13, (12 * 4)(INPUT);

	/* clear the used vector registers */
	clear(X0);
	clear(X1);
	clear(X2);
	clear(X3);
	clear(X4);
	clear(X5);
	clear(X6);
	clear(X7);
	clear(X10);
	clear(X11);
	clear(X12);
	clear(X13);

	movq (0 * 8)(%rsp), %rbx;
	movq (1 * 8)(%rsp), %r12;
	movq (2 * 8)(%rsp), %r13;
	movq (3 * 8)(%rsp), %r14;
	movq (4 * 8)(%rsp), %r15;
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);
	CFI_RESTORE(%r14);
	CFI_RESTORE(%r15);

	xorl %eax, %eax;
	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_chacha20_poly1305_amd64_ssse3_blocks1,
	  .-_gcry_chacha20_poly1305_amd64_ssse3_blocks1;)

#endif /*defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS)*/
#endif /*__x86_64*/
