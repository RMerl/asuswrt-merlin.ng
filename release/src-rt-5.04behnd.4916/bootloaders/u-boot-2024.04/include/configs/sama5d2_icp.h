/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for the SAMA5D2 ICP Board.
 *
 * Copyright (C) 2018 Microchip Corporation
 *		      Eugen Hristev <eugen.hristev@microchip.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "at91-sama5_common.h"

#undef CFG_SYS_AT91_MAIN_CLOCK
#define CFG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */

/* SDRAM */
#define CFG_SYS_SDRAM_BASE		0x20000000
#define CFG_SYS_SDRAM_SIZE		0x20000000

#ifdef CONFIG_SD_BOOT
/* u-boot env in sd/mmc card */
#define FAT_ENV_INTERFACE	"mmc"
#define FAT_ENV_DEVICE_AND_PART	"0"
#define FAT_ENV_FILE		"uboot.env"
/* bootstrap + u-boot + env in sd card */
#endif

/* SPL */

#endif
