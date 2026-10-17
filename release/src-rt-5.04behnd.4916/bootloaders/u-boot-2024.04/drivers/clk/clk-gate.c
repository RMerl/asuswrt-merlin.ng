// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2010-2011 Canonical Ltd <jeremy.kerr@canonical.com>
 * Copyright (C) 2011-2012 Mike Turquette, Linaro Ltd <mturquette@linaro.org>
 * Copyright 2019 NXP
 *
 * Gated clock implementation
 */

#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <clk.h>
#include <log.h>
#include <clk-uclass.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/printk.h>

#include "clk.h"

#define UBOOT_DM_CLK_GATE "clk_gate"

/**
 * DOC: basic gatable clock which can gate and ungate it's output
 *
 * Traits of this clock:
 * prepare - clk_(un)prepare only ensures parent is (un)prepared
 * enable - clk_enable and clk_disable are functional & control gating
 * rate - inherits rate from parent.  No clk_set_rate support
 * parent - fixed parent.  No clk_set_parent support
 */

/*
 * It works on following logic:
 *
 * For enabling clock, enable = 1
 *	set2dis = 1	-> clear bit	-> set = 0
 *	set2dis = 0	-> set bit	-> set = 1
 *
 * For disabling clock, enable = 0
 *	set2dis = 1	-> set bit	-> set = 1
 *	set2dis = 0	-> clear bit	-> set = 0
 *
 * So, result is always: enable xor set2dis.
 */
static void clk_gate_endisable(struct clk *clk, int enable)
{
	struct clk_gate *gate = to_clk_gate(clk);
	int set = gate->flags & CLK_GATE_SET_TO_DISABLE ? 1 : 0;
	u32 reg;

	set ^= enable;

	if (gate->flags & CLK_GATE_HIWORD_MASK) {
		reg = BIT(gate->bit_idx + 16);
		if (set)
			reg |= BIT(gate->bit_idx);
	} else {
#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
		reg = gate->io_gate_val;
#else
		reg = readl(gate->reg);
#endif

		if (set)
			reg |= BIT(gate->bit_idx);
		else
			reg &= ~BIT(gate->bit_idx);
	}

	writel(reg, gate->reg);
}

static int clk_gate_enable(struct clk *clk)
{
	clk_gate_endisable(clk, 1);

	return 0;
}

static int clk_gate_disable(struct clk *clk)
{
	clk_gate_endisable(clk, 0);

	return 0;
}

int clk_gate_is_enabled(struct clk *clk)
{
	struct clk_gate *gate = to_clk_gate(clk);
	u32 reg;

#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
	reg = gate->io_gate_val;
#else
	reg = readl(gate->reg);
#endif

	/* if a set bit disables this clk, flip it before masking */
	if (gate->flags & CLK_GATE_SET_TO_DISABLE)
		reg ^= BIT(gate->bit_idx);

	reg &= BIT(gate->bit_idx);

	return reg ? 1 : 0;
}

const struct clk_ops clk_gate_ops = {
	.enable = clk_gate_enable,
	.disable = clk_gate_disable,
	.get_rate = clk_generic_get_rate,
};

struct clk *clk_register_gate(struct device *dev, const char *name,
			      const char *parent_name, unsigned long flags,
			      void __iomem *reg, u8 bit_idx,
			      u8 clk_gate_flags, spinlock_t *lock)
{
	struct clk_gate *gate;
	struct clk *clk;
	int ret;

	if (clk_gate_flags & CLK_GATE_HIWORD_MASK) {
		if (bit_idx > 15) {
			dev_err(dev, "gate bit exceeds LOWORD field\n");
			return ERR_PTR(-EINVAL);
		}
	}

	/* allocate the gate */
	gate = kzalloc(sizeof(*gate), GFP_KERNEL);
	if (!gate)
		return ERR_PTR(-ENOMEM);

	/* struct clk_gate assignments */
	gate->reg = reg;
	gate->bit_idx = bit_idx;
	gate->flags = clk_gate_flags;
#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
	gate->io_gate_val = *(u32 *)reg;
#endif

	clk = &gate->clk;
	clk->flags = flags;

	ret = clk_register(clk, UBOOT_DM_CLK_GATE, name, parent_name);
	if (ret) {
		kfree(gate);
		return ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(clk_gate) = {
	.name	= UBOOT_DM_CLK_GATE,
	.id	= UCLASS_CLK,
	.ops	= &clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
