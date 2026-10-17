/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Ilko Iliev <iliev@ronetix.at>
 * Asen Dimov <dimov@ronetix.at>
 * Ronetix GmbH <www.ronetix.at>
 *
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the PM9G45 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* ARM asynchronous clock */
#define CFG_SYS_AT91_SLOW_CLOCK      32768
#define CFG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */

/* SDRAM */
#define CFG_SYS_SDRAM_BASE           0x70000000
#define CFG_SYS_SDRAM_SIZE		0x08000000

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE			ATMEL_BASE_CS3
/* our ALE is AD21 */
#define CFG_SYS_NAND_MASK_ALE		BIT(21)
/* our CLE is AD22 */
#define CFG_SYS_NAND_MASK_CLE		BIT(22)
#define CFG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CFG_SYS_NAND_READY_PIN		AT91_PIN_PD3
#endif

#ifdef CONFIG_NAND_BOOT
/* bootstrap + u-boot + env in nandflash */
#elif CONFIG_SD_BOOT
/* bootstrap + u-boot + env + linux in mmc */
#endif

/* Defines for SPL */

#ifdef CONFIG_SD_BOOT
#elif CONFIG_NAND_BOOT
#define CFG_SYS_NAND_U_BOOT_SIZE	0x80000

#define CFG_SYS_NAND_ECCSIZE		256
#define CFG_SYS_NAND_ECCBYTES	3
#define CFG_SYS_NAND_ECCPOS		{ 40, 41, 42, 43, 44, 45, 46, 47, \
					  48, 49, 50, 51, 52, 53, 54, 55, \
					  56, 57, 58, 59, 60, 61, 62, 63, }
#endif

#define CFG_SYS_MASTER_CLOCK		132096000
#define CFG_SYS_AT91_PLLA		0x20c73f03
#define CFG_SYS_MCKR			0x1301
#define CFG_SYS_MCKR_CSS		0x1302

#endif
