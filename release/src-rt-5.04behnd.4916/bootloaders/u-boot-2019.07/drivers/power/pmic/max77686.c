// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2014-2015 Samsung Electronics
 *  Przemyslaw Marczak  <p.marczak@samsung.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/max77686_pmic.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "LDO", .driver = MAX77686_LDO_DRIVER },
	{ .prefix = "BUCK", .driver = MAX77686_BUCK_DRIVER },
	{ },
};

static int max77686_reg_count(struct udevice *dev)
{
	return MAX77686_NUM_OF_REGS;
}

static int max77686_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int max77686_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int max77686_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children;

	regulators_node = dev_read_subnode(dev, "voltage-regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!\n", __func__,
		      dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	/* Always return success for this device */
	return 0;
}

static struct dm_pmic_ops max77686_ops = {
	.reg_count = max77686_reg_count,
	.read = max77686_read,
	.write = max77686_write,
};

static const struct udevice_id max77686_ids[] = {
	{ .compatible = "maxim,max77686" },
	{ }
};

U_BOOT_DRIVER(pmic_max77686) = {
	.name = "max77686_pmic",
	.id = UCLASS_PMIC,
	.of_match = max77686_ids,
	.bind = max77686_bind,
	.ops = &max77686_ops,
};
