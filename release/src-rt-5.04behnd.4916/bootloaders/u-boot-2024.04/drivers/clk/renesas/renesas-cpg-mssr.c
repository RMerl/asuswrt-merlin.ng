// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas RCar Gen3 CPG MSSR driver
 *
 * Copyright (C) 2018 Marek Vasut <marek.vasut@gmail.com>
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
#include <asm/io.h>
#include <linux/bitops.h>

#include <dt-bindings/clock/renesas-cpg-mssr.h>

#include "renesas-cpg-mssr.h"

bool renesas_clk_is_mod(struct clk *clk)
{
	return (clk->id >> 16) == CPG_MOD;
}

int renesas_clk_get_mod(struct clk *clk, struct cpg_mssr_info *info,
			const struct mssr_mod_clk **mssr)
{
	const unsigned long clkid = clk->id & 0xffff;
	int i;

	for (i = 0; i < info->mod_clk_size; i++) {
		if (info->mod_clk[i].id !=
		    (info->mod_clk_base + MOD_CLK_PACK(clkid)))
			continue;

		*mssr = &info->mod_clk[i];
		return 0;
	}

	return -ENODEV;
}

int renesas_clk_get_core(struct clk *clk, struct cpg_mssr_info *info,
			 const struct cpg_core_clk **core)
{
	const unsigned long clkid = clk->id & 0xffff;
	int i;

	for (i = 0; i < info->core_clk_size; i++) {
		if (info->core_clk[i].id != clkid)
			continue;

		*core = &info->core_clk[i];
		return 0;
	}

	return -ENODEV;
}

int renesas_clk_get_parent(struct clk *clk, struct cpg_mssr_info *info,
			   struct clk *parent)
{
	const struct cpg_core_clk *core;
	const struct mssr_mod_clk *mssr;
	int ret;

	if (renesas_clk_is_mod(clk)) {
		ret = renesas_clk_get_mod(clk, info, &mssr);
		if (ret)
			return ret;

		parent->id = mssr->parent;
	} else {
		ret = renesas_clk_get_core(clk, info, &core);
		if (ret)
			return ret;

		if (core->type == CLK_TYPE_IN)
			parent->id = ~0;	/* Top-level clock */
		else
			parent->id = core->parent;
	}

	parent->dev = clk->dev;

	return 0;
}

int renesas_clk_endisable(struct clk *clk, void __iomem *base,
			  struct cpg_mssr_info *info, bool enable)
{
	const unsigned long clkid = clk->id & 0xffff;
	const unsigned int reg = clkid / 100;
	const unsigned int bit = clkid % 100;
	const u32 bitmask = BIT(bit);

	if (!renesas_clk_is_mod(clk))
		return -EINVAL;

	debug("%s[%i] MSTP %lu=%02u/%02u %s\n", __func__, __LINE__,
	      clkid, reg, bit, enable ? "ON" : "OFF");

	if (enable) {
		clrbits_le32(base + info->control_regs[reg], bitmask);
		return wait_for_bit_le32(base + info->status_regs[reg],
				    bitmask, 0, 100, 0);
	} else {
		setbits_le32(base + info->control_regs[reg], bitmask);
		return 0;
	}
}

int renesas_clk_remove(void __iomem *base, struct cpg_mssr_info *info)
{
	unsigned int i;

	/* Stop TMU0 */
	clrbits_le32(TMU_BASE + TSTR0, TSTR0_STR0);

	/* Stop module clock */
	for (i = 0; i < info->mstp_table_size; i++) {
		clrsetbits_le32(base + info->control_regs[i],
				info->mstp_table[i].sdis,
				info->mstp_table[i].sen);

		if (info->reg_layout == CLK_REG_LAYOUT_RCAR_GEN4)
			continue;

		clrsetbits_le32(base + RMSTPCR(i),
				info->mstp_table[i].rdis,
				info->mstp_table[i].ren);
	}

	return 0;
}
