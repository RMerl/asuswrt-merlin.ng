/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * Hayden Fraser (Hayden.Fraser@freescale.com)
 */

#ifndef _M5253DEMO_H
#define _M5253DEMO_H

#include <linux/stringify.h>

#define CFG_SYS_UART_PORT		(0)


/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text*);

#ifdef CONFIG_DRIVER_DM9000
#	define CFG_EXTRA_ENV_SETTINGS		\
		"netdev=eth0\0"				\
		"inpclk=" __stringify(CONFIG_SYS_INPUT_CLKSRC) "\0"	\
		"loadaddr=10000\0"			\
		"u-boot=u-boot.bin\0"			\
		"load=tftp ${loadaddr) ${u-boot}\0"	\
		"upd=run load; run prog\0"		\
		"prog=prot off 0xff800000 0xff82ffff;"	\
		"era 0xff800000 0xff82ffff;"		\
		"cp.b ${loadaddr} 0xff800000 ${filesize};"	\
		"save\0"				\
		""
#endif

/* I2C */
#define CFG_SYS_I2C_PINMUX_REG	(*(u32 *) (CFG_SYS_MBAR+0x19C))
#define CFG_SYS_I2C_PINMUX_CLR	(0xFFFFE7FF)
#define CFG_SYS_I2C_PINMUX_SET	(0)

#undef CFG_SYS_PLL_BYPASS		/* bypass PLL for test purpose */
#define CFG_SYS_FAST_CLK
#ifdef CFG_SYS_FAST_CLK
#	define CFG_SYS_PLLCR	0x1243E054
#	define CFG_SYS_CLK		140000000
#else
#	define CFG_SYS_PLLCR	0x135a4140
#	define CFG_SYS_CLK		70000000
#endif

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CFG_SYS_MBAR		0x10000000	/* Register Base Addrs */
#define CFG_SYS_MBAR2		0x80000000	/* Module Base Addrs 2 */

/*
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_SYS_INIT_RAM_ADDR	0x20000000
#define CFG_SYS_INIT_RAM_SIZE	0x10000	/* Size of used area in internal SRAM */

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CFG_SYS_SDRAM_BASE		0x00000000
#define CFG_SYS_SDRAM_SIZE		16	/* SDRAM size in MB */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CFG_SYS_BOOTMAPSZ		(CFG_SYS_SDRAM_BASE + (CFG_SYS_SDRAM_SIZE << 20))

/* FLASH organization */
#define CFG_SYS_FLASH_BASE		(CFG_SYS_CS0_BASE)

#define FLASH_SST6401B		0x200
#define SST_ID_xF6401B		0x236D236D

#ifdef CONFIG_SYS_FLASH_CFI
/*
 * Unable to use CFI driver, due to incompatible sector erase command by SST.
 * Amd/Atmel use 0x30 for sector erase, SST use 0x50.
 * 0x30 is block erase in SST
 */
#	define CFG_SYS_FLASH_SIZE		0x800000
#else
#	define CFG_SYS_SST_SECT		2048
#	define CFG_SYS_SST_SECTSZ		0x1000
#endif

/* Cache Configuration */

#define ICACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 4)
#define CFG_SYS_ICACHE_INV		(CF_CACR_DCM)
#define CFG_SYS_CACHE_ACR0		(CFG_SYS_FLASH_BASE | \
					 CF_ADDRMASK(8) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CFG_SYS_CACHE_ACR1		(CFG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CFG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CFG_SYS_CACHE_ICACR		(CF_CACR_CENB | CF_CACR_CEIB | \
					 CF_CACR_DBWE)

#define CFG_SYS_CS0_BASE		0xFF800000
#define CFG_SYS_CS0_MASK		0x007F0021
#define CFG_SYS_CS0_CTRL		0x00001D80

#define CFG_SYS_CS1_BASE		0xE0000000
#define CFG_SYS_CS1_MASK		0x00000001
#define CFG_SYS_CS1_CTRL		0x00003DD8

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define CFG_SYS_GPIO_FUNC		0x00000008	/* Set gpio pins: none */
#define CFG_SYS_GPIO1_FUNC		0x00df00f0	/* 36-39(SWITCH),48-52(FPGAs),54 */
#define CFG_SYS_GPIO_EN		0x00000008	/* Set gpio output enable */
#define CFG_SYS_GPIO1_EN		0x00c70000	/* Set gpio output enable */
#define CFG_SYS_GPIO_OUT		0x00000008	/* Set outputs to default state */
#define CFG_SYS_GPIO1_OUT		0x00c70000	/* Set outputs to default state */
#define CFG_SYS_GPIO1_LED		0x00400000	/* user led */


#endif				/* _M5253DEMO_H */
