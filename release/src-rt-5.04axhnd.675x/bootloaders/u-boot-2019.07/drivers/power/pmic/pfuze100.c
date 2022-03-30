// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc
 * Peng Fan <Peng.Fan@freescale.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/pfuze100_pmic.h>
#include <power/pfuze3000_pmic.h>

static const struct pmic_child_info pmic_children_info[] = {
	/* sw[x], swbst */
	{ .prefix = "s", .driver = PFUZE100_REGULATOR_DRIVER },
	/* vgen[x], vsnvs, vcc, v33, vcc_sd */
	{ .prefix = "v", .driver = PFUZE100_REGULATOR_DRIVER },
	{ },
};

static int pfuze100_reg_count(struct udevice *dev)
{
	return dev->driver_data == PFUZE3000 ? PFUZE3000_NUM_OF_REGS : PFUZE100_NUM_OF_REGS;
}

static int pfuze100_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int pfuze100_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int pfuze100_bind(struct udevice *dev)
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

static struct dm_pmic_ops pfuze100_ops = {
	.reg_count = pfuze100_reg_count,
	.read = pfuze100_read,
	.write = pfuze100_write,
};

static const struct udevice_id pfuze100_ids[] = {
	{ .compatible = "fsl,pfuze100", .data = PFUZE100, },
	{ .compatible = "fsl,pfuze200", .data = PFUZE200, },
	{ .compatible = "fsl,pfuze3000", .data = PFUZE3000, },
	{ }
};

U_BOOT_DRIVER(pmic_pfuze100) = {
	.name = "pfuze100 pmic",
	.id = UCLASS_PMIC,
	.of_match = pfuze100_ids,
	.bind = pfuze100_bind,
	.ops = &pfuze100_ops,
};
