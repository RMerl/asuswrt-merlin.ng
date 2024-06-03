/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the mini-box PICOSAM9G45 board.
 * (C) Copyright 2015 Inter Act B.V.
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

#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK      32768
#define CONFIG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT

/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */
#define CONFIG_AT91_GPIO
#define CONFIG_AT91_GPIO_PULLUP	1	/* keep pullups on peripheral pins */

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define	CONFIG_USART_ID			ATMEL_ID_SYS

/* LCD */
#define LCD_BPP				LCD_COLOR8
#define CONFIG_LCD_LOGO
#undef LCD_TEST_PATTERN
#define CONFIG_LCD_INFO
#define CONFIG_LCD_INFO_BELOW_LOGO
#define CONFIG_ATMEL_LCD
#define CONFIG_ATMEL_LCD_RGB565
/* board specific(not enough SRAM) */
#define CONFIG_AT91SAM9G45_LCD_BASE		0x23E00000

/* LED */
#define CONFIG_AT91_LED
#define CONFIG_GREEN_LED	AT91_PIN_PD31	/* this is the user1 led */


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Command line configuration.
 */

/* SDRAM */
#define PHYS_SDRAM_1		ATMEL_BASE_CS1	/* on DDRSDRC1 */
#define PHYS_SDRAM_1_SIZE	0x08000000	/* 128 MB */
#define PHYS_SDRAM_2		ATMEL_BASE_CS6	/* on DDRSDRC0 */
#define PHYS_SDRAM_2_SIZE       0x08000000	/* 128 MB */
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 4 * 1024 - GENERATED_GBL_DATA_SIZE)

/* MMC */

#ifdef CONFIG_CMD_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#endif

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R
#define CONFIG_AT91_WANTS_COMMON_PHY

#define CONFIG_SYS_LOAD_ADDR		0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		0x23e00000

#ifdef CONFIG_SYS_USE_MMC
/* bootstrap + u-boot + env + linux in mmc */
#define CONFIG_ENV_SIZE		0x4000

#define CONFIG_BOOTCOMMAND	"fatload mmc 0:1 0x21000000 dtb; " \
				"fatload mmc 0:1 0x22000000 zImage; " \
				"bootz 0x22000000 - 0x21000000"
#endif

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	ROUND(3 * CONFIG_ENV_SIZE + 128*1024, 0x1000)

/* Defines for SPL */
#define CONFIG_SPL_MAX_SIZE		0x010000
#define CONFIG_SPL_STACK		0x310000

#define CONFIG_SYS_MONITOR_LEN		0x80000

#ifdef CONFIG_SYS_USE_MMC

#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00080000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x00080000

#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"

#define CONFIG_SPL_ATMEL_SIZE
#define CONFIG_SYS_MASTER_CLOCK		132096000
#define CONFIG_SYS_AT91_PLLA		0x20c73f03
#define CONFIG_SYS_MCKR			0x1301
#define CONFIG_SYS_MCKR_CSS		0x1302

#endif
#endif
