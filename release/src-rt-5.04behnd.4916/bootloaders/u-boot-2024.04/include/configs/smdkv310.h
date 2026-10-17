/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG SMDKV310 (EXYNOS4210) board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "exynos4-common.h"

/* High Level Configuration Options */
#define CFG_SYS_SDRAM_BASE		0x40000000

/* Handling Sleep Mode*/
#define S5P_CHECK_SLEEP			0x00000BAD
#define S5P_CHECK_DIDLE			0xBAD00000
#define S5P_CHECK_LPA			0xABAD0000

/* MMC SPL */
#define COPY_BL2_FNPTR_ADDR	0x00002488

/* SMDKV310 has 4 bank of DRAM */
#define SDRAM_BANK_SIZE		(512UL << 20UL)	/* 512 MB */
#define PHYS_SDRAM_1		CFG_SYS_SDRAM_BASE
#define PHYS_SDRAM_1_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_2		(CFG_SYS_SDRAM_BASE + SDRAM_BANK_SIZE)
#define PHYS_SDRAM_2_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_3		(CFG_SYS_SDRAM_BASE + (2 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_3_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_4		(CFG_SYS_SDRAM_BASE + (3 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_4_SIZE	SDRAM_BANK_SIZE

/* FLASH and environment organization */

/* Ethernet Controllor Driver */
#ifdef CONFIG_CMD_NET
#define CFG_ENV_SROM_BANK		1
#endif /*CONFIG_CMD_NET*/

#endif	/* __CONFIG_H */
