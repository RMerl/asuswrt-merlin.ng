/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  armboot - Startup Code for XScale CPU-core
 *
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000	Wolfgang Denk <wd@denx.de>
 *  Copyright (C) 2001	Alex Zuepke <azu@sysgo.de>
 *  Copyright (C) 2001	Marius Groger <mag@sysgo.de>
 *  Copyright (C) 2002	Alex Zupke <azu@sysgo.de>
 *  Copyright (C) 2002	Gary Jennejohn <garyj@denx.de>
 *  Copyright (C) 2002	Kyle Harris <kharris@nexus-tech.net>
 *  Copyright (C) 2003	Kai-Uwe Bloem <kai-uwe.bloem@auerswald.de>
 *  Copyright (C) 2003	Kshitij <kshitij@ti.com>
 *  Copyright (C) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (C) 2003	Robert Schwebel <r.schwebel@pengutronix.de>
 *  Copyright (C) 2004	Texas Instruments <r-woodruff2@ti.com>
 *  Copyright (C) 2010	Marek Vasut <marek.vasut@gmail.com>
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

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl  cpu_init_crit
#endif

#ifdef	CONFIG_CPU_PXA25X
	bl	lock_cache_for_stack
#endif
#ifdef	CONFIG_CPU_PXA27X
	/*
	 * enable clock for SRAM
	 */
	ldr	r0,=CKEN
	ldr	r1,[r0]
	orr	r1,r1,#(1 << 20)
	str	r1,[r0]
#endif
	bl	_main

/*------------------------------------------------------------------------------*/

	.globl	c_runtime_cpu_setup
c_runtime_cpu_setup:

#ifdef CONFIG_CPU_PXA25X
	/*
	 * Unlock (actually, disable) the cache now that board_init_f
	 * is done. We could do this earlier but we would need to add
	 * a new C runtime hook, whereas c_runtime_cpu_setup already
	 * exists.
	 * As this routine is just a call to cpu_init_crit, let us
	 * tail-optimize and do a simple branch here.
	 */
	b	cpu_init_crit
#else
	bx	lr
#endif

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
#if !defined(CONFIG_SKIP_LOWLEVEL_INIT) || defined(CONFIG_CPU_PXA25X)
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
	bic	r0, r0, #0x00003300	@ clear bits 13:12, 9:8 (--VI --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 1 (A) Align
	mcr	p15, 0, r0, c1, c0, 0

	mov	pc, lr		/* back to my caller */
#endif /* !CONFIG_SKIP_LOWLEVEL_INIT || CONFIG_CPU_PXA25X */

/*
 * Enable MMU to use DCache as DRAM.
 *
 * This is useful on PXA25x and PXA26x in early bootstages, where there is no
 * other possible memory available to hold stack.
 */
#ifdef CONFIG_CPU_PXA25X
.macro CPWAIT reg
	mrc	p15, 0, \reg, c2, c0, 0
	mov	\reg, \reg
	sub	pc, pc, #4
.endm
lock_cache_for_stack:
	/* Domain access -- enable for all CPs */
	ldr	r0, =0x0000ffff
	mcr	p15, 0, r0, c3, c0, 0

	/* Point TTBR to MMU table */
	ldr	r0, =mmutable
	mcr	p15, 0, r0, c2, c0, 0

	/* Kick in MMU, ICache, DCache, BTB */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, #0x1b00
	bic	r0, #0x0087
	orr	r0, #0x1800
	orr	r0, #0x0005
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT	r0

	/* Unlock Icache, Dcache */
	mcr	p15, 0, r0, c9, c1, 1
	mcr	p15, 0, r0, c9, c2, 1

	/* Flush Icache, Dcache, BTB */
	mcr	p15, 0, r0, c7, c7, 0

	/* Unlock I-TLB, D-TLB */
	mcr	p15, 0, r0, c10, c4, 1
	mcr	p15, 0, r0, c10, c8, 1

	/* Flush TLB */
	mcr	p15, 0, r0, c8, c7, 0

	/* Allocate 4096 bytes of Dcache as RAM */

	/* Drain pending loads and stores */
	mcr	p15, 0, r0, c7, c10, 4

	mov	r4, #0x00
	mov	r5, #0x00
	mov	r2, #0x01
	mcr	p15, 0, r0, c9, c2, 0
	CPWAIT	r0

	/* 128 lines reserved (128 x 32bytes = 4096 bytes total) */
	mov	r0, #128
	ldr	r1, =0xfffff000

alloc:
	mcr	p15, 0, r1, c7, c2, 5
	/* Drain pending loads and stores */
	mcr	p15, 0, r0, c7, c10, 4
	strd	r4, [r1], #8
	strd	r4, [r1], #8
	strd	r4, [r1], #8
	strd	r4, [r1], #8
	subs	r0, #0x01
	bne	alloc
	/* Drain pending loads and stores */
	mcr	p15, 0, r0, c7, c10, 4
	mov	r2, #0x00
	mcr	p15, 0, r2, c9, c2, 0
	CPWAIT	r0

	mov	pc, lr

.section .mmutable, "a"
mmutable:
	.align	14
	/* 0x00000000 - 0xffe00000 : 1:1, uncached mapping */
	.set	__base, 0
	.rept	0xfff
	.word	(__base << 20) | 0xc12
	.set	__base, __base + 1
	.endr

	/* 0xfff00000 : 1:1, cached mapping */
	.word	(0xfff << 20) | 0x1c1e
#endif	/* CONFIG_CPU_PXA25X */
