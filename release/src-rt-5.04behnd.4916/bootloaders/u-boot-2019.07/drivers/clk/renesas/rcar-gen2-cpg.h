/*
 * R-Car Gen2 Clock Pulse Generator
 *
 * Copyright (C) 2016 Cogent Embedded Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation; version 2 of the License.
 */

#ifndef __CLK_RENESAS_RCAR_GEN2_CPG_H__
#define __CLK_RENESAS_RCAR_GEN2_CPG_H__

enum rcar_gen2_clk_types {
	CLK_TYPE_GEN2_MAIN = CLK_TYPE_CUSTOM,
	CLK_TYPE_GEN2_PLL0,
	CLK_TYPE_GEN2_PLL1,
	CLK_TYPE_GEN2_PLL3,
	CLK_TYPE_GEN2_Z,
	CLK_TYPE_GEN2_LB,
	CLK_TYPE_GEN2_ADSP,
	CLK_TYPE_GEN2_SDH,
	CLK_TYPE_GEN2_SD0,
	CLK_TYPE_GEN2_SD1,
	CLK_TYPE_GEN2_QSPI,
	CLK_TYPE_GEN2_RCAN,
};

struct rcar_gen2_cpg_pll_config {
	unsigned int extal_div;
	unsigned int pll1_mult;
	unsigned int pll3_mult;
	unsigned int pll0_mult;		/* leave as zero if PLL0CR exists */
};

struct gen2_clk_priv {
	void __iomem		*base;
	struct cpg_mssr_info	*info;
	struct clk		clk_extal;
	struct clk		clk_extal_usb;
	const struct rcar_gen2_cpg_pll_config *cpg_pll_config;
};

int gen2_clk_probe(struct udevice *dev);
int gen2_clk_remove(struct udevice *dev);

extern const struct clk_ops gen2_clk_ops;

#endif
