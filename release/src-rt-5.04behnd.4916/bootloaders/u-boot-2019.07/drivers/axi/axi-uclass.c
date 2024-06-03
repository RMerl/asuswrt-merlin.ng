// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <axi.h>

int axi_read(struct udevice *dev, ulong address, void *data,
	     enum axi_size_t size)
{
	struct axi_ops *ops = axi_get_ops(dev);

	if (!ops->read)
		return -ENOSYS;

	return ops->read(dev, address, data, size);
}

int axi_write(struct udevice *dev, ulong address, void *data,
	      enum axi_size_t size)
{
	struct axi_ops *ops = axi_get_ops(dev);

	if (!ops->write)
		return -ENOSYS;

	return ops->write(dev, address, data, size);
}

UCLASS_DRIVER(axi) = {
	.id		= UCLASS_AXI,
	.name		= "axi",
	.post_bind	= dm_scan_fdt_dev,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};

