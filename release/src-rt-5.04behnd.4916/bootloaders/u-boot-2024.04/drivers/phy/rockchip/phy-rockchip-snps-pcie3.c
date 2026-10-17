// SPDX-License-Identifier: GPL-2.0
/*
 * Rockchip PCIE3.0 phy driver
 *
 * Copyright (C) 2021 Rockchip Electronics Co., Ltd.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <regmap.h>
#include <reset-uclass.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/lists.h>

/* Register for RK3568 */
#define GRF_PCIE30PHY_CON1			0x4
#define GRF_PCIE30PHY_CON6			0x18
#define GRF_PCIE30PHY_CON9			0x24
#define GRF_PCIE30PHY_DA_OCM			(BIT(15) | BIT(31))
#define GRF_PCIE30PHY_STATUS0			0x80
#define GRF_PCIE30PHY_WR_EN			(0xf << 16)
#define SRAM_INIT_DONE(reg)			(reg & BIT(14))

#define RK3568_BIFURCATION_LANE_0_1		BIT(0)

/* Register for RK3588 */
#define PHP_GRF_PCIESEL_CON			0x100
#define RK3588_PCIE3PHY_GRF_CMN_CON0		0x0
#define RK3588_PCIE3PHY_GRF_PHY0_STATUS1	0x904
#define RK3588_PCIE3PHY_GRF_PHY1_STATUS1	0xa04
#define RK3588_SRAM_INIT_DONE(reg)		(reg & BIT(0))

#define RK3588_BIFURCATION_LANE_0_1		BIT(0)
#define RK3588_BIFURCATION_LANE_2_3		BIT(1)
#define RK3588_LANE_AGGREGATION			BIT(2)

/**
 * struct rockchip_p3phy_priv - RK DW PCIe PHY state
 *
 * @mmio: The base address of PHY internal registers
 * @phy_grf: The regmap for controlling pipe signal
 * @p30phy: The reset signal for PHY
 * @clks: The clocks for PHY
 * @num_lanes: The number of lane to controller mappings
 * @lanes: The lane to controller mapping
 */
struct rockchip_p3phy_priv {
	void __iomem *mmio;
	struct regmap *phy_grf;
	struct regmap *pipe_grf;
	struct reset_ctl p30phy;
	struct clk_bulk clks;
	int num_lanes;
	u32 lanes[4];
};

struct rockchip_p3phy_ops {
	int (*phy_init)(struct phy *phy);
};

static int rockchip_p3phy_rk3568_init(struct phy *phy)
{
	struct rockchip_p3phy_priv *priv = dev_get_priv(phy->dev);
	bool bifurcation = false;
	int ret;
	u32 reg;

	/* Deassert PCIe PMA output clamp mode */
	regmap_write(priv->phy_grf, GRF_PCIE30PHY_CON9, GRF_PCIE30PHY_DA_OCM);

	for (int i = 0; i < priv->num_lanes; i++) {
		if (priv->lanes[i] > 1)
			bifurcation = true;
	}

	/* Set bifurcation if needed, and it doesn't care RC/EP */
	if (bifurcation) {
		regmap_write(priv->phy_grf, GRF_PCIE30PHY_CON6,
			     GRF_PCIE30PHY_WR_EN | RK3568_BIFURCATION_LANE_0_1);
		regmap_write(priv->phy_grf, GRF_PCIE30PHY_CON1,
			     GRF_PCIE30PHY_DA_OCM);
	} else {
		regmap_write(priv->phy_grf, GRF_PCIE30PHY_CON6,
			     GRF_PCIE30PHY_WR_EN & ~RK3568_BIFURCATION_LANE_0_1);
	}

	reset_deassert(&priv->p30phy);
	udelay(1);

	ret = regmap_read_poll_timeout(priv->phy_grf,
				       GRF_PCIE30PHY_STATUS0,
				       reg, SRAM_INIT_DONE(reg),
				       0, 500);
	if (ret)
		dev_err(phy->dev, "lock failed 0x%x\n", reg);

	return ret;
}

static const struct rockchip_p3phy_ops rk3568_ops = {
	.phy_init = rockchip_p3phy_rk3568_init,
};

static int rockchip_p3phy_rk3588_init(struct phy *phy)
{
	struct rockchip_p3phy_priv *priv = dev_get_priv(phy->dev);
	u32 reg = 0;
	u8 mode = 0;
	int ret;

	/* Deassert PCIe PMA output clamp mode */
	regmap_write(priv->phy_grf, RK3588_PCIE3PHY_GRF_CMN_CON0,
		     BIT(8) | BIT(24));

	/* Set bifurcation if needed */
	for (int i = 0; i < priv->num_lanes; i++) {
		if (!priv->lanes[i])
			mode |= (BIT(i) << 3);

		if (priv->lanes[i] > 1)
			mode |= (BIT(i) >> 1);
	}

	if (!mode) {
		reg = RK3588_LANE_AGGREGATION;
	} else {
		if (mode & (BIT(0) | BIT(1)))
			reg |= RK3588_BIFURCATION_LANE_0_1;

		if (mode & (BIT(2) | BIT(3)))
			reg |= RK3588_BIFURCATION_LANE_2_3;
	}

	regmap_write(priv->phy_grf, RK3588_PCIE3PHY_GRF_CMN_CON0,
		     (0x7 << 16) | reg);

	/* Set pcie1ln_sel in PHP_GRF_PCIESEL_CON */
	reg = (mode & (BIT(6) | BIT(7))) >> 6;
	if (reg)
		regmap_write(priv->pipe_grf, PHP_GRF_PCIESEL_CON,
			     (reg << 16) | reg);

	reset_deassert(&priv->p30phy);
	udelay(1);

	ret = regmap_read_poll_timeout(priv->phy_grf,
				       RK3588_PCIE3PHY_GRF_PHY0_STATUS1,
				       reg, RK3588_SRAM_INIT_DONE(reg),
				       0, 500);
	ret |= regmap_read_poll_timeout(priv->phy_grf,
					RK3588_PCIE3PHY_GRF_PHY1_STATUS1,
					reg, RK3588_SRAM_INIT_DONE(reg),
					0, 500);
	if (ret)
		dev_err(phy->dev, "lock failed 0x%x\n", reg);

	return ret;
}

static const struct rockchip_p3phy_ops rk3588_ops = {
	.phy_init = rockchip_p3phy_rk3588_init,
};

static int rochchip_p3phy_init(struct phy *phy)
{
	struct rockchip_p3phy_ops *ops =
		(struct rockchip_p3phy_ops *)dev_get_driver_data(phy->dev);
	struct rockchip_p3phy_priv *priv = dev_get_priv(phy->dev);
	int ret;

	ret = clk_enable_bulk(&priv->clks);
	if (ret)
		return ret;

	reset_assert(&priv->p30phy);
	udelay(1);

	ret = ops->phy_init(phy);
	if (ret)
		clk_disable_bulk(&priv->clks);

	return ret;
}

static int rochchip_p3phy_exit(struct phy *phy)
{
	struct rockchip_p3phy_priv *priv = dev_get_priv(phy->dev);

	clk_disable_bulk(&priv->clks);
	reset_assert(&priv->p30phy);

	return 0;
}

static int rockchip_p3phy_probe(struct udevice *dev)
{
	struct rockchip_p3phy_priv *priv = dev_get_priv(dev);
	int ret;

	priv->mmio = dev_read_addr_ptr(dev);
	if (!priv->mmio)
		return -EINVAL;

	priv->phy_grf = syscon_regmap_lookup_by_phandle(dev, "rockchip,phy-grf");
	if (IS_ERR(priv->phy_grf)) {
		dev_err(dev, "failed to find rockchip,phy_grf regmap\n");
		return PTR_ERR(priv->phy_grf);
	}

	if (device_is_compatible(dev, "rockchip,rk3588-pcie3-phy")) {
		priv->pipe_grf =
			syscon_regmap_lookup_by_phandle(dev, "rockchip,pipe-grf");
		if (IS_ERR(priv->pipe_grf)) {
			dev_err(dev, "failed to find rockchip,pipe_grf regmap\n");
			return PTR_ERR(priv->pipe_grf);
		}
	}

	ret = dev_read_size(dev, "data-lanes");
	if (ret > 0) {
		priv->num_lanes = ret / sizeof(u32);
		if (priv->num_lanes < 2 ||
		    priv->num_lanes > ARRAY_SIZE(priv->lanes)) {
			dev_err(dev, "unsupported data-lanes property size\n");
			return -EINVAL;
		}

		ret = dev_read_u32_array(dev, "data-lanes", priv->lanes,
					 priv->num_lanes);
		if (ret) {
			dev_err(dev, "failed to read data-lanes property\n");
			return ret;
		}
	}

	ret = reset_get_by_name(dev, "phy", &priv->p30phy);
	if (ret) {
		dev_err(dev, "no phy reset control specified\n");
		return ret;
	}

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret) {
		dev_err(dev, "failed to get clocks\n");
		return ret;
	}

	return 0;
}

static struct phy_ops rochchip_p3phy_ops = {
	.init = rochchip_p3phy_init,
	.exit = rochchip_p3phy_exit,
};

static const struct udevice_id rockchip_p3phy_of_match[] = {
	{
		.compatible = "rockchip,rk3568-pcie3-phy",
		.data = (ulong)&rk3568_ops,
	},
	{
		.compatible = "rockchip,rk3588-pcie3-phy",
		.data = (ulong)&rk3588_ops,
	},
	{ },
};

U_BOOT_DRIVER(rockchip_pcie3phy) = {
	.name		= "rockchip_pcie3phy",
	.id		= UCLASS_PHY,
	.of_match	= rockchip_p3phy_of_match,
	.ops		= &rochchip_p3phy_ops,
	.probe		= rockchip_p3phy_probe,
	.priv_auto	= sizeof(struct rockchip_p3phy_priv),
};
