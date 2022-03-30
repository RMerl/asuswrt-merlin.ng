/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  armboot - Startup Code for SA1100 CPU
 *
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000	Wolfgang Denk <wd@denx.de>
 *  Copyright (c) 2001	Alex Züpke <azu@sysgo.de>
 */

#include <asm-offsets.h>
#include <config.h>

/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * relocate armboot to ram
 * setup stack
 * jump to second stage
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

	mov	pc, lr

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


/* Interrupt-Controller base address */
IC_BASE:	.word	0x90050000
#define ICMR	0x04


/* Reset-Controller */
RST_BASE:		.word   0x90030000
#define RSRR	0x00
#define RCSR	0x04


/* PWR */
PWR_BASE:		.word   0x90020000
#define PSPR    0x08
#define PPCR    0x14
cpuspeed:		.word   CONFIG_SYS_CPUSPEED


cpu_init_crit:
	/*
	 * mask all IRQs
	 */
	ldr	r0, IC_BASE
	mov	r1, #0x00
	str	r1, [r0, #ICMR]

	/* set clock speed */
	ldr	r0, PWR_BASE
	ldr	r1, cpuspeed
	str	r1, [r0, #PPCR]

#ifndef CONFIG_SKIP_LOWLEVEL_INIT_ONLY
	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependend, you will
	 * find a lowlevel_init.S in your board directory.
	 */
	mov	ip,	lr
	bl	lowlevel_init
	mov	lr,	ip
#endif

	/*
	 * disable MMU stuff and enable I-cache
	 */
	mrc	p15,0,r0,c1,c0
	bic	r0, r0, #0x00002000	@ clear bit 13 (X)
	bic	r0, r0, #0x0000000f	@ clear bits 3-0 (WCAM)
	orr	r0, r0, #0x00001000	@ set bit 12 (I) Icache
	orr	r0, r0, #0x00000002	@ set bit 1 (A) Align
	mcr	p15,0,r0,c1,c0

	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	mov	pc, lr
