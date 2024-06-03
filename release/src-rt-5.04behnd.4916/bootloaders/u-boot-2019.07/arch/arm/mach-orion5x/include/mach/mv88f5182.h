/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * Based on original Kirkwood 88F6182 support which is
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Header file for Feroceon CPU core 88F5182 SOC.
 */

#ifndef _CONFIG_88F5182_H
#define _CONFIG_88F5182_H

/* SOC specific definitions */
#define F88F5182_REGS_PHYS_BASE		0xf1000000
#define ORION5X_REGS_PHY_BASE		F88F5182_REGS_PHYS_BASE

/* TCLK Core Clock defination */
#define CONFIG_SYS_TCLK			166000000 /* 166MHz */

#endif /* _CONFIG_88F5182_H */
