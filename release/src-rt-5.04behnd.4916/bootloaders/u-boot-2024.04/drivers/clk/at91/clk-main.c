// SPDX-License-Identifier: GPL-2.0+
/*
 * Main clock support for AT91 architectures.
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 *
 * Based on drivers/clk/at91/clk-main.c from Linux.
 */

#include <asm/processor.h>
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/clk-provider.h>
#include <linux/clk/at91_pmc.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/time.h>
#include "pmc.h"

#define UBOOT_DM_CLK_AT91_MAIN_RC		"at91-main-rc-clk"
#define UBOOT_DM_CLK_AT91_MAIN_OSC		"at91-main-osc-clk"
#define UBOOT_DM_CLK_AT91_RM9200_MAIN		"at91-rm9200-main-clk"
#define UBOOT_DM_CLK_AT91_SAM9X5_MAIN		"at91-sam9x5-main-clk"

#define MOR_KEY_MASK		GENMASK(23, 16)
#define SLOW_CLOCK_FREQ		32768

#define clk_main_parent_select(s)	(((s) & \
					(AT91_PMC_MOSCEN | \
					AT91_PMC_OSCBYPASS)) ? 1 : 0)

struct clk_main_rc {
	void __iomem	*reg;
	struct clk	clk;
};

#define to_clk_main_rc(_clk) container_of(_clk, struct clk_main_rc, clk)

struct clk_main_osc {
	void __iomem	*reg;
	struct clk	clk;
};

#define to_clk_main_osc(_clk) container_of(_clk, struct clk_main_osc, clk)

struct clk_main {
	void __iomem		*reg;
	const unsigned int	*clk_mux_table;
	const char * const	*parent_names;
	unsigned int		num_parents;
	int			type;
	struct clk		clk;
};

#define to_clk_main(_clk) container_of(_clk, struct clk_main, clk)

static int main_rc_enable(struct clk *clk)
{
	struct clk_main_rc *main_rc = to_clk_main_rc(clk);
	void __iomem *reg = main_rc->reg;
	unsigned int val;

	pmc_read(reg, AT91_CKGR_MOR, &val);

	if (!(val & AT91_PMC_MOSCRCEN)) {
		pmc_update_bits(reg, AT91_CKGR_MOR,
				MOR_KEY_MASK | AT91_PMC_MOSCRCEN,
				AT91_PMC_KEY | AT91_PMC_MOSCRCEN);
	}

	pmc_read(reg, AT91_PMC_SR, &val);
	while (!(val & AT91_PMC_MOSCRCS)) {
		pmc_read(reg, AT91_PMC_SR, &val);
		debug("waiting for main rc...\n");
		cpu_relax();
	}

	return 0;
}

static int main_rc_disable(struct clk *clk)
{
	struct clk_main_rc *main_rc = to_clk_main_rc(clk);
	struct reg *reg = main_rc->reg;
	unsigned int val;

	pmc_read(reg, AT91_CKGR_MOR, &val);

	if (!(val & AT91_PMC_MOSCRCEN))
		return 0;

	pmc_update_bits(reg, AT91_CKGR_MOR, MOR_KEY_MASK | AT91_PMC_MOSCRCEN,
			AT91_PMC_KEY);

	return 0;
}

static const struct clk_ops main_rc_clk_ops = {
	.enable = main_rc_enable,
	.disable = main_rc_disable,
	.get_rate = clk_generic_get_rate,
};

struct clk *at91_clk_main_rc(void __iomem *reg, const char *name,
			     const char *parent_name)
{
	struct clk_main_rc *main_rc;
	struct clk *clk;
	int ret;

	if (!reg || !name || !parent_name)
		return ERR_PTR(-EINVAL);

	main_rc = kzalloc(sizeof(*main_rc), GFP_KERNEL);
	if (!main_rc)
		return ERR_PTR(-ENOMEM);

	main_rc->reg = reg;
	clk = &main_rc->clk;

	ret = clk_register(clk, UBOOT_DM_CLK_AT91_MAIN_RC, name,
			   parent_name);
	if (ret) {
		kfree(main_rc);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_main_rc_clk) = {
	.name = UBOOT_DM_CLK_AT91_MAIN_RC,
	.id = UCLASS_CLK,
	.ops = &main_rc_clk_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static int clk_main_osc_enable(struct clk *clk)
{
	struct clk_main_osc *main = to_clk_main_osc(clk);
	void __iomem *reg = main->reg;
	unsigned int val;

	pmc_read(reg, AT91_CKGR_MOR, &val);
	val &= ~MOR_KEY_MASK;

	if (val & AT91_PMC_OSCBYPASS)
		return 0;

	if (!(val & AT91_PMC_MOSCEN)) {
		val |= AT91_PMC_MOSCEN | AT91_PMC_KEY;
		pmc_write(reg, AT91_CKGR_MOR, val);
	}

	pmc_read(reg, AT91_PMC_SR, &val);
	while (!(val & AT91_PMC_MOSCS)) {
		pmc_read(reg, AT91_PMC_SR, &val);
		debug("waiting for main osc..\n");
		cpu_relax();
	}

	return 0;
}

static int clk_main_osc_disable(struct clk *clk)
{
	struct clk_main_osc *main = to_clk_main_osc(clk);
	void __iomem *reg = main->reg;
	unsigned int val;

	pmc_read(reg, AT91_CKGR_MOR, &val);
	if (val & AT91_PMC_OSCBYPASS)
		return 0;

	if (!(val & AT91_PMC_MOSCEN))
		return 0;

	val &= ~(AT91_PMC_KEY | AT91_PMC_MOSCEN);
	pmc_write(reg, AT91_CKGR_MOR, val | AT91_PMC_KEY);

	return 0;
}

static const struct clk_ops main_osc_clk_ops = {
	.enable = clk_main_osc_enable,
	.disable = clk_main_osc_disable,
	.get_rate = clk_generic_get_rate,
};

struct clk *at91_clk_main_osc(void __iomem *reg, const char *name,
			      const char *parent_name, bool bypass)
{
	struct clk_main_osc *main;
	struct clk *clk;
	int ret;

	if (!reg || !name || !parent_name)
		return ERR_PTR(-EINVAL);

	main = kzalloc(sizeof(*main), GFP_KERNEL);
	if (!main)
		return ERR_PTR(-ENOMEM);

	main->reg = reg;
	clk = &main->clk;

	if (bypass) {
		pmc_update_bits(reg, AT91_CKGR_MOR,
				MOR_KEY_MASK | AT91_PMC_OSCBYPASS,
				AT91_PMC_KEY | AT91_PMC_OSCBYPASS);
	}

	ret = clk_register(clk, UBOOT_DM_CLK_AT91_MAIN_OSC, name, parent_name);
	if (ret) {
		kfree(main);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_main_osc_clk) = {
	.name = UBOOT_DM_CLK_AT91_MAIN_OSC,
	.id = UCLASS_CLK,
	.ops = &main_osc_clk_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static int clk_main_probe_frequency(void __iomem *reg)
{
	unsigned int cycles = 16;
	unsigned int cycle = DIV_ROUND_UP(USEC_PER_SEC, SLOW_CLOCK_FREQ);
	unsigned int mcfr;

	while (cycles--) {
		pmc_read(reg, AT91_CKGR_MCFR, &mcfr);
		if (mcfr & AT91_PMC_MAINRDY)
			return 0;
		udelay(cycle);
	}

	return -ETIMEDOUT;
}

static int clk_rm9200_main_enable(struct clk *clk)
{
	struct clk_main *main = to_clk_main(clk);

	return clk_main_probe_frequency(main->reg);
}

static const struct clk_ops rm9200_main_clk_ops = {
	.enable = clk_rm9200_main_enable,
};

struct clk *at91_clk_rm9200_main(void __iomem *reg, const char *name,
				 const char *parent_name)
{
	struct clk_main *main;
	struct clk *clk;
	int ret;

	if (!reg || !name || !parent_name)
		return ERR_PTR(-EINVAL);

	main = kzalloc(sizeof(*main), GFP_KERNEL);
	if (!main)
		return ERR_PTR(-ENOMEM);

	main->reg = reg;
	clk = &main->clk;

	ret = clk_register(clk, UBOOT_DM_CLK_AT91_RM9200_MAIN, name,
			   parent_name);
	if (ret) {
		kfree(main);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_rm9200_main_clk) = {
	.name = UBOOT_DM_CLK_AT91_RM9200_MAIN,
	.id = UCLASS_CLK,
	.ops = &rm9200_main_clk_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static inline bool clk_sam9x5_main_ready(void __iomem *reg)
{
	unsigned int val;

	pmc_read(reg, AT91_PMC_SR, &val);

	return !!(val & AT91_PMC_MOSCSELS);
}

static int clk_sam9x5_main_enable(struct clk *clk)
{
	struct clk_main *main = to_clk_main(clk);
	void __iomem *reg = main->reg;

	while (!clk_sam9x5_main_ready(reg)) {
		debug("waiting for main...");
		cpu_relax();
	}

	return clk_main_probe_frequency(reg);
}

static int clk_sam9x5_main_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk_main *main = to_clk_main(clk);
	void __iomem *reg = main->reg;
	unsigned int tmp, index;

	index = at91_clk_mux_val_to_index(main->clk_mux_table,
			main->num_parents, AT91_CLK_ID_TO_DID(parent->id));
	if (index < 0)
		return index;

	pmc_read(reg, AT91_CKGR_MOR, &tmp);
	tmp &= ~MOR_KEY_MASK;
	tmp |= AT91_PMC_KEY;

	if (index && !(tmp & AT91_PMC_MOSCSEL))
		pmc_write(reg, AT91_CKGR_MOR, tmp | AT91_PMC_MOSCSEL);
	else if (!index && (tmp & AT91_PMC_MOSCSEL))
		pmc_write(reg, AT91_CKGR_MOR, tmp & ~AT91_PMC_MOSCSEL);

	while (!clk_sam9x5_main_ready(reg))
		cpu_relax();

	return 0;
}

static const struct clk_ops sam9x5_main_clk_ops = {
	.enable = clk_sam9x5_main_enable,
	.set_parent = clk_sam9x5_main_set_parent,
	.get_rate = clk_generic_get_rate,
};

struct clk *at91_clk_sam9x5_main(void __iomem *reg, const char *name,
				 const char * const *parent_names,
				 int num_parents, const u32 *clk_mux_table,
				 int type)
{
	struct clk *clk = ERR_PTR(-ENOMEM);
	struct clk_main *main = NULL;
	unsigned int val;
	int ret;

	if (!reg || !name || !parent_names || !num_parents || !clk_mux_table)
		return ERR_PTR(-EINVAL);

	main = kzalloc(sizeof(*main), GFP_KERNEL);
	if (!main)
		return ERR_PTR(-ENOMEM);

	main->reg = reg;
	main->parent_names = parent_names;
	main->num_parents = num_parents;
	main->clk_mux_table = clk_mux_table;
	main->type = type;
	clk = &main->clk;
	clk->flags = CLK_GET_RATE_NOCACHE;
	pmc_read(reg, AT91_CKGR_MOR, &val);
	ret = clk_register(clk, UBOOT_DM_CLK_AT91_SAM9X5_MAIN, name,
			   main->parent_names[clk_main_parent_select(val)]);
	if (ret) {
		kfree(main);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_sam9x5_main_clk) = {
	.name = UBOOT_DM_CLK_AT91_SAM9X5_MAIN,
	.id = UCLASS_CLK,
	.ops = &sam9x5_main_clk_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
