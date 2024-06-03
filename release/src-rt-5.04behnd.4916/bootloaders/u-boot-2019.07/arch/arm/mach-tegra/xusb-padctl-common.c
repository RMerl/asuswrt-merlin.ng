// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014-2015, NVIDIA CORPORATION.  All rights reserved.
 */

#define pr_fmt(fmt) "tegra-xusb-padctl: " fmt

#include <common.h>
#include <errno.h>

#include "xusb-padctl-common.h"

#include <asm/arch/clock.h>

int tegra_xusb_phy_prepare(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->prepare)
		return phy->ops->prepare(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_enable(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->enable)
		return phy->ops->enable(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_disable(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->disable)
		return phy->ops->disable(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_unprepare(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->unprepare)
		return phy->ops->unprepare(phy);

	return phy ? -ENOSYS : -EINVAL;
}

struct tegra_xusb_phy *tegra_xusb_phy_get(unsigned int type)
{
	struct tegra_xusb_phy *phy;
	int i;

	for (i = 0; i < padctl.socdata->num_phys; i++) {
		phy = &padctl.socdata->phys[i];
		if (phy->type != type)
			continue;
		return phy;
	}

	return NULL;
}

static const struct tegra_xusb_padctl_lane *
tegra_xusb_padctl_find_lane(struct tegra_xusb_padctl *padctl, const char *name)
{
	unsigned int i;

	for (i = 0; i < padctl->socdata->num_lanes; i++)
		if (strcmp(name, padctl->socdata->lanes[i].name) == 0)
			return &padctl->socdata->lanes[i];

	return NULL;
}

static int
tegra_xusb_padctl_group_parse_dt(struct tegra_xusb_padctl *padctl,
				 struct tegra_xusb_padctl_group *group,
				 ofnode node)
{
	unsigned int i;
	int len, ret;

	group->name = ofnode_get_name(node);

	len = ofnode_read_string_count(node, "nvidia,lanes");
	if (len < 0) {
		pr_err("failed to parse \"nvidia,lanes\" property");
		return -EINVAL;
	}

	group->num_pins = len;

	for (i = 0; i < group->num_pins; i++) {
		ret = ofnode_read_string_index(node, "nvidia,lanes", i,
					       &group->pins[i]);
		if (ret) {
			pr_err("failed to read string from \"nvidia,lanes\" property");
			return -EINVAL;
		}
	}

	group->num_pins = len;

	ret = ofnode_read_string_index(node, "nvidia,function", 0,
				       &group->func);
	if (ret) {
		pr_err("failed to parse \"nvidia,func\" property");
		return -EINVAL;
	}

	group->iddq = ofnode_read_u32_default(node, "nvidia,iddq", -1);

	return 0;
}

static int tegra_xusb_padctl_find_function(struct tegra_xusb_padctl *padctl,
					   const char *name)
{
	unsigned int i;

	for (i = 0; i < padctl->socdata->num_functions; i++)
		if (strcmp(name, padctl->socdata->functions[i]) == 0)
			return i;

	return -ENOENT;
}

static int
tegra_xusb_padctl_lane_find_function(struct tegra_xusb_padctl *padctl,
				     const struct tegra_xusb_padctl_lane *lane,
				     const char *name)
{
	unsigned int i;
	int func;

	func = tegra_xusb_padctl_find_function(padctl, name);
	if (func < 0)
		return func;

	for (i = 0; i < lane->num_funcs; i++)
		if (lane->funcs[i] == func)
			return i;

	return -ENOENT;
}

static int
tegra_xusb_padctl_group_apply(struct tegra_xusb_padctl *padctl,
			      const struct tegra_xusb_padctl_group *group)
{
	unsigned int i;

	for (i = 0; i < group->num_pins; i++) {
		const struct tegra_xusb_padctl_lane *lane;
		unsigned int func;
		u32 value;

		lane = tegra_xusb_padctl_find_lane(padctl, group->pins[i]);
		if (!lane) {
			pr_err("no lane for pin %s", group->pins[i]);
			continue;
		}

		func = tegra_xusb_padctl_lane_find_function(padctl, lane,
							    group->func);
		if (func < 0) {
			pr_err("function %s invalid for lane %s: %d",
			      group->func, lane->name, func);
			continue;
		}

		value = padctl_readl(padctl, lane->offset);

		/* set pin function */
		value &= ~(lane->mask << lane->shift);
		value |= func << lane->shift;

		/*
		 * Set IDDQ if supported on the lane and specified in the
		 * configuration.
		 */
		if (lane->iddq > 0 && group->iddq >= 0) {
			if (group->iddq != 0)
				value &= ~(1 << lane->iddq);
			else
				value |= 1 << lane->iddq;
		}

		padctl_writel(padctl, value, lane->offset);
	}

	return 0;
}

static int
tegra_xusb_padctl_config_apply(struct tegra_xusb_padctl *padctl,
			       struct tegra_xusb_padctl_config *config)
{
	unsigned int i;

	for (i = 0; i < config->num_groups; i++) {
		const struct tegra_xusb_padctl_group *group;
		int err;

		group = &config->groups[i];

		err = tegra_xusb_padctl_group_apply(padctl, group);
		if (err < 0) {
			pr_err("failed to apply group %s: %d",
			      group->name, err);
			continue;
		}
	}

	return 0;
}

static int
tegra_xusb_padctl_config_parse_dt(struct tegra_xusb_padctl *padctl,
				  struct tegra_xusb_padctl_config *config,
				  ofnode node)
{
	ofnode subnode;

	config->name = ofnode_get_name(node);

	ofnode_for_each_subnode(subnode, node) {
		struct tegra_xusb_padctl_group *group;
		int err;

		group = &config->groups[config->num_groups];

		err = tegra_xusb_padctl_group_parse_dt(padctl, group, subnode);
		if (err < 0) {
			pr_err("failed to parse group %s", group->name);
			return err;
		}

		config->num_groups++;
	}

	return 0;
}

static int tegra_xusb_padctl_parse_dt(struct tegra_xusb_padctl *padctl,
				      ofnode node)
{
	ofnode subnode;
	int err;

	err = ofnode_read_resource(node, 0, &padctl->regs);
	if (err < 0) {
		pr_err("registers not found");
		return err;
	}

	ofnode_for_each_subnode(subnode, node) {
		struct tegra_xusb_padctl_config *config = &padctl->config;

		debug("%s: subnode=%s\n", __func__, ofnode_get_name(subnode));
		err = tegra_xusb_padctl_config_parse_dt(padctl, config,
							subnode);
		if (err < 0) {
			pr_err("failed to parse entry %s: %d",
			      config->name, err);
			continue;
		}
	}
	debug("%s: done\n", __func__);

	return 0;
}

struct tegra_xusb_padctl padctl;

int tegra_xusb_process_nodes(ofnode nodes[], unsigned int count,
			     const struct tegra_xusb_padctl_soc *socdata)
{
	unsigned int i;
	int err;

	debug("%s: count=%d\n", __func__, count);
	for (i = 0; i < count; i++) {
		debug("%s: i=%d, node=%p\n", __func__, i, nodes[i].np);
		if (!ofnode_is_available(nodes[i]))
			continue;

		padctl.socdata = socdata;

		err = tegra_xusb_padctl_parse_dt(&padctl, nodes[i]);
		if (err < 0) {
			pr_err("failed to parse DT: %d", err);
			continue;
		}

		/* deassert XUSB padctl reset */
		reset_set_enable(PERIPH_ID_XUSB_PADCTL, 0);

		err = tegra_xusb_padctl_config_apply(&padctl, &padctl.config);
		if (err < 0) {
			pr_err("failed to apply pinmux: %d", err);
			continue;
		}

		/* only a single instance is supported */
		break;
	}
	debug("%s: done\n", __func__);

	return 0;
}
