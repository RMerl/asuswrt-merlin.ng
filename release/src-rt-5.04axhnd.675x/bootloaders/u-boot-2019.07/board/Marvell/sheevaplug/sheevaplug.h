/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef __SHEEVAPLUG_H
#define __SHEEVAPLUG_H

#define SHEEVAPLUG_OE_LOW		(~(0))
#define SHEEVAPLUG_OE_HIGH		(~(0))
#define SHEEVAPLUG_OE_VAL_LOW		(1 << 29)	/* USB_PWEN low */
#define SHEEVAPLUG_OE_VAL_HIGH		(1 << 17)	/* LED pin high */

/* PHY related */
#define MV88E1116_LED_FCTRL_REG		10
#define MV88E1116_CPRSP_CR3_REG		21
#define MV88E1116_MAC_CTRL_REG		21
#define MV88E1116_PGADR_REG		22
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

#endif /* __SHEEVAPLUG_H */
