/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  armboot - Startup Code for ARM720 CPU-core
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 */

#include <asm-offsets.h>
#include <config.h>

/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from RAM!
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
#if !defined(CONFIG_SKIP_LOWLEVEL_INIT) && \
		!defined(CONFIG_SKIP_LOWLEVEL_INIT_ONLY)
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

#if !defined(CONFIG_SKIP_LOWLEVEL_INIT) && \
		!defined(CONFIG_SKIP_LOWLEVEL_INIT_ONLY)
cpu_init_crit:

	mov	ip, lr
	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependent, you will
	 * find a lowlevel_init.S in your board directory.
	 */
	bl	lowlevel_init
	mov	lr, ip

	mov	pc, lr
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */
