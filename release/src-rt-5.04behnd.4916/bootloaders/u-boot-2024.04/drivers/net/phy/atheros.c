// SPDX-License-Identifier: GPL-2.0+
/*
 * Atheros PHY drivers
 *
 * Copyright 2011, 2013 Freescale Semiconductor, Inc.
 * author Andy Fleming
 * Copyright (c) 2019 Michael Walle <michael@walle.cc>
 */
#include <common.h>
#include <phy.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <dt-bindings/net/qca-ar803x.h>

#define AR803x_PHY_DEBUG_ADDR_REG	0x1d
#define AR803x_PHY_DEBUG_DATA_REG	0x1e

/* Debug registers */
#define AR803x_DEBUG_REG_0		0x0
#define AR803x_RGMII_RX_CLK_DLY		BIT(15)

#define AR803x_DEBUG_REG_5		0x5
#define AR803x_RGMII_TX_CLK_DLY		BIT(8)

#define AR803x_DEBUG_REG_1F		0x1f
#define AR803x_PLL_ON			BIT(2)
#define AR803x_RGMII_1V8		BIT(3)

/* CLK_25M register is at MMD 7, address 0x8016 */
#define AR803x_CLK_25M_SEL_REG		0x8016

#define AR803x_CLK_25M_MASK		GENMASK(4, 2)
#define AR803x_CLK_25M_25MHZ_XTAL	0
#define AR803x_CLK_25M_25MHZ_DSP	1
#define AR803x_CLK_25M_50MHZ_PLL	2
#define AR803x_CLK_25M_50MHZ_DSP	3
#define AR803x_CLK_25M_62_5MHZ_PLL	4
#define AR803x_CLK_25M_62_5MHZ_DSP	5
#define AR803x_CLK_25M_125MHZ_PLL	6
#define AR803x_CLK_25M_125MHZ_DSP	7
#define AR8035_CLK_25M_MASK		GENMASK(4, 3)

#define AR803x_CLK_25M_DR_MASK		GENMASK(8, 7)
#define AR803x_CLK_25M_DR_FULL		0
#define AR803x_CLK_25M_DR_HALF		1
#define AR803x_CLK_25M_DR_QUARTER	2

#define AR8021_PHY_ID 0x004dd040
#define AR8031_PHY_ID 0x004dd074
#define AR8035_PHY_ID 0x004dd072

struct ar803x_priv {
	int flags;
#define AR803x_FLAG_KEEP_PLL_ENABLED	BIT(0) /* don't turn off internal PLL */
#define AR803x_FLAG_RGMII_1V8		BIT(1) /* use 1.8V RGMII I/O voltage */
	u16 clk_25m_reg;
	u16 clk_25m_mask;
};

static int ar803x_debug_reg_read(struct phy_device *phydev, u16 reg)
{
	int ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AR803x_PHY_DEBUG_ADDR_REG,
			reg);
	if (ret < 0)
		return ret;

	return phy_read(phydev, MDIO_DEVAD_NONE, AR803x_PHY_DEBUG_DATA_REG);
}

static int ar803x_debug_reg_mask(struct phy_device *phydev, u16 reg,
				 u16 clear, u16 set)
{
	int val;

	val = ar803x_debug_reg_read(phydev, reg);
	if (val < 0)
		return val;

	val &= 0xffff;
	val &= ~clear;
	val |= set;

	return phy_write(phydev, MDIO_DEVAD_NONE, AR803x_PHY_DEBUG_DATA_REG,
			 val);
}

static int ar803x_enable_rx_delay(struct phy_device *phydev, bool on)
{
	u16 clear = 0, set = 0;

	if (on)
		set = AR803x_RGMII_RX_CLK_DLY;
	else
		clear = AR803x_RGMII_RX_CLK_DLY;

	return ar803x_debug_reg_mask(phydev, AR803x_DEBUG_REG_0, clear, set);
}

static int ar803x_enable_tx_delay(struct phy_device *phydev, bool on)
{
	u16 clear = 0, set = 0;

	if (on)
		set = AR803x_RGMII_TX_CLK_DLY;
	else
		clear = AR803x_RGMII_TX_CLK_DLY;

	return ar803x_debug_reg_mask(phydev, AR803x_DEBUG_REG_5, clear, set);
}

static int ar8021_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR,
		  BMCR_ANENABLE | BMCR_ANRESTART);

	ar803x_enable_tx_delay(phydev, true);

	phydev->supported = phydev->drv->features;
	return 0;
}

static int ar803x_delay_config(struct phy_device *phydev)
{
	int ret;

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
		ret = ar803x_enable_tx_delay(phydev, true);
	else
		ret = ar803x_enable_tx_delay(phydev, false);

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
		ret = ar803x_enable_rx_delay(phydev, true);
	else
		ret = ar803x_enable_rx_delay(phydev, false);

	return ret;
}

static int ar803x_regs_config(struct phy_device *phydev)
{
	struct ar803x_priv *priv = phydev->priv;
	u16 set = 0, clear = 0;
	int val;
	int ret;

	/* no configuration available */
	if (!priv)
		return 0;

	/*
	 * Only supported on the AR8031, AR8035 has strappings for the PLL mode
	 * as well as the RGMII voltage.
	 */
	if (phydev->drv->uid == AR8031_PHY_ID) {
		if (priv->flags & AR803x_FLAG_KEEP_PLL_ENABLED)
			set |= AR803x_PLL_ON;
		else
			clear |= AR803x_PLL_ON;

		if (priv->flags & AR803x_FLAG_RGMII_1V8)
			set |= AR803x_RGMII_1V8;
		else
			clear |= AR803x_RGMII_1V8;

		ret = ar803x_debug_reg_mask(phydev, AR803x_DEBUG_REG_1F, clear,
					    set);
		if (ret < 0)
			return ret;
	}

	/* save the write access if the mask is empty */
	if (priv->clk_25m_mask) {
		val = phy_read_mmd(phydev, MDIO_MMD_AN, AR803x_CLK_25M_SEL_REG);
		if (val < 0)
			return val;
		val &= ~priv->clk_25m_mask;
		val |= priv->clk_25m_reg;
		ret = phy_write_mmd(phydev, MDIO_MMD_AN,
				    AR803x_CLK_25M_SEL_REG, val);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int ar803x_of_init(struct phy_device *phydev)
{
	struct ar803x_priv *priv;
	ofnode node, vddio_reg_node;
	u32 strength, freq, min_uV, max_uV;
	int sel;

	node = phy_get_ofnode(phydev);
	if (!ofnode_valid(node))
		return 0;

	priv = malloc(sizeof(*priv));
	if (!priv)
		return -ENOMEM;
	memset(priv, 0, sizeof(*priv));

	phydev->priv = priv;

	debug("%s: found PHY node: %s\n", __func__, ofnode_get_name(node));

	if (ofnode_read_bool(node, "qca,keep-pll-enabled"))
		priv->flags |= AR803x_FLAG_KEEP_PLL_ENABLED;

	/*
	 * We can't use the regulator framework because the regulator is
	 * a subnode of the PHY. So just read the two properties we are
	 * interested in.
	 */
	vddio_reg_node = ofnode_find_subnode(node, "vddio-regulator");
	if (ofnode_valid(vddio_reg_node)) {
		min_uV = ofnode_read_u32_default(vddio_reg_node,
						 "regulator-min-microvolt", 0);
		max_uV = ofnode_read_u32_default(vddio_reg_node,
						 "regulator-max-microvolt", 0);

		if (min_uV != max_uV) {
			free(priv);
			return -EINVAL;
		}

		switch (min_uV) {
		case 1500000:
			break;
		case 1800000:
			priv->flags |= AR803x_FLAG_RGMII_1V8;
			break;
		default:
			free(priv);
			return -EINVAL;
		}
	}

	/*
	 * Get the CLK_25M frequency from the device tree. Only XTAL and PLL
	 * sources are supported right now. There is also the possibilty to use
	 * the DSP as frequency reference, this is used for synchronous
	 * ethernet.
	 */
	if (!ofnode_read_u32(node, "qca,clk-out-frequency", &freq)) {
		switch (freq) {
		case 25000000:
			sel = AR803x_CLK_25M_25MHZ_XTAL;
			break;
		case 50000000:
			sel = AR803x_CLK_25M_50MHZ_PLL;
			break;
		case 62500000:
			sel = AR803x_CLK_25M_62_5MHZ_PLL;
			break;
		case 125000000:
			sel = AR803x_CLK_25M_125MHZ_PLL;
			break;
		default:
			dev_err(phydev->dev,
				"invalid qca,clk-out-frequency\n");
			free(priv);
			return -EINVAL;
		}

		priv->clk_25m_mask |= AR803x_CLK_25M_MASK;
		priv->clk_25m_reg |= FIELD_PREP(AR803x_CLK_25M_MASK, sel);
		/*
		 * Fixup for the AR8035 which only has two bits. The two
		 * remaining bits map to the same frequencies.
		 */

		if (phydev->drv->uid == AR8035_PHY_ID) {
			priv->clk_25m_reg &= AR8035_CLK_25M_MASK;
			priv->clk_25m_mask &= AR8035_CLK_25M_MASK;
		}
	}

	if (phydev->drv->uid == AR8031_PHY_ID &&
	    !ofnode_read_u32(node, "qca,clk-out-strength", &strength)) {
		switch (strength) {
		case AR803X_STRENGTH_FULL:
			sel = AR803x_CLK_25M_DR_FULL;
			break;
		case AR803X_STRENGTH_HALF:
			sel = AR803x_CLK_25M_DR_HALF;
			break;
		case AR803X_STRENGTH_QUARTER:
			sel = AR803x_CLK_25M_DR_QUARTER;
			break;
		default:
			dev_err(phydev->dev,
				"invalid qca,clk-out-strength\n");
			free(priv);
			return -EINVAL;
		}
		priv->clk_25m_mask |= AR803x_CLK_25M_DR_MASK;
		priv->clk_25m_reg |= FIELD_PREP(AR803x_CLK_25M_DR_MASK, sel);
	}

	debug("%s: flags=%x clk_25m_reg=%04x clk_25m_mask=%04x\n", __func__,
	      priv->flags, priv->clk_25m_reg, priv->clk_25m_mask);

	return 0;
}

static int ar803x_config(struct phy_device *phydev)
{
	int ret;

	ret = ar803x_of_init(phydev);
	if (ret < 0)
		return ret;

	ret = ar803x_delay_config(phydev);
	if (ret < 0)
		return ret;

	ret = ar803x_regs_config(phydev);
	if (ret < 0)
		return ret;

	phydev->supported = phydev->drv->features;

	genphy_config_aneg(phydev);
	genphy_restart_aneg(phydev);

	return 0;
}

U_BOOT_PHY_DRIVER(AR8021) = {
	.name = "AR8021",
	.uid = AR8021_PHY_ID,
	.mask = 0xfffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = ar8021_config,
	.startup = genphy_startup,
	.shutdown = genphy_shutdown,
};

U_BOOT_PHY_DRIVER(AR8031) = {
	.name = "AR8031/AR8033",
	.uid = AR8031_PHY_ID,
	.mask = 0xffffffef,
	.features = PHY_GBIT_FEATURES,
	.config = ar803x_config,
	.startup = genphy_startup,
	.shutdown = genphy_shutdown,
};

U_BOOT_PHY_DRIVER(AR8035) = {
	.name = "AR8035",
	.uid = AR8035_PHY_ID,
	.mask = 0xffffffef,
	.features = PHY_GBIT_FEATURES,
	.config = ar803x_config,
	.startup = genphy_startup,
	.shutdown = genphy_shutdown,
};
