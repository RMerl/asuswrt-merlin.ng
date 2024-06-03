/* SPDX-License-Identifier: GPL-2.0 */
/*
 * arch/arm/cpu/armv7/rmobile/lowlevel_init_ca15.S
 *     This file is lager low level initialize.
 *
 * Copyright (C) 2013, 2014 Renesas Electronics Corporation
 */

#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)
#ifndef CONFIG_SPL_BUILD
	mrc	p15, 0, r4, c0, c0, 5 /* mpidr */
	orr	r4, r4, r4, lsr #6
	and	r4, r4, #7 /* id 0-3 = ca15.0,1,2,3 */

	b do_lowlevel_init

	.pool

/*
 * CPU ID #1-#3 come here
 */
	.align  4
do_cpu_waiting:
	ldr	r1, =0xe6180000 /* sysc */
1:	ldr	r0, [r1, #0x20] /* sbar */
	tst	r0, r0
	beq	1b
	bx	r0

/*
 * Only CPU ID #0 comes here
 */
	.align  4
do_lowlevel_init:
	ldr	r2, =0xFF000044		/* PRR */
	ldr	r1, [r2]
	and	r1, r1, #0x7F00
	lsrs	r1, r1, #8
	cmp	r1, #0x4C		/* 0x4C is ID of r8a7794 */
	beq	_enable_actlr_smp

	/* surpress wfe if ca15 */
	tst r4, #4
	mrceq p15, 0, r0, c1, c0, 1	/* actlr */
	orreq r0, r0, #(1<<7)
	mcreq p15, 0, r0, c1, c0, 1

	/* and set l2 latency */
	mrc p15, 0, r0, c0, c0, 5	/* r0 = MPIDR */
	and r0, r0, #0xf00
	lsr r0, r0, #8
	tst r0, #1			/* only need for cluster 0 */
	bne _exit_init_l2_a15

	mrc p15, 1, r0, c9, c0, 2	/* r0 = L2CTLR */
	and r1, r0, #7
	cmp r1, #3			/* has already been set up */
	bicne r0, r0, #0xe7
	orrne r0, r0, #0x83		/* L2CTLR[7:6] + L2CTLR[2:0] */
#if defined(CONFIG_R8A7790)
	orrne r0, r0, #0x20		/* L2CTLR[5] */
#endif
	mcrne p15, 1, r0, c9, c0, 2

	b	_exit_init_l2_a15

_enable_actlr_smp: /* R8A7794 only (CA7) */
#ifndef CONFIG_DCACHE_OFF
	mrc    p15, 0, r0, c1, c0, 1
	orr    r0, r0, #0x40
	mcr    p15, 0, r0, c1, c0, 1
#endif

_exit_init_l2_a15:
	ldr	r3, =(CONFIG_SYS_INIT_SP_ADDR)
	sub	sp, r3, #4
	str	lr, [sp]

	/* initialize system */
	bl s_init

	ldr	lr, [sp]
#endif
	mov	pc, lr
	nop
ENDPROC(lowlevel_init)
	.ltorg
