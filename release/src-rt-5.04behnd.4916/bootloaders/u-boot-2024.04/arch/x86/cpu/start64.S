/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * 64-bit x86 Startup Code
 *
 * Copyright 2019 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <config.h>

.section .text.start
.code64
.globl _start
.type _start, @function
_start:
	/* Set up memory using the existing stack */
	mov	%rsp, %rdi
	call	board_init_f_alloc_reserve
	mov	%rax, %rsp

	call	board_init_f_init_reserve

	xor	%rdi, %rdi
	call	board_init_f
	call	board_init_f_r

	/* Should not return here */
	jmp	.

.globl board_init_f_r_trampoline64
.type board_init_f_r_trampoline64, @function
board_init_f_r_trampoline64:
	/*
	 * SDRAM has been initialised, U-Boot code has been copied into
	 * RAM, BSS has been cleared and relocation adjustments have been
	 * made. It is now time to jump into the in-RAM copy of U-Boot
	 *
	 * %eax = Address of top of new stack
	 */

	/* Stack grows down from top of SDRAM */
	movq	%rsi, %rsp

	/* New gd is in rdi */

	/* Re-enter U-Boot by calling board_init_f_r() */
	call	board_init_f_r
