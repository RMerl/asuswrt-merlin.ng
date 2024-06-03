/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SPL definitions for the i.MX7 SPL
 *
 * (C) Copyright 2017 CompuLab, Ltd. http://www.compulab.com
 *
 * Author: Uri Mashiach <uri.mashiach@compulab.co.il>
 */

#ifndef __IMX7_SPL_CONFIG_H
#define __IMX7_SPL_CONFIG_H

#ifdef CONFIG_SPL
/*
 * see figure 6-22 in i.MX 7Dual/Solo Reference manuals:
 *  - IMX7D/S OCRAM free area RAM (OCRAM) is from 0x00910000 to
 *    0x00946C00.
 *  - Set the stack at the end of the free area section, at 0x00946BB8.
 *  - The BOOT ROM loads what they consider the firmware image
 *    which consists of a 4K header in front of us that contains the IVT, DCD
 *    and some padding thus 'our' max size is really 0x00946BB8 - 0x00911000.
 *    64KB is more then enough for the SPL.
 */
#define CONFIG_SPL_MAX_SIZE		0x10000
#define CONFIG_SPL_STACK		0x00946BB8
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

/* Define the payload for FAT/EXT support */
#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
# ifdef CONFIG_OF_CONTROL
#  define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot-dtb.img"
# else
#  define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"
# endif
#endif

#define CONFIG_SPL_BSS_START_ADDR      0x88200000
#define CONFIG_SPL_BSS_MAX_SIZE        0x100000		/* 1 MB */
#define CONFIG_SYS_SPL_MALLOC_START    0x88300000
#define CONFIG_SYS_SPL_MALLOC_SIZE     0x100000		/* 1 MB */

#endif /* CONFIG_SPL */

#endif /* __IMX7_SPL_CONFIG_H */
