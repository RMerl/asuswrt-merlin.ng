/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Siddarth Gore <gores@marvell.com>
 */

#ifndef __GURUPLUG_H
#define __GURUPLUG_H

#define GURUPLUG_OE_LOW		(~(0))
#define GURUPLUG_OE_HIGH	(~(0))
#define GURUPLUG_OE_VAL_LOW	0
#define GURUPLUG_OE_VAL_HIGH	(0xf << 16) /* 4 LED Pins high */

/* PHY related */
#define MV88E1121_MAC_CTRL2_REG		21
#define MV88E1121_PGADR_REG		22
#define MV88E1121_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1121_RGMII_RXTM_CTRL	(1 << 5)

#endif /* __GURUPLUG_H */
