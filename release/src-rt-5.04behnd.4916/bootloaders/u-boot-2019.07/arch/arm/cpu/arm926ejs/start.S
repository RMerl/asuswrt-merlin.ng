/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  armboot - Startup Code for ARM926EJS CPU-core
 *
 *  Copyright (c) 2003  Texas Instruments
 *
 *  ----- Adapted for OMAP1610 OMAP730 from ARM925t code ------
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
 *  Copyright (c) 2010	Albert Aribaud <albert.u.boot@aribaud.net>
 */

#include <asm-offsets.h>
#include <config.h>
#include <common.h>

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

	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl	cpu_init_crit
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
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
cpu_init_crit:
	/*
	 * flush D cache before disabling it
	 */
	mov	r0, #0
flush_dcache:
	mrc	p15, 0, r15, c7, c10, 3
	bne	flush_dcache

	mcr	p15, 0, r0, c8, c7, 0	/* invalidate TLB */
	mcr	p15, 0, r0, c7, c5, 0	/* invalidate I Cache */

	/*
	 * disable MMU and D cache
	 * enable I cache if SYS_ICACHE_OFF is not defined
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00000300	/* clear bits 9:8 (---- --RS) */
	bic	r0, r0, #0x00000087	/* clear bits 7, 2:0 (B--- -CAM) */
#ifdef CONFIG_SYS_EXCEPTION_VECTORS_HIGH
	orr	r0, r0, #0x00002000	/* set bit 13 (--V- ----) */
#else
	bic	r0, r0, #0x00002000	/* clear bit 13 (--V- ----) */
#endif
	orr	r0, r0, #0x00000002	/* set bit 1 (A) Align */
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	orr	r0, r0, #0x00001000	/* set bit 12 (I) I-Cache */
#endif
	mcr	p15, 0, r0, c1, c0, 0

#ifndef CONFIG_SKIP_LOWLEVEL_INIT_ONLY
	/*
	 * Go setup Memory and board specific bits prior to relocation.
	 */
	mov	r4, lr		/* perserve link reg across call */
	bl	lowlevel_init	/* go setup pll,mux,memory */
	mov	lr, r4		/* restore link */
#endif
	mov	pc, lr		/* back to my caller */
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */
