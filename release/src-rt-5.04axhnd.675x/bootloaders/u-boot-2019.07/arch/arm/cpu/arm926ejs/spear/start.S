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
 */


#include <config.h>

/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * Below are the critical initializations already taken place in BootROM.
 * So, these are not taken care in Xloader
 * 1. Relocation to RAM
 * 2. Initializing stacks
 *
 *************************************************************************
 */

	.globl	reset

reset:
/*
 * Xloader has to return back to BootROM in a few cases.
 * eg. Ethernet boot, UART boot, USB boot
 * Saving registers for returning back
 */
	stmdb	sp!, {r0-r12,r14}
	bl	cpu_init_crit
/*
 * Clearing bss area is not done in Xloader.
 * BSS area lies in the DDR location which is not yet initialized
 * bss is assumed to be uninitialized.
 */
	ldmia	sp!, {r0-r12,pc}

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
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * enable instruction cache
	 */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #0x00001000	/* set bit 12 (I) I-Cache */
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * Go setup Memory and board specific bits prior to relocation.
	 */
	stmdb	sp!, {lr}
	bl	_main	/* _main will call board_init_f */
	ldmia	sp!, {pc}
