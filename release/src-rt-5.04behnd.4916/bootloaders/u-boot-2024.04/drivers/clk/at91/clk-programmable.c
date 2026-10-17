// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Programmable clock support for AT91 architectures.
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 *
 * Based on drivers/clk/at91/clk-programmable.c from Linux.
 */
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/clk-provider.h>
#include <linux/clk/at91_pmc.h>

#include "pmc.h"

#define UBOOT_DM_CLK_AT91_PROG		"at91-prog-clk"

#define PROG_ID_MAX		7

#define PROG_STATUS_MASK(id)	(1 << ((id) + 8))
#define PROG_PRES(_l, _p)	(((_p) >> (_l)->pres_shift) & (_l)->pres_mask)
#define PROG_MAX_RM9200_CSS	3

struct clk_programmable {
	void __iomem *base;
	const u32 *clk_mux_table;
	const u32 *mux_table;
	const struct clk_programmable_layout *layout;
	u32 num_parents;
	struct clk clk;
	u8 id;
};

#define to_clk_programmable(_c) container_of(_c, struct clk_programmable, clk)

static ulong clk_programmable_get_rate(struct clk *clk)
{
	struct clk_programmable *prog = to_clk_programmable(clk);
	const struct clk_programmable_layout *layout = prog->layout;
	ulong rate, parent_rate = clk_get_parent_rate(clk);
	unsigned int pckr;

	pmc_read(prog->base, AT91_PMC_PCKR(prog->id), &pckr);

	if (layout->is_pres_direct)
		rate = parent_rate / (PROG_PRES(layout, pckr) + 1);
	else
		rate = parent_rate >> PROG_PRES(layout, pckr);

	return rate;
}

static int clk_programmable_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk_programmable *prog = to_clk_programmable(clk);
	const struct clk_programmable_layout *layout = prog->layout;
	unsigned int mask = layout->css_mask;
	int index;

	index = at91_clk_mux_val_to_index(prog->clk_mux_table,
					  prog->num_parents, parent->id);
	if (index < 0)
		return index;

	index = at91_clk_mux_index_to_val(prog->mux_table, prog->num_parents,
					  index);
	if (index < 0)
		return index;

	if (layout->have_slck_mck)
		mask |= AT91_PMC_CSSMCK_MCK;

	if (index > layout->css_mask) {
		if (index > PROG_MAX_RM9200_CSS && !layout->have_slck_mck)
			return -EINVAL;

		index |= AT91_PMC_CSSMCK_MCK;
	}

	pmc_update_bits(prog->base, AT91_PMC_PCKR(prog->id), mask, index);

	return 0;
}

static ulong clk_programmable_set_rate(struct clk *clk, ulong rate)
{
	struct clk_programmable *prog = to_clk_programmable(clk);
	const struct clk_programmable_layout *layout = prog->layout;
	ulong parent_rate = clk_get_parent_rate(clk);
	ulong div = parent_rate / rate;
	int shift = 0;

	if (!parent_rate || !div)
		return -EINVAL;

	if (layout->is_pres_direct) {
		shift = div - 1;

		if (shift > layout->pres_mask)
			return -EINVAL;
	} else {
		shift = fls(div) - 1;

		if (div != (1 << shift))
			return -EINVAL;

		if (shift >= layout->pres_mask)
			return -EINVAL;
	}

	pmc_update_bits(prog->base, AT91_PMC_PCKR(prog->id),
			layout->pres_mask << layout->pres_shift,
			shift << layout->pres_shift);

	if (layout->is_pres_direct)
		return (parent_rate / shift + 1);

	return parent_rate >> shift;
}

static const struct clk_ops programmable_ops = {
	.get_rate = clk_programmable_get_rate,
	.set_parent = clk_programmable_set_parent,
	.set_rate = clk_programmable_set_rate,
};

struct clk *at91_clk_register_programmable(void __iomem *base, const char *name,
			const char *const *parent_names, u8 num_parents, u8 id,
			const struct clk_programmable_layout *layout,
			const u32 *clk_mux_table, const u32 *mux_table)
{
	struct clk_programmable *prog;
	struct clk *clk;
	u32 val, tmp;
	int ret;

	if (!base || !name || !parent_names || !num_parents ||
	    !layout || !clk_mux_table || !mux_table || id > PROG_ID_MAX)
		return ERR_PTR(-EINVAL);

	prog = kzalloc(sizeof(*prog), GFP_KERNEL);
	if (!prog)
		return ERR_PTR(-ENOMEM);

	prog->id = id;
	prog->layout = layout;
	prog->base = base;
	prog->clk_mux_table = clk_mux_table;
	prog->mux_table = mux_table;
	prog->num_parents = num_parents;

	pmc_read(prog->base, AT91_PMC_PCKR(prog->id), &tmp);
	val = tmp & prog->layout->css_mask;
	if (layout->have_slck_mck && (tmp & AT91_PMC_CSSMCK_MCK) && !val)
		ret = PROG_MAX_RM9200_CSS + 1;
	else
		ret = at91_clk_mux_val_to_index(prog->mux_table,
						prog->num_parents, val);
	if (ret < 0) {
		kfree(prog);
		return ERR_PTR(ret);
	}

	clk = &prog->clk;
	clk->flags = CLK_GET_RATE_NOCACHE;
	ret = clk_register(clk, UBOOT_DM_CLK_AT91_PROG, name,
			   parent_names[ret]);
	if (ret) {
		kfree(prog);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_prog_clk) = {
	.name = UBOOT_DM_CLK_AT91_PROG,
	.id = UCLASS_CLK,
	.ops = &programmable_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

const struct clk_programmable_layout at91rm9200_programmable_layout = {
	.pres_mask = 0x7,
	.pres_shift = 2,
	.css_mask = 0x3,
	.have_slck_mck = 0,
	.is_pres_direct = 0,
};

const struct clk_programmable_layout at91sam9g45_programmable_layout = {
	.pres_mask = 0x7,
	.pres_shift = 2,
	.css_mask = 0x3,
	.have_slck_mck = 1,
	.is_pres_direct = 0,
};

const struct clk_programmable_layout at91sam9x5_programmable_layout = {
	.pres_mask = 0x7,
	.pres_shift = 4,
	.css_mask = 0x7,
	.have_slck_mck = 0,
	.is_pres_direct = 0,
};
