// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/tps65090.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "fet", .driver = TPS65090_FET_DRIVER },
	{ },
};

static int tps65090_reg_count(struct udevice *dev)
{
	return TPS65090_NUM_REGS;
}

static int tps65090_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int tps65090_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret) {
		pr_err("read error %d from device: %p register: %#x!\n", ret,
		       dev, reg);
		return -EIO;
	}

	return 0;
}

static int tps65090_bind(struct udevice *dev)
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
		debug("%s: %s - no child found\n", __func__, dev->name);

	/* Always return success for this device */
	return 0;
}

static struct dm_pmic_ops tps65090_ops = {
	.reg_count = tps65090_reg_count,
	.read = tps65090_read,
	.write = tps65090_write,
};

static const struct udevice_id tps65090_ids[] = {
	{ .compatible = "ti,tps65090" },
	{ }
};

U_BOOT_DRIVER(pmic_tps65090) = {
	.name = "tps65090 pmic",
	.id = UCLASS_PMIC,
	.of_match = tps65090_ids,
	.bind = tps65090_bind,
	.ops = &tps65090_ops,
};
