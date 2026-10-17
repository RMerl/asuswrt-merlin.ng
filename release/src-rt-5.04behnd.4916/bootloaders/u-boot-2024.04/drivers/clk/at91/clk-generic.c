// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic clock support for AT91 architectures.
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 *
 * Based on drivers/clk/at91/clk-generated.c from Linux.
 */
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/io.h>
#include <linux/clk-provider.h>
#include <linux/clk/at91_pmc.h>

#include "pmc.h"

#define UBOOT_DM_CLK_AT91_GCK		"at91-gck-clk"

#define GENERATED_MAX_DIV	255

struct clk_gck {
	void __iomem *base;
	const u32 *clk_mux_table;
	const u32 *mux_table;
	const struct clk_pcr_layout *layout;
	struct clk_range range;
	struct clk clk;
	u32 num_parents;
	u32 id;
};

#define to_clk_gck(_c) container_of(_c, struct clk_gck, clk)

static int clk_gck_enable(struct clk *clk)
{
	struct clk_gck *gck = to_clk_gck(clk);

	pmc_write(gck->base, gck->layout->offset,
		  (gck->id & gck->layout->pid_mask));
	pmc_update_bits(gck->base, gck->layout->offset,
			gck->layout->cmd | AT91_PMC_PCR_GCKEN,
			gck->layout->cmd | AT91_PMC_PCR_GCKEN);

	return 0;
}

static int clk_gck_disable(struct clk *clk)
{
	struct clk_gck *gck = to_clk_gck(clk);

	pmc_write(gck->base, gck->layout->offset,
		  (gck->id & gck->layout->pid_mask));
	pmc_update_bits(gck->base, gck->layout->offset,
			gck->layout->cmd | AT91_PMC_PCR_GCKEN,
			gck->layout->cmd);

	return 0;
}

static int clk_gck_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk_gck *gck = to_clk_gck(clk);
	int index;

	index = at91_clk_mux_val_to_index(gck->clk_mux_table, gck->num_parents,
					  parent->id);
	if (index < 0)
		return index;

	index = at91_clk_mux_index_to_val(gck->mux_table, gck->num_parents,
					  index);
	if (index < 0)
		return index;

	pmc_write(gck->base, gck->layout->offset,
		  (gck->id & gck->layout->pid_mask));
	pmc_update_bits(gck->base, gck->layout->offset,
			gck->layout->gckcss_mask | gck->layout->cmd,
			(index << (ffs(gck->layout->gckcss_mask) - 1)) |
			gck->layout->cmd);

	return 0;
}

static ulong clk_gck_set_rate(struct clk *clk, ulong rate)
{
	struct clk_gck *gck = to_clk_gck(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 div;

	if (!rate || !parent_rate)
		return 0;

	if (gck->range.max && rate > gck->range.max)
		return 0;

	div = DIV_ROUND_CLOSEST(parent_rate, rate);
	if (div > GENERATED_MAX_DIV + 1 || !div)
		return 0;

	pmc_write(gck->base, gck->layout->offset,
		  (gck->id & gck->layout->pid_mask));
	pmc_update_bits(gck->base, gck->layout->offset,
			AT91_PMC_PCR_GCKDIV_MASK | gck->layout->cmd,
			((div - 1) << (ffs(AT91_PMC_PCR_GCKDIV_MASK) - 1)) |
			gck->layout->cmd);

	return parent_rate / div;
}

static ulong clk_gck_get_rate(struct clk *clk)
{
	struct clk_gck *gck = to_clk_gck(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 val, div;

	if (!parent_rate)
		return 0;

	pmc_write(gck->base, gck->layout->offset,
		  (gck->id & gck->layout->pid_mask));
	pmc_read(gck->base, gck->layout->offset, &val);

	div = (val & AT91_PMC_PCR_GCKDIV_MASK) >>
		(ffs(AT91_PMC_PCR_GCKDIV_MASK) - 1);

	return parent_rate / (div + 1);
}

static const struct clk_ops gck_ops = {
	.enable = clk_gck_enable,
	.disable = clk_gck_disable,
	.set_parent = clk_gck_set_parent,
	.set_rate = clk_gck_set_rate,
	.get_rate = clk_gck_get_rate,
};

struct clk *
at91_clk_register_generic(void __iomem *base,
			  const struct clk_pcr_layout *layout,
			  const char *name, const char * const *parent_names,
			  const u32 *clk_mux_table, const u32 *mux_table,
			  u8 num_parents, u8 id,
			  const struct clk_range *range)
{
	struct clk_gck *gck;
	struct clk *clk;
	int ret, index;
	u32 val;

	if (!base || !layout || !name || !parent_names || !num_parents ||
	    !clk_mux_table || !mux_table || !range)
		return ERR_PTR(-EINVAL);

	gck = kzalloc(sizeof(*gck), GFP_KERNEL);
	if (!gck)
		return ERR_PTR(-ENOMEM);

	gck->id = id;
	gck->base = base;
	gck->range = *range;
	gck->layout = layout;
	gck->clk_mux_table = clk_mux_table;
	gck->mux_table = mux_table;
	gck->num_parents = num_parents;

	clk = &gck->clk;
	clk->flags = CLK_GET_RATE_NOCACHE;

	pmc_write(gck->base, gck->layout->offset,
		  (gck->id & gck->layout->pid_mask));
	pmc_read(gck->base, gck->layout->offset, &val);

	val = (val & gck->layout->gckcss_mask) >>
		(ffs(gck->layout->gckcss_mask) - 1);

	index = at91_clk_mux_val_to_index(gck->mux_table, gck->num_parents,
					  val);
	if (index < 0) {
		kfree(gck);
		return ERR_PTR(index);
	}

	ret = clk_register(clk, UBOOT_DM_CLK_AT91_GCK, name,
			   parent_names[index]);
	if (ret) {
		kfree(gck);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_gck_clk) = {
	.name = UBOOT_DM_CLK_AT91_GCK,
	.id = UCLASS_CLK,
	.ops = &gck_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
