/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * A lowlevel_init function that sets up the stack to call a C function to
 * perform further init.
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <system-constants.h>

.pushsection .text.s_init, "ax"
WEAK(s_init)
	bx	lr
ENDPROC(s_init)
.popsection

.pushsection .text.lowlevel_init, "ax"
WEAK(lowlevel_init)
	/*
	 * Setup a temporary stack. Global data is not available yet.
	 */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)
	ldr	sp, =CONFIG_SPL_STACK
#else
	ldr	sp, =SYS_INIT_SP_ADDR
#endif
	bic	sp, sp, #7 /* 8-byte alignment for ABI compliance */
#ifdef CONFIG_SPL_DM
	mov	r9, #0
#else
	/*
	 * Set up global data for boards that still need it. This will be
	 * removed soon.
	 */
#ifdef CONFIG_SPL_BUILD
	ldr	r9, =gdata
#else
	sub	sp, sp, #GD_SIZE
	bic	sp, sp, #7
	mov	r9, sp
#endif
#endif
	/*
	 * Save the old lr(passed in ip) and the current lr to stack
	 */
	push	{ip, lr}

	/*
	 * Call the very early init function. This should do only the
	 * absolute bare minimum to get started. It should not:
	 *
	 * - set up DRAM
	 * - use global_data
	 * - clear BSS
	 * - try to start a console
	 *
	 * For boards with SPL this should be empty since SPL can do all of
	 * this init in the SPL board_init_f() function which is called
	 * immediately after this.
	 */
	bl	s_init
	pop	{ip, pc}
ENDPROC(lowlevel_init)
.popsection
