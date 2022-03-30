/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010  Eric C. Cooper <ecc@cmu.edu>
 *
 * Based on sheevaplug.h originally written by
 * Prafulla Wadaskar <prafulla@marvell.com>
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 */

#ifndef __DOCKSTAR_H
#define __DOCKSTAR_H

#define DOCKSTAR_OE_LOW		(~(0))
#define DOCKSTAR_OE_HIGH		(~(0))
#define DOCKSTAR_OE_VAL_LOW		(1 << 29)	/* USB_PWEN low */
#define DOCKSTAR_OE_VAL_HIGH		(1 << 17)	/* LED pin high */

/* PHY related */
#define MV88E1116_LED_FCTRL_REG		10
#define MV88E1116_CPRSP_CR3_REG		21
#define MV88E1116_MAC_CTRL_REG		21
#define MV88E1116_PGADR_REG		22
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

#endif /* __DOCKSTAR_H */
