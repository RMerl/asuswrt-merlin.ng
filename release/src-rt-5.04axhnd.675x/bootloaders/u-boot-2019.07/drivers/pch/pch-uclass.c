// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <pch.h>

int pch_get_spi_base(struct udevice *dev, ulong *sbasep)
{
	struct pch_ops *ops = pch_get_ops(dev);

	*sbasep = 0;
	if (!ops->get_spi_base)
		return -ENOSYS;

	return ops->get_spi_base(dev, sbasep);
}

int pch_set_spi_protect(struct udevice *dev, bool protect)
{
	struct pch_ops *ops = pch_get_ops(dev);

	if (!ops->set_spi_protect)
		return -ENOSYS;

	return ops->set_spi_protect(dev, protect);
}

int pch_get_gpio_base(struct udevice *dev, u32 *gbasep)
{
	struct pch_ops *ops = pch_get_ops(dev);

	*gbasep = 0;
	if (!ops->get_gpio_base)
		return -ENOSYS;

	return ops->get_gpio_base(dev, gbasep);
}

int pch_get_io_base(struct udevice *dev, u32 *iobasep)
{
	struct pch_ops *ops = pch_get_ops(dev);

	*iobasep = 0;
	if (!ops->get_io_base)
		return -ENOSYS;

	return ops->get_io_base(dev, iobasep);
}

int pch_ioctl(struct udevice *dev, ulong req, void *data, int size)
{
	struct pch_ops *ops = pch_get_ops(dev);

	if (!ops->ioctl)
		return -ENOSYS;

	return ops->ioctl(dev, req, data, size);
}

UCLASS_DRIVER(pch) = {
	.id		= UCLASS_PCH,
	.name		= "pch",
	.post_bind	= dm_scan_fdt_dev,
};
