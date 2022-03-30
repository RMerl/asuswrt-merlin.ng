/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the Renesas GRPEACH board
 *
 * Copyright (C) 2017-2019 Renesas Electronics
 */

#ifndef __GRPEACH_H
#define __GRPEACH_H

/* Board Clock , P1 clock frequency (XTAL=13.33MHz) */
#define CONFIG_SYS_CLK_FREQ	66666666

/* Serial Console */
#define CONFIG_BAUDRATE		115200

/* Miscellaneous */
#define CONFIG_SYS_PBSIZE	256
#define CONFIG_SYS_ARM_CACHE_WRITETHROUGH
#define CONFIG_CMDLINE_TAG
#define CONFIG_ARCH_CPU_INIT

/* Internal RAM Size (RZ/A1=3M, RZ/A1M=5M, RZ/A1H=10M) */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		(10 * 1024 * 1024)
#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_SDRAM_SIZE - 1024 * 1024)
#define CONFIG_SYS_LOAD_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 4 * 1024 * 1024)

#define CONFIG_ENV_OVERWRITE		1
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_SIZE			(CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_OFFSET		0x80000

/* Malloc */
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)

/* Kernel Boot */
#define CONFIG_BOOTARGS			"ignore_loglevel"

/* Network interface */
#define CONFIG_SH_ETHER_USE_PORT	0
#define CONFIG_SH_ETHER_PHY_ADDR	0
#define CONFIG_SH_ETHER_PHY_MODE PHY_INTERFACE_MODE_MII
#define CONFIG_SH_ETHER_CACHE_WRITEBACK
#define CONFIG_SH_ETHER_CACHE_INVALIDATE
#define CONFIG_SH_ETHER_ALIGNE_SIZE	64
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

#endif	/* __GRPEACH_H */
