/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Samsung Electronics
 * Hyungwon Hwang <human.hwang@samsung.com>
 */

#ifndef __CONFIG_ODROID_XU3_H
#define __CONFIG_ODROID_XU3_H

#include <configs/exynos5420-common.h>
#include <configs/exynos5-common.h>

#define CFG_SYS_SDRAM_BASE		0x40000000

#define TZPC_BASE_OFFSET		0x10000

#define SDRAM_BANK_SIZE			(256UL << 20UL)	/* 256 MB */

/* DFU */
#define DFU_DEFAULT_POLL_TIMEOUT	300
#define DFU_MANIFEST_POLL_TIMEOUT	25000

#define CFG_DFU_ALT_SYSTEM               \
	"uImage fat 0 1;"                   \
	"zImage fat 0 1;"                   \
	"Image.itb fat 0 1;"                \
	"uInitrd fat 0 1;"                  \
	"boot.scr fat 0 1;"                 \
	"boot.cmd fat 0 1;"                 \
	"exynos5422-odroidxu3.dtb fat 0 1;" \
	"exynos5422-odroidxu3-lite.dtb fat 0 1;" \
	"exynos5422-odroidxu4.dtb fat 0 1;" \
	"exynos5422-odroidhc1.dtb fat 0 1;" \
	"boot part 0 1;"                    \
	"root part 0 2\0"

#define CFG_DFU_ALT_BOOT_EMMC           \
	"u-boot raw 0x3e 0x800 mmcpart 1;" \
	"bl1 raw 0x0 0x1e mmcpart 1;"      \
	"bl2 raw 0x1e 0x1d mmcpart 1;"     \
	"tzsw raw 0x83e 0x200 mmcpart 1;"  \
	"params.bin raw 0x1880 0x20\0"

#define CFG_DFU_ALT_BOOT_SD   \
	"u-boot raw 0x3f 0x800;" \
	"bl1 raw 0x1 0x1e;"      \
	"bl2 raw 0x1f 0x1d;"     \
	"tzsw raw 0x83f 0x200;"  \
	"params.bin raw 0x1880 0x20\0"

/* Enable: board/samsung/common/misc.c to use set_dfu_alt_info() */
#define CFG_SET_DFU_ALT_BUF_LEN	(SZ_1K)

/* Set soc_rev, soc_id, board_rev, board_name, fdtfile */
#define CFG_ODROID_REV_AIN		9

/*
 * Need to override existing one (smdk5420) with odroid so set_board_info will
 * use proper prefix when creating full board_name (SYS_BOARD + type)
 */

/* Define new extra env settings, including DFU settings */
#undef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS \
	EXYNOS_DEVICE_SETTINGS \
	EXYNOS_FDTFILE_SETTING \
	MEM_LAYOUT_ENV_SETTINGS \
	BOOTENV \
	"rootfstype=ext4\0" \
	"console=console=ttySAC2,115200n8\0" \
	"fdtfile=exynos5422-odroidxu3.dtb\0" \
	"board=odroid\0" \
	"board_name=odroidxu3\0" \
	"mmcbootdev=0\0" \
	"mmcrootdev=0\0" \
	"mmcbootpart=1\0" \
	"mmcrootpart=2\0" \
	"dfu_alt_system="CFG_DFU_ALT_SYSTEM \
	"dfu_alt_info=Autoset by THOR/DFU command run.\0"

#endif	/* __CONFIG_H */
