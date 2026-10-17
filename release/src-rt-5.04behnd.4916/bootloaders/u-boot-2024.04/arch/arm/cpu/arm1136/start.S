/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  armboot - Startup Code for OMP2420/ARM1136 CPU-core
 *
 *  Copyright (c) 2004	Texas Instruments <r-woodruff2@ti.com>
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
 */

#include <asm-offsets.h>
#include <config.h>

/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************
 */

	.globl	reset

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

	/* the mask ROM code should have PLL and others stable */
#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
	bl  cpu_init_crit
#endif

	bl	_main

/*------------------------------------------------------------------------------*/

	.globl	c_runtime_cpu_setup
c_runtime_cpu_setup:

	bx	lr

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */
#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* Invalidate I+D+BTB caches */
	mcr	p15, 0, r0, c8, c7, 0	/* Invalidate Unified TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 1 (A) Align
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
	mcr	p15, 0, r0, c1, c0, 0

#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT_ONLY)
	/*
	 * Jump to board specific initialization... The Mask ROM will have already initialized
	 * basic memory.  Go here to bump up clock rate and handle wake up conditions.
	 */
	mov	ip, lr		/* persevere link reg across call */
	bl	lowlevel_init	/* go setup pll,mux,memory */
	mov	lr, ip		/* restore link */
#endif
	mov	pc, lr		/* back to my caller */
#endif /* !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT) */
