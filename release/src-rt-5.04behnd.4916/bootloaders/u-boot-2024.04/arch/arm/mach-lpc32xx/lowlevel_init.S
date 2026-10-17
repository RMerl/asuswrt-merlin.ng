/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * WORK Microwave work_92105 board low level init
 *
 * (C) Copyright 2014  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 *
 * Low level init is called from SPL to set up the clocks.
 * On entry, the LPC3250 is in Direct Run mode with all clocks
 * running at 13 MHz; on exit, ARM clock is 208 MHz, HCLK is
 * 104 MHz and PCLK is 13 MHz.
 *
 * This code must run from SRAM so that the clock changes do
 * not prevent it from executing.
 */

.globl lowlevel_init

lowlevel_init:

	/* Set ARM, HCLK, PCLK dividers for normal mode */
	ldr	r0, =0x0000003D
	ldr	r1, =0x40004040
	str	r0, [r1]

	/* Start HCLK PLL for 208 MHz */
	ldr	r0, =0x0001401E
	ldr	r1, =0x40004058
	str	r0, [r1]

	/* wait for HCLK PLL to lock */
1:
	ldr	r0, [r1]
	ands	r0, r0, #1
	beq	1b

	/* switch to normal mode */
	ldr	r1, =0x40004044
	ldr	r0, [r1]
	orr	r0, #0x00000004
	str	r0, [r1]

	/* Return to U-Boot via saved link register */
	mov	pc, lr
