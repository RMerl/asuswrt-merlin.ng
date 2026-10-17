// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps65219.h>
#include <dm/device.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "ldo", .driver = TPS65219_LDO_DRIVER },
	{ .prefix = "buck", .driver = TPS65219_BUCK_DRIVER },
	{ },
};

static int tps65219_reg_count(struct udevice *dev)
{
	return 0x41;
}

static int tps65219_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int tps65219_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int tps65219_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!\n", __func__,
		      dev->name);
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		printf("%s: %s - no child found\n", __func__, dev->name);

	/* Probe all the child devices */
	return dm_scan_fdt_dev(dev);
}

static struct dm_pmic_ops tps65219_ops = {
	.reg_count = tps65219_reg_count,
	.read = tps65219_read,
	.write = tps65219_write,
};

static const struct udevice_id tps65219_ids[] = {
	{ .compatible = "ti,tps65219" },
	{ }
};

U_BOOT_DRIVER(pmic_tps65219) = {
	.name = "tps65219_pmic",
	.id = UCLASS_PMIC,
	.of_match = tps65219_ids,
	.bind = tps65219_bind,
	.ops = &tps65219_ops,
};
