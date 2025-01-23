/* asm-common-amd64.h  -  Poly1305 macros for AMD64 assembly
 *
 * Copyright (C) 2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#ifndef GCRY_ASM_POLY1305_AMD64_H
#define GCRY_ASM_POLY1305_AMD64_H

#include "asm-common-amd64.h"

/**********************************************************************
  poly1305 for stitched chacha20-poly1305 AMD64 implementations
 **********************************************************************/

#define POLY_RSTATE    %r8
#define POLY_RSRC      %r9

#define POLY_R_H0      %rbx
#define POLY_R_H1      %rcx
#define POLY_R_H2      %r10
#define POLY_R_H2d     %r10d
#define POLY_R_R0      %r11
#define POLY_R_R1_MUL5 %r12
#define POLY_R_X0_HI   %r13
#define POLY_R_X0_LO   %r14
#define POLY_R_X1_HI   %r15
#define POLY_R_X1_LO   %rsi

#define POLY_S_R0      (4 * 4 + 0 * 8)(POLY_RSTATE)
#define POLY_S_R1      (4 * 4 + 1 * 8)(POLY_RSTATE)
#define POLY_S_H0      (4 * 4 + 2 * 8 + 0 * 8)(POLY_RSTATE)
#define POLY_S_H1      (4 * 4 + 2 * 8 + 1 * 8)(POLY_RSTATE)
#define POLY_S_H2d     (4 * 4 + 2 * 8 + 2 * 8)(POLY_RSTATE)

#define POLY1305_LOAD_STATE() \
	movq POLY_S_H0, POLY_R_H0; \
	movq POLY_S_H1, POLY_R_H1; \
	movl POLY_S_H2d, POLY_R_H2d; \
	movq POLY_S_R0, POLY_R_R0; \
	movq POLY_S_R1, POLY_R_R1_MUL5; \
	shrq $2, POLY_R_R1_MUL5; \
	addq POLY_S_R1, POLY_R_R1_MUL5;

#define POLY1305_STORE_STATE() \
	movq POLY_R_H0, POLY_S_H0; \
	movq POLY_R_H1, POLY_S_H1; \
	movl POLY_R_H2d, POLY_S_H2d;

/* a = h + m */
#define POLY1305_BLOCK_PART1(src_offset) \
	addq ((src_offset) + 0 * 8)(POLY_RSRC), POLY_R_H0; \
	adcq ((src_offset) + 1 * 8)(POLY_RSRC), POLY_R_H1; \
	adcl $1, POLY_R_H2d; \
	\
	/* h = a * r (partial mod 2^130-5): */ \
	\
	/* h0 * r1 */ \
	movq POLY_R_H0, %rax; \
	mulq POLY_S_R1; \
	movq %rax, POLY_R_X1_LO; \
	movq %rdx, POLY_R_X1_HI;

#define POLY1305_BLOCK_PART2() \
	\
	/* h0 * r0 */ \
	movq POLY_R_H0, %rax; \
	mulq POLY_R_R0; \
	movq %rax, POLY_R_X0_LO; \
	movq %rdx, POLY_R_X0_HI;

#define POLY1305_BLOCK_PART3() \
	\
	/* h1 * r0 */ \
	movq POLY_R_H1, %rax; \
	mulq POLY_R_R0; \
	addq %rax, POLY_R_X1_LO; \
	adcq %rdx, POLY_R_X1_HI; \
	\
	/* h1 * r1 mod 2^130-5 */ \
	movq POLY_R_R1_MUL5, %rax; \
	mulq POLY_R_H1;

#define POLY1305_BLOCK_PART4() \
	movq POLY_R_H2, POLY_R_H1; \
	imulq POLY_R_R1_MUL5, POLY_R_H1; /* h2 * r1 mod 2^130-5 */ \
	addq %rax, POLY_R_X0_LO; \
	adcq %rdx, POLY_R_X0_HI; \
	imulq POLY_R_R0, POLY_R_H2;      /* h2 * r0 */ \
	addq POLY_R_X1_LO, POLY_R_H1; \
	adcq POLY_R_X1_HI, POLY_R_H2;

#define POLY1305_BLOCK_PART5() \
	\
	/* carry propagation */ \
	movq POLY_R_H2, POLY_R_H0; \
	andl $3, POLY_R_H2d; \
	shrq $2, POLY_R_H0; \
	leaq (POLY_R_H0, POLY_R_H0, 4), POLY_R_H0; \
	addq POLY_R_X0_LO, POLY_R_H0; \
	adcq POLY_R_X0_HI, POLY_R_H1; \
	adcl $0, POLY_R_H2d;

#ifdef TESTING_POLY1305_ASM
/* for testing only, mixed C/asm poly1305.c is marginally faster (~2%). */
.align 8
.globl _gcry_poly1305_amd64_ssse3_blocks1
ELF(.type _gcry_poly1305_amd64_ssse3_blocks1,@function;)

_gcry_poly1305_amd64_ssse3_blocks1:
	/* input:
	 *	%rdi: poly1305-state
	 *	%rsi: src
	 *	%rdx: nblks
	 */
	pushq %rbp;
	movq %rsp, %rbp;

	subq $(10 * 8), %rsp;
	movq %rbx, (1 * 8)(%rsp);
	movq %r12, (2 * 8)(%rsp);
	movq %r13, (3 * 8)(%rsp);
	movq %r14, (4 * 8)(%rsp);
	movq %r15, (5 * 8)(%rsp);

	movq %rdx, (8 * 8)(%rsp); # NBLKS

	movq %rdi, POLY_RSTATE;
	movq %rsi, POLY_RSRC;

	POLY1305_LOAD_STATE();

.L_poly1:
	POLY1305_BLOCK_PART1(0 * 16);
	POLY1305_BLOCK_PART2();
	POLY1305_BLOCK_PART3();
	POLY1305_BLOCK_PART4();
	POLY1305_BLOCK_PART5();

	subq $1, (8 * 8)(%rsp); # NBLKS
	leaq (16)(POLY_RSRC), POLY_RSRC;
	jnz .L_poly1;

	POLY1305_STORE_STATE();

	movq (1 * 8)(%rsp), %rbx;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	movq (4 * 8)(%rsp), %r14;
	movq (5 * 8)(%rsp), %r15;

	xorl %eax, %eax;
	leave
	ret;
#endif

#endif /* GCRY_ASM_POLY1305_AMD64_H */
