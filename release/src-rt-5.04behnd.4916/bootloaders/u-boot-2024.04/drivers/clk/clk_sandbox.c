// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2015 Google, Inc
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <asm/clk.h>
#include <linux/clk-provider.h>

static ulong sandbox_clk_get_rate(struct clk *clk)
{
	struct sandbox_clk_priv *priv = dev_get_priv(clk->dev);

	if (!priv->probed)
		return -ENODEV;

	if (clk->id >= SANDBOX_CLK_ID_COUNT)
		return -EINVAL;

	return priv->rate[clk->id];
}

static ulong sandbox_clk_round_rate(struct clk *clk, ulong rate)
{
	struct sandbox_clk_priv *priv = dev_get_priv(clk->dev);

	if (!priv->probed)
		return -ENODEV;

	if (clk->id >= SANDBOX_CLK_ID_COUNT)
		return -EINVAL;

	if (!rate)
		return -EINVAL;

	return rate;
}

static ulong sandbox_clk_set_rate(struct clk *clk, ulong rate)
{
	struct sandbox_clk_priv *priv = dev_get_priv(clk->dev);
	ulong old_rate;

	if (!priv->probed)
		return -ENODEV;

	if (clk->id >= SANDBOX_CLK_ID_COUNT)
		return -EINVAL;

	if (!rate)
		return -EINVAL;

	old_rate = priv->rate[clk->id];
	priv->rate[clk->id] = rate;

	return old_rate;
}

static int sandbox_clk_enable(struct clk *clk)
{
	struct sandbox_clk_priv *priv = dev_get_priv(clk->dev);

	if (!priv->probed)
		return -ENODEV;

	if (clk->id >= SANDBOX_CLK_ID_COUNT)
		return -EINVAL;

	priv->enabled[clk->id] = true;

	return 0;
}

static int sandbox_clk_disable(struct clk *clk)
{
	struct sandbox_clk_priv *priv = dev_get_priv(clk->dev);

	if (!priv->probed)
		return -ENODEV;

	if (clk->id >= SANDBOX_CLK_ID_COUNT)
		return -EINVAL;

	priv->enabled[clk->id] = false;

	return 0;
}

static int sandbox_clk_request(struct clk *clk)
{
	struct sandbox_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id >= SANDBOX_CLK_ID_COUNT)
		return -EINVAL;

	priv->requested[clk->id] = true;
	return 0;
}

static struct clk_ops sandbox_clk_ops = {
	.round_rate	= sandbox_clk_round_rate,
	.get_rate	= sandbox_clk_get_rate,
	.set_rate	= sandbox_clk_set_rate,
	.enable		= sandbox_clk_enable,
	.disable	= sandbox_clk_disable,
	.request	= sandbox_clk_request,
};

static int sandbox_clk_probe(struct udevice *dev)
{
	struct sandbox_clk_priv *priv = dev_get_priv(dev);

	priv->probed = true;
	return 0;
}

static const struct udevice_id sandbox_clk_ids[] = {
	{ .compatible = "sandbox,clk" },
	{ }
};

U_BOOT_DRIVER(sandbox_clk) = {
	.name		= "sandbox_clk",
	.id		= UCLASS_CLK,
	.of_match	= sandbox_clk_ids,
	.ops		= &sandbox_clk_ops,
	.probe		= sandbox_clk_probe,
	.priv_auto	= sizeof(struct sandbox_clk_priv),
};

ulong sandbox_clk_query_rate(struct udevice *dev, int id)
{
	struct sandbox_clk_priv *priv = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_ID_COUNT)
		return -EINVAL;

	return priv->rate[id];
}

int sandbox_clk_query_enable(struct udevice *dev, int id)
{
	struct sandbox_clk_priv *priv = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_ID_COUNT)
		return -EINVAL;

	return priv->enabled[id];
}

int sandbox_clk_query_requested(struct udevice *dev, int id)
{
	struct sandbox_clk_priv *priv = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_ID_COUNT)
		return -EINVAL;
	return priv->requested[id];
}

int clk_fixed_rate_of_to_plat(struct udevice *dev)
{
	struct clk_fixed_rate *cplat;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct sandbox_clk_fixed_rate_plat *plat = dev_get_plat(dev);

	cplat = &plat->fixed;
	cplat->fixed_rate = plat->dtplat.clock_frequency;
#else
	cplat = to_clk_fixed_rate(dev);
#endif
	clk_fixed_rate_ofdata_to_plat_(dev, cplat);

	return 0;
}

static const struct udevice_id sandbox_clk_fixed_rate_match[] = {
	{ .compatible = "sandbox,fixed-clock" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sandbox_fixed_clock) = {
	.name = "sandbox_fixed_clock",
	.id = UCLASS_CLK,
	.of_match = sandbox_clk_fixed_rate_match,
	.of_to_plat = clk_fixed_rate_of_to_plat,
	.plat_auto = sizeof(struct sandbox_clk_fixed_rate_plat),
	.ops = &clk_fixed_rate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
