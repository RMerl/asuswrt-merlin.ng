// SPDX-License-Identifier: GPL-2.0
/*
 * RZ/G2L Clock Pulse Generator
 *
 * Copyright (C) 2021-2023 Renesas Electronics Corp.
 *
 * Based on renesas-cpg-mssr.c
 *
 * Copyright (C) 2015 Glider bvba
 * Copyright (C) 2013 Ideas On Board SPRL
 * Copyright (C) 2015 Renesas Electronics Corp.
 */

#include <asm/io.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/lists.h>
#include <dt-bindings/clock/renesas-cpg-mssr.h>
#include <linux/clk-provider.h>
#include <linux/iopoll.h>
#include <reset-uclass.h>
#include <reset.h>
#include <wait_bit.h>

#include "rzg2l-cpg.h"

/*
 * Monitor registers for both clock and reset signals are offset by 0x180 from
 * the corresponding control registers.
 */
#define CLK_MON_R(reg)		(0x180 + (reg))
#define RST_MON_R(reg)		(0x180 + (reg))

#define CPG_TIMEOUT_MSEC	100

static ulong rzg2l_cpg_clk_get_rate_by_id(struct udevice *dev, unsigned int id);
static ulong rzg2l_cpg_clk_get_rate_by_name(struct udevice *dev, const char *name);

struct rzg2l_cpg_data {
	void __iomem *base;
	struct rzg2l_cpg_info *info;
};

/*
 * The top 16 bits of the clock ID are used to identify if it is a core clock or
 * a module clock.
 */
#define CPG_CLK_TYPE_SHIFT	16
#define CPG_CLK_ID_MASK 	0xffff
#define CPG_CLK_ID(x)		((x) & CPG_CLK_ID_MASK)
#define CPG_CLK_PACK(type, id)	(((type) << CPG_CLK_TYPE_SHIFT) | CPG_CLK_ID(id))

static inline bool is_mod_clk(unsigned int id)
{
	return (id >> CPG_CLK_TYPE_SHIFT) == CPG_MOD;
}

static int rzg2l_cpg_clk_set(struct clk *clk, bool enable)
{
	struct rzg2l_cpg_data *data =
		(struct rzg2l_cpg_data *)dev_get_driver_data(clk->dev);
	const unsigned int cpg_clk_id = CPG_CLK_ID(clk->id);
	const struct rzg2l_mod_clk *mod_clk = NULL;
	u32 value;
	unsigned int i;

	dev_dbg(clk->dev, "%s %s clock %u\n", enable ? "enable" : "disable",
		is_mod_clk(clk->id) ? "module" : "core", cpg_clk_id);
	if (!is_mod_clk(clk->id)) {
		dev_err(clk->dev, "ID %lu is not a module clock\n", clk->id);
		return -EINVAL;
	}

	for (i = 0; i < data->info->num_mod_clks; i++) {
		if (data->info->mod_clks[i].id == cpg_clk_id) {
			mod_clk = &data->info->mod_clks[i];
			break;
		}
	}

	if (!mod_clk) {
		dev_err(clk->dev, "Module clock %u not found\n", cpg_clk_id);
		return -ENODEV;
	}

	value = BIT(mod_clk->bit) << 16;
	if (enable)
		value |= BIT(mod_clk->bit);
	writel(value, data->base + mod_clk->off);

	if (enable && wait_for_bit_32(data->base + CLK_MON_R(mod_clk->off),
				      BIT(mod_clk->bit), enable,
				      CPG_TIMEOUT_MSEC, false)) {
		dev_err(clk->dev, "Timeout\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int rzg2l_cpg_clk_enable(struct clk *clk)
{
	return rzg2l_cpg_clk_set(clk, true);
}

static int rzg2l_cpg_clk_disable(struct clk *clk)
{
	return rzg2l_cpg_clk_set(clk, false);
}

static int rzg2l_cpg_clk_of_xlate(struct clk *clk,
				  struct ofnode_phandle_args *args)
{
	struct rzg2l_cpg_data *data =
		(struct rzg2l_cpg_data *)dev_get_driver_data(clk->dev);
	u32 cpg_clk_type, cpg_clk_id;
	bool found = false;
	unsigned int i;

	if (args->args_count != 2) {
		dev_dbg(clk->dev, "Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	cpg_clk_type = args->args[0];
	cpg_clk_id = args->args[1];

	switch (cpg_clk_type) {
	case CPG_CORE:
		for (i = 0; i < data->info->num_core_clks; i++) {
			if (data->info->core_clks[i].id == cpg_clk_id) {
				found = true;
				break;
			}
		}
		if (!found) {
			dev_dbg(clk->dev,
				"Invalid second argument %u: Must be a valid core clock ID\n",
				cpg_clk_id);
			return -EINVAL;
		}
		break;
	case CPG_MOD:
		for (i = 0; i < data->info->num_mod_clks; i++) {
			if (data->info->mod_clks[i].id == cpg_clk_id) {
				found = true;
				break;
			}
		}
		if (!found) {
			dev_dbg(clk->dev,
				"Invalid second argument %u: Must be a valid module clock ID\n",
				cpg_clk_id);
			return -EINVAL;
		}
		break;
	default:
		dev_dbg(clk->dev,
			"Invalid first argument %u: Must be CPG_CORE or CPG_MOD\n",
			cpg_clk_type);
		return -EINVAL;
	}

	clk->id = CPG_CLK_PACK(cpg_clk_type, cpg_clk_id);

	return 0;
}

static ulong rzg2l_sdhi_clk_get_rate(struct udevice *dev, const struct cpg_core_clk *cc)
{
	struct rzg2l_cpg_data *data =
		(struct rzg2l_cpg_data *)dev_get_driver_data(dev);
	const ulong offset = CPG_CONF_OFFSET(cc->conf);
	const int shift = CPG_CONF_BITPOS(cc->conf);
	const u32 mask = CPG_CONF_BITMASK(cc->conf);
	unsigned int sel;

	sel = (readl(data->base + offset) >> shift) & mask;

	if (!sel || sel > cc->num_parents) {
		dev_err(dev, "Invalid SEL_SDHI%d_SET value %u\n", shift / 4, sel);
		return -EIO;
	}
	return rzg2l_cpg_clk_get_rate_by_name(dev, cc->parent_names[sel - 1]);
}

static ulong rzg2l_div_clk_get_rate(struct udevice *dev, const struct cpg_core_clk *cc)
{
	struct rzg2l_cpg_data *data =
		(struct rzg2l_cpg_data *)dev_get_driver_data(dev);
	const ulong offset = CPG_CONF_OFFSET(cc->conf);
	const int shift = CPG_CONF_BITPOS(cc->conf);
	const u32 mask = CPG_CONF_BITMASK(cc->conf);
	unsigned int sel, i;

	sel = (readl(data->base + offset) >> shift) & mask;

	for (i = 0; cc->dtable[i].div; i++) {
		if (cc->dtable[i].val == sel)
			return rzg2l_cpg_clk_get_rate_by_id(dev, cc->parent) / cc->dtable[i].div;
	}
	dev_err(dev, "Invalid selector value %u for clock %s\n", sel, cc->name);
	return -EINVAL;
}

static ulong rzg2l_core_clk_get_rate(struct udevice *dev, const struct cpg_core_clk *cc)
{
	switch (cc->type) {
	case CLK_TYPE_FF:
		const ulong parent_rate = rzg2l_cpg_clk_get_rate_by_id(dev, cc->parent);
		return parent_rate * cc->mult / cc->div;
	case CLK_TYPE_IN:
		struct clk clk_in;
		clk_get_by_name(dev, cc->name, &clk_in);
		return clk_get_rate(&clk_in);
	case CLK_TYPE_SD_MUX:
		return rzg2l_sdhi_clk_get_rate(dev, cc);
	case CLK_TYPE_DIV:
		return rzg2l_div_clk_get_rate(dev, cc);
	default:
		dev_err(dev, "get_rate needed for clock %u, type %d\n", cc->id, cc->type);
		return -ENOSYS;
	}
}

static ulong rzg2l_cpg_clk_get_rate_by_id(struct udevice *dev, unsigned int id)
{
	struct rzg2l_cpg_data *data =
		(struct rzg2l_cpg_data *)dev_get_driver_data(dev);
	const unsigned int cpg_clk_id = CPG_CLK_ID(id);
	unsigned int i;

	if (is_mod_clk(id)) {
		for (i = 0; i < data->info->num_mod_clks; i++) {
			if (data->info->mod_clks[i].id == cpg_clk_id)
				return rzg2l_cpg_clk_get_rate_by_id(dev,
								    data->info->mod_clks[i].parent);
		}

		dev_err(dev, "Module clock ID %u not found\n", cpg_clk_id);
		return -ENODEV;
	}

	for (i = 0; i < data->info->num_core_clks; i++) {
		if (data->info->core_clks[i].id == cpg_clk_id)
			return rzg2l_core_clk_get_rate(dev, &data->info->core_clks[i]);
	}

	dev_err(dev, "Core clock ID %u not found\n", cpg_clk_id);
	return -ENODEV;
}

static ulong rzg2l_cpg_clk_get_rate_by_name(struct udevice *dev, const char *name)
{
	struct rzg2l_cpg_data *data =
		(struct rzg2l_cpg_data *)dev_get_driver_data(dev);
	unsigned int i;

	for (i = 0; i < data->info->num_mod_clks; i++) {
		if (!strcmp(name, data->info->mod_clks[i].name))
			return rzg2l_cpg_clk_get_rate_by_id(dev, data->info->mod_clks[i].parent);
	}
	for (i = 0; i < data->info->num_core_clks; i++) {
		if (!strcmp(name, data->info->core_clks[i].name))
			return rzg2l_core_clk_get_rate(dev, &data->info->core_clks[i]);
	}

	dev_err(dev, "Clock name %s not found\n", name);
	return -EINVAL;
}

static ulong rzg2l_cpg_clk_get_rate(struct clk *clk)
{
	return rzg2l_cpg_clk_get_rate_by_id(clk->dev, clk->id);
}

static ulong rzg2l_sdhi_clk_set_rate(struct udevice *dev, const struct cpg_core_clk *cc, ulong rate)
{
	struct rzg2l_cpg_data *data =
		(struct rzg2l_cpg_data *)dev_get_driver_data(dev);
	const ulong offset = CPG_CONF_OFFSET(cc->conf);
	const int shift = CPG_CONF_BITPOS(cc->conf);
	int channel, new_sel, prev_sel;
	ulong target_rate;
	unsigned int i;
	u32 value;

	prev_sel = (readl(data->base + offset) >> shift) & 0x3;
	channel = shift / 4;

	/*
	 * Round the requested rate down, unless it is below the minimum
	 * supported rate. Assume that the parent clock names are listed in
	 * order of descending rate.
	 */
	for (i = 0; i < cc->num_parents; i++) {
		target_rate = rzg2l_cpg_clk_get_rate_by_name(dev, cc->parent_names[i]);
		if (rate >= target_rate) {
			new_sel = i + 1;
			break;
		}
	}
	if (!new_sel)
		new_sel = cc->num_parents - 1;

	if (new_sel == prev_sel)
		return target_rate;
	dev_dbg(dev, "sdhi set_rate rate=%lu target_rate=%lu sel=%d\n",
		rate, target_rate, new_sel);

	/*
	 * As per the HW manual, we should not directly switch from 533 MHz to
	 * 400 MHz and vice versa. To change the setting from 2’b01 (533 MHz)
	 * to 2’b10 (400 MHz) or vice versa, Switch to 2’b11 (266 MHz) first,
	 * and then switch to the target setting (2’b01 (533 MHz) or 2’b10
	 * (400 MHz)).
	 */
	if (new_sel != SEL_SDHI_266MHz && prev_sel != SEL_SDHI_266MHz) {
		u32 waitbit;
		int ret;

		dev_dbg(dev, "sdhi set_rate via 266MHz\n");
		value = (SEL_SDHI_WRITE_ENABLE | SEL_SDHI_266MHz) << shift;
		writel(value, data->base + offset);

		/* Wait for the switch to complete. */
		waitbit = channel ? CPG_CLKSTATUS_SELSDHI1_STS : CPG_CLKSTATUS_SELSDHI0_STS;
		ret = readl_poll_timeout(data->base + CPG_CLKSTATUS, value,
					 !(value & waitbit),
					 CPG_SDHI_CLK_SWITCH_STATUS_TIMEOUT_US);
		if (ret) {
			dev_err(dev, "Failed to switch SDHI%d clock source\n", channel);
			return -EIO;
		}
	}

	value = (SEL_SDHI_WRITE_ENABLE | new_sel) << shift;
	writel(value, data->base + offset);

	return target_rate;
}

static ulong rzg2l_core_clk_set_rate(struct udevice *dev, const struct cpg_core_clk *cc, ulong rate)
{
	if (cc->type == CLK_TYPE_SD_MUX)
		return rzg2l_sdhi_clk_set_rate(dev, cc, rate);

	/*
	 * The sdhi driver calls clk_set_rate for SD0_DIV4 and SD1_DIV4, even
	 * though they're in a fixed relationship with SD0 and SD1 respectively.
	 * To allow the driver to proceed, simply return the current rates
	 * without making any change.
	 */
	if (cc->id == CLK_SD0_DIV4 || cc->id == CLK_SD1_DIV4)
		return rzg2l_core_clk_get_rate(dev, cc);

	dev_err(dev, "set_rate needed for clock %u, type %d\n", cc->id, cc->type);
	return -ENOSYS;
}

static ulong rzg2l_cpg_clk_set_rate_by_id(struct udevice *dev, unsigned int id, ulong rate)
{
	struct rzg2l_cpg_data *data =
		(struct rzg2l_cpg_data *)dev_get_driver_data(dev);
	const unsigned int cpg_clk_id = CPG_CLK_ID(id);
	unsigned int i;

	if (is_mod_clk(id)) {
		for (i = 0; i < data->info->num_mod_clks; i++) {
			if (data->info->mod_clks[i].id == cpg_clk_id)
				return rzg2l_cpg_clk_set_rate_by_id(dev,
								    data->info->mod_clks[i].parent,
								    rate);
		}

		dev_err(dev, "Module clock ID %u not found\n", cpg_clk_id);
		return -ENODEV;
	}

	for (i = 0; i < data->info->num_core_clks; i++) {
		if (data->info->core_clks[i].id == cpg_clk_id)
			return rzg2l_core_clk_set_rate(dev, &data->info->core_clks[i], rate);
	}

	dev_err(dev, "Core clock ID %u not found\n", cpg_clk_id);
	return -ENODEV;
}

static ulong rzg2l_cpg_clk_set_rate(struct clk *clk, ulong rate)
{
	return rzg2l_cpg_clk_set_rate_by_id(clk->dev, clk->id, rate);
}

static const struct clk_ops rzg2l_cpg_clk_ops = {
	.enable		= rzg2l_cpg_clk_enable,
	.disable	= rzg2l_cpg_clk_disable,
	.of_xlate	= rzg2l_cpg_clk_of_xlate,
	.get_rate	= rzg2l_cpg_clk_get_rate,
	.set_rate	= rzg2l_cpg_clk_set_rate,
};

U_BOOT_DRIVER(rzg2l_cpg_clk) = {
	.name		= "rzg2l-cpg-clk",
	.id		= UCLASS_CLK,
	.ops		= &rzg2l_cpg_clk_ops,
	.flags		= DM_FLAG_VITAL,
};

static int rzg2l_cpg_rst_set(struct reset_ctl *reset_ctl, bool asserted)
{
	struct rzg2l_cpg_data *data =
		(struct rzg2l_cpg_data *)dev_get_driver_data(reset_ctl->dev);
	const struct rzg2l_reset *rst;
	u32 value;

	dev_dbg(reset_ctl->dev, "%s %lu\n", asserted ? "assert" : "deassert", reset_ctl->id);
	if (reset_ctl->id >= data->info->num_resets) {
		dev_err(reset_ctl->dev, "Invalid reset id %lu\n", reset_ctl->id);
		return -EINVAL;
	}
	rst = &data->info->resets[reset_ctl->id];

	value = BIT(rst->bit) << 16;
	if (!asserted)
		value |= BIT(rst->bit);
	writel(value, data->base + rst->off);

	return wait_for_bit_32(data->base + RST_MON_R(rst->off), BIT(rst->bit),
			       asserted, CPG_TIMEOUT_MSEC, false);
}

static int rzg2l_cpg_rst_assert(struct reset_ctl *reset_ctl)
{
	return rzg2l_cpg_rst_set(reset_ctl, true);
}

static int rzg2l_cpg_rst_deassert(struct reset_ctl *reset_ctl)
{
	return rzg2l_cpg_rst_set(reset_ctl, false);
}

static int rzg2l_cpg_rst_of_xlate(struct reset_ctl *reset_ctl,
				  struct ofnode_phandle_args *args)
{
	struct rzg2l_cpg_data *data =
		(struct rzg2l_cpg_data *)dev_get_driver_data(reset_ctl->dev);

	if (args->args[0] >= data->info->num_resets)
		return -EINVAL;

	reset_ctl->id = args->args[0];
	return 0;
}

static const struct reset_ops rzg2l_cpg_rst_ops = {
	.rst_assert	= rzg2l_cpg_rst_assert,
	.rst_deassert	= rzg2l_cpg_rst_deassert,
	.of_xlate	= rzg2l_cpg_rst_of_xlate,
};

U_BOOT_DRIVER(rzg2l_cpg_rst) = {
	.name		= "rzg2l-cpg-rst",
	.id		= UCLASS_RESET,
	.ops		= &rzg2l_cpg_rst_ops,
	.flags		= DM_FLAG_VITAL,
};

int rzg2l_cpg_bind(struct udevice *parent)
{
	struct udevice *cdev, *rdev;
	struct rzg2l_cpg_data *data;
	struct driver *drv;
	int ret;

	data = devm_kmalloc(parent, sizeof(*data), 0);
	if (!data)
		return -ENOMEM;

	data->base = dev_read_addr_ptr(parent);
	if (!data->base)
		return -EINVAL;

	data->info = (struct rzg2l_cpg_info *)dev_get_driver_data(parent);
	if (!data->info)
		return -EINVAL;

	drv = lists_driver_lookup_name("rzg2l-cpg-clk");
	if (!drv)
		return -ENOENT;

	ret = device_bind_with_driver_data(parent, drv, parent->name,
					   (ulong)data, dev_ofnode(parent),
					   &cdev);
	if (ret)
		return ret;

	drv = lists_driver_lookup_name("rzg2l-cpg-rst");
	if (!drv) {
		device_unbind(cdev);
		return -ENOENT;
	}

	ret = device_bind_with_driver_data(parent, drv, parent->name,
					   (ulong)data, dev_ofnode(parent),
					   &rdev);
	if (ret)
		device_unbind(cdev);

	return ret;
}
