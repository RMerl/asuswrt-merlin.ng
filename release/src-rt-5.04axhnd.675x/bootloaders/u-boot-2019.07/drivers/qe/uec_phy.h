/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2005, 2011 Freescale Semiconductor, Inc.
 *
 * Author: Shlomi Gridish <gridish@freescale.com>
 *
 * Description: UCC ethernet driver -- PHY handling
 *		Driver for UEC on QE
 *		Based on 8260_io/fcc_enet.c
 */
#ifndef __UEC_PHY_H__
#define __UEC_PHY_H__

#define MII_end ((u32)-2)
#define MII_read ((u32)-1)

#define MIIMIND_BUSY		0x00000001
#define MIIMIND_NOTVALID	0x00000004

#define UGETH_AN_TIMEOUT	2000

/* Cicada Extended Control Register 1 */
#define MII_CIS8201_EXT_CON1	    0x17
#define MII_CIS8201_EXTCON1_INIT    0x0000

/* Cicada Interrupt Mask Register */
#define MII_CIS8201_IMASK	    0x19
#define MII_CIS8201_IMASK_IEN	    0x8000
#define MII_CIS8201_IMASK_SPEED	    0x4000
#define MII_CIS8201_IMASK_LINK	    0x2000
#define MII_CIS8201_IMASK_DUPLEX    0x1000
#define MII_CIS8201_IMASK_MASK	    0xf000

/* Cicada Interrupt Status Register */
#define MII_CIS8201_ISTAT	    0x1a
#define MII_CIS8201_ISTAT_STATUS    0x8000
#define MII_CIS8201_ISTAT_SPEED	    0x4000
#define MII_CIS8201_ISTAT_LINK	    0x2000
#define MII_CIS8201_ISTAT_DUPLEX    0x1000

/* Cicada Auxiliary Control/Status Register */
#define MII_CIS8201_AUX_CONSTAT	       0x1c
#define MII_CIS8201_AUXCONSTAT_INIT    0x0004
#define MII_CIS8201_AUXCONSTAT_DUPLEX  0x0020
#define MII_CIS8201_AUXCONSTAT_SPEED   0x0018
#define MII_CIS8201_AUXCONSTAT_GBIT    0x0010
#define MII_CIS8201_AUXCONSTAT_100     0x0008

/* 88E1011 PHY Status Register */
#define MII_M1011_PHY_SPEC_STATUS		0x11
#define MII_M1011_PHY_SPEC_STATUS_1000		0x8000
#define MII_M1011_PHY_SPEC_STATUS_100		0x4000
#define MII_M1011_PHY_SPEC_STATUS_SPD_MASK	0xc000
#define MII_M1011_PHY_SPEC_STATUS_FULLDUPLEX	0x2000
#define MII_M1011_PHY_SPEC_STATUS_RESOLVED	0x0800
#define MII_M1011_PHY_SPEC_STATUS_LINK		0x0400

#define MII_M1011_IEVENT		0x13
#define MII_M1011_IEVENT_CLEAR		0x0000

#define MII_M1011_IMASK			0x12
#define MII_M1011_IMASK_INIT		0x6400
#define MII_M1011_IMASK_CLEAR		0x0000

/* 88E1111 PHY Register */
#define MII_M1111_PHY_EXT_CR            0x14
#define MII_M1111_RX_DELAY              0x80
#define MII_M1111_TX_DELAY              0x2
#define MII_M1111_PHY_EXT_SR            0x1b
#define MII_M1111_HWCFG_MODE_MASK       0xf
#define MII_M1111_HWCFG_MODE_RGMII      0xb

#define MII_DM9161_SCR			0x10
#define MII_DM9161_SCR_INIT		0x0610
#define MII_DM9161_SCR_RMII_INIT	0x0710

/* DM9161 Specified Configuration and Status Register */
#define MII_DM9161_SCSR			0x11
#define MII_DM9161_SCSR_100F		0x8000
#define MII_DM9161_SCSR_100H		0x4000
#define MII_DM9161_SCSR_10F		0x2000
#define MII_DM9161_SCSR_10H		0x1000

/* DM9161 Interrupt Register */
#define MII_DM9161_INTR			0x15
#define MII_DM9161_INTR_PEND		0x8000
#define MII_DM9161_INTR_DPLX_MASK	0x0800
#define MII_DM9161_INTR_SPD_MASK	0x0400
#define MII_DM9161_INTR_LINK_MASK	0x0200
#define MII_DM9161_INTR_MASK		0x0100
#define MII_DM9161_INTR_DPLX_CHANGE	0x0010
#define MII_DM9161_INTR_SPD_CHANGE	0x0008
#define MII_DM9161_INTR_LINK_CHANGE	0x0004
#define MII_DM9161_INTR_INIT		0x0000
#define MII_DM9161_INTR_STOP	\
(MII_DM9161_INTR_DPLX_MASK | MII_DM9161_INTR_SPD_MASK \
 | MII_DM9161_INTR_LINK_MASK | MII_DM9161_INTR_MASK)

/* DM9161 10BT Configuration/Status */
#define MII_DM9161_10BTCSR		0x12
#define MII_DM9161_10BTCSR_INIT		0x7800

#define MII_BASIC_FEATURES    (SUPPORTED_10baseT_Half | \
		 SUPPORTED_10baseT_Full | \
		 SUPPORTED_100baseT_Half | \
		 SUPPORTED_100baseT_Full | \
		 SUPPORTED_Autoneg | \
		 SUPPORTED_TP | \
		 SUPPORTED_MII)

#define MII_GBIT_FEATURES    (MII_BASIC_FEATURES | \
		 SUPPORTED_1000baseT_Half | \
		 SUPPORTED_1000baseT_Full)

#define MII_READ_COMMAND		0x00000001

#define MII_INTERRUPT_DISABLED		0x0
#define MII_INTERRUPT_ENABLED		0x1

#define SPEED_10    10
#define SPEED_100   100
#define SPEED_1000  1000

/* Duplex, half or full. */
#define DUPLEX_HALF		0x00
#define DUPLEX_FULL		0x01

/* Indicates what features are supported by the interface. */
#define SUPPORTED_10baseT_Half		(1 << 0)
#define SUPPORTED_10baseT_Full		(1 << 1)
#define SUPPORTED_100baseT_Half		(1 << 2)
#define SUPPORTED_100baseT_Full		(1 << 3)
#define SUPPORTED_1000baseT_Half	(1 << 4)
#define SUPPORTED_1000baseT_Full	(1 << 5)
#define SUPPORTED_Autoneg		(1 << 6)
#define SUPPORTED_TP			(1 << 7)
#define SUPPORTED_AUI			(1 << 8)
#define SUPPORTED_MII			(1 << 9)
#define SUPPORTED_FIBRE			(1 << 10)
#define SUPPORTED_BNC			(1 << 11)
#define SUPPORTED_10000baseT_Full	(1 << 12)

#define ADVERTISED_10baseT_Half		(1 << 0)
#define ADVERTISED_10baseT_Full		(1 << 1)
#define ADVERTISED_100baseT_Half	(1 << 2)
#define ADVERTISED_100baseT_Full	(1 << 3)
#define ADVERTISED_1000baseT_Half	(1 << 4)
#define ADVERTISED_1000baseT_Full	(1 << 5)
#define ADVERTISED_Autoneg		(1 << 6)
#define ADVERTISED_TP			(1 << 7)
#define ADVERTISED_AUI			(1 << 8)
#define ADVERTISED_MII			(1 << 9)
#define ADVERTISED_FIBRE		(1 << 10)
#define ADVERTISED_BNC			(1 << 11)
#define ADVERTISED_10000baseT_Full	(1 << 12)

/* Taken from mii_if_info and sungem_phy.h */
struct uec_mii_info {
	/* Information about the PHY type */
	/* And management functions */
	struct phy_info *phyinfo;

	struct eth_device *dev;

	/* forced speed & duplex (no autoneg)
	 * partner speed & duplex & pause (autoneg)
	 */
	int speed;
	int duplex;
	int pause;

	/* The most recently read link state */
	int link;

	/* Enabled Interrupts */
	u32 interrupts;

	u32 advertising;
	int autoneg;
	int mii_id;

	/* private data pointer */
	/* For use by PHYs to maintain extra state */
	void *priv;

	/* Provided by ethernet driver */
	int (*mdio_read) (struct eth_device * dev, int mii_id, int reg);
	void (*mdio_write) (struct eth_device * dev, int mii_id, int reg,
			    int val);
};

/* struct phy_info: a structure which defines attributes for a PHY
 *
 * id will contain a number which represents the PHY.  During
 * startup, the driver will poll the PHY to find out what its
 * UID--as defined by registers 2 and 3--is.  The 32-bit result
 * gotten from the PHY will be ANDed with phy_id_mask to
 * discard any bits which may change based on revision numbers
 * unimportant to functionality
 *
 * There are 6 commands which take a ugeth_mii_info structure.
 * Each PHY must declare config_aneg, and read_status.
 */
struct phy_info {
	u32 phy_id;
	char *name;
	unsigned int phy_id_mask;
	u32 features;

	/* Called to initialize the PHY */
	int (*init) (struct uec_mii_info * mii_info);

	/* Called to suspend the PHY for power */
	int (*suspend) (struct uec_mii_info * mii_info);

	/* Reconfigures autonegotiation (or disables it) */
	int (*config_aneg) (struct uec_mii_info * mii_info);

	/* Determines the negotiated speed and duplex */
	int (*read_status) (struct uec_mii_info * mii_info);

	/* Clears any pending interrupts */
	int (*ack_interrupt) (struct uec_mii_info * mii_info);

	/* Enables or disables interrupts */
	int (*config_intr) (struct uec_mii_info * mii_info);

	/* Clears up any memory if needed */
	void (*close) (struct uec_mii_info * mii_info);
};

struct phy_info *uec_get_phy_info (struct uec_mii_info *mii_info);
void uec_write_phy_reg (struct eth_device *dev, int mii_id, int regnum,
		    int value);
int uec_read_phy_reg (struct eth_device *dev, int mii_id, int regnum);
void mii_clear_phy_interrupt (struct uec_mii_info *mii_info);
void mii_configure_phy_interrupt (struct uec_mii_info *mii_info,
				  u32 interrupts);
#endif /* __UEC_PHY_H__ */
