/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 */
#ifndef __IMX6_SPL_CONFIG_H
#define __IMX6_SPL_CONFIG_H

#ifdef CONFIG_SPL
/*
 * see Figure 8-3 in IMX6DQ/IMX6SDL Reference manuals:
 *  - IMX6SDL OCRAM (IRAM) is from 0x00907000 to 0x0091FFFF
 *  - IMX6DQ has 2x IRAM of IMX6SDL but we intend to support IMX6SDL as well
 *  - BOOT ROM stack is at 0x0091FFB8
 *  - if icache/dcache is enabled (eFuse/strapping controlled) then the
 *    IMX BOOT ROM will setup MMU table at 0x00918000, therefore we need to
 *    fit between 0x00907000 and 0x00918000.
 *  - Additionally the BOOT ROM loads what they consider the firmware image
 *    which consists of a 4K header in front of us that contains the IVT, DCD
 *    and some padding thus 'our' max size is really 0x00908000 - 0x00918000
 *    or 64KB
 */
#define CONFIG_SPL_MAX_SIZE		0x10000
#define CONFIG_SPL_STACK		0x0091FFB8
/*
 * Pad SPL to 68KB (4KB header + 64KB max size). This allows to write the
 * SPL/U-Boot combination generated with u-boot-with-spl.imx directly to a
 * boot media (given that boot media specific offset is configured properly).
 */
#define CONFIG_SPL_PAD_TO		0x11000

/* MMC support */
#if defined(CONFIG_SPL_MMC_SUPPORT)
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SYS_MONITOR_LEN			409600	/* 400 KB */
#endif

/* SATA support */
#if defined(CONFIG_SPL_SATA_SUPPORT)
#define CONFIG_SPL_SATA_BOOT_DEVICE		0
#define CONFIG_SYS_SATA_FAT_BOOT_PARTITION	1
#endif

/* Define the payload for FAT/EXT support */
#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
# ifdef CONFIG_OF_CONTROL
#  define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot-dtb.img"
# else
#  define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"
# endif
#endif

#if defined(CONFIG_MX6SX) || defined(CONFIG_MX6SL) || \
	defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
#define CONFIG_SPL_BSS_START_ADDR      0x88200000
#define CONFIG_SPL_BSS_MAX_SIZE        0x100000		/* 1 MB */
#define CONFIG_SYS_SPL_MALLOC_START    0x88300000
#define CONFIG_SYS_SPL_MALLOC_SIZE     0x100000		/* 1 MB */
#else
#define CONFIG_SPL_BSS_START_ADDR	0x18200000
#define CONFIG_SPL_BSS_MAX_SIZE		0x100000	/* 1 MB */
#define CONFIG_SYS_SPL_MALLOC_START	0x18300000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000	/* 1 MB */
#endif
#endif

#endif
