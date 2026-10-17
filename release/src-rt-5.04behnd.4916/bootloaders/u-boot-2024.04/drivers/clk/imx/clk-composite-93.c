// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */
#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <linux/iopoll.h>
#include <linux/clk-provider.h>
#include <clk.h>
#include "clk.h"
#include <linux/err.h>

#define TIMEOUT_US	500U

#define CCM_DIV_SHIFT	0
#define CCM_DIV_WIDTH	8
#define CCM_MUX_SHIFT	8
#define CCM_MUX_MASK	3
#define CCM_OFF_SHIFT	24
#define CCM_BUSY_SHIFT	28

#define STAT_OFFSET	0x4
#define AUTHEN_OFFSET	0x30
#define TZ_NS_SHIFT	9
#define TZ_NS_MASK	BIT(9)

#define WHITE_LIST_SHIFT	16

#define readl_poll_timeout_atomic readl_poll_timeout

static int imx93_clk_composite_wait_ready(struct clk *clk, void __iomem *reg)
{
	int ret;
	u32 val;

	ret = readl_poll_timeout_atomic(reg + STAT_OFFSET, val, !(val & BIT(CCM_BUSY_SHIFT)),
					TIMEOUT_US);
	if (ret)
		pr_err("Slice[%s] busy timeout\n", "TODO");

	return ret;
}

static void imx93_clk_composite_gate_endisable(struct clk *clk, int enable)
{
	struct clk_gate *gate = to_clk_gate(clk);
	u32 reg;

	reg = readl(gate->reg);

	if (enable)
		reg &= ~BIT(gate->bit_idx);
	else
		reg |= BIT(gate->bit_idx);

	writel(reg, gate->reg);

	imx93_clk_composite_wait_ready(clk, gate->reg);
}

static int imx93_clk_composite_gate_enable(struct clk *clk)
{
	imx93_clk_composite_gate_endisable(clk, 1);

	return 0;
}

static int imx93_clk_composite_gate_disable(struct clk *clk)
{
	imx93_clk_composite_gate_endisable(clk, 0);

	return 0;
}

static const struct clk_ops imx93_clk_composite_gate_ops = {
	.enable = imx93_clk_composite_gate_enable,
	.disable = imx93_clk_composite_gate_disable,
};

struct clk *imx93_clk_composite_flags(const char *name,
				      const char * const *parent_names,
				      int num_parents, void __iomem *reg, u32 domain_id,
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
	mux->shift = CCM_MUX_SHIFT;
	mux->mask = CCM_MUX_MASK;
	mux->num_parents = num_parents;
	mux->parent_names = parent_names;
	mux->flags = flags;

	div = kzalloc(sizeof(*div), GFP_KERNEL);
	if (!div)
		goto fail;

	div->reg = reg;
	div->shift = CCM_DIV_SHIFT;
	div->width = CCM_DIV_WIDTH;
	div->flags = CLK_DIVIDER_ROUND_CLOSEST | flags;

	gate = kzalloc(sizeof(*gate), GFP_KERNEL);
	if (!gate)
		goto fail;

	gate->reg = reg;
	gate->bit_idx = CCM_OFF_SHIFT;
	gate->flags = flags;

	clk = clk_register_composite(NULL, name,
				     parent_names, num_parents,
				     &mux->clk, &clk_mux_ops,
				     &div->clk, &clk_divider_ops,
				     &gate->clk, &imx93_clk_composite_gate_ops,
				     flags);

	if (IS_ERR(clk))
		goto fail;

	return clk;

fail:
	kfree(gate);
	kfree(div);
	kfree(mux);
	return ERR_CAST(clk);
}
