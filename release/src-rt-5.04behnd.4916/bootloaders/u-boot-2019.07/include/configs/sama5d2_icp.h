/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for the SAMA5D2 ICP Board.
 *
 * Copyright (C) 2018 Microchip Corporation
 *		      Eugen Hristev <eugen.hristev@microchip.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "at91-sama5_common.h"

#undef CONFIG_SYS_AT91_MAIN_CLOCK
#define CONFIG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */

#define CONFIG_MISC_INIT_R

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x20000000

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_INIT_SP_ADDR		0x218000
#else
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 - GENERATED_GBL_DATA_SIZE)
#endif

#define CONFIG_SYS_LOAD_ADDR		0x22000000 /* load address */

/* NAND flash */
#undef CONFIG_CMD_NAND

/* SPI flash */
#define CONFIG_SF_DEFAULT_SPEED		66000000

#undef CONFIG_BOOTCOMMAND
#ifdef CONFIG_SD_BOOT
/* u-boot env in sd/mmc card */
#define FAT_ENV_INTERFACE	"mmc"
#define FAT_ENV_DEVICE_AND_PART	"0"
#define FAT_ENV_FILE		"uboot.env"
#define CONFIG_ENV_SIZE		0x4000
/* bootstrap + u-boot + env in sd card */
#define CONFIG_BOOTCOMMAND	"fatload mmc 0:1 0x21000000 at91-sama5d2_icp.dtb; " \
				"fatload mmc 0:1 0x22000000 zImage; " \
				"bootz 0x22000000 - 0x21000000"
#undef CONFIG_BOOTARGS
#define CONFIG_BOOTARGS \
	"console=ttyS0,115200 earlyprintk root=/dev/mmcblk0p2 rw rootwait"
#endif

/* SPL */
#define CONFIG_SPL_MAX_SIZE		0x10000
#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#ifdef CONFIG_SD_BOOT
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"
#endif

#endif
