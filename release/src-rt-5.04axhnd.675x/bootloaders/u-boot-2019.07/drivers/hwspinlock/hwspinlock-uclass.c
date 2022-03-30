// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <hwspinlock.h>
#include <dm/device-internal.h>

static inline const struct hwspinlock_ops *
hwspinlock_dev_ops(struct udevice *dev)
{
	return (const struct hwspinlock_ops *)dev->driver->ops;
}

static int hwspinlock_of_xlate_default(struct hwspinlock *hws,
				       struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		hws->id = args->args[0];
	else
		hws->id = 0;

	return 0;
}

int hwspinlock_get_by_index(struct udevice *dev, int index,
			    struct hwspinlock *hws)
{
	int ret;
	struct ofnode_phandle_args args;
	struct udevice *dev_hws;
	const struct hwspinlock_ops *ops;

	assert(hws);
	hws->dev = NULL;

	ret = dev_read_phandle_with_args(dev, "hwlocks", "#hwlock-cells", 1,
					 index, &args);
	if (ret) {
		dev_dbg(dev, "%s: dev_read_phandle_with_args: err=%d\n",
			__func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_HWSPINLOCK,
					  args.node, &dev_hws);
	if (ret) {
		dev_dbg(dev,
			"%s: uclass_get_device_by_of_offset failed: err=%d\n",
			__func__, ret);
		return ret;
	}

	hws->dev = dev_hws;

	ops = hwspinlock_dev_ops(dev_hws);

	if (ops->of_xlate)
		ret = ops->of_xlate(hws, &args);
	else
		ret = hwspinlock_of_xlate_default(hws, &args);
	if (ret)
		dev_dbg(dev, "of_xlate() failed: %d\n", ret);

	return ret;
}

int hwspinlock_lock_timeout(struct hwspinlock *hws, unsigned int timeout)
{
	const struct hwspinlock_ops *ops;
	ulong start;
	int ret;

	assert(hws);

	if (!hws->dev)
		return -EINVAL;

	ops = hwspinlock_dev_ops(hws->dev);
	if (!ops->lock)
		return -ENOSYS;

	start = get_timer(0);
	do {
		ret = ops->lock(hws->dev, hws->id);
		if (!ret)
			return ret;

		if (ops->relax)
			ops->relax(hws->dev);
	} while (get_timer(start) < timeout);

	return -ETIMEDOUT;
}

int hwspinlock_unlock(struct hwspinlock *hws)
{
	const struct hwspinlock_ops *ops;

	assert(hws);

	if (!hws->dev)
		return -EINVAL;

	ops = hwspinlock_dev_ops(hws->dev);
	if (!ops->unlock)
		return -ENOSYS;

	return ops->unlock(hws->dev, hws->id);
}

static int hwspinlock_post_bind(struct udevice *dev)
{
#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	struct hwspinlock_ops *ops = device_get_ops(dev);
	static int reloc_done;

	if (!reloc_done) {
		if (ops->lock)
			ops->lock += gd->reloc_off;
		if (ops->unlock)
			ops->unlock += gd->reloc_off;
		if (ops->relax)
			ops->relax += gd->reloc_off;

		reloc_done++;
	}
#endif
	return 0;
}

UCLASS_DRIVER(hwspinlock) = {
	.id		= UCLASS_HWSPINLOCK,
	.name		= "hwspinlock",
	.post_bind	= hwspinlock_post_bind,
};
