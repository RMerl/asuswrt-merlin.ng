/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common board functions for siemens AT91SAM9G45 based boards
 * (C) Copyright 2013 Siemens AG
 *
 * Based on:
 * U-Boot file: include/configs/at91sam9m10g45ek.h
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/hardware.h>
#include <linux/sizes.h>

/*
 * Warning: changing CONFIG_TEXT_BASE requires
 * adapting the initial boot program.
 * Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */

/* ARM asynchronous clock */
#define CFG_SYS_AT91_SLOW_CLOCK      32768
#define CFG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */

/* serial console */
#define CFG_USART_BASE		ATMEL_BASE_DBGU
#define CFG_USART_ID			ATMEL_ID_SYS

/* SDRAM */
#define CFG_SYS_SDRAM_BASE           ATMEL_BASE_CS6
#define CFG_SYS_SDRAM_SIZE		0x08000000

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE			ATMEL_BASE_CS3
/* our ALE is AD21 */
#define CFG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CFG_SYS_NAND_MASK_CLE		(1 << 22)
#define CFG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CFG_SYS_NAND_READY_PIN		AT91_PIN_PC8
#endif

/* DFU class support */
#define DFU_MANIFEST_POLL_TIMEOUT	25000

/* bootstrap + u-boot + env in nandflash */

/* Defines for SPL */

#define CFG_SYS_NAND_U_BOOT_SIZE	0x80000
#define	CFG_SYS_NAND_U_BOOT_START	CONFIG_TEXT_BASE
#define CFG_SYS_NAND_U_BOOT_DST	CONFIG_TEXT_BASE

#define CFG_SYS_NAND_ECCSIZE		256
#define CFG_SYS_NAND_ECCBYTES	3
#define CFG_SYS_NAND_ECCPOS		{ 40, 41, 42, 43, 44, 45, 46, 47, \
					  48, 49, 50, 51, 52, 53, 54, 55, \
					  56, 57, 58, 59, 60, 61, 62, 63, }

#define CFG_SYS_MASTER_CLOCK		132096000
#define AT91_PLL_LOCK_TIMEOUT		1000000
#define CFG_SYS_AT91_PLLA		0x20c73f03
#define CFG_SYS_MCKR			0x1301
#define CFG_SYS_MCKR_CSS		0x1302

#endif
