/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015
 * Gerald Kerma <dreagle@doukki.net>
 * Tony Dinh <mibodhi@gmail.com>
 */

#ifndef __NSA310S_H
#define __NSA310S_H

/* low GPIO's */
#define HDD1_GREEN_LED		(1 << 16)
#define HDD1_RED_LED		(1 << 13)
#define USB_GREEN_LED		(1 << 15)
#define USB_POWER			(1 << 21)
#define SYS_GREEN_LED		(1 << 28)
#define SYS_ORANGE_LED		(1 << 29)

#define COPY_GREEN_LED		(1 << 22)
#define COPY_RED_LED		(1 << 23)

#define PIN_USB_GREEN_LED	15
#define PIN_USB_POWER		21

#define NSA310S_OE_LOW		(~(0))
#define NSA310S_VAL_LOW		(SYS_GREEN_LED | USB_POWER)

/* high GPIO's */
#define HDD2_GREEN_LED		(1 << 2)
#define HDD2_POWER			(1 << 1)

#define NSA310S_OE_HIGH		(~(0))
#define NSA310S_VAL_HIGH	(HDD2_POWER)

/* PHY related */
#define MV88E1318_PGADR_REG		22
#define MV88E1318_MAC_CTRL_PG	2
#define MV88E1318_MAC_CTRL_REG	21
#define MV88E1318_RGMII_TX_CTRL	(1 << 4)
#define MV88E1318_RGMII_RX_CTRL	(1 << 5)
#define MV88E1318_LED_PG		3
#define MV88E1318_LED_POL_REG	17
#define MV88E1318_LED2_4		(1 << 4)
#define MV88E1318_LED2_5		(1 << 5)

#endif /* __NSA310S_H */
