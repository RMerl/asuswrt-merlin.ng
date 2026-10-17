// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright (C) 2011 Sascha Hauer, Pengutronix <s.hauer@pengutronix.de>
 * Copyright (C) 2011 Richard Zhao, Linaro <richard.zhao@linaro.org>
 * Copyright (C) 2011-2012 Mike Turquette, Linaro Ltd <mturquette@linaro.org>
 *
 * Simple multiplexer clock implementation
 */

/*
 * U-Boot CCF porting node:
 *
 * The Linux kernel - as of tag: 5.0-rc3 is using also the imx_clk_fixup_mux()
 * version of CCF mux. It is used on e.g. imx6q to provide fixes (like
 * imx_cscmr1_fixup) for broken HW.
 *
 * At least for IMX6Q (but NOT IMX6QP) it is important when we set the parent
 * clock.
 */

#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <log.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/uclass.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/printk.h>

#include "clk.h"

#define UBOOT_DM_CLK_CCF_MUX "ccf_clk_mux"

int clk_mux_val_to_index(struct clk *clk, u32 *table, unsigned int flags,
			 unsigned int val)
{
	struct clk_mux *mux = to_clk_mux(clk);
	int num_parents = mux->num_parents;

	if (table) {
		int i;

		for (i = 0; i < num_parents; i++)
			if (table[i] == val)
				return i;
		return -EINVAL;
	}

	if (val && (flags & CLK_MUX_INDEX_BIT))
		val = ffs(val) - 1;

	if (val && (flags & CLK_MUX_INDEX_ONE))
		val--;

	if (val >= num_parents)
		return -EINVAL;

	return val;
}

unsigned int clk_mux_index_to_val(u32 *table, unsigned int flags, u8 index)
{
	unsigned int val = index;

	if (table) {
		val = table[index];
	} else {
		if (flags & CLK_MUX_INDEX_BIT)
			val = 1 << index;

		if (flags & CLK_MUX_INDEX_ONE)
			val++;
	}

	return val;
}

u8 clk_mux_get_parent(struct clk *clk)
{
	struct clk_mux *mux = to_clk_mux(clk);
	u32 val;

#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
	val = mux->io_mux_val;
#else
	val = readl(mux->reg);
#endif
	val >>= mux->shift;
	val &= mux->mask;

	return clk_mux_val_to_index(clk, mux->table, mux->flags, val);
}

static int clk_fetch_parent_index(struct clk *clk,
				  struct clk *parent)
{
	struct clk_mux *mux = to_clk_mux(clk);

	int i;

	if (!parent)
		return -EINVAL;

	for (i = 0; i < mux->num_parents; i++) {
		if (!strcmp(parent->dev->name, mux->parent_names[i]))
			return i;
	}

	return -EINVAL;
}

static int clk_mux_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk_mux *mux = to_clk_mux(clk);
	int index;
	u32 val;
	u32 reg;

	index = clk_fetch_parent_index(clk, parent);
	if (index < 0) {
		log_err("Could not fetch index\n");
		return index;
	}

	val = clk_mux_index_to_val(mux->table, mux->flags, index);

	if (mux->flags & CLK_MUX_HIWORD_MASK) {
		reg = mux->mask << (mux->shift + 16);
	} else {
#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
		reg = mux->io_mux_val;
#else
		reg = readl(mux->reg);
#endif
		reg &= ~(mux->mask << mux->shift);
	}
	val = val << mux->shift;
	reg |= val;
#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
	mux->io_mux_val = reg;
#else
	writel(reg, mux->reg);
#endif

	return 0;
}

const struct clk_ops clk_mux_ops = {
	.get_rate = clk_generic_get_rate,
	.set_parent = clk_mux_set_parent,
};

struct clk *clk_hw_register_mux_table(struct device *dev, const char *name,
		const char * const *parent_names, u8 num_parents,
		unsigned long flags,
		void __iomem *reg, u8 shift, u32 mask,
		u8 clk_mux_flags, u32 *table)
{
	struct clk_mux *mux;
	struct clk *clk;
	u8 width = 0;
	int ret;

	if (clk_mux_flags & CLK_MUX_HIWORD_MASK) {
		width = fls(mask) - ffs(mask) + 1;
		if (width + shift > 16) {
			dev_err(dev, "mux value exceeds LOWORD field\n");
			return ERR_PTR(-EINVAL);
		}
	}

	/* allocate the mux */
	mux = kzalloc(sizeof(*mux), GFP_KERNEL);
	if (!mux)
		return ERR_PTR(-ENOMEM);

	/* U-Boot specific assignments */
	mux->parent_names = parent_names;
	mux->num_parents = num_parents;

	/* struct clk_mux assignments */
	mux->reg = reg;
	mux->shift = shift;
	mux->mask = mask;
	mux->flags = clk_mux_flags;
	mux->table = table;
#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
	mux->io_mux_val = *(u32 *)reg;
#endif

	clk = &mux->clk;
	clk->flags = flags;

	/*
	 * Read the current mux setup - so we assign correct parent.
	 *
	 * Changing parent would require changing internals of udevice struct
	 * for the corresponding clock (to do that define .set_parent() method).
	 */
	ret = clk_register(clk, UBOOT_DM_CLK_CCF_MUX, name,
			   parent_names[clk_mux_get_parent(clk)]);
	if (ret) {
		kfree(mux);
		return ERR_PTR(ret);
	}

	return clk;
}

struct clk *clk_register_mux_table(struct device *dev, const char *name,
		const char * const *parent_names, u8 num_parents,
		unsigned long flags,
		void __iomem *reg, u8 shift, u32 mask,
		u8 clk_mux_flags, u32 *table)
{
	struct clk *clk;

	clk = clk_hw_register_mux_table(dev, name, parent_names, num_parents,
				       flags, reg, shift, mask, clk_mux_flags,
				       table);
	if (IS_ERR(clk))
		return ERR_CAST(clk);
	return clk;
}

struct clk *clk_register_mux(struct device *dev, const char *name,
		const char * const *parent_names, u8 num_parents,
		unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_mux_flags)
{
	u32 mask = BIT(width) - 1;

	return clk_register_mux_table(dev, name, parent_names, num_parents,
				      flags, reg, shift, mask, clk_mux_flags,
				      NULL);
}

U_BOOT_DRIVER(ccf_clk_mux) = {
	.name	= UBOOT_DM_CLK_CCF_MUX,
	.id	= UCLASS_CLK,
	.ops	= &clk_mux_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
