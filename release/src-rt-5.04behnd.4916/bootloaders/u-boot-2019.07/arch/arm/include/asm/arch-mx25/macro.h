/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Matthias Weisser <weisserm@arcor.de>
 *
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 *
 * Common asm macros for imx25
 */

#ifndef __ASM_ARM_ARCH_MACRO_H__
#define __ASM_ARM_ARCH_MACRO_H__
#ifdef __ASSEMBLY__

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
 */
.macro init_aips mpr=0x77777777
	ldr	r0, =IMX_AIPS1_BASE
	ldr	r1, =\mpr
	str	r1, [r0, #AIPS_MPR_0_7]
	str	r1, [r0, #AIPS_MPR_8_15]
	ldr	r2, =IMX_AIPS2_BASE
	str	r1, [r2, #AIPS_MPR_0_7]
	str	r1, [r2, #AIPS_MPR_8_15]
.endm

/*
 * MAX (Multi-Layer AHB Crossbar Switch) setup
 *
 * Default argument values:
 *  - MPR: priority is IAHB > DAHB > USBOTG > RTIC > eSDHC2/SDMA
 *  - SGPCR: always park on last master
 *  - MGPCR: restore default values
 */
.macro init_max mpr=0x00043210, sgpcr=0x00000010, mgpcr=0x00000000
	ldr	r0, =IMX_MAX_BASE
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
.endm

/*
 * M3IF setup
 *
 * Default argument values:
 *  - CTL:
 * MRRP[0] = LCDC on priority list (1 << 0)			= 0x00000001
 * MRRP[1] = MAX1 not on priority list (0 << 1)			= 0x00000000
 * MRRP[2] = MAX0 not on priority list (0 << 2)			= 0x00000000
 * MRRP[3] = USBH not on priority list (0 << 3)			= 0x00000000
 * MRRP[4] = SDMA not on priority list (0 << 4)			= 0x00000000
 * MRRP[5] = eSDHC1/ATA/FEC not on priority list (0 << 5)	= 0x00000000
 * MRRP[6] = LCDC/SLCDC/MAX2 not on priority list (0 << 6)	= 0x00000000
 * MRRP[7] = CSI not on priority list (0 << 7)			= 0x00000000
 *								------------
 *								  0x00000001
 */
.macro init_m3if ctl=0x00000001
	/* M3IF Control Register (M3IFCTL) */
	write32	IMX_M3IF_CTRL_BASE, \ctl
.endm

#endif /* __ASSEMBLY__ */
#endif /* __ASM_ARM_ARCH_MACRO_H__ */
