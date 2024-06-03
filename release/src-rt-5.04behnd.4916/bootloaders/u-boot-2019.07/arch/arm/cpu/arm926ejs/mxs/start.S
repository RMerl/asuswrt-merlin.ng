/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  armboot - Startup Code for ARM926EJS CPU-core
 *
 *  Copyright (c) 2003  Texas Instruments
 *
 *  ----- Adapted for OMAP1610 OMAP730 from ARM925t code ------
 *
 *  Copyright (c) 2001	Marius Groger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Zupke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
 *  Copyright (c) 2010	Albert Aribaud <albert.u.boot@aribaud.net>
 *
 * Change to support call back into iMX28 bootrom
 * Copyright (c) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
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
	 * If the CPU is configured in "Wait JTAG connection mode", the stack
	 * pointer is not configured and is zero. This will cause crash when
	 * trying to push data onto stack right below here. Load the SP and make
	 * it point to the end of OCRAM if the SP is zero.
	 */
	cmp	sp, #0x00000000
	ldreq	sp, =CONFIG_SYS_INIT_SP_ADDR

	/*
	 * Store all registers on old stack pointer, this will allow us later to
	 * return to the BootROM and let the BootROM load U-Boot into RAM.
	 *
	 * WARNING: Register r0 and r1 are used by the BootROM to pass data
	 *          to the called code. Register r0 will contain arbitrary
	 *          data that are set in the BootStream. In case this code
	 *          was started with CALL instruction, register r1 will contain
	 *          pointer to the return value this function can then set.
	 *          The code below MUST NOT CHANGE register r0 and r1 !
	 */
	push	{r0-r12,r14}

	/* Save control register c1 */
	mrc	p15, 0, r2, c1, c0, 0
	push	{r2}

	/* Set the cpu to SVC32 mode and store old CPSR register content. */
	mrs	r2, cpsr
	push	{r2}
	bic	r2, r2, #0x1f
	orr	r2, r2, #0xd3
	msr	cpsr, r2

	bl	board_init_ll

	/* Restore BootROM's CPU mode (especially FIQ). */
	pop	{r2}
	msr	cpsr,r2

	/*
	 * Restore c1 register. Especially set exception vector location
	 * back to BootROM space which is required by bootrom for USB boot.
	 */
	pop	{r2}
	mcr	p15, 0, r2, c1, c0, 0

	pop	{r0-r12,r14}

	/*
	 * In case this code was started by the CALL instruction, the register
	 * r0 is examined by the BootROM after this code returns. The value in
	 * r0 must be set to 0 to indicate successful return.
	 */
	mov r0, #0

	bx	lr
