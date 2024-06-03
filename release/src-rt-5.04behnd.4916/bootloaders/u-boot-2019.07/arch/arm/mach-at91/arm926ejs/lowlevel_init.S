/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *		       Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * Copyright (C) 2008 Ronetix Ilko Iliev (www.ronetix.at)
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 */

#include <config.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_wdt.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/at91_matrix.h>
#include <asm/arch/at91sam9_sdramc.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_rstc.h>
#ifdef CONFIG_ATMEL_LEGACY
#include <asm/arch/at91sam9_matrix.h>
#endif
#ifndef CONFIG_SYS_MATRIX_EBICSA_VAL
#define CONFIG_SYS_MATRIX_EBICSA_VAL CONFIG_SYS_MATRIX_EBI0CSA_VAL
#endif

.globl lowlevel_init
.type lowlevel_init,function
lowlevel_init:

POS1:
	adr	r5, POS1	/* r5 = POS1 run time */
	ldr	r0, =POS1	/* r0 = POS1 compile */
	sub	r5, r5, r0	/* r0 = CONFIG_SYS_TEXT_BASE-1 */

	/* memory control configuration 1 */
	ldr	r0, =SMRDATA
	ldr	r2, =SMRDATA1
	add	r0, r0, r5
	add	r2, r2, r5
0:
	/* the address */
	ldr	r1, [r0], #4
	/* the value */
	ldr	r3, [r0], #4
	str	r3, [r1]
	cmp	r2, r0
	bne	0b

/* ----------------------------------------------------------------------------
 * PMC Init Step 1.
 * ----------------------------------------------------------------------------
 * - Check if the PLL is already initialized
 * ----------------------------------------------------------------------------
 */
	ldr	r1, =(AT91_ASM_PMC_MCKR)
	ldr	r0, [r1]
	and	r0, r0, #3
	cmp	r0, #0
	bne	PLL_setup_end

/* ---------------------------------------------------------------------------
 * - Enable the Main Oscillator
 * ---------------------------------------------------------------------------
 */
	ldr	r1, =(AT91_ASM_PMC_MOR)
	ldr	r2, =(AT91_ASM_PMC_SR)
	/* Main oscillator Enable register PMC_MOR: */
	ldr	r0, =CONFIG_SYS_MOR_VAL
	str	r0, [r1]

	/* Reading the PMC Status to detect when the Main Oscillator is enabled */
	mov	r4, #AT91_PMC_IXR_MOSCS
MOSCS_Loop:
	ldr	r3, [r2]
	and	r3, r4, r3
	cmp	r3, #AT91_PMC_IXR_MOSCS
	bne	MOSCS_Loop

/* ----------------------------------------------------------------------------
 * PMC Init Step 2.
 * ----------------------------------------------------------------------------
 * Setup PLLA
 * ----------------------------------------------------------------------------
 */
	ldr	r1, =(AT91_ASM_PMC_PLLAR)
	ldr	r0, =CONFIG_SYS_PLLAR_VAL
	str	r0, [r1]

	/* Reading the PMC Status register to detect when the PLLA is locked */
	mov	r4, #AT91_PMC_IXR_LOCKA
MOSCS_Loop1:
	ldr	r3, [r2]
	and	r3, r4, r3
	cmp	r3, #AT91_PMC_IXR_LOCKA
	bne	MOSCS_Loop1

/* ----------------------------------------------------------------------------
 * PMC Init Step 3.
 * ----------------------------------------------------------------------------
 * - Switch on the Main Oscillator
 * ----------------------------------------------------------------------------
 */
	ldr	r1, =(AT91_ASM_PMC_MCKR)

	/* -Master Clock Controller register PMC_MCKR */
	ldr	r0, =CONFIG_SYS_MCKR1_VAL
	str	r0, [r1]

	/* Reading the PMC Status to detect when the Master clock is ready */
	mov	r4, #AT91_PMC_IXR_MCKRDY
MCKRDY_Loop:
	ldr	r3, [r2]
	and	r3, r4, r3
	cmp	r3, #AT91_PMC_IXR_MCKRDY
	bne	MCKRDY_Loop

	ldr	r0, =CONFIG_SYS_MCKR2_VAL
	str	r0, [r1]

	/* Reading the PMC Status to detect when the Master clock is ready */
	mov	r4, #AT91_PMC_IXR_MCKRDY
MCKRDY_Loop1:
	ldr	r3, [r2]
	and	r3, r4, r3
	cmp	r3, #AT91_PMC_IXR_MCKRDY
	bne	MCKRDY_Loop1
PLL_setup_end:

/* ----------------------------------------------------------------------------
 * - memory control configuration 2
 * ----------------------------------------------------------------------------
 */
	ldr	r0, =(AT91_ASM_SDRAMC_TR)
	ldr	r1, [r0]
	cmp	r1, #0
	bne	SDRAM_setup_end

	ldr	r0, =SMRDATA1
	ldr	r2, =SMRDATA2
	add	r0, r0, r5
	add	r2, r2, r5
2:
	/* the address */
	ldr	r1, [r0], #4
	/* the value */
	ldr	r3, [r0], #4
	str	r3, [r1]
	cmp	r2, r0
	bne	2b

SDRAM_setup_end:
	/* everything is fine now */
	mov	pc, lr

	.ltorg

SMRDATA:
	.word AT91_ASM_WDT_MR
	.word CONFIG_SYS_WDTC_WDMR_VAL
	/* configure PIOx as EBI0 D[16-31] */
#if defined(CONFIG_AT91SAM9263)
	.word AT91_ASM_PIOD_PDR
	.word CONFIG_SYS_PIOD_PDR_VAL1
	.word AT91_ASM_PIOD_PUDR
	.word CONFIG_SYS_PIOD_PPUDR_VAL
	.word AT91_ASM_PIOD_ASR
	.word CONFIG_SYS_PIOD_PPUDR_VAL
#elif defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9261) \
	|| defined(CONFIG_AT91SAM9G20)
	.word AT91_ASM_PIOC_PDR
	.word CONFIG_SYS_PIOC_PDR_VAL1
	.word AT91_ASM_PIOC_PUDR
	.word CONFIG_SYS_PIOC_PPUDR_VAL
#endif
	.word AT91_ASM_MATRIX_CSA0
	.word CONFIG_SYS_MATRIX_EBICSA_VAL

	/* flash */
	.word AT91_ASM_SMC_MODE0
	.word CONFIG_SYS_SMC0_MODE0_VAL

	.word AT91_ASM_SMC_CYCLE0
	.word CONFIG_SYS_SMC0_CYCLE0_VAL

	.word AT91_ASM_SMC_PULSE0
	.word CONFIG_SYS_SMC0_PULSE0_VAL

	.word AT91_ASM_SMC_SETUP0
	.word CONFIG_SYS_SMC0_SETUP0_VAL

SMRDATA1:
	.word AT91_ASM_SDRAMC_MR
	.word CONFIG_SYS_SDRC_MR_VAL1
	.word AT91_ASM_SDRAMC_TR
	.word CONFIG_SYS_SDRC_TR_VAL1
	.word AT91_ASM_SDRAMC_CR
	.word CONFIG_SYS_SDRC_CR_VAL
	.word AT91_ASM_SDRAMC_MDR
	.word CONFIG_SYS_SDRC_MDR_VAL
	.word AT91_ASM_SDRAMC_MR
	.word CONFIG_SYS_SDRC_MR_VAL2
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL1
	.word AT91_ASM_SDRAMC_MR
	.word CONFIG_SYS_SDRC_MR_VAL3
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL2
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL3
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL4
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL5
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL6
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL7
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL8
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL9
	.word AT91_ASM_SDRAMC_MR
	.word CONFIG_SYS_SDRC_MR_VAL4
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL10
	.word AT91_ASM_SDRAMC_MR
	.word CONFIG_SYS_SDRC_MR_VAL5
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL11
	.word AT91_ASM_SDRAMC_TR
	.word CONFIG_SYS_SDRC_TR_VAL2
	.word CONFIG_SYS_SDRAM_BASE
	.word CONFIG_SYS_SDRAM_VAL12
	/* User reset enable*/
	.word AT91_ASM_RSTC_MR
	.word CONFIG_SYS_RSTC_RMR_VAL
#ifdef CONFIG_SYS_MATRIX_MCFG_REMAP
	/* MATRIX_MCFG - REMAP all masters */
	.word AT91_ASM_MATRIX_MCFG
	.word 0x1FF
#endif
SMRDATA2:
	.word 0
