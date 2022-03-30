// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 NVIDIA Corporation
 */

#define pr_fmt(fmt) "as3722: " fmt

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <i2c.h>
#include <dm/lists.h>
#include <power/as3722.h>
#include <power/pmic.h>

#define AS3722_NUM_OF_REGS	0x92

static int as3722_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret < 0)
		return ret;

	return 0;
}

static int as3722_write(struct udevice *dev, uint reg, const uint8_t *buff,
			int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret < 0)
		return ret;

	return 0;
}

static int as3722_read_id(struct udevice *dev, uint *idp, uint *revisionp)
{
	int ret;

	ret = pmic_reg_read(dev, AS3722_ASIC_ID1);
	if (ret < 0) {
		pr_err("failed to read ID1 register: %d\n", ret);
		return ret;
	}
	*idp = ret;

	ret = pmic_reg_read(dev, AS3722_ASIC_ID2);
	if (ret < 0) {
		pr_err("failed to read ID2 register: %d\n", ret);
		return ret;
	}
	*revisionp = ret;

	return 0;
}

/* TODO(treding@nvidia.com): Add proper regulator support to avoid this */
int as3722_sd_set_voltage(struct udevice *dev, unsigned int sd, u8 value)
{
	int ret;

	if (sd > 6)
		return -EINVAL;

	ret = pmic_reg_write(dev, AS3722_SD_VOLTAGE(sd), value);
	if (ret < 0) {
		pr_err("failed to write SD%u voltage register: %d\n", sd, ret);
		return ret;
	}

	return 0;
}

int as3722_ldo_set_voltage(struct udevice *dev, unsigned int ldo, u8 value)
{
	int ret;

	if (ldo > 11)
		return -EINVAL;

	ret = pmic_reg_write(dev, AS3722_LDO_VOLTAGE(ldo), value);
	if (ret < 0) {
		pr_err("failed to write LDO%u voltage register: %d\n", ldo,
		       ret);
		return ret;
	}

	return 0;
}

static int as3722_probe(struct udevice *dev)
{
	uint id, revision;
	int ret;

	ret = as3722_read_id(dev, &id, &revision);
	if (ret < 0) {
		pr_err("failed to read ID: %d\n", ret);
		return ret;
	}

	if (id != AS3722_DEVICE_ID) {
		pr_err("unknown device\n");
		return -ENOENT;
	}

	debug("AS3722 revision %#x found on I2C bus %s\n", revision, dev->name);

	return 0;
}

#if CONFIG_IS_ENABLED(PMIC_CHILDREN)
static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "sd", .driver = "as3722_stepdown"},
	{ .prefix = "ldo", .driver = "as3722_ldo"},
	{ },
};

static int as3722_bind(struct udevice *dev)
{
	struct udevice *gpio_dev;
	ofnode regulators_node;
	int children;
	int ret;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found\n", __func__,
		      dev->name);
		return -ENXIO;
	}

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);
	ret = device_bind_driver(dev, "gpio_as3722", "gpio_as3722", &gpio_dev);
	if (ret) {
		debug("%s: Cannot bind GPIOs (ret=%d)\n", __func__, ret);
		return ret;
	}

	return 0;
}
#endif

static int as3722_reg_count(struct udevice *dev)
{
	return AS3722_NUM_OF_REGS;
}

static struct dm_pmic_ops as3722_ops = {
	.reg_count = as3722_reg_count,
	.read = as3722_read,
	.write = as3722_write,
};

static const struct udevice_id as3722_ids[] = {
	{ .compatible = "ams,as3722" },
	{ }
};

U_BOOT_DRIVER(pmic_as3722) = {
	.name = "as3722_pmic",
	.id = UCLASS_PMIC,
	.of_match = as3722_ids,
#if CONFIG_IS_ENABLED(PMIC_CHILDREN)
	.bind = as3722_bind,
#endif
	.probe = as3722_probe,
	.ops = &as3722_ops,
};
