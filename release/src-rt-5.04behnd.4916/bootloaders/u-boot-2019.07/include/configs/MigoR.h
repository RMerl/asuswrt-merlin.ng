/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Renesas Solutions Migo-R board
 *
 * Copyright (C) 2007 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#ifndef __MIGO_R_H
#define __MIGO_R_H

#define CONFIG_CPU_SH7722	1

#define CONFIG_DISPLAY_BOARDINFO
#undef  CONFIG_SHOW_BOOT_PROGRESS

/* SMC9111 */
#define CONFIG_SMC91111
#define CONFIG_SMC91111_BASE    (0xB0000000)

/* MEMORY */
#define MIGO_R_SDRAM_BASE	(0x8C000000)
#define MIGO_R_FLASH_BASE_1	(0xA0000000)
#define MIGO_R_FLASH_BANK_SIZE	(64 * 1024 * 1024)

#define CONFIG_SYS_PBSIZE		256		/* Buffer size for Console output */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200 }	/* List of legal baudrate settings for this board */

/* SCIF */
#define CONFIG_CONS_SCIF0	1

#define CONFIG_SYS_MEMTEST_START	(MIGO_R_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (60 * 1024 * 1024))

/* Enable alternate, more extensive, memory test */
/* Scratch address used by the alternate memory test */
#undef  CONFIG_SYS_MEMTEST_SCRATCH

/* Enable temporary baudrate change while serial download */
#undef  CONFIG_SYS_LOADS_BAUD_CHANGE

#define CONFIG_SYS_SDRAM_BASE	(MIGO_R_SDRAM_BASE)
/* maybe more, but if so u-boot doesn't know about it... */
#define CONFIG_SYS_SDRAM_SIZE	(64 * 1024 * 1024)
/* default load address for scripts ?!? */
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 * 1024)

/* Address of u-boot image in Flash (NOT run time address in SDRAM) ?!? */
#define CONFIG_SYS_MONITOR_BASE	(MIGO_R_FLASH_BASE_1)
/* Monitor size */
#define CONFIG_SYS_MONITOR_LEN	(128 * 1024)
/* Size of DRAM reserved for malloc() use */
#define CONFIG_SYS_MALLOC_LEN	(256 * 1024)
#define CONFIG_SYS_BOOTMAPSZ	(8 * 1024 * 1024)

/* FLASH */
#undef  CONFIG_SYS_FLASH_QUIET_TEST
/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_EMPTY_INFO
/* Physical start address of Flash memory */
#define CONFIG_SYS_FLASH_BASE	(MIGO_R_FLASH_BASE_1)
/* Max number of sectors on each Flash chip */
#define CONFIG_SYS_MAX_FLASH_SECT	512

/* if you use all NOR Flash , you change dip-switch. Please see MIGO_R01 Manual. */
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE + (0 * MIGO_R_FLASH_BANK_SIZE) }

/* Timeout for Flash erase operations (in ms) */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(3 * 1000)
/* Timeout for Flash write operations (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(3 * 1000)
/* Timeout for Flash set sector lock bit operations (in ms) */
#define CONFIG_SYS_FLASH_LOCK_TOUT	(3 * 1000)
/* Timeout for Flash clear lock bit operations (in ms) */
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	(3 * 1000)

/* Use hardware flash sectors protection instead of U-Boot software protection */
#undef  CONFIG_SYS_DIRECT_FLASH_TFTP

/* ENV setting */
#define CONFIG_ENV_OVERWRITE	1
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SIZE		(CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
/* Offset of env Flash sector relative to CONFIG_SYS_FLASH_BASE */
#define CONFIG_ENV_OFFSET		(CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SECT_SIZE)

/* Board Clock */
#define CONFIG_SYS_CLK_FREQ	33333333
#define CONFIG_SH_SCIF_CLK_FREQ CONFIG_SYS_CLK_FREQ

#endif	/* __MIGO_R_H */
