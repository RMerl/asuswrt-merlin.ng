// SPDX-License-Identifier: GPL-2.0+
/*
 * TI multiplexer clock support
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 *
 * Based on Linux kernel drivers/clk/ti/mux.c
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <clk-uclass.h>
#include <asm/io.h>
#include <linux/clk-provider.h>
#include "clk.h"

struct clk_ti_mux_priv {
	struct clk_bulk parents;
	struct clk_ti_reg reg;
	u32 flags;
	u32 mux_flags;
	u32 mask;
	u32 shift;
	s32 latch;
};

static struct clk *clk_ti_mux_get_parent_by_index(struct clk_bulk *parents,
						  int index)
{
	if (index < 0 || !parents)
		return ERR_PTR(-EINVAL);

	if (index >= parents->count)
		return ERR_PTR(-ENODEV);

	return &parents->clks[index];
}

static int clk_ti_mux_get_parent_index(struct clk_bulk *parents,
				       struct clk *parent)
{
	int i;

	if (!parents || !parent)
		return -EINVAL;

	for (i = 0; i < parents->count; i++) {
		if (parents->clks[i].dev == parent->dev)
			return i;
	}

	return -ENODEV;
}

static int clk_ti_mux_get_index(struct clk *clk)
{
	struct clk_ti_mux_priv *priv = dev_get_priv(clk->dev);
	u32 val;

	val = clk_ti_readl(&priv->reg);
	val >>= priv->shift;
	val &= priv->mask;

	if (val && (priv->flags & CLK_MUX_INDEX_BIT))
		val = ffs(val) - 1;

	if (val && (priv->flags & CLK_MUX_INDEX_ONE))
		val--;

	if (val >= priv->parents.count)
		return -EINVAL;

	return val;
}

static int clk_ti_mux_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk_ti_mux_priv *priv = dev_get_priv(clk->dev);
	int index;
	u32 val;

	index = clk_ti_mux_get_parent_index(&priv->parents, parent);
	if (index < 0) {
		dev_err(clk->dev, "failed to get parent clock\n");
		return index;
	}

	index = clk_mux_index_to_val(NULL, priv->flags, index);

	if (priv->flags & CLK_MUX_HIWORD_MASK) {
		val = priv->mask << (priv->shift + 16);
	} else {
		val = clk_ti_readl(&priv->reg);
		val &= ~(priv->mask << priv->shift);
	}

	val |= index << priv->shift;
	clk_ti_writel(val, &priv->reg);
	clk_ti_latch(&priv->reg, priv->latch);
	return 0;
}

static ulong clk_ti_mux_set_rate(struct clk *clk, ulong rate)
{
	struct clk_ti_mux_priv *priv = dev_get_priv(clk->dev);
	struct clk *parent;
	int index;

	if ((clk->flags & CLK_SET_RATE_PARENT) == 0)
		return -ENOSYS;

	index = clk_ti_mux_get_index(clk);
	parent = clk_ti_mux_get_parent_by_index(&priv->parents, index);
	if (IS_ERR(parent))
		return PTR_ERR(parent);

	rate = clk_set_rate(parent, rate);
	dev_dbg(clk->dev, "rate=%ld\n", rate);
	return rate;
}

static ulong clk_ti_mux_get_rate(struct clk *clk)
{
	struct clk_ti_mux_priv *priv = dev_get_priv(clk->dev);
	int index;
	struct clk *parent;
	ulong rate;

	index = clk_ti_mux_get_index(clk);
	parent = clk_ti_mux_get_parent_by_index(&priv->parents, index);
	if (IS_ERR(parent))
		return PTR_ERR(parent);

	rate = clk_get_rate(parent);
	dev_dbg(clk->dev, "rate=%ld\n", rate);
	return rate;
}

static ulong clk_ti_mux_round_rate(struct clk *clk, ulong rate)
{
	struct clk_ti_mux_priv *priv = dev_get_priv(clk->dev);
	struct clk *parent;
	int index;

	if ((clk->flags & CLK_SET_RATE_PARENT) == 0)
		return -ENOSYS;

	index = clk_ti_mux_get_index(clk);
	parent = clk_ti_mux_get_parent_by_index(&priv->parents, index);
	if (IS_ERR(parent))
		return PTR_ERR(parent);

	rate = clk_round_rate(parent, rate);
	dev_dbg(clk->dev, "rate=%ld\n", rate);
	return rate;
}

static int clk_ti_mux_request(struct clk *clk)
{
	struct clk_ti_mux_priv *priv = dev_get_priv(clk->dev);
	struct clk *parent;
	int index;

	clk->flags = priv->flags;

	index = clk_ti_mux_get_index(clk);
	parent = clk_ti_mux_get_parent_by_index(&priv->parents, index);
	if (IS_ERR(parent))
		return PTR_ERR(parent);

	return clk_ti_mux_set_parent(clk, parent);
}

static struct clk_ops clk_ti_mux_ops = {
	.request = clk_ti_mux_request,
	.round_rate = clk_ti_mux_round_rate,
	.get_rate = clk_ti_mux_get_rate,
	.set_rate = clk_ti_mux_set_rate,
	.set_parent = clk_ti_mux_set_parent,
};

static int clk_ti_mux_remove(struct udevice *dev)
{
	struct clk_ti_mux_priv *priv = dev_get_priv(dev);
	int err;

	err = clk_release_all(priv->parents.clks, priv->parents.count);
	if (err)
		dev_dbg(dev, "could not release all parents' clocks\n");

	return err;
}

static int clk_ti_mux_probe(struct udevice *dev)
{
	struct clk_ti_mux_priv *priv = dev_get_priv(dev);
	int err;

	err = clk_get_bulk(dev, &priv->parents);
	if (err || priv->parents.count < 2) {
		dev_err(dev, "mux-clock must have parents\n");
		return err ? err : -EFAULT;
	}

	/* Generate bit-mask based on parents info */
	priv->mask = priv->parents.count;
	if (!(priv->mux_flags & CLK_MUX_INDEX_ONE))
		priv->mask--;

	priv->mask = (1 << fls(priv->mask)) - 1;
	return 0;
}

static int clk_ti_mux_of_to_plat(struct udevice *dev)
{
	struct clk_ti_mux_priv *priv = dev_get_priv(dev);
	int err;

	err = clk_ti_get_reg_addr(dev, 0, &priv->reg);
	if (err) {
		dev_err(dev, "failed to get register address\n");
		return err;
	}

	priv->shift = dev_read_u32_default(dev, "ti,bit-shift", 0);
	priv->latch = dev_read_s32_default(dev, "ti,latch-bit", -EINVAL);

	priv->flags = CLK_SET_RATE_NO_REPARENT;
	if (dev_read_bool(dev, "ti,set-rate-parent"))
		priv->flags |= CLK_SET_RATE_PARENT;

	if (dev_read_bool(dev, "ti,index-starts-at-one"))
		priv->mux_flags |= CLK_MUX_INDEX_ONE;

	return 0;
}

static const struct udevice_id clk_ti_mux_of_match[] = {
	{.compatible = "ti,mux-clock"},
	{},
};

U_BOOT_DRIVER(clk_ti_mux) = {
	.name = "ti_mux_clock",
	.id = UCLASS_CLK,
	.of_match = clk_ti_mux_of_match,
	.of_to_plat = clk_ti_mux_of_to_plat,
	.probe = clk_ti_mux_probe,
	.remove = clk_ti_mux_remove,
	.priv_auto = sizeof(struct clk_ti_mux_priv),
	.ops = &clk_ti_mux_ops,
};
