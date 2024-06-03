// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Texas Instruments Incorporated, <www.ti.com>
 * Keerthy <j-keerthy@ti.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/lp873x.h>
#include <dm/device.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "ldo", .driver = LP873X_LDO_DRIVER },
	{ .prefix = "buck", .driver = LP873X_BUCK_DRIVER },
	{ },
};

static int lp873x_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int lp873x_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int lp873x_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!\n", __func__,
		      dev->name);
		return -ENXIO;
	}

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		printf("%s: %s - no child found\n", __func__, dev->name);

	/* Always return success for this device */
	return 0;
}

static struct dm_pmic_ops lp873x_ops = {
	.read = lp873x_read,
	.write = lp873x_write,
};

static const struct udevice_id lp873x_ids[] = {
	{ .compatible = "ti,lp8732", .data = LP8732 },
	{ .compatible = "ti,lp8733" , .data = LP8733 },
	{ }
};

U_BOOT_DRIVER(pmic_lp873x) = {
	.name = "lp873x_pmic",
	.id = UCLASS_PMIC,
	.of_match = lp873x_ids,
	.bind = lp873x_bind,
	.ops = &lp873x_ops,
};
