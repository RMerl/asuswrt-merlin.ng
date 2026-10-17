// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Copyright (C) 2018 SiFive, Inc.
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
 *
 * References:
 * - SiFive FU540-C000 manual v1p0, Chapter 7 "Clocking and Reset"
 */

#include <dt-bindings/clock/sifive-fu540-prci.h>

#include "sifive-prci.h"

/* PRCI integration data for each WRPLL instance */
static struct __prci_wrpll_data __prci_corepll_data = {
	.cfg0_offs = PRCI_COREPLLCFG0_OFFSET,
	.cfg1_offs = PRCI_COREPLLCFG1_OFFSET,
	.enable_bypass = sifive_prci_coreclksel_use_hfclk,
	.disable_bypass = sifive_prci_coreclksel_use_corepll,
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

/* Linux clock framework integration */
static const struct __prci_clock_ops sifive_fu540_prci_wrpll_clk_ops = {
	.set_rate = sifive_prci_wrpll_set_rate,
	.round_rate = sifive_prci_wrpll_round_rate,
	.recalc_rate = sifive_prci_wrpll_recalc_rate,
	.enable_clk = sifive_prci_clock_enable,
};

static const struct __prci_clock_ops sifive_fu540_prci_tlclksel_clk_ops = {
	.recalc_rate = sifive_prci_tlclksel_recalc_rate,
};

/* List of clock controls provided by the PRCI */
struct __prci_clock __prci_init_clocks_fu540[] = {
	[PRCI_CLK_COREPLL] = {
		.name = "corepll",
		.parent_name = "hfclk",
		.ops = &sifive_fu540_prci_wrpll_clk_ops,
		.pwd = &__prci_corepll_data,
	},
	[PRCI_CLK_DDRPLL] = {
		.name = "ddrpll",
		.parent_name = "hfclk",
		.ops = &sifive_fu540_prci_wrpll_clk_ops,
		.pwd = &__prci_ddrpll_data,
	},
	[PRCI_CLK_GEMGXLPLL] = {
		.name = "gemgxlpll",
		.parent_name = "hfclk",
		.ops = &sifive_fu540_prci_wrpll_clk_ops,
		.pwd = &__prci_gemgxlpll_data,
	},
	[PRCI_CLK_TLCLK] = {
		.name = "tlclk",
		.parent_name = "corepll",
		.ops = &sifive_fu540_prci_tlclksel_clk_ops,
	},
};
