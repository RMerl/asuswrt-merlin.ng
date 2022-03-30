/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the BuS EB+CPU5283 boards (aka EB+MCF-EV123)
 *
 * (C) Copyright 2005-2009 BuS Elektronik GmbH & Co.KG <esw@bus-elektonik.de>
 */

#ifndef _CONFIG_EB_CPU5282_H_
#define _CONFIG_EB_CPU5282_H_

#undef CONFIG_SYS_HALT_BEFOR_RAM_JUMP

/*----------------------------------------------------------------------*
 * High Level Configuration Options (easy to change)                    *
 *----------------------------------------------------------------------*/

#define CONFIG_MCFUART
#define CONFIG_SYS_UART_PORT		(0)

#undef	CONFIG_MONITOR_IS_IN_RAM		/* starts uboot direct */

#define CONFIG_BOOTCOMMAND "printenv"

/*----------------------------------------------------------------------*
 * Options								*
 *----------------------------------------------------------------------*/

#define CONFIG_BOOT_RETRY_TIME	-1
#define CONFIG_RESET_TO_RETRY
#define CONFIG_SPLASH_SCREEN

#define CONFIG_HW_WATCHDOG

#define STATUS_LED_ACTIVE		0

/*----------------------------------------------------------------------*
 * Configuration for environment					*
 * Environment is in the second sector of the first 256k of flash	*
 *----------------------------------------------------------------------*/

#define CONFIG_ENV_ADDR		0xFF040000
#define CONFIG_ENV_SECT_SIZE	0x00020000

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Command line configuration.
 */

#define CONFIG_MCFTMR

#define	CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

#define CONFIG_SYS_LOAD_ADDR		0x20000

#define CONFIG_SYS_MEMTEST_START	0x100000
#define CONFIG_SYS_MEMTEST_END		0x400000
/*#define CONFIG_SYS_DRAM_TEST		1 */
#undef CONFIG_SYS_DRAM_TEST

/*----------------------------------------------------------------------*
 * Clock and PLL Configuration						*
 *----------------------------------------------------------------------*/
#define	CONFIG_SYS_CLK			80000000      /* 8MHz * 8 */

/* PLL Configuration: Ext Clock * 8 (see table 9-4 of MCF user manual) */

#define CONFIG_SYS_MFD		0x02	/* PLL Multiplication Factor Devider */
#define CONFIG_SYS_RFD		0x00	/* PLL Reduce Frecuency Devider */

/*----------------------------------------------------------------------*
 * Network								*
 *----------------------------------------------------------------------*/

#define CONFIG_MCFFEC
#define CONFIG_MII_INIT			1
#define CONFIG_SYS_DISCOVER_PHY
#define CONFIG_SYS_RX_ETH_BUFFER	8
#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN

#define CONFIG_SYS_FEC0_PINMUX		0
#define CONFIG_SYS_FEC0_MIIBASE		CONFIG_SYS_FEC0_IOBASE
#define MCFFEC_TOUT_LOOP		50000

#define CONFIG_OVERWRITE_ETHADDR_ONCE

/*-------------------------------------------------------------------------
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 *-----------------------------------------------------------------------*/

#define	CONFIG_SYS_MBAR			0x40000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 *-----------------------------------------------------------------------*/

#define CONFIG_SYS_INIT_RAM_ADDR	0x20000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x10000
#define CONFIG_SYS_GBL_DATA_OFFSET	\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE0		0x00000000
#define	CONFIG_SYS_SDRAM_SIZE0		16	/* SDRAM size in MB */

#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_SDRAM_BASE0
#define	CONFIG_SYS_SDRAM_SIZE		CONFIG_SYS_SDRAM_SIZE0

#define CONFIG_SYS_MONITOR_LEN		0x20000
#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	64*1024

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define	CONFIG_SYS_BOOTMAPSZ	(8 << 20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_FLASH_SHOW_PROGRESS	45

#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_CS0_BASE
#define	CONFIG_SYS_INT_FLASH_BASE	0xF0000000
#define CONFIG_SYS_INT_FLASH_ENABLE	0x21

#define	CONFIG_SYS_MAX_FLASH_SECT	128
#define	CONFIG_SYS_MAX_FLASH_BANKS	1
#define	CONFIG_SYS_FLASH_ERASE_TOUT	10000000

#define CONFIG_SYS_FLASH_SIZE		16*1024*1024
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

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

#define CONFIG_SYS_CS0_BASE		0xFF000000
#define CONFIG_SYS_CS0_CTRL		0x00001980
#define CONFIG_SYS_CS0_MASK		0x00FF0001

#define CONFIG_SYS_CS2_BASE		0xE0000000
#define CONFIG_SYS_CS2_CTRL		0x00001980
#define CONFIG_SYS_CS2_MASK		0x000F0001

#define CONFIG_SYS_CS3_BASE		0xE0100000
#define CONFIG_SYS_CS3_CTRL		0x00001980
#define CONFIG_SYS_CS3_MASK		0x000F0001

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

#define CONFIG_SYS_PASPAR		0x0F0F
#define CONFIG_SYS_PEHLPAR		0xC0
#define CONFIG_SYS_PUAPAR		0x0F
#define CONFIG_SYS_DDRUA		0x05
#define CONFIG_SYS_PJPAR		0xFF

/*-----------------------------------------------------------------------
 * I2C
 */

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL

#define CONFIG_SYS_FSL_I2C_OFFSET	0x00000300
#define CONFIG_SYS_IMMR			CONFIG_SYS_MBAR

#define CONFIG_SYS_FSL_I2C_SPEED	100000
#define CONFIG_SYS_FSL_I2C_SLAVE	0

#ifdef CONFIG_CMD_DATE
#define CONFIG_RTC_DS1338
#define CONFIG_I2C_RTC_ADDR		0x68
#endif

/*-----------------------------------------------------------------------
 * VIDEO configuration
 */

#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_VCXK			1

#define CONFIG_SYS_VCXK_DEFAULT_LINEALIGN	2
#define	CONFIG_SYS_VCXK_DOUBLEBUFFERED		1
#define CONFIG_SYS_VCXK_BASE			CONFIG_SYS_CS2_BASE

#define CONFIG_SYS_VCXK_ACKNOWLEDGE_PORT	MCFGPTB_GPTPORT
#define CONFIG_SYS_VCXK_ACKNOWLEDGE_DDR		MCFGPTB_GPTDDR
#define CONFIG_SYS_VCXK_ACKNOWLEDGE_PIN		0x0001

#define CONFIG_SYS_VCXK_ENABLE_PORT		MCFGPTB_GPTPORT
#define CONFIG_SYS_VCXK_ENABLE_DDR		MCFGPTB_GPTDDR
#define CONFIG_SYS_VCXK_ENABLE_PIN		0x0002

#define CONFIG_SYS_VCXK_REQUEST_PORT		MCFGPTB_GPTPORT
#define CONFIG_SYS_VCXK_REQUEST_DDR		MCFGPTB_GPTDDR
#define CONFIG_SYS_VCXK_REQUEST_PIN		0x0004

#define CONFIG_SYS_VCXK_INVERT_PORT		MCFGPIO_PORTE
#define CONFIG_SYS_VCXK_INVERT_DDR		MCFGPIO_DDRE
#define CONFIG_SYS_VCXK_INVERT_PIN		MCFGPIO_PORT2

#endif /* CONFIG_VIDEO */
#endif	/* _CONFIG_M5282EVB_H */
/*---------------------------------------------------------------------*/
