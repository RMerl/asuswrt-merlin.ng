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
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <errno.h>
#include <log.h>
#include <wait_bit.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>
#include <reset-uclass.h>

#include <dt-bindings/clock/renesas-cpg-mssr.h>

#include "renesas-cpg-mssr.h"
#include "rcar-gen3-cpg.h"
#include "rcar-cpg-lib.h"

#define CPG_PLL0CR		0x00d8
#define CPG_PLL2CR		0x002c
#define CPG_PLL4CR		0x01f4

#define SD0CKCR1		0x08a4

static const struct clk_div_table gen3_cpg_rpcsrc_div_table[] = {
	{ 2, 5 }, { 3, 6 }, { 0, 0 },
};

static const struct clk_div_table gen4_cpg_rpcsrc_div_table[] = {
	{ 0, 4 }, { 1, 6 }, { 2, 5 }, { 3, 6 }, { 0, 0 },
};

static const struct clk_div_table r8a77970_cpg_sd0h_div_table[] = {
	{  0,  2 }, {  1,  3 }, {  2,  4 }, {  3,  6 },
	{  4,  8 }, {  5, 12 }, {  6, 16 }, {  7, 18 },
	{  8, 24 }, { 10, 36 }, { 11, 48 }, {  0,  0 },
};

static const struct clk_div_table r8a77970_cpg_sd0_div_table[] = {
	{  4,  8 }, {  5, 12 }, {  6, 16 }, {  7, 18 },
	{  8, 24 }, { 10, 36 }, { 11, 48 }, { 12, 10 },
	{  0,  0 },
};

static int gen3_clk_get_parent(struct gen3_clk_priv *priv, struct clk *clk,
			       struct cpg_mssr_info *info, struct clk *parent)
{
	const struct cpg_core_clk *core;
	u8 shift;
	int ret;

	if (!renesas_clk_is_mod(clk)) {
		ret = renesas_clk_get_core(clk, info, &core);
		if (ret)
			return ret;

		if (core->type == CLK_TYPE_GEN3_MDSEL) {
			shift = priv->cpg_mode & BIT(core->offset) ? 16 : 0;
			parent->dev = clk->dev;
			parent->id = core->parent >> shift;
			parent->id &= 0xffff;
			return 0;
		}
	}

	return renesas_clk_get_parent(clk, info, parent);
}

static int gen3_clk_enable(struct clk *clk)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);

	return renesas_clk_endisable(clk, priv->base, priv->info, true);
}

static int gen3_clk_disable(struct clk *clk)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);

	return renesas_clk_endisable(clk, priv->base, priv->info, false);
}

static u64 gen3_clk_get_rate64(struct clk *clk);

static int gen3_clk_setup_sdif_div(struct clk *clk, ulong rate)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);
	struct cpg_mssr_info *info = priv->info;
	const struct cpg_core_clk *core;
	struct clk parent, grandparent;
	int ret;

	/*
	 * The clk may be either CPG_MOD or core clock, in case this is MOD
	 * clock, use core clock one level up, otherwise use the clock as-is.
	 * Note that parent clock here always represents core clock. Also note
	 * that grandparent clock are the parent clock of the core clock here.
	 */
	if (renesas_clk_is_mod(clk)) {
		ret = gen3_clk_get_parent(priv, clk, info, &parent);
		if (ret) {
			printf("%s[%i] parent fail, ret=%i\n", __func__, __LINE__, ret);
			return ret;
		}
	} else {
		parent = *clk;
	}

	if (renesas_clk_is_mod(&parent))
		return 0;

	ret = renesas_clk_get_core(&parent, info, &core);
	if (ret)
		return ret;

	ret = renesas_clk_get_parent(&parent, info, &grandparent);
	if (ret) {
		printf("%s[%i] grandparent fail, ret=%i\n", __func__, __LINE__, ret);
		return ret;
	}

	switch (core->type) {
	case CLK_TYPE_GEN3_SDH:
		fallthrough;
	case CLK_TYPE_GEN4_SDH:
		return rcar_clk_set_rate64_sdh(core->parent,
					       gen3_clk_get_rate64(&grandparent),
					       rate, priv->base + core->offset);

	case CLK_TYPE_GEN3_SD:
		fallthrough;
	case CLK_TYPE_GEN4_SD:
		return rcar_clk_set_rate64_sd(core->parent,
					      gen3_clk_get_rate64(&grandparent),
					      rate, priv->base + core->offset);

	case CLK_TYPE_R8A77970_SD0:
		return rcar_clk_set_rate64_div_table(core->parent,
						     gen3_clk_get_rate64(&grandparent),
						     rate, priv->base + core->offset,
						     CPG_SDCKCR_SD0FC_MASK,
						     r8a77970_cpg_sd0_div_table, "SD");
	}

	return 0;
}

static u64 gen3_clk_get_rate64_pll_mul_reg(struct gen3_clk_priv *priv,
					   struct clk *parent,
					   u32 mul_reg, u32 mult, u32 div,
					   char *name)
{
	u32 value;
	u64 rate;

	if (mul_reg) {
		value = readl(priv->base + mul_reg);
		mult = (((value >> 24) & 0x7f) + 1) * 2;
		div = 1;
	}

	rate = (gen3_clk_get_rate64(parent) * mult) / div;

	debug("%s[%i] %s clk: mult=%u div=%u => rate=%llu\n",
	      __func__, __LINE__, name, mult, div, rate);
	return rate;
}

static u64 gen3_clk_get_rate64(struct clk *clk)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);
	struct cpg_mssr_info *info = priv->info;
	struct clk parent;
	const struct cpg_core_clk *core;
	const struct rcar_gen3_cpg_pll_config *gen3_pll_config =
					priv->gen3_cpg_pll_config;
	const struct rcar_gen4_cpg_pll_config *gen4_pll_config =
					priv->gen4_cpg_pll_config;
	u32 value, div;
	u64 rate = 0;
	u8 shift;
	int ret;

	debug("%s[%i] Clock: id=%lu\n", __func__, __LINE__, clk->id);

	ret = gen3_clk_get_parent(priv, clk, info, &parent);
	if (ret) {
		printf("%s[%i] parent fail, ret=%i\n", __func__, __LINE__, ret);
		return ret;
	}

	if (renesas_clk_is_mod(clk)) {
		rate = gen3_clk_get_rate64(&parent);
		debug("%s[%i] MOD clk: parent=%lu => rate=%llu\n",
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
			debug("%s[%i] EXTAL clk: rate=%llu\n",
			      __func__, __LINE__, rate);
			return rate;
		}

		if (core->id == info->clk_extalr_id) {
			rate = clk_get_rate(&priv->clk_extalr);
			debug("%s[%i] EXTALR clk: rate=%llu\n",
			      __func__, __LINE__, rate);
			return rate;
		}

		return -EINVAL;

	case CLK_TYPE_GEN3_MAIN:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, 1, gen3_pll_config->extal_div,
						"MAIN");

	case CLK_TYPE_GEN3_PLL0:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						CPG_PLL0CR, 0, 0, "PLL0");

	case CLK_TYPE_GEN3_PLL1:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, gen3_pll_config->pll1_mult,
						gen3_pll_config->pll1_div,
						"PLL1");

	case CLK_TYPE_GEN3_PLL2:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						CPG_PLL2CR, 0, 0, "PLL2");

	case CLK_TYPE_GEN3_PLL3:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, gen3_pll_config->pll3_mult,
						gen3_pll_config->pll3_div,
						"PLL3");

	case CLK_TYPE_GEN3_PLL4:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						CPG_PLL4CR, 0, 0, "PLL4");

	case CLK_TYPE_GEN4_MAIN:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, 1, gen4_pll_config->extal_div,
						"MAIN");

	case CLK_TYPE_GEN4_PLL1:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, gen4_pll_config->pll1_mult,
						gen4_pll_config->pll1_div,
						"PLL1");

	case CLK_TYPE_GEN4_PLL2:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, gen4_pll_config->pll2_mult,
						gen4_pll_config->pll2_div,
						"PLL2");

	case CLK_TYPE_GEN4_PLL2X_3X:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						core->offset, 0, 0, "PLL2X_3X");

	case CLK_TYPE_GEN4_PLL3:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, gen4_pll_config->pll3_mult,
						gen4_pll_config->pll3_div,
						"PLL3");

	case CLK_TYPE_GEN4_PLL4:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, gen4_pll_config->pll4_mult,
						gen4_pll_config->pll4_div,
						"PLL4");

	case CLK_TYPE_GEN4_PLL5:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, gen4_pll_config->pll5_mult,
						gen4_pll_config->pll5_div,
						"PLL5");

	case CLK_TYPE_GEN4_PLL6:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, gen4_pll_config->pll6_mult,
						gen4_pll_config->pll6_div,
						"PLL6");

	case CLK_TYPE_GEN4_PLL7:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, gen4_pll_config->pll7_mult,
						gen4_pll_config->pll7_div,
						"PLL7");

	case CLK_TYPE_FF:
		return gen3_clk_get_rate64_pll_mul_reg(priv, &parent,
						0, core->mult, core->div,
						"FIXED");

	case CLK_TYPE_GEN3_MDSEL:
		shift = priv->cpg_mode & BIT(core->offset) ? 16 : 0;
		div = (core->div >> shift) & 0xffff;
		rate = gen3_clk_get_rate64(&parent) / div;
		debug("%s[%i] PE clk: parent=%i div=%u => rate=%llu\n",
		      __func__, __LINE__, (core->parent >> shift) & 0xffff,
		      div, rate);
		return rate;

	case CLK_TYPE_GEN4_SDSRC:
		div = ((readl(priv->base + SD0CKCR1) >> 29) & 0x03) + 4;
		rate = gen3_clk_get_rate64(&parent) / div;
		debug("%s[%i] SDSRC clk: parent=%i div=%u => rate=%llu\n",
		      __func__, __LINE__, core->parent, div, rate);
		return rate;

	case CLK_TYPE_GEN3_SDH:	/* Fixed factor 1:1 */
		fallthrough;
	case CLK_TYPE_GEN4_SDH:	/* Fixed factor 1:1 */
		return rcar_clk_get_rate64_sdh(core->parent,
					       gen3_clk_get_rate64(&parent),
					       priv->base + core->offset);

	case CLK_TYPE_R8A77970_SD0H:
		return rcar_clk_get_rate64_div_table(core->parent,
						     gen3_clk_get_rate64(&parent),
						     priv->base + core->offset,
						     CPG_SDCKCR_SDHFC_MASK,
						     r8a77970_cpg_sd0h_div_table, "SDH");

	case CLK_TYPE_GEN3_SD:
		fallthrough;
	case CLK_TYPE_GEN4_SD:
		return rcar_clk_get_rate64_sd(core->parent,
					      gen3_clk_get_rate64(&parent),
					      priv->base + core->offset);

	case CLK_TYPE_R8A77970_SD0:
		return rcar_clk_get_rate64_div_table(core->parent,
						     gen3_clk_get_rate64(&parent),
						     priv->base + core->offset,
						     CPG_SDCKCR_SD0FC_MASK,
						     r8a77970_cpg_sd0_div_table, "SD");

	case CLK_TYPE_GEN3_RPCSRC:
		return rcar_clk_get_rate64_div_table(core->parent,
						     gen3_clk_get_rate64(&parent),
						     priv->base + CPG_RPCCKCR,
						     CPG_RPCCKCR_DIV_POST_MASK,
						     gen3_cpg_rpcsrc_div_table,
						     "RPCSRC");

	case CLK_TYPE_GEN4_RPCSRC:
		return rcar_clk_get_rate64_div_table(core->parent,
						     gen3_clk_get_rate64(&parent),
						     priv->base + CPG_RPCCKCR,
						     CPG_RPCCKCR_DIV_POST_MASK,
						     gen4_cpg_rpcsrc_div_table,
						     "RPCSRC");

	case CLK_TYPE_GEN3_D3_RPCSRC:
	case CLK_TYPE_GEN3_E3_RPCSRC:
		/*
		 * Register RPCSRC as fixed factor clock based on the
		 * MD[4:1] pins and CPG_RPCCKCR[4:3] register value for
		 * which has been set prior to booting the kernel.
		 */
		value = (readl(priv->base + CPG_RPCCKCR) & GENMASK(4, 3)) >> 3;

		switch (value) {
		case 0:
			div = 5;
			break;
		case 1:
			div = 3;
			break;
		case 2:
			div = core->div;
			break;
		case 3:
		default:
			div = 2;
			break;
		}

		rate = gen3_clk_get_rate64(&parent) / div;
		debug("%s[%i] E3/D3 RPCSRC clk: parent=%i div=%u => rate=%llu\n",
		      __func__, __LINE__, (core->parent >> 16) & 0xffff, div, rate);

		return rate;

	case CLK_TYPE_GEN3_RPC:
	case CLK_TYPE_GEN4_RPC:
		return rcar_clk_get_rate64_rpc(core->parent,
					       gen3_clk_get_rate64(&parent),
					       priv->base + CPG_RPCCKCR);

	case CLK_TYPE_GEN3_RPCD2:
	case CLK_TYPE_GEN4_RPCD2:
		return rcar_clk_get_rate64_rpcd2(core->parent,
						 gen3_clk_get_rate64(&parent));

	}

	printf("%s[%i] unknown fail\n", __func__, __LINE__);

	return -ENOENT;
}

static ulong gen3_clk_get_rate(struct clk *clk)
{
	return gen3_clk_get_rate64(clk);
}

static ulong gen3_clk_set_rate(struct clk *clk, ulong rate)
{
	/* Force correct SD-IF divider configuration if applicable */
	gen3_clk_setup_sdif_div(clk, rate);
	return gen3_clk_get_rate64(clk);
}

static int gen3_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count != 2) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	clk->id = (args->args[0] << 16) | args->args[1];

	return 0;
}

const struct clk_ops gen3_clk_ops = {
	.enable		= gen3_clk_enable,
	.disable	= gen3_clk_disable,
	.get_rate	= gen3_clk_get_rate,
	.set_rate	= gen3_clk_set_rate,
	.of_xlate	= gen3_clk_of_xlate,
};

static int gen3_clk_probe(struct udevice *dev)
{
	struct gen3_clk_priv *priv = dev_get_priv(dev);
	struct cpg_mssr_info *info =
		(struct cpg_mssr_info *)dev_get_driver_data(dev);
	const void *pll_config;
	fdt_addr_t rst_base;
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	priv->info = info;
	ret = fdt_node_offset_by_compatible(gd->fdt_blob, -1, info->reset_node);
	if (ret < 0)
		return ret;

	rst_base = fdtdec_get_addr(gd->fdt_blob, ret, "reg");
	if (rst_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->cpg_mode = readl(rst_base + info->reset_modemr_offset);

	pll_config = info->get_pll_config(priv->cpg_mode);

	if (info->reg_layout == CLK_REG_LAYOUT_RCAR_GEN2_AND_GEN3) {
		priv->info->status_regs = mstpsr;
		priv->info->control_regs = smstpcr;
		priv->info->reset_regs = srcr;
		priv->info->reset_clear_regs = srstclr;
		priv->gen3_cpg_pll_config = pll_config;
		if (!priv->gen3_cpg_pll_config->extal_div)
			return -EINVAL;
	} else if (info->reg_layout == CLK_REG_LAYOUT_RCAR_GEN4) {
		priv->info->status_regs = mstpsr_for_gen4;
		priv->info->control_regs = mstpcr_for_gen4;
		priv->info->reset_regs = srcr_for_gen4;
		priv->info->reset_clear_regs = srstclr_for_gen4;
		priv->gen4_cpg_pll_config = pll_config;
		if (!priv->gen4_cpg_pll_config->extal_div)
			return -EINVAL;
	} else {
		return -EINVAL;
	}

	ret = clk_get_by_name(dev, "extal", &priv->clk_extal);
	if (ret < 0)
		return ret;

	if (info->extalr_node) {
		ret = clk_get_by_name(dev, info->extalr_node, &priv->clk_extalr);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int gen3_clk_remove(struct udevice *dev)
{
	struct gen3_clk_priv *priv = dev_get_priv(dev);

	return renesas_clk_remove(priv->base, priv->info);
}

U_BOOT_DRIVER(clk_gen3) = {
	.name		= "clk_gen3",
	.id		= UCLASS_CLK,
	.priv_auto	= sizeof(struct gen3_clk_priv),
	.ops		= &gen3_clk_ops,
	.probe		= gen3_clk_probe,
	.remove		= gen3_clk_remove,
	.flags          = DM_FLAG_OS_PREPARE | DM_FLAG_VITAL,
};

static int gen3_reset_assert(struct reset_ctl *reset_ctl)
{
	struct udevice *cdev = (struct udevice *)dev_get_driver_data(reset_ctl->dev);
	struct gen3_clk_priv *priv = dev_get_priv(cdev);
	unsigned int packed_id = MOD_CLK_PACK(reset_ctl->id);
	unsigned int reg = packed_id / 32;
	unsigned int bit = packed_id % 32;
	u32 bitmask = BIT(bit);

	writel(bitmask, priv->base + priv->info->reset_regs[reg]);

	return 0;
}

static int gen3_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct udevice *cdev = (struct udevice *)dev_get_driver_data(reset_ctl->dev);
	struct gen3_clk_priv *priv = dev_get_priv(cdev);
	unsigned int packed_id = MOD_CLK_PACK(reset_ctl->id);
	unsigned int reg = packed_id / 32;
	unsigned int bit = packed_id % 32;
	u32 bitmask = BIT(bit);

	writel(bitmask, priv->base + priv->info->reset_clear_regs[reg]);

	return 0;
}

static const struct reset_ops rst_gen3_ops = {
	.rst_assert = gen3_reset_assert,
	.rst_deassert = gen3_reset_deassert,
};

U_BOOT_DRIVER(rst_gen3) = {
	.name = "rst_gen3",
	.id = UCLASS_RESET,
	.ops = &rst_gen3_ops,
	.flags = DM_FLAG_OS_PREPARE | DM_FLAG_VITAL,
};

int gen3_cpg_bind(struct udevice *parent)
{
	struct cpg_mssr_info *info =
		(struct cpg_mssr_info *)dev_get_driver_data(parent);
	struct udevice *cdev, *rdev;
	struct driver *drv;
	int ret;

	drv = lists_driver_lookup_name("clk_gen3");
	if (!drv)
		return -ENOENT;

	ret = device_bind_with_driver_data(parent, drv, "clk_gen3", (ulong)info,
					   dev_ofnode(parent), &cdev);
	if (ret)
		return ret;

	drv = lists_driver_lookup_name("rst_gen3");
	if (!drv)
		return -ENOENT;

	ret = device_bind_with_driver_data(parent, drv, "rst_gen3", (ulong)cdev,
					   dev_ofnode(parent), &rdev);
	if (ret)
		device_unbind(cdev);

	return ret;
}
