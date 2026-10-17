/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011-2012
 * Gerald Kerma <dreagle@doukki.net>
 * Simon Baatz <gmbnomis@gmail.com>
 * Luka Perkov <luka@openwrt.org>
 */

#ifndef __IB62x0_H
#define __IB62x0_H

#define IB62x0_OE_LOW		(~(1 << 22 | 1 << 24 | 1 << 25 | 1 << 27))
#define IB62x0_OE_HIGH		(~(0))
#define IB62x0_OE_VAL_LOW	0
#define IB62x0_OE_VAL_HIGH	0

/* PHY related */
#define MV88E1116_LED_FCTRL_REG		10
#define MV88E1116_CPRSP_CR3_REG		21
#define MV88E1116_MAC_CTRL_REG		21
#define MV88E1116_PGADR_REG		22
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

/* SATAHC related */
#define MVSATAHC_LED_CONF_REG       (MV_SATA_BASE + 0x2C)
#define MVSATAHC_LED_POLARITY_CTRL  (1 << 3)

#endif /* __IB62x0_H */
