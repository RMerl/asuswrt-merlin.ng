// SPDX-License-Identifier: GPL-2.0+
/*
 * Master clock support for AT91 architectures.
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 *
 * Based on drivers/clk/at91/clk-master.c from Linux.
 */

#include <asm/processor.h>
#include <clk-uclass.h>
#include <common.h>
#include <div64.h>
#include <dm.h>
#include <linux/clk-provider.h>
#include <linux/clk/at91_pmc.h>

#include "pmc.h"

#define UBOOT_DM_CLK_AT91_MASTER_PRES		"at91-master-clk-pres"
#define UBOOT_DM_CLK_AT91_MASTER_DIV		"at91-master-clk-div"
#define UBOOT_DM_CLK_AT91_SAMA7G5_MASTER	"at91-sama7g5-master-clk"

#define MASTER_PRES_MASK	0x7
#define MASTER_PRES_MAX		MASTER_PRES_MASK
#define MASTER_DIV_SHIFT	8
#define MASTER_DIV_MASK		0x7

#define PMC_MCR			0x30
#define PMC_MCR_ID_MSK		GENMASK(3, 0)
#define PMC_MCR_CMD		BIT(7)
#define PMC_MCR_DIV		GENMASK(10, 8)
#define PMC_MCR_CSS		GENMASK(20, 16)
#define PMC_MCR_CSS_SHIFT	(16)
#define PMC_MCR_EN		BIT(28)

#define PMC_MCR_ID(x)		((x) & PMC_MCR_ID_MSK)

#define MASTER_MAX_ID		4

struct clk_master {
	void __iomem *base;
	const struct clk_master_layout *layout;
	const struct clk_master_characteristics *characteristics;
	const u32 *mux_table;
	const u32 *clk_mux_table;
	u32 num_parents;
	struct clk clk;
	u8 id;
};

#define to_clk_master(_clk) container_of(_clk, struct clk_master, clk)

static inline bool clk_master_ready(struct clk_master *master)
{
	unsigned int bit = master->id ? AT91_PMC_MCKXRDY : AT91_PMC_MCKRDY;
	unsigned int status;

	pmc_read(master->base, AT91_PMC_SR, &status);

	return !!(status & bit);
}

static int clk_master_enable(struct clk *clk)
{
	struct clk_master *master = to_clk_master(clk);

	while (!clk_master_ready(master)) {
		debug("waiting for mck %d\n", master->id);
		cpu_relax();
	}

	return 0;
}

static ulong clk_master_pres_get_rate(struct clk *clk)
{
	struct clk_master *master = to_clk_master(clk);
	const struct clk_master_layout *layout = master->layout;
	const struct clk_master_characteristics *characteristics =
						master->characteristics;
	ulong rate = clk_get_parent_rate(clk);
	unsigned int mckr;
	u8 pres;

	if (!rate)
		return 0;

	pmc_read(master->base, master->layout->offset, &mckr);
	mckr &= layout->mask;

	pres = (mckr >> layout->pres_shift) & MASTER_PRES_MASK;

	if (characteristics->have_div3_pres && pres == MASTER_PRES_MAX)
		pres = 3;
	else
		pres = (1 << pres);

	return DIV_ROUND_CLOSEST_ULL(rate, pres);
}

static const struct clk_ops master_pres_ops = {
	.enable = clk_master_enable,
	.get_rate = clk_master_pres_get_rate,
};

struct clk *at91_clk_register_master_pres(void __iomem *base,
		const char *name, const char * const *parent_names,
		int num_parents, const struct clk_master_layout *layout,
		const struct clk_master_characteristics *characteristics,
		const u32 *mux_table)
{
	struct clk_master *master;
	struct clk *clk;
	unsigned int val;
	int ret;

	if (!base || !name || !num_parents || !parent_names ||
	    !layout || !characteristics || !mux_table)
		return ERR_PTR(-EINVAL);

	master = kzalloc(sizeof(*master), GFP_KERNEL);
	if (!master)
		return ERR_PTR(-ENOMEM);

	master->layout = layout;
	master->characteristics = characteristics;
	master->base = base;
	master->num_parents = num_parents;
	master->mux_table = mux_table;

	pmc_read(master->base, master->layout->offset, &val);
	clk = &master->clk;
	clk->flags = CLK_GET_RATE_NOCACHE | CLK_IS_CRITICAL;
	ret = clk_register(clk, UBOOT_DM_CLK_AT91_MASTER_PRES, name,
			   parent_names[val & AT91_PMC_CSS]);
	if (ret) {
		kfree(master);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_master_pres_clk) = {
	.name = UBOOT_DM_CLK_AT91_MASTER_PRES,
	.id = UCLASS_CLK,
	.ops = &master_pres_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static ulong clk_master_div_get_rate(struct clk *clk)
{
	struct clk_master *master = to_clk_master(clk);
	const struct clk_master_layout *layout = master->layout;
	const struct clk_master_characteristics *characteristics =
						master->characteristics;
	ulong rate = clk_get_parent_rate(clk);
	unsigned int mckr;
	u8 div;

	if (!rate)
		return 0;

	pmc_read(master->base, master->layout->offset, &mckr);
	mckr &= layout->mask;
	div = (mckr >> MASTER_DIV_SHIFT) & MASTER_DIV_MASK;

	rate = DIV_ROUND_CLOSEST_ULL(rate, characteristics->divisors[div]);
	if (rate < characteristics->output.min)
		pr_warn("master clk is underclocked");
	else if (rate > characteristics->output.max)
		pr_warn("master clk is overclocked");

	return rate;
}

static const struct clk_ops master_div_ops = {
	.enable = clk_master_enable,
	.get_rate = clk_master_div_get_rate,
};

struct clk *at91_clk_register_master_div(void __iomem *base,
		const char *name, const char *parent_name,
		const struct clk_master_layout *layout,
		const struct clk_master_characteristics *characteristics)
{
	struct clk_master *master;
	struct clk *clk;
	int ret;

	if (!base || !name || !parent_name || !layout || !characteristics)
		return ERR_PTR(-EINVAL);

	master = kzalloc(sizeof(*master), GFP_KERNEL);
	if (!master)
		return ERR_PTR(-ENOMEM);

	master->layout = layout;
	master->characteristics = characteristics;
	master->base = base;
	master->num_parents = 1;

	clk = &master->clk;
	clk->flags = CLK_GET_RATE_NOCACHE | CLK_IS_CRITICAL;
	ret = clk_register(clk, UBOOT_DM_CLK_AT91_MASTER_DIV, name,
			   parent_name);
	if (ret) {
		kfree(master);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_master_div_clk) = {
	.name = UBOOT_DM_CLK_AT91_MASTER_DIV,
	.id = UCLASS_CLK,
	.ops = &master_div_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static int clk_sama7g5_master_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk_master *master = to_clk_master(clk);
	int index;

	index = at91_clk_mux_val_to_index(master->clk_mux_table,
					  master->num_parents, parent->id);
	if (index < 0)
		return index;

	index = at91_clk_mux_index_to_val(master->mux_table,
					  master->num_parents, index);
	if (index < 0)
		return index;

	pmc_write(master->base, PMC_MCR, PMC_MCR_ID(master->id));
	pmc_update_bits(master->base, PMC_MCR,
			PMC_MCR_CSS | PMC_MCR_CMD | PMC_MCR_ID_MSK,
			(index << PMC_MCR_CSS_SHIFT) | PMC_MCR_CMD |
			PMC_MCR_ID(master->id));
	return 0;
}

static int clk_sama7g5_master_enable(struct clk *clk)
{
	struct clk_master *master = to_clk_master(clk);

	pmc_write(master->base, PMC_MCR, PMC_MCR_ID(master->id));
	pmc_update_bits(master->base, PMC_MCR,
			PMC_MCR_EN | PMC_MCR_CMD | PMC_MCR_ID_MSK,
			PMC_MCR_EN | PMC_MCR_CMD | PMC_MCR_ID(master->id));

	return 0;
}

static int clk_sama7g5_master_disable(struct clk *clk)
{
	struct clk_master *master = to_clk_master(clk);

	pmc_write(master->base, PMC_MCR, master->id);
	pmc_update_bits(master->base, PMC_MCR,
			PMC_MCR_EN | PMC_MCR_CMD | PMC_MCR_ID_MSK,
			PMC_MCR_CMD | PMC_MCR_ID(master->id));

	return 0;
}

static ulong clk_sama7g5_master_set_rate(struct clk *clk, ulong rate)
{
	struct clk_master *master = to_clk_master(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	ulong div, rrate;

	if (!parent_rate)
		return 0;

	div = DIV_ROUND_CLOSEST(parent_rate, rate);
	if ((div > (1 << (MASTER_PRES_MAX - 1))) || (div & (div - 1))) {
		return 0;
	} else if (div == 3) {
		rrate = DIV_ROUND_CLOSEST(parent_rate, MASTER_PRES_MAX);
		div = MASTER_PRES_MAX;
	} else {
		rrate = DIV_ROUND_CLOSEST(parent_rate, div);
		div = ffs(div) - 1;
	}

	pmc_write(master->base, PMC_MCR, master->id);
	pmc_update_bits(master->base, PMC_MCR,
			PMC_MCR_DIV | PMC_MCR_CMD | PMC_MCR_ID_MSK,
			(div << MASTER_DIV_SHIFT) | PMC_MCR_CMD |
			PMC_MCR_ID(master->id));

	return rrate;
}

static ulong clk_sama7g5_master_get_rate(struct clk *clk)
{
	struct clk_master *master = to_clk_master(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	unsigned int val;
	ulong div;

	if (!parent_rate)
		return 0;

	pmc_write(master->base, PMC_MCR, master->id);
	pmc_read(master->base, PMC_MCR, &val);

	div = (val >> MASTER_DIV_SHIFT) & MASTER_DIV_MASK;

	if (div == MASTER_PRES_MAX)
		div = 3;
	else
		div = 1 << div;

	return DIV_ROUND_CLOSEST(parent_rate, div);
}

static const struct clk_ops sama7g5_master_ops = {
	.enable = clk_sama7g5_master_enable,
	.disable = clk_sama7g5_master_disable,
	.set_rate = clk_sama7g5_master_set_rate,
	.get_rate = clk_sama7g5_master_get_rate,
	.set_parent = clk_sama7g5_master_set_parent,
};

struct clk *at91_clk_sama7g5_register_master(void __iomem *base,
		const char *name, const char * const *parent_names,
		int num_parents, const u32 *mux_table, const u32 *clk_mux_table,
		bool critical, u8 id)
{
	struct clk_master *master;
	struct clk *clk;
	u32 val, index;
	int ret;

	if (!base || !name || !num_parents || !parent_names ||
	    !mux_table || !clk_mux_table || id > MASTER_MAX_ID)
		return ERR_PTR(-EINVAL);

	master = kzalloc(sizeof(*master), GFP_KERNEL);
	if (!master)
		return ERR_PTR(-ENOMEM);

	master->base = base;
	master->id = id;
	master->mux_table = mux_table;
	master->clk_mux_table = clk_mux_table;
	master->num_parents = num_parents;

	pmc_write(master->base, PMC_MCR, master->id);
	pmc_read(master->base, PMC_MCR, &val);

	index = at91_clk_mux_val_to_index(master->mux_table,
				master->num_parents,
				(val & PMC_MCR_CSS) >> PMC_MCR_CSS_SHIFT);
	if (index < 0) {
		kfree(master);
		return ERR_PTR(index);
	}

	clk = &master->clk;
	clk->flags = CLK_GET_RATE_NOCACHE | (critical ? CLK_IS_CRITICAL : 0);

	ret = clk_register(clk, UBOOT_DM_CLK_AT91_SAMA7G5_MASTER, name,
			   parent_names[index]);
	if (ret) {
		kfree(master);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_sama7g5_master_clk) = {
	.name = UBOOT_DM_CLK_AT91_SAMA7G5_MASTER,
	.id = UCLASS_CLK,
	.ops = &sama7g5_master_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

const struct clk_master_layout at91rm9200_master_layout = {
	.mask = 0x31F,
	.pres_shift = 2,
	.offset = AT91_PMC_MCKR,
};

const struct clk_master_layout at91sam9x5_master_layout = {
	.mask = 0x373,
	.pres_shift = 4,
	.offset = AT91_PMC_MCKR,
};
