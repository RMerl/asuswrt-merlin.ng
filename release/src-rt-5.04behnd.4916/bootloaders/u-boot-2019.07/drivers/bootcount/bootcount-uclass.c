// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <bootcount.h>

int dm_bootcount_get(struct udevice *dev, u32 *bootcount)
{
	struct bootcount_ops *ops = bootcount_get_ops(dev);

	assert(ops);
	if (!ops->get)
		return -ENOSYS;
	return ops->get(dev, bootcount);
}

int dm_bootcount_set(struct udevice *dev, const u32 bootcount)
{
	struct bootcount_ops *ops = bootcount_get_ops(dev);

	assert(ops);
	if (!ops->set)
		return -ENOSYS;
	return ops->set(dev, bootcount);
}

/* Now implement the generic default functions */
void bootcount_store(ulong val)
{
	struct udevice *dev = NULL;
	ofnode node;
	const char *propname = "u-boot,bootcount-device";
	int ret = -ENODEV;

	/*
	 * If there's a preferred bootcount device selected by the user (by
	 * setting '/chosen/u-boot,bootcount-device' in the DTS), try to use
	 * it if available.
	 */
	node = ofnode_get_chosen_node(propname);
	if (ofnode_valid(node))
		ret = uclass_get_device_by_ofnode(UCLASS_BOOTCOUNT, node, &dev);

	/* If there was no user-selected device, use the first available one */
	if (ret)
		ret = uclass_get_device(UCLASS_BOOTCOUNT, 0, &dev);

	if (dev)
		ret = dm_bootcount_set(dev, val);

	if (ret)
		pr_debug("%s: failed to store 0x%lx\n", __func__, val);
}

ulong bootcount_load(void)
{
	struct udevice *dev = NULL;
	ofnode node;
	const char *propname = "u-boot,bootcount-device";
	int ret = -ENODEV;
	u32 val;

	/*
	 * If there's a preferred bootcount device selected by the user (by
	 * setting '/chosen/u-boot,bootcount-device' in the DTS), try to use
	 * it if available.
	 */
	node = ofnode_get_chosen_node(propname);
	if (ofnode_valid(node))
		ret = uclass_get_device_by_ofnode(UCLASS_BOOTCOUNT, node, &dev);

	/* If there was no user-selected device, use the first available one */
	if (ret)
		ret = uclass_get_device(UCLASS_BOOTCOUNT, 0, &dev);

	if (dev)
		ret = dm_bootcount_get(dev, &val);

	if (ret)
		pr_debug("%s: failed to load bootcount\n", __func__);

	/* Return the 0, if the call to dm_bootcount_get failed */
	return ret ? 0 : val;
}

UCLASS_DRIVER(bootcount) = {
	.name		= "bootcount",
	.id		= UCLASS_BOOTCOUNT,
};
