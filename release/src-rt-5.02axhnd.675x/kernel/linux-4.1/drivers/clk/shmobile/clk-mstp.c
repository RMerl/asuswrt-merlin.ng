/*
 * R-Car MSTP clocks
 *
 * Copyright (C) 2013 Ideas On Board SPRL
 *
 * Contact: Laurent Pinchart <laurent.pinchart@ideasonboard.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/spinlock.h>

/*
 * MSTP clocks. We can't use standard gate clocks as we need to poll on the
 * status register when enabling the clock.
 */

#define MSTP_MAX_CLOCKS		32

/**
 * struct mstp_clock_group - MSTP gating clocks group
 *
 * @data: clocks in this group
 * @smstpcr: module stop control register
 * @mstpsr: module stop status register (optional)
 * @lock: protects writes to SMSTPCR
 */
struct mstp_clock_group {
	struct clk_onecell_data data;
	void __iomem *smstpcr;
	void __iomem *mstpsr;
	spinlock_t lock;
};

/**
 * struct mstp_clock - MSTP gating clock
 * @hw: handle between common and hardware-specific interfaces
 * @bit_index: control bit index
 * @group: MSTP clocks group
 */
struct mstp_clock {
	struct clk_hw hw;
	u32 bit_index;
	struct mstp_clock_group *group;
};

#define to_mstp_clock(_hw) container_of(_hw, struct mstp_clock, hw)

static int cpg_mstp_clock_endisable(struct clk_hw *hw, bool enable)
{
	struct mstp_clock *clock = to_mstp_clock(hw);
	struct mstp_clock_group *group = clock->group;
	u32 bitmask = BIT(clock->bit_index);
	unsigned long flags;
	unsigned int i;
	u32 value;

	spin_lock_irqsave(&group->lock, flags);

	value = clk_readl(group->smstpcr);
	if (enable)
		value &= ~bitmask;
	else
		value |= bitmask;
	clk_writel(value, group->smstpcr);

	spin_unlock_irqrestore(&group->lock, flags);

	if (!enable || !group->mstpsr)
		return 0;

	for (i = 1000; i > 0; --i) {
		if (!(clk_readl(group->mstpsr) & bitmask))
			break;
		cpu_relax();
	}

	if (!i) {
		pr_err("%s: failed to enable %p[%d]\n", __func__,
		       group->smstpcr, clock->bit_index);
		return -ETIMEDOUT;
	}

	return 0;
}

static int cpg_mstp_clock_enable(struct clk_hw *hw)
{
	return cpg_mstp_clock_endisable(hw, true);
}

static void cpg_mstp_clock_disable(struct clk_hw *hw)
{
	cpg_mstp_clock_endisable(hw, false);
}

static int cpg_mstp_clock_is_enabled(struct clk_hw *hw)
{
	struct mstp_clock *clock = to_mstp_clock(hw);
	struct mstp_clock_group *group = clock->group;
	u32 value;

	if (group->mstpsr)
		value = clk_readl(group->mstpsr);
	else
		value = clk_readl(group->smstpcr);

	return !(value & BIT(clock->bit_index));
}

static const struct clk_ops cpg_mstp_clock_ops = {
	.enable = cpg_mstp_clock_enable,
	.disable = cpg_mstp_clock_disable,
	.is_enabled = cpg_mstp_clock_is_enabled,
};

static struct clk * __init
cpg_mstp_clock_register(const char *name, const char *parent_name,
			unsigned int index, struct mstp_clock_group *group)
{
	struct clk_init_data init;
	struct mstp_clock *clock;
	struct clk *clk;

	clock = kzalloc(sizeof(*clock), GFP_KERNEL);
	if (!clock) {
		pr_err("%s: failed to allocate MSTP clock.\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	init.name = name;
	init.ops = &cpg_mstp_clock_ops;
	init.flags = CLK_IS_BASIC | CLK_SET_RATE_PARENT;
	init.parent_names = &parent_name;
	init.num_parents = 1;

	clock->bit_index = index;
	clock->group = group;
	clock->hw.init = &init;

	clk = clk_register(NULL, &clock->hw);

	if (IS_ERR(clk))
		kfree(clock);

	return clk;
}

static void __init cpg_mstp_clocks_init(struct device_node *np)
{
	struct mstp_clock_group *group;
	const char *idxname;
	struct clk **clks;
	unsigned int i;

	group = kzalloc(sizeof(*group), GFP_KERNEL);
	clks = kmalloc(MSTP_MAX_CLOCKS * sizeof(*clks), GFP_KERNEL);
	if (group == NULL || clks == NULL) {
		kfree(group);
		kfree(clks);
		pr_err("%s: failed to allocate group\n", __func__);
		return;
	}

	spin_lock_init(&group->lock);
	group->data.clks = clks;

	group->smstpcr = of_iomap(np, 0);
	group->mstpsr = of_iomap(np, 1);

	if (group->smstpcr == NULL) {
		pr_err("%s: failed to remap SMSTPCR\n", __func__);
		kfree(group);
		kfree(clks);
		return;
	}

	for (i = 0; i < MSTP_MAX_CLOCKS; ++i)
		clks[i] = ERR_PTR(-ENOENT);

	if (of_find_property(np, "clock-indices", &i))
		idxname = "clock-indices";
	else
		idxname = "renesas,clock-indices";

	for (i = 0; i < MSTP_MAX_CLOCKS; ++i) {
		const char *parent_name;
		const char *name;
		u32 clkidx;
		int ret;

		/* Skip clocks with no name. */
		ret = of_property_read_string_index(np, "clock-output-names",
						    i, &name);
		if (ret < 0 || strlen(name) == 0)
			continue;

		parent_name = of_clk_get_parent_name(np, i);
		ret = of_property_read_u32_index(np, idxname, i, &clkidx);
		if (parent_name == NULL || ret < 0)
			break;

		if (clkidx >= MSTP_MAX_CLOCKS) {
			pr_err("%s: invalid clock %s %s index %u)\n",
			       __func__, np->name, name, clkidx);
			continue;
		}

		clks[clkidx] = cpg_mstp_clock_register(name, parent_name,
						       clkidx, group);
		if (!IS_ERR(clks[clkidx])) {
			group->data.clk_num = max(group->data.clk_num,
						  clkidx + 1);
			/*
			 * Register a clkdev to let board code retrieve the
			 * clock by name and register aliases for non-DT
			 * devices.
			 *
			 * FIXME: Remove this when all devices that require a
			 * clock will be instantiated from DT.
			 */
			clk_register_clkdev(clks[clkidx], name, NULL);
		} else {
			pr_err("%s: failed to register %s %s clock (%ld)\n",
			       __func__, np->name, name, PTR_ERR(clks[clkidx]));
		}
	}

	of_clk_add_provider(np, of_clk_src_onecell_get, &group->data);
}
CLK_OF_DECLARE(cpg_mstp_clks, "renesas,cpg-mstp-clocks", cpg_mstp_clocks_init);
