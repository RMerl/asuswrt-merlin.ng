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
