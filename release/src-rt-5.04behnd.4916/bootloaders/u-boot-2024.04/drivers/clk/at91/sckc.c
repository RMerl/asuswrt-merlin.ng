// SPDX-License-Identifier: GPL-2.0+
/*
 * Slow clock support for AT91 architectures.
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dt-bindings/clk/at91.h>
#include <linux/clk-provider.h>

#include "pmc.h"

#define UBOOT_DM_CLK_AT91_SAM9X60_TD_SLCK	"at91-sam9x60-td-slck"
#define UBOOT_DM_CLK_AT91_SCKC			"at91-sckc"

#define AT91_OSC_SEL		BIT(24)
#define AT91_OSC_SEL_SHIFT	(24)

struct sam9x60_sckc {
	void __iomem *reg;
	const char **parent_names;
	unsigned int num_parents;
	struct clk clk;
};

#define to_sam9x60_sckc(c)	container_of(c, struct sam9x60_sckc, clk)

static int sam9x60_sckc_of_xlate(struct clk *clk,
				 struct ofnode_phandle_args *args)
{
	if (args->args_count != 1) {
		debug("AT91: SCKC: Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	clk->id = AT91_TO_CLK_ID(PMC_TYPE_SLOW, args->args[0]);

	return 0;
}

static const struct clk_ops sam9x60_sckc_ops = {
	.of_xlate = sam9x60_sckc_of_xlate,
	.get_rate = clk_generic_get_rate,
};

static int sam9x60_td_slck_set_parent(struct clk *clk, struct clk *parent)
{
	struct sam9x60_sckc *sckc = to_sam9x60_sckc(clk);
	u32 i;

	for (i = 0; i < sckc->num_parents; i++) {
		if (!strcmp(parent->dev->name, sckc->parent_names[i]))
			break;
	}
	if (i == sckc->num_parents)
		return -EINVAL;

	pmc_update_bits(sckc->reg, 0, AT91_OSC_SEL, (i << AT91_OSC_SEL_SHIFT));

	return 0;
}

static const struct clk_ops sam9x60_td_slck_ops = {
	.get_rate = clk_generic_get_rate,
	.set_parent = sam9x60_td_slck_set_parent,
};

static struct clk *at91_sam9x60_clk_register_td_slck(struct sam9x60_sckc *sckc,
		const char *name, const char * const *parent_names,
		int num_parents)
{
	struct clk *clk;
	int ret = -ENOMEM;
	u32 val, i;

	if (!sckc || !name || !parent_names || num_parents != 2)
		return ERR_PTR(-EINVAL);

	sckc->parent_names = kzalloc(sizeof(*sckc->parent_names) * num_parents,
				     GFP_KERNEL);
	if (!sckc->parent_names)
		return ERR_PTR(ret);

	for (i = 0; i < num_parents; i++) {
		sckc->parent_names[i] = kmemdup(parent_names[i],
				strlen(parent_names[i]) + 1, GFP_KERNEL);
		if (!sckc->parent_names[i])
			goto free;
	}
	sckc->num_parents = num_parents;

	pmc_read(sckc->reg, 0, &val);
	val = (val & AT91_OSC_SEL) >> AT91_OSC_SEL_SHIFT;

	clk = &sckc->clk;
	ret = clk_register(clk, UBOOT_DM_CLK_AT91_SAM9X60_TD_SLCK, name,
			   parent_names[val]);
	if (ret)
		goto free;

	return clk;

free:
	for (; i >= 0; i--)
		kfree(sckc->parent_names[i]);
	kfree(sckc->parent_names);

	return ERR_PTR(ret);
}

U_BOOT_DRIVER(at91_sam9x60_td_slck) = {
	.name = UBOOT_DM_CLK_AT91_SAM9X60_TD_SLCK,
	.id = UCLASS_CLK,
	.ops = &sam9x60_td_slck_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static int at91_sam9x60_sckc_probe(struct udevice *dev)
{
	struct sam9x60_sckc *sckc = dev_get_priv(dev);
	void __iomem *base = devfdt_get_addr_ptr(dev);
	const char *slow_rc_osc, *slow_osc;
	const char *parents[2];
	struct clk *clk, c;
	int ret;

	ret = clk_get_by_index(dev, 0, &c);
	if (ret)
		return ret;
	slow_rc_osc = clk_hw_get_name(&c);

	ret = clk_get_by_index(dev, 1, &c);
	if (ret)
		return ret;
	slow_osc = clk_hw_get_name(&c);

	clk = clk_register_fixed_factor(NULL, "md_slck", slow_rc_osc, 0, 1, 1);
	if (IS_ERR(clk))
		return PTR_ERR(clk);
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_SLOW, 0), clk);

	parents[0] = slow_rc_osc;
	parents[1] = slow_osc;
	sckc[1].reg = base;
	clk = at91_sam9x60_clk_register_td_slck(&sckc[1], "td_slck",
						parents, 2);
	if (IS_ERR(clk))
		return PTR_ERR(clk);
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_SLOW, 1), clk);

	return 0;
}

static const struct udevice_id sam9x60_sckc_ids[] = {
	{ .compatible = "microchip,sam9x60-sckc" },
	{ /* Sentinel. */ },
};

U_BOOT_DRIVER(at91_sckc) = {
	.name = UBOOT_DM_CLK_AT91_SCKC,
	.id = UCLASS_CLK,
	.of_match = sam9x60_sckc_ids,
	.priv_auto	= sizeof(struct sam9x60_sckc) * 2,
	.ops = &sam9x60_sckc_ops,
	.probe = at91_sam9x60_sckc_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
