// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_RAM

#include <common.h>
#include <ram.h>
#include <dm.h>
#include <errno.h>
#include <dm/lists.h>
#include <dm/root.h>

int ram_get_info(struct udevice *dev, struct ram_info *info)
{
	struct ram_ops *ops = ram_get_ops(dev);

	if (!ops->get_info)
		return -ENOSYS;

	return ops->get_info(dev, info);
}

UCLASS_DRIVER(ram) = {
	.id		= UCLASS_RAM,
	.name		= "ram",
};
