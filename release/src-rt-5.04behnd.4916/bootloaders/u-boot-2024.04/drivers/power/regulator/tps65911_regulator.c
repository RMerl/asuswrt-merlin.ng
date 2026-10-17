// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps65910_pmic.h>

/* fist row is control registers, second is voltage registers */
static const char tps65911_vdd_reg[][TPS65911_VDD_NUM] = {
	{ TPS65911_REG_VDD1, TPS65911_REG_VDD2,
	  TPS65911_REG_VDDCTRL, TPS65911_REG_VIO },
	{ TPS65911_REG_VDD1_OP, TPS65911_REG_VDD2_OP,
	  TPS65911_REG_VDDCTRL_OP, 0x00 },
};

static const char tps65911_ldo_reg[TPS65911_LDO_NUM] = {
	TPS65911_REG_LDO1, TPS65911_REG_LDO2, TPS65911_REG_LDO3,
	TPS65911_REG_LDO4, TPS65911_REG_LDO5, TPS65911_REG_LDO6,
	TPS65911_REG_LDO7, TPS65911_REG_LDO8
};

static int tps65911_regulator_enable(struct udevice *dev, int op, bool *enable)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	u32 adr = uc_pdata->ctrl_reg;
	int val, ret;

	val = pmic_reg_read(dev->parent, adr);
	if (val < 0)
		return val;

	if (op == PMIC_OP_GET) {
		if (val & TPS65910_SUPPLY_STATE_ON)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		val &= ~TPS65910_SUPPLY_STATE_MASK;

		if (*enable)
			val |= TPS65910_SUPPLY_STATE_ON;

		ret = pmic_reg_write(dev->parent, adr, val);
		if (ret)
			return ret;
	}

	return 0;
}

static int tps65911_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = tps65911_regulator_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int tps65911_set_enable(struct udevice *dev, bool enable)
{
	return tps65911_regulator_enable(dev, PMIC_OP_SET, &enable);
}

/**
 * tps65911_vdd_volt2hex() - convert voltage in uV into
 *			     applicable to register hex value
 *
 * @uV:		voltage in uV
 *
 * Return: voltage in hex on success, -ve on failure
 */
static int tps65911_vdd_volt2hex(int uV)
{
	if (uV > TPS65911_VDD_VOLT_MAX)
		return -EINVAL;

	if (uV < TPS65911_VDD_VOLT_MIN)
		uV = TPS65911_VDD_VOLT_MIN;

	return (uV - TPS65911_VDD_VOLT_BASE) / 12500;
}

/**
 * tps65911_vdd_hex2volt() - convert register hex value into
 *			     actual voltage in uV
 *
 * @hex:	hex value of register
 *
 * Return: voltage in uV on success, -ve on failure
 */
static int tps65911_vdd_hex2volt(int hex)
{
	if (hex > TPS65910_VDD_SEL_MAX)
		return -EINVAL;

	if (hex < TPS65910_VDD_SEL_MIN)
		hex = TPS65910_VDD_SEL_MIN;

	return TPS65911_VDD_VOLT_BASE + hex * 12500;
}

static int tps65911_vio_range[4] = {
	1500000, 1800000, 2500000, 3300000
};

static int tps65911_vio_val(struct udevice *dev, int op, int *uV)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	u32 adr = uc_pdata->volt_reg;
	int i, val;

	val = pmic_reg_read(dev->parent, adr);
	if (val < 0)
		return val;

	if (op == PMIC_OP_GET) {
		*uV = 0;

		val &= TPS65910_SEL_MASK;

		*uV = tps65911_vio_range[val >> 2];

		return 0;
	}

	val &= ~TPS65910_SEL_MASK;

	for (i = 0; i < ARRAY_SIZE(tps65911_vio_range); i++)
		if (*uV <= tps65911_vio_range[i])
			break;

	return pmic_reg_write(dev->parent, adr, val | i << 2);
}

static int tps65911_vdd_val(struct udevice *dev, int op, int *uV)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	u32 adr = uc_pdata->volt_reg;
	int val, ret;

	/* in case vdd is vio */
	if (!adr)
		return tps65911_vio_val(dev, op, uV);

	val = pmic_reg_read(dev->parent, adr);
	if (val < 0)
		return val;

	if (op == PMIC_OP_GET) {
		*uV = 0;

		ret = tps65911_vdd_hex2volt(val);
		if (ret < 0)
			return ret;

		*uV = ret;
		return 0;
	}

	val = tps65911_vdd_volt2hex(*uV);
	if (val < 0)
		return val;

	return pmic_reg_write(dev->parent, adr, val);
}

static int tps65911_vdd_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;

	/* check for vddctrl and vddio cases */
	if (!strcmp("vddctrl", dev->name)) {
		uc_pdata->ctrl_reg = tps65911_vdd_reg[0][2];
		uc_pdata->volt_reg = tps65911_vdd_reg[1][2];
		return 0;
	}

	if (!strcmp("vddio", dev->name)) {
		uc_pdata->ctrl_reg = tps65911_vdd_reg[0][3];
		uc_pdata->volt_reg = tps65911_vdd_reg[1][3];
		return 0;
	}

	if (dev->driver_data > 0) {
		u8 idx = dev->driver_data - 1;

		uc_pdata->ctrl_reg = tps65911_vdd_reg[0][idx];
		uc_pdata->volt_reg = tps65911_vdd_reg[1][idx];
	}

	return 0;
}

static int vdd_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = tps65911_vdd_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int vdd_set_value(struct udevice *dev, int uV)
{
	return tps65911_vdd_val(dev, PMIC_OP_SET, &uV);
}

static const struct dm_regulator_ops tps65911_vdd_ops = {
	.get_value  = vdd_get_value,
	.set_value  = vdd_set_value,
	.get_enable = tps65911_get_enable,
	.set_enable = tps65911_set_enable,
};

U_BOOT_DRIVER(tps65911_vdd) = {
	.name = TPS65911_VDD_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65911_vdd_ops,
	.probe = tps65911_vdd_probe,
};

/**
 * tps65911_ldo_volt2hex() - convert voltage in uV into
 *			     applicable to register hex value
 *
 * @idx:	regulator index
 * @uV:		voltage in uV
 *
 * Return: voltage in hex on success, -ve on failure
 */
static int tps65911_ldo_volt2hex(int idx, int uV)
{
	int step;

	if (uV > TPS65911_LDO_VOLT_MAX)
		return -EINVAL;

	if (uV < TPS65911_LDO_VOLT_BASE)
		uV = TPS65911_LDO_VOLT_BASE;

	switch (idx) {
	case 1:
	case 2:
	case 4:
		step = TPS65911_LDO124_VOLT_STEP;
		break;
	case 3:
	case 5:
	case 6:
	case 7:
	case 8:
		step = TPS65911_LDO358_VOLT_STEP;
		break;
	default:
		return -EINVAL;
	};

	return ((uV - TPS65911_LDO_VOLT_BASE) / step) << 2;
}

/**
 * tps65911_ldo_hex2volt() - convert register hex value into
 *			     actual voltage in uV
 *
 * @idx:	regulator index
 * @hex:	hex value of register
 *
 * Return: voltage in uV on success, -ve on failure
 */
static int tps65911_ldo_hex2volt(int idx, int hex)
{
	int step;

	switch (idx) {
	case 1:
	case 2:
	case 4:
		if (hex > TPS65911_LDO124_VOLT_MAX_HEX)
			return -EINVAL;

		step = TPS65911_LDO124_VOLT_STEP;
		break;
	case 3:
	case 5:
	case 6:
	case 7:
	case 8:
		if (hex > TPS65911_LDO358_VOLT_MAX_HEX)
			return -EINVAL;

		if (hex < TPS65911_LDO358_VOLT_MIN_HEX)
			hex = TPS65911_LDO358_VOLT_MIN_HEX;

		step = TPS65911_LDO358_VOLT_STEP;
		break;
	default:
		return -EINVAL;
	};

	return TPS65911_LDO_VOLT_BASE + hex * step;
}

static int tps65911_ldo_val(struct udevice *dev, int op, int *uV)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	u32 adr = uc_pdata->ctrl_reg;
	int idx = dev->driver_data;
	int val, hex, ret;

	val = pmic_reg_read(dev->parent, adr);
	if (val < 0)
		return val;

	if (op == PMIC_OP_GET) {
		*uV = 0;
		val &= TPS65911_LDO_SEL_MASK;

		ret = tps65911_ldo_hex2volt(idx, val >> 2);
		if (ret < 0)
			return ret;

		*uV = ret;
		return 0;
	}

	hex = tps65911_ldo_volt2hex(idx, *uV);
	if (hex < 0)
		return hex;

	val &= ~TPS65911_LDO_SEL_MASK;

	return pmic_reg_write(dev->parent, adr, val | hex);
}

static int tps65911_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	u8 idx = dev->driver_data - 1;

	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->ctrl_reg = tps65911_ldo_reg[idx];

	return 0;
}

static int ldo_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = tps65911_ldo_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int ldo_set_value(struct udevice *dev, int uV)
{
	return tps65911_ldo_val(dev, PMIC_OP_SET, &uV);
}

static const struct dm_regulator_ops tps65911_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = tps65911_get_enable,
	.set_enable = tps65911_set_enable,
};

U_BOOT_DRIVER(tps65911_ldo) = {
	.name = TPS65911_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65911_ldo_ops,
	.probe = tps65911_ldo_probe,
};
