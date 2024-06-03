// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/s5m8767.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "LDO", .driver = S5M8767_LDO_DRIVER },
	{ .prefix = "BUCK", .driver = S5M8767_BUCK_DRIVER },
	{ },
};

static int s5m8767_reg_count(struct udevice *dev)
{
	return S5M8767_NUM_OF_REGS;
}

static int s5m8767_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int s5m8767_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

int s5m8767_enable_32khz_cp(struct udevice *dev)
{
	return pmic_clrsetbits(dev, S5M8767_EN32KHZ_CP, 0, 1 << 1);
}

static int s5m8767_bind(struct udevice *dev)
{
	int children;
	ofnode node;

	node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(node)) {
		debug("%s: %s regulators subnode not found!\n", __func__,
		      dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, node, pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	/* Always return success for this device */
	return 0;
}

static struct dm_pmic_ops s5m8767_ops = {
	.reg_count = s5m8767_reg_count,
	.read = s5m8767_read,
	.write = s5m8767_write,
};

static const struct udevice_id s5m8767_ids[] = {
	{ .compatible = "samsung,s5m8767-pmic" },
	{ }
};

U_BOOT_DRIVER(pmic_s5m8767) = {
	.name = "s5m8767_pmic",
	.id = UCLASS_PMIC,
	.of_match = s5m8767_ids,
	.bind = s5m8767_bind,
	.ops = &s5m8767_ops,
};
