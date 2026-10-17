// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Texas Instruments Incorporated, <www.ti.com>
 * Keerthy <j-keerthy@ti.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <linux/printk.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps65941.h>
#include <dm/device.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "ldo", .driver = TPS65941_LDO_DRIVER },
	{ .prefix = "buck", .driver = TPS65941_BUCK_DRIVER },
	{ },
};

static int tps65941_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int tps65941_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int tps65941_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!\n", __func__,
		      dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		printf("%s: %s - no child found\n", __func__, dev->name);

	/* Probe all the child devices */
	return dm_scan_fdt_dev(dev);
}

static struct dm_pmic_ops tps65941_ops = {
	.read = tps65941_read,
	.write = tps65941_write,
};

static const struct udevice_id tps65941_ids[] = {
	{ .compatible = "ti,tps659411", .data = TPS659411 },
	{ .compatible = "ti,tps659412", .data = TPS659411 },
	{ .compatible = "ti,tps659413", .data = TPS659413 },
	{ .compatible = "ti,lp876441",  .data =  LP876441 },
	{ }
};

U_BOOT_DRIVER(pmic_tps65941) = {
	.name = "tps65941_pmic",
	.id = UCLASS_PMIC,
	.of_match = tps65941_ids,
	.bind = tps65941_bind,
	.ops = &tps65941_ops,
};
