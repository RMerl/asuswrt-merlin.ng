// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx GMII2RGMII phy driver
 *
 * Copyright (C) 2018 Xilinx, Inc.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <phy.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define ZYNQ_GMII2RGMII_REG		0x10
#define ZYNQ_GMII2RGMII_SPEED_MASK	(BMCR_SPEED1000 | BMCR_SPEED100)

static int xilinxgmiitorgmii_config(struct phy_device *phydev)
{
	ofnode node = phy_get_ofnode(phydev);
	struct phy_device *ext_phydev;
	struct ofnode_phandle_args phandle;
	int ext_phyaddr = -1;
	int ret;

	debug("%s\n", __func__);

	if (phydev->interface != PHY_INTERFACE_MODE_GMII) {
		printf("Incorrect interface type\n");
		return -EINVAL;
	}

	if (!ofnode_valid(node))
		return -EINVAL;

	phydev->addr = ofnode_read_u32_default(node, "reg", -1);
	ret = ofnode_parse_phandle_with_args(node, "phy-handle",
					     NULL, 0, 0, &phandle);
	if (ret)
		return ret;

	ext_phyaddr = ofnode_read_u32_default(phandle.node, "reg", -1);
	ext_phydev = phy_find_by_mask(phydev->bus,
				      1 << ext_phyaddr);
	if (!ext_phydev) {
		printf("%s, No external phy device found\n", __func__);
		return -EINVAL;
	}

	ext_phydev->interface = ofnode_read_phy_mode(node);
	if (ext_phydev->interface == PHY_INTERFACE_MODE_NA) {
		ext_phydev->interface = PHY_INTERFACE_MODE_RGMII;
	} else if (!phy_interface_is_rgmii(ext_phydev)) {
		printf("Incorrect external interface type\n");
		return -EINVAL;
	}

	ext_phydev->node = phandle.node;
	phydev->priv = ext_phydev;

	debug("%s, gmii2rgmmi:0x%x, extphy:0x%x\n", __func__, phydev->addr,
	      ext_phyaddr);

	if (ext_phydev->drv->config)
		ext_phydev->drv->config(ext_phydev);

	return 0;
}

static int xilinxgmiitorgmii_extread(struct phy_device *phydev, int addr,
				     int devaddr, int regnum)
{
	struct phy_device *ext_phydev = phydev->priv;

	debug("%s\n", __func__);
	if (ext_phydev->drv->readext)
		ext_phydev->drv->readext(ext_phydev, addr, devaddr, regnum);

	return 0;
}

static int xilinxgmiitorgmii_extwrite(struct phy_device *phydev, int addr,
				      int devaddr, int regnum, u16 val)

{
	struct phy_device *ext_phydev = phydev->priv;

	debug("%s\n", __func__);
	if (ext_phydev->drv->writeext)
		ext_phydev->drv->writeext(ext_phydev, addr, devaddr, regnum,
					  val);

	return 0;
}

static int xilinxgmiitorgmii_startup(struct phy_device *phydev)
{
	u16 val = 0;
	struct phy_device *ext_phydev = phydev->priv;

	debug("%s\n", __func__);
	ext_phydev->dev = phydev->dev;
	if (ext_phydev->drv->startup)
		ext_phydev->drv->startup(ext_phydev);

	val = phy_read(phydev, phydev->addr, ZYNQ_GMII2RGMII_REG);
	val &= ~ZYNQ_GMII2RGMII_SPEED_MASK;

	if (ext_phydev->speed == SPEED_1000)
		val |= BMCR_SPEED1000;
	else if (ext_phydev->speed == SPEED_100)
		val |= BMCR_SPEED100;

	phy_write(phydev, phydev->addr, ZYNQ_GMII2RGMII_REG, val |
		  BMCR_FULLDPLX);

	phydev->duplex = ext_phydev->duplex;
	phydev->speed = ext_phydev->speed;
	phydev->link = ext_phydev->link;

	return 0;
}

static int xilinxgmiitorgmii_probe(struct phy_device *phydev)
{
	debug("%s\n", __func__);

	phydev->flags |= PHY_FLAG_BROKEN_RESET;

	return 0;
}

U_BOOT_PHY_DRIVER(gmii2rgmii) = {
	.name = "XILINX GMII2RGMII",
	.uid = PHY_GMII2RGMII_ID,
	.mask = 0xffffffff,
	.features = PHY_GBIT_FEATURES,
	.probe = xilinxgmiitorgmii_probe,
	.config = xilinxgmiitorgmii_config,
	.startup = xilinxgmiitorgmii_startup,
	.writeext = xilinxgmiitorgmii_extwrite,
	.readext = xilinxgmiitorgmii_extread,
};
