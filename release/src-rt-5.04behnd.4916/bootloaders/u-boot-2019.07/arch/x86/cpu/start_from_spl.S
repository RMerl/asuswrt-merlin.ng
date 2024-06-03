/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * 32-bit x86 Startup Code when running from SPL
 *
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <config.h>

.section .text.start
.code32
.globl _start
.type _start, @function
_start:
	/* Set up memory using the existing stack */
	movl	$(CONFIG_SYS_CAR_ADDR + CONFIG_SYS_CAR_SIZE - 4), %eax
#ifdef CONFIG_DCACHE_RAM_MRC_VAR_SIZE
	subl	$CONFIG_DCACHE_RAM_MRC_VAR_SIZE, %eax
#endif
	/*
	 * We don't subject CONFIG_DCACHE_RAM_MRC_VAR_SIZE since memory is
	 * already set up. This has the happy side-effect of putting gd in a
	 * new place separate from SPL, so the memset() in
	 * board_init_f_init_reserve() does not cause any problems (otherwise
	 * it would zero out the gd and crash)
	 */
	call	board_init_f_alloc_reserve
	mov	%eax, %esp

	call	board_init_f_init_reserve

	xorl	%eax, %eax
	call	board_init_f
	call	board_init_f_r

	/* Should not return here */
	jmp	.

.globl board_init_f_r_trampoline
.type board_init_f_r_trampoline, @function
board_init_f_r_trampoline:
	/*
	 * SPL has been executed and SDRAM has been initialised, U-Boot code
	 * has been copied into RAM, BSS has been cleared and relocation
	 * adjustments have been made. It is now time to jump into the in-RAM
	 * copy of U-Boot
	 *
	 * %eax = Address of top of new stack
	 */

	/* Stack grows down from top of SDRAM */
	movl	%eax, %esp

	/* Re-enter U-Boot by calling board_init_f_r() */
	call	board_init_f_r

die:
	hlt
	jmp	die
	hlt

	.align 4
_dt_ucode_base_size:
	/* These next two fields are filled in by binman */
.globl ucode_base
ucode_base:	/* Declared in microcode.h */
	.long	0			/* microcode base */
.globl ucode_size
ucode_size:	/* Declared in microcode.h */
	.long	0			/* microcode size */
