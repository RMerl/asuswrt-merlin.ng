/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sysam AMCORE board configuration
 *
 * (C) Copyright 2016  Angelo Dureghello <angelo@sysam.it>
 */

#ifndef __AMCORE_CONFIG_H
#define __AMCORE_CONFIG_H

#define CFG_SYS_UART_PORT		0

#define CFG_SYS_UART_PORT		0
#define CFG_SYS_BAUDRATE_TABLE		{ 9600, 19200, 38400, 57600, 115200 }

#define CFG_EXTRA_ENV_SETTINGS					\
	"upgrade_uboot=loady; "					\
		"protect off 0xffc00000 0xffc2ffff; "		\
		"erase 0xffc00000 0xffc2ffff; "			\
		"cp.b 0x20000 0xffc00000 ${filesize}\0"		\
	"upgrade_kernel=loady; "				\
		"erase 0xffc30000 0xffefffff; "			\
		"cp.b 0x20000 0xffc30000 ${filesize}\0"		\
	"upgrade_jffs2=loady; "					\
		"erase 0xfff00000 0xffffffff; "			\
		"cp.b 0x20000 0xfff00000 ${filesize}\0"

#define CFG_SYS_CLK			45000000
#define CFG_SYS_CPU_CLK		(CFG_SYS_CLK * 2)
/* Register Base Addrs */
#define CFG_SYS_MBAR			0x10000000
/* Definitions for initial stack pointer and data area (in DPRAM) */
#define CFG_SYS_INIT_RAM_ADDR	0x20000000
/* size of internal SRAM */
#define CFG_SYS_INIT_RAM_SIZE	0x1000

#define CFG_SYS_SDRAM_BASE		0x00000000
#define CFG_SYS_SDRAM_SIZE		0x1000000
#define CFG_SYS_FLASH_BASE		0xffc00000

/* amcore design has flash data bytes wired swapped */
#define CFG_SYS_WRITE_SWAPPED_DATA

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text*);

/* memory map space for linux boot data */
#define CFG_SYS_BOOTMAPSZ		(8 << 20)

/*
 * Cache Configuration
 *
 * Special 8K version 3 core cache.
 * This is a single unified instruction/data cache.
 * sdram - single region - no masks
 */

#define ICACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 4)
#define CFG_SYS_ICACHE_INV           (CF_CACR_CINVA)
#define CFG_SYS_CACHE_ACR0		(CF_ACR_CM_WT | CF_ACR_SM_ALL | \
					 CF_ACR_EN)
#define CFG_SYS_CACHE_ICACR		(CF_CACR_DCM_P | CF_CACR_ESB | \
					 CF_CACR_EC)

/* CS0 - AMD Flash, address 0xffc00000 */
#define	CFG_SYS_CS0_BASE		(CFG_SYS_FLASH_BASE>>16)
/* 4MB, AA=0,V=1  C/I BIT for errata */
#define	CFG_SYS_CS0_MASK		0x003f0001
/* WS=10, AA=1, PS=16bit (10) */
#define	CFG_SYS_CS0_CTRL		0x1980
/* CS1 - DM9000 Ethernet Controller, address 0x30000000 */
#define CFG_SYS_CS1_BASE		0x3000
#define CFG_SYS_CS1_MASK		0x00070001
#define CFG_SYS_CS1_CTRL		0x0100

#endif  /* __AMCORE_CONFIG_H */
