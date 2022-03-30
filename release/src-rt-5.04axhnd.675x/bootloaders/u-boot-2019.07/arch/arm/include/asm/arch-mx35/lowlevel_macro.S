/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 */

#include <asm/arch/imx-regs.h>
#include <generated/asm-offsets.h>
#include <asm/macro.h>

/*
 * AIPS setup - Only setup MPROTx registers.
 * The PACR default values are good.
 *
 * Default argument values:
 *  - MPR: Set all MPROTx to be non-bufferable, trusted for R/W, not forced to
 *    user-mode.
 *  - OPACR: Clear the on and off peripheral modules Supervisor Protect bit for
 *    SDMA to access them.
 */
.macro init_aips mpr=0x77777777, opacr=0x00000000
	ldr	r0, =AIPS1_BASE_ADDR
	ldr	r1, =\mpr
	str	r1, [r0, #AIPS_MPR_0_7]
	str	r1, [r0, #AIPS_MPR_8_15]
	ldr	r2, =AIPS2_BASE_ADDR
	str	r1, [r2, #AIPS_MPR_0_7]
	str	r1, [r2, #AIPS_MPR_8_15]

	/* Did not change the AIPS control registers access type. */
	ldr	r1, =\opacr
	str	r1, [r0, #AIPS_OPACR_0_7]
	str	r1, [r0, #AIPS_OPACR_8_15]
	str	r1, [r0, #AIPS_OPACR_16_23]
	str	r1, [r0, #AIPS_OPACR_24_31]
	str	r1, [r0, #AIPS_OPACR_32_39]
	str	r1, [r2, #AIPS_OPACR_0_7]
	str	r1, [r2, #AIPS_OPACR_8_15]
	str	r1, [r2, #AIPS_OPACR_16_23]
	str	r1, [r2, #AIPS_OPACR_24_31]
	str	r1, [r2, #AIPS_OPACR_32_39]
.endm

/*
 * MAX (Multi-Layer AHB Crossbar Switch) setup
 *
 * Default argument values:
 *  - MPR: priority is M4 > M2 > M3 > M5 > M0 > M1
 *  - SGPCR: always park on last master
 *  - MGPCR: restore default values
 */
.macro init_max mpr=0x00302154, sgpcr=0x00000010, mgpcr=0x00000000
	ldr	r0, =MAX_BASE_ADDR
	ldr	r1, =\mpr
	str	r1, [r0, #MAX_MPR0]	/* for S0 */
	str	r1, [r0, #MAX_MPR1]	/* for S1 */
	str	r1, [r0, #MAX_MPR2]	/* for S2 */
	str	r1, [r0, #MAX_MPR3]	/* for S3 */
	str	r1, [r0, #MAX_MPR4]	/* for S4 */
	ldr	r1, =\sgpcr
	str	r1, [r0, #MAX_SGPCR0]	/* for S0 */
	str	r1, [r0, #MAX_SGPCR1]	/* for S1 */
	str	r1, [r0, #MAX_SGPCR2]	/* for S2 */
	str	r1, [r0, #MAX_SGPCR3]	/* for S3 */
	str	r1, [r0, #MAX_SGPCR4]	/* for S4 */
	ldr	r1, =\mgpcr
	str	r1, [r0, #MAX_MGPCR0]	/* for M0 */
	str	r1, [r0, #MAX_MGPCR1]	/* for M1 */
	str	r1, [r0, #MAX_MGPCR2]	/* for M2 */
	str	r1, [r0, #MAX_MGPCR3]	/* for M3 */
	str	r1, [r0, #MAX_MGPCR4]	/* for M4 */
	str	r1, [r0, #MAX_MGPCR5]	/* for M5 */
.endm

/*
 * M3IF setup
 *
 * Default argument values:
 *  - CTL:
 * MRRP[0] = L2CC0 not on priority list (0 << 0)	= 0x00000000
 * MRRP[1] = L2CC1 not on priority list (0 << 1)	= 0x00000000
 * MRRP[2] = MBX not on priority list (0 << 2)		= 0x00000000
 * MRRP[3] = MAX1 not on priority list (0 << 3)		= 0x00000000
 * MRRP[4] = SDMA not on priority list (0 << 4)		= 0x00000000
 * MRRP[5] = MPEG4 not on priority list (0 << 5)	= 0x00000000
 * MRRP[6] = IPU1 on priority list (1 << 6)		= 0x00000040
 * MRRP[7] = IPU2 not on priority list (0 << 7)		= 0x00000000
 *							------------
 *							  0x00000040
 */
.macro init_m3if ctl=0x00000040
	/* M3IF Control Register (M3IFCTL) */
	write32	M3IF_BASE_ADDR, \ctl
.endm

.macro core_init
	mrc	p15, 0, r1, c1, c0, 0

	/* Set branch prediction enable */
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #7
	mcr	p15, 0, r0, c1, c0, 1
	orr	r1, r1, #1 << 11

	/* Set unaligned access enable */
	orr	r1, r1, #1 << 22

	/* Set low int latency enable */
	orr	r1, r1, #1 << 21

	mcr	p15, 0, r1, c1, c0, 0

	mov	r0, #0

	mcr	p15, 0, r0, c15, c2, 4

	mcr	p15, 0, r0, c7, c7, 0	/* Invalidate I cache and D cache */
	mcr	p15, 0, r0, c8, c7, 0	/* Invalidate TLBs */
	mcr	p15, 0, r0, c7, c10, 4	/* Drain the write buffer */

	/* Setup the Peripheral Port Memory Remap Register */
	ldr	r0, =0x40000015		/* Start from AIPS 2-GB region */
	mcr	p15, 0, r0, c15, c2, 4
.endm
