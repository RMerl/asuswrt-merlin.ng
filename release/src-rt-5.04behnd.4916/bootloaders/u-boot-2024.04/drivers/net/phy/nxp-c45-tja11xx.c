// SPDX-License-Identifier: GPL-2.0+
/*
 * NXP C45 PHY driver
 *
 * Copyright 2021 NXP
 * Author: Radu Pirea <radu-nicolae.pirea@oss.nxp.com>
 */
#include <common.h>
#include <dm.h>
#include <dm/devres.h>
#include <linux/delay.h>
#include <linux/math64.h>
#include <linux/mdio.h>
#include <phy.h>

#define PHY_ID_TJA_1103			0x001BB010
#define PHY_ID_TJA_1120			0x001BB031

#define VEND1_DEVICE_CONTROL		0x0040
#define DEVICE_CONTROL_RESET		BIT(15)
#define DEVICE_CONTROL_CONFIG_GLOBAL_EN	BIT(14)
#define DEVICE_CONTROL_CONFIG_ALL_EN	BIT(13)

#define VEND1_PORT_CONTROL		0x8040
#define PORT_CONTROL_EN			BIT(14)

#define VEND1_PHY_CONTROL		0x8100
#define PHY_CONFIG_EN			BIT(14)
#define PHY_START_OP			BIT(0)

#define VEND1_PHY_CONFIG		0x8108
#define PHY_CONFIG_AUTO			BIT(0)

#define VEND1_PORT_INFRA_CONTROL	0xAC00
#define PORT_INFRA_CONTROL_EN		BIT(14)

#define VEND1_RXID			0xAFCC
#define VEND1_TXID			0xAFCD
#define ID_ENABLE			BIT(15)

#define VEND1_ABILITIES			0xAFC4
#define RGMII_ID_ABILITY		BIT(15)
#define RGMII_ABILITY			BIT(14)
#define RMII_ABILITY			BIT(10)
#define REVMII_ABILITY			BIT(9)
#define MII_ABILITY			BIT(8)
#define SGMII_ABILITY			BIT(0)

#define VEND1_MII_BASIC_CONFIG		0xAFC6
#define MII_BASIC_CONFIG_REV		BIT(8)
#define MII_BASIC_CONFIG_SGMII		0x9
#define MII_BASIC_CONFIG_RGMII		0x7
#define MII_BASIC_CONFIG_RMII		0x5
#define MII_BASIC_CONFIG_MII		0x4

#define RGMII_PERIOD_PS			8000U
#define PS_PER_DEGREE			div_u64(RGMII_PERIOD_PS, 360)
#define MIN_ID_PS			1644U
#define MAX_ID_PS			2260U
#define DEFAULT_ID_PS			2000U

#define RESET_DELAY_MS			25
#define CONF_EN_DELAY_US		450

struct nxp_c45_phy {
	u32 tx_delay;
	u32 rx_delay;
};

static int nxp_c45_soft_reset(struct phy_device *phydev)
{
	int tries = 10, ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_DEVICE_CONTROL,
			    DEVICE_CONTROL_RESET);
	if (ret)
		return ret;

	do {
		ret = phy_read_mmd(phydev, MDIO_MMD_VEND1,
				   VEND1_DEVICE_CONTROL);
		if (!(ret & DEVICE_CONTROL_RESET))
			return 0;
		mdelay(RESET_DELAY_MS);
	} while (tries--);

	return -EIO;
}

static int nxp_c45_start_op(struct phy_device *phydev)
{
	return phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_PHY_CONTROL,
			     PHY_START_OP);
}

static int nxp_c45_config_enable(struct phy_device *phydev)
{
	phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_DEVICE_CONTROL,
		      DEVICE_CONTROL_CONFIG_GLOBAL_EN |
		      DEVICE_CONTROL_CONFIG_ALL_EN);
	udelay(CONF_EN_DELAY_US);

	phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_PORT_CONTROL,
		      PORT_CONTROL_EN);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_PHY_CONTROL,
		      PHY_CONFIG_EN);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_PORT_INFRA_CONTROL,
		      PORT_INFRA_CONTROL_EN);

	return 0;
}

static u64 nxp_c45_get_phase_shift(u64 phase_offset_raw)
{
	/* The delay in degree phase is 73.8 + phase_offset_raw * 0.9.
	 * To avoid floating point operations we'll multiply by 10
	 * and get 1 decimal point precision.
	 */
	phase_offset_raw *= 10;
	phase_offset_raw -= 738;
	return div_u64(phase_offset_raw, 9);
}

static void nxp_c45_disable_delays(struct phy_device *phydev)
{
	phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_TXID, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_RXID, 0);
}

static int nxp_c45_check_delay(struct phy_device *phydev, u32 delay)
{
	if (delay < MIN_ID_PS) {
		pr_err("%s: delay value smaller than %u\n",
		       phydev->drv->name, MIN_ID_PS);
		return -EINVAL;
	}

	if (delay > MAX_ID_PS) {
		pr_err("%s: delay value higher than %u\n",
		       phydev->drv->name, MAX_ID_PS);
		return -EINVAL;
	}

	return 0;
}

static int nxp_c45_get_delays(struct phy_device *phydev)
{
	struct nxp_c45_phy *priv = phydev->priv;
	int ret;

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID) {
		ret = dev_read_u32(phydev->dev, "tx-internal-delay-ps",
				   &priv->tx_delay);
		if (ret)
			priv->tx_delay = DEFAULT_ID_PS;

		ret = nxp_c45_check_delay(phydev, priv->tx_delay);
		if (ret) {
			pr_err("%s: tx-internal-delay-ps invalid value\n",
			       phydev->drv->name);
			return ret;
		}
	}

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID) {
		ret = dev_read_u32(phydev->dev, "rx-internal-delay-ps",
				   &priv->rx_delay);
		if (ret)
			priv->rx_delay = DEFAULT_ID_PS;

		ret = nxp_c45_check_delay(phydev, priv->rx_delay);
		if (ret) {
			pr_err("%s: rx-internal-delay-ps invalid value\n",
			       phydev->drv->name);
			return ret;
		}
	}

	return 0;
}

static void nxp_c45_set_delays(struct phy_device *phydev)
{
	struct nxp_c45_phy *priv = phydev->priv;
	u64 tx_delay = priv->tx_delay;
	u64 rx_delay = priv->rx_delay;
	u64 degree;

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID) {
		degree = div_u64(tx_delay, PS_PER_DEGREE);
		phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_TXID,
			      ID_ENABLE | nxp_c45_get_phase_shift(degree));
	} else {
		phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_TXID, 0);
	}

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID) {
		degree = div_u64(rx_delay, PS_PER_DEGREE);
		phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_RXID,
			      ID_ENABLE | nxp_c45_get_phase_shift(degree));
	} else {
		phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_RXID, 0);
	}
}

static int nxp_c45_set_phy_mode(struct phy_device *phydev)
{
	int ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND1, VEND1_ABILITIES);
	pr_debug("%s: Clause 45 managed PHY abilities 0x%x\n",
		 phydev->drv->name, ret);

	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RGMII:
		if (!(ret & RGMII_ABILITY)) {
			pr_err("%s: rgmii mode not supported\n",
			       phydev->drv->name);
			return -EINVAL;
		}
		phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_MII_BASIC_CONFIG,
			      MII_BASIC_CONFIG_RGMII);
		nxp_c45_disable_delays(phydev);
		break;
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
		if (!(ret & RGMII_ID_ABILITY)) {
			pr_err("%s: rgmii-id, rgmii-txid, rgmii-rxid modes are not supported\n",
			       phydev->drv->name);
			return -EINVAL;
		}
		phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_MII_BASIC_CONFIG,
			      MII_BASIC_CONFIG_RGMII);
		ret = nxp_c45_get_delays(phydev);
		if (ret)
			return ret;

		nxp_c45_set_delays(phydev);
		break;
	case PHY_INTERFACE_MODE_MII:
		if (!(ret & MII_ABILITY)) {
			pr_err("%s: mii mode not supported\n",
			       phydev->drv->name);
			return -EINVAL;
		}
		phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_MII_BASIC_CONFIG,
			      MII_BASIC_CONFIG_MII);
		break;
	case PHY_INTERFACE_MODE_RMII:
		if (!(ret & RMII_ABILITY)) {
			pr_err("%s: rmii mode not supported\n",
			       phydev->drv->name);
			return -EINVAL;
		}
		phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_MII_BASIC_CONFIG,
			      MII_BASIC_CONFIG_RMII);
		break;
	case PHY_INTERFACE_MODE_SGMII:
		if (!(ret & SGMII_ABILITY)) {
			pr_err("%s: sgmii mode not supported\n",
			       phydev->drv->name);
			return -EINVAL;
		}
		phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_MII_BASIC_CONFIG,
			      MII_BASIC_CONFIG_SGMII);
		break;
	case PHY_INTERFACE_MODE_INTERNAL:
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int nxp_c45_config(struct phy_device *phydev)
{
	int ret;

	ret = nxp_c45_soft_reset(phydev);
	if (ret)
		return ret;

	ret = nxp_c45_config_enable(phydev);
	if (ret) {
		pr_err("%s: Failed to enable config\n", phydev->drv->name);
		return ret;
	}

	phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_PHY_CONFIG,
		      PHY_CONFIG_AUTO);

	ret = nxp_c45_set_phy_mode(phydev);
	if (ret) {
		pr_err("%s: Failed to set phy mode\n", phydev->drv->name);
		return ret;
	}

	phydev->autoneg = AUTONEG_DISABLE;

	return nxp_c45_start_op(phydev);
}

static int nxp_c45_speed(struct phy_device *phydev)
{
	int val;

	val = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_CTRL1);
	if (val < 0)
		return val;

	if (val & MDIO_PMA_CTRL1_SPEED100)
		phydev->speed = SPEED_100;
	else if (val & MDIO_PMA_CTRL1_SPEED1000)
		phydev->speed = SPEED_1000;
	else
		phydev->speed = 0;

	return 0;
}

static int nxp_c45_startup(struct phy_device *phydev)
{
	u32 reg;
	int ret;

	reg = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_STAT1);
	phydev->link = !!(reg & MDIO_STAT1_LSTATUS);
	ret = nxp_c45_speed(phydev);
	if (ret < 0)
		return ret;

	phydev->duplex = DUPLEX_FULL;
	return 0;
}

static int nxp_c45_probe(struct phy_device *phydev)
{
	struct nxp_c45_phy *priv;

	priv = devm_kzalloc(phydev->priv, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	phydev->priv = priv;

	return 0;
}

#define NXP_C45_COMMON_FEATURES	(SUPPORTED_TP | \
	SUPPORTED_MII)

U_BOOT_PHY_DRIVER(nxp_c45_tja1103) = {
	.name = "NXP C45 TJA1103",
	.uid  = PHY_ID_TJA_1103,
	.mask = 0xfffff0,
	.features = NXP_C45_COMMON_FEATURES | SUPPORTED_100baseT_Full,
	.probe = &nxp_c45_probe,
	.config = &nxp_c45_config,
	.startup = &nxp_c45_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(nxp_c45_tja1120) = {
	.name = "NXP C45 TJA1120",
	.uid  = PHY_ID_TJA_1120,
	.mask = 0xfffff0,
	.features = NXP_C45_COMMON_FEATURES | SUPPORTED_1000baseT_Full,
	.probe = &nxp_c45_probe,
	.config = &nxp_c45_config,
	.startup = &nxp_c45_startup,
	.shutdown = &genphy_shutdown,
};
