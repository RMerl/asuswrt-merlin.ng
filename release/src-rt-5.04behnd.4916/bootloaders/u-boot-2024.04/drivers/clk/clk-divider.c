// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright (C) 2011 Sascha Hauer, Pengutronix <s.hauer@pengutronix.de>
 * Copyright (C) 2011 Richard Zhao, Linaro <richard.zhao@linaro.org>
 * Copyright (C) 2011-2012 Mike Turquette, Linaro Ltd <mturquette@linaro.org>
 *
 */

#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <log.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <dm/uclass.h>
#include <dm/lists.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <linux/bug.h>
#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/log2.h>
#include <div64.h>
#include <clk.h>
#include <linux/printk.h>
#include "clk.h"

#define UBOOT_DM_CLK_CCF_DIVIDER "ccf_clk_divider"

unsigned int clk_divider_get_table_div(const struct clk_div_table *table,
				       unsigned int val)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->val == val)
			return clkt->div;
	return 0;
}

static unsigned int _get_div(const struct clk_div_table *table,
			     unsigned int val, unsigned long flags, u8 width)
{
	if (flags & CLK_DIVIDER_ONE_BASED)
		return val;
	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		return 1 << val;
	if (flags & CLK_DIVIDER_MAX_AT_ZERO)
		return val ? val : clk_div_mask(width) + 1;
	if (table)
		return clk_divider_get_table_div(table, val);
	return val + 1;
}

unsigned long divider_recalc_rate(struct clk *hw, unsigned long parent_rate,
				  unsigned int val,
				  const struct clk_div_table *table,
				  unsigned long flags, unsigned long width)
{
	unsigned int div;

	div = _get_div(table, val, flags, width);
	if (!div) {
		WARN(!(flags & CLK_DIVIDER_ALLOW_ZERO),
		     "%s: Zero divisor and CLK_DIVIDER_ALLOW_ZERO not set\n",
		     clk_hw_get_name(hw));
		return parent_rate;
	}

	return DIV_ROUND_UP_ULL((u64)parent_rate, div);
}

static ulong clk_divider_recalc_rate(struct clk *clk)
{
	struct clk_divider *divider = to_clk_divider(clk);
	unsigned long parent_rate = clk_get_parent_rate(clk);
	unsigned int val;

#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
	val = divider->io_divider_val;
#else
	val = readl(divider->reg);
#endif
	val >>= divider->shift;
	val &= clk_div_mask(divider->width);

	return divider_recalc_rate(clk, parent_rate, val, divider->table,
				   divider->flags, divider->width);
}

bool clk_divider_is_valid_table_div(const struct clk_div_table *table,
				    unsigned int div)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div == div)
			return true;
	return false;
}

bool clk_divider_is_valid_div(const struct clk_div_table *table,
			      unsigned int div, unsigned long flags)
{
	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		return is_power_of_2(div);
	if (table)
		return clk_divider_is_valid_table_div(table, div);
	return true;
}

unsigned int clk_divider_get_table_val(const struct clk_div_table *table,
				       unsigned int div)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div == div)
			return clkt->val;
	return 0;
}

static unsigned int _get_val(const struct clk_div_table *table,
			     unsigned int div, unsigned long flags, u8 width)
{
	if (flags & CLK_DIVIDER_ONE_BASED)
		return div;
	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		return __ffs(div);
	if (flags & CLK_DIVIDER_MAX_AT_ZERO)
		return (div == clk_div_mask(width) + 1) ? 0 : div;
	if (table)
		return clk_divider_get_table_val(table, div);
	return div - 1;
}
int divider_get_val(unsigned long rate, unsigned long parent_rate,
		    const struct clk_div_table *table, u8 width,
		    unsigned long flags)
{
	unsigned int div, value;

	div = DIV_ROUND_UP_ULL((u64)parent_rate, rate);

	if (!clk_divider_is_valid_div(table, div, flags))
		return -EINVAL;

	value = _get_val(table, div, flags, width);

	return min_t(unsigned int, value, clk_div_mask(width));
}

static ulong clk_divider_set_rate(struct clk *clk, unsigned long rate)
{
	struct clk_divider *divider = to_clk_divider(clk);
	unsigned long parent_rate = clk_get_parent_rate(clk);
	int value;
	u32 val;

	value = divider_get_val(rate, parent_rate, divider->table,
				divider->width, divider->flags);
	if (value < 0)
		return value;

	if (divider->flags & CLK_DIVIDER_HIWORD_MASK) {
		val = clk_div_mask(divider->width) << (divider->shift + 16);
	} else {
		val = readl(divider->reg);
		val &= ~(clk_div_mask(divider->width) << divider->shift);
	}
	val |= (u32)value << divider->shift;
	writel(val, divider->reg);

	return clk_get_rate(clk);
}

const struct clk_ops clk_divider_ops = {
	.get_rate = clk_divider_recalc_rate,
	.set_rate = clk_divider_set_rate,
};

static struct clk *_register_divider(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_divider_flags, const struct clk_div_table *table)
{
	struct clk_divider *div;
	struct clk *clk;
	int ret;

	if (clk_divider_flags & CLK_DIVIDER_HIWORD_MASK) {
		if (width + shift > 16) {
			dev_warn(dev, "divider value exceeds LOWORD field\n");
			return ERR_PTR(-EINVAL);
		}
	}

	/* allocate the divider */
	div = kzalloc(sizeof(*div), GFP_KERNEL);
	if (!div)
		return ERR_PTR(-ENOMEM);

	/* struct clk_divider assignments */
	div->reg = reg;
	div->shift = shift;
	div->width = width;
	div->flags = clk_divider_flags;
	div->table = table;
#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
	div->io_divider_val = *(u32 *)reg;
#endif

	/* register the clock */
	clk = &div->clk;
	clk->flags = flags;

	ret = clk_register(clk, UBOOT_DM_CLK_CCF_DIVIDER, name, parent_name);
	if (ret) {
		kfree(div);
		return ERR_PTR(ret);
	}

	return clk;
}

struct clk *clk_register_divider(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_divider_flags)
{
	struct clk *clk;

	clk =  _register_divider(dev, name, parent_name, flags, reg, shift,
				 width, clk_divider_flags, NULL);
	if (IS_ERR(clk))
		return ERR_CAST(clk);
	return clk;
}

U_BOOT_DRIVER(ccf_clk_divider) = {
	.name	= UBOOT_DM_CLK_CCF_DIVIDER,
	.id	= UCLASS_CLK,
	.ops	= &clk_divider_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
