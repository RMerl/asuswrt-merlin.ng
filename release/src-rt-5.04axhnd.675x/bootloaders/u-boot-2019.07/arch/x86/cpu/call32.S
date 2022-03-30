/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <asm/global_data.h>
#include <asm/msr-index.h>
#include <asm/processor-flags.h>

	/*
	 * rdi - 32-bit code segment selector
	 * rsi - target address
	 * rdx - table address (0 if none)
	 */
.code64
.globl cpu_call32
cpu_call32:
	cli

	/* Save table pointer */
	mov	%edx, %ebx

	/*
	 * Debugging option, this outputs characters to the console UART
	 * mov	$0x3f8,%edx
	 * mov	$'a',%al
	 * out	%al,(%dx)
	 */

	pushf
	push	%rdi	/* 32-bit code segment */
	lea	compat(%rip), %rax
	push	%rax
	.byte	0x48	/* REX prefix to force 64-bit far return */
	retf
.code32
compat:
	/*
	 * We are now in compatibility mode with a default operand size of
	 * 32 bits. First disable paging.
	 */
	movl	%cr0, %eax
	andl	$~X86_CR0_PG, %eax
	movl	%eax, %cr0

	/* Invalidate TLB */
	xorl	%eax, %eax
	movl	%eax, %cr3

	/* Disable Long mode in EFER (Extended Feature Enable Register) */
	movl	$MSR_EFER, %ecx
	rdmsr
	btr	$_EFER_LME, %eax
	wrmsr

	/* Set up table pointer for _x86boot_start */
	mov	%ebx, %ecx

	/* Jump to the required target */
	pushl	%edi	/* 32-bit code segment */
	pushl	%esi	/* 32-bit target address */
	retf
