// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Freescale Semiconductor, Inc
 */

#define LOG_CATEGORY UCLASS_THERMAL

#include <common.h>
#include <dm.h>
#include <thermal.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/list.h>


int thermal_get_temp(struct udevice *dev, int *temp)
{
	const struct dm_thermal_ops *ops = device_get_ops(dev);

	if (!ops->get_temp)
		return -ENOSYS;

	return ops->get_temp(dev, temp);
}

UCLASS_DRIVER(thermal) = {
	.id		= UCLASS_THERMAL,
	.name		= "thermal",
};
