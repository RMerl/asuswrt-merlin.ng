/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012
 * David Purdy <david.c.purdy@gmail.com>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef __POGO_E02_H
#define __POGO_E02_H

/* GPIO configuration */
#define POGO_E02_OE_LOW				(~(0))
#define POGO_E02_OE_HIGH			(~(0))
#define POGO_E02_OE_VAL_LOW			(1 << 29)
#define POGO_E02_OE_VAL_HIGH			0

/* PHY related */
#define MV88E1116_LED_FCTRL_REG			10
#define MV88E1116_CPRSP_CR3_REG			21
#define MV88E1116_MAC_CTRL_REG			21
#define MV88E1116_PGADR_REG			22
#define MV88E1116_RGMII_TXTM_CTRL		(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL		(1 << 5)

#endif /* __POGO_E02_H */
