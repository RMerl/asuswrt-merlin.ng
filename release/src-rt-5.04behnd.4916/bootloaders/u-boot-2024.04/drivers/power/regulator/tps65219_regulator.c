// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <linux/delay.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps65219.h>

static const unsigned int tps65219_buck_vout[TPS65219_BUCK_NUM] = {
	[0] = TPS65219_BUCK1_VOUT_REG,
	[1] = TPS65219_BUCK2_VOUT_REG,
	[2] = TPS65219_BUCK3_VOUT_REG
};

static const unsigned int tps65219_ldo_vout[TPS65219_LDO_NUM] = {
	[0] = TPS65219_LDO1_VOUT_REG,
	[1] = TPS65219_LDO2_VOUT_REG,
	[2] = TPS65219_LDO3_VOUT_REG,
	[3] = TPS65219_LDO4_VOUT_REG,
};

static int tps65219_reg_enable(struct udevice *dev, unsigned int adr, int idx,
			       int op, bool *enable)
{
	int ret;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		if (ret & BIT(idx))
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			ret |= BIT(idx);
		else
			ret &= ~BIT(idx);

		ret = pmic_reg_write(dev->parent, adr, ret);
		if (ret)
			return ret;
	}

	return 0;
}

static int tps65219_buck_enable(struct udevice *dev, int op, bool *enable)
{
	unsigned int adr;
	struct dm_regulator_uclass_plat *uc_pdata;
	int idx;

	idx = dev->driver_data - 1;
	uc_pdata = dev_get_uclass_plat(dev);
	adr = uc_pdata->ctrl_reg;

	return tps65219_reg_enable(dev, adr, idx, op, enable);
}

static int tps65219_buck_volt2val(int uV)
{
	if (uV > TPS65219_BUCK_VOLT_MAX)
		return -EINVAL;
	else if (uV >= 1400000)
		return (uV - 1400000) / 100000 + 0x20;
	else if (uV >= 600000)
		return (uV - 600000) / 25000 + 0x00;
	else
		return -EINVAL;
}

static int tps65219_buck_val2volt(int val)
{
	if (val > TPS65219_VOLT_MASK)
		return -EINVAL;
	else if (val > 0x34)
		return TPS65219_BUCK_VOLT_MAX;
	else if (val > 0x20)
		return 1400000 + (val - 0x20) * 100000;
	else if (val >= 0)
		return 600000 + val * 25000;
	else
		return -EINVAL;
}

static int tps65219_buck_val(struct udevice *dev, int op, int *uV)
{
	unsigned int adr;
	int ret, val;
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	adr = uc_pdata->volt_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		*uV = 0;

		ret &= TPS65219_VOLT_MASK;
		ret = tps65219_buck_val2volt(ret);
		if (ret < 0)
			return ret;

		*uV = ret;
		return 0;
	}

	val = tps65219_buck_volt2val(*uV);
	if (val < 0)
		return val;

	ret &= ~TPS65219_VOLT_MASK;
	ret |= val;

	ret = pmic_reg_write(dev->parent, adr, ret);

	udelay(100);

	return ret;
}

static int tps65219_ldo_enable(struct udevice *dev, int op, bool *enable)
{
	unsigned int adr;
	struct dm_regulator_uclass_plat *uc_pdata;
	int idx;

	idx = TPS65219_BUCK_NUM + (dev->driver_data - 1);
	uc_pdata = dev_get_uclass_plat(dev);
	adr = uc_pdata->ctrl_reg;

	return tps65219_reg_enable(dev, adr, idx, op, enable);
}

static int tps65219_ldo_volt2val(int idx, int uV)
{
	int base = TPS65219_LDO12_VOLT_MIN;
	int max = TPS65219_LDO12_VOLT_MAX;

	if (idx > 1) {
		base = TPS65219_LDO34_VOLT_MIN;
		max = TPS65219_LDO34_VOLT_MAX;
	}

	if (uV > max)
		return -EINVAL;
	else if (uV >= base)
		return (uV - TPS65219_LDO12_VOLT_MIN) / 50000;
	else
		return -EINVAL;
}

static int tps65219_ldo_val2volt(int idx, int val)
{
	int reg_base = TPS65219_LDO12_VOLT_REG_MIN;
	int reg_max = TPS65219_LDO12_VOLT_REG_MAX;
	int base = TPS65219_LDO12_VOLT_MIN;
	int max = TPS65219_LDO12_VOLT_MAX;

	if (idx > 1) {
		base = TPS65219_LDO34_VOLT_MIN;
		max = TPS65219_LDO34_VOLT_MAX;
		reg_base = TPS65219_LDO34_VOLT_REG_MIN;
		reg_max = TPS65219_LDO34_VOLT_REG_MAX;
	}

	if (val > TPS65219_VOLT_MASK || val < 0)
		return -EINVAL;
	else if (val >= reg_max)
		return max;
	else if (val <= reg_base)
		return base;
	else if (val >= 0)
		return TPS65219_LDO12_VOLT_MIN + (50000 * val);
	else
		return -EINVAL;
}

static int tps65219_ldo_val(struct udevice *dev, int op, int *uV)
{
	unsigned int adr;
	int ret, val;
	struct dm_regulator_uclass_plat *uc_pdata;
	int idx;

	idx = dev->driver_data - 1;
	uc_pdata = dev_get_uclass_plat(dev);
	adr = uc_pdata->volt_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		*uV = 0;

		ret &= TPS65219_VOLT_MASK;
		ret = tps65219_ldo_val2volt(idx, ret);
		if (ret < 0)
			return ret;

		*uV = ret;
		return 0;
	}

	/* LDO1 & LDO2 in BYPASS mode only supports 1.5V max */
	if (idx < 2 &&
	    (ret & BIT(TPS65219_LDO12_BYP_CONFIG)) &&
	    *uV < TPS65219_LDO12_VOLT_BYP_MIN)
		return -EINVAL;

	val = tps65219_ldo_volt2val(idx, *uV);
	if (val < 0)
		return val;

	ret &= ~TPS65219_VOLT_MASK;
	ret |= val;

	ret = pmic_reg_write(dev->parent, adr, ret);

	udelay(100);

	return ret;
}

static int tps65219_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	int idx;

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_LDO;

	/* idx must be in 1..TPS65219_LDO_NUM */
	idx = dev->driver_data;
	if (idx < 1 || idx > TPS65219_LDO_NUM) {
		printf("Wrong ID for regulator\n");
		return -EINVAL;
	}

	uc_pdata->ctrl_reg = TPS65219_ENABLE_CTRL_REG;
	uc_pdata->volt_reg = tps65219_ldo_vout[idx - 1];

	return 0;
}

static int tps65219_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	int idx;

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_BUCK;

	/* idx must be in 1..TPS65219_BUCK_NUM */
	idx = dev->driver_data;
	if (idx < 1 || idx > TPS65219_BUCK_NUM) {
		printf("Wrong ID for regulator\n");
		return -EINVAL;
	}

	uc_pdata->ctrl_reg = TPS65219_ENABLE_CTRL_REG;
	uc_pdata->volt_reg = tps65219_buck_vout[idx - 1];

	return 0;
}

static int ldo_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = tps65219_ldo_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int ldo_set_value(struct udevice *dev, int uV)
{
	return tps65219_ldo_val(dev, PMIC_OP_SET, &uV);
}

static int ldo_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = tps65219_ldo_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	return tps65219_ldo_enable(dev, PMIC_OP_SET, &enable);
}

static int buck_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = tps65219_buck_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int buck_set_value(struct udevice *dev, int uV)
{
	return tps65219_buck_val(dev, PMIC_OP_SET, &uV);
}

static int buck_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = tps65219_buck_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	return tps65219_buck_enable(dev, PMIC_OP_SET, &enable);
}

static const struct dm_regulator_ops tps65219_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = ldo_get_enable,
	.set_enable = ldo_set_enable,
};

U_BOOT_DRIVER(tps65219_ldo) = {
	.name = TPS65219_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65219_ldo_ops,
	.probe = tps65219_ldo_probe,
};

static const struct dm_regulator_ops tps65219_buck_ops = {
	.get_value  = buck_get_value,
	.set_value  = buck_set_value,
	.get_enable = buck_get_enable,
	.set_enable = buck_set_enable,
};

U_BOOT_DRIVER(tps65219_buck) = {
	.name = TPS65219_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65219_buck_ops,
	.probe = tps65219_buck_probe,
};
