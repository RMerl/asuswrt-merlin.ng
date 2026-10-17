/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for the SAMA7G5EK Board.
 *
 * Copyright (C) 2020 Microchip Corporation
 *		      Eugen Hristev <eugen.hristev@microchip.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_AT91_SLOW_CLOCK      32768
#define CFG_SYS_AT91_MAIN_CLOCK      24000000 /* from 24 MHz crystal */
/* SDRAM */
#define CFG_SYS_SDRAM_BASE		0x60000000
#define CFG_SYS_SDRAM_SIZE		0x20000000

#endif
