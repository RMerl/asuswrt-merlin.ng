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
 * Warning: changing CONFIG_SYS_TEXT_BASE requires
 * adapting the initial boot program.
 * Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768	/* 32.768 kHz crystal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	16000000/* 16.0 MHz crystal */

/* Misc CPU related */
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SERIAL_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */

#define CONFIG_PREBOOT				/* enable preboot variable */

/*
 * Hardware drivers
 */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * SDRAM: 1 bank, min 32, max 128 MB
 * Initialized before u-boot gets started.
 */
#define PHYS_SDRAM					ATMEL_BASE_CS1 /* 0x20000000 */
#define PHYS_SDRAM_SIZE				0x02000000     /* 32 MByte */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_SDRAM_SIZE		PHYS_SDRAM_SIZE

#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + 0x00100000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x01E00000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x00100000)

/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above
 * that address while providing maximum stack area below.
 */
#define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM0 + 16 * 1024 - GENERATED_GBL_DATA_SIZE)

/* NAND flash */
#ifdef CONFIG_CMD_NAND
# define CONFIG_SYS_MAX_NAND_DEVICE		1
# define CONFIG_SYS_NAND_BASE			ATMEL_BASE_CS3 /* 0x40000000 */
# define CONFIG_SYS_NAND_DBW_8
# define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
# define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
# define CONFIG_SYS_NAND_ENABLE_PIN		GPIO_PIN_PD(15)
# define CONFIG_SYS_NAND_READY_PIN		GPIO_PIN_PA(22)
#endif

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT			20
#undef CONFIG_RESET_PHY_R

/* hw-controller addresses */
#define CONFIG_ET1100_BASE		0x70000000

#ifdef CONFIG_SYS_USE_DATAFLASH

/* bootstrap + u-boot + env in dataflash on CS0 */
#define CONFIG_ENV_OFFSET	0x4200
#define CONFIG_ENV_SIZE		0x4200
#define CONFIG_ENV_SECT_SIZE	0x210

#elif CONFIG_SYS_USE_NANDFLASH

/* bootstrap + u-boot + env + linux in nandflash */
# define CONFIG_ENV_OFFSET		0xC0000
# define CONFIG_ENV_SIZE		0x20000

#endif

#define CONFIG_SYS_CBSIZE		512

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		ROUND(3 * CONFIG_ENV_SIZE + \
					128*1024, 0x1000)

#endif
