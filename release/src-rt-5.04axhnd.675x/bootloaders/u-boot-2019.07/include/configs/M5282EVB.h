/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Motorola MC5282EVB board.
 *
 * (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef _CONFIG_M5282EVB_H
#define _CONFIG_M5282EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MCFTMR

#define CONFIG_MCFUART
#define CONFIG_SYS_UART_PORT		(0)

#undef	CONFIG_MONITOR_IS_IN_RAM	/* define if monitor is started from a pre-loader */

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */
#define CONFIG_ENV_ADDR		0xffe04000
#define CONFIG_ENV_SIZE		0x2000

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text*);

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Command line configuration.
 */

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

#ifdef CONFIG_MCFFEC
#	define CONFIG_IPADDR	192.162.1.2
#	define CONFIG_NETMASK	255.255.255.0
#	define CONFIG_SERVERIP	192.162.1.1
#	define CONFIG_GATEWAYIP	192.162.1.1
#endif				/* CONFIG_MCFFEC */

#define CONFIG_HOSTNAME		"M5282EVB"
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"loadaddr=10000\0"			\
	"u-boot=u-boot.bin\0"			\
	"load=tftp ${loadaddr) ${u-boot}\0"	\
	"upd=run load; run prog\0"		\
	"prog=prot off ffe00000 ffe3ffff;"	\
	"era ffe00000 ffe3ffff;"		\
	"cp.b ${loadaddr} ffe00000 ${filesize};"\
	"save\0"				\
	""

#define CONFIG_SYS_LOAD_ADDR		0x20000

#define CONFIG_SYS_MEMTEST_START	0x400
#define CONFIG_SYS_MEMTEST_END		0x380000

#define	CONFIG_SYS_CLK			64000000

/* PLL Configuration: Ext Clock * 6 (see table 9-4 of MCF user manual) */

#define CONFIG_SYS_MFD			0x02	/* PLL Multiplication Factor Devider */
#define CONFIG_SYS_RFD			0x00	/* PLL Reduce Frecuency Devider */

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
#define	CONFIG_SYS_MBAR		0x40000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x20000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x10000	/* Size of used area in internal SRAM    */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define	CONFIG_SYS_SDRAM_SIZE		16	/* SDRAM size in MB */
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_CS0_BASE
#define	CONFIG_SYS_INT_FLASH_BASE	0xf0000000
#define CONFIG_SYS_INT_FLASH_ENABLE	0x21

/* If M5282 port is fully implemented the monitor base will be behind
 * the vector table. */
#if (CONFIG_SYS_TEXT_BASE != CONFIG_SYS_INT_FLASH_BASE)
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE + 0x400)
#else
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_TEXT_BASE + 0x418)	/* 24 Byte for CFM-Config */
#endif

#define CONFIG_SYS_MONITOR_LEN		0x20000
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)
#define CONFIG_SYS_BOOTPARAMS_LEN	64*1024

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CONFIG_SYS_BOOTMAPSZ		(CONFIG_SYS_SDRAM_BASE + (CONFIG_SYS_SDRAM_SIZE << 20))

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#ifdef CONFIG_SYS_FLASH_CFI

#	define CONFIG_SYS_FLASH_SIZE		0x1000000	/* Max size that the board might have */
#	define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#	define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#	define CONFIG_SYS_MAX_FLASH_SECT	137	/* max number of sectors on one chip */
#	define CONFIG_SYS_FLASH_CHECKSUM
#	define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16

#define ICACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 4)
#define CONFIG_SYS_ICACHE_INV		(CF_CACR_CINV + CF_CACR_DCM)
#define CONFIG_SYS_CACHE_ACR0		(CONFIG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CONFIG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CONFIG_SYS_CACHE_ICACR		(CF_CACR_CENB | CF_CACR_DISD | \
					 CF_CACR_CEIB | CF_CACR_DBWE | \
					 CF_CACR_EUSP)

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */
#define CONFIG_SYS_CS0_BASE		0xFFE00000
#define CONFIG_SYS_CS0_CTRL		0x00001980
#define CONFIG_SYS_CS0_MASK		0x001F0001

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define CONFIG_SYS_PACNT		0x0000000	/* Port A D[31:24] */
#define CONFIG_SYS_PADDR		0x0000000
#define CONFIG_SYS_PADAT		0x0000000

#define CONFIG_SYS_PBCNT		0x0000000	/* Port B D[23:16] */
#define CONFIG_SYS_PBDDR		0x0000000
#define CONFIG_SYS_PBDAT		0x0000000

#define CONFIG_SYS_PCCNT		0x0000000	/* Port C D[15:08] */
#define CONFIG_SYS_PCDDR		0x0000000
#define CONFIG_SYS_PCDAT		0x0000000

#define CONFIG_SYS_PDCNT		0x0000000	/* Port D D[07:00] */
#define CONFIG_SYS_PCDDR		0x0000000
#define CONFIG_SYS_PCDAT		0x0000000

#define CONFIG_SYS_PEHLPAR		0xC0
#define CONFIG_SYS_PUAPAR		0x0F	/* UA0..UA3 = Uart 0 +1 */
#define CONFIG_SYS_DDRUA		0x05
#define CONFIG_SYS_PJPAR		0xFF

#endif				/* _CONFIG_M5282EVB_H */
