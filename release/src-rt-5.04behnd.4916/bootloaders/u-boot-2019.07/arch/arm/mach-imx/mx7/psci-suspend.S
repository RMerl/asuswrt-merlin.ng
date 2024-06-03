/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#include <config.h>
#include <linux/linkage.h>

#include <asm/armv7.h>
#include <asm/psci.h>

	.pushsection ._secure.text, "ax"

	.arch_extension sec

.globl v7_invalidate_l1
v7_invalidate_l1:
	mov	r0, #0
	mcr	p15, 2, r0, c0, c0, 0
	mrc	p15, 1, r0, c0, c0, 0

	movw	r1, #0x7fff
	and	r2, r1, r0, lsr #13

	movw	r1, #0x3ff

	and	r3, r1, r0, lsr #3      @ NumWays - 1
	add	r2, r2, #1              @ NumSets

	and	r0, r0, #0x7
	add	r0, r0, #4      @ SetShift

	clz	r1, r3          @ WayShift
	add	r4, r3, #1      @ NumWays
1:
	sub	r2, r2, #1      @ NumSets--
	mov	r3, r4          @ Temp = NumWays
2:
	subs	r3, r3, #1      @ Temp--
	mov	r5, r3, lsl r1
	mov	r6, r2, lsl r0
	orr	r5, r5, r6      @ Reg = (Temp<<WayShift)|(NumSets<<SetShift)
	mcr	p15, 0, r5, c7, c6, 2
	bgt	2b
	cmp	r2, #0
	bgt	1b
	dsb	st
	isb
	mov	pc, lr

.globl psci_system_resume
psci_system_resume:
	mov	sp, r0

	/* invalidate L1 I-cache first */
	mov	r6, #0x0
	mcr	p15, 0, r6, c7, c5, 0
	mcr	p15, 0, r6, c7, c5, 6
	/* enable the Icache and branch prediction */
	mov	r6, #0x1800
	mcr	p15, 0, r6, c1, c0, 0
	isb

	bl	v7_invalidate_l1
	b	imx_system_resume

	.popsection
