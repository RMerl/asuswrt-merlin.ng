/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the esd TASREG board.
 *
 * (C) Copyright 2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef _M5249EVB_H
#define _M5249EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CFG_SYS_UART_PORT		(0)

/*
 * Clock configuration: enable only one of the following options
 */

#undef  CFG_SYS_PLL_BYPASS				/* bypass PLL for test purpose */
#define CFG_SYS_FAST_CLK		1		/* MCF5249 can run at 140MHz   */
#define	CFG_SYS_CLK			132025600	/* MCF5249 can run at 140MHz   */

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CFG_SYS_MBAR		0x10000000	/* Register Base Addrs */
#define	CFG_SYS_MBAR2		0x80000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_SYS_INIT_RAM_ADDR	0x20000000
#define CFG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in internal SRAM	*/

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text);

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CFG_SYS_SDRAM_BASE		0x00000000
#define CFG_SYS_SDRAM_SIZE		16		/* SDRAM size in MB */
#define CFG_SYS_FLASH_BASE		(CFG_SYS_CS0_BASE)

#if 0 /* test-only */
#define CFG_PRAM		512 /* test-only for SDRAM problem!!!!!!!!!!!!!!!!!!!! */
#endif

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CFG_SYS_BOOTMAPSZ		(CFG_SYS_SDRAM_BASE + (CFG_SYS_SDRAM_SIZE << 20))

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#ifdef CONFIG_SYS_FLASH_CFI

#	define CFG_SYS_FLASH_SIZE		0x1000000	/* Max size that the board might have */
#	define CFG_SYS_FLASH_BANKS_LIST	{ CFG_SYS_FLASH_BASE }
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */

#define ICACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 4)
#define CFG_SYS_ICACHE_INV		(CF_CACR_DCM)
#define CFG_SYS_CACHE_ACR0		(CFG_SYS_FLASH_BASE | \
					 CF_ADDRMASK(2) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CFG_SYS_CACHE_ACR1		(CFG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CFG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CFG_SYS_CACHE_ICACR		(CF_CACR_CENB | CF_CACR_CEIB | \
					 CF_CACR_DBWE)

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */

/* CS0 - AMD Flash, address 0xffc00000 */
#define	CFG_SYS_CS0_BASE		0xffe00000
#define	CFG_SYS_CS0_CTRL		0x00001980	/* WS=0110, AA=1, PS=10         */
/** Note: There is a CSMR0/DRAM vector problem, need to disable C/I ***/
#define	CFG_SYS_CS0_MASK		0x003f0021	/* 4MB, AA=0, WP=0, C/I=1, V=1  */

/* CS1 - FPGA, address 0xe0000000 */
#define	CFG_SYS_CS1_BASE		0xe0000000
#define	CFG_SYS_CS1_CTRL		0x00000d80	/* WS=0011, AA=1, PS=10         */
#define	CFG_SYS_CS1_MASK		0x00010001	/* 128kB, AA=0, WP=0, C/I=0, V=1*/

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define	CFG_SYS_GPIO_FUNC		0x00000008	/* Set gpio pins: none          */
#define	CFG_SYS_GPIO1_FUNC		0x00df00f0	/* 36-39(SWITCH),48-52(FPGAs),54*/
#define	CFG_SYS_GPIO_EN		0x00000008	/* Set gpio output enable       */
#define	CFG_SYS_GPIO1_EN		0x00c70000	/* Set gpio output enable       */
#define	CFG_SYS_GPIO_OUT		0x00000008	/* Set outputs to default state */
#define	CFG_SYS_GPIO1_OUT		0x00c70000	/* Set outputs to default state */
#define CFG_SYS_GPIO1_LED		0x00400000	/* user led                     */


#endif	/* M5249 */
