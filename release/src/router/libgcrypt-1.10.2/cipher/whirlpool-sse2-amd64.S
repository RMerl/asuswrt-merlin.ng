/* whirlpool-sse2-amd64.S  -  AMD64 assembly implementation of Whirlpool
 *
 * Copyright (C) 2014 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#ifdef __x86_64
#include <config.h>
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && defined(USE_WHIRLPOOL)

#include "asm-common-amd64.h"

.text

/* look-up table offsets on RTAB */
#define RC (0)
#define C0 (RC + (8 * 10))
#define C1 (C0 + (8 * 256))
#define C2 (C1 + (8 * 256))
#define C3 (C2 + (8 * 256))
#define C4 (C3 + (8 * 256))
#define C5 (C4 + (8 * 256))
#define C6 (C5 + (8 * 256))
#define C7 (C6 + (8 * 256))

/* stack variables */
#define STACK_DATAP  (0)
#define STACK_STATEP (STACK_DATAP + 8)
#define STACK_ROUNDS (STACK_STATEP + 8)
#define STACK_NBLKS  (STACK_ROUNDS + 8)
#define STACK_RBP    (STACK_NBLKS + 8)
#define STACK_RBX    (STACK_RBP + 8)
#define STACK_R12    (STACK_RBX + 8)
#define STACK_R13    (STACK_R12 + 8)
#define STACK_R14    (STACK_R13 + 8)
#define STACK_R15    (STACK_R14 + 8)
#define STACK_MAX    (STACK_R15 + 8)

/* register macros */
#define RTAB	%rbp

#define RI1	%rax
#define RI2	%rbx
#define RI3	%rcx
#define RI4	%rdx

#define RI1d	%eax
#define RI2d	%ebx
#define RI3d	%ecx
#define RI4d	%edx

#define RI1bl	%al
#define RI2bl	%bl
#define RI3bl	%cl
#define RI4bl	%dl

#define RI1bh	%ah
#define RI2bh	%bh
#define RI3bh	%ch
#define RI4bh	%dh

#define RB0	%r8
#define RB1	%r9
#define RB2	%r10
#define RB3	%r11
#define RB4	%r12
#define RB5	%r13
#define RB6	%r14
#define RB7	%r15

#define RT0	%rsi
#define RT1	%rdi

#define RT0d	%esi
#define RT1d	%edi

#define XKEY0	%xmm0
#define XKEY1	%xmm1
#define XKEY2	%xmm2
#define XKEY3	%xmm3
#define XKEY4	%xmm4
#define XKEY5	%xmm5
#define XKEY6	%xmm6
#define XKEY7	%xmm7

#define XSTATE0	%xmm8
#define XSTATE1	%xmm9
#define XSTATE2	%xmm10
#define XSTATE3	%xmm11
#define XSTATE4	%xmm12
#define XSTATE5	%xmm13
#define XSTATE6	%xmm14
#define XSTATE7	%xmm15

/***********************************************************************
 * AMD64 assembly implementation of Whirlpool.
 *  - Using table-lookups
 *  - Store state in XMM registers
 ***********************************************************************/
#define __do_whirl(op, ri, \
		   b0, b1, b2, b3, b4, b5, b6, b7, \
		   load_ri, load_arg) \
	movzbl		ri ## bl,	RT0d; \
	movzbl		ri ## bh,	RT1d; \
	shrq		$16,		ri; \
	op ## q		C7(RTAB,RT0,8),	b7; \
	op ## q		C6(RTAB,RT1,8),	b6; \
	movzbl		ri ## bl,	RT0d; \
	movzbl		ri ## bh,	RT1d; \
	shrq		$16,		ri; \
	op ## q		C5(RTAB,RT0,8),	b5; \
	op ## q		C4(RTAB,RT1,8),	b4; \
	movzbl		ri ## bl,	RT0d; \
	movzbl		ri ## bh,	RT1d; \
	shrl		$16,		ri ## d; \
	op ## q		C3(RTAB,RT0,8),	b3; \
	op ## q		C2(RTAB,RT1,8),	b2; \
	movzbl		ri ## bl,	RT0d; \
	movzbl		ri ## bh,	RT1d; \
	load_ri(	load_arg,	ri); \
	op ## q		C1(RTAB,RT0,8),	b1; \
	op ## q		C0(RTAB,RT1,8),	b0;

#define do_whirl(op, ri, rb_add, load_ri, load_arg) \
	__do_whirl(op, ##ri, rb_add, load_ri, load_arg)

#define dummy(...) /*_*/

#define do_movq(src, dst) movq src, dst;

#define RB_ADD0 RB0, RB1, RB2, RB3, RB4, RB5, RB6, RB7
#define RB_ADD1 RB1, RB2, RB3, RB4, RB5, RB6, RB7, RB0
#define RB_ADD2 RB2, RB3, RB4, RB5, RB6, RB7, RB0, RB1
#define RB_ADD3 RB3, RB4, RB5, RB6, RB7, RB0, RB1, RB2
#define RB_ADD4 RB4, RB5, RB6, RB7, RB0, RB1, RB2, RB3
#define RB_ADD5 RB5, RB6, RB7, RB0, RB1, RB2, RB3, RB4
#define RB_ADD6 RB6, RB7, RB0, RB1, RB2, RB3, RB4, RB5
#define RB_ADD7 RB7, RB0, RB1, RB2, RB3, RB4, RB5, RB6

.align 8
.globl _gcry_whirlpool_transform_amd64
ELF(.type  _gcry_whirlpool_transform_amd64,@function;)

_gcry_whirlpool_transform_amd64:
	/* input:
	 *	%rdi: state
	 *	%rsi: inblk
	 *	%rdx: nblks
	 *      %rcx: look-up tables
	 */
	CFI_STARTPROC();
	cmp $0, %rdx;
	je .Lskip;

	subq $STACK_MAX, %rsp;
	CFI_ADJUST_CFA_OFFSET(STACK_MAX);
	movq %rbp, STACK_RBP(%rsp);
	movq %rbx, STACK_RBX(%rsp);
	movq %r12, STACK_R12(%rsp);
	movq %r13, STACK_R13(%rsp);
	movq %r14, STACK_R14(%rsp);
	movq %r15, STACK_R15(%rsp);
	CFI_REL_OFFSET(%rbp, STACK_RBP);
	CFI_REL_OFFSET(%rbx, STACK_RBX);
	CFI_REL_OFFSET(%r12, STACK_R12);
	CFI_REL_OFFSET(%r13, STACK_R13);
	CFI_REL_OFFSET(%r14, STACK_R14);
	CFI_REL_OFFSET(%r15, STACK_R15);

	movq %rdx, STACK_NBLKS(%rsp);
	movq %rdi, STACK_STATEP(%rsp);
	movq %rsi, STACK_DATAP(%rsp);

	movq %rcx, RTAB;

	jmp .Lfirst_block;

.align 8
.Lblock_loop:
	movq STACK_DATAP(%rsp), %rsi;
	movq RI1, %rdi;

.Lfirst_block:
	/* load data_block */
	movq 0*8(%rsi), RB0;
	movq 1*8(%rsi), RB1;
	bswapq RB0;
	movq 2*8(%rsi), RB2;
	bswapq RB1;
	movq 3*8(%rsi), RB3;
	bswapq RB2;
	movq 4*8(%rsi), RB4;
	bswapq RB3;
	movq 5*8(%rsi), RB5;
	bswapq RB4;
	movq RB0, XSTATE0;
	movq 6*8(%rsi), RB6;
	bswapq RB5;
	movq RB1, XSTATE1;
	movq 7*8(%rsi), RB7;
	bswapq RB6;
	movq RB2, XSTATE2;
	bswapq RB7;
	movq RB3, XSTATE3;
	movq RB4, XSTATE4;
	movq RB5, XSTATE5;
	movq RB6, XSTATE6;
	movq RB7, XSTATE7;

	/* load key */
	movq 0*8(%rdi), XKEY0;
	movq 1*8(%rdi), XKEY1;
	movq 2*8(%rdi), XKEY2;
	movq 3*8(%rdi), XKEY3;
	movq 4*8(%rdi), XKEY4;
	movq 5*8(%rdi), XKEY5;
	movq 6*8(%rdi), XKEY6;
	movq 7*8(%rdi), XKEY7;

	movq XKEY0, RI1;
	movq XKEY1, RI2;
	movq XKEY2, RI3;
	movq XKEY3, RI4;

	/* prepare and store state */
	pxor XKEY0, XSTATE0;
	pxor XKEY1, XSTATE1;
	pxor XKEY2, XSTATE2;
	pxor XKEY3, XSTATE3;
	pxor XKEY4, XSTATE4;
	pxor XKEY5, XSTATE5;
	pxor XKEY6, XSTATE6;
	pxor XKEY7, XSTATE7;

	movq XSTATE0, 0*8(%rdi);
	movq XSTATE1, 1*8(%rdi);
	movq XSTATE2, 2*8(%rdi);
	movq XSTATE3, 3*8(%rdi);
	movq XSTATE4, 4*8(%rdi);
	movq XSTATE5, 5*8(%rdi);
	movq XSTATE6, 6*8(%rdi);
	movq XSTATE7, 7*8(%rdi);

	addq $64, STACK_DATAP(%rsp);
	movl $(0), STACK_ROUNDS(%rsp);
.align 8
.Lround_loop:
	do_whirl(mov, RI1 /*XKEY0*/, RB_ADD0, do_movq, XKEY4);
	do_whirl(xor, RI2 /*XKEY1*/, RB_ADD1, do_movq, XKEY5);
	do_whirl(xor, RI3 /*XKEY2*/, RB_ADD2, do_movq, XKEY6);
	do_whirl(xor, RI4 /*XKEY3*/, RB_ADD3, do_movq, XKEY7);
	do_whirl(xor, RI1 /*XKEY0*/, RB_ADD4, do_movq, XSTATE0);
	do_whirl(xor, RI2 /*XKEY1*/, RB_ADD5, do_movq, XSTATE1);
	do_whirl(xor, RI3 /*XKEY2*/, RB_ADD6, do_movq, XSTATE2);
	do_whirl(xor, RI4 /*XKEY3*/, RB_ADD7, do_movq, XSTATE3);

	movl STACK_ROUNDS(%rsp), RT0d;
	movq RB1, XKEY1;
	addl $1, STACK_ROUNDS(%rsp);
	movq RB2, XKEY2;
	movq RB3, XKEY3;
	xorq RC(RTAB,RT0,8), RB0; /* Add round constant */
	movq RB4, XKEY4;
	movq RB5, XKEY5;
	movq RB0, XKEY0;
	movq RB6, XKEY6;
	movq RB7, XKEY7;

	do_whirl(xor, RI1 /*XSTATE0*/, RB_ADD0, do_movq, XSTATE4);
	do_whirl(xor, RI2 /*XSTATE1*/, RB_ADD1, do_movq, XSTATE5);
	do_whirl(xor, RI3 /*XSTATE2*/, RB_ADD2, do_movq, XSTATE6);
	do_whirl(xor, RI4 /*XSTATE3*/, RB_ADD3, do_movq, XSTATE7);

	cmpl $10, STACK_ROUNDS(%rsp);
	je .Lis_last_round;

	do_whirl(xor, RI1 /*XSTATE4*/, RB_ADD4, do_movq, XKEY0);
	do_whirl(xor, RI2 /*XSTATE5*/, RB_ADD5, do_movq, XKEY1);
	do_whirl(xor, RI3 /*XSTATE6*/, RB_ADD6, do_movq, XKEY2);
	do_whirl(xor, RI4 /*XSTATE7*/, RB_ADD7, do_movq, XKEY3);
	movq RB0, XSTATE0;
	movq RB1, XSTATE1;
	movq RB2, XSTATE2;
	movq RB3, XSTATE3;
	movq RB4, XSTATE4;
	movq RB5, XSTATE5;
	movq RB6, XSTATE6;
	movq RB7, XSTATE7;

	jmp .Lround_loop;
.align 8
.Lis_last_round:
	do_whirl(xor, RI1 /*XSTATE4*/, RB_ADD4, dummy, _);
	movq STACK_STATEP(%rsp), RI1;
	do_whirl(xor, RI2 /*XSTATE5*/, RB_ADD5, dummy, _);
	do_whirl(xor, RI3 /*XSTATE6*/, RB_ADD6, dummy, _);
	do_whirl(xor, RI4 /*XSTATE7*/, RB_ADD7, dummy, _);

	/* store state */
	xorq RB0, 0*8(RI1);
	xorq RB1, 1*8(RI1);
	xorq RB2, 2*8(RI1);
	xorq RB3, 3*8(RI1);
	xorq RB4, 4*8(RI1);
	xorq RB5, 5*8(RI1);
	xorq RB6, 6*8(RI1);
	xorq RB7, 7*8(RI1);

	subq $1, STACK_NBLKS(%rsp);
	jnz .Lblock_loop;

	movq STACK_RBP(%rsp), %rbp;
	movq STACK_RBX(%rsp), %rbx;
	movq STACK_R12(%rsp), %r12;
	movq STACK_R13(%rsp), %r13;
	movq STACK_R14(%rsp), %r14;
	movq STACK_R15(%rsp), %r15;
	CFI_RESTORE(%rbp);
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);
	CFI_RESTORE(%r14);
	CFI_RESTORE(%r15);
	addq $STACK_MAX, %rsp;
	CFI_ADJUST_CFA_OFFSET(-STACK_MAX);
.Lskip:
	movl $(STACK_MAX + 8), %eax;
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_whirlpool_transform_amd64,.-_gcry_whirlpool_transform_amd64;)

#endif
#endif
