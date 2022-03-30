/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Net Insight <www.netinsight.net>
 * Written-by: Simon Kagstrom <simon.kagstrom@netinsight.net>
 *
 * Based on sheevaplug.h:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef __OPENRD_BASE_H
#define __OPENRD_BASE_H

#define OPENRD_OE_LOW		(~(1<<28))        /* RS232 / RS485 */
#define OPENRD_OE_HIGH		(~(1<<2))         /* SD / UART1 */
#define OPENRD_OE_VAL_LOW		(0)       /* Sel RS232 */
#define OPENRD_OE_VAL_HIGH		(1 << 2)  /* Sel SD */

/* PHY related */
#define MV88E1116_LED_FCTRL_REG		10
#define MV88E1116_CPRSP_CR3_REG		21
#define MV88E1116_MAC_CTRL_REG		21
#define MV88E1116_PGADR_REG		22
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

#endif /* __OPENRD_BASE_H */
