// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright (C) 2011 Sascha Hauer, Pengutronix <s.hauer@pengutronix.de>
 */

#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <div64.h>
#include <log.h>
#include <malloc.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <linux/clk-provider.h>
#include <linux/err.h>

#include "clk.h"

#define UBOOT_DM_CLK_IMX_FIXED_FACTOR "ccf_clk_fixed_factor"

static ulong clk_factor_recalc_rate(struct clk *clk)
{
	struct clk_fixed_factor *fix = to_clk_fixed_factor(clk);
	unsigned long parent_rate = clk_get_parent_rate(clk);
	unsigned long long int rate;

	rate = (unsigned long long int)parent_rate * fix->mult;
	do_div(rate, fix->div);
	return (ulong)rate;
}

const struct clk_ops ccf_clk_fixed_factor_ops = {
	.get_rate = clk_factor_recalc_rate,
};

struct clk *clk_hw_register_fixed_factor(struct device *dev,
		const char *name, const char *parent_name, unsigned long flags,
		unsigned int mult, unsigned int div)
{
	struct clk_fixed_factor *fix;
	struct clk *clk;
	int ret;

	fix = kzalloc(sizeof(*fix), GFP_KERNEL);
	if (!fix)
		return ERR_PTR(-ENOMEM);

	/* struct clk_fixed_factor assignments */
	fix->mult = mult;
	fix->div = div;
	clk = &fix->clk;
	clk->flags = flags;

	ret = clk_register(clk, UBOOT_DM_CLK_IMX_FIXED_FACTOR, name,
			   parent_name);
	if (ret) {
		kfree(fix);
		return ERR_PTR(ret);
	}

	return clk;
}

struct clk *clk_register_fixed_factor(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		unsigned int mult, unsigned int div)
{
	struct clk *clk;

	clk = clk_hw_register_fixed_factor(dev, name, parent_name, flags, mult,
					  div);
	if (IS_ERR(clk))
		return ERR_CAST(clk);
	return clk;
}

U_BOOT_DRIVER(imx_clk_fixed_factor) = {
	.name	= UBOOT_DM_CLK_IMX_FIXED_FACTOR,
	.id	= UCLASS_CLK,
	.ops	= &ccf_clk_fixed_factor_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
