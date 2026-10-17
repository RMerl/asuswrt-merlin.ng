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

#undef CFG_SYS_AT91_MAIN_CLOCK
#define CFG_SYS_AT91_MAIN_CLOCK      24000000 /* from 24 MHz crystal */

/* SDRAM */
#define CFG_SYS_SDRAM_BASE		0x20000000
#define CFG_SYS_SDRAM_SIZE		0x20000000

/* NAND Flash */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE		ATMEL_BASE_CS3
/* our ALE is AD21 */
#define CFG_SYS_NAND_MASK_ALE	BIT(21)
/* our CLE is AD22 */
#define CFG_SYS_NAND_MASK_CLE	BIT(22)
#endif

#endif /* __CONFIG_H */
