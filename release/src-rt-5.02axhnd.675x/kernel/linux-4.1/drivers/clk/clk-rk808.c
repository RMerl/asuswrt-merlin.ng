/*
 * Clkout driver for Rockchip RK808
 *
 * Copyright (c) 2014, Fuzhou Rockchip Electronics Co., Ltd
 *
 * Author:Chris Zhong <zyw@rock-chips.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/mfd/rk808.h>
#include <linux/i2c.h>

#define RK808_NR_OUTPUT 2

struct rk808_clkout {
	struct rk808 *rk808;
	struct clk_onecell_data clk_data;
	struct clk_hw		clkout1_hw;
	struct clk_hw		clkout2_hw;
};

static unsigned long rk808_clkout_recalc_rate(struct clk_hw *hw,
					      unsigned long parent_rate)
{
	return 32768;
}

static int rk808_clkout2_enable(struct clk_hw *hw, bool enable)
{
	struct rk808_clkout *rk808_clkout = container_of(hw,
							 struct rk808_clkout,
							 clkout2_hw);
	struct rk808 *rk808 = rk808_clkout->rk808;

	return regmap_update_bits(rk808->regmap, RK808_CLK32OUT_REG,
				  CLK32KOUT2_EN, enable ? CLK32KOUT2_EN : 0);
}

static int rk808_clkout2_prepare(struct clk_hw *hw)
{
	return rk808_clkout2_enable(hw, true);
}

static void rk808_clkout2_unprepare(struct clk_hw *hw)
{
	rk808_clkout2_enable(hw, false);
}

static int rk808_clkout2_is_prepared(struct clk_hw *hw)
{
	struct rk808_clkout *rk808_clkout = container_of(hw,
							 struct rk808_clkout,
							 clkout2_hw);
	struct rk808 *rk808 = rk808_clkout->rk808;
	uint32_t val;

	int ret = regmap_read(rk808->regmap, RK808_CLK32OUT_REG, &val);

	if (ret < 0)
		return ret;

	return (val & CLK32KOUT2_EN) ? 1 : 0;
}

static const struct clk_ops rk808_clkout1_ops = {
	.recalc_rate = rk808_clkout_recalc_rate,
};

static const struct clk_ops rk808_clkout2_ops = {
	.prepare = rk808_clkout2_prepare,
	.unprepare = rk808_clkout2_unprepare,
	.is_prepared = rk808_clkout2_is_prepared,
	.recalc_rate = rk808_clkout_recalc_rate,
};

static int rk808_clkout_probe(struct platform_device *pdev)
{
	struct rk808 *rk808 = dev_get_drvdata(pdev->dev.parent);
	struct i2c_client *client = rk808->i2c;
	struct device_node *node = client->dev.of_node;
	struct clk_init_data init = {};
	struct clk **clk_table;
	struct rk808_clkout *rk808_clkout;

	rk808_clkout = devm_kzalloc(&client->dev,
				    sizeof(*rk808_clkout), GFP_KERNEL);
	if (!rk808_clkout)
		return -ENOMEM;

	rk808_clkout->rk808 = rk808;

	clk_table = devm_kcalloc(&client->dev, RK808_NR_OUTPUT,
				 sizeof(struct clk *), GFP_KERNEL);
	if (!clk_table)
		return -ENOMEM;

	init.flags = CLK_IS_ROOT;
	init.parent_names = NULL;
	init.num_parents = 0;
	init.name = "rk808-clkout1";
	init.ops = &rk808_clkout1_ops;
	rk808_clkout->clkout1_hw.init = &init;

	/* optional override of the clockname */
	of_property_read_string_index(node, "clock-output-names",
				      0, &init.name);

	clk_table[0] = devm_clk_register(&client->dev,
					 &rk808_clkout->clkout1_hw);
	if (IS_ERR(clk_table[0]))
		return PTR_ERR(clk_table[0]);

	init.name = "rk808-clkout2";
	init.ops = &rk808_clkout2_ops;
	rk808_clkout->clkout2_hw.init = &init;

	/* optional override of the clockname */
	of_property_read_string_index(node, "clock-output-names",
				      1, &init.name);

	clk_table[1] = devm_clk_register(&client->dev,
					 &rk808_clkout->clkout2_hw);
	if (IS_ERR(clk_table[1]))
		return PTR_ERR(clk_table[1]);

	rk808_clkout->clk_data.clks = clk_table;
	rk808_clkout->clk_data.clk_num = RK808_NR_OUTPUT;

	return of_clk_add_provider(node, of_clk_src_onecell_get,
				   &rk808_clkout->clk_data);
}

static int rk808_clkout_remove(struct platform_device *pdev)
{
	struct rk808 *rk808 = dev_get_drvdata(pdev->dev.parent);
	struct i2c_client *client = rk808->i2c;
	struct device_node *node = client->dev.of_node;

	of_clk_del_provider(node);

	return 0;
}

static struct platform_driver rk808_clkout_driver = {
	.probe = rk808_clkout_probe,
	.remove = rk808_clkout_remove,
	.driver		= {
		.name	= "rk808-clkout",
	},
};

module_platform_driver(rk808_clkout_driver);

MODULE_DESCRIPTION("Clkout driver for the rk808 series PMICs");
MODULE_AUTHOR("Chris Zhong <zyw@rock-chips.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:rk808-clkout");
