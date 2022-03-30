/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 */

#define DCGU_BASE		0x0004c000
#define DCGU_EN_WDT_RESET_OFFS	0x000000FC
#define DCGU_EN_WDT_RESET(base)	((base) + DCGU_EN_WDT_RESET_OFFS)

/* The magic value to write in order to activate the WDT */
#define DCGU_MAGIC_WDT		0x1909
