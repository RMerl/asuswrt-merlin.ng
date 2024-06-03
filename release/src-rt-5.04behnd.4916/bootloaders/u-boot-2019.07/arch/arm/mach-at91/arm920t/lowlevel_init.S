/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *		       Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * Modified for the at91rm9200dk board by
 * (C) Copyright 2004
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 */

#include <config.h>

#ifndef CONFIG_SKIP_LOWLEVEL_INIT

#include <asm/arch/hardware.h>
#include <asm/arch/at91_mc.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_pio.h>

#define ARM920T_CONTROL	0xC0000000	/* @ set bit 31 (iA) and 30 (nF) */

_MTEXT_BASE:
#undef START_FROM_MEM
#ifdef START_FROM_MEM
	.word	CONFIG_SYS_TEXT_BASE-PHYS_FLASH_1
#else
	.word	CONFIG_SYS_TEXT_BASE
#endif

.globl lowlevel_init
lowlevel_init:
	ldr     r1, =AT91_ASM_PMC_MOR
	/* Main oscillator Enable register */
#ifdef CONFIG_SYS_USE_MAIN_OSCILLATOR
	ldr     r0, =0x0000FF01		/* Enable main oscillator */
#else
	ldr     r0, =0x0000FF00		/* Disable main oscillator */
#endif
	str     r0, [r1] /*AT91C_CKGR_MOR] */
	/* Add loop to compensate Main Oscillator startup time */
	ldr     r0, =0x00000010
LoopOsc:
	subs    r0, r0, #1
	bhi     LoopOsc

	/* memory control configuration */
	/* this isn't very elegant, but	 what the heck */
	ldr	r0, =SMRDATA
	ldr	r1, _MTEXT_BASE
	sub	r0, r0, r1
	ldr	r2, =SMRDATAE
	sub	r2, r2, r1
pllloop:
	/* the address */
	ldr	r1, [r0], #4
	/* the value */
	ldr	r3, [r0], #4
	str	r3, [r1]
	cmp	r2, r0
	bne	pllloop
	/* delay - this is all done by guess */
	ldr	r0, =0x00010000
	/* (vs reading PMC_SR for LOCKA, LOCKB ... or MOSCS earlier) */
lock:
	subs	r0, r0, #1
	bhi	lock
	ldr	r0, =SMRDATA1
	ldr	r1, _MTEXT_BASE
	sub	r0, r0, r1
	ldr	r2, =SMRDATA1E
	sub	r2, r2, r1
sdinit:
	/* the address */
	ldr	r1, [r0], #4
	/* the value */
	ldr	r3, [r0], #4
	str	r3, [r1]
	cmp	r2, r0
	bne	sdinit

	/* switch from FastBus to Asynchronous clock mode */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #ARM920T_CONTROL
	mcr	p15, 0, r0, c1, c0, 0

	/* everything is fine now */
	mov	pc, lr

	.ltorg

SMRDATA:
	.word AT91_ASM_MC_EBI_CFG
	.word CONFIG_SYS_EBI_CFGR_VAL
	.word AT91_ASM_MC_SMC_CSR0
	.word CONFIG_SYS_SMC_CSR0_VAL
	.word AT91_ASM_PMC_PLLAR
	.word CONFIG_SYS_PLLAR_VAL
	.word AT91_ASM_PMC_PLLBR
	.word CONFIG_SYS_PLLBR_VAL
	.word AT91_ASM_PMC_MCKR
	.word CONFIG_SYS_MCKR_VAL
SMRDATAE:
	/* here there's a delay */
SMRDATA1:
	.word AT91_ASM_PIOC_ASR
	.word CONFIG_SYS_PIOC_ASR_VAL
	.word AT91_ASM_PIOC_BSR
	.word CONFIG_SYS_PIOC_BSR_VAL
	.word AT91_ASM_PIOC_PDR
	.word CONFIG_SYS_PIOC_PDR_VAL
	.word AT91_ASM_MC_EBI_CSA
	.word CONFIG_SYS_EBI_CSA_VAL
	.word AT91_ASM_MC_SDRAMC_CR
	.word CONFIG_SYS_SDRC_CR_VAL
	.word AT91_ASM_MC_SDRAMC_MR
	.word CONFIG_SYS_SDRC_MR_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word AT91_ASM_MC_SDRAMC_MR
	.word CONFIG_SYS_SDRC_MR_VAL1
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word AT91_ASM_MC_SDRAMC_MR
	.word CONFIG_SYS_SDRC_MR_VAL2
	.word CONFIG_SYS_SDRAM1
	.word CONFIG_SYS_SDRAM_VAL
	.word AT91_ASM_MC_SDRAMC_TR
	.word CONFIG_SYS_SDRC_TR_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word AT91_ASM_MC_SDRAMC_MR
	.word CONFIG_SYS_SDRC_MR_VAL3
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
SMRDATA1E:
	/* SMRDATA1 is 176 bytes long */
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */
