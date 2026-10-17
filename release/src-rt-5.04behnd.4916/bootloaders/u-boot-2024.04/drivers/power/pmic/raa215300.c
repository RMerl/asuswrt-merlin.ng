// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Renesas Electronics Corporation
 */

#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <i2c.h>
#include <power/pmic.h>

#define RAA215300_REG_COUNT 0x80

static int raa215300_reg_count(struct udevice *dev)
{
	return RAA215300_REG_COUNT;
}

static struct dm_pmic_ops raa215300_ops = {
	.reg_count = raa215300_reg_count,
	.read = dm_i2c_read,
	.write = dm_i2c_write,
};

static const struct udevice_id raa215300_ids[] = {
	{ .compatible = "renesas,raa215300" },
	{ /* sentinel */ }
};

static int raa215300_bind(struct udevice *dev)
{
	if (IS_ENABLED(CONFIG_SYSRESET_RAA215300)) {
		struct driver *drv = lists_driver_lookup_name("raa215300_sysreset");
		if (!drv)
			return -ENOENT;

		return device_bind(dev, drv, dev->name, NULL, dev_ofnode(dev),
				   NULL);
	}

	return 0;
}

U_BOOT_DRIVER(raa215300_pmic) = {
	.name = "raa215300_pmic",
	.id = UCLASS_PMIC,
	.of_match = raa215300_ids,
	.bind = raa215300_bind,
	.ops = &raa215300_ops,
};
