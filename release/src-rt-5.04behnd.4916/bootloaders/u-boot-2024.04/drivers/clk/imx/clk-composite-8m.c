// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <linux/clk-provider.h>
#include <clk.h>
#include "clk.h"
#include <linux/err.h>

#define UBOOT_DM_CLK_IMX_COMPOSITE "imx_clk_composite"

#define PCG_PREDIV_SHIFT	16
#define PCG_PREDIV_WIDTH	3
#define PCG_PREDIV_MAX		8

#define PCG_DIV_SHIFT		0
#define PCG_DIV_WIDTH		6
#define PCG_DIV_MAX		64

#define PCG_PCS_SHIFT		24
#define PCG_PCS_MASK		0x7

#define PCG_CGC_SHIFT		28

static unsigned long imx8m_clk_composite_divider_recalc_rate(struct clk *clk)
{
	struct clk_divider *divider = (struct clk_divider *)to_clk_divider(clk);
	struct clk_composite *composite = (struct clk_composite *)clk->data;
	ulong parent_rate = clk_get_parent_rate(&composite->clk);
	unsigned long prediv_rate;
	unsigned int prediv_value;
	unsigned int div_value;

	debug("%s: name %s prate: %lu reg: %p\n", __func__,
	      (&composite->clk)->dev->name, parent_rate, divider->reg);
	prediv_value = readl(divider->reg) >> divider->shift;
	prediv_value &= clk_div_mask(divider->width);

	prediv_rate = divider_recalc_rate(clk, parent_rate, prediv_value,
					  NULL, divider->flags,
					  divider->width);

	div_value = readl(divider->reg) >> PCG_DIV_SHIFT;
	div_value &= clk_div_mask(PCG_DIV_WIDTH);

	return divider_recalc_rate(clk, prediv_rate, div_value, NULL,
				   divider->flags, PCG_DIV_WIDTH);
}

static int imx8m_clk_composite_compute_dividers(unsigned long rate,
						unsigned long parent_rate,
						int *prediv, int *postdiv)
{
	int div1, div2;
	int error = INT_MAX;
	int ret = -EINVAL;

	*prediv = 1;
	*postdiv = 1;

	for (div1 = 1; div1 <= PCG_PREDIV_MAX; div1++) {
		for (div2 = 1; div2 <= PCG_DIV_MAX; div2++) {
			int new_error = ((parent_rate / div1) / div2) - rate;

			if (abs(new_error) < abs(error)) {
				*prediv = div1;
				*postdiv = div2;
				error = new_error;
				ret = 0;
			}
		}
	}
	return ret;
}

/*
 * The clk are bound to a dev, because it is part of composite clk
 * use composite clk to get dev
 */
static ulong imx8m_clk_composite_divider_set_rate(struct clk *clk,
						  unsigned long rate)
{
	struct clk_divider *divider = (struct clk_divider *)to_clk_divider(clk);
	struct clk_composite *composite = (struct clk_composite *)clk->data;
	ulong parent_rate = clk_get_parent_rate(&composite->clk);
	int prediv_value;
	int div_value;
	int ret;
	u32 val;

	ret = imx8m_clk_composite_compute_dividers(rate, parent_rate,
						   &prediv_value, &div_value);
	if (ret)
		return ret;

	val = readl(divider->reg);
	val &= ~((clk_div_mask(divider->width) << divider->shift) |
			(clk_div_mask(PCG_DIV_WIDTH) << PCG_DIV_SHIFT));

	val |= (u32)(prediv_value  - 1) << divider->shift;
	val |= (u32)(div_value - 1) << PCG_DIV_SHIFT;
	writel(val, divider->reg);

	return clk_get_rate(&composite->clk);
}

static const struct clk_ops imx8m_clk_composite_divider_ops = {
	.get_rate = imx8m_clk_composite_divider_recalc_rate,
	.set_rate = imx8m_clk_composite_divider_set_rate,
};

struct clk *imx8m_clk_composite_flags(const char *name,
				      const char * const *parent_names,
				      int num_parents, void __iomem *reg,
				      unsigned long flags)
{
	struct clk *clk = ERR_PTR(-ENOMEM);
	struct clk_divider *div = NULL;
	struct clk_gate *gate = NULL;
	struct clk_mux *mux = NULL;

	mux = kzalloc(sizeof(*mux), GFP_KERNEL);
	if (!mux)
		goto fail;

	mux->reg = reg;
	mux->shift = PCG_PCS_SHIFT;
	mux->mask = PCG_PCS_MASK;
	mux->num_parents = num_parents;
	mux->flags = flags;
	mux->parent_names = parent_names;

	div = kzalloc(sizeof(*div), GFP_KERNEL);
	if (!div)
		goto fail;

	div->reg = reg;
	div->shift = PCG_PREDIV_SHIFT;
	div->width = PCG_PREDIV_WIDTH;
	div->flags = CLK_DIVIDER_ROUND_CLOSEST | flags;

	gate = kzalloc(sizeof(*gate), GFP_KERNEL);
	if (!gate)
		goto fail;

	gate->reg = reg;
	gate->bit_idx = PCG_CGC_SHIFT;
	gate->flags = flags;

	clk = clk_register_composite(NULL, name,
				     parent_names, num_parents,
				     &mux->clk, &clk_mux_ops, &div->clk,
				     &imx8m_clk_composite_divider_ops,
				     &gate->clk, &clk_gate_ops, flags);
	if (IS_ERR(clk))
		goto fail;

	return clk;

fail:
	kfree(gate);
	kfree(div);
	kfree(mux);
	return ERR_CAST(clk);
}
