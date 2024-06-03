// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>

#include "clk-uniphier.h"

/**
 * struct uniphier_clk_priv - private data for UniPhier clock driver
 *
 * @base: base address of the clock provider
 * @data: SoC specific data
 */
struct uniphier_clk_priv {
	struct udevice *dev;
	void __iomem *base;
	const struct uniphier_clk_data *data;
};

static void uniphier_clk_gate_enable(struct uniphier_clk_priv *priv,
				     const struct uniphier_clk_gate_data *gate)
{
	u32 val;

	val = readl(priv->base + gate->reg);
	val |= BIT(gate->bit);
	writel(val, priv->base + gate->reg);
}

static void uniphier_clk_mux_set_parent(struct uniphier_clk_priv *priv,
					const struct uniphier_clk_mux_data *mux,
					u8 id)
{
	u32 val;
	int i;

	for (i = 0; i < mux->num_parents; i++) {
		if (mux->parent_ids[i] != id)
			continue;

		val = readl(priv->base + mux->reg);
		val &= ~mux->masks[i];
		val |= mux->vals[i];
		writel(val, priv->base + mux->reg);
		return;
	}

	WARN_ON(1);
}

static u8 uniphier_clk_mux_get_parent(struct uniphier_clk_priv *priv,
				      const struct uniphier_clk_mux_data *mux)
{
	u32 val;
	int i;

	val = readl(priv->base + mux->reg);

	for (i = 0; i < mux->num_parents; i++)
		if ((mux->masks[i] & val) == mux->vals[i])
			return mux->parent_ids[i];

	dev_err(priv->dev, "invalid mux setting\n");

	return UNIPHIER_CLK_ID_INVALID;
}

static const struct uniphier_clk_data *uniphier_clk_get_data(
					struct uniphier_clk_priv *priv, u8 id)
{
	const struct uniphier_clk_data *data;

	for (data = priv->data; data->type != UNIPHIER_CLK_TYPE_END; data++)
		if (data->id == id)
			return data;

	dev_err(priv->dev, "id=%u not found\n", id);

	return NULL;
}

static const struct uniphier_clk_data *uniphier_clk_get_parent_data(
					struct uniphier_clk_priv *priv,
					const struct uniphier_clk_data *data)
{
	const struct uniphier_clk_data *parent_data;
	u8 parent_id = UNIPHIER_CLK_ID_INVALID;

	switch (data->type) {
	case UNIPHIER_CLK_TYPE_GATE:
		parent_id = data->data.gate.parent_id;
		break;
	case UNIPHIER_CLK_TYPE_MUX:
		parent_id = uniphier_clk_mux_get_parent(priv, &data->data.mux);
		break;
	default:
		break;
	}

	if (parent_id == UNIPHIER_CLK_ID_INVALID)
		return NULL;

	parent_data = uniphier_clk_get_data(priv, parent_id);

	WARN_ON(!parent_data);

	return parent_data;
}

static void __uniphier_clk_enable(struct uniphier_clk_priv *priv,
				  const struct uniphier_clk_data *data)
{
	const struct uniphier_clk_data *parent_data;

	if (data->type == UNIPHIER_CLK_TYPE_GATE)
		uniphier_clk_gate_enable(priv, &data->data.gate);

	parent_data = uniphier_clk_get_parent_data(priv, data);
	if (!parent_data)
		return;

	return __uniphier_clk_enable(priv, parent_data);
}

static int uniphier_clk_enable(struct clk *clk)
{
	struct uniphier_clk_priv *priv = dev_get_priv(clk->dev);
	const struct uniphier_clk_data *data;

	data = uniphier_clk_get_data(priv, clk->id);
	if (!data)
		return -ENODEV;

	__uniphier_clk_enable(priv, data);

	return 0;
}

static unsigned long __uniphier_clk_get_rate(
					struct uniphier_clk_priv *priv,
					const struct uniphier_clk_data *data)
{
	const struct uniphier_clk_data *parent_data;

	if (data->type == UNIPHIER_CLK_TYPE_FIXED_RATE)
		return data->data.rate.fixed_rate;

	parent_data = uniphier_clk_get_parent_data(priv, data);
	if (!parent_data)
		return 0;

	return __uniphier_clk_get_rate(priv, parent_data);
}

static unsigned long uniphier_clk_get_rate(struct clk *clk)
{
	struct uniphier_clk_priv *priv = dev_get_priv(clk->dev);
	const struct uniphier_clk_data *data;

	data = uniphier_clk_get_data(priv, clk->id);
	if (!data)
		return -ENODEV;

	return __uniphier_clk_get_rate(priv, data);
}

static unsigned long __uniphier_clk_set_rate(
					struct uniphier_clk_priv *priv,
					const struct uniphier_clk_data *data,
					unsigned long rate, bool set)
{
	const struct uniphier_clk_data *best_parent_data = NULL;
	const struct uniphier_clk_data *parent_data;
	unsigned long best_rate = 0;
	unsigned long parent_rate;
	u8 parent_id;
	int i;

	if (data->type == UNIPHIER_CLK_TYPE_FIXED_RATE)
		return data->data.rate.fixed_rate;

	if (data->type == UNIPHIER_CLK_TYPE_GATE) {
		parent_data = uniphier_clk_get_parent_data(priv, data);
		if (!parent_data)
			return 0;

		return __uniphier_clk_set_rate(priv, parent_data, rate, set);
	}

	if (WARN_ON(data->type != UNIPHIER_CLK_TYPE_MUX))
		return -EINVAL;

	for (i = 0; i < data->data.mux.num_parents; i++) {
		parent_id = data->data.mux.parent_ids[i];
		parent_data = uniphier_clk_get_data(priv, parent_id);
		if (WARN_ON(!parent_data))
			return -EINVAL;

		parent_rate = __uniphier_clk_set_rate(priv, parent_data, rate,
						      false);

		if (parent_rate <= rate && best_rate < parent_rate) {
			best_rate = parent_rate;
			best_parent_data = parent_data;
		}
	}

	dev_dbg(priv->dev, "id=%u, best_rate=%lu\n", data->id, best_rate);

	if (!best_parent_data)
		return -EINVAL;

	if (!set)
		return best_rate;

	uniphier_clk_mux_set_parent(priv, &data->data.mux,
				    best_parent_data->id);

	return best_rate = __uniphier_clk_set_rate(priv, best_parent_data,
						   rate, true);
}

static unsigned long uniphier_clk_set_rate(struct clk *clk, ulong rate)
{
	struct uniphier_clk_priv *priv = dev_get_priv(clk->dev);
	const struct uniphier_clk_data *data;

	data = uniphier_clk_get_data(priv, clk->id);
	if (!data)
		return -ENODEV;

	return __uniphier_clk_set_rate(priv, data, rate, true);
}

static const struct clk_ops uniphier_clk_ops = {
	.enable = uniphier_clk_enable,
	.get_rate = uniphier_clk_get_rate,
	.set_rate = uniphier_clk_set_rate,
};

static int uniphier_clk_probe(struct udevice *dev)
{
	struct uniphier_clk_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev->parent);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = devm_ioremap(dev, addr, SZ_4K);
	if (!priv->base)
		return -ENOMEM;

	priv->dev = dev;
	priv->data = (void *)dev_get_driver_data(dev);

	return 0;
}

static const struct udevice_id uniphier_clk_match[] = {
	/* System clock */
	{
		.compatible = "socionext,uniphier-ld4-clock",
		.data = (ulong)uniphier_pxs2_sys_clk_data,
	},
	{
		.compatible = "socionext,uniphier-pro4-clock",
		.data = (ulong)uniphier_pxs2_sys_clk_data,
	},
	{
		.compatible = "socionext,uniphier-sld8-clock",
		.data = (ulong)uniphier_pxs2_sys_clk_data,
	},
	{
		.compatible = "socionext,uniphier-pro5-clock",
		.data = (ulong)uniphier_pxs2_sys_clk_data,
	},
	{
		.compatible = "socionext,uniphier-pxs2-clock",
		.data = (ulong)uniphier_pxs2_sys_clk_data,
	},
	{
		.compatible = "socionext,uniphier-ld11-clock",
		.data = (ulong)uniphier_ld20_sys_clk_data,
	},
	{
		.compatible = "socionext,uniphier-ld20-clock",
		.data = (ulong)uniphier_ld20_sys_clk_data,
	},
	{
		.compatible = "socionext,uniphier-pxs3-clock",
		.data = (ulong)uniphier_pxs3_sys_clk_data,
	},
	/* Media I/O clock */
	{
		.compatible = "socionext,uniphier-ld4-mio-clock",
		.data = (ulong)uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-pro4-mio-clock",
		.data = (ulong)uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-sld8-mio-clock",
		.data = (ulong)uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-pro5-sd-clock",
		.data = (ulong)uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-pxs2-sd-clock",
		.data = (ulong)uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-ld11-mio-clock",
		.data = (ulong)uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-ld20-sd-clock",
		.data = (ulong)uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-pxs3-sd-clock",
		.data = (ulong)uniphier_mio_clk_data,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_clk) = {
	.name = "uniphier-clk",
	.id = UCLASS_CLK,
	.of_match = uniphier_clk_match,
	.probe = uniphier_clk_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_clk_priv),
	.ops = &uniphier_clk_ops,
};
