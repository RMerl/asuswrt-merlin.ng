// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright 2012 Freescale Semiconductor, Inc.
 * Copyright 2012 Linaro Ltd.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <linux/clk-provider.h>
#include <div64.h>
#include <clk.h>
#include "clk.h"
#include <linux/err.h>

#define UBOOT_DM_CLK_IMX_PFD "imx_clk_pfd"

struct clk_pfd {
	struct clk	clk;
	void __iomem	*reg;
	u8		idx;
};

#define to_clk_pfd(_clk) container_of(_clk, struct clk_pfd, clk)

#define SET	0x4
#define CLR	0x8
#define OTG	0xc

static unsigned long clk_pfd_recalc_rate(struct clk *clk)
{
	struct clk_pfd *pfd =
		to_clk_pfd(dev_get_clk_ptr(clk->dev));
	unsigned long parent_rate = clk_get_parent_rate(clk);
	u64 tmp = parent_rate;
	u8 frac = (readl(pfd->reg) >> (pfd->idx * 8)) & 0x3f;

	tmp *= 18;
	do_div(tmp, frac);

	return tmp;
}

static unsigned long clk_pfd_set_rate(struct clk *clk, unsigned long rate)
{
	struct clk_pfd *pfd = to_clk_pfd(clk);
	unsigned long parent_rate = clk_get_parent_rate(clk);
	u64 tmp = parent_rate;
	u8 frac;

	tmp = tmp * 18 + rate / 2;
	do_div(tmp, rate);
	frac = tmp;
	if (frac < 12)
		frac = 12;
	else if (frac > 35)
		frac = 35;

	writel(0x3f << (pfd->idx * 8), pfd->reg + CLR);
	writel(frac << (pfd->idx * 8), pfd->reg + SET);

	return 0;
}

static const struct clk_ops clk_pfd_ops = {
	.get_rate	= clk_pfd_recalc_rate,
	.set_rate	= clk_pfd_set_rate,
};

struct clk *imx_clk_pfd(const char *name, const char *parent_name,
			void __iomem *reg, u8 idx)
{
	struct clk_pfd *pfd;
	struct clk *clk;
	int ret;

	pfd = kzalloc(sizeof(*pfd), GFP_KERNEL);
	if (!pfd)
		return ERR_PTR(-ENOMEM);

	pfd->reg = reg;
	pfd->idx = idx;

	/* register the clock */
	clk = &pfd->clk;

	ret = clk_register(clk, UBOOT_DM_CLK_IMX_PFD, name, parent_name);
	if (ret) {
		kfree(pfd);
		return ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(clk_pfd) = {
	.name	= UBOOT_DM_CLK_IMX_PFD,
	.id	= UCLASS_CLK,
	.ops	= &clk_pfd_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
