// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Keerthy <j-keerthy@ti.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/lp873x.h>

static const char lp873x_buck_ctrl[LP873X_BUCK_NUM] = {0x2, 0x4};
static const char lp873x_buck_volt[LP873X_BUCK_NUM] = {0x6, 0x7};
static const char lp873x_ldo_ctrl[LP873X_LDO_NUM] = {0x8, 0x9};
static const char lp873x_ldo_volt[LP873X_LDO_NUM] = {0xA, 0xB};

static int lp873x_buck_enable(struct udevice *dev, int op, bool *enable)
{
	int ret;
	unsigned int adr;
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	adr = uc_pdata->ctrl_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		ret &= LP873X_BUCK_MODE_MASK;

		if (ret)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			ret |= LP873X_BUCK_MODE_MASK;
		else
			ret &= ~(LP873X_BUCK_MODE_MASK);
		ret = pmic_reg_write(dev->parent, adr, ret);
		if (ret)
			return ret;
	}

	return 0;
}

static int lp873x_buck_volt2hex(int uV)
{
	if (uV > LP873X_BUCK_VOLT_MAX)
		return -EINVAL;
	else if (uV > 1400000)
		return (uV - 1420000) / 20000 + 0x9E;
	else if (uV > 730000)
		return (uV - 735000) / 5000 + 0x18;
	else if (uV >= 700000)
		return (uV - 700000) / 10000 + 0x1;
	else
		return -EINVAL;
}

static int lp873x_buck_hex2volt(int hex)
{
	if (hex > LP873X_BUCK_VOLT_MAX_HEX)
		return -EINVAL;
	else if (hex > 0x9D)
		return 1400000 + (hex - 0x9D) * 20000;
	else if (hex > 0x17)
		return 730000 + (hex - 0x17) * 5000;
	else if (hex >= 0x14)
		return 700000 + (hex - 0x14) * 10000;
	else
		return -EINVAL;
}

static int lp873x_buck_val(struct udevice *dev, int op, int *uV)
{
	unsigned int hex, adr;
	int ret;
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);

	if (op == PMIC_OP_GET)
		*uV = 0;

	adr = uc_pdata->volt_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		ret &= LP873X_BUCK_VOLT_MASK;
		ret = lp873x_buck_hex2volt(ret);
		if (ret < 0)
			return ret;
		*uV = ret;

		return 0;
	}

	hex = lp873x_buck_volt2hex(*uV);
	if (hex < 0)
		return hex;

	ret &= 0x0;
	ret |= hex;

	ret = pmic_reg_write(dev->parent, adr, ret);

	return ret;
}

static int lp873x_ldo_enable(struct udevice *dev, int op, bool *enable)
{
	int ret;
	unsigned int adr;
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	adr = uc_pdata->ctrl_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		ret &= LP873X_LDO_MODE_MASK;

		if (ret)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			ret |= LP873X_LDO_MODE_MASK;
		else
			ret &= ~(LP873X_LDO_MODE_MASK);

		ret = pmic_reg_write(dev->parent, adr, ret);
		if (ret)
			return ret;
	}

	return 0;
}

static int lp873x_ldo_volt2hex(int uV)
{
	if (uV > LP873X_LDO_VOLT_MAX)
		return -EINVAL;

	return (uV - 800000) / 100000;
}

static int lp873x_ldo_hex2volt(int hex)
{
	if (hex > LP873X_LDO_VOLT_MAX_HEX)
		return -EINVAL;

	if (!hex)
		return 0;

	return (hex * 100000) + 800000;
}

static int lp873x_ldo_val(struct udevice *dev, int op, int *uV)
{
	unsigned int hex, adr;
	int ret;

	struct dm_regulator_uclass_plat *uc_pdata;

	if (op == PMIC_OP_GET)
		*uV = 0;

	uc_pdata = dev_get_uclass_plat(dev);

	adr = uc_pdata->volt_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		ret &= LP873X_LDO_VOLT_MASK;
		ret = lp873x_ldo_hex2volt(ret);
		if (ret < 0)
			return ret;
		*uV = ret;
		return 0;
	}

	hex = lp873x_ldo_volt2hex(*uV);
	if (hex < 0)
		return hex;

	ret &= ~LP873X_LDO_VOLT_MASK;
	ret |= hex;
	if (*uV > 1650000)
		ret |= 0x80;
	ret = pmic_reg_write(dev->parent, adr, ret);

	return ret;
}

static int lp873x_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_LDO;

	int idx = dev->driver_data;
	if (idx >= LP873X_LDO_NUM) {
		printf("Wrong ID for regulator\n");
		return -1;
	}

	uc_pdata->ctrl_reg = lp873x_ldo_ctrl[idx];
	uc_pdata->volt_reg = lp873x_ldo_volt[idx];

	return 0;
}

static int ldo_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = lp873x_ldo_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int ldo_set_value(struct udevice *dev, int uV)
{
	return lp873x_ldo_val(dev, PMIC_OP_SET, &uV);
}

static int ldo_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = lp873x_ldo_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	return lp873x_ldo_enable(dev, PMIC_OP_SET, &enable);
}

static int lp873x_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	int idx;

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_BUCK;

	idx = dev->driver_data;
	if (idx >= LP873X_BUCK_NUM) {
		printf("Wrong ID for regulator\n");
		return -1;
	}

	uc_pdata->ctrl_reg = lp873x_buck_ctrl[idx];
	uc_pdata->volt_reg = lp873x_buck_volt[idx];

	return 0;
}

static int buck_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = lp873x_buck_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int buck_set_value(struct udevice *dev, int uV)
{
	return lp873x_buck_val(dev, PMIC_OP_SET, &uV);
}

static int buck_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;


	ret = lp873x_buck_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	return lp873x_buck_enable(dev, PMIC_OP_SET, &enable);
}

static const struct dm_regulator_ops lp873x_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = ldo_get_enable,
	.set_enable = ldo_set_enable,
};

U_BOOT_DRIVER(lp873x_ldo) = {
	.name = LP873X_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &lp873x_ldo_ops,
	.probe = lp873x_ldo_probe,
};

static const struct dm_regulator_ops lp873x_buck_ops = {
	.get_value  = buck_get_value,
	.set_value  = buck_set_value,
	.get_enable = buck_get_enable,
	.set_enable = buck_set_enable,
};

U_BOOT_DRIVER(lp873x_buck) = {
	.name = LP873X_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &lp873x_buck_ops,
	.probe = lp873x_buck_probe,
};
