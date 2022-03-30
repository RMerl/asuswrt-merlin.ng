// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm-demo.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/list.h>

DECLARE_GLOBAL_DATA_PTR;

UCLASS_DRIVER(demo) = {
	.name		= "demo",
	.id		= UCLASS_DEMO,
};

int demo_hello(struct udevice *dev, int ch)
{
	const struct demo_ops *ops = device_get_ops(dev);

	if (!ops->hello)
		return -ENOSYS;

	return ops->hello(dev, ch);
}

int demo_status(struct udevice *dev, int *status)
{
	const struct demo_ops *ops = device_get_ops(dev);

	if (!ops->status)
		return -ENOSYS;

	return ops->status(dev, status);
}

int demo_get_light(struct udevice *dev)
{
	const struct demo_ops *ops = device_get_ops(dev);

	if (!ops->get_light)
		return -ENOSYS;

	return ops->get_light(dev);
}

int demo_set_light(struct udevice *dev, int light)
{
	const struct demo_ops *ops = device_get_ops(dev);

	if (!ops->set_light)
		return -ENOSYS;

	return ops->set_light(dev, light);
}

int demo_parse_dt(struct udevice *dev)
{
	struct dm_demo_pdata *pdata = dev_get_platdata(dev);
	int dn = dev_of_offset(dev);

	pdata->sides = fdtdec_get_int(gd->fdt_blob, dn, "sides", 0);
	pdata->colour = fdt_getprop(gd->fdt_blob, dn, "colour", NULL);
	if (!pdata->sides || !pdata->colour) {
		debug("%s: Invalid device tree data\n", __func__);
		return -EINVAL;
	}

	return 0;
}
