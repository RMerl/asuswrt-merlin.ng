/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/configs/blanche.h
 *     This file is blanche board configuration.
 *
 * Copyright (C) 2016 Renesas Electronics Corporation
 */

#ifndef __BLANCHE_H
#define __BLANCHE_H

#include "rcar-gen2-common.h"

/* STACK */
#define STACK_AREA_SIZE			0x00100000
#define LOW_LEVEL_MERAM_STACK \
		(SYS_INIT_SP_ADDR + STACK_AREA_SIZE - 4)

/* MEMORY */
#define RCAR_GEN2_SDRAM_BASE		0x40000000
#define RCAR_GEN2_SDRAM_SIZE		(1024u * 1024 * 1024)
#define RCAR_GEN2_UBOOT_SDRAM_SIZE	(512 * 1024 * 1024)

/* FLASH */
#if defined(CONFIG_MTD_NOR_FLASH)
#define CFG_SYS_FLASH_BASE		0x00000000
#define CFG_SYS_FLASH_SIZE		0x04000000	/* 64 MB */
#define CFG_SYS_FLASH_BANKS_LIST	{ (CFG_SYS_FLASH_BASE) }
#define CFG_SYS_FLASH_BANKS_SIZES	{ (CFG_SYS_FLASH_SIZE) }
#endif

/* Board Clock */

/* ENV setting */

#endif	/* __BLANCHE_H */
