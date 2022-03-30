// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Placeholder regulator driver for as3722.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <power/as3722.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define AS3722_LDO_CONTROL0_MAX_INDEX 7

static int stepdown_get_value(struct udevice *dev)
{
	return -ENOSYS;
}

static int stepdown_set_value(struct udevice *dev, int uvolt)
{
	return -ENOSYS;
}

static int stepdown_set_enable(struct udevice *dev, bool enable)
{
	struct udevice *pmic = dev_get_parent(dev);
	int sd = dev->driver_data;
	int ret;

	ret = pmic_clrsetbits(pmic, AS3722_SD_CONTROL, 0, 1 << sd);
	if (ret < 0) {
		debug("%s: failed to write SD control register: %d", __func__,
		      ret);
		return ret;
	}

	return 0;
}

static int stepdown_get_enable(struct udevice *dev)
{
	struct udevice *pmic = dev_get_parent(dev);
	int sd = dev->driver_data;
	int ret;

	ret = pmic_reg_read(pmic, AS3722_SD_CONTROL);
	if (ret < 0) {
		debug("%s: failed to read SD control register: %d", __func__,
		      ret);
		return ret;
	}

	return ret & (1 << sd) ? true : false;
}

static int ldo_get_value(struct udevice *dev)
{
	return -ENOSYS;
}

static int ldo_set_value(struct udevice *dev, int uvolt)
{
	return -ENOSYS;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	struct udevice *pmic = dev_get_parent(dev);
	u8 ctrl_reg = AS3722_LDO_CONTROL0;
	int ldo = dev->driver_data;
	int ret;

	if (ldo > AS3722_LDO_CONTROL0_MAX_INDEX) {
		ctrl_reg = AS3722_LDO_CONTROL1;
		ldo -= 8;
	}

	ret = pmic_clrsetbits(pmic, ctrl_reg, !enable << ldo, enable << ldo);
	if (ret < 0) {
		debug("%s: failed to write LDO control register: %d", __func__,
		      ret);
		return ret;
	}

	return 0;
}

static int ldo_get_enable(struct udevice *dev)
{
	struct udevice *pmic = dev_get_parent(dev);
	u8 ctrl_reg = AS3722_LDO_CONTROL0;
	int ldo = dev->driver_data;
	int ret;

	if (ldo > AS3722_LDO_CONTROL0_MAX_INDEX) {
		ctrl_reg = AS3722_LDO_CONTROL1;
		ldo -= 8;
	}

	ret = pmic_reg_read(pmic, ctrl_reg);
	if (ret < 0) {
		debug("%s: failed to read SD control register: %d", __func__,
		      ret);
		return ret;
	}

	return ret & (1 << ldo) ? true : false;
}

static int as3722_stepdown_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;

	return 0;
}

static int as3722_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_LDO;

	return 0;
}

static const struct dm_regulator_ops as3722_stepdown_ops = {
	.get_value  = stepdown_get_value,
	.set_value  = stepdown_set_value,
	.get_enable = stepdown_get_enable,
	.set_enable = stepdown_set_enable,
};

static const struct dm_regulator_ops as3722_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = ldo_get_enable,
	.set_enable = ldo_set_enable,
};

U_BOOT_DRIVER(as3722_stepdown) = {
	.name = "as3722_stepdown",
	.id = UCLASS_REGULATOR,
	.ops = &as3722_stepdown_ops,
	.probe = as3722_stepdown_probe,
};

U_BOOT_DRIVER(as3722_ldo) = {
	.name = "as3722_ldo",
	.id = UCLASS_REGULATOR,
	.ops = &as3722_ldo_ops,
	.probe = as3722_ldo_probe,
};
