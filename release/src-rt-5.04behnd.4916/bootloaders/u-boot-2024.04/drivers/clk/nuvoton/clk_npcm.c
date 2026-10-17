// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 *
 * Formula for calculating clock rate:
 * Fout = ((Fin / PRE_DIV) / div) / POST_DIV
 */

#include <div64.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/log2.h>
#include "clk_npcm.h"

static int clkid_to_clksel(struct npcm_clk_select *selector, int id)
{
	int i;

	for (i = 0; i < selector->num_parents; i++) {
		if (selector->parents[i].id == id)
			return selector->parents[i].clksel;
	}

	return -EINVAL;
}

static int clksel_to_clkid(struct npcm_clk_select *selector, int clksel)
{
	int i;

	for (i = 0; i < selector->num_parents; i++) {
		if (selector->parents[i].clksel == clksel)
			return selector->parents[i].id;
	}

	return -EINVAL;
}

static struct npcm_clk_pll *npcm_clk_pll_get(struct npcm_clk_data *clk_data, int id)
{
	struct npcm_clk_pll *pll = clk_data->clk_plls;
	int i;

	for (i = 0; i < clk_data->num_plls; i++) {
		if (pll->id == id)
			return pll;
		pll++;
	}

	return NULL;
}

static struct npcm_clk_select *npcm_clk_selector_get(struct npcm_clk_data *clk_data,
						     int id)
{
	struct npcm_clk_select *selector = clk_data->clk_selectors;
	int i;

	for (i = 0; i < clk_data->num_selectors; i++) {
		if (selector->id == id)
			return selector;
		selector++;
	}

	return NULL;
}

static struct npcm_clk_div *npcm_clk_divider_get(struct npcm_clk_data *clk_data,
						 int id)
{
	struct npcm_clk_div *divider = clk_data->clk_dividers;
	int i;

	for (i = 0; i < clk_data->num_dividers; i++) {
		if (divider->id == id)
			return divider;
		divider++;
	}

	return NULL;
}

static ulong npcm_clk_get_fin(struct clk *clk)
{
	struct npcm_clk_priv *priv = dev_get_priv(clk->dev);
	struct npcm_clk_select *selector;
	struct clk parent;
	ulong parent_rate;
	u32 val, clksel;
	int ret;

	selector = npcm_clk_selector_get(priv->clk_data, clk->id);
	if (!selector)
		return 0;

	if (selector->flags & FIXED_PARENT) {
		clksel = 0;
	} else {
		val = readl(priv->base + selector->reg);
		clksel = (val & selector->mask) >> (ffs(selector->mask) - 1);
	}
	parent.id = clksel_to_clkid(selector, clksel);

	ret = clk_request(clk->dev, &parent);
	if (ret)
		return 0;

	parent_rate = clk_get_rate(&parent);

	debug("fin of clk%lu = %lu\n", clk->id, parent_rate);
	return parent_rate;
}

static u32 npcm_clk_get_div(struct clk *clk)
{
	struct npcm_clk_priv *priv = dev_get_priv(clk->dev);
	struct npcm_clk_div *divider;
	u32 val, div;

	divider = npcm_clk_divider_get(priv->clk_data, clk->id);
	if (!divider)
		return 0;

	val = readl(priv->base + divider->reg);
	div = (val & divider->mask) >> (ffs(divider->mask) - 1);
	if (divider->flags & DIV_TYPE1)
		div = div + 1;
	else
		div = 1 << div;

	if (divider->flags & PRE_DIV2)
		div = div << 1;

	return div;
}

static int npcm_clk_set_div(struct clk *clk, u32 div)
{
	struct npcm_clk_priv *priv = dev_get_priv(clk->dev);
	struct npcm_clk_div *divider;
	u32 val, clkdiv;

	divider = npcm_clk_divider_get(priv->clk_data, clk->id);
	if (!divider)
		return -EINVAL;

	if (divider->flags & DIV_RO)
		return 0;

	if (divider->flags & PRE_DIV2)
		div = div >> 1;

	if (divider->flags & DIV_TYPE1)
		clkdiv = div - 1;
	else
		clkdiv = ilog2(div);

	if (clkdiv > (divider->mask >> (ffs(divider->mask) - 1))) {
		printf("clkdiv(%d) for clk(%ld) is over limit\n",
		       clkdiv, clk->id);
		return -EINVAL;
	}

	val = readl(priv->base + divider->reg);
	val &= ~divider->mask;
	val |= (clkdiv << (ffs(divider->mask) - 1)) & divider->mask;
	writel(val, priv->base + divider->reg);

	return 0;
}

static ulong npcm_clk_get_fout(struct clk *clk)
{
	ulong parent_rate;
	u32 div;

	parent_rate = npcm_clk_get_fin(clk);
	if (!parent_rate)
		return -EINVAL;

	div = npcm_clk_get_div(clk);
	if (!div)
		return -EINVAL;

	debug("fout of clk%lu = (%lu / %u)\n", clk->id, parent_rate, div);
	return (parent_rate / div);
}

static ulong npcm_clk_get_pll_fout(struct clk *clk)
{
	struct npcm_clk_priv *priv = dev_get_priv(clk->dev);
	struct npcm_clk_pll *pll;
	struct clk parent;
	ulong parent_rate;
	ulong fbdv, indv, otdv1, otdv2;
	u32 val;
	u64 ret;

	pll = npcm_clk_pll_get(priv->clk_data, clk->id);
	if (!pll)
		return -ENODEV;

	parent.id = pll->parent_id;
	ret = clk_request(clk->dev, &parent);
	if (ret)
		return ret;

	parent_rate = clk_get_rate(&parent);

	val = readl(priv->base + pll->reg);
	indv = FIELD_GET(PLLCON_INDV, val);
	fbdv = FIELD_GET(PLLCON_FBDV, val);
	otdv1 = FIELD_GET(PLLCON_OTDV1, val);
	otdv2 = FIELD_GET(PLLCON_OTDV2, val);

	ret = (u64)parent_rate * fbdv;
	do_div(ret, indv * otdv1 * otdv2);
	if (pll->flags & POST_DIV2)
		do_div(ret, 2);

	debug("fout of pll(id %lu) = %llu\n", clk->id, ret);
	return ret;
}

static ulong npcm_clk_get_rate(struct clk *clk)
{
	struct npcm_clk_priv *priv = dev_get_priv(clk->dev);
	struct npcm_clk_data *clk_data = priv->clk_data;
	struct clk refclk;
	int ret;

	debug("%s: id %lu\n", __func__, clk->id);
	if (clk->id == clk_data->refclk_id) {
		ret = clk_get_by_name(clk->dev, "refclk", &refclk);
		if (!ret)
			return clk_get_rate(&refclk);
		else
			return ret;
	}

	if (clk->id >= clk_data->pll0_id &&
	    clk->id < clk_data->pll0_id + clk_data->num_plls)
		return npcm_clk_get_pll_fout(clk);
	else
		return npcm_clk_get_fout(clk);
}

static ulong npcm_clk_set_rate(struct clk *clk, ulong rate)
{
	ulong parent_rate;
	u32 div;
	int ret;

	debug("%s: id %lu, rate %lu\n", __func__, clk->id, rate);
	parent_rate = npcm_clk_get_fin(clk);
	if (!parent_rate)
		return -EINVAL;

	div = DIV_ROUND_UP(parent_rate, rate);
	ret = npcm_clk_set_div(clk, div);
	if (ret)
		return ret;

	debug("%s: rate %lu, new rate %lu\n", __func__, rate, npcm_clk_get_rate(clk));
	return npcm_clk_get_rate(clk);
}

static int npcm_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct npcm_clk_priv *priv = dev_get_priv(clk->dev);
	struct npcm_clk_select *selector;
	int clksel;
	u32 val;

	debug("%s: id %lu, parent %lu\n", __func__, clk->id, parent->id);
	selector = npcm_clk_selector_get(priv->clk_data, clk->id);
	if (!selector)
		return -EINVAL;

	clksel = clkid_to_clksel(selector, parent->id);
	if (clksel < 0)
		return -EINVAL;

	val = readl(priv->base + selector->reg);
	val &= ~selector->mask;
	val |= clksel << (ffs(selector->mask) - 1);
	writel(val, priv->base + selector->reg);

	return 0;
}

static int npcm_clk_request(struct clk *clk)
{
	struct npcm_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id >= priv->num_clks)
		return -EINVAL;

	return 0;
}

const struct clk_ops npcm_clk_ops = {
	.get_rate = npcm_clk_get_rate,
	.set_rate = npcm_clk_set_rate,
	.set_parent = npcm_clk_set_parent,
	.request = npcm_clk_request,
};
