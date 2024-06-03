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
#define CONFIG_MCFTMR

#define CONFIG_MCFUART
#define CONFIG_SYS_UART_PORT		(0)

#undef  CONFIG_WATCHDOG

#undef CONFIG_MONITOR_IS_IN_RAM		/* no pre-loader required!!! ;-) */

/*
 * BOOTP options
 */
#undef CONFIG_BOOTP_BOOTFILESIZE

/*
 * Command line configuration.
 */

#define CONFIG_SYS_DEVICE_NULLDEV	1	/* include nulldev device	*/
#define CONFIG_MX_CYCLIC	1	/* enable mdc/mwc commands	*/

#define CONFIG_SYS_LOAD_ADDR		0x200000	/* default load address */

#define CONFIG_SYS_MEMTEST_START	0x400
#define CONFIG_SYS_MEMTEST_END		0x380000

/*
 * Clock configuration: enable only one of the following options
 */

#undef  CONFIG_SYS_PLL_BYPASS				/* bypass PLL for test purpose */
#define CONFIG_SYS_FAST_CLK		1		/* MCF5249 can run at 140MHz   */
#define	CONFIG_SYS_CLK			132025600	/* MCF5249 can run at 140MHz   */

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CONFIG_SYS_MBAR		0x10000000	/* Register Base Addrs */
#define	CONFIG_SYS_MBAR2		0x80000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x20000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in internal SRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text);

#define CONFIG_ENV_OFFSET		0x4000	/* Address of Environment Sector*/
#define CONFIG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/
#define CONFIG_ENV_SECT_SIZE	0x2000 /* see README - env sector total size	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		16		/* SDRAM size in MB */
#define CONFIG_SYS_FLASH_BASE		(CONFIG_SYS_CS0_BASE)

#if 0 /* test-only */
#define CONFIG_PRAM		512 /* test-only for SDRAM problem!!!!!!!!!!!!!!!!!!!! */
#endif

#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE + 0x400)

#define CONFIG_SYS_MONITOR_LEN		0x20000
#define CONFIG_SYS_MALLOC_LEN		(1 * 1024*1024)	/* Reserve 1 MB for malloc()	*/
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
#define CONFIG_SYS_ICACHE_INV		(CF_CACR_DCM)
#define CONFIG_SYS_CACHE_ACR0		(CONFIG_SYS_FLASH_BASE | \
					 CF_ADDRMASK(2) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CONFIG_SYS_CACHE_ACR1		(CONFIG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CONFIG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CONFIG_SYS_CACHE_ICACR		(CF_CACR_CENB | CF_CACR_CEIB | \
					 CF_CACR_DBWE)

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */

/* CS0 - AMD Flash, address 0xffc00000 */
#define	CONFIG_SYS_CS0_BASE		0xffe00000
#define	CONFIG_SYS_CS0_CTRL		0x00001980	/* WS=0110, AA=1, PS=10         */
/** Note: There is a CSMR0/DRAM vector problem, need to disable C/I ***/
#define	CONFIG_SYS_CS0_MASK		0x003f0021	/* 4MB, AA=0, WP=0, C/I=1, V=1  */

/* CS1 - FPGA, address 0xe0000000 */
#define	CONFIG_SYS_CS1_BASE		0xe0000000
#define	CONFIG_SYS_CS1_CTRL		0x00000d80	/* WS=0011, AA=1, PS=10         */
#define	CONFIG_SYS_CS1_MASK		0x00010001	/* 128kB, AA=0, WP=0, C/I=0, V=1*/

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define	CONFIG_SYS_GPIO_FUNC		0x00000008	/* Set gpio pins: none          */
#define	CONFIG_SYS_GPIO1_FUNC		0x00df00f0	/* 36-39(SWITCH),48-52(FPGAs),54*/
#define	CONFIG_SYS_GPIO_EN		0x00000008	/* Set gpio output enable       */
#define	CONFIG_SYS_GPIO1_EN		0x00c70000	/* Set gpio output enable       */
#define	CONFIG_SYS_GPIO_OUT		0x00000008	/* Set outputs to default state */
#define	CONFIG_SYS_GPIO1_OUT		0x00c70000	/* Set outputs to default state */
#define CONFIG_SYS_GPIO1_LED		0x00400000	/* user led                     */

#endif	/* M5249 */
