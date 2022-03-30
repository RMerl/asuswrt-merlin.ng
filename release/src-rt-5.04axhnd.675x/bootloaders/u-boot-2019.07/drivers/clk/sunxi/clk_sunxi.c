// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/arch/ccu.h>
#include <linux/log2.h>

static const struct ccu_clk_gate *priv_to_gate(struct ccu_priv *priv,
					       unsigned long id)
{
	return &priv->desc->gates[id];
}

static int sunxi_set_gate(struct clk *clk, bool on)
{
	struct ccu_priv *priv = dev_get_priv(clk->dev);
	const struct ccu_clk_gate *gate = priv_to_gate(priv, clk->id);
	u32 reg;

	if (!(gate->flags & CCU_CLK_F_IS_VALID)) {
		printf("%s: (CLK#%ld) unhandled\n", __func__, clk->id);
		return 0;
	}

	debug("%s: (CLK#%ld) off#0x%x, BIT(%d)\n", __func__,
	      clk->id, gate->off, ilog2(gate->bit));

	reg = readl(priv->base + gate->off);
	if (on)
		reg |= gate->bit;
	else
		reg &= ~gate->bit;

	writel(reg, priv->base + gate->off);

	return 0;
}

static int sunxi_clk_enable(struct clk *clk)
{
	return sunxi_set_gate(clk, true);
}

static int sunxi_clk_disable(struct clk *clk)
{
	return sunxi_set_gate(clk, false);
}

struct clk_ops sunxi_clk_ops = {
	.enable = sunxi_clk_enable,
	.disable = sunxi_clk_disable,
};

int sunxi_clk_probe(struct udevice *dev)
{
	struct ccu_priv *priv = dev_get_priv(dev);
	struct clk_bulk clk_bulk;
	struct reset_ctl_bulk rst_bulk;
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENOMEM;

	priv->desc = (const struct ccu_desc *)dev_get_driver_data(dev);
	if (!priv->desc)
		return -EINVAL;

	ret = clk_get_bulk(dev, &clk_bulk);
	if (!ret)
		clk_enable_bulk(&clk_bulk);

	ret = reset_get_bulk(dev, &rst_bulk);
	if (!ret)
		reset_deassert_bulk(&rst_bulk);

	return 0;
}
