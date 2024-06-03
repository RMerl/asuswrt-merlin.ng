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
 * Warning: changing CONFIG_SYS_TEXT_BASE requires
 * adapting the initial boot program.
 * Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */

#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK      32768
#define CONFIG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT_ONLY

/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */
#define CONFIG_AT91_GPIO
#define CONFIG_AT91_GPIO_PULLUP	1	/* keep pullups on peripheral pins */

/* serial console */
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS

/* LED */
#define CONFIG_AT91_LED
#define CONFIG_RED_LED		AT91_PIN_PD31	/* this is the user1 led */
#define CONFIG_GREEN_LED	AT91_PIN_PD0	/* this is the user2 led */


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE           ATMEL_BASE_CS6
#define CONFIG_SYS_SDRAM_SIZE		0x08000000

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + SZ_32K - GENERATED_GBL_DATA_SIZE)

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PC8
#define CONFIG_SYS_NAND_DRIVER_ECC_LAYOUT
#endif

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_AT91_WANTS_COMMON_PHY

/* DFU class support */
#define CONFIG_SYS_DFU_DATA_BUF_SIZE	(SZ_1M)
#define DFU_MANIFEST_POLL_TIMEOUT	25000

#define CONFIG_SYS_LOAD_ADDR	ATMEL_BASE_CS6

/* bootstrap + u-boot + env in nandflash */
#define CONFIG_ENV_OFFSET		0x100000
#define CONFIG_ENV_OFFSET_REDUND	0x180000
#define CONFIG_ENV_SIZE			SZ_128K

#define CONFIG_BOOTCOMMAND						\
	"nand read 0x70000000 0x200000 0x300000;"			\
	"bootm 0x70000000"

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	ROUND(3 * CONFIG_ENV_SIZE + \
				SZ_4M, 0x1000)

/* Defines for SPL */
#define CONFIG_SPL_MAX_SIZE		(12 * SZ_1K)
#define CONFIG_SPL_STACK		(SZ_16K)

#define CONFIG_SPL_BSS_START_ADDR	CONFIG_SPL_MAX_SIZE
#define CONFIG_SPL_BSS_MAX_SIZE		(SZ_2K)

#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SPL_NAND_RAW_ONLY
#define CONFIG_SPL_NAND_SOFTECC
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x20000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x80000
#define	CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_DST	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_5_ADDR_CYCLE

#define CONFIG_SYS_NAND_PAGE_SIZE	SZ_2K
#define CONFIG_SYS_NAND_BLOCK_SIZE	(SZ_128K)
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCSIZE		256
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_ECCPOS		{ 40, 41, 42, 43, 44, 45, 46, 47, \
					  48, 49, 50, 51, 52, 53, 54, 55, \
					  56, 57, 58, 59, 60, 61, 62, 63, }

#define CONFIG_SPL_ATMEL_SIZE
#define CONFIG_SYS_MASTER_CLOCK		132096000
#define AT91_PLL_LOCK_TIMEOUT		1000000
#define CONFIG_SYS_AT91_PLLA		0x20c73f03
#define CONFIG_SYS_MCKR			0x1301
#define CONFIG_SYS_MCKR_CSS		0x1302

#define CONFIG_SPL_PAD_TO		CONFIG_SYS_NAND_U_BOOT_OFFS
#define CONFIG_SYS_SPL_LEN		CONFIG_SPL_PAD_TO

#endif
