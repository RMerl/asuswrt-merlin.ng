/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for the SAMA5D2 PTC EK Board.
 *
 * Copyright (C) 2017 Microchip Technology Inc.
 *		      Wenyou Yang <wenyou.yang@microchip.com>
 *		      Ludovic Desroches <ludovic.desroches@microchip.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "at91-sama5_common.h"

#undef CONFIG_SYS_AT91_MAIN_CLOCK
#define CONFIG_SYS_AT91_MAIN_CLOCK      24000000 /* from 24 MHz crystal */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x20000000

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_LOAD_ADDR		0x22000000 /* load address */

/* NAND Flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		ATMEL_BASE_CS3
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE	BIT(21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE	BIT(22)
#define CONFIG_SYS_NAND_ONFI_DETECTION
#endif

#endif /* __CONFIG_H */
