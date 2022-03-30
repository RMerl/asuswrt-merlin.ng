/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef _EMAC_H_
#define _EMAC_H_

#define EMAC_IEVENT_REG		0x004
#define EMAC_IMASK_REG		0x008
#define EMAC_R_DES_ACTIVE_REG	0x010
#define EMAC_X_DES_ACTIVE_REG	0x014
#define EMAC_ECNTRL_REG		0x024
#define EMAC_MII_DATA_REG	0x040
#define EMAC_MII_CTRL_REG	0x044
#define EMAC_MIB_CTRL_STS_REG	0x064
#define EMAC_RCNTRL_REG		0x084
#define EMAC_TCNTRL_REG		0x0C4
#define EMAC_PHY_ADDR_LOW	0x0E4
#define EMAC_PHY_ADDR_HIGH	0x0E8
#define EMAC_TFWR_STR_FWD	0x144
#define EMAC_RX_SECTIOM_FULL	0x190
#define EMAC_TX_SECTION_EMPTY	0x1A0
#define EMAC_TRUNC_FL		0x1B0

/* GEMAC definitions and settings */
#define EMAC_PORT_0			0
#define EMAC_PORT_1			1

/* GEMAC Bit definitions */
#define EMAC_IEVENT_HBERR                BIT(31)
#define EMAC_IEVENT_BABR                 BIT(30)
#define EMAC_IEVENT_BABT                 BIT(29)
#define EMAC_IEVENT_GRA                  BIT(28)
#define EMAC_IEVENT_TXF                  BIT(27)
#define EMAC_IEVENT_TXB                  BIT(26)
#define EMAC_IEVENT_RXF                  BIT(25)
#define EMAC_IEVENT_RXB                  BIT(24)
#define EMAC_IEVENT_MII                  BIT(23)
#define EMAC_IEVENT_EBERR                BIT(22)
#define EMAC_IEVENT_LC                   BIT(21)
#define EMAC_IEVENT_RL                   BIT(20)
#define EMAC_IEVENT_UN                   BIT(19)

#define EMAC_IMASK_HBERR                 BIT(31)
#define EMAC_IMASK_BABR                  BIT(30)
#define EMAC_IMASKT_BABT                 BIT(29)
#define EMAC_IMASK_GRA                   BIT(28)
#define EMAC_IMASKT_TXF                  BIT(27)
#define EMAC_IMASK_TXB                   BIT(26)
#define EMAC_IMASKT_RXF                  BIT(25)
#define EMAC_IMASK_RXB                   BIT(24)
#define EMAC_IMASK_MII                   BIT(23)
#define EMAC_IMASK_EBERR                 BIT(22)
#define EMAC_IMASK_LC                    BIT(21)
#define EMAC_IMASKT_RL                   BIT(20)
#define EMAC_IMASK_UN                    BIT(19)

#define EMAC_RCNTRL_MAX_FL_SHIFT         16
#define EMAC_RCNTRL_LOOP                 BIT(0)
#define EMAC_RCNTRL_DRT                  BIT(1)
#define EMAC_RCNTRL_MII_MODE             BIT(2)
#define EMAC_RCNTRL_PROM                 BIT(3)
#define EMAC_RCNTRL_BC_REJ               BIT(4)
#define EMAC_RCNTRL_FCE                  BIT(5)
#define EMAC_RCNTRL_RGMII                BIT(6)
#define EMAC_RCNTRL_SGMII                BIT(7)
#define EMAC_RCNTRL_RMII                 BIT(8)
#define EMAC_RCNTRL_RMII_10T             BIT(9)
#define EMAC_RCNTRL_CRC_FWD		 BIT(10)

#define EMAC_TCNTRL_GTS                  BIT(0)
#define EMAC_TCNTRL_HBC                  BIT(1)
#define EMAC_TCNTRL_FDEN                 BIT(2)
#define EMAC_TCNTRL_TFC_PAUSE            BIT(3)
#define EMAC_TCNTRL_RFC_PAUSE            BIT(4)

#define EMAC_ECNTRL_RESET                BIT(0)      /* reset the EMAC */
#define EMAC_ECNTRL_ETHER_EN             BIT(1)      /* enable the EMAC */
#define EMAC_ECNTRL_SPEED                BIT(5)
#define EMAC_ECNTRL_DBSWAP               BIT(8)

#define EMAC_X_WMRK_STRFWD               BIT(8)

#define EMAC_X_DES_ACTIVE_TDAR           BIT(24)
#define EMAC_R_DES_ACTIVE_RDAR           BIT(24)

#define EMAC_TFWR			(0x4)
#define EMAC_RX_SECTION_FULL_32		(0x5)
#define EMAC_TRUNC_FL_16K		(0x3FFF)
#define EMAC_TX_SECTION_EMPTY_30	(0x30)
#define EMAC_MIBC_NO_CLR_NO_DIS		(0x0)

/*
 * The possible operating speeds of the MAC, currently supporting 10, 100 and
 * 1000Mb modes.
 */
enum mac_speed {PFE_MAC_SPEED_10M, PFE_MAC_SPEED_100M, PFE_MAC_SPEED_1000M,
		PFE_MAC_SPEED_1000M_PCS};

/* MII-related definitios */
#define EMAC_MII_DATA_ST         0x40000000      /* Start of frame delimiter */
#define EMAC_MII_DATA_OP_RD      0x20000000      /* Perform a read operation */
#define EMAC_MII_DATA_OP_CL45_RD 0x30000000      /* Perform a read operation */
#define EMAC_MII_DATA_OP_WR      0x10000000      /* Perform a write operation */
#define EMAC_MII_DATA_OP_CL45_WR 0x10000000      /* Perform a write operation */
#define EMAC_MII_DATA_PA_MSK     0x0f800000      /* PHY Address field mask */
#define EMAC_MII_DATA_RA_MSK     0x007c0000      /* PHY Register field mask */
#define EMAC_MII_DATA_TA         0x00020000      /* Turnaround */
#define EMAC_MII_DATA_DATAMSK    0x0000ffff      /* PHY data field */

#define EMAC_MII_DATA_RA_SHIFT   18      /* MII Register address bits */
#define EMAC_MII_DATA_RA_MASK	 0x1F      /* MII Register address mask */
#define EMAC_MII_DATA_PA_SHIFT   23      /* MII PHY address bits */
#define EMAC_MII_DATA_PA_MASK    0x1F      /* MII PHY address mask */

#define EMAC_MII_DATA_RA(v) ((v & EMAC_MII_DATA_RA_MASK) <<\
				EMAC_MII_DATA_RA_SHIFT)
#define EMAC_MII_DATA_PA(v) ((v & EMAC_MII_DATA_RA_MASK) <<\
				EMAC_MII_DATA_PA_SHIFT)
#define EMAC_MII_DATA(v)    (v & 0xffff)

#define EMAC_MII_SPEED_SHIFT	1
#define EMAC_HOLDTIME_SHIFT	8
#define EMAC_HOLDTIME_MASK	0x7
#define EMAC_HOLDTIME(v)    ((v & EMAC_HOLDTIME_MASK) << EMAC_HOLDTIME_SHIFT)

/* Internal PHY Registers - SGMII */
#define PHY_SGMII_CR_PHY_RESET      0x8000
#define PHY_SGMII_CR_RESET_AN       0x0200
#define PHY_SGMII_CR_DEF_VAL        0x1140
#define PHY_SGMII_DEV_ABILITY_SGMII 0x4001
#define PHY_SGMII_IF_MODE_AN        0x0002
#define PHY_SGMII_IF_MODE_SGMII     0x0001
#define PHY_SGMII_IF_MODE_SGMII_GBT 0x0008
#define PHY_SGMII_ENABLE_AN         0x1000

#endif /* _EMAC_H_ */
