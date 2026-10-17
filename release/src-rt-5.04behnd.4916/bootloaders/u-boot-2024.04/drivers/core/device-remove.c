// SPDX-License-Identifier: GPL-2.0+
/*
 * Device manager
 *
 * Copyright (c) 2014 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#define LOG_CATEGORY	LOGC_DM

#include <common.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <power-domain.h>
#include <asm/global_data.h>

int device_chld_unbind(struct udevice *dev, struct driver *drv)
{
	struct udevice *pos, *n;
	int ret, saved_ret = 0;

	assert(dev);

	device_foreach_child_safe(pos, n, dev) {
		if (drv && (pos->driver != drv))
			continue;

		ret = device_unbind(pos);
		if (ret && !saved_ret) {
			log_warning("device '%s' failed to unbind\n",
				    pos->name);
			saved_ret = ret;
		}
	}

	return log_ret(saved_ret);
}

int device_chld_remove(struct udevice *dev, struct driver *drv,
		       uint flags)
{
	struct udevice *pos, *n;
	int result = 0;

	assert(dev);

	device_foreach_child_safe(pos, n, dev) {
		int ret;

		if (drv && (pos->driver != drv))
			continue;

		ret = device_remove(pos, flags);
		if (ret == -EPROBE_DEFER)
			result = ret;
		else if (ret && ret != -EKEYREJECTED)
			return ret;
	}

	return result;
}

int device_unbind(struct udevice *dev)
{
	const struct driver *drv;
	int ret;

	if (!dev)
		return log_msg_ret("dev", -EINVAL);

	if (dev_get_flags(dev) & DM_FLAG_ACTIVATED)
		return log_msg_ret("active", -EINVAL);

	if (!(dev_get_flags(dev) & DM_FLAG_BOUND))
		return log_msg_ret("not-bound", -EINVAL);

	drv = dev->driver;
	assert(drv);

	if (drv->unbind) {
		ret = drv->unbind(dev);
		if (ret)
			return log_msg_ret("unbind", ret);
	}

	ret = device_chld_unbind(dev, NULL);
	if (ret)
		return log_msg_ret("child unbind", ret);

	ret = uclass_pre_unbind_device(dev);
	if (ret)
		return log_msg_ret("uc", ret);
	if (dev_get_flags(dev) & DM_FLAG_ALLOC_PDATA) {
		free(dev_get_plat(dev));
		dev_set_plat(dev, NULL);
	}
	if (dev_get_flags(dev) & DM_FLAG_ALLOC_UCLASS_PDATA) {
		free(dev_get_uclass_plat(dev));
		dev_set_uclass_plat(dev, NULL);
	}
	if (dev_get_flags(dev) & DM_FLAG_ALLOC_PARENT_PDATA) {
		free(dev_get_parent_plat(dev));
		dev_set_parent_plat(dev, NULL);
	}
	ret = uclass_unbind_device(dev);
	if (ret)
		return log_msg_ret("uc", ret);

	if (dev->parent)
		list_del(&dev->sibling_node);

	devres_release_all(dev);

	if (dev_get_flags(dev) & DM_FLAG_NAME_ALLOCED)
		free((char *)dev->name);
	free(dev);

	return 0;
}

/**
 * device_free() - Free memory buffers allocated by a device
 * @dev:	Device that is to be started
 */
void device_free(struct udevice *dev)
{
	int size;

	if (dev->driver->priv_auto) {
		free(dev_get_priv(dev));
		dev_set_priv(dev, NULL);
	}
	size = dev->uclass->uc_drv->per_device_auto;
	if (size) {
		free(dev_get_uclass_priv(dev));
		dev_set_uclass_priv(dev, NULL);
	}
	if (dev->parent) {
		size = dev->parent->driver->per_child_auto;
		if (!size)
			size = dev->parent->uclass->uc_drv->per_child_auto;
		if (size) {
			free(dev_get_parent_priv(dev));
			dev_set_parent_priv(dev, NULL);
		}
	}
	dev_bic_flags(dev, DM_FLAG_PLATDATA_VALID);

	devres_release_probe(dev);
}

/**
 * flags_remove() - Figure out whether to remove a device
 *
 * If this is called with @flags == DM_REMOVE_NON_VITAL | DM_REMOVE_ACTIVE_DMA,
 * then it returns 0 (=go head and remove) if the device is not matked vital
 * but is marked DM_REMOVE_ACTIVE_DMA.
 *
 * If this is called with @flags == DM_REMOVE_ACTIVE_DMA,
 * then it returns 0 (=go head and remove) if the device is marked
 * DM_REMOVE_ACTIVE_DMA, regardless of whether it is marked vital.
 *
 * @flags: Flags passed to device_remove()
 * @drv_flags: Driver flags
 * Return: 0 if the device should be removed,
 * -EKEYREJECTED if @flags includes a flag in DM_REMOVE_ACTIVE_ALL but
 *	@drv_flags does not (indicates that this device has nothing to do for
 *	DMA shutdown or OS prepare)
 * -EPROBE_DEFER if @flags is DM_REMOVE_NON_VITAL but @drv_flags contains
 *	DM_FLAG_VITAL (indicates the device is vital and should not be removed)
 */
static int flags_remove(uint flags, uint drv_flags)
{
	if (!(flags & DM_REMOVE_NORMAL)) {
		bool vital_match;
		bool active_match;

		active_match = !(flags & DM_REMOVE_ACTIVE_ALL) ||
			(drv_flags & flags);
		vital_match = !(flags & DM_REMOVE_NON_VITAL) ||
			!(drv_flags & DM_FLAG_VITAL);
		if (!vital_match)
			return -EPROBE_DEFER;
		if (!active_match)
			return -EKEYREJECTED;
	}

	return 0;
}

int device_remove(struct udevice *dev, uint flags)
{
	const struct driver *drv;
	int ret;

	if (!dev)
		return -EINVAL;

	if (!(dev_get_flags(dev) & DM_FLAG_ACTIVATED))
		return 0;

	ret = device_notify(dev, EVT_DM_PRE_REMOVE);
	if (ret)
		return ret;

	/*
	 * If the child returns EKEYREJECTED, continue. It just means that it
	 * didn't match the flags.
	 */
	ret = device_chld_remove(dev, NULL, flags);
	if (ret && ret != -EKEYREJECTED)
		return ret;

	/*
	 * Remove the device if called with the "normal" remove flag set,
	 * or if the remove flag matches any of the drivers remove flags
	 */
	drv = dev->driver;
	assert(drv);
	ret = flags_remove(flags, drv->flags);
	if (ret) {
		log_debug("%s: When removing: flags=%x, drv->flags=%x, err=%d\n",
			  dev->name, flags, drv->flags, ret);
		return ret;
	}

	ret = uclass_pre_remove_device(dev);
	if (ret)
		return ret;

	if (drv->remove) {
		ret = drv->remove(dev);
		if (ret)
			goto err_remove;
	}

	if (dev->parent && dev->parent->driver->child_post_remove) {
		ret = dev->parent->driver->child_post_remove(dev);
		if (ret) {
			dm_warn("%s: Device '%s' failed child_post_remove()",
				__func__, dev->name);
		}
	}

	if (!(flags & DM_REMOVE_NO_PD) &&
	    !(drv->flags &
	      (DM_FLAG_DEFAULT_PD_CTRL_OFF | DM_FLAG_LEAVE_PD_ON)) &&
	    dev != gd->cur_serial_dev)
		dev_power_domain_off(dev);

	device_free(dev);

	dev_bic_flags(dev, DM_FLAG_ACTIVATED);

	ret = device_notify(dev, EVT_DM_POST_REMOVE);
	if (ret)
		goto err_remove;

	return 0;

err_remove:
	/* We can't put the children back */
	dm_warn("%s: Device '%s' failed to remove, but children are gone\n",
		__func__, dev->name);

	return ret;
}
