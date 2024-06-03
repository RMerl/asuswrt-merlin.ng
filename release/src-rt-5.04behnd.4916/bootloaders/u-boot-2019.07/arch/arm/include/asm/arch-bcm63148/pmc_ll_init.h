/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include <asm/arch/pmc.h>
#include <asm/arch/pmc_addr.h>
#include <asm/arch/misc.h>

#define SETLEDS(a,b,c,d)

/*  *********************************************************************
    *  This function power up any necessary modules that are controlled by
    *  PMC for board to boot such as vdsl.
    *  This is called when still executing in place on flash
    ********************************************************************* */
pmc_ll_init:

	ldr	r0, =MISC_BASE	/* check if PMC ROM is enabled or not */
	ldr	r0, [r0, #MISC_STRAP_BUS]
	lsr	r0, #MISC_STRAP_BUS_PMC_ROM_BOOT_SHIFT
	ands	r0, #0x1
	beq	pmbd

	SETLEDS('P','M','C','S')
	b	pmcs
pmbd:
	SETLEDS('P','M','B','S')
pmcs:

	/* workaround for the high temp lock issue. no need for 148 because
	   these setting are already in the chip */
#if defined(CONFIG_BCM63138)
	/* config AFE PLL */
	mov	r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
	mov	r1, #0x5		/*  cfg[0] reg offset in PLL_BPCM_REGS */ 
	bl	pmc_read_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	mov	r0, #1
	lsl	r0, #27
	orr	r2, r1, r0
	mov	r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
	mov	r1, #0x5		/* cfg[0] reg offset in PLL_BPCM_REGS */
	bl	pmc_write_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	/* overwrite ndiv and pdiv */
	ldr	r2, =0x80000001
	mov	r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
	mov	r1, #0x12		/* pdiv reg offset in PLL_BPCM_REGS */
	bl	pmc_write_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	ldr	r2, =0x80000030
	mov	r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
	mov	r1, #0x11		/* ndiv reg offset in PLL_BPCM_REGS */
	bl	pmc_write_bpcm_reg
	cmp	r0, #0
	bne	pmc_error
#endif

	/* start AFE PLL */
	mov	r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
	mov	r1, #0x4		 /* resets reg offset in PLL_BPCM_REGS */ 
	bl	pmc_read_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	orr	r2, r1, #0x3
	mov	r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
	mov	r1, #0x4		 /* resets reg offset in PLL_BPCM_REGS */
	bl	 pmc_write_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	/* wait AFE PLL to lock */
afel:
	mov	r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
	mov	r1, #0xa
	bl	pmc_read_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	/* Wait for AFE PLL lock: repeat read until bit 31 (AFE PLL lock bit) is set */
	ldr	r0, =0x80000000
	and	r1, r0
	cmp	r1, #0		 /* if bit 31 is not one, repeat read of reg 0x1700a */
	beq	afel

	/* AFE is locked, commence LMEM init */
	/* Enable VDSL step 0. Power on zone 0, 1 and 2 */
	mov	r5, #0x10
pwr_zone_vdsl:
	mov	r0, #PMB_ADDR_VDSL3_CORE
	mov	r1, r5
	bl	pmc_read_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	ldr	r0, =0x1d00
	orr	r2, r0, r1
	mov	r0, #PMB_ADDR_VDSL3_CORE
	mov	r1, r5
	bl	pmc_write_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	add	r5, #4
	/* zone 0 starts from 0x10 offset */
	cmp	r5, #(0x10+4*PMB_ZONES_VDSL3_CORE)
	bne	pwr_zone_vdsl

	/* Enable VDSL step 1: initiate a read of register 0x1600a via the PMC message handler */
	mov	r0, #PMB_ADDR_VDSL3_CORE
	mov	r1, #0xa
	bl	pmc_read_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	/* Enable VDSL step 1: or data with 0xffffff01 and write back into 0x1600a */ 
	ldr	r0, =0xffffff01
	orr	r2, r0, r1
	mov	r0, #PMB_ADDR_VDSL3_CORE
	mov	r1, #0xa
	bl	pmc_write_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	/* Enable VDSL step 2   : initiate a read of register 0x1600c via the PMC message handler */
	mov	r0, #PMB_ADDR_VDSL3_CORE
	mov	r1, #0xc
	bl	pmc_read_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	/* Enable VDSL step 2   : set the bottom two bits high and rewrite back into 0x1600c */
	mov	r0, #0x3
	orr	r2, r0, r1
	mov	r0, #PMB_ADDR_VDSL3_CORE
	mov	r1, #0xc
	bl	pmc_write_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	/* Enable VDSL step 3: initiate a read of register 0x1600a via the PMC message handler */
	mov	r0, #PMB_ADDR_VDSL3_CORE
	mov	r1, #0xa
	bl	pmc_read_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	/* Enable VDSL step 3   : write to reg 0x1600a */
	ldr	r0, =0xffffff03
	orr	r2, r0, r1
	mov	r0, #PMB_ADDR_VDSL3_CORE
	mov	r1, #0xa
	bl	pmc_write_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	/* Enable PCM_BMU zones */
	mov	r5, #0x10
pwr_zone_apm:
	mov	r0, #PMB_ADDR_APM
	mov	r1, r5
	bl	pmc_read_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	ldr	r0, =0x1d00
	orr	r2, r0, r1
	mov	r0, #PMB_ADDR_APM
	mov	r1, r5
	bl	pmc_write_bpcm_reg
	cmp	r0, #0
	bne	pmc_error

	add	r5, #4
	/* zone 0 starts from 0x10 offset */
	cmp	r5, #(0x10+4*PMB_ZONES_APM)
	bne	pwr_zone_apm

	/* Move LDO reference to make it settle to the right voltage */
	ldr	r0, =0x80100130
	ldr	r1, [r0]
	//set bit[15] high
	ldr	r2, =0x8000
	orr	r1, r2
	str	r1, [r0]

#if defined(CONFIG_BCM63138)
	/* 63148 does not need to deassert */
	//Wait 550usec de-assert bit[15].
	ldr	r2, =110000
w2:
	sub	r2, #1
	cmp	r2, #0
	bne	w2
	ldr	r1, [r0]
	bic	r1, #0x8000
	str	r1, [r0]
#endif

	b	pmc_done

pmc_error:
	SETLEDS('P','M','C','E')
	mov	r0, #1
	/* failed to power lmem? dead and stuck here */
	b	pmc_error

pmc_done:
	SETLEDS('P','M','C','D')
	mov	r0, #0
