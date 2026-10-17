// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Samsung Electronics
 * Copyright (C) 2023 Linaro Ltd.
 *
 * Authors:
 *   Thomas Abraham <thomas.ab@samsung.com>
 *   Sam Protsenko <semen.protsenko@linaro.org>
 *
 * This file contains the utility functions to register the pll clocks.
 */

#include <asm/io.h>
#include <div64.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <clk.h>
#include "clk.h"

#define UBOOT_DM_CLK_SAMSUNG_PLL0822X	"samsung_clk_pll0822x"
#define UBOOT_DM_CLK_SAMSUNG_PLL0831X	"samsung_clk_pll0831x"

struct samsung_clk_pll {
	struct clk		clk;
	void __iomem		*con_reg;
	enum samsung_pll_type	type;
};

#define to_clk_pll(_clk) container_of(_clk, struct samsung_clk_pll, clk)

/*
 * PLL0822x Clock Type
 */

#define PLL0822X_MDIV_MASK		0x3ff
#define PLL0822X_PDIV_MASK		0x3f
#define PLL0822X_SDIV_MASK		0x7
#define PLL0822X_MDIV_SHIFT		16
#define PLL0822X_PDIV_SHIFT		8
#define PLL0822X_SDIV_SHIFT		0

static unsigned long samsung_pll0822x_recalc_rate(struct clk *clk)
{
	struct samsung_clk_pll *pll = to_clk_pll(clk);
	u32 mdiv, pdiv, sdiv, pll_con3;
	u64 fvco = clk_get_parent_rate(clk);

	pll_con3 = readl_relaxed(pll->con_reg);
	mdiv = (pll_con3 >> PLL0822X_MDIV_SHIFT) & PLL0822X_MDIV_MASK;
	pdiv = (pll_con3 >> PLL0822X_PDIV_SHIFT) & PLL0822X_PDIV_MASK;
	sdiv = (pll_con3 >> PLL0822X_SDIV_SHIFT) & PLL0822X_SDIV_MASK;

	fvco *= mdiv;
	do_div(fvco, (pdiv << sdiv));
	return (unsigned long)fvco;
}

static const struct clk_ops samsung_pll0822x_clk_min_ops = {
	.get_rate = samsung_pll0822x_recalc_rate,
};

/*
 * PLL0831x Clock Type
 */

#define PLL0831X_KDIV_MASK		0xffff
#define PLL0831X_MDIV_MASK		0x1ff
#define PLL0831X_PDIV_MASK		0x3f
#define PLL0831X_SDIV_MASK		0x7
#define PLL0831X_MDIV_SHIFT		16
#define PLL0831X_PDIV_SHIFT		8
#define PLL0831X_SDIV_SHIFT		0
#define PLL0831X_KDIV_SHIFT		0

static unsigned long samsung_pll0831x_recalc_rate(struct clk *clk)
{
	struct samsung_clk_pll *pll = to_clk_pll(clk);
	u32 mdiv, pdiv, sdiv, pll_con3, pll_con5;
	s16 kdiv;
	u64 fvco = clk_get_parent_rate(clk);

	pll_con3 = readl_relaxed(pll->con_reg);
	pll_con5 = readl_relaxed(pll->con_reg + 8);
	mdiv = (pll_con3 >> PLL0831X_MDIV_SHIFT) & PLL0831X_MDIV_MASK;
	pdiv = (pll_con3 >> PLL0831X_PDIV_SHIFT) & PLL0831X_PDIV_MASK;
	sdiv = (pll_con3 >> PLL0831X_SDIV_SHIFT) & PLL0831X_SDIV_MASK;
	kdiv = (s16)((pll_con5 >> PLL0831X_KDIV_SHIFT) & PLL0831X_KDIV_MASK);

	fvco *= (mdiv << 16) + kdiv;
	do_div(fvco, (pdiv << sdiv));
	fvco >>= 16;

	return (unsigned long)fvco;
}

static const struct clk_ops samsung_pll0831x_clk_min_ops = {
	.get_rate = samsung_pll0831x_recalc_rate,
};

static struct clk *_samsung_clk_register_pll(void __iomem *base,
					const struct samsung_pll_clock *pll_clk)
{
	struct samsung_clk_pll *pll;
	struct clk *clk;
	const char *drv_name;
	int ret;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	pll->con_reg = base + pll_clk->con_offset;
	pll->type = pll_clk->type;
	clk = &pll->clk;
	clk->flags = pll_clk->flags;

	switch (pll_clk->type) {
	case pll_0822x:
		drv_name = UBOOT_DM_CLK_SAMSUNG_PLL0822X;
		break;
	case pll_0831x:
		drv_name = UBOOT_DM_CLK_SAMSUNG_PLL0831X;
		break;
	default:
		kfree(pll);
		return ERR_PTR(-ENODEV);
	}

	ret = clk_register(clk, drv_name, pll_clk->name, pll_clk->parent_name);
	if (ret) {
		kfree(pll);
		return ERR_PTR(ret);
	}

	return clk;
}

void samsung_clk_register_pll(void __iomem *base,
			      const struct samsung_pll_clock *clk_list,
			      unsigned int nr_clk)
{
	unsigned int cnt;

	for (cnt = 0; cnt < nr_clk; cnt++) {
		struct clk *clk;
		const struct samsung_pll_clock *pll_clk;

		pll_clk = &clk_list[cnt];
		clk = _samsung_clk_register_pll(base, pll_clk);
		clk_dm(pll_clk->id, clk);
	}
}

U_BOOT_DRIVER(samsung_pll0822x_clk) = {
	.name	= UBOOT_DM_CLK_SAMSUNG_PLL0822X,
	.id	= UCLASS_CLK,
	.ops	= &samsung_pll0822x_clk_min_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(samsung_pll0831x_clk) = {
	.name	= UBOOT_DM_CLK_SAMSUNG_PLL0831X,
	.id	= UCLASS_CLK,
	.ops	= &samsung_pll0831x_clk_min_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};
