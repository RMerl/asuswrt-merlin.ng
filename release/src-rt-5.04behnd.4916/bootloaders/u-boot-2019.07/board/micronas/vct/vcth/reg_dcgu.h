/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008-2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 */

#define DCGU_BASE		0x00084000

/* Relative offsets of the register adresses */

#define DCGU_CLK_EN1_OFFS	0x00000010
#define DCGU_CLK_EN1(base)	((base) + DCGU_CLK_EN1_OFFS)
#define DCGU_CLK_EN2_OFFS	0x00000014
#define DCGU_CLK_EN2(base)	((base) + DCGU_CLK_EN2_OFFS)
#define DCGU_RESET_UNIT1_OFFS	0x00000018
#define DCGU_RESET_UNIT1(base)	((base) + DCGU_RESET_UNIT1_OFFS)
#define DCGU_USBPHY_STAT_OFFS	0x00000054
#define DCGU_USBPHY_STAT(base)	((base) + DCGU_USBPHY_STAT_OFFS)
#define DCGU_EN_WDT_RESET_OFFS	0x00000064
#define DCGU_EN_WDT_RESET(base)	((base) + DCGU_EN_WDT_RESET_OFFS)

/* The magic value to write in order to activate the WDT */
#define DCGU_MAGIC_WDT		0x1909
