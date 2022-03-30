// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <errno.h>
#include <mtd.h>

/**
 * mtd_probe - Probe the device @dev if not already done
 *
 * @dev: U-Boot device to probe
 *
 * @return 0 on success, an error otherwise.
 */
int mtd_probe(struct udevice *dev)
{
	if (device_active(dev))
		return 0;

	return device_probe(dev);
}

/*
 * Implement a MTD uclass which should include most flash drivers.
 * The uclass private is pointed to mtd_info.
 */

UCLASS_DRIVER(mtd) = {
	.id		= UCLASS_MTD,
	.name		= "mtd",
	.per_device_auto_alloc_size = sizeof(struct mtd_info),
};
