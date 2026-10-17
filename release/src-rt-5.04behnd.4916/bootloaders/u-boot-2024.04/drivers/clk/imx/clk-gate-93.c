// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <linux/bug.h>
#include <linux/clk-provider.h>
#include <clk.h>
#include "clk.h"
#include <linux/err.h>

#define UBOOT_DM_CLK_IMX_GATE93 "imx_clk_gate93"

#define DIRECT_OFFSET		0x0

/*
 * 0b000 - LPCG will be OFF in any CPU mode.
 * 0b100 - LPCG will be ON in any CPU mode.
 */
#define LPM_SETTING_OFF		0x0
#define LPM_SETTING_ON		0x4

#define LPM_CUR_OFFSET		0x1c

#define AUTHEN_OFFSET		0x30
#define CPULPM_EN		BIT(2)
#define TZ_NS_SHIFT		9
#define TZ_NS_MASK		BIT(9)

#define WHITE_LIST_SHIFT	16

struct imx93_clk_gate {
	struct clk clk;
	void __iomem	*reg;
	u32		bit_idx;
	u32		val;
	u32		mask;
	unsigned int	*share_count;
};

#define to_imx93_clk_gate(_clk) container_of(_clk, struct imx93_clk_gate, clk)

static void imx93_clk_gate_do_hardware(struct clk *clk, bool enable)
{
	struct imx93_clk_gate *gate = to_imx93_clk_gate(clk);
	u32 val;

	val = readl(gate->reg + AUTHEN_OFFSET);
	if (val & CPULPM_EN) {
		val = enable ? LPM_SETTING_ON : LPM_SETTING_OFF;
		writel(val, gate->reg + LPM_CUR_OFFSET);
	} else {
		val = readl(gate->reg + DIRECT_OFFSET);
		val &= ~(gate->mask << gate->bit_idx);
		if (enable)
			val |= (gate->val & gate->mask) << gate->bit_idx;
		writel(val, gate->reg + DIRECT_OFFSET);
	}
}

static int imx93_clk_gate_enable(struct clk *clk)
{
	struct imx93_clk_gate *gate = to_imx93_clk_gate(clk);

	if (gate->share_count && (*gate->share_count)++ > 0)
		return 0;

	imx93_clk_gate_do_hardware(clk, true);

	return 0;
}

static int imx93_clk_gate_disable(struct clk *clk)
{
	struct imx93_clk_gate *gate = to_imx93_clk_gate(clk);

	if (gate->share_count) {
		if (WARN_ON(*gate->share_count == 0))
			return 0;
		else if (--(*gate->share_count) > 0)
			return 0;
	}

	imx93_clk_gate_do_hardware(clk, false);

	return 0;
}

static ulong imx93_clk_set_rate(struct clk *clk, ulong rate)
{
	struct clk *parent = clk_get_parent(clk);

	if (parent)
		return clk_set_rate(parent, rate);

	return -ENODEV;
}

static const struct clk_ops imx93_clk_gate_ops = {
	.enable = imx93_clk_gate_enable,
	.disable = imx93_clk_gate_disable,
	.get_rate = clk_generic_get_rate,
	.set_rate = imx93_clk_set_rate,
};

struct clk *imx93_clk_gate(struct device *dev, const char *name, const char *parent_name,
			   unsigned long flags, void __iomem *reg, u32 bit_idx, u32 val,
			   u32 mask, u32 domain_id, unsigned int *share_count)
{
	struct imx93_clk_gate *gate;
	struct clk *clk;
	int ret;

	gate = kzalloc(sizeof(*gate), GFP_KERNEL);
	if (!gate)
		return ERR_PTR(-ENOMEM);

	gate->reg = reg;
	gate->bit_idx = bit_idx;
	gate->val = val;
	gate->mask = mask;
	gate->share_count = share_count;

	clk = &gate->clk;

	ret = clk_register(clk, UBOOT_DM_CLK_IMX_GATE93, name, parent_name);
	if (ret) {
		kfree(gate);
		return ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(clk_gate93) = {
	.name	= UBOOT_DM_CLK_IMX_GATE93,
	.id	= UCLASS_CLK,
	.ops	= &imx93_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
