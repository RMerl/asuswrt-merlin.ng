/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000, 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
 *  base on code by
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <config.h>

/*
 * unsigned long long get_ticks(void);
 *
 * read timebase as "long long"
 */
	.globl	get_ticks
get_ticks:
1:	mftbu	r3
	mftb	r4
	mftbu	r5
	cmp	0,r3,r5
	bne	1b
	blr

/*
 * Delay for a number of ticks
 */
	.globl	wait_ticks
wait_ticks:
	stwu	r1, -16(r1)
	mflr	r0		/* save link register */
	stw	r0, 20(r1)	/* Use r0 or GDB will be unhappy */
	stw	r14, 12(r1)	/* save used registers */
	stw	r15, 8(r1)
	mr	r14, r3		/* save tick count */
	bl	get_ticks	/* Get start time */

	/* Calculate end time */
	addc    r14, r4, r14	/* Compute end time lower */
	addze	r15, r3		/*     and end time upper */

#if defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
	bl	schedule	/* Trigger watchdog, if needed */
#endif
1:	bl	get_ticks	/* Get current time */
	subfc	r4, r4, r14	/* Subtract current time from end time */
	subfe.	r3, r3, r15
	bge	1b		/* Loop until time expired */

	lwz     r15, 8(r1)	/* restore saved registers */
	lwz     r14, 12(r1)
	lwz     r0, 20(r1)
	addi	r1,r1,16
	mtlr	r0
	blr
