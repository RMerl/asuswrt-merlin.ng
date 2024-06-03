/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Freescale MCF54418 TWR board.
 *
 * Copyright 2010-2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef _M54418TWR_H
#define _M54418TWR_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MCFUART
#define CONFIG_SYS_UART_PORT		(0)
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600 , 19200 , 38400 , 57600, 115200 }

#define LDS_BOARD_TEXT			board/freescale/m54418twr/sbf_dram_init.o (.text*)

#undef CONFIG_WATCHDOG

#define CONFIG_TIMESTAMP	/* Print image info with timestamp */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * NAND FLASH
 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_JFFS2_NAND
#define CONFIG_NAND_FSL_NFC
#define CONFIG_SYS_NAND_BASE		0xFC0FC000
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define NAND_MAX_CHIPS			CONFIG_SYS_MAX_NAND_DEVICE
#define CONFIG_SYS_NAND_SELECT_DEVICE
#endif

/* Network configuration */
#define CONFIG_MCFFEC
#ifdef CONFIG_MCFFEC
#define CONFIG_MII_INIT		1
#define CONFIG_SYS_DISCOVER_PHY
#define CONFIG_SYS_RX_ETH_BUFFER	2
#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#define CONFIG_SYS_TX_ETH_BUFFER	2
#define CONFIG_HAS_ETH1

#define CONFIG_SYS_FEC0_PINMUX		0
#define CONFIG_SYS_FEC0_MIIBASE	CONFIG_SYS_FEC0_IOBASE
#define CONFIG_SYS_FEC1_PINMUX		0
#define CONFIG_SYS_FEC1_MIIBASE	CONFIG_SYS_FEC0_MIIBASE
#define MCFFEC_TOUT_LOOP		50000
#define CONFIG_SYS_FEC0_PHYADDR	0
#define CONFIG_SYS_FEC1_PHYADDR	1

#define CONFIG_ETHPRIME	"FEC0"
#define CONFIG_IPADDR		192.168.1.2
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_SERVERIP	192.168.1.1
#define CONFIG_GATEWAYIP	192.168.1.1

#define CONFIG_SYS_FEC_BUF_USE_SRAM
/* If CONFIG_SYS_DISCOVER_PHY is not defined - hardcoded */
#ifndef CONFIG_SYS_DISCOVER_PHY
#define FECDUPLEX	FULL
#define FECSPEED	_100BASET
#define LINKSTATUS	1
#else
#define LINKSTATUS	0
#ifndef CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#endif
#endif			/* CONFIG_SYS_DISCOVER_PHY */
#endif

#define CONFIG_HOSTNAME		"M54418TWR"

#if defined(CONFIG_CF_SBF)
/* ST Micro serial flash */
#define	CONFIG_SYS_LOAD_ADDR2		0x40010007
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"inpclk=" __stringify(CONFIG_SYS_INPUT_CLKSRC) "\0"	\
	"loadaddr=0x40010000\0"			\
	"sbfhdr=sbfhdr.bin\0"			\
	"uboot=u-boot.bin\0"			\
	"load=tftp ${loadaddr} ${sbfhdr};"	\
	"tftp " __stringify(CONFIG_SYS_LOAD_ADDR2) " ${uboot} \0"	\
	"upd=run load; run prog\0"		\
	"prog=sf probe 0:1 1000000 3;"		\
	"sf erase 0 40000;"			\
	"sf write ${loadaddr} 0 40000;"		\
	"save\0"				\
	""
#elif defined(CONFIG_SYS_NAND_BOOT)
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"inpclk=" __stringify(CONFIG_SYS_INPUT_CLKSRC) "\0"	\
	"loadaddr=0x40010000\0"			\
	"u-boot=u-boot.bin\0"			\
	"load=tftp ${loadaddr} ${u-boot};\0"	\
	"upd=run load; run prog\0"		\
	"prog=nand device 0;"			\
	"nand erase 0 40000;"			\
	"nb_update ${loadaddr} ${filesize};"	\
	"save\0"				\
	""
#else
#define CONFIG_SYS_UBOOT_END	0x3FFFF
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"inpclk=" __stringify(CONFIG_SYS_INPUT_CLKSRC) "\0"	\
	"loadaddr=40010000\0"			\
	"u-boot=u-boot.bin\0"			\
	"load=tftp ${loadaddr) ${u-boot}\0"	\
	"upd=run load; run prog\0"		\
	"prog=prot off mram" " ;"	\
	"cp.b ${loadaddr} 0 ${filesize};"	\
	"save\0"				\
	""
#endif

/* Realtime clock */
#undef CONFIG_MCFRTC
#define CONFIG_RTC_MCFRRTC
#define CONFIG_SYS_MCFRRTC_BASE		0xFC0A8000

/* Timer */
#define CONFIG_MCFTMR
#undef CONFIG_MCFPIT

/* I2c */
#undef CONFIG_SYS_FSL_I2C
#undef	CONFIG_SYS_I2C_SOFT	/* I2C bit-banged */
/* I2C speed and slave address  */
#define CONFIG_SYS_I2C_SPEED		80000
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_OFFSET		0x58000
#define CONFIG_SYS_IMMR		CONFIG_SYS_MBAR

/* DSPI and Serial Flash */
#define CONFIG_CF_DSPI
#define CONFIG_SERIAL_FLASH
#define CONFIG_SYS_SBFHDR_SIZE		0x7

/* Input, PCI, Flexbus, and VCO */
#define CONFIG_EXTRA_CLOCK

#define CONFIG_PRAM			2048	/* 2048 KB */

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x10000)

#define CONFIG_SYS_MBAR		0xFC000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x80000000
/* End of used area in internal SRAM */
#define CONFIG_SYS_INIT_RAM_SIZE	0x10000
#define CONFIG_SYS_INIT_RAM_CTRL	0x221
#define CONFIG_SYS_GBL_DATA_OFFSET	((CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE) - 32)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_SBFHDR_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - 32)

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_SDRAM_SIZE		128	/* SDRAM size in MB */

#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + 0x400)
#define CONFIG_SYS_MEMTEST_END		((CONFIG_SYS_SDRAM_SIZE - 3) << 20)
#define CONFIG_SYS_DRAM_TEST

#if defined(CONFIG_CF_SBF) || defined(CONFIG_SYS_NAND_BOOT)
#define CONFIG_SERIAL_BOOT
#endif

#if defined(CONFIG_SERIAL_BOOT)
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_TEXT_BASE + 0x400)
#else
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE + 0x400)
#endif

#define CONFIG_SYS_BOOTPARAMS_LEN	(64 * 1024)
/* Reserve 256 kB for Monitor */
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)
/* Reserve 256 kB for malloc() */
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
/* Initial Memory map for Linux */
#define CONFIG_SYS_BOOTMAPSZ (CONFIG_SYS_SDRAM_BASE + \
				(CONFIG_SYS_SDRAM_SIZE << 20))

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */
#if !defined(CONFIG_SERIAL_BOOT)  /*MRAM boot*/
#define CONFIG_ENV_ADDR		(0x40000 - 0x1000) /*MRAM size 40000*/
#define CONFIG_ENV_SIZE		0x1000
#endif

#if defined(CONFIG_CF_SBF)
#define CONFIG_ENV_OFFSET		0x40000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE		0x10000
#endif
#if defined(CONFIG_SYS_NAND_BOOT)
#define CONFIG_ENV_OFFSET	0x80000
#define CONFIG_ENV_SIZE	0x20000
#define CONFIG_ENV_SECT_SIZE	0x20000
#endif
#undef CONFIG_ENV_OVERWRITE

/* FLASH organization */
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_CS0_BASE

#ifdef CONFIG_SYS_FLASH_CFI

/* Max size that the board might have */
#define CONFIG_SYS_FLASH_SIZE		0x1000000
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_BANKS	1
/* max number of sectors on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT	270
/* "Real" (hardware) sectors protection */
#define CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_CS0_BASE }
#else
/* max number of sectors on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT	270
/* max number of sectors on one chip */
#define CONFIG_SYS_MAX_FLASH_BANKS	0
#endif

/*
 * This is setting for JFFS2 support in u-boot.
 * NOTE: Enable CONFIG_CMD_JFFS2 for JFFS2 support.
 */
#ifdef CONFIG_CMD_JFFS2
#define CONFIG_JFFS2_DEV		"nand0"
#define CONFIG_JFFS2_PART_OFFSET	(0x800000)

#endif

/* Cache Configuration */
#define CONFIG_SYS_CACHELINE_SIZE	16
#define ICACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 4)
#define CONFIG_SYS_ICACHE_INV		(CF_CACR_BCINVA + CF_CACR_ICINVA)
#define CONFIG_SYS_DCACHE_INV		(CF_CACR_DCINVA)
#define CONFIG_SYS_CACHE_ACR2		(CONFIG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CONFIG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CONFIG_SYS_CACHE_ICACR		(CF_CACR_BEC | CF_CACR_IEC | \
					 CF_CACR_ICINVA | CF_CACR_EUSP)
#define CONFIG_SYS_CACHE_DCACR		((CONFIG_SYS_CACHE_ICACR | \
					 CF_CACR_DEC | CF_CACR_DDCM_P | \
					 CF_CACR_DCINVA) & ~CF_CACR_ICINVA)

#define CACR_STATUS	(CONFIG_SYS_INIT_RAM_ADDR + \
			CONFIG_SYS_INIT_RAM_SIZE - 12)

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */
/*
 * CS0 - NOR Flash 16MB
 * CS1 - Available
 * CS2 - Available
 * CS3 - Available
 * CS4 - Available
 * CS5 - Available
 */

 /* Flash */
#define CONFIG_SYS_CS0_BASE		0x00000000
#define CONFIG_SYS_CS0_MASK		0x000F0101
#define CONFIG_SYS_CS0_CTRL		0x00001D60

#endif				/* _M54418TWR_H */
