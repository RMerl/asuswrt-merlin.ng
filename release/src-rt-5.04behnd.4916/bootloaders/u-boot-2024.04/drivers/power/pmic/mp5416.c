// SPDX-License-Identifier:      GPL-2.0+
/*
 * Copyright 2020 Gateworks Corporation
 */
#include <common.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/mp5416.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct pmic_child_info pmic_children_info[] = {
	/* buck */
	{ .prefix = "b", .driver = MP6416_REGULATOR_DRIVER },
	/* ldo */
	{ .prefix = "l", .driver = MP6416_REGULATOR_DRIVER },
	{ },
};

static int mp5416_reg_count(struct udevice *dev)
{
	return MP5416_NUM_OF_REGS - 1;
}

static int mp5416_write(struct udevice *dev, uint reg, const uint8_t *buff,
			int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int mp5416_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int mp5416_bind(struct udevice *dev)
{
	int children;
	ofnode regulators_node;

	debug("%s %s\n", __func__, dev->name);
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

static int mp5416_probe(struct udevice *dev)
{
	debug("%s %s\n", __func__, dev->name);

	return 0;
}

static struct dm_pmic_ops mp5416_ops = {
	.reg_count = mp5416_reg_count,
	.read = mp5416_read,
	.write = mp5416_write,
};

static const struct udevice_id mp5416_ids[] = {
	{ .compatible = "mps,mp5416", },
	{ }
};

U_BOOT_DRIVER(pmic_mp5416) = {
	.name = "mp5416 pmic",
	.id = UCLASS_PMIC,
	.of_match = mp5416_ids,
	.bind = mp5416_bind,
	.probe = mp5416_probe,
	.ops = &mp5416_ops,
};
