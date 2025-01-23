/* chacha20-amd64-avx2.S  -  AVX2 implementation of ChaCha20 cipher
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
#if defined(HAVE_GCC_INLINE_ASM_AVX2) && \
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
#define STACK_VEC_X12 (32)
#define STACK_VEC_X13 (32 + STACK_VEC_X12)
#define STACK_TMP     (32 + STACK_VEC_X13)
#define STACK_TMP1    (32 + STACK_TMP)

#define STACK_MAX     (32 + STACK_TMP1)

/* vector registers */
#define X0 %ymm0
#define X1 %ymm1
#define X2 %ymm2
#define X3 %ymm3
#define X4 %ymm4
#define X5 %ymm5
#define X6 %ymm6
#define X7 %ymm7
#define X8 %ymm8
#define X9 %ymm9
#define X10 %ymm10
#define X11 %ymm11
#define X12 %ymm12
#define X13 %ymm13
#define X14 %ymm14
#define X15 %ymm15

#define X0h %xmm0
#define X1h %xmm1
#define X2h %xmm2
#define X3h %xmm3
#define X4h %xmm4
#define X5h %xmm5
#define X6h %xmm6
#define X7h %xmm7
#define X8h %xmm8
#define X9h %xmm9
#define X10h %xmm10
#define X11h %xmm11
#define X12h %xmm12
#define X13h %xmm13
#define X14h %xmm14
#define X15h %xmm15

/**********************************************************************
  helper macros
 **********************************************************************/

/* 4x4 32-bit integer matrix transpose */
#define transpose_4x4(x0,x1,x2,x3,t1,t2) \
	vpunpckhdq x1, x0, t2; \
	vpunpckldq x1, x0, x0; \
	\
	vpunpckldq x3, x2, t1; \
	vpunpckhdq x3, x2, x2; \
	\
	vpunpckhqdq t1, x0, x1; \
	vpunpcklqdq t1, x0, x0; \
	\
	vpunpckhqdq x2, t2, x3; \
	vpunpcklqdq x2, t2, x2;

/* 2x2 128-bit matrix transpose */
#define transpose_16byte_2x2(x0,x1,t1) \
	vmovdqa    x0, t1; \
	vperm2i128 $0x20, x1, x0, x0; \
	vperm2i128 $0x31, x1, t1, x1;

/* xor register with unaligned src and save to unaligned dst */
#define xor_src_dst(dst, src, offset, xreg) \
	vpxor offset(src), xreg, xreg; \
	vmovdqu xreg, offset(dst);

/**********************************************************************
  8-way chacha20
 **********************************************************************/

#define ROTATE2(v1,v2,c,tmp)	\
	vpsrld $(32 - (c)), v1, tmp;	\
	vpslld $(c), v1, v1;		\
	vpaddb tmp, v1, v1;		\
	vpsrld $(32 - (c)), v2, tmp;	\
	vpslld $(c), v2, v2;		\
	vpaddb tmp, v2, v2;

#define ROTATE_SHUF_2(v1,v2,shuf)	\
	vpshufb shuf, v1, v1;		\
	vpshufb shuf, v2, v2;

#define XOR(ds,s) \
	vpxor s, ds, ds;

#define PLUS(ds,s) \
	vpaddd s, ds, ds;

#define QUARTERROUND2(a1,b1,c1,d1,a2,b2,c2,d2,ign,tmp1,\
		      interleave_op1,interleave_op2,\
		      interleave_op3,interleave_op4)		\
	vbroadcasti128 .Lshuf_rol16 rRIP, tmp1;			\
		interleave_op1;					\
	PLUS(a1,b1); PLUS(a2,b2); XOR(d1,a1); XOR(d2,a2);	\
	    ROTATE_SHUF_2(d1, d2, tmp1);			\
		interleave_op2;					\
	PLUS(c1,d1); PLUS(c2,d2); XOR(b1,c1); XOR(b2,c2);	\
	    ROTATE2(b1, b2, 12, tmp1);				\
	vbroadcasti128 .Lshuf_rol8 rRIP, tmp1;			\
		interleave_op3;					\
	PLUS(a1,b1); PLUS(a2,b2); XOR(d1,a1); XOR(d2,a2);	\
	    ROTATE_SHUF_2(d1, d2, tmp1);			\
		interleave_op4;					\
	PLUS(c1,d1); PLUS(c2,d2); XOR(b1,c1); XOR(b2,c2);	\
	    ROTATE2(b1, b2,  7, tmp1);

.align 32
chacha20_data:
.Lshuf_rol16:
	.byte 2,3,0,1,6,7,4,5,10,11,8,9,14,15,12,13
.Lshuf_rol8:
	.byte 3,0,1,2,7,4,5,6,11,8,9,10,15,12,13,14
.Linc_counter:
	.byte 0,1,2,3,4,5,6,7
.Lunsigned_cmp:
	.long 0x80000000

.align 8
.globl _gcry_chacha20_amd64_avx2_blocks8
ELF(.type _gcry_chacha20_amd64_avx2_blocks8,@function;)

_gcry_chacha20_amd64_avx2_blocks8:
	/* input:
	 *	%rdi: input
	 *	%rsi: dst
	 *	%rdx: src
	 *	%rcx: nblks (multiple of 8)
	 */
	CFI_STARTPROC();

	vzeroupper;

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	subq $STACK_MAX, %rsp;
	andq $~31, %rsp;

.Loop8:
	mov $20, ROUND;

	/* Construct counter vectors X12 and X13 */
	vpmovzxbd .Linc_counter rRIP, X0;
	vpbroadcastd .Lunsigned_cmp rRIP, X2;
	vpbroadcastd (12 * 4)(INPUT), X12;
	vpbroadcastd (13 * 4)(INPUT), X13;
	vpaddd X0, X12, X12;
	vpxor X2, X0, X0;
	vpxor X2, X12, X1;
	vpcmpgtd X1, X0, X0;
	vpsubd X0, X13, X13;
	vmovdqa X12, (STACK_VEC_X12)(%rsp);
	vmovdqa X13, (STACK_VEC_X13)(%rsp);

	/* Load vectors */
	vpbroadcastd (0 * 4)(INPUT), X0;
	vpbroadcastd (1 * 4)(INPUT), X1;
	vpbroadcastd (2 * 4)(INPUT), X2;
	vpbroadcastd (3 * 4)(INPUT), X3;
	vpbroadcastd (4 * 4)(INPUT), X4;
	vpbroadcastd (5 * 4)(INPUT), X5;
	vpbroadcastd (6 * 4)(INPUT), X6;
	vpbroadcastd (7 * 4)(INPUT), X7;
	vpbroadcastd (8 * 4)(INPUT), X8;
	vpbroadcastd (9 * 4)(INPUT), X9;
	vpbroadcastd (10 * 4)(INPUT), X10;
	vpbroadcastd (11 * 4)(INPUT), X11;
	vpbroadcastd (14 * 4)(INPUT), X14;
	vpbroadcastd (15 * 4)(INPUT), X15;
	vmovdqa X15, (STACK_TMP)(%rsp);

.Lround2:
	QUARTERROUND2(X0, X4,  X8, X12,   X1, X5,  X9, X13, tmp:=,X15,,,,)
	vmovdqa (STACK_TMP)(%rsp), X15;
	vmovdqa X8, (STACK_TMP)(%rsp);
	QUARTERROUND2(X2, X6, X10, X14,   X3, X7, X11, X15, tmp:=,X8,,,,)
	QUARTERROUND2(X0, X5, X10, X15,   X1, X6, X11, X12, tmp:=,X8,,,,)
	vmovdqa (STACK_TMP)(%rsp), X8;
	vmovdqa X15, (STACK_TMP)(%rsp);
	QUARTERROUND2(X2, X7,  X8, X13,   X3, X4,  X9, X14, tmp:=,X15,,,,)
	sub $2, ROUND;
	jnz .Lround2;

	vmovdqa X8, (STACK_TMP1)(%rsp);

	/* tmp := X15 */
	vpbroadcastd (0 * 4)(INPUT), X15;
	PLUS(X0, X15);
	vpbroadcastd (1 * 4)(INPUT), X15;
	PLUS(X1, X15);
	vpbroadcastd (2 * 4)(INPUT), X15;
	PLUS(X2, X15);
	vpbroadcastd (3 * 4)(INPUT), X15;
	PLUS(X3, X15);
	vpbroadcastd (4 * 4)(INPUT), X15;
	PLUS(X4, X15);
	vpbroadcastd (5 * 4)(INPUT), X15;
	PLUS(X5, X15);
	vpbroadcastd (6 * 4)(INPUT), X15;
	PLUS(X6, X15);
	vpbroadcastd (7 * 4)(INPUT), X15;
	PLUS(X7, X15);
	transpose_4x4(X0, X1, X2, X3, X8, X15);
	transpose_4x4(X4, X5, X6, X7, X8, X15);
	vmovdqa (STACK_TMP1)(%rsp), X8;
	transpose_16byte_2x2(X0, X4, X15);
	transpose_16byte_2x2(X1, X5, X15);
	transpose_16byte_2x2(X2, X6, X15);
	transpose_16byte_2x2(X3, X7, X15);
	vmovdqa (STACK_TMP)(%rsp), X15;
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 0), X0);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 0), X1);
	vpbroadcastd (8 * 4)(INPUT), X0;
	PLUS(X8, X0);
	vpbroadcastd (9 * 4)(INPUT), X0;
	PLUS(X9, X0);
	vpbroadcastd (10 * 4)(INPUT), X0;
	PLUS(X10, X0);
	vpbroadcastd (11 * 4)(INPUT), X0;
	PLUS(X11, X0);
	vmovdqa (STACK_VEC_X12)(%rsp), X0;
	PLUS(X12, X0);
	vmovdqa (STACK_VEC_X13)(%rsp), X0;
	PLUS(X13, X0);
	vpbroadcastd (14 * 4)(INPUT), X0;
	PLUS(X14, X0);
	vpbroadcastd (15 * 4)(INPUT), X0;
	PLUS(X15, X0);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 0), X2);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 0), X3);

	/* Update counter */
	addq $8, (12 * 4)(INPUT);

	transpose_4x4(X8, X9, X10, X11, X0, X1);
	transpose_4x4(X12, X13, X14, X15, X0, X1);
	xor_src_dst(DST, SRC, (64 * 4 + 16 * 0), X4);
	xor_src_dst(DST, SRC, (64 * 5 + 16 * 0), X5);
	transpose_16byte_2x2(X8, X12, X0);
	transpose_16byte_2x2(X9, X13, X0);
	transpose_16byte_2x2(X10, X14, X0);
	transpose_16byte_2x2(X11, X15, X0);
	xor_src_dst(DST, SRC, (64 * 6 + 16 * 0), X6);
	xor_src_dst(DST, SRC, (64 * 7 + 16 * 0), X7);
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 2), X8);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 2), X9);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 2), X10);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 2), X11);
	xor_src_dst(DST, SRC, (64 * 4 + 16 * 2), X12);
	xor_src_dst(DST, SRC, (64 * 5 + 16 * 2), X13);
	xor_src_dst(DST, SRC, (64 * 6 + 16 * 2), X14);
	xor_src_dst(DST, SRC, (64 * 7 + 16 * 2), X15);

	sub $8, NBLKS;
	lea (8 * 64)(DST), DST;
	lea (8 * 64)(SRC), SRC;
	jnz .Loop8;

	/* clear the used vector registers and stack */
	vpxor X0, X0, X0;
	vmovdqa X0, (STACK_VEC_X12)(%rsp);
	vmovdqa X0, (STACK_VEC_X13)(%rsp);
	vmovdqa X0, (STACK_TMP)(%rsp);
	vmovdqa X0, (STACK_TMP1)(%rsp);
	vzeroall;

	/* eax zeroed by round loop. */
	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_chacha20_amd64_avx2_blocks8,
	  .-_gcry_chacha20_amd64_avx2_blocks8;)

/**********************************************************************
  8-way stitched chacha20-poly1305
 **********************************************************************/

#define _ /*_*/

.align 8
.globl _gcry_chacha20_poly1305_amd64_avx2_blocks8
ELF(.type _gcry_chacha20_poly1305_amd64_avx2_blocks8,@function;)

_gcry_chacha20_poly1305_amd64_avx2_blocks8:
	/* input:
	 *	%rdi: input
	 *	%rsi: dst
	 *	%rdx: src
	 *	%rcx: nblks (multiple of 8)
	 *	%r9: poly1305-state
	 *	%r8: poly1305-src
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	vzeroupper;

	subq $(9 * 8) + STACK_MAX + 32, %rsp;
	andq $~31, %rsp;

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

.Loop_poly8:

	/* Construct counter vectors X12 and X13 */
	vpmovzxbd .Linc_counter rRIP, X0;
	vpbroadcastd .Lunsigned_cmp rRIP, X2;
	vpbroadcastd (12 * 4)(INPUT), X12;
	vpbroadcastd (13 * 4)(INPUT), X13;
	vpaddd X0, X12, X12;
	vpxor X2, X0, X0;
	vpxor X2, X12, X1;
	vpcmpgtd X1, X0, X0;
	vpsubd X0, X13, X13;
	vmovdqa X12, (STACK_VEC_X12)(%rsp);
	vmovdqa X13, (STACK_VEC_X13)(%rsp);

	/* Load vectors */
	vpbroadcastd (0 * 4)(INPUT), X0;
	vpbroadcastd (1 * 4)(INPUT), X1;
	vpbroadcastd (2 * 4)(INPUT), X2;
	vpbroadcastd (3 * 4)(INPUT), X3;
	vpbroadcastd (4 * 4)(INPUT), X4;
	vpbroadcastd (5 * 4)(INPUT), X5;
	vpbroadcastd (6 * 4)(INPUT), X6;
	vpbroadcastd (7 * 4)(INPUT), X7;
	vpbroadcastd (8 * 4)(INPUT), X8;
	vpbroadcastd (9 * 4)(INPUT), X9;
	vpbroadcastd (10 * 4)(INPUT), X10;
	vpbroadcastd (11 * 4)(INPUT), X11;
	vpbroadcastd (14 * 4)(INPUT), X14;
	vpbroadcastd (15 * 4)(INPUT), X15;
	vmovdqa X15, (STACK_TMP)(%rsp);

	/* Process eight ChaCha20 blocks and 32 Poly1305 blocks. */

	movl $20, (STACK_MAX + 8 * 8 + 4)(%rsp);
.Lround8_with_poly1305_outer:
	movl $6, (STACK_MAX + 8 * 8)(%rsp);
.Lround8_with_poly1305_inner1:
	/* rounds 0-5 & 10-15 */
		      POLY1305_BLOCK_PART1(0 * 16)
	QUARTERROUND2(X0, X4,  X8, X12,   X1, X5,  X9, X13, tmp:=,X15,
		      POLY1305_BLOCK_PART2(),
		      POLY1305_BLOCK_PART3(),
		      POLY1305_BLOCK_PART4(),
		      POLY1305_BLOCK_PART5())
	vmovdqa (STACK_TMP)(%rsp), X15;
	vmovdqa X8, (STACK_TMP)(%rsp);
		      POLY1305_BLOCK_PART1(1 * 16)
	QUARTERROUND2(X2, X6, X10, X14,   X3, X7, X11, X15, tmp:=,X8,
		      POLY1305_BLOCK_PART2(),
		      POLY1305_BLOCK_PART3(),
		      POLY1305_BLOCK_PART4(),
		      POLY1305_BLOCK_PART5())
		      POLY1305_BLOCK_PART1(2 * 16)
	QUARTERROUND2(X0, X5, X10, X15,   X1, X6, X11, X12, tmp:=,X8,
		      POLY1305_BLOCK_PART2(),
		      POLY1305_BLOCK_PART3(),
		      POLY1305_BLOCK_PART4(),
		      POLY1305_BLOCK_PART5())
	vmovdqa (STACK_TMP)(%rsp), X8;
	vmovdqa X15, (STACK_TMP)(%rsp);
		      POLY1305_BLOCK_PART1(3 * 16)
		      lea (4 * 16)(POLY_RSRC), POLY_RSRC;
	QUARTERROUND2(X2, X7,  X8, X13,   X3, X4,  X9, X14, tmp:=,X15,
		      POLY1305_BLOCK_PART2(),
		      POLY1305_BLOCK_PART3(),
		      POLY1305_BLOCK_PART4(),
		      POLY1305_BLOCK_PART5())

	subl $2, (STACK_MAX + 8 * 8)(%rsp);
	jnz .Lround8_with_poly1305_inner1;

	movl $4, (STACK_MAX + 8 * 8)(%rsp);
.Lround8_with_poly1305_inner2:
	/* rounds 6-9 & 16-19 */
		      POLY1305_BLOCK_PART1(0 * 16)
	QUARTERROUND2(X0, X4,  X8, X12,   X1, X5,  X9, X13, tmp:=,X15,
		      POLY1305_BLOCK_PART2(),
		      _,
		      POLY1305_BLOCK_PART3(),
		      _)
	vmovdqa (STACK_TMP)(%rsp), X15;
	vmovdqa X8, (STACK_TMP)(%rsp);
	QUARTERROUND2(X2, X6, X10, X14,   X3, X7, X11, X15, tmp:=,X8,
		      _,
		      POLY1305_BLOCK_PART4(),
		      _,
		      POLY1305_BLOCK_PART5())
		      POLY1305_BLOCK_PART1(1 * 16);
		      lea (2 * 16)(POLY_RSRC), POLY_RSRC;
	QUARTERROUND2(X0, X5, X10, X15,   X1, X6, X11, X12, tmp:=,X8,
		      _,
		      POLY1305_BLOCK_PART2(),
		      _,
		      POLY1305_BLOCK_PART3())
	vmovdqa (STACK_TMP)(%rsp), X8;
	vmovdqa X15, (STACK_TMP)(%rsp);
	QUARTERROUND2(X2, X7,  X8, X13,   X3, X4,  X9, X14, tmp:=,X15,
		      POLY1305_BLOCK_PART4(),
		      _,
		      POLY1305_BLOCK_PART5(),
		      _)

	subl $2, (STACK_MAX + 8 * 8)(%rsp);
	jnz .Lround8_with_poly1305_inner2;

	subl $10, (STACK_MAX + 8 * 8 + 4)(%rsp);
	jnz .Lround8_with_poly1305_outer;

	movq (STACK_MAX + 5 * 8)(%rsp), SRC;
	movq (STACK_MAX + 6 * 8)(%rsp), DST;

	vmovdqa X8, (STACK_TMP1)(%rsp);

	/* tmp := X15 */
	vpbroadcastd (0 * 4)(INPUT), X15;
	PLUS(X0, X15);
	vpbroadcastd (1 * 4)(INPUT), X15;
	PLUS(X1, X15);
	vpbroadcastd (2 * 4)(INPUT), X15;
	PLUS(X2, X15);
	vpbroadcastd (3 * 4)(INPUT), X15;
	PLUS(X3, X15);
	vpbroadcastd (4 * 4)(INPUT), X15;
	PLUS(X4, X15);
	vpbroadcastd (5 * 4)(INPUT), X15;
	PLUS(X5, X15);
	vpbroadcastd (6 * 4)(INPUT), X15;
	PLUS(X6, X15);
	vpbroadcastd (7 * 4)(INPUT), X15;
	PLUS(X7, X15);
	transpose_4x4(X0, X1, X2, X3, X8, X15);
	transpose_4x4(X4, X5, X6, X7, X8, X15);
	vmovdqa (STACK_TMP1)(%rsp), X8;
	transpose_16byte_2x2(X0, X4, X15);
	transpose_16byte_2x2(X1, X5, X15);
	transpose_16byte_2x2(X2, X6, X15);
	transpose_16byte_2x2(X3, X7, X15);
	vmovdqa (STACK_TMP)(%rsp), X15;
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 0), X0);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 0), X1);
	vpbroadcastd (8 * 4)(INPUT), X0;
	PLUS(X8, X0);
	vpbroadcastd (9 * 4)(INPUT), X0;
	PLUS(X9, X0);
	vpbroadcastd (10 * 4)(INPUT), X0;
	PLUS(X10, X0);
	vpbroadcastd (11 * 4)(INPUT), X0;
	PLUS(X11, X0);
	vmovdqa (STACK_VEC_X12)(%rsp), X0;
	PLUS(X12, X0);
	vmovdqa (STACK_VEC_X13)(%rsp), X0;
	PLUS(X13, X0);
	vpbroadcastd (14 * 4)(INPUT), X0;
	PLUS(X14, X0);
	vpbroadcastd (15 * 4)(INPUT), X0;
	PLUS(X15, X0);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 0), X2);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 0), X3);

	/* Update counter */
	addq $8, (12 * 4)(INPUT);

	transpose_4x4(X8, X9, X10, X11, X0, X1);
	transpose_4x4(X12, X13, X14, X15, X0, X1);
	xor_src_dst(DST, SRC, (64 * 4 + 16 * 0), X4);
	xor_src_dst(DST, SRC, (64 * 5 + 16 * 0), X5);
	transpose_16byte_2x2(X8, X12, X0);
	transpose_16byte_2x2(X9, X13, X0);
	transpose_16byte_2x2(X10, X14, X0);
	transpose_16byte_2x2(X11, X15, X0);
	xor_src_dst(DST, SRC, (64 * 6 + 16 * 0), X6);
	xor_src_dst(DST, SRC, (64 * 7 + 16 * 0), X7);
	xor_src_dst(DST, SRC, (64 * 0 + 16 * 2), X8);
	xor_src_dst(DST, SRC, (64 * 1 + 16 * 2), X9);
	xor_src_dst(DST, SRC, (64 * 2 + 16 * 2), X10);
	xor_src_dst(DST, SRC, (64 * 3 + 16 * 2), X11);
	xor_src_dst(DST, SRC, (64 * 4 + 16 * 2), X12);
	xor_src_dst(DST, SRC, (64 * 5 + 16 * 2), X13);
	xor_src_dst(DST, SRC, (64 * 6 + 16 * 2), X14);
	xor_src_dst(DST, SRC, (64 * 7 + 16 * 2), X15);

	subq $8, (STACK_MAX + 7 * 8)(%rsp); # NBLKS

	lea (8 * 64)(DST), DST;
	lea (8 * 64)(SRC), SRC;
	movq SRC, (STACK_MAX + 5 * 8)(%rsp);
	movq DST, (STACK_MAX + 6 * 8)(%rsp);

	jnz .Loop_poly8;

	/* Store state */
	POLY1305_STORE_STATE();

	/* clear the used vector registers and stack */
	vpxor X0, X0, X0;
	vmovdqa X0, (STACK_VEC_X12)(%rsp);
	vmovdqa X0, (STACK_VEC_X13)(%rsp);
	vmovdqa X0, (STACK_TMP)(%rsp);
	vmovdqa X0, (STACK_TMP1)(%rsp);
	vzeroall;

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
ELF(.size _gcry_chacha20_poly1305_amd64_avx2_blocks8,
	  .-_gcry_chacha20_poly1305_amd64_avx2_blocks8;)

#endif /*defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS)*/
#endif /*__x86_64*/
