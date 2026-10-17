// SPDX-License-Identifier: GPL-2.0+
/*
 * Peripheral clock support for AT91 architectures.
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 *
 * Based on drivers/clk/at91/clk-peripheral.c from Linux.
 */
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/io.h>
#include <linux/clk-provider.h>
#include <linux/clk/at91_pmc.h>

#include "pmc.h"

#define UBOOT_DM_CLK_AT91_PERIPH		"at91-periph-clk"
#define UBOOT_DM_CLK_AT91_SAM9X5_PERIPH		"at91-sam9x5-periph-clk"

#define PERIPHERAL_ID_MIN	2
#define PERIPHERAL_ID_MAX	31
#define PERIPHERAL_MASK(id)	(1 << ((id) & PERIPHERAL_ID_MAX))

#define PERIPHERAL_MAX_SHIFT	3

struct clk_peripheral {
	void __iomem *base;
	struct clk clk;
	u32 id;
};

#define to_clk_peripheral(_c) container_of(_c, struct clk_peripheral, clk)

struct clk_sam9x5_peripheral {
	const struct clk_pcr_layout *layout;
	void __iomem *base;
	struct clk clk;
	struct clk_range range;
	u32 id;
	u32 div;
	bool auto_div;
};

#define to_clk_sam9x5_peripheral(_c) \
	container_of(_c, struct clk_sam9x5_peripheral, clk)

static int clk_peripheral_enable(struct clk *clk)
{
	struct clk_peripheral *periph = to_clk_peripheral(clk);
	int offset = AT91_PMC_PCER;
	u32 id = periph->id;

	if (id < PERIPHERAL_ID_MIN)
		return 0;
	if (id > PERIPHERAL_ID_MAX)
		offset = AT91_PMC_PCER1;
	pmc_write(periph->base, offset, PERIPHERAL_MASK(id));

	return 0;
}

static int clk_peripheral_disable(struct clk *clk)
{
	struct clk_peripheral *periph = to_clk_peripheral(clk);
	int offset = AT91_PMC_PCDR;
	u32 id = periph->id;

	if (id < PERIPHERAL_ID_MIN)
		return -EINVAL;

	if (id > PERIPHERAL_ID_MAX)
		offset = AT91_PMC_PCDR1;
	pmc_write(periph->base, offset, PERIPHERAL_MASK(id));

	return 0;
}

static const struct clk_ops peripheral_ops = {
	.enable = clk_peripheral_enable,
	.disable = clk_peripheral_disable,
	.get_rate = clk_generic_get_rate,
};

struct clk *
at91_clk_register_peripheral(void __iomem *base, const char *name,
			     const char *parent_name, u32 id)
{
	struct clk_peripheral *periph;
	struct clk *clk;
	int ret;

	if (!base || !name || !parent_name || id > PERIPHERAL_ID_MAX)
		return ERR_PTR(-EINVAL);

	periph = kzalloc(sizeof(*periph), GFP_KERNEL);
	if (!periph)
		return ERR_PTR(-ENOMEM);

	periph->id = id;
	periph->base = base;

	clk = &periph->clk;
	clk->flags = CLK_GET_RATE_NOCACHE;
	ret = clk_register(clk, UBOOT_DM_CLK_AT91_PERIPH, name, parent_name);
	if (ret) {
		kfree(periph);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_periph_clk) = {
	.name = UBOOT_DM_CLK_AT91_PERIPH,
	.id = UCLASS_CLK,
	.ops = &peripheral_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static int clk_sam9x5_peripheral_enable(struct clk *clk)
{
	struct clk_sam9x5_peripheral *periph = to_clk_sam9x5_peripheral(clk);

	if (periph->id < PERIPHERAL_ID_MIN)
		return 0;

	pmc_write(periph->base, periph->layout->offset,
		  (periph->id & periph->layout->pid_mask));
	pmc_update_bits(periph->base, periph->layout->offset,
			periph->layout->cmd | AT91_PMC_PCR_EN,
			periph->layout->cmd | AT91_PMC_PCR_EN);

	return 0;
}

static int clk_sam9x5_peripheral_disable(struct clk *clk)
{
	struct clk_sam9x5_peripheral *periph = to_clk_sam9x5_peripheral(clk);

	if (periph->id < PERIPHERAL_ID_MIN)
		return -EINVAL;

	pmc_write(periph->base, periph->layout->offset,
		  (periph->id & periph->layout->pid_mask));
	pmc_update_bits(periph->base, periph->layout->offset,
			AT91_PMC_PCR_EN | periph->layout->cmd,
			periph->layout->cmd);

	return 0;
}

static ulong clk_sam9x5_peripheral_get_rate(struct clk *clk)
{
	struct clk_sam9x5_peripheral *periph = to_clk_sam9x5_peripheral(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 val, shift = ffs(periph->layout->div_mask) - 1;

	if (!parent_rate)
		return 0;

	pmc_write(periph->base, periph->layout->offset,
		  (periph->id & periph->layout->pid_mask));
	pmc_read(periph->base, periph->layout->offset, &val);
	shift = (val & periph->layout->div_mask) >> shift;

	return parent_rate >> shift;
}

static ulong clk_sam9x5_peripheral_set_rate(struct clk *clk, ulong rate)
{
	struct clk_sam9x5_peripheral *periph = to_clk_sam9x5_peripheral(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	int shift;

	if (!parent_rate)
		return 0;

	if (periph->id < PERIPHERAL_ID_MIN || !periph->range.max) {
		if (parent_rate == rate)
			return rate;
		else
			return 0;
	}

	if (periph->range.max && rate > periph->range.max)
		return 0;

	for (shift = 0; shift <= PERIPHERAL_MAX_SHIFT; shift++) {
		if (parent_rate >> shift <= rate)
			break;
	}
	if (shift == PERIPHERAL_MAX_SHIFT + 1)
		return 0;

	pmc_write(periph->base, periph->layout->offset,
		  (periph->id & periph->layout->pid_mask));
	pmc_update_bits(periph->base, periph->layout->offset,
			periph->layout->div_mask | periph->layout->cmd,
			(shift << (ffs(periph->layout->div_mask) - 1)) |
			periph->layout->cmd);

	return parent_rate >> shift;
}

static const struct clk_ops sam9x5_peripheral_ops = {
	.enable = clk_sam9x5_peripheral_enable,
	.disable = clk_sam9x5_peripheral_disable,
	.get_rate = clk_sam9x5_peripheral_get_rate,
	.set_rate = clk_sam9x5_peripheral_set_rate,
};

struct clk *
at91_clk_register_sam9x5_peripheral(void __iomem *base,
				    const struct clk_pcr_layout *layout,
				    const char *name, const char *parent_name,
				    u32 id, const struct clk_range *range)
{
	struct clk_sam9x5_peripheral *periph;
	struct clk *clk;
	int ret;

	if (!base || !layout || !name || !parent_name || !range)
		return ERR_PTR(-EINVAL);

	periph = kzalloc(sizeof(*periph), GFP_KERNEL);
	if (!periph)
		return ERR_PTR(-ENOMEM);

	periph->id = id;
	periph->base = base;
	periph->layout = layout;
	periph->range = *range;

	clk = &periph->clk;
	clk->flags = CLK_GET_RATE_NOCACHE;
	ret = clk_register(clk, UBOOT_DM_CLK_AT91_SAM9X5_PERIPH, name,
			   parent_name);
	if (ret) {
		kfree(periph);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_sam9x5_periph_clk) = {
	.name = UBOOT_DM_CLK_AT91_SAM9X5_PERIPH,
	.id = UCLASS_CLK,
	.ops = &sam9x5_peripheral_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
