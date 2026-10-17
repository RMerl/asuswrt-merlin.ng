/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the AT91SAM9RLEK board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/hardware.h>

/* ARM asynchronous clock */
#define CFG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CFG_SYS_AT91_MAIN_CLOCK	12000000	/* main clock xtal */

/* SDRAM */
#define CFG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CFG_SYS_SDRAM_SIZE		0x04000000

#define CFG_SYS_INIT_RAM_SIZE	(16 * 1024)
#define CFG_SYS_INIT_RAM_ADDR	ATMEL_BASE_SRAM

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE			ATMEL_BASE_CS3
/* our ALE is AD21 */
#define CFG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CFG_SYS_NAND_MASK_CLE		(1 << 22)
#define CFG_SYS_NAND_ENABLE_PIN		AT91_PIN_PB6
#define CFG_SYS_NAND_READY_PIN		AT91_PIN_PD17

#endif

#endif
