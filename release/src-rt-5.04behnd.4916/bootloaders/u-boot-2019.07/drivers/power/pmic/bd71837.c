// SPDX-License-Identifier:      GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/bd71837.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct pmic_child_info pmic_children_info[] = {
	/* buck */
	{ .prefix = "b", .driver = BD71837_REGULATOR_DRIVER},
	/* ldo */
	{ .prefix = "l", .driver = BD71837_REGULATOR_DRIVER},
	{ },
};

static int bd71837_reg_count(struct udevice *dev)
{
	return BD71837_REG_NUM;
}

static int bd71837_write(struct udevice *dev, uint reg, const uint8_t *buff,
			 int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int bd71837_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int bd71837_bind(struct udevice *dev)
{
	int children;
	ofnode regulators_node;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!", __func__,
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

static struct dm_pmic_ops bd71837_ops = {
	.reg_count = bd71837_reg_count,
	.read = bd71837_read,
	.write = bd71837_write,
};

static const struct udevice_id bd71837_ids[] = {
	{ .compatible = "rohm,bd71837", .data = 0x4b, },
	{ }
};

U_BOOT_DRIVER(pmic_bd71837) = {
	.name = "bd71837 pmic",
	.id = UCLASS_PMIC,
	.of_match = bd71837_ids,
	.bind = bd71837_bind,
	.ops = &bd71837_ops,
};
