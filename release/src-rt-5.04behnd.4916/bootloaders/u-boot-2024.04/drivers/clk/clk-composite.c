// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2013 NVIDIA CORPORATION.  All rights reserved.
 * Copyright 2019 NXP
 */

#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <log.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <linux/clk-provider.h>
#include <linux/err.h>

#include "clk.h"

#define UBOOT_DM_CLK_COMPOSITE "clk_composite"

static u8 clk_composite_get_parent(struct clk *clk)
{
	struct clk_composite *composite = to_clk_composite(clk_dev_binded(clk) ?
		(struct clk *)dev_get_clk_ptr(clk->dev) : clk);
	struct clk *mux = composite->mux;

	if (mux)
		return clk_mux_get_parent(mux);
	else
		return 0;
}

static int clk_composite_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk_composite *composite = to_clk_composite(clk_dev_binded(clk) ?
		(struct clk *)dev_get_clk_ptr(clk->dev) : clk);
	const struct clk_ops *mux_ops = composite->mux_ops;
	struct clk *mux = composite->mux;

	if (!mux || !mux_ops)
		return -ENOSYS;

	return mux_ops->set_parent(mux, parent);
}

static unsigned long clk_composite_recalc_rate(struct clk *clk)
{
	struct clk_composite *composite = to_clk_composite(clk_dev_binded(clk) ?
		(struct clk *)dev_get_clk_ptr(clk->dev) : clk);
	const struct clk_ops *rate_ops = composite->rate_ops;
	struct clk *rate = composite->rate;

	if (rate && rate_ops)
		return rate_ops->get_rate(rate);
	else
		return clk_get_parent_rate(clk);
}

static ulong clk_composite_set_rate(struct clk *clk, unsigned long rate)
{
	struct clk_composite *composite = to_clk_composite(clk_dev_binded(clk) ?
		(struct clk *)dev_get_clk_ptr(clk->dev) : clk);
	const struct clk_ops *rate_ops = composite->rate_ops;
	struct clk *clk_rate = composite->rate;

	if (rate && rate_ops && rate_ops->set_rate)
		return rate_ops->set_rate(clk_rate, rate);
	else
		return clk_get_rate(clk);
}

static int clk_composite_enable(struct clk *clk)
{
	struct clk_composite *composite = to_clk_composite(clk_dev_binded(clk) ?
		(struct clk *)dev_get_clk_ptr(clk->dev) : clk);
	const struct clk_ops *gate_ops = composite->gate_ops;
	struct clk *gate = composite->gate;

	if (gate && gate_ops)
		return gate_ops->enable(gate);
	else
		return 0;
}

static int clk_composite_disable(struct clk *clk)
{
	struct clk_composite *composite = to_clk_composite(clk_dev_binded(clk) ?
		(struct clk *)dev_get_clk_ptr(clk->dev) : clk);
	const struct clk_ops *gate_ops = composite->gate_ops;
	struct clk *gate = composite->gate;

	if (gate && gate_ops)
		return gate_ops->disable(gate);
	else
		return 0;
}

struct clk *clk_register_composite(struct device *dev, const char *name,
				   const char * const *parent_names,
				   int num_parents, struct clk *mux,
				   const struct clk_ops *mux_ops,
				   struct clk *rate,
				   const struct clk_ops *rate_ops,
				   struct clk *gate,
				   const struct clk_ops *gate_ops,
				   unsigned long flags)
{
	struct clk *clk;
	struct clk_composite *composite;
	int ret;

	if (!num_parents || (num_parents != 1 && !mux))
		return ERR_PTR(-EINVAL);

	composite = kzalloc(sizeof(*composite), GFP_KERNEL);
	if (!composite)
		return ERR_PTR(-ENOMEM);

	if (mux && mux_ops) {
		composite->mux = mux;
		composite->mux_ops = mux_ops;
		mux->data = (ulong)composite;
	}

	if (rate && rate_ops) {
		if (!rate_ops->get_rate) {
			clk = ERR_PTR(-EINVAL);
			goto err;
		}

		composite->rate = rate;
		composite->rate_ops = rate_ops;
		rate->data = (ulong)composite;
	}

	if (gate && gate_ops) {
		if (!gate_ops->enable || !gate_ops->disable) {
			clk = ERR_PTR(-EINVAL);
			goto err;
		}

		composite->gate = gate;
		composite->gate_ops = gate_ops;
		gate->data = (ulong)composite;
	}

	clk = &composite->clk;
	clk->flags = flags;
	ret = clk_register(clk, UBOOT_DM_CLK_COMPOSITE, name,
			   parent_names[clk_composite_get_parent(clk)]);
	if (ret) {
		clk = ERR_PTR(ret);
		goto err;
	}

	if (composite->mux)
		composite->mux->dev = clk->dev;
	if (composite->rate)
		composite->rate->dev = clk->dev;
	if (composite->gate)
		composite->gate->dev = clk->dev;

	return clk;

err:
	kfree(composite);
	return clk;
}

static const struct clk_ops clk_composite_ops = {
	.set_parent = clk_composite_set_parent,
	.get_rate = clk_composite_recalc_rate,
	.set_rate = clk_composite_set_rate,
	.enable = clk_composite_enable,
	.disable = clk_composite_disable,
};

U_BOOT_DRIVER(clk_composite) = {
	.name	= UBOOT_DM_CLK_COMPOSITE,
	.id	= UCLASS_CLK,
	.ops	= &clk_composite_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
