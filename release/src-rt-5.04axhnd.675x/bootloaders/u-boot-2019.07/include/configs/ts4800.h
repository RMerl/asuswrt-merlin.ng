/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Savoir-faire Linux Inc.
 *
 * Derived from MX51EVK code by
 *   Guennadi Liakhovetski <lg@denx.de>
 *   Freescale Semiconductor, Inc.
 *
 * Configuration settings for the TS4800 Board
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */

#define CONFIG_SKIP_LOWLEVEL_INIT	/* U-Boot is a 2nd stage bootloader */

#define CONFIG_HW_WATCHDOG

#define CONFIG_MACH_TYPE	MACH_TYPE_TS48XX

/* text base address used when linking */

#include <asm/arch/imx-regs.h>

/* enable passing of ATAGs */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(10 * 1024 * 1024)

/*
 * Hardware drivers
 */

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE

/*
 * MMC Configs
 * */
#define CONFIG_SYS_FSL_ESDHC_ADDR	MMC_SDHC1_BASE_ADDR

/*
 * Eth Configs
 */
#define CONFIG_PHY_SMSC

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE	        FEC_BASE_ADDR
#define CONFIG_ETHPRIME		"FEC"
#define CONFIG_FEC_MXC_PHYADDR	0

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE		/* disable vendor parameters protection (serial#, ethaddr) */

/***********************************************************
 * Command definition
 ***********************************************************/

/* Environment variables */


#define CONFIG_LOADADDR		0x91000000	/* loadaddr env var */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"fdt_file=imx51-ts4800.dtb\0" \
	"fdt_addr=0x90fe0000\0" \
	"mmcdev=0\0" \
	"mmcpart=2\0" \
	"mmcroot=/dev/mmcblk0p3 rootwait rw\0" \
	"mmcargs=setenv bootargs root=${mmcroot}\0" \
	"addtty=setenv bootargs ${bootargs} console=ttymxc0,${baudrate}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image};\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs addtty; " \
		"if run loadfdt; then " \
			"bootz ${loadaddr} - ${fdt_addr}; " \
		"else " \
			"echo ERR: cannot load FDT; " \
		"fi; "


#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loadimage; then " \
				"run mmcboot; " \
			"fi; " \
		"fi; " \
	"fi; "

/*
 * Miscellaneous configurable options
 */

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(256 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CONFIG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Low level init */
#define CONFIG_SYS_DDR_CLKSEL	0
#define CONFIG_SYS_CLKTL_CBCDR	0x59E35100
#define CONFIG_SYS_MAIN_PWR_ON

/*-----------------------------------------------------------------------
 * Environment organization
 */

#define CONFIG_ENV_OFFSET      (6 * 64 * 1024)
#define CONFIG_ENV_SIZE        (8 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV 0

#endif
