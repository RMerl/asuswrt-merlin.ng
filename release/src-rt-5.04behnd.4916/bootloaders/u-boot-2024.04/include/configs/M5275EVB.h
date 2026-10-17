/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Motorola MC5275EVB board.
 *
 * By Arthur Shipkowski <art@videon-central.com>
 * Copyright (C) 2005 Videon Central, Inc.
 *
 * Based off of M5272C3 board code by Josef Baumgartner
 * <josef.baumgartner@telex.de>
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef _M5275EVB_H
#define _M5275EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CFG_SYS_UART_PORT		(0)

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text);

/* Available command configuration */

/* I2C */
#define CFG_SYS_I2C_PINMUX_REG	(gpio_reg->par_feci2c)
#define CFG_SYS_I2C_PINMUX_CLR	(0xFFF0)
#define CFG_SYS_I2C_PINMUX_SET	(0x000F)

#define CFG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"loadaddr=10000\0"			\
	"uboot=u-boot.bin\0"			\
	"load=tftp ${loadaddr} ${uboot}\0"	\
	"upd=run load; run prog\0"		\
	"prog=prot off ffe00000 ffe3ffff;"	\
	"era ffe00000 ffe3ffff;"		\
	"cp.b ${loadaddr} ffe00000 ${filesize};"\
	"save\0"				\
	""

#define CFG_SYS_CLK			150000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CFG_SYS_MBAR		0x40000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_SYS_INIT_RAM_ADDR	0x20000000
#define CFG_SYS_INIT_RAM_SIZE	0x10000	/* Size of used area in internal SRAM */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CFG_SYS_SDRAM_BASE		0x00000000
#define CFG_SYS_SDRAM_SIZE		16	/* SDRAM size in MB */
#define CFG_SYS_FLASH_BASE		CFG_SYS_CS0_BASE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CFG_SYS_BOOTMAPSZ		(CFG_SYS_SDRAM_BASE + (CFG_SYS_SDRAM_SIZE << 20))

/*-----------------------------------------------------------------------
 * FLASH organization
 */

#define CFG_SYS_FLASH_SIZE		0x200000

/*-----------------------------------------------------------------------
 * Cache Configuration
 */

#define ICACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 4)
#define CFG_SYS_ICACHE_INV		(CF_CACR_CINV | CF_CACR_INVI)
#define CFG_SYS_CACHE_ACR0		(CFG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CFG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CFG_SYS_CACHE_ICACR		(CF_CACR_CENB | CF_CACR_CINV | \
					 CF_CACR_DISD | CF_CACR_INVI | \
					 CF_CACR_CEIB | CF_CACR_DCM | \
					 CF_CACR_EUSP)

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */
#define CFG_SYS_CS0_BASE		0xffe00000
#define CFG_SYS_CS0_CTRL		0x00001980
#define CFG_SYS_CS0_MASK		0x001F0001

#define CFG_SYS_CS1_BASE		0x30000000
#define CFG_SYS_CS1_CTRL		0x00001900
#define CFG_SYS_CS1_MASK		0x00070001


#endif	/* _M5275EVB_H */
