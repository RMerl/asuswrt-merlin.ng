/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Matthias Weisser <weisserm@arcor.de>
 *
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 *
 * Based on U-Boot and RedBoot sources for several different i.mx
 * platforms.
 */

#include <asm/macro.h>
#include <asm/arch/macro.h>
#include <asm/arch/imx-regs.h>
#include <generated/asm-offsets.h>

/*
 * clocks
 */
.macro init_clocks

	/* disable clock output */
	write32	IMX_CCM_BASE + CCM_MCR, 0x00000000
	write32	IMX_CCM_BASE + CCM_CCTL, 0x50030000

	/*
	 * enable all implemented clocks in all three
	 * clock control registers
	 */
	write32	IMX_CCM_BASE + CCM_CGCR0, 0x1fffffff
	write32	IMX_CCM_BASE + CCM_CGCR1, 0xffffffff
	write32	IMX_CCM_BASE + CCM_CGCR2, 0xfffff

	/* Devide NAND clock by 32 */
	write32	IMX_CCM_BASE + CCM_PCDR2, 0x0101011F
.endm

/*
 * sdram controller init
 */
.macro init_lpddr
	ldr	r0, =IMX_ESDRAMC_BASE
	ldr	r2, =IMX_SDRAM_BANK0_BASE

	/*
	 * reset SDRAM controller
	 * then wait for initialization to complete
	 */
	ldr	r1, =(1 << 1) | (1 << 2)
	str	r1, [r0, #ESDRAMC_ESDMISC]
1:	ldr	r3, [r0, #ESDRAMC_ESDMISC]
	tst	r3, #(1 << 31)
	beq	1b
	ldr	r1, =(1 << 2)
	str	r1, [r0, #ESDRAMC_ESDMISC]

	ldr	r1, =0x002a7420
	str	r1, [r0, #ESDRAMC_ESDCFG0]

	/* control | precharge */
	ldr	r1, =0x92216008
	str	r1, [r0, #ESDRAMC_ESDCTL0]
	/* dram command encoded in address */
	str	r1, [r2, #0x400]

	/* auto refresh */
	ldr	r1, =0xa2216008
	str	r1, [r0, #ESDRAMC_ESDCTL0]
	/* read dram twice to auto refresh */
	ldr	    r3, [r2]
	ldr     r3, [r2]

	/* control | load mode */
	ldr	r1, =0xb2216008
	str	r1, [r0, #ESDRAMC_ESDCTL0]

	/* mode register of lpddram */
	strb	r1, [r2, #0x33]

	/* extended mode register of lpddrram */
	ldr		r2, =0x81000000
	strb	r1, [r2]

	/* control | normal */
	ldr	r1, =0x82216008
	str	r1, [r0, #ESDRAMC_ESDCTL0]
.endm

.globl lowlevel_init
lowlevel_init:
	init_aips
	init_max
	init_clocks
	init_lpddr
	mov	pc, lr
