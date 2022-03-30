// SPDX-License-Identifier: GPL-2.0+
/*
 * Xilinx PCS/PMA Core phy driver
 *
 * Copyright (C) 2015 - 2016 Xilinx, Inc.
 */

#include <config.h>
#include <common.h>
#include <phy.h>
#include <dm.h>

#define MII_PHY_STATUS_SPD_MASK		0x0C00
#define MII_PHY_STATUS_FULLDUPLEX	0x1000
#define MII_PHY_STATUS_1000		0x0800
#define MII_PHY_STATUS_100		0x0400
#define XPCSPMA_PHY_CTRL_ISOLATE_DISABLE 0xFBFF

/* Mask used for ID comparisons */
#define XILINX_PHY_ID_MASK		0xfffffff0

/* Known PHY IDs */
#define XILINX_PHY_ID			0x01740c00

/* struct phy_device dev_flags definitions */
#define XAE_PHY_TYPE_MII		0
#define XAE_PHY_TYPE_GMII		1
#define XAE_PHY_TYPE_RGMII_1_3		2
#define XAE_PHY_TYPE_RGMII_2_0		3
#define XAE_PHY_TYPE_SGMII		4
#define XAE_PHY_TYPE_1000BASE_X		5

static int xilinxphy_startup(struct phy_device *phydev)
{
	int err;
	int status = 0;

	debug("%s\n", __func__);
	/* Update the link, but return if there
	 * was an error
	 */
	err = genphy_update_link(phydev);
	if (err)
		return err;

	if (AUTONEG_ENABLE == phydev->autoneg) {
		status = phy_read(phydev, MDIO_DEVAD_NONE, MII_LPA);
		status = status & MII_PHY_STATUS_SPD_MASK;

		if (status & MII_PHY_STATUS_FULLDUPLEX)
			phydev->duplex = DUPLEX_FULL;
		else
			phydev->duplex = DUPLEX_HALF;

		switch (status) {
		case MII_PHY_STATUS_1000:
			phydev->speed = SPEED_1000;
			break;

		case MII_PHY_STATUS_100:
			phydev->speed = SPEED_100;
			break;

		default:
			phydev->speed = SPEED_10;
			break;
		}
	} else {
		int bmcr = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);

		if (bmcr < 0)
			return bmcr;

		if (bmcr & BMCR_FULLDPLX)
			phydev->duplex = DUPLEX_FULL;
		else
			phydev->duplex = DUPLEX_HALF;

		if (bmcr & BMCR_SPEED1000)
			phydev->speed = SPEED_1000;
		else if (bmcr & BMCR_SPEED100)
			phydev->speed = SPEED_100;
		else
			phydev->speed = SPEED_10;
	}

	/*
	 * For 1000BASE-X Phy Mode the speed/duplex will always be
	 * 1000Mbps/fullduplex
	 */
	if (phydev->flags == XAE_PHY_TYPE_1000BASE_X) {
		phydev->duplex = DUPLEX_FULL;
		phydev->speed = SPEED_1000;
	}

	return 0;
}

static int xilinxphy_of_init(struct phy_device *phydev)
{
	u32 phytype;
	ofnode node;

	debug("%s\n", __func__);
	node = phy_get_ofnode(phydev);
	if (!ofnode_valid(node))
		return -EINVAL;

	phytype = ofnode_read_u32_default(node, "xlnx,phy-type", -1);
	if (phytype == XAE_PHY_TYPE_1000BASE_X)
		phydev->flags |= XAE_PHY_TYPE_1000BASE_X;

	return 0;
}

static int xilinxphy_config(struct phy_device *phydev)
{
	int temp;

	debug("%s\n", __func__);
	xilinxphy_of_init(phydev);
	temp = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	temp &= XPCSPMA_PHY_CTRL_ISOLATE_DISABLE;
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, temp);

	return 0;
}

static struct phy_driver xilinxphy_driver = {
	.uid = XILINX_PHY_ID,
	.mask = XILINX_PHY_ID_MASK,
	.name = "Xilinx PCS/PMA PHY",
	.features = PHY_GBIT_FEATURES,
	.config = &xilinxphy_config,
	.startup = &xilinxphy_startup,
	.shutdown = &genphy_shutdown,
};

int phy_xilinx_init(void)
{
	debug("%s\n", __func__);
	phy_register(&xilinxphy_driver);

	return 0;
}
