/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sysam stmark2 board configuration
 *
 * (C) Copyright 2017  Angelo Dureghello <angelo@sysam.it>
 */

#ifndef __STMARK2_CONFIG_H
#define __STMARK2_CONFIG_H

#define CFG_SYS_UART_PORT		0

#define LDS_BOARD_TEXT						\
	board/sysam/stmark2/sbf_dram_init.o (.text*)

#define CFG_EXTRA_ENV_SETTINGS				\
	"kern_size=0x700000\0"					\
	"loadaddr=0x40001000\0"					\
		"-(rootfs)\0"					\
	"update_uboot=loady ${loadaddr}; "			\
		"sf probe 0:1 50000000; "			\
		"sf erase 0 0x80000; "				\
		"sf write ${loadaddr} 0 ${filesize}\0"		\
	"update_kernel=loady ${loadaddr}; "			\
		"setenv kern_size ${filesize}; saveenv; "	\
		"sf probe 0:1 50000000; "			\
		"sf erase 0x100000 0x700000; "			\
		"sf write ${loadaddr} 0x100000 ${filesize}\0"	\
	"update_rootfs=loady ${loadaddr}; "			\
		"sf probe 0:1 50000000; "			\
		"sf erase 0x00800000 0x100000; "		\
		"sf write ${loadaddr} 0x00800000 ${filesize}\0"	\
	""

#define CFG_SYS_SBFHDR_SIZE		0x7

/* Input, PCI, Flexbus, and VCO */

#define CFG_PRAM			2048	/* 2048 KB */

#define CFG_SYS_MBAR			0xFC000000

/*
 * Definitions for initial stack pointer and data area (in internal SRAM)
 */
#define CFG_SYS_INIT_RAM_ADDR	0x80000000
/* End of used area in internal SRAM */
#define CFG_SYS_INIT_RAM_SIZE	0x10000
#define CFG_SYS_INIT_RAM_CTRL	0x221
#define CFG_SYS_INIT_SP_OFFSET	((CFG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE) - 32)
#define CFG_SYS_SBFHDR_DATA_OFFSET	(CFG_SYS_INIT_RAM_SIZE - 32)

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CFG_SYS_SDRAM_BASE		0x40000000
#define CFG_SYS_SDRAM_SIZE		128	/* SDRAM size in MB */

#define CFG_SYS_DRAM_TEST

/* Reserve 256 kB for Monitor */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
/* Initial Memory map for Linux */
#define CFG_SYS_BOOTMAPSZ		(CFG_SYS_SDRAM_BASE + \
					(CFG_SYS_SDRAM_SIZE << 20))

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */

/* Cache Configuration */
#define ICACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 4)
#define CFG_SYS_ICACHE_INV		(CF_CACR_BCINVA + CF_CACR_ICINVA)
#define CFG_SYS_DCACHE_INV		(CF_CACR_DCINVA)
#define CFG_SYS_CACHE_ACR2		(CFG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CFG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CFG_SYS_CACHE_ICACR		(CF_CACR_BEC | CF_CACR_IEC | \
					 CF_CACR_ICINVA | CF_CACR_EUSP)
#define CFG_SYS_CACHE_DCACR		((CFG_SYS_CACHE_ICACR | \
					 CF_CACR_DEC | CF_CACR_DDCM_P | \
					 CF_CACR_DCINVA) & ~CF_CACR_ICINVA)

#define CACR_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					CFG_SYS_INIT_RAM_SIZE - 12)


#define CFG_SYS_I2C_0

#endif /* __STMARK2_CONFIG_H */
