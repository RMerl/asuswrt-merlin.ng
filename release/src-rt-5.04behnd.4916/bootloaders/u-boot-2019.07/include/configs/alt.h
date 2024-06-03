/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/configs/alt.h
 *     This file is alt board configuration.
 *
 * Copyright (C) 2014 Renesas Electronics Corporation
 */

#ifndef __ALT_H
#define __ALT_H

#include "rcar-gen2-common.h"

#define CONFIG_SYS_INIT_SP_ADDR		0x4f000000
#define STACK_AREA_SIZE			0x00100000
#define LOW_LEVEL_MERAM_STACK \
		(CONFIG_SYS_INIT_SP_ADDR + STACK_AREA_SIZE - 4)

/* MEMORY */
#define RCAR_GEN2_SDRAM_BASE		0x40000000
#define RCAR_GEN2_SDRAM_SIZE		(1024u * 1024 * 1024)
#define RCAR_GEN2_UBOOT_SDRAM_SIZE	(512 * 1024 * 1024)

/* FLASH */
#define CONFIG_SPI_FLASH_QUAD

/* SH Ether */
#define CONFIG_SH_ETHER_USE_PORT	0
#define CONFIG_SH_ETHER_PHY_ADDR	0x1
#define CONFIG_SH_ETHER_PHY_MODE PHY_INTERFACE_MODE_RMII
#define CONFIG_SH_ETHER_CACHE_WRITEBACK
#define CONFIG_SH_ETHER_CACHE_INVALIDATE
#define CONFIG_SH_ETHER_ALIGNE_SIZE	64
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Board Clock */
#define RMOBILE_XTAL_CLK	20000000u
#define CONFIG_SYS_CLK_FREQ	RMOBILE_XTAL_CLK

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"bootm_size=0x10000000\0"	\
	"usb_pgood_delay=2000\0"

/* SPL support */
#define CONFIG_SPL_STACK		0xe6340000
#define CONFIG_SPL_MAX_SIZE		0x4000
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x140000
#ifdef CONFIG_SPL_BUILD
#define CONFIG_CONS_SCIF2
#define CONFIG_SH_SCIF_CLK_FREQ		65000000
#endif

#endif /* __ALT_H */
