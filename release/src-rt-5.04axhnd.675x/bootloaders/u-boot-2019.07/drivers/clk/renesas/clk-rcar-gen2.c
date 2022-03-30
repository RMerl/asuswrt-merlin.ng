// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas RCar Gen2 CPG MSSR driver
 *
 * Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on the following driver from Linux kernel:
 * r8a7796 Clock Pulse Generator / Module Standby and Software Reset
 *
 * Copyright (C) 2016 Glider bvba
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>

#include <dt-bindings/clock/renesas-cpg-mssr.h>

#include "renesas-cpg-mssr.h"
#include "rcar-gen2-cpg.h"

#define CPG_RST_MODEMR		0x0060

#define CPG_PLL0CR		0x00d8
#define CPG_SDCKCR		0x0074

struct clk_div_table {
	u8	val;
	u8	div;
};

/* SDHI divisors */
static const struct clk_div_table cpg_sdh_div_table[] = {
	{  0,  2 }, {  1,  3 }, {  2,  4 }, {  3,  6 },
	{  4,  8 }, {  5, 12 }, {  6, 16 }, {  7, 18 },
	{  8, 24 }, { 10, 36 }, { 11, 48 }, {  0,  0 },
};

static const struct clk_div_table cpg_sd01_div_table[] = {
	{  4,  8 }, {  5, 12 }, {  6, 16 }, {  7, 18 },
	{  8, 24 }, { 10, 36 }, { 11, 48 }, { 12, 10 },
	{  0,  0 },
};

static u8 gen2_clk_get_sdh_div(const struct clk_div_table *table, u8 val)
{
	for (;;) {
		if (!(*table).div)
			return 0xff;

		if ((*table).val == val)
			return (*table).div;

		table++;
	}
}

static int gen2_clk_enable(struct clk *clk)
{
	struct gen2_clk_priv *priv = dev_get_priv(clk->dev);

	return renesas_clk_endisable(clk, priv->base, true);
}

static int gen2_clk_disable(struct clk *clk)
{
	struct gen2_clk_priv *priv = dev_get_priv(clk->dev);

	return renesas_clk_endisable(clk, priv->base, false);
}

static ulong gen2_clk_get_rate(struct clk *clk)
{
	struct gen2_clk_priv *priv = dev_get_priv(clk->dev);
	struct cpg_mssr_info *info = priv->info;
	struct clk parent;
	const struct cpg_core_clk *core;
	const struct rcar_gen2_cpg_pll_config *pll_config =
					priv->cpg_pll_config;
	u32 value, mult, div, rate = 0;
	int ret;

	debug("%s[%i] Clock: id=%lu\n", __func__, __LINE__, clk->id);

	ret = renesas_clk_get_parent(clk, info, &parent);
	if (ret) {
		printf("%s[%i] parent fail, ret=%i\n", __func__, __LINE__, ret);
		return ret;
	}

	if (renesas_clk_is_mod(clk)) {
		rate = gen2_clk_get_rate(&parent);
		debug("%s[%i] MOD clk: parent=%lu => rate=%u\n",
		      __func__, __LINE__, parent.id, rate);
		return rate;
	}

	ret = renesas_clk_get_core(clk, info, &core);
	if (ret)
		return ret;

	switch (core->type) {
	case CLK_TYPE_IN:
		if (core->id == info->clk_extal_id) {
			rate = clk_get_rate(&priv->clk_extal);
			debug("%s[%i] EXTAL clk: rate=%u\n",
			      __func__, __LINE__, rate);
			return rate;
		}

		if (core->id == info->clk_extal_usb_id) {
			rate = clk_get_rate(&priv->clk_extal_usb);
			debug("%s[%i] EXTALR clk: rate=%u\n",
			      __func__, __LINE__, rate);
			return rate;
		}

		return -EINVAL;

	case CLK_TYPE_FF:
		rate = (gen2_clk_get_rate(&parent) * core->mult) / core->div;
		debug("%s[%i] FIXED clk: parent=%i mul=%i div=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, core->mult, core->div, rate);
		return rate;

	case CLK_TYPE_DIV6P1:	/* DIV6 Clock with 1 parent clock */
		value = (readl(priv->base + core->offset) & 0x3f) + 1;
		rate = gen2_clk_get_rate(&parent) / value;
		debug("%s[%i] DIV6P1 clk: parent=%i div=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, value, rate);
		return rate;

	case CLK_TYPE_GEN2_MAIN:
		rate = gen2_clk_get_rate(&parent) / pll_config->extal_div;
		debug("%s[%i] MAIN clk: parent=%i extal_div=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, pll_config->extal_div, rate);
		return rate;

	case CLK_TYPE_GEN2_PLL0:
		/*
		 * PLL0 is a  configurable multiplier clock except on R-Car
		 * V2H/E2. Register the PLL0 clock as a fixed factor clock for
		 * now as there's no generic multiplier clock implementation and
		 * we  currently  have no need to change  the multiplier value.
		 */
		mult = pll_config->pll0_mult;
		if (!mult) {
			value = readl(priv->base + CPG_PLL0CR);
			mult = (((value >> 24) & 0x7f) + 1) * 2;
		}

		rate = (gen2_clk_get_rate(&parent) * mult) / info->pll0_div;
		debug("%s[%i] PLL0 clk: parent=%i mult=%u => rate=%u\n",
		      __func__, __LINE__, core->parent, mult, rate);
		return rate;

	case CLK_TYPE_GEN2_PLL1:
		rate = (gen2_clk_get_rate(&parent) * pll_config->pll1_mult) / 2;
		debug("%s[%i] PLL1 clk: parent=%i mul=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, pll_config->pll1_mult, rate);
		return rate;

	case CLK_TYPE_GEN2_PLL3:
		rate = gen2_clk_get_rate(&parent) * pll_config->pll3_mult;
		debug("%s[%i] PLL3 clk: parent=%i mul=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, pll_config->pll3_mult, rate);
		return rate;

	case CLK_TYPE_GEN2_SDH:
		value = (readl(priv->base + CPG_SDCKCR) >> 8) & 0xf;
		div = gen2_clk_get_sdh_div(cpg_sdh_div_table, value);
		rate = gen2_clk_get_rate(&parent) / div;
		debug("%s[%i] SDH clk: parent=%i div=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, div, rate);
		return rate;

	case CLK_TYPE_GEN2_SD0:
		value = (readl(priv->base + CPG_SDCKCR) >> 4) & 0xf;
		div = gen2_clk_get_sdh_div(cpg_sd01_div_table, value);
		rate = gen2_clk_get_rate(&parent) / div;
		debug("%s[%i] SD0 clk: parent=%i div=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, div, rate);
		return rate;

	case CLK_TYPE_GEN2_SD1:
		value = (readl(priv->base + CPG_SDCKCR) >> 0) & 0xf;
		div = gen2_clk_get_sdh_div(cpg_sd01_div_table, value);
		rate = gen2_clk_get_rate(&parent) / div;
		debug("%s[%i] SD1 clk: parent=%i div=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, div, rate);
		return rate;
	}

	printf("%s[%i] unknown fail\n", __func__, __LINE__);

	return -ENOENT;
}

static int gen2_clk_setup_mmcif_div(struct clk *clk, ulong rate)
{
	struct gen2_clk_priv *priv = dev_get_priv(clk->dev);
	struct cpg_mssr_info *info = priv->info;
	const struct cpg_core_clk *core;
	struct clk parent, pparent;
	u32 val;
	int ret;

	ret = renesas_clk_get_parent(clk, info, &parent);
	if (ret) {
		debug("%s[%i] parent fail, ret=%i\n", __func__, __LINE__, ret);
		return ret;
	}

	if (renesas_clk_is_mod(&parent))
		return 0;

	ret = renesas_clk_get_core(&parent, info, &core);
	if (ret)
		return ret;

	if (strcmp(core->name, "mmc0") && strcmp(core->name, "mmc1"))
		return 0;

	ret = renesas_clk_get_parent(&parent, info, &pparent);
	if (ret) {
		debug("%s[%i] parent fail, ret=%i\n", __func__, __LINE__, ret);
		return ret;
	}

	val = (gen2_clk_get_rate(&pparent) / rate) - 1;

	debug("%s[%i] MMCIF offset=%x\n", __func__, __LINE__, core->offset);

	writel(val, priv->base + core->offset);

	return 0;
}

static ulong gen2_clk_set_rate(struct clk *clk, ulong rate)
{
	/* Force correct MMC-IF divider configuration if applicable */
	gen2_clk_setup_mmcif_div(clk, rate);
	return gen2_clk_get_rate(clk);
}

static int gen2_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count != 2) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	clk->id = (args->args[0] << 16) | args->args[1];

	return 0;
}

const struct clk_ops gen2_clk_ops = {
	.enable		= gen2_clk_enable,
	.disable	= gen2_clk_disable,
	.get_rate	= gen2_clk_get_rate,
	.set_rate	= gen2_clk_set_rate,
	.of_xlate	= gen2_clk_of_xlate,
};

int gen2_clk_probe(struct udevice *dev)
{
	struct gen2_clk_priv *priv = dev_get_priv(dev);
	struct cpg_mssr_info *info =
		(struct cpg_mssr_info *)dev_get_driver_data(dev);
	fdt_addr_t rst_base;
	u32 cpg_mode;
	int ret;

	priv->base = (struct gen2_base *)devfdt_get_addr(dev);
	if (!priv->base)
		return -EINVAL;

	priv->info = info;
	ret = fdt_node_offset_by_compatible(gd->fdt_blob, -1, info->reset_node);
	if (ret < 0)
		return ret;

	rst_base = fdtdec_get_addr(gd->fdt_blob, ret, "reg");
	if (rst_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	cpg_mode = readl(rst_base + CPG_RST_MODEMR);

	priv->cpg_pll_config =
		(struct rcar_gen2_cpg_pll_config *)info->get_pll_config(cpg_mode);
	if (!priv->cpg_pll_config->extal_div)
		return -EINVAL;

	ret = clk_get_by_name(dev, "extal", &priv->clk_extal);
	if (ret < 0)
		return ret;

	if (info->extal_usb_node) {
		ret = clk_get_by_name(dev, info->extal_usb_node,
				      &priv->clk_extal_usb);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int gen2_clk_remove(struct udevice *dev)
{
	struct gen2_clk_priv *priv = dev_get_priv(dev);

	return renesas_clk_remove(priv->base, priv->info);
}
