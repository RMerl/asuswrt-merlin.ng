/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Header file for Feroceon CPU core 88FR131 Based KW88F6192 SOC.
 */

#ifndef _CONFIG_KW88F6192_H
#define _CONFIG_KW88F6192_H

/* SOC specific definations */
#define KW88F6192_REGS_PHYS_BASE	0xf1000000
#define KW_REGS_PHY_BASE		KW88F6192_REGS_PHYS_BASE

/* TCLK Core Clock defination */
#define CONFIG_SYS_TCLK	  166000000 /* 166MHz */

#endif /* _CONFIG_KW88F6192_H */
