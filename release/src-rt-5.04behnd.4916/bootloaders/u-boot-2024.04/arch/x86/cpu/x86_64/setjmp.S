/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 Intel Corporation
 *
 * See arch/x86/include/asm/setjmp.h for jmp_buf format
 */

#include <linux/linkage.h>

.text
.align 8

ENTRY(setjmp)

	pop	%rcx
	movq	%rcx, (%rdi)	/* Return address */
	movq	%rsp, 8(%rdi)
	movq	%rbp, 16(%rdi)
	movq	%rbx, 24(%rdi)
	movq	%r12, 32(%rdi)
	movq	%r13, 40(%rdi)
	movq	%r14, 48(%rdi)
	movq	%r15, 56(%rdi)
	xorq	%rax, %rax	/* Direct invocation returns 0 */
	jmpq	*%rcx

ENDPROC(setjmp)

.align 8

ENTRY(longjmp)

	movq	(%rdi), %rcx	/* Return address */
	movq	8(%rdi), %rsp
	movq	16(%rdi), %rbp
	movq	24(%rdi), %rbx
	movq	32(%rdi), %r12
	movq	40(%rdi), %r13
	movq	48(%rdi), %r14
	movq	56(%rdi), %r15

	movq	%rsi, %rax	/* Value to be returned by setjmp() */
	testq	%rax, %rax	/* cannot be 0 in this case */
	jnz	1f
	incq	%rax		/* Return 1 instead */
1:
	jmpq	*%rcx

ENDPROC(longjmp)
