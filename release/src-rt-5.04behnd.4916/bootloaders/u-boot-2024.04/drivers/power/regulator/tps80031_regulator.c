// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps80031.h>

static const char tps80031_smps_reg[][TPS80031_SMPS_NUM] = {
	{   0x54,   0x5a,   0x66,   0x42,   0x48 },
	{   0x56,   0x5c,   0x68,   0x44,   0x4a },
	{ BIT(3), BIT(4), BIT(6), BIT(0), BIT(1) },
};

static const char tps80031_ldo_reg[][TPS80031_LDO_NUM] = {
	{ 0x9e, 0x86, 0x8e, 0x8a, 0x9a, 0x92, 0xa6, 0x96, 0xa2 },
	{ 0x9f, 0x87, 0x8f, 0x8b, 0x9b, 0x93, 0xa7, 0x97, 0xa3 },
};

static int tps80031_regulator_enable(struct udevice *dev, int op, bool *enable)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	u32 adr = uc_pdata->ctrl_reg;
	int val, ret;

	val = pmic_reg_read(dev->parent, adr);
	if (val < 0)
		return val;

	if (op == PMIC_OP_GET) {
		if (val & REGULATOR_MODE_ON)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		val &= ~REGULATOR_STATUS_MASK;

		if (*enable)
			val |= REGULATOR_MODE_ON;

		ret = pmic_reg_write(dev->parent, adr, val);
		if (ret)
			return ret;
	}

	return 0;
}

static int tps80031_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = tps80031_regulator_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int tps80031_set_enable(struct udevice *dev, bool enable)
{
	return tps80031_regulator_enable(dev, PMIC_OP_SET, &enable);
}

/**
 * tps80031_ldo_volt2hex() - convert voltage in uV into
 *			     applicable to register hex value
 *
 * @uV:		voltage in uV
 *
 * Return: voltage in hex on success, -ve on failure
 */
static int tps80031_ldo_volt2hex(int uV)
{
	if (uV > LDO_VOLT_MAX)
		return -EINVAL;

	if (uV < LDO_VOLT_MIN)
		uV = LDO_VOLT_MIN;

	return DIV_ROUND_UP(uV - LDO_VOLT_BASE, 102000);
}

/**
 * tps80031_ldo_hex2volt() - convert register hex value into
 *			     actual voltage in uV
 *
 * @hex:	hex value of register
 *
 * Return: voltage in uV on success, -ve on failure
 */
static int tps80031_ldo_hex2volt(int hex)
{
	if (hex > LDO_VOLT_MAX_HEX)
		return -EINVAL;

	if (hex < LDO_VOLT_MIN_HEX)
		hex = LDO_VOLT_MIN_HEX;

	return LDO_VOLT_BASE + hex * 102000;
}

static int tps80031_ldo_val(struct udevice *dev, int op, int *uV)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	u32 adr = uc_pdata->volt_reg;
	int val, hex, ret;

	val = pmic_reg_read(dev->parent, adr);
	if (val < 0)
		return val;

	if (op == PMIC_OP_GET) {
		*uV = 0;

		ret = tps80031_ldo_hex2volt(val & LDO_VOLT_MASK);
		if (ret < 0)
			return ret;

		*uV = ret;
		return 0;
	}

	hex = tps80031_ldo_volt2hex(*uV);
	if (hex < 0)
		return hex;

	val &= ~LDO_VOLT_MASK;

	return pmic_reg_write(dev->parent, adr, val | hex);
}

static int tps80031_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);

	uc_pdata->type = REGULATOR_TYPE_LDO;

	/* check for ldoln and ldousb cases */
	if (!strcmp("ldoln", dev->name)) {
		uc_pdata->ctrl_reg = tps80031_ldo_reg[CTRL][7];
		uc_pdata->volt_reg = tps80031_ldo_reg[VOLT][7];
		return 0;
	}

	if (!strcmp("ldousb", dev->name)) {
		uc_pdata->ctrl_reg = tps80031_ldo_reg[CTRL][8];
		uc_pdata->volt_reg = tps80031_ldo_reg[VOLT][8];
		return 0;
	}

	if (dev->driver_data > 0) {
		u8 idx = dev->driver_data - 1;

		uc_pdata->ctrl_reg = tps80031_ldo_reg[CTRL][idx];
		uc_pdata->volt_reg = tps80031_ldo_reg[VOLT][idx];
	}

	return 0;
}

static int ldo_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = tps80031_ldo_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int ldo_set_value(struct udevice *dev, int uV)
{
	return tps80031_ldo_val(dev, PMIC_OP_SET, &uV);
}

static const struct dm_regulator_ops tps80031_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = tps80031_get_enable,
	.set_enable = tps80031_set_enable,
};

U_BOOT_DRIVER(tps80031_ldo) = {
	.name = TPS80031_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps80031_ldo_ops,
	.probe = tps80031_ldo_probe,
};

struct tps80031_smps_priv {
	int flags;
};

/* DCDC voltages for the selector of 0x39 to 0x3F */
static int tps80031_dcdc_voltages[5] = {
	1350000, 1500000, 1800000, 1900000, 2100000
};

/**
 * tps80031_smps_volt2hex() - convert voltage in uV into
 *			      applicable to register hex value
 *
 * @base:	base voltage in uV
 * @uV:		voltage in uV
 *
 * Return: voltage in hex on success, -ve on failure
 */
static int tps80031_smps_volt2hex(u32 base, int uV)
{
	int i;

	if (uV < base)
		return 1;

	if (uV > SMPS_VOLT_LINEAR) {
		for (i = 0; i < ARRAY_SIZE(tps80031_dcdc_voltages); i++)
			if (uV <= tps80031_dcdc_voltages[i])
				break;

		return SMPS_VOLT_NLINEAR_HEX + i;
	}

	return DIV_ROUND_UP(uV - base, 12500);
}

/**
 * tps80031_smps_hex2volt() - convert register hex value into
 *			      actual voltage in uV
 *
 * @base:	base voltage in uV
 * @hex:	hex value of register
 *
 * Return: voltage in uV on success, -ve on failure
 */
static int tps80031_smps_hex2volt(u32 base, int hex)
{
	if (!hex)
		return 0;

	/* if reg value exceeds linear scale use table */
	if (hex > SMPS_VOLT_LINEAR_HEX)
		return tps80031_dcdc_voltages[hex - SMPS_VOLT_LINEAR_HEX];
	else
		return base + hex * 12500;
}

static int tps80031_smps_val(struct udevice *dev, int op, int *uV)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	struct tps80031_smps_priv *priv = dev_get_priv(dev);
	u32 adr = uc_pdata->volt_reg;
	int base, val, hex, ret;

	/* If offset flag was set then base voltage is higher */
	if (priv->flags & TPS80031_OFFSET_FLAG)
		base = SMPS_VOLT_BASE_OFFSET;
	else
		base = SMPS_VOLT_BASE;

	val = pmic_reg_read(dev->parent, adr);
	if (val < 0)
		return val;

	if (op == PMIC_OP_GET) {
		*uV = 0;

		ret = tps80031_smps_hex2volt(base, val & SMPS_VOLT_MASK);
		if (ret < 0)
			return ret;

		*uV = ret;
		return 0;
	}

	hex = tps80031_smps_volt2hex(base, *uV);
	if (hex < 0)
		return hex;

	val &= ~SMPS_VOLT_MASK;

	return pmic_reg_write(dev->parent, adr, val | hex);
}

static int tps80031_smps_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	struct tps80031_smps_priv *priv = dev_get_priv(dev);
	int idx = dev->driver_data - 1;
	int val;

	uc_pdata->type = REGULATOR_TYPE_BUCK;

	uc_pdata->ctrl_reg = tps80031_smps_reg[CTRL][idx];
	uc_pdata->volt_reg = tps80031_smps_reg[VOLT][idx];

	/* Determine if smps regulator uses higher voltage */
	val = pmic_reg_read(dev->parent, TPS80031_SMPS_OFFSET);
	if (val & tps80031_smps_reg[OFFSET][idx])
		priv->flags |= TPS80031_OFFSET_FLAG;

	return 0;
}

static int smps_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = tps80031_smps_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int smps_set_value(struct udevice *dev, int uV)
{
	return tps80031_smps_val(dev, PMIC_OP_SET, &uV);
}

static const struct dm_regulator_ops tps80031_smps_ops = {
	.get_value  = smps_get_value,
	.set_value  = smps_set_value,
	.get_enable = tps80031_get_enable,
	.set_enable = tps80031_set_enable,
};

U_BOOT_DRIVER(tps80031_smps) = {
	.name = TPS80031_SMPS_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps80031_smps_ops,
	.probe = tps80031_smps_probe,
	.priv_auto = sizeof(struct tps80031_smps_priv),
};
