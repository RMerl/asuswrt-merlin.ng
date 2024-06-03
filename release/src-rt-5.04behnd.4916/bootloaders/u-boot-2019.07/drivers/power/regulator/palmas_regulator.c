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
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/palmas.h>

#define	REGULATOR_ON		0x1
#define	REGULATOR_OFF		0x0

#define	SMPS_MODE_MASK		0x3
#define	SMPS_MODE_SHIFT		0x0
#define	LDO_MODE_MASK		0x1
#define	LDO_MODE_SHIFT		0x0

static const char palmas_smps_ctrl[][PALMAS_SMPS_NUM] = {
	{0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c},
	{0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38},
	{0x20, 0x24, 0x2c, 0x30, 0x38},
};

static const char palmas_smps_volt[][PALMAS_SMPS_NUM] = {
	{0x23, 0x27, 0x2b, 0x2f, 0x33, 0x37, 0x3b, 0x3c},
	{0x23, 0x27, 0x2b, 0x2f, 0x33, 0x37, 0x3b},
	{0x23, 0x27, 0x2f, 0x33, 0x3B}
};

static const char palmas_ldo_ctrl[][PALMAS_LDO_NUM] = {
	{0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e, 0x60, 0x62, 0x64},
	{0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e, 0x60, 0x62, 0x64},
	{0x50, 0x52, 0x54, 0x5e, 0x62}
};

static const char palmas_ldo_volt[][PALMAS_LDO_NUM] = {
	{0x51, 0x53, 0x55, 0x57, 0x59, 0x5b, 0x5d, 0x5f, 0x61, 0x63, 0x65},
	{0x51, 0x53, 0x55, 0x57, 0x59, 0x5b, 0x5d, 0x5f, 0x61, 0x63, 0x65},
	{0x51, 0x53, 0x55, 0x5f, 0x63}
};

static int palmas_smps_enable(struct udevice *dev, int op, bool *enable)
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
		ret &= PALMAS_SMPS_STATUS_MASK;

		if (ret)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			ret |= PALMAS_SMPS_MODE_MASK;
		else
			ret &= ~(PALMAS_SMPS_MODE_MASK);

		ret = pmic_reg_write(dev->parent, adr, ret);
		if (ret)
			return ret;
	}

	return 0;
}

static int palmas_smps_volt2hex(int uV)
{
	if (uV > PALMAS_LDO_VOLT_MAX)
		return -EINVAL;

	if (uV > 1650000)
		return (uV - 1000000) / 20000 + 0x6;

	if (uV == 500000)
		return 0x6;
	else
		return 0x6 + ((uV - 500000) / 10000);
}

static int palmas_smps_hex2volt(int hex, bool range)
{
	unsigned int uV = 0;

	if (hex > PALMAS_SMPS_VOLT_MAX_HEX)
		return -EINVAL;

	if (hex < 0x7)
		uV = 500000;
	else
		uV = 500000 + (hex - 0x6) * 10000;

	if (range)
		uV *= 2;

	return uV;
}

static int palmas_smps_val(struct udevice *dev, int op, int *uV)
{
	unsigned int hex, adr;
	int ret;
	bool range;
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	if (op == PMIC_OP_GET)
		*uV = 0;

	adr = uc_pdata->volt_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		if (ret & PALMAS_SMPS_RANGE_MASK)
			range =  true;
		else
			range = false;

		ret &= PALMAS_SMPS_VOLT_MASK;
		ret = palmas_smps_hex2volt(ret, range);
		if (ret < 0)
			return ret;
		*uV = ret;

		return 0;
	}

	hex = palmas_smps_volt2hex(*uV);
	if (hex < 0)
		return hex;

	ret &= ~PALMAS_SMPS_VOLT_MASK;
	ret |= hex;
	if (*uV > 1650000)
		ret |= PALMAS_SMPS_RANGE_MASK;

	return pmic_reg_write(dev->parent, adr, ret);
}

static int palmas_ldo_bypass_enable(struct udevice *dev, bool enabled)
{
	int type = dev_get_driver_data(dev_get_parent(dev));
	struct dm_regulator_uclass_platdata *p;
	unsigned int adr;
	int reg;

	if (type == TPS65917) {
		/* bypass available only on LDO1 and LDO2 */
		if (dev->driver_data > 2)
			return -ENOTSUPP;
	} else if (type == TPS659038) {
		/* bypass available only on LDO9 */
		if (dev->driver_data != 9)
			return -ENOTSUPP;
	}

	p = dev_get_uclass_platdata(dev);
	adr = p->ctrl_reg;

	reg = pmic_reg_read(dev->parent, adr);
	if (reg < 0)
		return reg;

	if (enabled)
		reg |= PALMAS_LDO_BYPASS_EN;
	else
		reg &= ~PALMAS_LDO_BYPASS_EN;

	return pmic_reg_write(dev->parent, adr, reg);
}

static int palmas_ldo_enable(struct udevice *dev, int op, bool *enable)
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
		ret &= PALMAS_LDO_STATUS_MASK;

		if (ret)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			ret |= PALMAS_LDO_MODE_MASK;
		else
			ret &= ~(PALMAS_LDO_MODE_MASK);

		ret = pmic_reg_write(dev->parent, adr, ret);
		if (ret)
			return ret;

		ret = palmas_ldo_bypass_enable(dev, false);
		if (ret && (ret != -ENOTSUPP))
			return ret;
	}

	return 0;
}

static int palmas_ldo_volt2hex(int uV)
{
	if (uV > PALMAS_LDO_VOLT_MAX)
		return -EINVAL;

	return (uV - 850000) / 50000;
}

static int palmas_ldo_hex2volt(int hex)
{
	if (hex > PALMAS_LDO_VOLT_MAX_HEX)
		return -EINVAL;

	if (!hex)
		return 0;

	return (hex * 50000) + 850000;
}

static int palmas_ldo_val(struct udevice *dev, int op, int *uV)
{
	unsigned int hex, adr;
	int ret;

	struct dm_regulator_uclass_platdata *uc_pdata;

	if (op == PMIC_OP_GET)
		*uV = 0;

	uc_pdata = dev_get_uclass_platdata(dev);

	adr = uc_pdata->volt_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		ret &= PALMAS_LDO_VOLT_MASK;
		ret = palmas_ldo_hex2volt(ret);
		if (ret < 0)
			return ret;
		*uV = ret;
		return 0;
	}

	hex = palmas_ldo_volt2hex(*uV);
	if (hex < 0)
		return hex;

	ret &= ~PALMAS_LDO_VOLT_MASK;
	ret |= hex;
	if (*uV > 1650000)
		ret |= 0x80;

	return pmic_reg_write(dev->parent, adr, ret);
}

static int palmas_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct udevice *parent;

	uc_pdata = dev_get_uclass_platdata(dev);

	parent = dev_get_parent(dev);
	int type = dev_get_driver_data(parent);

	uc_pdata->type = REGULATOR_TYPE_LDO;

	if (dev->driver_data) {
		u8 idx = dev->driver_data - 1;
		uc_pdata->ctrl_reg = palmas_ldo_ctrl[type][idx];
		uc_pdata->volt_reg = palmas_ldo_volt[type][idx];
	} else {
		/* check for ldoln and ldousb cases */
		if (!strcmp("ldoln", dev->name)) {
			uc_pdata->ctrl_reg = palmas_ldo_ctrl[type][9];
			uc_pdata->volt_reg = palmas_ldo_volt[type][9];
		} else if (!strcmp("ldousb", dev->name)) {
			uc_pdata->ctrl_reg = palmas_ldo_ctrl[type][10];
			uc_pdata->volt_reg = palmas_ldo_volt[type][10];
		}
	}

	return 0;
}

static int ldo_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = palmas_ldo_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int ldo_set_value(struct udevice *dev, int uV)
{
	return palmas_ldo_val(dev, PMIC_OP_SET, &uV);
}

static int ldo_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = palmas_ldo_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	return palmas_ldo_enable(dev, PMIC_OP_SET, &enable);
}

static int palmas_smps_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct udevice *parent;
	int idx;

	uc_pdata = dev_get_uclass_platdata(dev);

	parent = dev_get_parent(dev);
	int type = dev_get_driver_data(parent);

	uc_pdata->type = REGULATOR_TYPE_BUCK;

	switch (type) {
	case PALMAS:
	case TPS659038:
		switch (dev->driver_data) {
		case 123:
		case 12:
			uc_pdata->ctrl_reg = palmas_smps_ctrl[type][0];
			uc_pdata->volt_reg = palmas_smps_volt[type][0];
			break;
		case 3:
			uc_pdata->ctrl_reg = palmas_smps_ctrl[type][1];
			uc_pdata->volt_reg = palmas_smps_volt[type][1];
			break;
		case 45:
			uc_pdata->ctrl_reg = palmas_smps_ctrl[type][2];
			uc_pdata->volt_reg = palmas_smps_volt[type][2];
			break;
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			idx = dev->driver_data - 3;
			uc_pdata->ctrl_reg = palmas_smps_ctrl[type][idx];
			uc_pdata->volt_reg = palmas_smps_volt[type][idx];
			break;

		default:
			printf("Wrong ID for regulator\n");
		}
		break;

	case TPS65917:
		switch (dev->driver_data) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			idx = dev->driver_data - 1;
			uc_pdata->ctrl_reg = palmas_smps_ctrl[type][idx];
			uc_pdata->volt_reg = palmas_smps_volt[type][idx];
			break;
		case 12:
			idx = 0;
			uc_pdata->ctrl_reg = palmas_smps_ctrl[type][idx];
			uc_pdata->volt_reg = palmas_smps_volt[type][idx];
			break;
		default:
			printf("Wrong ID for regulator\n");
		}
		break;

	default:
			printf("Invalid PMIC ID\n");
	}

	return 0;
}

static int smps_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = palmas_smps_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int smps_set_value(struct udevice *dev, int uV)
{
	return palmas_smps_val(dev, PMIC_OP_SET, &uV);
}

static int smps_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = palmas_smps_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int smps_set_enable(struct udevice *dev, bool enable)
{
	return palmas_smps_enable(dev, PMIC_OP_SET, &enable);
}

static const struct dm_regulator_ops palmas_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = ldo_get_enable,
	.set_enable = ldo_set_enable,
};

U_BOOT_DRIVER(palmas_ldo) = {
	.name = PALMAS_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &palmas_ldo_ops,
	.probe = palmas_ldo_probe,
};

static const struct dm_regulator_ops palmas_smps_ops = {
	.get_value  = smps_get_value,
	.set_value  = smps_set_value,
	.get_enable = smps_get_enable,
	.set_enable = smps_set_enable,
};

U_BOOT_DRIVER(palmas_smps) = {
	.name = PALMAS_SMPS_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &palmas_smps_ops,
	.probe = palmas_smps_probe,
};
