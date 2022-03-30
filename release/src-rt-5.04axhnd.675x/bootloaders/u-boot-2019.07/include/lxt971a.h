/* SPDX-License-Identifier: GPL-2.0+ */
/***********************************************************************
 *
 * Copyright (C) 2004 by FS Forth-Systeme GmbH.
 * All rights reserved.
 *
 * $Id: ns9750_eth.h,v 1.2 2004/02/24 13:25:39 mpietrek Exp $
 * @Author: Markus Pietrek
 * @References: [1] NS9750 Hardware Reference, December 2003
 *              [2] Intel LXT971 Datasheet #249414 Rev. 02
 *              [3] NS7520 Linux Ethernet Driver
 */

#ifndef __LXT971A_H__
#define __LXT971A_H__

/* PHY definitions (LXT971A) [2] */
#define PHY_LXT971_PORT_CFG		(0x10)
#define PHY_LXT971_STAT2		(0x11)
#define PHY_LXT971_INT_ENABLE		(0x12)
#define PHY_LXT971_INT_STATUS		(0x13)
#define PHY_LXT971_LED_CFG		(0x14)
#define PHY_LXT971_DIG_CFG		(0x1A)
#define PHY_LXT971_TX_CTRL		(0x1E)

/* PORT_CFG Port Configuration Register Bit Fields */
#define PHY_LXT971_PORT_CFG_RES1        (0x8000)
#define PHY_LXT971_PORT_CFG_FORCE_LNK   (0x4000)
#define PHY_LXT971_PORT_CFG_TX_DISABLE  (0x2000)
#define PHY_LXT971_PORT_CFG_BYPASS_SCR  (0x1000)
#define PHY_LXT971_PORT_CFG_RES2        (0x0800)
#define PHY_LXT971_PORT_CFG_JABBER      (0x0400)
#define PHY_LXT971_PORT_CFG_SQE	        (0x0200)
#define PHY_LXT971_PORT_CFG_TP_LOOPBACK (0x0100)
#define PHY_LXT971_PORT_CFG_CRS_SEL     (0x0080)
#define PHY_LXT971_PORT_CFG_SLEEP_MODE  (0x0040)
#define PHY_LXT971_PORT_CFG_PRE_EN      (0x0020)
#define PHY_LXT971_PORT_CFG_SLEEP_T_MA  (0x0018)
#define PHY_LXT971_PORT_CFG_SLEEP_T_104 (0x0010)
#define PHY_LXT971_PORT_CFG_SLEEP_T_200 (0x0001)
#define PHY_LXT971_PORT_CFG_SLEEP_T_304 (0x0000)
#define PHY_LXT971_PORT_CFG_FLT_CODE_EN (0x0004)
#define PHY_LXT971_PORT_CFG_ALT_NP      (0x0002)
#define PHY_LXT971_PORT_CFG_FIBER_SEL   (0x0001)

/* STAT2 Status Register #2 Bit Fields */
#define PHY_LXT971_STAT2_RES1		(0x8000)
#define PHY_LXT971_STAT2_100BTX		(0x4000)
#define PHY_LXT971_STAT2_TX_STATUS	(0x2000)
#define PHY_LXT971_STAT2_RX_STATUS	(0x1000)
#define PHY_LXT971_STAT2_COL_STATUS	(0x0800)
#define PHY_LXT971_STAT2_LINK		(0x0400)
#define PHY_LXT971_STAT2_DUPLEX_MODE	(0x0200)
#define PHY_LXT971_STAT2_AUTO_NEG	(0x0100)
#define PHY_LXT971_STAT2_AUTO_NEG_COMP	(0x0080)
#define PHY_LXT971_STAT2_RES2		(0x0040)
#define PHY_LXT971_STAT2_POLARITY	(0x0020)
#define PHY_LXT971_STAT2_PAUSE		(0x0010)
#define PHY_LXT971_STAT2_ERROR		(0x0008)
#define PHY_LXT971_STAT2_RES3		(0x0007)

/* INT_ENABLE Interrupt Enable Register Bit Fields */
#define PHY_LXT971_INT_ENABLE_RES1      (0xFF00)
#define PHY_LXT971_INT_ENABLE_ANMSK     (0x0080)
#define PHY_LXT971_INT_ENABLE_SPEEDMSK  (0x0040)
#define PHY_LXT971_INT_ENABLE_DUPLEXMSK (0x0020)
#define PHY_LXT971_INT_ENABLE_LINKMSK   (0x0010)
#define PHY_LXT971_INT_ENABLE_RES2      (0x000C)
#define PHY_LXT971_INT_ENABLE_INTEN     (0x0002)
#define PHY_LXT971_INT_ENABLE_TINT      (0x0001)

/* INT_STATUS Interrupt Status Register Bit Fields */
#define PHY_LXT971_INT_STATUS_RES1      (0xFF00)
#define PHY_LXT971_INT_STATUS_ANDONE    (0x0080)
#define PHY_LXT971_INT_STATUS_SPEEDCHG  (0x0040)
#define PHY_LXT971_INT_STATUS_DUPLEXCHG (0x0020)
#define PHY_LXT971_INT_STATUS_LINKCHG   (0x0010)
#define PHY_LXT971_INT_STATUS_RES2      (0x0008)
#define PHY_LXT971_INT_STATUS_MDINT     (0x0004)
#define PHY_LXT971_INT_STATUS_RES3      (0x0003)

/* LED_CFG Interrupt LED Configuration Register Bit Fields */
#define PHY_LXT971_LED_CFG_SHIFT_LED1   (0x000C)
#define PHY_LXT971_LED_CFG_SHIFT_LED2   (0x0008)
#define PHY_LXT971_LED_CFG_SHIFT_LED3   (0x0004)
#define PHY_LXT971_LED_CFG_LEDFREQ_MA	(0x000C)
#define PHY_LXT971_LED_CFG_LEDFREQ_RES	(0x000C)
#define PHY_LXT971_LED_CFG_LEDFREQ_100	(0x0008)
#define PHY_LXT971_LED_CFG_LEDFREQ_60	(0x0004)
#define PHY_LXT971_LED_CFG_LEDFREQ_30	(0x0000)
#define PHY_LXT971_LED_CFG_PULSE_STR    (0x0002)
#define PHY_LXT971_LED_CFG_RES1         (0x0001)

/* only one of these values must be shifted for each SHIFT_LED?  */
#define PHY_LXT971_LED_CFG_UNUSED1      (0x000F)
#define PHY_LXT971_LED_CFG_DUPLEX_COL   (0x000E)
#define PHY_LXT971_LED_CFG_LINK_ACT     (0x000D)
#define PHY_LXT971_LED_CFG_LINK_RX      (0x000C)
#define PHY_LXT971_LED_CFG_TEST_BLK_SLW (0x000B)
#define PHY_LXT971_LED_CFG_TEST_BLK_FST (0x000A)
#define PHY_LXT971_LED_CFG_TEST_OFF     (0x0009)
#define PHY_LXT971_LED_CFG_TEST_ON      (0x0008)
#define PHY_LXT971_LED_CFG_RX_OR_TX     (0x0007)
#define PHY_LXT971_LED_CFG_UNUSED2      (0x0006)
#define PHY_LXT971_LED_CFG_DUPLEX       (0x0005)
#define PHY_LXT971_LED_CFG_LINK	        (0x0004)
#define PHY_LXT971_LED_CFG_COLLISION    (0x0003)
#define PHY_LXT971_LED_CFG_RECEIVE      (0x0002)
#define PHY_LXT971_LED_CFG_TRANSMIT     (0x0001)
#define PHY_LXT971_LED_CFG_SPEED        (0x0000)

/* DIG_CFG Digitial Configuration Register Bit Fields */
#define PHY_LXT971_DIG_CFG_RES1		(0xF000)
#define PHY_LXT971_DIG_CFG_MII_DRIVE	(0x0800)
#define PHY_LXT971_DIG_CFG_RES2		(0x0400)
#define PHY_LXT971_DIG_CFG_SHOW_SYMBOL	(0x0200)
#define PHY_LXT971_DIG_CFG_RES3		(0x01FF)

#define PHY_LXT971_MDIO_MAX_CLK		(8000000)
#define PHY_MDIO_MAX_CLK		(2500000)

/* TX_CTRL Transmit Control Register Bit Fields
   documentation is buggy for this register, therefore setting not included */

typedef enum
{
	PHY_NONE    = 0x0000, /* no PHY detected yet */
	PHY_LXT971A = 0x0013
} PhyType;

#endif /* __LXT971A_H__ */
