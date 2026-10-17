/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009-2012
 * Wojciech Dubowik <wojciech.dubowik@neratec.com>
 * Luka Perkov <luka@openwrt.org>
 */

#ifndef __DS109_H
#define __DS109_H

#define DS109_OE_LOW			(0)
#define DS109_OE_HIGH			(0)
#define DS109_OE_VAL_LOW		((1 << 22)|(1 << 23))
#define DS109_OE_VAL_HIGH		((1 << 1)|1)

/* PHY related */
#define MV88E1116_LED_FCTRL_REG		10
#define MV88E1116_CPRSP_CR3_REG		21
#define MV88E1116_MAC_CTRL_REG		21
#define MV88E1116_MAC_CTRL2_REG		21

#define MV88E1116_PGADR_REG		22
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

#endif /* __DS109_H */
