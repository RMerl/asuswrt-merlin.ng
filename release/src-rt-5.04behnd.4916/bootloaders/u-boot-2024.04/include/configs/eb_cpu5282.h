/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the BuS EB+CPU5283 boards (aka EB+MCF-EV123)
 *
 * (C) Copyright 2005-2009 BuS Elektronik GmbH & Co.KG <esw@bus-elektonik.de>
 */

#ifndef _CONFIG_EB_CPU5282_H_
#define _CONFIG_EB_CPU5282_H_

/*----------------------------------------------------------------------*
 * High Level Configuration Options (easy to change)                    *
 *----------------------------------------------------------------------*/

#define CFG_SYS_UART_PORT		(0)

/*----------------------------------------------------------------------*
 * Options								*
 *----------------------------------------------------------------------*/

#define STATUS_LED_ACTIVE		0

/*----------------------------------------------------------------------*
 * Configuration for environment					*
 * Environment is in the second sector of the first 256k of flash	*
 *----------------------------------------------------------------------*/

/*#define CFG_SYS_DRAM_TEST		1 */
#undef CFG_SYS_DRAM_TEST

/*----------------------------------------------------------------------*
 * Clock and PLL Configuration						*
 *----------------------------------------------------------------------*/
#define	CFG_SYS_CLK			80000000      /* 8MHz * 8 */

/* PLL Configuration: Ext Clock * 8 (see table 9-4 of MCF user manual) */

#define CFG_SYS_MFD		0x02	/* PLL Multiplication Factor Devider */
#define CFG_SYS_RFD		0x00	/* PLL Reduce Frecuency Devider */

/*----------------------------------------------------------------------*
 * Network								*
 *----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 *-----------------------------------------------------------------------*/

#define	CFG_SYS_MBAR			0x40000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 *-----------------------------------------------------------------------*/

#define CFG_SYS_INIT_RAM_ADDR	0x20000000
#define CFG_SYS_INIT_RAM_SIZE	0x10000

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CFG_SYS_SDRAM_BASE0		0x00000000
#define	CFG_SYS_SDRAM_SIZE0		16	/* SDRAM size in MB */

#define CFG_SYS_SDRAM_BASE		CFG_SYS_SDRAM_BASE0
#define	CFG_SYS_SDRAM_SIZE		CFG_SYS_SDRAM_SIZE0

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define	CFG_SYS_BOOTMAPSZ	(8 << 20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */

#define CFG_SYS_FLASH_BASE		CFG_SYS_CS0_BASE
#define	CFG_SYS_INT_FLASH_BASE	0xF0000000
#define CFG_SYS_INT_FLASH_ENABLE	0x21

#define CFG_SYS_FLASH_SIZE		16*1024*1024

#define CFG_SYS_FLASH_BANKS_LIST	{ CFG_SYS_FLASH_BASE }

/*-----------------------------------------------------------------------
 * Cache Configuration
 */

#define ICACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 4)
#define CFG_SYS_ICACHE_INV		(CF_CACR_CINV + CF_CACR_DCM)
#define CFG_SYS_CACHE_ACR0		(CFG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CFG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CFG_SYS_CACHE_ICACR		(CF_CACR_CENB | CF_CACR_DISD | \
					 CF_CACR_CEIB | CF_CACR_DBWE | \
					 CF_CACR_EUSP)

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */

#define CFG_SYS_CS0_BASE		0xFF000000
#define CFG_SYS_CS0_CTRL		0x00001980
#define CFG_SYS_CS0_MASK		0x00FF0001

#define CFG_SYS_CS2_BASE		0xE0000000
#define CFG_SYS_CS2_CTRL		0x00001980
#define CFG_SYS_CS2_MASK		0x000F0001

#define CFG_SYS_CS3_BASE		0xE0100000
#define CFG_SYS_CS3_CTRL		0x00001980
#define CFG_SYS_CS3_MASK		0x000F0001

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define CFG_SYS_PACNT		0x0000000	/* Port A D[31:24] */
#define CFG_SYS_PADDR		0x0000000
#define CFG_SYS_PADAT		0x0000000

#define CFG_SYS_PBCNT		0x0000000	/* Port B D[23:16] */
#define CFG_SYS_PBDDR		0x0000000
#define CFG_SYS_PBDAT		0x0000000

#define CFG_SYS_PDCNT		0x0000000	/* Port D D[07:00] */

#define CFG_SYS_PASPAR		0x0F0F
#define CFG_SYS_PEHLPAR		0xC0
#define CFG_SYS_PUAPAR		0x0F
#define CFG_SYS_DDRUA		0x05
#define CFG_SYS_PJPAR		0xFF


#endif	/* _CONFIG_M5282EVB_H */
/*---------------------------------------------------------------------*/
