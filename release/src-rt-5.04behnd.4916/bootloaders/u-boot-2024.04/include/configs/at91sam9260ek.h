/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the AT91SAM9260EK & AT91SAM9G20EK boards.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>

/*
 * Warning: changing CONFIG_TEXT_BASE requires
 * adapting the initial boot program.
 * Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */

/* ARM asynchronous clock */
#define CFG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CFG_SYS_AT91_MAIN_CLOCK	18432000	/* main clock xtal */

/*
 * SDRAM: 1 bank, min 32, max 128 MB
 * Initialized before u-boot gets started.
 */
#define CFG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CFG_SYS_SDRAM_SIZE		0x04000000

#define CFG_SYS_INIT_RAM_SIZE	(16 * 1024)
#ifdef CONFIG_AT91SAM9XE
# define CFG_SYS_INIT_RAM_ADDR	ATMEL_BASE_SRAM
#else
# define CFG_SYS_INIT_RAM_ADDR	ATMEL_BASE_SRAM1
#endif

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CFG_SYS_NAND_MASK_ALE	(1 << 21)
#define CFG_SYS_NAND_MASK_CLE	(1 << 22)
#define CFG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CFG_SYS_NAND_READY_PIN	AT91_PIN_PC13
#endif

/* USB */
#define CFG_SYS_USB_OHCI_REGS_BASE		0x00500000	/* AT91SAM9260_UHP_BASE */

#endif
