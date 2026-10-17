/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Linaro
 * Linus Walleij <linus.walleij@linaro.org>
 * Common ARM Integrator configuration settings
 */

#define CFG_SYS_TIMERBASE		0x13000100	/* Timer1 */

/*
 * The ARM boot monitor initializes the board.
 * However, the default U-Boot code also performs the initialization.
 * If desired, this can be prevented by defining SKIP_LOWLEVEL_INIT
 * - see documentation supplied with board for details of how to choose the
 * image to run at reset/power up
 * e.g. whether the ARM Boot Monitor runs before U-Boot
 */

/*
 * The ARM boot monitor does not relocate U-Boot.
 * However, the default U-Boot code performs the relocation check,
 * and may relocate the code if the memory map is changed.
 * If necessary this can be prevented by defining SKIP_RELOCATE_UBOOT
 */
/* #define SKIP_CONFIG_RELOCATE_UBOOT */

/*
 * Physical Memory Map
 */
#define PHYS_SDRAM_1		0x00000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x08000000	/* 128 MB */
#define CFG_SYS_SDRAM_BASE	PHYS_SDRAM_1

/*
 * FLASH and environment organization
 * Top varies according to amount fitted
 * Reserve top 4 blocks of flash
 * - ARM Boot Monitor
 * - Unused
 * - SIB block
 * - U-Boot environment
 */
#define CFG_SYS_FLASH_BASE		0x24000000

/* Timeout values in ticks */
