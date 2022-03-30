// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2016 Samsung Electronics
 *  Jaehoon Chung <jh80.chung@samsung.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/max8998_pmic.h>

static int max8998_reg_count(struct udevice *dev)
{
	return PMIC_NUM_OF_REGS;
}

static int max8998_write(struct udevice *dev, uint reg, const uint8_t *buff,
		int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret)
		pr_err("write error to device: %p register: %#x!\n", dev, reg);

	return ret;
}

static int max8998_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret)
		pr_err("read error from device: %p register: %#x!\n", dev, reg);

	return ret;
}

static struct dm_pmic_ops max8998_ops = {
	.reg_count = max8998_reg_count,
	.read	= max8998_read,
	.write	= max8998_write,
};

static const struct udevice_id max8998_ids[] = {
	{ .compatible = "maxim,max8998" },
	{ }
};

U_BOOT_DRIVER(pmic_max8998) = {
	.name		= "max8998_pmic",
	.id		= UCLASS_PMIC,
	.of_match	= max8998_ids,
	.ops		= &max8998_ops,
};
