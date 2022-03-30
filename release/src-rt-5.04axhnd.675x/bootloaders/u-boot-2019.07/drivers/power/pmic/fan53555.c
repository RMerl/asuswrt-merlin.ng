// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) 2018 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>

static int pmic_fan53555_reg_count(struct udevice *dev)
{
	return 1;
};

static int pmic_fan53555_read(struct udevice *dev, uint reg,
			      u8 *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("%s: read error for register: %#x!", dev->name, reg);
		return -EIO;
	}

	return 0;
}

static int pmic_fan53555_write(struct udevice *dev, uint reg,
			       const u8 *buff, int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("%s: write error for register: %#x!", dev->name, reg);
		return -EIO;
	}

	return 0;
}

static int pmic_fan53555_bind(struct udevice *dev)
{
	/*
	 * The FAN53555 has only a single regulator and therefore doesn't
	 * have a subnode.  So we have to rebind a child device (the one
	 * regulator) here.
	 */

	const char *regulator_driver_name = "fan53555_regulator";
	struct udevice *child;
	struct driver *drv;

	debug("%s\n", __func__);

	drv = lists_driver_lookup_name(regulator_driver_name);
	if (!drv) {
		dev_err(dev, "no driver '%s'\n", regulator_driver_name);
		return -ENOENT;
	}

	return device_bind_with_driver_data(dev, drv, "SW", 0,
					    dev_ofnode(dev), &child);
};

static struct dm_pmic_ops pmic_fan53555_ops = {
	.reg_count = pmic_fan53555_reg_count,
	.read = pmic_fan53555_read,
	.write = pmic_fan53555_write,
};

static const struct udevice_id pmic_fan53555_match[] = {
	{ .compatible = "fcs,fan53555" },
	{ },
};

U_BOOT_DRIVER(pmic_fan53555) = {
	.name = "pmic_fan53555",
	.id = UCLASS_PMIC,
	.of_match = pmic_fan53555_match,
	.bind = pmic_fan53555_bind,
	.ops = &pmic_fan53555_ops,
};
