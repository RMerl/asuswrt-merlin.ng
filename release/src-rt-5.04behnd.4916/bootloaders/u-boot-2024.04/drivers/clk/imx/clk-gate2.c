// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright (C) 2010-2011 Canonical Ltd <jeremy.kerr@canonical.com>
 * Copyright (C) 2011-2012 Mike Turquette, Linaro Ltd <mturquette@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Gated clock implementation
 *
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <linux/bug.h>
#include <linux/clk-provider.h>
#include <clk.h>
#include "clk.h"
#include <linux/err.h>

#define UBOOT_DM_CLK_IMX_GATE2 "imx_clk_gate2"

struct clk_gate2 {
	struct clk clk;
	void __iomem	*reg;
	u8		bit_idx;
	u8		cgr_val;
	u8		flags;
	unsigned int	*share_count;
};

#define to_clk_gate2(_clk) container_of(_clk, struct clk_gate2, clk)

static int clk_gate2_enable(struct clk *clk)
{
	struct clk_gate2 *gate = to_clk_gate2(clk);
	u32 reg;

	if (gate->share_count && (*gate->share_count)++ > 0)
		return 0;

	reg = readl(gate->reg);
	reg &= ~(3 << gate->bit_idx);
	reg |= gate->cgr_val << gate->bit_idx;
	writel(reg, gate->reg);

	return 0;
}

static int clk_gate2_disable(struct clk *clk)
{
	struct clk_gate2 *gate = to_clk_gate2(clk);
	u32 reg;

	if (gate->share_count) {
		if (WARN_ON(*gate->share_count == 0))
			return 0;
		else if (--(*gate->share_count) > 0)
			return 0;
	}

	reg = readl(gate->reg);
	reg &= ~(3 << gate->bit_idx);
	writel(reg, gate->reg);

	return 0;
}

static ulong clk_gate2_set_rate(struct clk *clk, ulong rate)
{
	struct clk *parent = clk_get_parent(clk);

	if (parent)
		return clk_set_rate(parent, rate);

	return -ENODEV;
}

static const struct clk_ops clk_gate2_ops = {
	.set_rate = clk_gate2_set_rate,
	.enable = clk_gate2_enable,
	.disable = clk_gate2_disable,
	.get_rate = clk_generic_get_rate,
};

struct clk *clk_register_gate2(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 bit_idx, u8 cgr_val,
		u8 clk_gate2_flags, unsigned int *share_count)
{
	struct clk_gate2 *gate;
	struct clk *clk;
	int ret;

	gate = kzalloc(sizeof(*gate), GFP_KERNEL);
	if (!gate)
		return ERR_PTR(-ENOMEM);

	gate->reg = reg;
	gate->bit_idx = bit_idx;
	gate->cgr_val = cgr_val;
	gate->flags = clk_gate2_flags;
	gate->share_count = share_count;

	clk = &gate->clk;

	ret = clk_register(clk, UBOOT_DM_CLK_IMX_GATE2, name, parent_name);
	if (ret) {
		kfree(gate);
		return ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(clk_gate2) = {
	.name	= UBOOT_DM_CLK_IMX_GATE2,
	.id	= UCLASS_CLK,
	.ops	= &clk_gate2_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
