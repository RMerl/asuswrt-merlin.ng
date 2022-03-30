/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch-ae3xx/ae3xx.h>

/*
 * CPU and Board Configuration Options
 */
#define CONFIG_USE_INTERRUPT

#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_SKIP_TRUNOFF_WATCHDOG

#define CONFIG_ARCH_MAP_SYSMEM

#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_SERVERIP

#ifdef CONFIG_SKIP_LOWLEVEL_INIT
#ifdef CONFIG_OF_CONTROL
#undef CONFIG_OF_SEPARATE
#define CONFIG_OF_EMBED
#endif
#endif

/*
 * Timer
 */
#define CONFIG_SYS_CLK_FREQ	39062500
#define VERSION_CLOCK		CONFIG_SYS_CLK_FREQ

/*
 * Use Externel CLOCK or PCLK
 */
#undef CONFIG_FTRTC010_EXTCLK

#ifndef CONFIG_FTRTC010_EXTCLK
#define CONFIG_FTRTC010_PCLK
#endif

#ifdef CONFIG_FTRTC010_EXTCLK
#define TIMER_CLOCK	32768			/* CONFIG_FTRTC010_EXTCLK */
#else
#define TIMER_CLOCK	CONFIG_SYS_HZ		/* CONFIG_FTRTC010_PCLK */
#endif

#define TIMER_LOAD_VAL	0xffffffff

/*
 * Real Time Clock
 */
#define CONFIG_RTC_FTRTC010

/*
 * Real Time Clock Divider
 * RTC_DIV_COUNT			(OSC_CLK/OSC_5MHZ)
 */
#define OSC_5MHZ			(5*1000000)
#define OSC_CLK				(4*OSC_5MHZ)
#define RTC_DIV_COUNT			(0.5)	/* Why?? */

/*
 * Serial console configuration
 */

/* FTUART is a high speed NS 16C550A compatible UART, addr: 0x99600000 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_COM1		CONFIG_FTUART010_02_BASE
#ifndef CONFIG_DM_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#endif
#define CONFIG_SYS_NS16550_CLK		((18432000 * 20) / 25)	/* AG101P */

/*
 * Miscellaneous configurable options
 */

/*
 * Size of malloc() pool
 */
/* 512kB is suggested, (CONFIG_ENV_SIZE + 128 * 1024) was not enough */
#define CONFIG_SYS_MALLOC_LEN		(512 << 10)

/*
 * Physical Memory Map
 */
#define PHYS_SDRAM_0	0x00000000  /* SDRAM Bank #1 */

#define PHYS_SDRAM_1 \
	(PHYS_SDRAM_0 + PHYS_SDRAM_0_SIZE)	/* SDRAM Bank #2 */

#define PHYS_SDRAM_0_SIZE	0x20000000	/* 512 MB */
#define PHYS_SDRAM_1_SIZE	0x20000000	/* 512 MB */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_0

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0xA0000 - \
					GENERATED_GBL_DATA_SIZE)

/*
 * Load address and memory test area should agree with
 * arch/nds32/config.mk. Be careful not to overwrite U-Boot itself.
 */
#define CONFIG_SYS_LOAD_ADDR		0x300000

/* memtest works on 63 MB in DRAM */
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_0
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_0 + 0x03F00000)

/*
 * Static memory controller configuration
 */
#define CONFIG_FTSMC020

#ifdef CONFIG_FTSMC020
#include <faraday/ftsmc020.h>

#define CONFIG_SYS_FTSMC020_CONFIGS	{			\
	{ FTSMC020_BANK0_CONFIG, FTSMC020_BANK0_TIMING, },	\
	{ FTSMC020_BANK1_CONFIG, FTSMC020_BANK1_TIMING, },	\
}

#ifndef CONFIG_SKIP_LOWLEVEL_INIT	/* FLASH is on BANK 0 */
#define FTSMC020_BANK0_LOWLV_CONFIG	(FTSMC020_BANK_ENABLE	|	\
					 FTSMC020_BANK_SIZE_32M	|	\
					 FTSMC020_BANK_MBW_32)

#define FTSMC020_BANK0_LOWLV_TIMING	(FTSMC020_TPR_RBE	|	\
					 FTSMC020_TPR_AST(1)	|	\
					 FTSMC020_TPR_CTW(1)	|	\
					 FTSMC020_TPR_ATI(1)	|	\
					 FTSMC020_TPR_AT2(1)	|	\
					 FTSMC020_TPR_WTC(1)	|	\
					 FTSMC020_TPR_AHT(1)	|	\
					 FTSMC020_TPR_TRNA(1))
#endif

/*
 * FLASH on ADP_AG101P is connected to BANK0
 * Just disalbe the other BANK to avoid detection error.
 */
#define FTSMC020_BANK0_CONFIG	(FTSMC020_BANK_ENABLE             |	\
				 FTSMC020_BANK_BASE(PHYS_FLASH_1) |	\
				 FTSMC020_BANK_SIZE_32M           |	\
				 FTSMC020_BANK_MBW_32)

#define FTSMC020_BANK0_TIMING	(FTSMC020_TPR_AST(3)   |	\
				 FTSMC020_TPR_CTW(3)   |	\
				 FTSMC020_TPR_ATI(0xf) |	\
				 FTSMC020_TPR_AT2(3)   |	\
				 FTSMC020_TPR_WTC(3)   |	\
				 FTSMC020_TPR_AHT(3)   |	\
				 FTSMC020_TPR_TRNA(0xf))

#define FTSMC020_BANK1_CONFIG	(0x00)
#define FTSMC020_BANK1_TIMING	(0x00)
#endif /* CONFIG_FTSMC020 */

/*
 * FLASH and environment organization
 */
/* use CFI framework */

#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_CFI_FLASH_STATUS_POLL

/* support JEDEC */
#ifdef CONFIG_CFI_FLASH
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT	1
#endif

/* Do not use CONFIG_FLASH_CFI_LEGACY to detect on board flash */
#define PHYS_FLASH_1			0x88000000	/* BANK 0 */
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_SYS_FLASH_BANKS_LIST	{ PHYS_FLASH_1, }
#define CONFIG_SYS_MONITOR_BASE		PHYS_FLASH_1

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* TO for Flash Erase (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* TO for Flash Write (ms) */

/* max number of memory banks */
/*
 * There are 4 banks supported for this Controller,
 * but we have only 1 bank connected to flash on board
 */
#ifndef CONFIG_SYS_MAX_FLASH_BANKS_DETECT
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#endif
#define CONFIG_SYS_FLASH_BANKS_SIZES {0x4000000}

/* max number of sectors on one chip */
#define CONFIG_FLASH_SECTOR_SIZE	(0x10000*2)
#define CONFIG_SYS_MAX_FLASH_SECT	512

/* environments */
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x140000
#define CONFIG_ENV_SIZE			8192
#define CONFIG_ENV_OVERWRITE


/* SPI FLASH */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 16 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */

/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)
/* Increase max gunzip size */
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)

#endif	/* __CONFIG_H */
