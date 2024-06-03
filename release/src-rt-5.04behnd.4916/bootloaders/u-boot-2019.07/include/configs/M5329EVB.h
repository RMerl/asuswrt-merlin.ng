/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Freescale MCF5329 FireEngine board.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef _M5329EVB_H
#define _M5329EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MCFUART
#define CONFIG_SYS_UART_PORT		(0)

#undef CONFIG_WATCHDOG
#define CONFIG_WATCHDOG_TIMEOUT	5000	/* timeout in milliseconds, max timeout is 6.71sec */

#define CONFIG_SYS_UNIFY_CACHE

#define CONFIG_MCFFEC
#ifdef CONFIG_MCFFEC
#	define CONFIG_MII_INIT		1
#	define CONFIG_SYS_DISCOVER_PHY
#	define CONFIG_SYS_RX_ETH_BUFFER	8
#	define CONFIG_SYS_FAULT_ECHO_LINK_DOWN

#	define CONFIG_SYS_FEC0_PINMUX		0
#	define CONFIG_SYS_FEC0_MIIBASE		CONFIG_SYS_FEC0_IOBASE
#	define MCFFEC_TOUT_LOOP		50000
/* If CONFIG_SYS_DISCOVER_PHY is not defined - hardcoded */
#	ifndef CONFIG_SYS_DISCOVER_PHY
#		define FECDUPLEX	FULL
#		define FECSPEED		_100BASET
#	else
#		ifndef CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#			define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#		endif
#	endif			/* CONFIG_SYS_DISCOVER_PHY */
#endif

#define CONFIG_MCFRTC
#undef RTC_DEBUG

/* Timer */
#define CONFIG_MCFTMR
#undef CONFIG_MCFPIT

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	80000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x58000
#define CONFIG_SYS_IMMR		CONFIG_SYS_MBAR

#define CONFIG_UDP_CHECKSUM

#ifdef CONFIG_MCFFEC
#	define CONFIG_IPADDR	192.162.1.2
#	define CONFIG_NETMASK	255.255.255.0
#	define CONFIG_SERVERIP	192.162.1.1
#	define CONFIG_GATEWAYIP	192.162.1.1
#endif				/* FEC_ENET */

#define CONFIG_HOSTNAME		"M5329EVB"
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"			\
	"loadaddr=40010000\0"	\
	"u-boot=u-boot.bin\0"	\
	"load=tftp ${loadaddr) ${u-boot}\0"	\
	"upd=run load; run prog\0"	\
	"prog=prot off 0 3ffff;"	\
	"era 0 3ffff;"	\
	"cp.b ${loadaddr} 0 ${filesize};"	\
	"save\0"	\
	""

#define CONFIG_PRAM		512	/* 512 KB */

#define CONFIG_SYS_LOAD_ADDR		0x40010000

#define CONFIG_SYS_CLK			80000000
#define CONFIG_SYS_CPU_CLK		CONFIG_SYS_CLK * 3

#define CONFIG_SYS_MBAR		0xFC000000

#define CONFIG_SYS_LATCH_ADDR		(CONFIG_SYS_CS1_BASE + 0x80000)

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x80000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x8000	/* Size of used area in internal SRAM */
#define CONFIG_SYS_INIT_RAM_CTRL	0x221
#define CONFIG_SYS_GBL_DATA_OFFSET	((CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE) - 0x10)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_SDRAM_SIZE		32	/* SDRAM size in MB */
#define CONFIG_SYS_SDRAM_CFG1		0x53722730
#define CONFIG_SYS_SDRAM_CFG2		0x56670000
#define CONFIG_SYS_SDRAM_CTRL		0xE1092000
#define CONFIG_SYS_SDRAM_EMOD		0x40010000
#define CONFIG_SYS_SDRAM_MODE		0x018D0000

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE + 0x400
#define CONFIG_SYS_MEMTEST_END		((CONFIG_SYS_SDRAM_SIZE - 3) << 20)

#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE + 0x400)
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */

#define CONFIG_SYS_BOOTPARAMS_LEN	64*1024
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc() */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CONFIG_SYS_BOOTMAPSZ		(CONFIG_SYS_SDRAM_BASE + (CONFIG_SYS_SDRAM_SIZE << 20))
#define CONFIG_SYS_BOOTM_LEN		(CONFIG_SYS_SDRAM_SIZE << 20)

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#ifdef CONFIG_SYS_FLASH_CFI
#	define CONFIG_SYS_FLASH_SIZE		0x800000	/* Max size that the board might have */
#	define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#	define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#	define CONFIG_SYS_MAX_FLASH_SECT	137	/* max number of sectors on one chip */
#endif

#ifdef CONFIG_NANDFLASH_SIZE
#	define CONFIG_SYS_MAX_NAND_DEVICE	1
#	define CONFIG_SYS_NAND_BASE		CONFIG_SYS_CS2_BASE
#	define CONFIG_SYS_NAND_SIZE		1
#	define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }
#	define NAND_ALLOW_ERASE_ALL	1
#	define CONFIG_JFFS2_NAND	1
#	define CONFIG_JFFS2_DEV		"nand0"
#	define CONFIG_JFFS2_PART_SIZE	(CONFIG_SYS_CS2_MASK & ~1)
#	define CONFIG_JFFS2_PART_OFFSET	0x00000000
#endif

#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_CS0_BASE

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */
#define CONFIG_ENV_OFFSET		0x4000
#define CONFIG_ENV_SECT_SIZE	0x2000

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text*);

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16

#define ICACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 4)
#define CONFIG_SYS_ICACHE_INV		(CF_CACR_CINVA)
#define CONFIG_SYS_CACHE_ACR0		(CONFIG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CONFIG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CONFIG_SYS_CACHE_ICACR		(CF_CACR_EC | CF_CACR_CINVA | \
					 CF_CACR_DCM_P)

/*-----------------------------------------------------------------------
 * Chipselect bank definitions
 */
/*
 * CS0 - NOR Flash 1, 2, 4, or 8MB
 * CS1 - CompactFlash and registers
 * CS2 - NAND Flash 16, 32, or 64MB
 * CS3 - Available
 * CS4 - Available
 * CS5 - Available
 */
#define CONFIG_SYS_CS0_BASE		0
#define CONFIG_SYS_CS0_MASK		0x007f0001
#define CONFIG_SYS_CS0_CTRL		0x00001fa0

#define CONFIG_SYS_CS1_BASE		0x10000000
#define CONFIG_SYS_CS1_MASK		0x001f0001
#define CONFIG_SYS_CS1_CTRL		0x002A3780

#ifdef CONFIG_NANDFLASH_SIZE
#define CONFIG_SYS_CS2_BASE		0x20000000
#define CONFIG_SYS_CS2_MASK		((CONFIG_NANDFLASH_SIZE << 20) | 1)
#define CONFIG_SYS_CS2_CTRL		0x00001f60
#endif

#endif				/* _M5329EVB_H */
