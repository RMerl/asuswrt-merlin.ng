// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) EETS GmbH, 2017, Felix Brack <f.brack@eets.ch>
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps65910_pmic.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "ldo_", .driver = TPS65910_LDO_DRIVER },
	{ .prefix = "buck_", .driver = TPS65910_BUCK_DRIVER },
	{ .prefix = "boost_", .driver = TPS65910_BOOST_DRIVER },
	{ },
};

static int pmic_tps65910_reg_count(struct udevice *dev)
{
	return TPS65910_NUM_REGS;
}

static int pmic_tps65910_write(struct udevice *dev, uint reg, const u8 *buffer,
			       int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buffer, len);
	if (ret)
		pr_err("%s write error on register %02x\n", dev->name, reg);

	return ret;
}

static int pmic_tps65910_read(struct udevice *dev, uint reg, u8 *buffer,
			      int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buffer, len);
	if (ret)
		pr_err("%s read error on register %02x\n", dev->name, reg);

	return ret;
}

static int pmic_tps65910_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s regulators subnode not found\n", dev->name);
		return -EINVAL;
	}

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		debug("%s has no children (regulators)\n", dev->name);

	return 0;
}

static int pmic_tps65910_probe(struct udevice *dev)
{
	/* use I2C control interface instead of I2C smartreflex interface to
	 * access smartrefelex registers VDD1_OP_REG, VDD1_SR_REG, VDD2_OP_REG
	 * and VDD2_SR_REG
	 */
	return pmic_clrsetbits(dev, TPS65910_REG_DEVICE_CTRL, 0,
			       TPS65910_I2C_SEL_MASK);
}

static struct dm_pmic_ops pmic_tps65910_ops = {
	.reg_count = pmic_tps65910_reg_count,
	.read = pmic_tps65910_read,
	.write = pmic_tps65910_write,
};

static const struct udevice_id pmic_tps65910_match[] = {
	{ .compatible = "ti,tps65910" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(pmic_tps65910) = {
	.name = "pmic_tps65910",
	.id = UCLASS_PMIC,
	.of_match = pmic_tps65910_match,
	.bind = pmic_tps65910_bind,
	.probe = pmic_tps65910_probe,
	.ops = &pmic_tps65910_ops,
};
