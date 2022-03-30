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
 * parameters for the ahbc controller
 */
#define AHBC_CR_A		(CONFIG_FTAHBC020S_BASE + FTAHBC020S_CR)
#define AHBC_BSR6_A	(CONFIG_FTAHBC020S_BASE + FTAHBC020S_SLAVE_BSR_6)

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

	/*
	 * mem_init:
	 *	There are 2 bank connected to FTSMC020 on AG101
	 *	BANK0: FLASH/ROM (SW5, J16), BANK1: OnBoard SDRAM.
	 *	we need to set onboard SDRAM before remap and relocation.
	 */
	led	0x01

  /*
   * for Orca and Emerald
   * disable write protection and reset bank size
   */
	li	$r0, SMC_BANK0_CR_A
	lwi $r1, [$r0+#0x00]
	ori $r1, $r1, 0x8f0
	xori $r1, $r1, 0x8f0
	/* check board */
	li      $r3, CONFIG_FTPMU010_BASE + BOARD_ID_REG
  lwi     $r3, [$r3]
  li      $r4, BOARD_ID_FAMILY_MASK
  and     $r3, $r3, $r4
  li      $r4, BOARD_ID_FAMILY_K7
  xor     $r4, $r3, $r4
  beqz    $r4, use_flash_16bit_boot
	/* 32-bit mode */
use_flash_32bit_boot:
	ori     $r1, $r1, 0x50
  li      $r2, 0x00151151
  j       sdram_b0_cr
	/* 16-bit mode */
use_flash_16bit_boot:
  ori     $r1, $r1, 0x60
  li      $r2, 0x00153153
	/* SRAM bank0 config */
sdram_b0_cr:
  swi     $r1, [$r0+#0x00]
  swi     $r2, [$r0+#0x04]

	/* config AHB Controller */
	led	0x02

	/*
	 * config PMU controller
	 */
	/* ftpmu010_dlldis_disable, must do it in lowleve_init */
	led	0x03
	setbf32	PMU_PDLLCR0_A, FTPMU010_PDLLCR0_DLLDIS		! 0x00010000

	/*
	 * config SDRAM controller
	 */
	led	0x04
	write32	SDMC_TP1_A, SDMC_TP1_D				! 0x00011312
	led	0x05
	write32	SDMC_TP2_A, SDMC_TP2_D				! 0x00480180
	led	0x06
	write32	SDMC_CR1_A, SDMC_CR1_D				! 0x00002326

	led	0x07
	write32	SDMC_CR2_A, FTSDMC021_CR2_IPREC			! 0x00000010
	wait_sdram

	led	0x08
	write32	SDMC_CR2_A, FTSDMC021_CR2_ISMR			! 0x00000004
	wait_sdram

	led	0x09
	write32	SDMC_CR2_A, FTSDMC021_CR2_IREF			! 0x00000008
	wait_sdram

	led	0x0a
	move	$lp, $r11
	ret


#ifndef CONFIG_SKIP_LOWLEVEL_INIT

.globl	lowlevel_init
lowlevel_init:
	move	$r10, $lp
	led	0x10
	jal	remap
#if (defined(NDS32_EXT_FPU_DP) || defined(NDS32_EXT_FPU_SP))
	led	0x1f
	jal	enable_fpu
#endif
	led	0x20
	ret	$r10

remap:
	move	$r11, $lp
#ifdef __NDS32_N1213_43U1H__	/* NDS32 V0 ISA - AG101 Only */
	bal	2f
relo_base:
	move	$r0, $lp
#else
relo_base:
	mfusr	$r0, $pc
#endif /* __NDS32_N1213_43U1H__ */

	/* Remapping */
	led	0x1a
	write32	SDMC_B0_BSR_A, SDMC_B0_BSR_D		! 0x00001800
	write32	SDMC_B1_BSR_A, SDMC_B1_BSR_D		! 0x00001880

	/* clear empty BSR registers */
	led	0x1b
	li	$r4, CONFIG_FTSDMC021_BASE
	li	$r5, 0x0
	swi	$r5, [$r4 + FTSDMC021_BANK2_BSR]
	swi	$r5, [$r4 + FTSDMC021_BANK3_BSR]

#ifdef CONFIG_MEM_REMAP
	/*
	 * Copy ROM code to SDRAM base for memory remap layout.
	 * This is not the real relocation, the real relocation is the function
	 * relocate_code() is start.S which supports the systems is memory
	 * remapped or not.
	 */
	/*
	 * Doing memory remap is essential for preparing some non-OS or RTOS
	 * applications.
	 *
	 * This is also a must on ADP-AG101 board.
	 * The reason is because the ROM/FLASH circuit on PCB board.
	 * AG101-A0 board has 2 jumpers MA17 and SW5 to configure which
	 * ROM/FLASH is used to boot.
	 *
	 * When SW5 = "0101", MA17 = LO, the ROM is connected to BANK0,
	 * and the FLASH is connected to BANK1.
	 * When SW5 = "1010", MA17 = HI, the ROM is disabled (still at BANK0),
	 * and the FLASH is connected to BANK0.
	 * It will occur problem when doing flash probing if the flash is at
	 * BANK0 (0x00000000) while memory remapping was skipped.
	 *
	 * Other board like ADP-AG101P may not enable this since there is only
	 * a FLASH connected to bank0.
	 */
	led	0x11
   /*
    * for Orca and Emerald
    * read sdram base address automatically
    */
	li	$r5, AHBC_BSR6_A
	lwi $r8, [$r5]
	li	$r4, 0xfff00000   /* r4 = bank6 base */
	and $r4, $r4, $r8

	la	$r5, _start@GOTOFF
	la  $r6, _end@GOTOFF
1:
	lwi.p	$r7, [$r5], #4
	swi.p	$r7, [$r4], #4
	blt	$r5, $r6, 1b

	/* set remap bit */
	/*
	 * MEM remap bit is operational
	 * - use it to map writeable memory at 0x00000000, in place of flash
	 * - before remap: flash/rom 0x00000000, sdram: 0x10000000-0x4fffffff
	 * - after  remap: flash/rom 0x80000000, sdram: 0x00000000
	 */
	led	0x1c
	write32 SDMC_B0_BSR_A, 0x00001000
	write32 SDMC_B1_BSR_A, 0x00001200
	li $r5, CONFIG_SYS_TEXT_BASE	/* flash base address */
	add $r11, $r11, $r5	/* add flash address offset for ret */
	add $r10, $r10, $r5
	move $lp, $r11
	setbf15	AHBC_CR_A, FTAHBC020S_CR_REMAP		! 0x1

  /*
   * for Orca and Emerald
   * extend sdram size from 256MB to 2GB
   */
	li	$r5, AHBC_BSR6_A
	lwi $r6, [$r5]
	li  $r4, 0xfff0ffff
	and $r6 ,$r4, $r6
	li	$r4, 0x000b0000
	or  $r6, $r4, $r6
	swi	$r6, [$r5]

  /*
   * for Orca and Emerald
   * extend rom base from 256MB to 2GB
   */
	li	$r4, AHBC_BSR4_A
	lwi $r5, [$r4]
	li	$r6, 0xffffff
	and $r5, $r5, $r6
	li  $r6, 0x80000000
	or  $r5, $r5, $r6
	swi $r5,	[$r4]
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

.globl show_led
show_led:
    li      $r8, (CONFIG_DEBUG_LED)
    swi     $r7, [$r8]
    ret
#endif /* #ifndef CONFIG_SKIP_LOWLEVEL_INIT */
