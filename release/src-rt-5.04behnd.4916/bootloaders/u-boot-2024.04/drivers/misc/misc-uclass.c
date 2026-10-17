// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 */

#define LOG_CATEGORY UCLASS_MISC

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <misc.h>

/*
 * Implement a  miscellaneous uclass for those do not fit other more
 * general classes. A set of generic read, write and ioctl methods may
 * be used to access the device.
 */

int misc_read(struct udevice *dev, int offset, void *buf, int size)
{
	const struct misc_ops *ops = device_get_ops(dev);

	if (!ops->read)
		return -ENOSYS;

	return ops->read(dev, offset, buf, size);
}

int misc_write(struct udevice *dev, int offset, const void *buf, int size)
{
	const struct misc_ops *ops = device_get_ops(dev);

	if (!ops->write)
		return -ENOSYS;

	return ops->write(dev, offset, buf, size);
}

int misc_ioctl(struct udevice *dev, unsigned long request, void *buf)
{
	const struct misc_ops *ops = device_get_ops(dev);

	if (!ops->ioctl)
		return -ENOSYS;

	return ops->ioctl(dev, request, buf);
}

int misc_call(struct udevice *dev, int msgid, void *tx_msg, int tx_size,
	      void *rx_msg, int rx_size)
{
	const struct misc_ops *ops = device_get_ops(dev);

	if (!ops->call)
		return -ENOSYS;

	return ops->call(dev, msgid, tx_msg, tx_size, rx_msg, rx_size);
}

int misc_set_enabled(struct udevice *dev, bool val)
{
	const struct misc_ops *ops = device_get_ops(dev);

	if (!ops->set_enabled)
		return -ENOSYS;

	return ops->set_enabled(dev, val);
}

UCLASS_DRIVER(misc) = {
	.id		= UCLASS_MISC,
	.name		= "misc",
#if CONFIG_IS_ENABLED(OF_REAL)
	.post_bind	= dm_scan_fdt_dev,
#endif
};
