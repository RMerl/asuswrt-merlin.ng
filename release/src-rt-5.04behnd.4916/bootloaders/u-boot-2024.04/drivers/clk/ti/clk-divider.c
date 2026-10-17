// SPDX-License-Identifier: GPL-2.0+
/*
 * TI divider clock support
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 *
 * Loosely based on Linux kernel drivers/clk/ti/divider.c
 */

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <asm/io.h>
#include <linux/clk-provider.h>
#include <linux/kernel.h>
#include <linux/log2.h>
#include "clk.h"

/*
 * The reverse of DIV_ROUND_UP: The maximum number which
 * divided by m is r
 */
#define MULT_ROUND_UP(r, m) ((r) * (m) + (m) - 1)

struct clk_ti_divider_priv {
	struct clk parent;
	struct clk_ti_reg reg;
	const struct clk_div_table *table;
	u8 shift;
	u8 flags;
	u8 div_flags;
	s8 latch;
	u16 min;
	u16 max;
	u16 mask;
};

static unsigned int _get_div(const struct clk_div_table *table, ulong flags,
			     unsigned int val)
{
	if (flags & CLK_DIVIDER_ONE_BASED)
		return val;

	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		return 1 << val;

	if (table)
		return clk_divider_get_table_div(table, val);

	return val + 1;
}

static unsigned int _get_val(const struct clk_div_table *table, ulong flags,
			     unsigned int div)
{
	if (flags & CLK_DIVIDER_ONE_BASED)
		return div;

	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		return __ffs(div);

	if (table)
		return clk_divider_get_table_val(table, div);

	return div - 1;
}

static int _div_round_up(const struct clk_div_table *table, ulong parent_rate,
			 ulong rate)
{
	const struct clk_div_table *clkt;
	int up = INT_MAX;
	int div = DIV_ROUND_UP_ULL((u64)parent_rate, rate);

	for (clkt = table; clkt->div; clkt++) {
		if (clkt->div == div)
			return clkt->div;
		else if (clkt->div < div)
			continue;

		if ((clkt->div - div) < (up - div))
			up = clkt->div;
	}

	return up;
}

static int _div_round(const struct clk_div_table *table, ulong parent_rate,
		      ulong rate)
{
	if (table)
		return _div_round_up(table, parent_rate, rate);

	return DIV_ROUND_UP(parent_rate, rate);
}

static int clk_ti_divider_best_div(struct clk *clk, ulong rate,
				   ulong *best_parent_rate)
{
	struct clk_ti_divider_priv *priv = dev_get_priv(clk->dev);
	ulong parent_rate, parent_round_rate, max_div;
	ulong best_rate, r;
	int i, best_div = 0;

	parent_rate = clk_get_rate(&priv->parent);
	if (IS_ERR_VALUE(parent_rate))
		return parent_rate;

	if (!rate)
		rate = 1;

	if (!(clk->flags & CLK_SET_RATE_PARENT)) {
		best_div = _div_round(priv->table, parent_rate, rate);
		if (best_div == 0)
			best_div = 1;

		if (best_div > priv->max)
			best_div = priv->max;

		*best_parent_rate = parent_rate;
		return best_div;
	}

	max_div = min(ULONG_MAX / rate, (ulong)priv->max);
	for (best_rate = 0, i = 1; i <= max_div; i++) {
		if (!clk_divider_is_valid_div(priv->table, priv->div_flags, i))
			continue;

		/*
		 * It's the most ideal case if the requested rate can be
		 * divided from parent clock without needing to change
		 * parent rate, so return the divider immediately.
		 */
		if ((rate * i) == parent_rate) {
			*best_parent_rate = parent_rate;
			dev_dbg(clk->dev, "rate=%ld, best_rate=%ld, div=%d\n",
				rate, rate, i);
			return i;
		}

		parent_round_rate = clk_round_rate(&priv->parent,
						   MULT_ROUND_UP(rate, i));
		if (IS_ERR_VALUE(parent_round_rate))
			continue;

		r = DIV_ROUND_UP(parent_round_rate, i);
		if (r <= rate && r > best_rate) {
			best_div = i;
			best_rate = r;
			*best_parent_rate = parent_round_rate;
			if (best_rate == rate)
				break;
		}
	}

	if (best_div == 0) {
		best_div = priv->max;
		parent_round_rate = clk_round_rate(&priv->parent, 1);
		if (IS_ERR_VALUE(parent_round_rate))
			return parent_round_rate;
	}

	dev_dbg(clk->dev, "rate=%ld, best_rate=%ld, div=%d\n", rate, best_rate,
		best_div);

	return best_div;
}

static ulong clk_ti_divider_round_rate(struct clk *clk, ulong rate)
{
	ulong parent_rate;
	int div;

	div = clk_ti_divider_best_div(clk, rate, &parent_rate);
	if (div < 0)
		return div;

	return DIV_ROUND_UP(parent_rate, div);
}

static ulong clk_ti_divider_set_rate(struct clk *clk, ulong rate)
{
	struct clk_ti_divider_priv *priv = dev_get_priv(clk->dev);
	ulong parent_rate;
	int div;
	u32 val, v;

	div = clk_ti_divider_best_div(clk, rate, &parent_rate);
	if (div < 0)
		return div;

	if (clk->flags & CLK_SET_RATE_PARENT) {
		parent_rate = clk_set_rate(&priv->parent, parent_rate);
		if (IS_ERR_VALUE(parent_rate))
			return parent_rate;
	}

	val = _get_val(priv->table, priv->div_flags, div);

	v = clk_ti_readl(&priv->reg);
	v &= ~(priv->mask << priv->shift);
	v |= val << priv->shift;
	clk_ti_writel(v, &priv->reg);
	clk_ti_latch(&priv->reg, priv->latch);

	return clk_get_rate(clk);
}

static ulong clk_ti_divider_get_rate(struct clk *clk)
{
	struct clk_ti_divider_priv *priv = dev_get_priv(clk->dev);
	ulong rate, parent_rate;
	unsigned int div;
	u32 v;

	parent_rate = clk_get_rate(&priv->parent);
	if (IS_ERR_VALUE(parent_rate))
		return parent_rate;

	v = clk_ti_readl(&priv->reg) >> priv->shift;
	v &= priv->mask;

	div = _get_div(priv->table, priv->div_flags, v);
	if (!div) {
		if (!(priv->div_flags & CLK_DIVIDER_ALLOW_ZERO))
			dev_warn(clk->dev,
				 "zero divisor and CLK_DIVIDER_ALLOW_ZERO not set\n");
		return parent_rate;
	}

	rate = DIV_ROUND_UP(parent_rate, div);
	dev_dbg(clk->dev, "rate=%ld\n", rate);
	return rate;
}

static int clk_ti_divider_request(struct clk *clk)
{
	struct clk_ti_divider_priv *priv = dev_get_priv(clk->dev);

	clk->flags = priv->flags;
	return 0;
}

const struct clk_ops clk_ti_divider_ops = {
	.request = clk_ti_divider_request,
	.round_rate = clk_ti_divider_round_rate,
	.get_rate = clk_ti_divider_get_rate,
	.set_rate = clk_ti_divider_set_rate
};

static int clk_ti_divider_remove(struct udevice *dev)
{
	struct clk_ti_divider_priv *priv = dev_get_priv(dev);
	int err;

	err = clk_release_all(&priv->parent, 1);
	if (err) {
		dev_err(dev, "failed to release parent clock\n");
		return err;
	}

	return 0;
}

static int clk_ti_divider_probe(struct udevice *dev)
{
	struct clk_ti_divider_priv *priv = dev_get_priv(dev);
	int err;

	err = clk_get_by_index(dev, 0, &priv->parent);
	if (err) {
		dev_err(dev, "failed to get parent clock\n");
		return err;
	}

	return 0;
}

static int clk_ti_divider_of_to_plat(struct udevice *dev)
{
	struct clk_ti_divider_priv *priv = dev_get_priv(dev);
	struct clk_div_table *table = NULL;
	u32 val, valid_div;
	u32 min_div = 0;
	u32 max_val, max_div = 0;
	u16 mask;
	int i, div_num, err;

	err = clk_ti_get_reg_addr(dev, 0, &priv->reg);
	if (err) {
		dev_err(dev, "failed to get register address\n");
		return err;
	}

	priv->shift = dev_read_u32_default(dev, "ti,bit-shift", 0);
	priv->latch = dev_read_s32_default(dev, "ti,latch-bit", -EINVAL);
	if (dev_read_bool(dev, "ti,index-starts-at-one"))
		priv->div_flags |= CLK_DIVIDER_ONE_BASED;

	if (dev_read_bool(dev, "ti,index-power-of-two"))
		priv->div_flags |= CLK_DIVIDER_POWER_OF_TWO;

	if (dev_read_bool(dev, "ti,set-rate-parent"))
		priv->flags |= CLK_SET_RATE_PARENT;

	if (dev_read_prop(dev, "ti,dividers", &div_num)) {
		div_num /= sizeof(u32);

		/* Determine required size for divider table */
		for (i = 0, valid_div = 0; i < div_num; i++) {
			dev_read_u32_index(dev, "ti,dividers", i, &val);
			if (val)
				valid_div++;
		}

		if (!valid_div) {
			dev_err(dev, "no valid dividers\n");
			return -EINVAL;
		}

		table = calloc(valid_div + 1, sizeof(*table));
		if (!table)
			return -ENOMEM;

		for (i = 0, valid_div = 0; i < div_num; i++) {
			dev_read_u32_index(dev, "ti,dividers", i, &val);
			if (!val)
				continue;

			table[valid_div].div = val;
			table[valid_div].val = i;
			valid_div++;
			if (val > max_div)
				max_div = val;

			if (!min_div || val < min_div)
				min_div = val;
		}

		max_val = max_div;
	} else {
		/* Divider table not provided, determine min/max divs */
		min_div = dev_read_u32_default(dev, "ti,min-div", 1);
		if (dev_read_u32(dev, "ti,max-div", &max_div)) {
			dev_err(dev, "missing 'max-div' property\n");
			return -EFAULT;
		}

		max_val = max_div;
		if (!(priv->div_flags & CLK_DIVIDER_ONE_BASED) &&
		    !(priv->div_flags & CLK_DIVIDER_POWER_OF_TWO))
			max_val--;
	}

	priv->table = table;
	priv->min = min_div;
	priv->max = max_div;

	if (priv->div_flags & CLK_DIVIDER_POWER_OF_TWO)
		mask = fls(max_val) - 1;
	else
		mask = max_val;

	priv->mask = (1 << fls(mask)) - 1;
	return 0;
}

static const struct udevice_id clk_ti_divider_of_match[] = {
	{.compatible = "ti,divider-clock"},
	{}
};

U_BOOT_DRIVER(clk_ti_divider) = {
	.name = "ti_divider_clock",
	.id = UCLASS_CLK,
	.of_match = clk_ti_divider_of_match,
	.of_to_plat = clk_ti_divider_of_to_plat,
	.probe = clk_ti_divider_probe,
	.remove = clk_ti_divider_remove,
	.priv_auto = sizeof(struct clk_ti_divider_priv),
	.ops = &clk_ti_divider_ops,
};
