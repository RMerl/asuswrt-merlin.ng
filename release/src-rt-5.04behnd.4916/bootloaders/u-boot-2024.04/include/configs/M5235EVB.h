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

#ifndef _M5235EVB_H
#define _M5235EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CFG_SYS_UART_PORT		(0)

/* I2C */
#define CFG_SYS_I2C_PINMUX_REG	(gpio->par_qspi)
#define CFG_SYS_I2C_PINMUX_CLR	~(GPIO_PAR_FECI2C_SCL_MASK | GPIO_PAR_FECI2C_SDA_MASK)
#define CFG_SYS_I2C_PINMUX_SET	(GPIO_PAR_FECI2C_SCL_I2CSCL | GPIO_PAR_FECI2C_SDA_I2CSDA)

/* this must be included AFTER the definition of CONFIG COMMANDS (if any) */

#define CFG_EXTRA_ENV_SETTINGS		\
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

#define CFG_PRAM		512	/* 512 KB */

#define CFG_SYS_CLK			75000000
#define CFG_SYS_CPU_CLK		CFG_SYS_CLK * 2

#define CFG_SYS_MBAR		0x40000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_SYS_INIT_RAM_ADDR	0x20000000
#define CFG_SYS_INIT_RAM_SIZE	0x10000	/* Size of used area in internal SRAM */
#define CFG_SYS_INIT_RAM_CTRL	0x21

/*-----------------------------------------------------------------------
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
/* Initial Memory map for Linux */
#define CFG_SYS_BOOTMAPSZ		(CFG_SYS_SDRAM_BASE + (CFG_SYS_SDRAM_SIZE << 20))

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#ifdef CONFIG_SYS_FLASH_CFI
#	define CFG_SYS_FLASH_SIZE		0x800000	/* Max size that the board might have */
#endif

#define CFG_SYS_FLASH_BASE		(CFG_SYS_CS0_BASE)

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text);

/*-----------------------------------------------------------------------
 * Cache Configuration
 */

#define ICACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 4)
#define CFG_SYS_ICACHE_INV		(CF_CACR_CINV)
#define CFG_SYS_CACHE_ACR0		(CFG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CFG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CFG_SYS_CACHE_ICACR		(CF_CACR_CENB | CF_CACR_DISD | \
					 CF_CACR_CEIB | CF_CACR_DCM | \
					 CF_CACR_EUSP)

/*-----------------------------------------------------------------------
 * Chipselect bank definitions
 */
/*
 * CS0 - NOR Flash 1, 2, 4, or 8MB
 * CS1 - Available
 * CS2 - Available
 * CS3 - Available
 * CS4 - Available
 * CS5 - Available
 * CS6 - Available
 * CS7 - Available
 */
#ifdef CONFIG_NORFLASH_PS32BIT
#	define CFG_SYS_CS0_BASE	0xFFC00000
#	define CFG_SYS_CS0_MASK	0x003f0001
#	define CFG_SYS_CS0_CTRL	0x00001D00
#else
#	define CFG_SYS_CS0_BASE	0xFFE00000
#	define CFG_SYS_CS0_MASK	0x001f0001
#	define CFG_SYS_CS0_CTRL	0x00001D80
#endif


#endif				/* _M5329EVB_H */
