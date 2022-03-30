// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>

static inline struct phy_ops *phy_dev_ops(struct udevice *dev)
{
	return (struct phy_ops *)dev->driver->ops;
}

static int generic_phy_xlate_offs_flags(struct phy *phy,
					struct ofnode_phandle_args *args)
{
	debug("%s(phy=%p)\n", __func__, phy);

	if (args->args_count > 1) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		phy->id = args->args[0];
	else
		phy->id = 0;

	return 0;
}

int generic_phy_get_by_index(struct udevice *dev, int index,
			     struct phy *phy)
{
	struct ofnode_phandle_args args;
	struct phy_ops *ops;
	struct udevice *phydev;
	int i, ret;

	debug("%s(dev=%p, index=%d, phy=%p)\n", __func__, dev, index, phy);

	assert(phy);
	phy->dev = NULL;
	ret = dev_read_phandle_with_args(dev, "phys", "#phy-cells", 0, index,
					 &args);
	if (ret) {
		debug("%s: dev_read_phandle_with_args failed: err=%d\n",
		      __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_PHY, args.node, &phydev);
	if (ret) {
		debug("%s: uclass_get_device_by_ofnode failed: err=%d\n",
		      __func__, ret);

		/* Check if args.node's parent is a PHY provider */
		ret = uclass_get_device_by_ofnode(UCLASS_PHY,
						  ofnode_get_parent(args.node),
						  &phydev);
		if (ret)
			return ret;

		/* insert phy idx at first position into args array */
		for (i = args.args_count; i >= 1 ; i--)
			args.args[i] = args.args[i - 1];

		args.args_count++;
		args.args[0] = ofnode_read_u32_default(args.node, "reg", -1);
	}

	phy->dev = phydev;

	ops = phy_dev_ops(phydev);

	if (ops->of_xlate)
		ret = ops->of_xlate(phy, &args);
	else
		ret = generic_phy_xlate_offs_flags(phy, &args);
	if (ret) {
		debug("of_xlate() failed: %d\n", ret);
		goto err;
	}

	return 0;

err:
	return ret;
}

int generic_phy_get_by_name(struct udevice *dev, const char *phy_name,
			    struct phy *phy)
{
	int index;

	debug("%s(dev=%p, name=%s, phy=%p)\n", __func__, dev, phy_name, phy);

	index = dev_read_stringlist_search(dev, "phy-names", phy_name);
	if (index < 0) {
		debug("dev_read_stringlist_search() failed: %d\n", index);
		return index;
	}

	return generic_phy_get_by_index(dev, index, phy);
}

int generic_phy_init(struct phy *phy)
{
	struct phy_ops const *ops = phy_dev_ops(phy->dev);

	return ops->init ? ops->init(phy) : 0;
}

int generic_phy_reset(struct phy *phy)
{
	struct phy_ops const *ops = phy_dev_ops(phy->dev);

	return ops->reset ? ops->reset(phy) : 0;
}

int generic_phy_exit(struct phy *phy)
{
	struct phy_ops const *ops = phy_dev_ops(phy->dev);

	return ops->exit ? ops->exit(phy) : 0;
}

int generic_phy_power_on(struct phy *phy)
{
	struct phy_ops const *ops = phy_dev_ops(phy->dev);

	return ops->power_on ? ops->power_on(phy) : 0;
}

int generic_phy_power_off(struct phy *phy)
{
	struct phy_ops const *ops = phy_dev_ops(phy->dev);

	return ops->power_off ? ops->power_off(phy) : 0;
}

UCLASS_DRIVER(phy) = {
	.id		= UCLASS_PHY,
	.name		= "phy",
};
