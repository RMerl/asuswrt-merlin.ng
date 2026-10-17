// SPDX-License-Identifier: GPL-2.0+
/**
 * PCIe SERDES driver for AM654x SoC
 *
 * Copyright (C) 2018 Texas Instruments
 * Author: Kishon Vijay Abraham I <kishon@ti.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dt-bindings/phy/phy.h>
#include <generic-phy.h>
#include <asm/io.h>
#include <power-domain.h>
#include <regmap.h>
#include <syscon.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>

#define CMU_R07C		0x7c
#define CMU_MASTER_CDN_O	BIT(24)

#define COMLANE_R138		0xb38
#define CFG_VERSION_REG_MASK	GENMASK(23, 16)
#define CFG_VERSION_REG_SHIFT 16
#define VERSION			0x70

#define COMLANE_R190		0xb90
#define L1_MASTER_CDN_O		BIT(9)

#define COMLANE_R194		0xb94
#define CMU_OK_I_0		BIT(19)

#define SERDES_CTRL		0x1fd0
#define POR_EN			BIT(29)

#define WIZ_LANEXCTL_STS	0x1fe0
#define TX0_ENABLE_OVL		BIT(31)
#define TX0_ENABLE_MASK		GENMASK(30, 29)
#define TX0_ENABLE_SHIFT	29
#define TX0_DISABLE_STATE	0x0
#define TX0_SLEEP_STATE		0x1
#define TX0_SNOOZE_STATE	0x2
#define TX0_ENABLE_STATE	0x3
#define RX0_ENABLE_OVL		BIT(15)
#define RX0_ENABLE_MASK		GENMASK(14, 13)
#define RX0_ENABLE_SHIFT	13
#define RX0_DISABLE_STATE	0x0
#define RX0_SLEEP_STATE		0x1
#define RX0_SNOOZE_STATE	0x2
#define RX0_ENABLE_STATE	0x3

#define WIZ_PLL_CTRL		0x1ff4
#define PLL_ENABLE_OVL		BIT(31)
#define PLL_ENABLE_MASK		GENMASK(30, 29)
#define PLL_ENABLE_SHIFT	29
#define PLL_DISABLE_STATE	0x0
#define PLL_SLEEP_STATE		0x1
#define PLL_SNOOZE_STATE	0x2
#define PLL_ENABLE_STATE	0x3
#define PLL_OK			BIT(28)

#define PLL_LOCK_TIME		1000	/* in milliseconds */
#define SLEEP_TIME		100	/* in microseconds */

#define LANE_USB3		0x0
#define LANE_PCIE0_LANE0	0x1

#define LANE_PCIE1_LANE0	0x0
#define LANE_PCIE0_LANE1	0x1

#define SERDES_NUM_CLOCKS	3

/* SERDES control MMR bit offsets */
#define SERDES_CTL_LANE_FUNC_SEL_SHIFT	0
#define SERDES_CTL_LANE_FUNC_SEL_MASK	GENMASK(1, 0)
#define SERDES_CTL_CLK_SEL_SHIFT	4
#define SERDES_CTL_CLK_SEL_MASK		GENMASK(7, 4)

/**
 * struct serdes_am654_mux_clk_data - clock controller information structure
 */
struct serdes_am654_mux_clk_data {
	struct regmap *regmap;
	struct clk_bulk parents;
};

static int serdes_am654_mux_clk_probe(struct udevice *dev)
{
	struct serdes_am654_mux_clk_data *data = dev_get_priv(dev);
	struct udevice *syscon;
	struct regmap *regmap;
	int ret;

	debug("%s(dev=%s)\n", __func__, dev->name);

	if (!data)
		return -ENOMEM;

	ret = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   "ti,serdes-clk", &syscon);
	if (ret) {
		dev_err(dev, "unable to find syscon device\n");
		return ret;
	}

	regmap = syscon_get_regmap(syscon);
	if (IS_ERR(regmap)) {
		dev_err(dev, "Fail to get Syscon regmap\n");
		return PTR_ERR(regmap);
	}

	data->regmap = regmap;

	ret = clk_get_bulk(dev, &data->parents);
	if (ret) {
		dev_err(dev, "Failed to obtain parent clocks\n");
		return ret;
	}

	return 0;
}

static int mux_table[SERDES_NUM_CLOCKS][3] = {
	/*
	 * The entries represent values for selecting between
	 * {left input, external reference clock, right input}
	 * Only one of Left Output or Right Output should be used since
	 * both left and right output clock uses the same bits and modifying
	 * one clock will impact the other.
	 */
	{ BIT(2),               0, BIT(0) }, /* Mux of CMU refclk */
	{     -1,          BIT(3), BIT(1) }, /* Mux of Left Output */
	{ BIT(1), BIT(3) | BIT(1),     -1 }, /* Mux of Right Output */
};

static int serdes_am654_mux_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct serdes_am654_mux_clk_data *data = dev_get_priv(clk->dev);
	u32 val;
	int i;

	debug("%s(clk=%s, parent=%s)\n", __func__, clk->dev->name,
	      parent->dev->name);

	/*
	 * Since we have the same device-tree node represent both the
	 * clock and serdes device, we have two devices associated with
	 * the serdes node. assigned-clocks for this node is processed twice,
	 * once for the clock device and another time for the serdes
	 * device. When it is processed for the clock device, it is before
	 * the probe for clock device has been called. We ignore this case
	 * and rely on assigned-clocks to be processed correctly for the
	 * serdes case.
	 */
	if (!data->regmap)
		return 0;

	for (i = 0; i < data->parents.count; i++) {
		if (clk_is_match(&data->parents.clks[i], parent))
			break;
	}

	if (i >= data->parents.count)
		return -EINVAL;

	val = mux_table[clk->id][i];
	val <<= SERDES_CTL_CLK_SEL_SHIFT;

	regmap_update_bits(data->regmap, 0, SERDES_CTL_CLK_SEL_MASK, val);

	return 0;
}

static struct clk_ops serdes_am654_mux_clk_ops = {
	.set_parent = serdes_am654_mux_clk_set_parent,
};

U_BOOT_DRIVER(serdes_am654_mux_clk) = {
	.name = "ti-serdes-am654-mux-clk",
	.id = UCLASS_CLK,
	.probe = serdes_am654_mux_clk_probe,
	.priv_auto	= sizeof(struct serdes_am654_mux_clk_data),
	.ops = &serdes_am654_mux_clk_ops,
};

struct serdes_am654 {
	struct regmap *regmap;
	struct regmap *serdes_ctl;
};

static int serdes_am654_enable_pll(struct serdes_am654 *phy)
{
	u32 mask = PLL_ENABLE_OVL | PLL_ENABLE_MASK;
	u32 val = PLL_ENABLE_OVL | (PLL_ENABLE_STATE << PLL_ENABLE_SHIFT);

	regmap_update_bits(phy->regmap, WIZ_PLL_CTRL, mask, val);

	return regmap_read_poll_timeout(phy->regmap, WIZ_PLL_CTRL, val,
					val & PLL_OK, 1000, PLL_LOCK_TIME);
}

static void serdes_am654_disable_pll(struct serdes_am654 *phy)
{
	u32 mask = PLL_ENABLE_OVL | PLL_ENABLE_MASK;

	regmap_update_bits(phy->regmap, WIZ_PLL_CTRL, mask, 0);
}

static int serdes_am654_enable_txrx(struct serdes_am654 *phy)
{
	u32 mask;
	u32 val;

	/* Enable TX */
	mask = TX0_ENABLE_OVL | TX0_ENABLE_MASK;
	val = TX0_ENABLE_OVL | (TX0_ENABLE_STATE << TX0_ENABLE_SHIFT);
	regmap_update_bits(phy->regmap, WIZ_LANEXCTL_STS, mask, val);

	/* Enable RX */
	mask = RX0_ENABLE_OVL | RX0_ENABLE_MASK;
	val = RX0_ENABLE_OVL | (RX0_ENABLE_STATE << RX0_ENABLE_SHIFT);
	regmap_update_bits(phy->regmap, WIZ_LANEXCTL_STS, mask, val);

	return 0;
}

static int serdes_am654_disable_txrx(struct serdes_am654 *phy)
{
	u32 mask;

	/* Disable TX */
	mask = TX0_ENABLE_OVL | TX0_ENABLE_MASK;
	regmap_update_bits(phy->regmap, WIZ_LANEXCTL_STS, mask, 0);

	/* Disable RX */
	mask = RX0_ENABLE_OVL | RX0_ENABLE_MASK;
	regmap_update_bits(phy->regmap, WIZ_LANEXCTL_STS, mask, 0);

	return 0;
}

static int serdes_am654_power_on(struct phy *x)
{
	struct serdes_am654 *phy = dev_get_priv(x->dev);
	int ret;
	u32 val;

	ret = serdes_am654_enable_pll(phy);
	if (ret) {
		dev_err(x->dev, "Failed to enable PLL\n");
		return ret;
	}

	ret = serdes_am654_enable_txrx(phy);
	if (ret) {
		dev_err(x->dev, "Failed to enable TX RX\n");
		return ret;
	}

	return regmap_read_poll_timeout(phy->regmap, COMLANE_R194, val,
					val & CMU_OK_I_0, SLEEP_TIME,
					PLL_LOCK_TIME);
}

static int serdes_am654_power_off(struct phy *x)
{
	struct serdes_am654 *phy = dev_get_priv(x->dev);

	serdes_am654_disable_txrx(phy);
	serdes_am654_disable_pll(phy);

	return 0;
}

static int serdes_am654_init(struct phy *x)
{
	struct serdes_am654 *phy = dev_get_priv(x->dev);
	u32 mask;
	u32 val;

	mask = CFG_VERSION_REG_MASK;
	val = VERSION << CFG_VERSION_REG_SHIFT;
	regmap_update_bits(phy->regmap, COMLANE_R138, mask, val);

	val = CMU_MASTER_CDN_O;
	regmap_update_bits(phy->regmap, CMU_R07C, val, val);

	val = L1_MASTER_CDN_O;
	regmap_update_bits(phy->regmap, COMLANE_R190, val, val);

	return 0;
}

static int serdes_am654_reset(struct phy *x)
{
	struct serdes_am654 *phy = dev_get_priv(x->dev);
	u32 val;

	val = POR_EN;
	regmap_update_bits(phy->regmap, SERDES_CTRL, val, val);
	mdelay(1);
	regmap_update_bits(phy->regmap, SERDES_CTRL, val, 0);

	return 0;
}

static int serdes_am654_of_xlate(struct phy *x,
				 struct ofnode_phandle_args *args)
{
	struct serdes_am654 *phy = dev_get_priv(x->dev);

	if (args->args_count != 2) {
		dev_err(x->dev, "Invalid DT PHY argument count: %d\n",
			args->args_count);
		return -EINVAL;
	}

	if (args->args[0] != PHY_TYPE_PCIE) {
		dev_err(x->dev, "Unrecognized PHY type: %d\n",
			args->args[0]);
		return -EINVAL;
	}

	x->id = args->args[0] | (args->args[1] << 16);

	/* Setup mux mode using second argument */
	regmap_update_bits(phy->serdes_ctl, 0, SERDES_CTL_LANE_FUNC_SEL_MASK,
			   args->args[1]);

	return 0;
}

static int serdes_am654_bind(struct udevice *dev)
{
	int ret;

	ret = device_bind_driver_to_node(dev->parent,
					 "ti-serdes-am654-mux-clk",
					 dev_read_name(dev), dev_ofnode(dev),
					 NULL);
	if (ret) {
		dev_err(dev, "%s: not able to bind clock driver\n", __func__);
		return ret;
	}

	return 0;
}

static int serdes_am654_probe(struct udevice *dev)
{
	struct serdes_am654 *phy = dev_get_priv(dev);
	struct power_domain serdes_pwrdmn;
	struct regmap *serdes_ctl;
	struct regmap *map;
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &map);
	if (ret)
		return ret;

	phy->regmap = map;

	serdes_ctl = syscon_regmap_lookup_by_phandle(dev, "ti,serdes-clk");
	if (IS_ERR(serdes_ctl)) {
		dev_err(dev, "unable to find syscon device\n");
		return PTR_ERR(serdes_ctl);
	}

	phy->serdes_ctl = serdes_ctl;

	ret = power_domain_get_by_index(dev, &serdes_pwrdmn, 0);
	if (ret) {
		dev_err(dev, "failed to get power domain\n");
		return ret;
	}

	ret = power_domain_on(&serdes_pwrdmn);
	if (ret) {
		dev_err(dev, "Power domain on failed\n");
		return ret;
	}

	return 0;
}

static const struct udevice_id serdes_am654_phy_ids[] = {
	{
		.compatible = "ti,phy-am654-serdes",
	},
};

static const struct phy_ops serdes_am654_phy_ops = {
	.reset		= serdes_am654_reset,
	.init		= serdes_am654_init,
	.power_on	= serdes_am654_power_on,
	.power_off	= serdes_am654_power_off,
	.of_xlate	= serdes_am654_of_xlate,
};

U_BOOT_DRIVER(am654_serdes_phy) = {
	.name	= "am654_serdes_phy",
	.id	= UCLASS_PHY,
	.of_match = serdes_am654_phy_ids,
	.bind = serdes_am654_bind,
	.ops = &serdes_am654_phy_ops,
	.probe = serdes_am654_probe,
	.priv_auto	= sizeof(struct serdes_am654),
};
