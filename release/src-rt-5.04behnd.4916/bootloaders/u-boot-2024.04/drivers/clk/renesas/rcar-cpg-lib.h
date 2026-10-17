/* SPDX-License-Identifier: GPL-2.0 */
/*
 * R-Car Gen3 Clock Pulse Generator Library
 *
 * Copyright (C) 2015-2018 Glider bvba
 * Copyright (C) 2019 Renesas Electronics Corp.
 *
 * Based on clk-rcar-gen3.c
 *
 * Copyright (C) 2015 Renesas Electronics Corp.
 */

#ifndef __CLK_RENESAS_RCAR_CPG_LIB_H__
#define __CLK_RENESAS_RCAR_CPG_LIB_H__

s64 rcar_clk_get_rate64_div_table(unsigned int parent, u64 parent_rate,
				  void __iomem *reg, const u32 mask,
				  const struct clk_div_table *table, char *name);

int rcar_clk_set_rate64_div_table(unsigned int parent, u64 parent_rate, ulong rate,
				  void __iomem *reg, const u32 mask,
				  const struct clk_div_table *table, char *name);

s64 rcar_clk_get_rate64_sdh(unsigned int parent, u64 parent_rate, void __iomem *reg);
s64 rcar_clk_get_rate64_sd(unsigned int parent, u64 parent_rate, void __iomem *reg);
s64 rcar_clk_get_rate64_rpc(unsigned int parent, u64 parent_rate, void __iomem *reg);
u64 rcar_clk_get_rate64_rpcd2(unsigned int parent, u64 parent_rate);
int rcar_clk_set_rate64_sdh(unsigned int parent, u64 parent_rate, ulong rate,
			    void __iomem *reg);
int rcar_clk_set_rate64_sd(unsigned int parent, u64 parent_rate, ulong rate,
			   void __iomem *reg);

#endif
