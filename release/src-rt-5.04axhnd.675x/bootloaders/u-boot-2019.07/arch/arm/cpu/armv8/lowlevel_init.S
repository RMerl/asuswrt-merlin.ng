/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * A lowlevel_init function that sets up the stack to call a C function to
 * perform further init.
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)
	/*
	 * Setup a temporary stack. Global data is not available yet.
	 */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)
	ldr	w0, =CONFIG_SPL_STACK
#else
	ldr	w0, =CONFIG_SYS_INIT_SP_ADDR
#endif
	bic	sp, x0, #0xf	/* 16-byte alignment for ABI compliance */

	/*
	 * Save the old LR(passed in x29) and the current LR to stack
	 */
	stp	x29, x30, [sp, #-16]!

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
	ldp	x29, x30, [sp]
	ret
ENDPROC(lowlevel_init)
