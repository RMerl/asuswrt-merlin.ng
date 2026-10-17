// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas RCar Gen3 CPG MSSR driver
 *
 * Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on the following driver from Linux kernel:
 * r8a7796 Clock Pulse Generator / Module Standby and Software Reset
 *
 * Copyright (C) 2016 Glider bvba
 */

#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <wait_bit.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>

#include <dt-bindings/clock/renesas-cpg-mssr.h>

#include "renesas-cpg-mssr.h"
#include "rcar-gen3-cpg.h"
#include "rcar-cpg-lib.h"

#define SDnSRCFC_SHIFT		2
#define STPnHCK_TABLE		(CPG_SDCKCR_STPnHCK >> SDnSRCFC_SHIFT)

/* Non-constant mask variant of FIELD_GET/FIELD_PREP */
#define field_get(_mask, _reg) (((_reg) & (_mask)) >> (ffs(_mask) - 1))
#define field_prep(_mask, _val) (((_val) << (ffs(_mask) - 1)) & (_mask))

static const struct clk_div_table cpg_sdh_div_table[] = {
	{ 0, 1 }, { 1, 2 }, { STPnHCK_TABLE | 2, 4 }, { STPnHCK_TABLE | 3, 8 },
	{ STPnHCK_TABLE | 4, 16 }, { 0, 0 },
};

static const struct clk_div_table cpg_sd_div_table[] = {
	{ 0, 2 }, { 1, 4 }, { 0, 0 },
};

static const struct clk_div_table cpg_rpc_div_table[] = {
	{ 1, 2 }, { 3, 4 }, { 5, 6 }, { 7, 8 }, { 0, 0 },
};

static unsigned int rcar_clk_get_table_div(const struct clk_div_table *table,
					   const u32 value)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->val == value)
			return clkt->div;
	return 0;
}

static int rcar_clk_get_table_val(const struct clk_div_table *table,
				  unsigned int div)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div == div)
			return clkt->val;
	return -EINVAL;
}

s64 rcar_clk_get_rate64_div_table(unsigned int parent, u64 parent_rate,
				  void __iomem *reg, const u32 mask,
				  const struct clk_div_table *table, char *name)
{
	u32 value, div;
	u64 rate;

	value = field_get(mask, readl(reg));
	div = rcar_clk_get_table_div(table, value);
	if (!div)
		return -EINVAL;

	rate = parent_rate / div;
	debug("%s[%i] %s clk: parent=%i div=%u => rate=%llu\n",
	      __func__, __LINE__, name, parent, div, rate);

	return rate;
}

int rcar_clk_set_rate64_div_table(unsigned int parent, u64 parent_rate, ulong rate,
				  void __iomem *reg, const u32 mask,
				  const struct clk_div_table *table, char *name)
{
	u32 value = 0, div = 0;

	div = DIV_ROUND_CLOSEST(parent_rate, rate);
	value = rcar_clk_get_table_val(table, div);
	if (value < 0)
		return value;

	clrsetbits_le32(reg, mask, field_prep(mask, value));

	debug("%s[%i] %s clk: parent=%i div=%u rate=%lu => val=%u\n",
	      __func__, __LINE__, name, parent, div, rate, value);

	return 0;
}

s64 rcar_clk_get_rate64_rpc(unsigned int parent, u64 parent_rate, void __iomem *reg)
{
	return rcar_clk_get_rate64_div_table(parent, parent_rate, reg,
					     CPG_RPCCKCR_DIV_PRE_MASK,
					     cpg_rpc_div_table, "RPC");
}

u64 rcar_clk_get_rate64_rpcd2(unsigned int parent, u64 parent_rate)
{
	u64 rate = 0;

	rate = parent_rate / 2;
	debug("%s[%i] RPCD2 clk: parent=%i => rate=%llu\n",
	      __func__, __LINE__, parent, rate);

	return rate;
}

s64 rcar_clk_get_rate64_sdh(unsigned int parent, u64 parent_rate, void __iomem *reg)
{
	/*
	 * This takes STPnHCK and STPnCK bits into consideration
	 * in the table look up too, hence the inobvious GENMASK
	 * below. Bits [7:5] always read zero, so this is OKish.
	 */
	return rcar_clk_get_rate64_div_table(parent, parent_rate, reg,
					     CPG_SDCKCR_SRCFC_MASK |
					     GENMASK(9, 5),
					     cpg_sdh_div_table, "SDH");
}

s64 rcar_clk_get_rate64_sd(unsigned int parent, u64 parent_rate, void __iomem *reg)
{
	return rcar_clk_get_rate64_div_table(parent, parent_rate, reg,
					     CPG_SDCKCR_FC_MASK,
					     cpg_sd_div_table, "SD");
}

int rcar_clk_set_rate64_sdh(unsigned int parent, u64 parent_rate, ulong rate,
			    void __iomem *reg)
{
	/*
	 * This takes STPnHCK and STPnCK bits into consideration
	 * in the table look up too, hence the inobvious GENMASK
	 * below. Bits [7:5] always read zero, so this is OKish.
	 */
	return rcar_clk_set_rate64_div_table(parent, parent_rate, rate, reg,
					     CPG_SDCKCR_SRCFC_MASK |
					     GENMASK(9, 5),
					     cpg_sdh_div_table, "SDH");
}

int rcar_clk_set_rate64_sd(unsigned int parent, u64 parent_rate, ulong rate,
			   void __iomem *reg)
{
	return rcar_clk_set_rate64_div_table(parent, parent_rate, rate, reg,
					     CPG_SDCKCR_FC_MASK,
					     cpg_sd_div_table, "SD");
}
