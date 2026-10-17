/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2009-2015
 * Daniel Gorsulowski <daniel.gorsulowski@esd.eu>
 * esd electronic system design gmbh <www.esd.eu>
 *
 * Configuation settings for the esd MEESC board.
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
#define CFG_SYS_AT91_SLOW_CLOCK	32768	/* 32.768 kHz crystal */
#define CFG_SYS_AT91_MAIN_CLOCK	16000000/* 16.0 MHz crystal */

/* Misc CPU related */

/*
 * Hardware drivers
 */

/*
 * SDRAM: 1 bank, min 32, max 128 MB
 * Initialized before u-boot gets started.
 */
#define PHYS_SDRAM					ATMEL_BASE_CS1 /* 0x20000000 */
#define PHYS_SDRAM_SIZE				0x02000000     /* 32 MByte */

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_SDRAM_SIZE		PHYS_SDRAM_SIZE

#define CFG_SYS_INIT_RAM_ADDR	ATMEL_BASE_SRAM0
#define CFG_SYS_INIT_RAM_SIZE	(16 * 1024)

/* NAND flash */
#ifdef CONFIG_CMD_NAND
# define CFG_SYS_NAND_BASE			ATMEL_BASE_CS3 /* 0x40000000 */
# define CFG_SYS_NAND_MASK_ALE		(1 << 21)
# define CFG_SYS_NAND_MASK_CLE		(1 << 22)
# define CFG_SYS_NAND_ENABLE_PIN		GPIO_PIN_PD(15)
# define CFG_SYS_NAND_READY_PIN		GPIO_PIN_PA(22)
#endif

/* hw-controller addresses */
#define CFG_ET1100_BASE		0x70000000

#endif
