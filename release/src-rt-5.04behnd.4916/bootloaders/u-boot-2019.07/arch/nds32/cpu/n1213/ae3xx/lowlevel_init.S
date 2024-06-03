/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

.pic

.text

#include <common.h>
#include <config.h>

#include <asm/macro.h>
#include <generated/asm-offsets.h>

/*
 * parameters for the SDRAM controller
 */
#define SDMC_TP1_A		(CONFIG_FTSDMC021_BASE + FTSDMC021_TP1)
#define SDMC_TP2_A		(CONFIG_FTSDMC021_BASE + FTSDMC021_TP2)
#define SDMC_CR1_A		(CONFIG_FTSDMC021_BASE + FTSDMC021_CR1)
#define SDMC_CR2_A		(CONFIG_FTSDMC021_BASE + FTSDMC021_CR2)
#define SDMC_B0_BSR_A		(CONFIG_FTSDMC021_BASE + FTSDMC021_BANK0_BSR)
#define SDMC_B1_BSR_A		(CONFIG_FTSDMC021_BASE + FTSDMC021_BANK1_BSR)

#define SDMC_TP1_D		CONFIG_SYS_FTSDMC021_TP1
#define SDMC_TP2_D		CONFIG_SYS_FTSDMC021_TP2
#define SDMC_CR1_D		CONFIG_SYS_FTSDMC021_CR1
#define SDMC_CR2_D		CONFIG_SYS_FTSDMC021_CR2

#define SDMC_B0_BSR_D		CONFIG_SYS_FTSDMC021_BANK0_BSR
#define SDMC_B1_BSR_D		CONFIG_SYS_FTSDMC021_BANK1_BSR


/*
 * for Orca and Emerald
 */
#define BOARD_ID_REG		0x104
#define BOARD_ID_FAMILY_MASK 	0xfff000
#define BOARD_ID_FAMILY_V5   	0x556000
#define BOARD_ID_FAMILY_K7   	0x74b000

/*
 * parameters for the static memory controller
 */
#define SMC_BANK0_CR_A		(CONFIG_FTSMC020_BASE + FTSMC020_BANK0_CR)
#define SMC_BANK0_TPR_A		(CONFIG_FTSMC020_BASE + FTSMC020_BANK0_TPR)

#define SMC_BANK0_CR_D		FTSMC020_BANK0_LOWLV_CONFIG
#define SMC_BANK0_TPR_D		FTSMC020_BANK0_LOWLV_TIMING

/*
 * for Orca and Emerald
 */
#define AHBC_BSR4_A	(CONFIG_FTAHBC020S_BASE + FTAHBC020S_SLAVE_BSR_4)
#define AHBC_BSR6_D		CONFIG_SYS_FTAHBC020S_SLAVE_BSR_6

/*
 * parameters for the pmu controoler
 */
#define PMU_PDLLCR0_A		(CONFIG_FTPMU010_BASE + FTPMU010_PDLLCR0)

/*
 * numeric 7 segment display
 */
.macro	led, num
	write32	CONFIG_DEBUG_LED, \num
.endm

/*
 * Waiting for SDRAM to set up
 */
.macro	wait_sdram
	li	$r0, CONFIG_FTSDMC021_BASE
1:
	lwi	$r1, [$r0+FTSDMC021_CR2]
	bnez	$r1, 1b
.endm

.globl	mem_init
mem_init:
	move	$r11, $lp
	li	$r0, SMC_BANK0_CR_A
	lwi $r1, [$r0+#0x00]
	ori $r1, $r1, 0x8f0
	xori $r1, $r1, 0x8f0
	/* 16-bit mode */
	ori	  $r1, $r1, 0x60
	li	  $r2, 0x00153153
	swi	$r1, [$r0+#0x00]
	swi	  $r2, [$r0+#0x04]
	move	  $lp, $r11
	ret

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
.globl	lowlevel_init
lowlevel_init:
	move	$r10, $lp
	jal	remap

#if (defined(NDS32_EXT_FPU_DP) || defined(NDS32_EXT_FPU_SP))
	jal	enable_fpu
#endif
	ret	$r10

remap:
	move	$r11, $lp
relo_base:
	mfusr	$r0, $pc

#ifdef CONFIG_MEM_REMAP
	li	$r4, 0x00000000
	li	$r5, 0x80000000
	la  $r6, _end@GOTOFF
1:
	lmw.bim	$r12, [$r5], $r19
	smw.bim	$r12, [$r4], $r19
	blt	$r5, $r6, 1b
#endif /* #ifdef CONFIG_MEM_REMAP */
	move $lp, $r11
2:
	ret

	/*
	 * enable_fpu:
	 *  Some of Andes CPU version support FPU coprocessor, if so,
	 *  and toolchain support FPU instruction set, we should enable it.
	 */
#if (defined(NDS32_EXT_FPU_DP) || defined(NDS32_EXT_FPU_SP))
enable_fpu:
	mfsr    $r0, $CPU_VER     /* enable FPU if it exists */
	srli    $r0, $r0, 3
	andi    $r0, $r0, 1
	beqz    $r0, 1f           /* skip if no COP */
	mfsr    $r0, $FUCOP_EXIST
	srli    $r0, $r0, 31
	beqz    $r0, 1f           /* skip if no FPU */
	mfsr    $r0, $FUCOP_CTL
	ori     $r0, $r0, 1
	mtsr    $r0, $FUCOP_CTL
1:
	ret
#endif

#endif /* #ifndef CONFIG_SKIP_LOWLEVEL_INIT */
