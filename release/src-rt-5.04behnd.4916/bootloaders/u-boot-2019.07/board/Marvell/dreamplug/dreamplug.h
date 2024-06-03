/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Jason Cooper <u-boot@lakedaemon.net>
 *
 * Based on work by:
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Siddarth Gore <gores@marvell.com>
 */

#ifndef __DREAMPLUG_H
#define __DREAMPLUG_H

#define DREAMPLUG_OE_LOW	(~(0))
#define DREAMPLUG_OE_HIGH	(~(0))
#define DREAMPLUG_OE_VAL_LOW	0
#define DREAMPLUG_OE_VAL_HIGH	(0xf << 16) /* 4 LED Pins high */

/* PHY related */
#define MV88E1116_MAC_CTRL2_REG		21
#define MV88E1116_PGADR_REG		22
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

#endif /* __DREAMPLUG_H */
