/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 */

#include <config.h>
#include <asm/arch/imx-regs.h>
#include <generated/asm-offsets.h>
#include "mx35pdk.h"
#include <asm/arch/lowlevel_macro.S>

/*
 * return soc version
 * 	0x10:  TO1
 *	0x20:  TO2
 *	0x30:  TO3
 */
.macro check_soc_version ret, tmp
	ldr \tmp, =IIM_BASE_ADDR
	ldr \ret, [\tmp, #IIM_SREV]
	cmp \ret, #0x00
	moveq \tmp, #ROMPATCH_REV
	ldreq \ret, [\tmp]
	moveq \ret, \ret, lsl #4
	addne \ret, \ret, #0x10
.endm

/* CPLD on CS5 setup */
.macro init_debug_board
	ldr r0, =DBG_BASE_ADDR
	ldr r1, =DBG_CSCR_U_CONFIG
	str r1, [r0, #0x00]
	ldr r1, =DBG_CSCR_L_CONFIG
	str r1, [r0, #0x04]
	ldr r1, =DBG_CSCR_A_CONFIG
	str r1, [r0, #0x08]
.endm

/* clock setup */
.macro init_clock
	ldr r0, =CCM_BASE_ADDR

	/* default CLKO to 1/32 of the ARM core*/
	ldr r1, [r0, #CLKCTL_COSR]
	bic r1, r1, #0x00000FF00
	bic r1, r1, #0x0000000FF
	mov r2, #0x00006C00
	add r2, r2, #0x67
	orr r1, r1, r2
	str r1, [r0, #CLKCTL_COSR]

	ldr r2, =CCM_CCMR_CONFIG
	str r2, [r0, #CLKCTL_CCMR]

	check_soc_version r1, r2
	cmp r1, #CHIP_REV_2_0
	ldrhs r3, =CCM_MPLL_532_HZ
	bhs 1f
	ldr r2, [r0, #CLKCTL_PDR0]
	tst r2, #CLKMODE_CONSUMER
	ldrne r3, =CCM_MPLL_532_HZ  /* consumer path*/
	ldreq r3, =CCM_MPLL_399_HZ  /* auto path*/
1:
	str r3, [r0, #CLKCTL_MPCTL]

	ldr r1, =CCM_PPLL_300_HZ
	str r1, [r0, #CLKCTL_PPCTL]

	ldr r1, =CCM_PDR0_CONFIG
	bic r1, r1, #0x800000
	str r1, [r0, #CLKCTL_PDR0]

	ldr r1, [r0, #CLKCTL_CGR0]
	orr r1, r1, #0x0C300000
	str r1, [r0, #CLKCTL_CGR0]

	ldr r1, [r0, #CLKCTL_CGR1]
	orr r1, r1, #0x00000C00
	orr r1, r1, #0x00000003
	str r1, [r0, #CLKCTL_CGR1]

	ldr r1, [r0, #CLKCTL_CGR2]
	orr r1, r1, #0x00C00000
	str r1, [r0, #CLKCTL_CGR2]
.endm

.macro setup_sdram
	ldr r0, =ESDCTL_BASE_ADDR
	mov r3, #0x2000
	str r3, [r0, #0x0]
	str r3, [r0, #0x8]

	/*ip(r12) has used to save lr register in upper calling*/
	mov fp, lr

	mov r5, #0x00
	mov r2, #0x00
	mov r1, #CSD0_BASE_ADDR
	bl setup_sdram_bank

	mov r5, #0x00
	mov r2, #0x00
	mov r1, #CSD1_BASE_ADDR
	bl setup_sdram_bank

	mov lr, fp

1:
	ldr r3, =ESDCTL_DELAY_LINE5
	str r3, [r0, #0x30]
.endm

.globl lowlevel_init
lowlevel_init:
	mov r10, lr

	core_init

	init_aips

	init_max

	init_m3if

	init_clock
	init_debug_board

	cmp pc, #PHYS_SDRAM_1
	blo init_sdram_start
	cmp pc, #(PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE)
	blo skip_sdram_setup

init_sdram_start:
	/*init_sdram*/
	setup_sdram

skip_sdram_setup:
	mov lr, r10
	mov pc, lr


/*
 * r0: ESDCTL control base, r1: sdram slot base
 * r2: DDR type(0:DDR2, 1:MDDR) r3, r4:working base
 */
setup_sdram_bank:
	mov r3, #0xE
	tst r2, #0x1
	orreq r3, r3, #0x300 /*DDR2*/
	str r3, [r0, #0x10]
	bic r3, r3, #0x00A
	str r3, [r0, #0x10]
	beq 2f

	mov r3, #0x20000
1:      subs r3, r3, #1
	bne 1b

2:      tst r2, #0x1
	ldreq r3, =ESDCTL_DDR2_CONFIG
	ldrne r3, =ESDCTL_MDDR_CONFIG
	cmp r1, #CSD1_BASE_ADDR
	strlo r3, [r0, #0x4]
	strhs r3, [r0, #0xC]

	ldr r3, =ESDCTL_0x92220000
	strlo r3, [r0, #0x0]
	strhs r3, [r0, #0x8]
	mov r3, #0xDA
	ldr r4, =ESDCTL_PRECHARGE
	strb r3, [r1, r4]

	tst r2, #0x1
	bne skip_set_mode

	cmp r1, #CSD1_BASE_ADDR
	ldr r3, =ESDCTL_0xB2220000
	strlo r3, [r0, #0x0]
	strhs r3, [r0, #0x8]
	mov r3, #0xDA
	ldr r4, =ESDCTL_DDR2_EMR2
	strb r3, [r1, r4]
	ldr r4, =ESDCTL_DDR2_EMR3
	strb r3, [r1, r4]
	ldr r4, =ESDCTL_DDR2_EN_DLL
	strb r3, [r1, r4]
	ldr r4, =ESDCTL_DDR2_RESET_DLL
	strb r3, [r1, r4]

	ldr r3, =ESDCTL_0x92220000
	strlo r3, [r0, #0x0]
	strhs r3, [r0, #0x8]
	mov r3, #0xDA
	ldr r4, =ESDCTL_PRECHARGE
	strb r3, [r1, r4]

skip_set_mode:
	cmp r1, #CSD1_BASE_ADDR
	ldr r3, =ESDCTL_0xA2220000
	strlo r3, [r0, #0x0]
	strhs r3, [r0, #0x8]
	mov r3, #0xDA
	strb r3, [r1]
	strb r3, [r1]

	ldr r3, =ESDCTL_0xB2220000
	strlo r3, [r0, #0x0]
	strhs r3, [r0, #0x8]
	tst r2, #0x1
	ldreq r4, =ESDCTL_DDR2_MR
	ldrne r4, =ESDCTL_MDDR_MR
	mov r3, #0xDA
	strb r3, [r1, r4]
	ldreq r4, =ESDCTL_DDR2_OCD_DEFAULT
	streqb r3, [r1, r4]
	ldreq r4, =ESDCTL_DDR2_EN_DLL
	ldrne r4, =ESDCTL_MDDR_EMR
	strb r3, [r1, r4]

	cmp r1, #CSD1_BASE_ADDR
	ldr r3, =ESDCTL_0x82228080
	strlo r3, [r0, #0x0]
	strhs r3, [r0, #0x8]

	tst r2, #0x1
	moveq r4, #0x20000
	movne r4, #0x200
1:      subs r4, r4, #1
	bne 1b

	str r3, [r1, #0x100]
	ldr r4, [r1, #0x100]
	cmp r3, r4
	movne r3, #1
	moveq r3, #0

	mov pc, lr
