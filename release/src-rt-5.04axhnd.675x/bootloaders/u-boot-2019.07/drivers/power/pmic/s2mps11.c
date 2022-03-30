// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2015 Samsung Electronics
 *  Przemyslaw Marczak  <p.marczak@samsung.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/s2mps11.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = S2MPS11_OF_LDO_PREFIX, .driver = S2MPS11_LDO_DRIVER },
	{ .prefix = S2MPS11_OF_BUCK_PREFIX, .driver = S2MPS11_BUCK_DRIVER },
	{ },
};

static int s2mps11_reg_count(struct udevice *dev)
{
	return S2MPS11_REG_COUNT;
}

static int s2mps11_write(struct udevice *dev, uint reg, const uint8_t *buff,
			 int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret)
		pr_err("write error to device: %p register: %#x!\n", dev, reg);

	return ret;
}

static int s2mps11_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret)
		pr_err("read error from device: %p register: %#x!\n", dev, reg);

	return ret;
}

static int s2mps11_probe(struct udevice *dev)
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

	return 0;
}

static struct dm_pmic_ops s2mps11_ops = {
	.reg_count = s2mps11_reg_count,
	.read = s2mps11_read,
	.write = s2mps11_write,
};

static const struct udevice_id s2mps11_ids[] = {
	{ .compatible = "samsung,s2mps11-pmic" },
	{ }
};

U_BOOT_DRIVER(pmic_s2mps11) = {
	.name = "s2mps11_pmic",
	.id = UCLASS_PMIC,
	.of_match = s2mps11_ids,
	.ops = &s2mps11_ops,
	.probe = s2mps11_probe,
};
