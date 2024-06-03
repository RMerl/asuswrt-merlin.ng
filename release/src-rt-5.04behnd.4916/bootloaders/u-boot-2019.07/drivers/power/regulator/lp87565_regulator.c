// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Keerthy <j-keerthy@ti.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/lp87565.h>

static const char lp87565_buck_ctrl1[LP87565_BUCK_NUM] = {0x2, 0x4, 0x6, 0x8, 0x2, 0x6};
static const char lp87565_buck_vout[LP87565_BUCK_NUM] = {0xA, 0xC, 0xE, 0x10, 0xA, 0xE };

static int lp87565_buck_enable(struct udevice *dev, int op, bool *enable)
{
	int ret;
	unsigned int adr;
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);
	adr = uc_pdata->ctrl_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		ret &= LP87565_BUCK_MODE_MASK;

		if (ret)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			ret |= LP87565_BUCK_MODE_MASK;
		else
			ret &= ~LP87565_BUCK_MODE_MASK;
		ret = pmic_reg_write(dev->parent, adr, ret);
		if (ret)
			return ret;
	}

	return 0;
}

static int lp87565_buck_volt2val(int uV)
{
	if (uV > LP87565_BUCK_VOLT_MAX)
		return -EINVAL;
	else if (uV > 1400000)
		return (uV - 1420000) / 20000 + 0x9E;
	else if (uV > 730000)
		return (uV - 735000) / 5000 + 0x18;
	else if (uV >= 500000)
		return (uV - 500000) / 10000;
	else
		return -EINVAL;
}

static int lp87565_buck_val2volt(int val)
{
	if (val > LP87565_BUCK_VOLT_MAX_HEX)
		return -EINVAL;
	else if (val > 0x9D)
		return 1400000 + (val - 0x9D) * 20000;
	else if (val > 0x17)
		return 730000 + (val - 0x17) * 5000;
	else if (val >= 0x0)
		return 500000 + val * 10000;
	else
		return -EINVAL;
}

static int lp87565_buck_val(struct udevice *dev, int op, int *uV)
{
	unsigned int hex, adr;
	int ret;
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	if (op == PMIC_OP_GET)
		*uV = 0;

	adr = uc_pdata->volt_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		ret &= LP87565_BUCK_VOLT_MASK;
		ret = lp87565_buck_val2volt(ret);
		if (ret < 0)
			return ret;
		*uV = ret;

		return 0;
	}

	hex = lp87565_buck_volt2val(*uV);
	if (hex < 0)
		return hex;

	ret &= 0x0;
	ret = hex;

	ret = pmic_reg_write(dev->parent, adr, ret);

	return ret;
}

static int lp87565_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	int idx;

	uc_pdata = dev_get_uclass_platdata(dev);
	uc_pdata->type = REGULATOR_TYPE_BUCK;

	idx = dev->driver_data;
	if (idx == 0 || idx == 1 || idx == 2 || idx == 3) {
		debug("Single phase regulator\n");
	} else if (idx == 23) {
		idx = 5;
	} else if (idx == 10) {
		idx = 4;
	} else {
		printf("Wrong ID for regulator\n");
		return -EINVAL;
	}

	uc_pdata->ctrl_reg = lp87565_buck_ctrl1[idx];
	uc_pdata->volt_reg = lp87565_buck_vout[idx];

	return 0;
}

static int buck_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = lp87565_buck_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int buck_set_value(struct udevice *dev, int uV)
{
	return lp87565_buck_val(dev, PMIC_OP_SET, &uV);
}

static int buck_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;


	ret = lp87565_buck_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	return lp87565_buck_enable(dev, PMIC_OP_SET, &enable);
}

static const struct dm_regulator_ops lp87565_buck_ops = {
	.get_value  = buck_get_value,
	.set_value  = buck_set_value,
	.get_enable = buck_get_enable,
	.set_enable = buck_set_enable,
};

U_BOOT_DRIVER(lp87565_buck) = {
	.name = LP87565_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &lp87565_buck_ops,
	.probe = lp87565_buck_probe,
};
