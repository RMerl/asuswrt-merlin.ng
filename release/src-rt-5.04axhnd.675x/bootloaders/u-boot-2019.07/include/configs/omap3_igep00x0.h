/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common configuration settings for IGEP technology based boards
 *
 * (C) Copyright 2012
 * ISEE 2007 SL, <www.iseebcn.com>
 */

#ifndef __IGEP00X0_H
#define __IGEP00X0_H

#include <configs/ti_omap3_common.h>

/*
 * We are only ever GP parts and will utilize all of the "downloaded image"
 * area in SRAM which starts at 0x40200000 and ends at 0x4020FFFF (64KB).
 */

#define CONFIG_REVISION_TAG		1

/* TPS65950 */
#define PBIASLITEVMODE1			(1 << 8)

/* LED */
#define IGEP0020_GPIO_LED		27
#define IGEP0030_GPIO_LED		16

/* Board and revision detection GPIOs */
#define IGEP0030_USB_TRANSCEIVER_RESET		54
#define GPIO_IGEP00X0_BOARD_DETECTION		28
#define GPIO_IGEP00X0_REVISION_DETECTION	129

#ifndef CONFIG_SPL_BUILD

/* Environment */
#define ENV_DEVICE_SETTINGS \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0"

#define MEM_LAYOUT_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	"scriptaddr=0x87E00000\0" \
	"pxefile_addr_r=0x87F00000\0"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>

#define ENV_FINDFDT \
	"findfdt="\
		"if test ${board_name} = igep0020; then " \
			"if test ${board_rev} = F; then " \
				"setenv fdtfile omap3-igep0020-rev-f.dtb; " \
			"else " \
				"setenv fdtfile omap3-igep0020.dtb; fi; fi; " \
		"if test ${board_name} = igep0030; then " \
			"if test ${board_rev} = G; then " \
				"setenv fdtfile omap3-igep0030-rev-g.dtb; " \
			"else " \
				"setenv fdtfile omap3-igep0030.dtb; fi; fi; " \
		"if test ${fdtfile} = ''; then " \
			"echo WARNING: Could not determine device tree to use; fi; \0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_FINDFDT \
	ENV_DEVICE_SETTINGS \
	MEM_LAYOUT_SETTINGS \
	BOOTENV

#endif

#define CONFIG_SYS_MTDPARTS_RUNTIME

/* OneNAND config */
#define CONFIG_USE_ONENAND_BOARD_INIT
#define CONFIG_SYS_ONENAND_BASE		ONENAND_MAP
#define CONFIG_SYS_ONENAND_BLOCK_SIZE	(128*1024)

/* NAND config */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{ 2,  3,  4,  5,  6,  7,  8,  9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW_DETECTION_SW

/* UBI configuration */
#define CONFIG_SPL_UBI			1
#define CONFIG_SPL_UBI_MAX_VOL_LEBS	256
#define CONFIG_SPL_UBI_MAX_PEB_SIZE	(256*1024)
#define CONFIG_SPL_UBI_MAX_PEBS		4096
#define CONFIG_SPL_UBI_VOL_IDS		8
#define CONFIG_SPL_UBI_LOAD_MONITOR_ID	0
#define CONFIG_SPL_UBI_LOAD_KERNEL_ID	3
#define CONFIG_SPL_UBI_LOAD_ARGS_ID	4
#define CONFIG_SPL_UBI_PEB_OFFSET	4
#define CONFIG_SPL_UBI_VID_OFFSET	512
#define CONFIG_SPL_UBI_LEB_START	2048
#define CONFIG_SPL_UBI_INFO_ADDR	0x88080000

/* environment organization */
#define CONFIG_ENV_UBI_PART		"UBI"
#define CONFIG_ENV_UBI_VOLUME		"config"
#define CONFIG_ENV_UBI_VOLUME_REDUND	"config_r"
#define CONFIG_ENV_SIZE			(32*1024)

#endif /* __IGEP00X0_H */
