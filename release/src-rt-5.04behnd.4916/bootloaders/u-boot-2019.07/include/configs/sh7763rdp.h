/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Renesas SH7763RDP board
 *
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2008 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 */

#ifndef __SH7763RDP_H
#define __SH7763RDP_H

#define CONFIG_CPU_SH7763	1
#define __LITTLE_ENDIAN		1

#define CONFIG_ENV_OVERWRITE    1

#define CONFIG_DISPLAY_BOARDINFO
#undef  CONFIG_SHOW_BOOT_PROGRESS

/* SCIF */
#define CONFIG_CONS_SCIF2		1

#define CONFIG_SYS_PBSIZE		256	/* Buffer size for Console output */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200 }	/* List of legal baudrate
												settings for this board */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		(0x8C000000)
#define CONFIG_SYS_SDRAM_SIZE		(64 * 1024 * 1024)
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (60 * 1024 * 1024))

/* Flash(NOR) */
#define CONFIG_SYS_FLASH_BASE		(0xA0000000)
#define CONFIG_SYS_FLASH_CFI_WIDTH (FLASH_CFI_16BIT)
#define CONFIG_SYS_MAX_FLASH_BANKS (1)
#define CONFIG_SYS_MAX_FLASH_SECT  (520)

/* U-Boot setting */
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 4 * 1024 * 1024)
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_MONITOR_LEN		(128 * 1024)
/* Size of DRAM reserved for malloc() use */
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)
#define CONFIG_SYS_BOOTMAPSZ		(8 * 1024 * 1024)

#undef  CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* print 'E' for empty sector on flinfo */
/* Timeout for Flash erase operations (in ms) */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(3 * 1000)
/* Timeout for Flash write operations (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(3 * 1000)
/* Timeout for Flash set sector lock bit operations (in ms) */
#define CONFIG_SYS_FLASH_LOCK_TOUT		(3 * 1000)
/* Timeout for Flash clear lock bit operations (in ms) */
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	(3 * 1000)
/* Use hardware flash sectors protection instead of U-Boot software protection */
#undef  CONFIG_SYS_DIRECT_FLASH_TFTP
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SIZE		(CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + (1 * CONFIG_ENV_SECT_SIZE))
/* Offset of env Flash sector relative to CONFIG_SYS_FLASH_BASE */
#define CONFIG_ENV_OFFSET		(CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_SYS_FLASH_BASE + (2 * CONFIG_ENV_SECT_SIZE))

/* Clock */
#define CONFIG_SYS_CLK_FREQ	66666666
#define CONFIG_SH_SCIF_CLK_FREQ CONFIG_SYS_CLK_FREQ

/* Ether */
#define CONFIG_SH_ETHER_USE_PORT (1)
#define CONFIG_SH_ETHER_PHY_ADDR (0x01)
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI
#define CONFIG_SH_ETHER_PHY_MODE PHY_INTERFACE_MODE_MII

#endif /* __SH7763RDP_H */
