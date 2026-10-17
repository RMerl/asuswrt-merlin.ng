/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/rcar-gen4-common.h
 *	This file is R-Car Gen4 common configuration file.
 *
 * Copyright (C) 2021 Renesas Electronics Corporation
 */

#ifndef __RCAR_GEN4_COMMON_H
#define __RCAR_GEN4_COMMON_H

#include <asm/arch/renesas.h>

/* Console */
#define CFG_SYS_BAUDRATE_TABLE	{ 38400, 115200, 921600, 1843200 }

/* Memory */
#define DRAM_RSV_SIZE			0x08000000
#define CFG_SYS_SDRAM_BASE		(0x40000000 + DRAM_RSV_SIZE)
#define CFG_SYS_SDRAM_SIZE		(0x80000000u - DRAM_RSV_SIZE)
#define CFG_MAX_MEM_MAPPED		(0x80000000u - DRAM_RSV_SIZE)

/* PHY needs a longer autoneg timeout */
#define PHY_ANEG_TIMEOUT		20000

/* Environment setting */
#define CFG_EXTRA_ENV_SETTINGS					\
	"bootm_size=0x10000000\0"

#endif	/* __RCAR_GEN4_COMMON_H */
