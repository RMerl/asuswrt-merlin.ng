// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Keerthy <j-keerthy@ti.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <linux/delay.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps65941.h>

static const char tps65941_buck_ctrl[TPS65941_BUCK_NUM] = {0x4, 0x6, 0x8, 0xA,
								0xC};
static const char tps65941_buck_vout[TPS65941_BUCK_NUM] = {0xE, 0x10, 0x12,
								0x14, 0x16};
static const char tps65941_ldo_ctrl[TPS65941_BUCK_NUM] = {0x1D, 0x1E, 0x1F,
								0x20};
static const char tps65941_ldo_vout[TPS65941_BUCK_NUM] = {0x23, 0x24, 0x25,
								0x26};

static int tps65941_buck_enable(struct udevice *dev, int op, bool *enable)
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
		ret &= TPS65941_BUCK_MODE_MASK;

		if (ret)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			ret |= TPS65941_BUCK_MODE_MASK;
		else
			ret &= ~TPS65941_BUCK_MODE_MASK;
		ret = pmic_reg_write(dev->parent, adr, ret);
		if (ret)
			return ret;
	}

	return 0;
}

static int tps65941_buck_volt2val(int uV)
{
	if (uV > TPS65941_BUCK_VOLT_MAX)
		return -EINVAL;
	else if (uV > 1650000)
		return (uV - 1660000) / 20000 + 0xAB;
	else if (uV > 1110000)
		return (uV - 1110000) / 10000 + 0x73;
	else if (uV > 600000)
		return (uV - 600000) / 5000 + 0x0F;
	else if (uV >= 300000)
		return (uV - 300000) / 20000 + 0x00;
	else
		return -EINVAL;
}

static int tps65941_buck_val2volt(int val)
{
	if (val > TPS65941_BUCK_VOLT_MAX_HEX)
		return -EINVAL;
	else if (val > 0xAB)
		return 1660000 + (val - 0xAB) * 20000;
	else if (val > 0x73)
		return 1100000 + (val - 0x73) * 10000;
	else if (val > 0xF)
		return 600000 + (val - 0xF) * 5000;
	else if (val >= 0x0)
		return 300000 + val * 5000;
	else
		return -EINVAL;
}

int tps65941_lookup_slew(int id)
{
	switch (id) {
	case 0:
		return 33000;
	case 1:
		return 20000;
	case 2:
		return 10000;
	case 3:
		return 5000;
	case 4:
		return 2500;
	case 5:
		return 1300;
	case 6:
		return 630;
	case 7:
		return 310;
	default:
		return -1;
	}
}

static int tps65941_buck_val(struct udevice *dev, int op, int *uV)
{
	unsigned int hex, adr;
	int ret, delta, uwait, slew;
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);

	if (op == PMIC_OP_GET)
		*uV = 0;

	adr = uc_pdata->volt_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	ret &= TPS65941_BUCK_VOLT_MASK;
	ret = tps65941_buck_val2volt(ret);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		*uV = ret;
		return 0;
	}

	/*
	 * Compute the delta voltage, find the slew rate and wait
	 * for the appropriate amount of time after voltage switch
	 */
	if (*uV > ret)
		delta = *uV - ret;
	else
		delta = ret - *uV;

	slew = pmic_reg_read(dev->parent, uc_pdata->ctrl_reg + 1);
	if (slew < 0)
		return ret;

	slew &= TP65941_BUCK_CONF_SLEW_MASK;
	slew = tps65941_lookup_slew(slew);
	if (slew <= 0)
		return ret;

	uwait = delta / slew;

	hex = tps65941_buck_volt2val(*uV);
	if (hex < 0)
		return hex;

	ret &= 0x0;
	ret = hex;

	ret = pmic_reg_write(dev->parent, adr, ret);

	udelay(uwait);

	return ret;
}

static int tps65941_ldo_enable(struct udevice *dev, int op, bool *enable)
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
		ret &= TPS65941_LDO_MODE_MASK;

		if (ret)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			ret |= TPS65941_LDO_MODE_MASK;
		else
			ret &= ~TPS65941_LDO_MODE_MASK;
		ret = pmic_reg_write(dev->parent, adr, ret);
		if (ret)
			return ret;
	}

	return 0;
}

static int tps65941_ldo_val2volt(int val)
{
	if (val > TPS65941_LDO_VOLT_MAX_HEX || val < TPS65941_LDO_VOLT_MIN_HEX)
		return -EINVAL;
	else if (val >= TPS65941_LDO_VOLT_MIN_HEX)
		return 600000 + (val - TPS65941_LDO_VOLT_MIN_HEX) * 50000;
	else
		return -EINVAL;
}

static int tps65941_ldo_val(struct udevice *dev, int op, int *uV)
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

	ret &= TPS65941_LDO_VOLT_MASK;
	ret = tps65941_ldo_val2volt(ret);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		*uV = ret;
		return 0;
	}

	hex = tps65941_buck_volt2val(*uV);
	if (hex < 0)
		return hex;

	ret &= 0x0;
	ret = hex;

	ret = pmic_reg_write(dev->parent, adr, ret);

	return ret;
}

static int tps65941_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	int idx;

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_LDO;

	idx = dev->driver_data;
	if (idx == 1 || idx == 2 || idx == 3 || idx == 4) {
		debug("Single phase regulator\n");
	} else {
		printf("Wrong ID for regulator\n");
		return -EINVAL;
	}

	uc_pdata->ctrl_reg = tps65941_ldo_ctrl[idx - 1];
	uc_pdata->volt_reg = tps65941_ldo_vout[idx - 1];

	return 0;
}

static int tps65941_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	int idx;

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_BUCK;

	idx = dev->driver_data;
	if (idx == 1 || idx == 2 || idx == 3 || idx == 4 || idx == 5) {
		debug("Single phase regulator\n");
	} else if (idx == 12) {
		idx = 1;
	} else if (idx == 34) {
		idx = 3;
	} else if (idx == 123) {
		idx = 1;
	} else if (idx == 1234) {
		idx = 1;
	} else {
		printf("Wrong ID for regulator\n");
		return -EINVAL;
	}

	uc_pdata->ctrl_reg = tps65941_buck_ctrl[idx - 1];
	uc_pdata->volt_reg = tps65941_buck_vout[idx - 1];

	return 0;
}

static int ldo_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = tps65941_ldo_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int ldo_set_value(struct udevice *dev, int uV)
{
	return tps65941_ldo_val(dev, PMIC_OP_SET, &uV);
}

static int ldo_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = tps65941_ldo_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	return tps65941_ldo_enable(dev, PMIC_OP_SET, &enable);
}

static int buck_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = tps65941_buck_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int buck_set_value(struct udevice *dev, int uV)
{
	return tps65941_buck_val(dev, PMIC_OP_SET, &uV);
}

static int buck_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = tps65941_buck_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	return tps65941_buck_enable(dev, PMIC_OP_SET, &enable);
}

static const struct dm_regulator_ops tps65941_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = ldo_get_enable,
	.set_enable = ldo_set_enable,
};

U_BOOT_DRIVER(tps65941_ldo) = {
	.name = TPS65941_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65941_ldo_ops,
	.probe = tps65941_ldo_probe,
};

static const struct dm_regulator_ops tps65941_buck_ops = {
	.get_value  = buck_get_value,
	.set_value  = buck_set_value,
	.get_enable = buck_get_enable,
	.set_enable = buck_set_enable,
};

U_BOOT_DRIVER(tps65941_buck) = {
	.name = TPS65941_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65941_buck_ops,
	.probe = tps65941_buck_probe,
};
