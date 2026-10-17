// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2021 SiFive, Inc.
 * Wesley Terpstra
 * Paul Walmsley
 * Zong Li
 * Pragnesh Patel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <dt-bindings/clock/sifive-fu740-prci.h>
#include "sifive-prci.h"
#include <asm/io.h>

int sifive_prci_fu740_pcieauxclk_enable(struct __prci_clock *pc, bool enable)
{
	struct __prci_wrpll_data *pwd = pc->pwd;
	struct __prci_data *pd = pc->pd;
	u32 v;

	if (pwd->cfg1_offs != PRCI_PCIEAUXCFG1_OFFSET)
		return -EINVAL;

	v = readl(pd->va + pwd->cfg1_offs);
	v = enable ? (v | PRCI_PCIEAUXCFG1_MASK) : (v & ~PRCI_PCIEAUXCFG1_MASK);
	writel(v, pd->va + pwd->cfg1_offs);

	return 0;
}

/* PRCI integration data for each WRPLL instance */
static struct __prci_wrpll_data __prci_corepll_data = {
	.cfg0_offs = PRCI_COREPLLCFG0_OFFSET,
	.cfg1_offs = PRCI_COREPLLCFG1_OFFSET,
	.enable_bypass = sifive_prci_coreclksel_use_hfclk,
	.disable_bypass = sifive_prci_coreclksel_use_final_corepll,
};

static struct __prci_wrpll_data __prci_ddrpll_data = {
	.cfg0_offs = PRCI_DDRPLLCFG0_OFFSET,
	.cfg1_offs = PRCI_DDRPLLCFG1_OFFSET,
	.release_reset = sifive_prci_ddr_release_reset,
};

static struct __prci_wrpll_data __prci_gemgxlpll_data = {
	.cfg0_offs = PRCI_GEMGXLPLLCFG0_OFFSET,
	.cfg1_offs = PRCI_GEMGXLPLLCFG1_OFFSET,
	.release_reset = sifive_prci_ethernet_release_reset,
};

static struct __prci_wrpll_data __prci_dvfscorepll_data = {
	.cfg0_offs = PRCI_DVFSCOREPLLCFG0_OFFSET,
	.cfg1_offs = PRCI_DVFSCOREPLLCFG1_OFFSET,
	.enable_bypass = sifive_prci_corepllsel_use_corepll,
	.disable_bypass = sifive_prci_corepllsel_use_dvfscorepll,
};

static struct __prci_wrpll_data __prci_hfpclkpll_data = {
	.cfg0_offs = PRCI_HFPCLKPLLCFG0_OFFSET,
	.cfg1_offs = PRCI_HFPCLKPLLCFG1_OFFSET,
	.enable_bypass = sifive_prci_hfpclkpllsel_use_hfclk,
	.disable_bypass = sifive_prci_hfpclkpllsel_use_hfpclkpll,
};

static struct __prci_wrpll_data __prci_cltxpll_data = {
	.cfg0_offs = PRCI_CLTXPLLCFG0_OFFSET,
	.cfg1_offs = PRCI_CLTXPLLCFG1_OFFSET,
	.release_reset = sifive_prci_cltx_release_reset,
};

static struct __prci_wrpll_data __prci_pcieaux_data = {
	.cfg1_offs = PRCI_PCIEAUXCFG1_OFFSET,
};

/* Linux clock framework integration */

static const struct __prci_clock_ops sifive_fu740_prci_wrpll_clk_ops = {
	.set_rate = sifive_prci_wrpll_set_rate,
	.round_rate = sifive_prci_wrpll_round_rate,
	.recalc_rate = sifive_prci_wrpll_recalc_rate,
	.enable_clk = sifive_prci_clock_enable,
};

static const struct __prci_clock_ops sifive_fu740_prci_tlclksel_clk_ops = {
	.recalc_rate = sifive_prci_tlclksel_recalc_rate,
};

static const struct __prci_clock_ops sifive_fu740_prci_hfpclkplldiv_clk_ops = {
	.recalc_rate = sifive_prci_hfpclkplldiv_recalc_rate,
};

static const struct __prci_clock_ops sifive_fu740_prci_pcieaux_clk_ops = {
	.enable_clk = sifive_prci_fu740_pcieauxclk_enable,
};

/* List of clock controls provided by the PRCI */
struct __prci_clock __prci_init_clocks_fu740[] = {
	[FU740_PRCI_CLK_COREPLL] = {
		.name = "corepll",
		.parent_name = "hfclk",
		.ops = &sifive_fu740_prci_wrpll_clk_ops,
		.pwd = &__prci_corepll_data,
	},
	[FU740_PRCI_CLK_DDRPLL] = {
		.name = "ddrpll",
		.parent_name = "hfclk",
		.ops = &sifive_fu740_prci_wrpll_clk_ops,
		.pwd = &__prci_ddrpll_data,
	},
	[FU740_PRCI_CLK_GEMGXLPLL] = {
		.name = "gemgxlpll",
		.parent_name = "hfclk",
		.ops = &sifive_fu740_prci_wrpll_clk_ops,
		.pwd = &__prci_gemgxlpll_data,
	},
	[FU740_PRCI_CLK_DVFSCOREPLL] = {
		.name = "dvfscorepll",
		.parent_name = "hfclk",
		.ops = &sifive_fu740_prci_wrpll_clk_ops,
		.pwd = &__prci_dvfscorepll_data,
	},
	[FU740_PRCI_CLK_HFPCLKPLL] = {
		.name = "hfpclkpll",
		.parent_name = "hfclk",
		.ops = &sifive_fu740_prci_wrpll_clk_ops,
		.pwd = &__prci_hfpclkpll_data,
	},
	[FU740_PRCI_CLK_CLTXPLL] = {
		.name = "cltxpll",
		.parent_name = "hfclk",
		.ops = &sifive_fu740_prci_wrpll_clk_ops,
		.pwd = &__prci_cltxpll_data,
	},
	[FU740_PRCI_CLK_TLCLK] = {
		.name = "tlclk",
		.parent_name = "corepll",
		.ops = &sifive_fu740_prci_tlclksel_clk_ops,
	},
	[FU740_PRCI_CLK_PCLK] = {
		.name = "pclk",
		.parent_name = "hfpclkpll",
		.ops = &sifive_fu740_prci_hfpclkplldiv_clk_ops,
	},
	[FU740_PRCI_CLK_PCIE_AUX] {
		.name = "pcieaux",
		.parent_name = "",
		.ops = &sifive_fu740_prci_pcieaux_clk_ops,
		.pwd = &__prci_pcieaux_data,
	}
};
