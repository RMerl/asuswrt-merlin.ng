/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2004, 2007, 2008 Freescale Semiconductor.
 * Srikanth Srinivasan <srikanth.srinivaan@freescale.com>
 */
#include <config.h>
#include <mpc86xx.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

/* If this is a multi-cpu system then we need to handle the
 * 2nd cpu.  The assumption is that the 2nd cpu is being
 * held in boot holdoff mode until the 1st cpu unlocks it
 * from Linux.	We'll do some basic cpu init and then pass
 * it to the Linux Reset Vector.
 * Sri:	 Much of this initialization is not required. Linux
 * rewrites the bats, and the sprs and also enables the L1 cache.
 *
 * Core 0 must copy this to a 1M aligned region and set BPTR
 * to point to it.
 */
	.align 12
.globl __secondary_start_page
__secondary_start_page:
	.space 0x100	/* space over to reset vector loc */
	mfspr	r0, MSSCR0
	andi.	r0, r0, 0x0020
	rlwinm	r0,r0,27,31,31
	mtspr	PIR, r0

	/* Invalidate BATs */
	li	r0, 0
	mtspr	IBAT0U, r0
	mtspr	IBAT1U, r0
	mtspr	IBAT2U, r0
	mtspr	IBAT3U, r0
	mtspr	IBAT4U, r0
	mtspr	IBAT5U, r0
	mtspr	IBAT6U, r0
	mtspr	IBAT7U, r0
	isync
	mtspr	DBAT0U, r0
	mtspr	DBAT1U, r0
	mtspr	DBAT2U, r0
	mtspr	DBAT3U, r0
	mtspr	DBAT4U, r0
	mtspr	DBAT5U, r0
	mtspr	DBAT6U, r0
	mtspr	DBAT7U, r0
	isync
	sync

	/* enable extended addressing */
	mfspr	r0, HID0
	lis	r0, (HID0_HIGH_BAT_EN | HID0_XBSEN | HID0_XAEN)@h
	ori	r0, r0, (HID0_HIGH_BAT_EN | HID0_XBSEN | HID0_XAEN)@l
	mtspr	HID0, r0
	sync
	isync

#ifdef CONFIG_SYS_L2
	/* init the L2 cache */
	addis	r3, r0, L2_INIT@h
	ori	r3, r3, L2_INIT@l
	sync
	mtspr	l2cr, r3
#ifdef CONFIG_ALTIVEC
	dssall
#endif
	/* invalidate the L2 cache */
	mfspr	r3, l2cr
	rlwinm.	r3, r3, 0, 0, 0
	beq	1f

	mfspr	r3, l2cr
	rlwinm	r3, r3, 0, 1, 31

#ifdef	CONFIG_ALTIVEC
	dssall
#endif
	sync
	mtspr	l2cr, r3
	sync
1:	mfspr	r3, l2cr
	oris	r3, r3, L2CR_L2I@h
	mtspr	l2cr, r3

invl2:
	mfspr	r3, l2cr
	andis.	r3, r3, L2CR_L2I@h
	bne	invl2
	sync
#endif

	/* enable and invalidate the data cache */
	mfspr	r3, HID0
	li	r5, HID0_DCFI|HID0_DLOCK
	andc	r3, r3, r5
	mtspr	HID0, r3		/* no invalidate, unlock */
	ori	r3, r3, HID0_DCE
	ori	r5, r3, HID0_DCFI
	mtspr	HID0, r5		/* enable + invalidate */
	mtspr	HID0, r3		/* enable */
	sync
#ifdef CONFIG_SYS_L2
	sync
	lis	r3, L2_ENABLE@h
	ori	r3, r3, L2_ENABLE@l
	mtspr	l2cr, r3
	isync
	sync
#endif

	/* enable and invalidate the instruction cache*/
	mfspr	r3, HID0
	li	r5, HID0_ICFI|HID0_ILOCK
	andc	r3, r3, r5
	ori	r3, r3, HID0_ICE
	ori	r5, r3, HID0_ICFI
	mtspr	HID0, r5
	mtspr	HID0, r3
	isync
	sync

	/* TBEN in HID0 */
	mfspr	r4, HID0
	oris	r4, r4, 0x0400
	mtspr	HID0, r4
	sync
	isync

	/* MCP|SYNCBE|ABE in HID1 */
	mfspr	r4, HID1
	oris	r4, r4, 0x8000
	ori	r4, r4, 0x0C00
	mtspr	HID1, r4
	sync
	isync

	lis	r3, CONFIG_LINUX_RESET_VEC@h
	ori	r3, r3, CONFIG_LINUX_RESET_VEC@l
	mtlr	r3
	blr

	/* Never Returns, Running in Linux Now */
