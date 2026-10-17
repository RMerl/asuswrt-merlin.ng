// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Sjoerd Simons <sjoerd.simons@collabora.co.uk>
 *
 * Rockchip GMAC ethernet IP driver for U-Boot
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <log.h>
#include <net.h>
#include <phy.h>
#include <syscon.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/grf_px30.h>
#include <asm/arch-rockchip/grf_rk322x.h>
#include <asm/arch-rockchip/grf_rk3288.h>
#include <asm/arch-rk3308/grf_rk3308.h>
#include <asm/arch-rockchip/grf_rk3328.h>
#include <asm/arch-rockchip/grf_rk3368.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <asm/arch-rockchip/grf_rv1108.h>
#include <dm/pinctrl.h>
#include <dt-bindings/clock/rk3288-cru.h>
#include <linux/bitops.h>
#include "designware.h"

DECLARE_GLOBAL_DATA_PTR;
#define DELAY_ENABLE(soc, tx, rx) \
	(((tx) ? soc##_TXCLK_DLY_ENA_GMAC_ENABLE : soc##_TXCLK_DLY_ENA_GMAC_DISABLE) | \
	((rx) ? soc##_RXCLK_DLY_ENA_GMAC_ENABLE : soc##_RXCLK_DLY_ENA_GMAC_DISABLE))

/*
 * Platform data for the gmac
 *
 * dw_eth_pdata: Required platform data for designware driver (must be first)
 */
struct gmac_rockchip_plat {
	struct dw_eth_pdata dw_eth_pdata;
	bool clock_input;
	int tx_delay;
	int rx_delay;
};

struct rk_gmac_ops {
	int (*fix_mac_speed)(struct dw_eth_dev *priv);
	void (*set_to_rmii)(struct gmac_rockchip_plat *pdata);
	void (*set_to_rgmii)(struct gmac_rockchip_plat *pdata);
};


static int gmac_rockchip_of_to_plat(struct udevice *dev)
{
	struct gmac_rockchip_plat *pdata = dev_get_plat(dev);
	const char *string;

	string = dev_read_string(dev, "clock_in_out");
	if (!strcmp(string, "input"))
		pdata->clock_input = true;
	else
		pdata->clock_input = false;

	/* Check the new naming-style first... */
	pdata->tx_delay = dev_read_u32_default(dev, "tx_delay", -ENOENT);
	pdata->rx_delay = dev_read_u32_default(dev, "rx_delay", -ENOENT);

	/* ... and fall back to the old naming style or default, if necessary */
	if (pdata->tx_delay == -ENOENT)
		pdata->tx_delay = dev_read_u32_default(dev, "tx-delay", 0x30);
	if (pdata->rx_delay == -ENOENT)
		pdata->rx_delay = dev_read_u32_default(dev, "rx-delay", 0x10);

	return designware_eth_of_to_plat(dev);
}

static int px30_gmac_fix_mac_speed(struct dw_eth_dev *priv)
{
	struct px30_grf *grf;
	struct clk clk_speed;
	int speed, ret;
	enum {
		PX30_GMAC_SPEED_SHIFT = 0x2,
		PX30_GMAC_SPEED_MASK  = BIT(2),
		PX30_GMAC_SPEED_10M   = 0,
		PX30_GMAC_SPEED_100M  = BIT(2),
	};

	ret = clk_get_by_name(priv->phydev->dev, "clk_mac_speed",
			      &clk_speed);
	if (ret)
		return ret;

	switch (priv->phydev->speed) {
	case 10:
		speed = PX30_GMAC_SPEED_10M;
		ret = clk_set_rate(&clk_speed, 2500000);
		if (ret)
			return ret;
		break;
	case 100:
		speed = PX30_GMAC_SPEED_100M;
		ret = clk_set_rate(&clk_speed, 25000000);
		if (ret)
			return ret;
		break;
	default:
		debug("Unknown phy speed: %d\n", priv->phydev->speed);
		return -EINVAL;
	}

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->mac_con1, PX30_GMAC_SPEED_MASK, speed);

	return 0;
}

static int rk3228_gmac_fix_mac_speed(struct dw_eth_dev *priv)
{
	struct rk322x_grf *grf;
	int clk;
	enum {
		RK3228_GMAC_CLK_SEL_SHIFT = 8,
		RK3228_GMAC_CLK_SEL_MASK  = GENMASK(9, 8),
		RK3228_GMAC_CLK_SEL_125M  = 0 << 8,
		RK3228_GMAC_CLK_SEL_25M   = 3 << 8,
		RK3228_GMAC_CLK_SEL_2_5M  = 2 << 8,
	};

	switch (priv->phydev->speed) {
	case 10:
		clk = RK3228_GMAC_CLK_SEL_2_5M;
		break;
	case 100:
		clk = RK3228_GMAC_CLK_SEL_25M;
		break;
	case 1000:
		clk = RK3228_GMAC_CLK_SEL_125M;
		break;
	default:
		debug("Unknown phy speed: %d\n", priv->phydev->speed);
		return -EINVAL;
	}

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->mac_con[1], RK3228_GMAC_CLK_SEL_MASK, clk);

	return 0;
}

static int rk3288_gmac_fix_mac_speed(struct dw_eth_dev *priv)
{
	struct rk3288_grf *grf;
	int clk;

	switch (priv->phydev->speed) {
	case 10:
		clk = RK3288_GMAC_CLK_SEL_2_5M;
		break;
	case 100:
		clk = RK3288_GMAC_CLK_SEL_25M;
		break;
	case 1000:
		clk = RK3288_GMAC_CLK_SEL_125M;
		break;
	default:
		debug("Unknown phy speed: %d\n", priv->phydev->speed);
		return -EINVAL;
	}

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->soc_con1, RK3288_GMAC_CLK_SEL_MASK, clk);

	return 0;
}

static int rk3308_gmac_fix_mac_speed(struct dw_eth_dev *priv)
{
	struct rk3308_grf *grf;
	struct clk clk_speed;
	int speed, ret;
	enum {
		RK3308_GMAC_SPEED_SHIFT = 0x0,
		RK3308_GMAC_SPEED_MASK  = BIT(0),
		RK3308_GMAC_SPEED_10M   = 0,
		RK3308_GMAC_SPEED_100M  = BIT(0),
	};

	ret = clk_get_by_name(priv->phydev->dev, "clk_mac_speed",
			      &clk_speed);
	if (ret)
		return ret;

	switch (priv->phydev->speed) {
	case 10:
		speed = RK3308_GMAC_SPEED_10M;
		ret = clk_set_rate(&clk_speed, 2500000);
		if (ret)
			return ret;
		break;
	case 100:
		speed = RK3308_GMAC_SPEED_100M;
		ret = clk_set_rate(&clk_speed, 25000000);
		if (ret)
			return ret;
		break;
	default:
		debug("Unknown phy speed: %d\n", priv->phydev->speed);
		return -EINVAL;
	}

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->mac_con0, RK3308_GMAC_SPEED_MASK, speed);

	return 0;
}

static int rk3328_gmac_fix_mac_speed(struct dw_eth_dev *priv)
{
	struct rk3328_grf_regs *grf;
	int clk;
	enum {
		RK3328_GMAC_CLK_SEL_SHIFT = 11,
		RK3328_GMAC_CLK_SEL_MASK  = GENMASK(12, 11),
		RK3328_GMAC_CLK_SEL_125M  = 0 << 11,
		RK3328_GMAC_CLK_SEL_25M   = 3 << 11,
		RK3328_GMAC_CLK_SEL_2_5M  = 2 << 11,
	};

	switch (priv->phydev->speed) {
	case 10:
		clk = RK3328_GMAC_CLK_SEL_2_5M;
		break;
	case 100:
		clk = RK3328_GMAC_CLK_SEL_25M;
		break;
	case 1000:
		clk = RK3328_GMAC_CLK_SEL_125M;
		break;
	default:
		debug("Unknown phy speed: %d\n", priv->phydev->speed);
		return -EINVAL;
	}

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->mac_con[1], RK3328_GMAC_CLK_SEL_MASK, clk);

	return 0;
}

static int rk3368_gmac_fix_mac_speed(struct dw_eth_dev *priv)
{
	struct rk3368_grf *grf;
	int clk;
	enum {
		RK3368_GMAC_CLK_SEL_2_5M = 2 << 4,
		RK3368_GMAC_CLK_SEL_25M = 3 << 4,
		RK3368_GMAC_CLK_SEL_125M = 0 << 4,
		RK3368_GMAC_CLK_SEL_MASK = GENMASK(5, 4),
	};

	switch (priv->phydev->speed) {
	case 10:
		clk = RK3368_GMAC_CLK_SEL_2_5M;
		break;
	case 100:
		clk = RK3368_GMAC_CLK_SEL_25M;
		break;
	case 1000:
		clk = RK3368_GMAC_CLK_SEL_125M;
		break;
	default:
		debug("Unknown phy speed: %d\n", priv->phydev->speed);
		return -EINVAL;
	}

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->soc_con15, RK3368_GMAC_CLK_SEL_MASK, clk);

	return 0;
}

static int rk3399_gmac_fix_mac_speed(struct dw_eth_dev *priv)
{
	struct rk3399_grf_regs *grf;
	int clk;

	switch (priv->phydev->speed) {
	case 10:
		clk = RK3399_GMAC_CLK_SEL_2_5M;
		break;
	case 100:
		clk = RK3399_GMAC_CLK_SEL_25M;
		break;
	case 1000:
		clk = RK3399_GMAC_CLK_SEL_125M;
		break;
	default:
		debug("Unknown phy speed: %d\n", priv->phydev->speed);
		return -EINVAL;
	}

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->soc_con5, RK3399_GMAC_CLK_SEL_MASK, clk);

	return 0;
}

static int rv1108_set_rmii_speed(struct dw_eth_dev *priv)
{
	struct rv1108_grf *grf;
	int clk, speed;
	enum {
		RV1108_GMAC_SPEED_MASK		= BIT(2),
		RV1108_GMAC_SPEED_10M		= 0 << 2,
		RV1108_GMAC_SPEED_100M		= 1 << 2,
		RV1108_GMAC_CLK_SEL_MASK	= BIT(7),
		RV1108_GMAC_CLK_SEL_2_5M	= 0 << 7,
		RV1108_GMAC_CLK_SEL_25M		= 1 << 7,
	};

	switch (priv->phydev->speed) {
	case 10:
		clk = RV1108_GMAC_CLK_SEL_2_5M;
		speed = RV1108_GMAC_SPEED_10M;
		break;
	case 100:
		clk = RV1108_GMAC_CLK_SEL_25M;
		speed = RV1108_GMAC_SPEED_100M;
		break;
	default:
		debug("Unknown phy speed: %d\n", priv->phydev->speed);
		return -EINVAL;
	}

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->gmac_con0,
		     RV1108_GMAC_CLK_SEL_MASK | RV1108_GMAC_SPEED_MASK,
		     clk | speed);

	return 0;
}

static void px30_gmac_set_to_rmii(struct gmac_rockchip_plat *pdata)
{
	struct px30_grf *grf;
	enum {
		PX30_GMAC_PHY_INTF_SEL_SHIFT = 4,
		PX30_GMAC_PHY_INTF_SEL_MASK  = GENMASK(4, 6),
		PX30_GMAC_PHY_INTF_SEL_RMII  = BIT(6),
	};

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	rk_clrsetreg(&grf->mac_con1,
		     PX30_GMAC_PHY_INTF_SEL_MASK,
		     PX30_GMAC_PHY_INTF_SEL_RMII);
}

static void rk3228_gmac_set_to_rgmii(struct gmac_rockchip_plat *pdata)
{
	struct rk322x_grf *grf;
	enum {
		RK3228_RMII_MODE_SHIFT = 10,
		RK3228_RMII_MODE_MASK  = BIT(10),

		RK3228_GMAC_PHY_INTF_SEL_SHIFT = 4,
		RK3228_GMAC_PHY_INTF_SEL_MASK  = GENMASK(6, 4),
		RK3228_GMAC_PHY_INTF_SEL_RGMII = BIT(4),

		RK3228_RXCLK_DLY_ENA_GMAC_MASK = BIT(1),
		RK3228_RXCLK_DLY_ENA_GMAC_DISABLE = 0,
		RK3228_RXCLK_DLY_ENA_GMAC_ENABLE = BIT(1),

		RK3228_TXCLK_DLY_ENA_GMAC_MASK = BIT(0),
		RK3228_TXCLK_DLY_ENA_GMAC_DISABLE = 0,
		RK3228_TXCLK_DLY_ENA_GMAC_ENABLE = BIT(0),
	};
	enum {
		RK3228_CLK_RX_DL_CFG_GMAC_SHIFT = 0x7,
		RK3228_CLK_RX_DL_CFG_GMAC_MASK = GENMASK(13, 7),

		RK3228_CLK_TX_DL_CFG_GMAC_SHIFT = 0x0,
		RK3228_CLK_TX_DL_CFG_GMAC_MASK = GENMASK(6, 0),
	};

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->mac_con[1],
		     RK3228_RMII_MODE_MASK |
		     RK3228_GMAC_PHY_INTF_SEL_MASK |
		     RK3228_RXCLK_DLY_ENA_GMAC_MASK |
		     RK3228_TXCLK_DLY_ENA_GMAC_MASK,
		     RK3228_GMAC_PHY_INTF_SEL_RGMII |
		     DELAY_ENABLE(RK3228, pdata->tx_delay, pdata->rx_delay));

	rk_clrsetreg(&grf->mac_con[0],
		     RK3228_CLK_RX_DL_CFG_GMAC_MASK |
		     RK3228_CLK_TX_DL_CFG_GMAC_MASK,
		     pdata->rx_delay << RK3228_CLK_RX_DL_CFG_GMAC_SHIFT |
		     pdata->tx_delay << RK3228_CLK_TX_DL_CFG_GMAC_SHIFT);
}

static void rk3288_gmac_set_to_rgmii(struct gmac_rockchip_plat *pdata)
{
	struct rk3288_grf *grf;

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->soc_con1,
		     RK3288_RMII_MODE_MASK | RK3288_GMAC_PHY_INTF_SEL_MASK,
		     RK3288_GMAC_PHY_INTF_SEL_RGMII);

	rk_clrsetreg(&grf->soc_con3,
		     RK3288_RXCLK_DLY_ENA_GMAC_MASK |
		     RK3288_TXCLK_DLY_ENA_GMAC_MASK |
		     RK3288_CLK_RX_DL_CFG_GMAC_MASK |
		     RK3288_CLK_TX_DL_CFG_GMAC_MASK,
		     DELAY_ENABLE(RK3288, pdata->rx_delay, pdata->tx_delay) |
		     pdata->rx_delay << RK3288_CLK_RX_DL_CFG_GMAC_SHIFT |
		     pdata->tx_delay << RK3288_CLK_TX_DL_CFG_GMAC_SHIFT);
}

static void rk3308_gmac_set_to_rmii(struct gmac_rockchip_plat *pdata)
{
	struct rk3308_grf *grf;
	enum {
		RK3308_GMAC_PHY_INTF_SEL_SHIFT = 2,
		RK3308_GMAC_PHY_INTF_SEL_MASK  = GENMASK(4, 2),
		RK3308_GMAC_PHY_INTF_SEL_RMII  = BIT(4),
	};

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	rk_clrsetreg(&grf->mac_con0,
		     RK3308_GMAC_PHY_INTF_SEL_MASK,
		     RK3308_GMAC_PHY_INTF_SEL_RMII);
}

static void rk3328_gmac_set_to_rgmii(struct gmac_rockchip_plat *pdata)
{
	struct rk3328_grf_regs *grf;
	enum {
		RK3328_RMII_MODE_SHIFT = 9,
		RK3328_RMII_MODE_MASK  = BIT(9),

		RK3328_GMAC_PHY_INTF_SEL_SHIFT = 4,
		RK3328_GMAC_PHY_INTF_SEL_MASK  = GENMASK(6, 4),
		RK3328_GMAC_PHY_INTF_SEL_RGMII = BIT(4),

		RK3328_RXCLK_DLY_ENA_GMAC_MASK = BIT(1),
		RK3328_RXCLK_DLY_ENA_GMAC_DISABLE = 0,
		RK3328_RXCLK_DLY_ENA_GMAC_ENABLE = BIT(1),

		RK3328_TXCLK_DLY_ENA_GMAC_MASK = BIT(0),
		RK3328_TXCLK_DLY_ENA_GMAC_DISABLE = 0,
		RK3328_TXCLK_DLY_ENA_GMAC_ENABLE = BIT(0),
	};
	enum {
		RK3328_CLK_RX_DL_CFG_GMAC_SHIFT = 0x7,
		RK3328_CLK_RX_DL_CFG_GMAC_MASK = GENMASK(13, 7),

		RK3328_CLK_TX_DL_CFG_GMAC_SHIFT = 0x0,
		RK3328_CLK_TX_DL_CFG_GMAC_MASK = GENMASK(6, 0),
	};

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->mac_con[1],
		     RK3328_RMII_MODE_MASK |
		     RK3328_GMAC_PHY_INTF_SEL_MASK |
		     RK3328_RXCLK_DLY_ENA_GMAC_MASK |
		     RK3328_TXCLK_DLY_ENA_GMAC_MASK,
		     RK3328_GMAC_PHY_INTF_SEL_RGMII |
		     DELAY_ENABLE(RK3328, pdata->tx_delay, pdata->rx_delay));

	rk_clrsetreg(&grf->mac_con[0],
		     RK3328_CLK_RX_DL_CFG_GMAC_MASK |
		     RK3328_CLK_TX_DL_CFG_GMAC_MASK,
		     pdata->rx_delay << RK3328_CLK_RX_DL_CFG_GMAC_SHIFT |
		     pdata->tx_delay << RK3328_CLK_TX_DL_CFG_GMAC_SHIFT);
}

static void rk3368_gmac_set_to_rgmii(struct gmac_rockchip_plat *pdata)
{
	struct rk3368_grf *grf;
	enum {
		RK3368_GMAC_PHY_INTF_SEL_RGMII = 1 << 9,
		RK3368_GMAC_PHY_INTF_SEL_MASK = GENMASK(11, 9),
		RK3368_RMII_MODE_MASK  = BIT(6),
		RK3368_RMII_MODE       = BIT(6),
	};
	enum {
		RK3368_RXCLK_DLY_ENA_GMAC_MASK = BIT(15),
		RK3368_RXCLK_DLY_ENA_GMAC_DISABLE = 0,
		RK3368_RXCLK_DLY_ENA_GMAC_ENABLE = BIT(15),
		RK3368_TXCLK_DLY_ENA_GMAC_MASK = BIT(7),
		RK3368_TXCLK_DLY_ENA_GMAC_DISABLE = 0,
		RK3368_TXCLK_DLY_ENA_GMAC_ENABLE = BIT(7),
		RK3368_CLK_RX_DL_CFG_GMAC_SHIFT = 8,
		RK3368_CLK_RX_DL_CFG_GMAC_MASK = GENMASK(14, 8),
		RK3368_CLK_TX_DL_CFG_GMAC_SHIFT = 0,
		RK3368_CLK_TX_DL_CFG_GMAC_MASK = GENMASK(6, 0),
	};

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->soc_con15,
		     RK3368_RMII_MODE_MASK | RK3368_GMAC_PHY_INTF_SEL_MASK,
		     RK3368_GMAC_PHY_INTF_SEL_RGMII);

	rk_clrsetreg(&grf->soc_con16,
		     RK3368_RXCLK_DLY_ENA_GMAC_MASK |
		     RK3368_TXCLK_DLY_ENA_GMAC_MASK |
		     RK3368_CLK_RX_DL_CFG_GMAC_MASK |
		     RK3368_CLK_TX_DL_CFG_GMAC_MASK,
		     DELAY_ENABLE(RK3368, pdata->tx_delay, pdata->rx_delay) |
		     pdata->rx_delay << RK3368_CLK_RX_DL_CFG_GMAC_SHIFT |
		     pdata->tx_delay << RK3368_CLK_TX_DL_CFG_GMAC_SHIFT);
}

static void rk3399_gmac_set_to_rgmii(struct gmac_rockchip_plat *pdata)
{
	struct rk3399_grf_regs *grf;

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	rk_clrsetreg(&grf->soc_con5,
		     RK3399_GMAC_PHY_INTF_SEL_MASK,
		     RK3399_GMAC_PHY_INTF_SEL_RGMII);

	rk_clrsetreg(&grf->soc_con6,
		     RK3399_RXCLK_DLY_ENA_GMAC_MASK |
		     RK3399_TXCLK_DLY_ENA_GMAC_MASK |
		     RK3399_CLK_RX_DL_CFG_GMAC_MASK |
		     RK3399_CLK_TX_DL_CFG_GMAC_MASK,
		     DELAY_ENABLE(RK3399, pdata->tx_delay, pdata->rx_delay) |
		     pdata->rx_delay << RK3399_CLK_RX_DL_CFG_GMAC_SHIFT |
		     pdata->tx_delay << RK3399_CLK_TX_DL_CFG_GMAC_SHIFT);
}

static void rv1108_gmac_set_to_rmii(struct gmac_rockchip_plat *pdata)
{
	struct rv1108_grf *grf;

	enum {
		RV1108_GMAC_PHY_INTF_SEL_MASK  = GENMASK(6, 4),
		RV1108_GMAC_PHY_INTF_SEL_RMII  = 4 << 4,
	};

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->gmac_con0,
		     RV1108_GMAC_PHY_INTF_SEL_MASK,
		     RV1108_GMAC_PHY_INTF_SEL_RMII);
}

static int gmac_rockchip_probe(struct udevice *dev)
{
	struct gmac_rockchip_plat *pdata = dev_get_plat(dev);
	struct rk_gmac_ops *ops =
		(struct rk_gmac_ops *)dev_get_driver_data(dev);
	struct dw_eth_pdata *dw_pdata = dev_get_plat(dev);
	struct eth_pdata *eth_pdata = &dw_pdata->eth_pdata;
	struct clk clk;
	ulong rate;
	int ret;

	ret = clk_set_defaults(dev, CLK_DEFAULTS_PRE);
	if (ret)
		debug("%s clk_set_defaults failed %d\n", __func__, ret);

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	switch (eth_pdata->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
		/* Set to RGMII mode */
		if (ops->set_to_rgmii)
			ops->set_to_rgmii(pdata);
		else
			return -EPERM;

		/*
		 * If the gmac clock is from internal pll, need to set and
		 * check the return value for gmac clock at RGMII mode. If
		 * the gmac clock is from external source, the clock rate
		 * is not set, because of it is bypassed.
		 */

		if (!pdata->clock_input) {
			rate = clk_set_rate(&clk, 125000000);
			if (rate != 125000000)
				return -EINVAL;
		}
		break;

	case PHY_INTERFACE_MODE_RGMII_ID:
		/* Set to RGMII mode */
		if (ops->set_to_rgmii) {
			pdata->tx_delay = 0;
			pdata->rx_delay = 0;
			ops->set_to_rgmii(pdata);
		} else
			return -EPERM;

		if (!pdata->clock_input) {
			rate = clk_set_rate(&clk, 125000000);
			if (rate != 125000000)
				return -EINVAL;
		}
		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set to RMII mode */
		if (ops->set_to_rmii)
			ops->set_to_rmii(pdata);
		else
			return -EPERM;

		if (!pdata->clock_input) {
			rate = clk_set_rate(&clk, 50000000);
			if (rate != 50000000)
				return -EINVAL;
		}
		break;

	case PHY_INTERFACE_MODE_RGMII_RXID:
		 /* Set to RGMII_RXID mode */
		if (ops->set_to_rgmii) {
			pdata->tx_delay = 0;
			ops->set_to_rgmii(pdata);
		} else
			return -EPERM;

		if (!pdata->clock_input) {
			rate = clk_set_rate(&clk, 125000000);
			if (rate != 125000000)
				return -EINVAL;
		}
		break;

	case PHY_INTERFACE_MODE_RGMII_TXID:
		/* Set to RGMII_TXID mode */
		if (ops->set_to_rgmii) {
			pdata->rx_delay = 0;
			ops->set_to_rgmii(pdata);
		} else
			return -EPERM;

		if (!pdata->clock_input) {
			rate = clk_set_rate(&clk, 125000000);
			if (rate != 125000000)
				return -EINVAL;
		}
		break;

	default:
		debug("NO interface defined!\n");
		return -ENXIO;
	}

	return designware_eth_probe(dev);
}

static int gmac_rockchip_eth_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct dw_eth_dev *priv = dev_get_priv(dev);
	struct rk_gmac_ops *ops =
		(struct rk_gmac_ops *)dev_get_driver_data(dev);
	int ret;

	ret = designware_eth_init(priv, pdata->enetaddr);
	if (ret)
		return ret;
	ret = ops->fix_mac_speed(priv);
	if (ret)
		return ret;
	ret = designware_eth_enable(priv);
	if (ret)
		return ret;

	return 0;
}

const struct eth_ops gmac_rockchip_eth_ops = {
	.start			= gmac_rockchip_eth_start,
	.send			= designware_eth_send,
	.recv			= designware_eth_recv,
	.free_pkt		= designware_eth_free_pkt,
	.stop			= designware_eth_stop,
	.write_hwaddr		= designware_eth_write_hwaddr,
};

const struct rk_gmac_ops px30_gmac_ops = {
	.fix_mac_speed = px30_gmac_fix_mac_speed,
	.set_to_rmii = px30_gmac_set_to_rmii,
};

const struct rk_gmac_ops rk3228_gmac_ops = {
	.fix_mac_speed = rk3228_gmac_fix_mac_speed,
	.set_to_rgmii = rk3228_gmac_set_to_rgmii,
};

const struct rk_gmac_ops rk3288_gmac_ops = {
	.fix_mac_speed = rk3288_gmac_fix_mac_speed,
	.set_to_rgmii = rk3288_gmac_set_to_rgmii,
};

const struct rk_gmac_ops rk3308_gmac_ops = {
	.fix_mac_speed = rk3308_gmac_fix_mac_speed,
	.set_to_rmii = rk3308_gmac_set_to_rmii,
};

const struct rk_gmac_ops rk3328_gmac_ops = {
	.fix_mac_speed = rk3328_gmac_fix_mac_speed,
	.set_to_rgmii = rk3328_gmac_set_to_rgmii,
};

const struct rk_gmac_ops rk3368_gmac_ops = {
	.fix_mac_speed = rk3368_gmac_fix_mac_speed,
	.set_to_rgmii = rk3368_gmac_set_to_rgmii,
};

const struct rk_gmac_ops rk3399_gmac_ops = {
	.fix_mac_speed = rk3399_gmac_fix_mac_speed,
	.set_to_rgmii = rk3399_gmac_set_to_rgmii,
};

const struct rk_gmac_ops rv1108_gmac_ops = {
	.fix_mac_speed = rv1108_set_rmii_speed,
	.set_to_rmii = rv1108_gmac_set_to_rmii,
};

static const struct udevice_id rockchip_gmac_ids[] = {
	{ .compatible = "rockchip,px30-gmac",
	  .data = (ulong)&px30_gmac_ops },
	{ .compatible = "rockchip,rk3228-gmac",
	  .data = (ulong)&rk3228_gmac_ops },
	{ .compatible = "rockchip,rk3288-gmac",
	  .data = (ulong)&rk3288_gmac_ops },
	{ .compatible = "rockchip,rk3308-mac",
	  .data = (ulong)&rk3308_gmac_ops },
	{ .compatible = "rockchip,rk3328-gmac",
	  .data = (ulong)&rk3328_gmac_ops },
	{ .compatible = "rockchip,rk3368-gmac",
	  .data = (ulong)&rk3368_gmac_ops },
	{ .compatible = "rockchip,rk3399-gmac",
	  .data = (ulong)&rk3399_gmac_ops },
	{ .compatible = "rockchip,rv1108-gmac",
	  .data = (ulong)&rv1108_gmac_ops },
	{ }
};

U_BOOT_DRIVER(eth_gmac_rockchip) = {
	.name	= "gmac_rockchip",
	.id	= UCLASS_ETH,
	.of_match = rockchip_gmac_ids,
	.of_to_plat = gmac_rockchip_of_to_plat,
	.probe	= gmac_rockchip_probe,
	.ops	= &gmac_rockchip_eth_ops,
	.priv_auto	= sizeof(struct dw_eth_dev),
	.plat_auto	= sizeof(struct gmac_rockchip_plat),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
