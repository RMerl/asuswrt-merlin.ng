/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG/GOOGLE PEACH-PI board.
 */

#ifndef __CONFIG_PEACH_PI_H
#define __CONFIG_PEACH_PI_H

#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"kernel_addr_r=0x22000000\0" \
	"fdt_addr_r=0x23000000\0" \
	"ramdisk_addr_r=0x23300000\0" \
	"scriptaddr=0x30000000\0" \
	"pxefile_addr_r=0x31000000\0"

#include <configs/exynos5420-common.h>
#include <configs/exynos5-dt-common.h>
#include <configs/exynos5-common.h>

#define CONFIG_SYS_SDRAM_BASE	0x20000000
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_IRAM_TOP - 0x800)

/* select serial console configuration */
#define CONFIG_DEFAULT_CONSOLE	"console=ttySAC1,115200n8\0"

/* Display */
#ifdef CONFIG_LCD
#define CONFIG_EXYNOS_FB
#define CONFIG_EXYNOS_DP
#define LCD_BPP			LCD_COLOR16
#endif

#define CONFIG_POWER_TPS65090_EC

/* DRAM Memory Banks */
#define SDRAM_BANK_SIZE		(512UL << 20UL)	/* 512 MB */

#endif	/* __CONFIG_PEACH_PI_H */
