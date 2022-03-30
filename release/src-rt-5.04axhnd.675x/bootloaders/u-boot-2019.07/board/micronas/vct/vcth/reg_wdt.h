/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 */

#define WDT_BASE		0x000b0000
#define WDT_CR_OFFS		0x00000000
#define WDT_CR(base)		((base) + WDT_CR_OFFS)
#define WDT_TORR_OFFS		0x00000004
#define WDT_TORR(base)		((base) + WDT_TORR_OFFS)
