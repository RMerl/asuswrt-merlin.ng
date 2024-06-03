/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * DP83848 ethernet Physical layer
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 */


/* National Semiconductor PHYSICAL LAYER TRANSCEIVER DP83848 */

#define DP83848_CTL_REG		0x0	/* Basic Mode Control Reg */
#define DP83848_STAT_REG		0x1	/* Basic Mode Status Reg */
#define DP83848_PHYID1_REG		0x2	/* PHY Idendifier Reg 1 */
#define DP83848_PHYID2_REG		0x3	/* PHY Idendifier Reg 2 */
#define DP83848_ANA_REG			0x4	/* Auto_Neg Advt Reg  */
#define DP83848_ANLPA_REG		0x5	/* Auto_neg Link Partner Ability Reg */
#define DP83848_ANE_REG			0x6	/* Auto-neg Expansion Reg  */
#define DP83848_PHY_STAT_REG		0x10	/* PHY Status Register  */
#define DP83848_PHY_INTR_CTRL_REG	0x11	/* PHY Interrupt Control Register */
#define DP83848_PHY_CTRL_REG		0x19	/* PHY Status Register  */

/*--Bit definitions: DP83848_CTL_REG */
#define DP83848_RESET		(1 << 15)  /* 1= S/W Reset */
#define DP83848_LOOPBACK	(1 << 14)  /* 1=loopback Enabled */
#define DP83848_SPEED_SELECT	(1 << 13)
#define DP83848_AUTONEG		(1 << 12)
#define DP83848_POWER_DOWN	(1 << 11)
#define DP83848_ISOLATE		(1 << 10)
#define DP83848_RESTART_AUTONEG	(1 << 9)
#define DP83848_DUPLEX_MODE	(1 << 8)
#define DP83848_COLLISION_TEST	(1 << 7)

/*--Bit definitions: DP83848_STAT_REG */
#define DP83848_100BASE_T4	(1 << 15)
#define DP83848_100BASE_TX_FD	(1 << 14)
#define DP83848_100BASE_TX_HD	(1 << 13)
#define DP83848_10BASE_T_FD	(1 << 12)
#define DP83848_10BASE_T_HD	(1 << 11)
#define DP83848_MF_PREAMB_SUPPR	(1 << 6)
#define DP83848_AUTONEG_COMP	(1 << 5)
#define DP83848_RMT_FAULT	(1 << 4)
#define DP83848_AUTONEG_ABILITY	(1 << 3)
#define DP83848_LINK_STATUS	(1 << 2)
#define DP83848_JABBER_DETECT	(1 << 1)
#define DP83848_EXTEND_CAPAB	(1 << 0)

/*--definitions: DP83848_PHYID1 */
#define DP83848_PHYID1_OUI	0x2000
#define DP83848_PHYID2_OUI	0x5c90

/*--Bit definitions: DP83848_ANAR, DP83848_ANLPAR */
#define DP83848_NP		(1 << 15)
#define DP83848_ACK		(1 << 14)
#define DP83848_RF		(1 << 13)
#define DP83848_PAUSE		(1 << 10)
#define DP83848_T4		(1 << 9)
#define DP83848_TX_FDX		(1 << 8)
#define DP83848_TX_HDX		(1 << 7)
#define DP83848_10_FDX		(1 << 6)
#define DP83848_10_HDX		(1 << 5)
#define DP83848_AN_IEEE_802_3	0x0001

/*--Bit definitions: DP83848_ANER */
#define DP83848_PDF		(1 << 4)
#define DP83848_LP_NP_ABLE	(1 << 3)
#define DP83848_NP_ABLE		(1 << 2)
#define DP83848_PAGE_RX		(1 << 1)
#define DP83848_LP_AN_ABLE	(1 << 0)

/*--Bit definitions: DP83848_PHY_STAT */
#define DP83848_RX_ERR_LATCH		(1 << 13)
#define DP83848_POLARITY_STAT		(1 << 12)
#define DP83848_FALSE_CAR_SENSE		(1 << 11)
#define DP83848_SIG_DETECT		(1 << 10)
#define DP83848_DESCRAM_LOCK		(1 << 9)
#define DP83848_PAGE_RCV		(1 << 8)
#define DP83848_PHY_RMT_FAULT		(1 << 6)
#define DP83848_JABBER			(1 << 5)
#define DP83848_AUTONEG_COMPLETE	(1 << 4)
#define DP83848_LOOPBACK_STAT		(1 << 3)
#define DP83848_DUPLEX			(1 << 2)
#define DP83848_SPEED			(1 << 1)
#define DP83848_LINK			(1 << 0)
