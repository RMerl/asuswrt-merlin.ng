// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/max77663.h>

/* fist row is control registers, second is voltage registers */
static const char max77663_sd_reg[][MAX77663_SD_NUM] = {
	{ 0x1d, 0x1e, 0x1f, 0x20, 0x21 },
	{ 0x16, 0x17, 0x18, 0x19, 0x2a },
};

static const char max77663_ldo_reg[MAX77663_LDO_NUM] = {
	0x23, 0x25, 0x27, 0x29, 0x2b, 0x2d, 0x2f, 0x31, 0x33
};

static int max77663_sd_enable(struct udevice *dev, int op, bool *enable)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	u32 adr = uc_pdata->ctrl_reg;
	int val, ret;

	val = pmic_reg_read(dev->parent, adr);
	if (val < 0)
		return val;

	if (op == PMIC_OP_GET) {
		if (val & SD_STATUS_MASK)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		val &= ~SD_STATUS_MASK;

		if (*enable)
			val |= SD_STATUS_MASK;

		ret = pmic_reg_write(dev->parent, adr, val);
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * max77663_*_volt2hex() - convert voltage in uV into
 *			   applicable to register hex value
 *
 * @idx:	regulator index
 * @uV:		voltage in uV
 *
 * Return: voltage in hex on success, -ve on failure
 */
static int max77663_sd_volt2hex(int idx, int uV)
{
	switch (idx) {
	case 0:
		/* SD0 has max voltage 1.4V */
		if (uV > SD0_VOLT_MAX)
			return -EINVAL;
		break;
	case 1:
		/* SD1 has max voltage 1.55V */
		if (uV > SD1_VOLT_MAX)
			return -EINVAL;
		break;
	default:
		/* SD2 and SD3 have max voltage 3.79V */
		if (uV > SD_VOLT_MAX)
			return -EINVAL;
		break;
	};

	if (uV < SD_VOLT_MIN)
		uV = SD_VOLT_MIN;

	return (uV - SD_VOLT_BASE) / 12500;
}

/**
 * max77663_*_hex2volt() - convert register hex value into
 *			   actual voltage in uV
 *
 * @idx:	regulator index
 * @hex:	hex value of register
 *
 * Return: voltage in uV on success, -ve on failure
 */
static int max77663_sd_hex2volt(int idx, int hex)
{
	switch (idx) {
	case 0:
		/* SD0 has max voltage 1.4V */
		if (hex > SD0_VOLT_MAX_HEX)
			return -EINVAL;
		break;
	case 1:
		/* SD1 has max voltage 1.55V */
		if (hex > SD1_VOLT_MAX_HEX)
			return -EINVAL;
		break;
	default:
		/* SD2 and SD3 have max voltage 3.79V */
		if (hex > SD_VOLT_MAX_HEX)
			return -EINVAL;
		break;
	};

	if (hex < SD_VOLT_MIN_HEX)
		hex = SD_VOLT_MIN_HEX;

	return SD_VOLT_BASE + hex * 12500;
}

static int max77663_sd_val(struct udevice *dev, int op, int *uV)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	u32 adr = uc_pdata->volt_reg;
	int idx = dev->driver_data;
	int hex, ret;

	if (op == PMIC_OP_GET) {
		hex = pmic_reg_read(dev->parent, adr);
		if (hex < 0)
			return hex;

		*uV = 0;

		ret = max77663_sd_hex2volt(idx, hex);
		if (ret < 0)
			return ret;
		*uV = ret;

		return 0;
	}

	/* SD regulators use entire register for voltage */
	hex = max77663_sd_volt2hex(idx, *uV);
	if (hex < 0)
		return hex;

	return pmic_reg_write(dev->parent, adr, hex);
}

static int max77663_sd_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	int idx = dev->driver_data;

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->ctrl_reg = max77663_sd_reg[0][idx];
	uc_pdata->volt_reg = max77663_sd_reg[1][idx];

	return 0;
}

static int sd_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = max77663_sd_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int sd_set_value(struct udevice *dev, int uV)
{
	return max77663_sd_val(dev, PMIC_OP_SET, &uV);
}

static int sd_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = max77663_sd_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int sd_set_enable(struct udevice *dev, bool enable)
{
	return max77663_sd_enable(dev, PMIC_OP_SET, &enable);
}

static const struct dm_regulator_ops max77663_sd_ops = {
	.get_value  = sd_get_value,
	.set_value  = sd_set_value,
	.get_enable = sd_get_enable,
	.set_enable = sd_set_enable,
};

U_BOOT_DRIVER(max77663_sd) = {
	.name = MAX77663_SD_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &max77663_sd_ops,
	.probe = max77663_sd_probe,
};

static int max77663_ldo_enable(struct udevice *dev, int op, bool *enable)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	u32 adr = uc_pdata->ctrl_reg;
	int val, ret;

	val = pmic_reg_read(dev->parent, adr);
	if (val < 0)
		return val;

	if (op == PMIC_OP_GET) {
		if (val & LDO_STATUS_MASK)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		val &= ~LDO_STATUS_MASK;

		if (*enable)
			val |= LDO_STATUS_MASK;

		ret = pmic_reg_write(dev->parent, adr, val);
		if (ret)
			return ret;
	}

	return 0;
}

static int max77663_ldo_volt2hex(int idx, int uV)
{
	switch (idx) {
	case 0:
	case 1:
		if (uV > LDO01_VOLT_MAX)
			return -EINVAL;

		return (uV - LDO_VOLT_BASE) / 25000;
	case 4:
		if (uV > LDO4_VOLT_MAX)
			return -EINVAL;

		return (uV - LDO_VOLT_BASE) / 12500;
	default:
		if (uV > LDO_VOLT_MAX)
			return -EINVAL;

		return (uV - LDO_VOLT_BASE) / 50000;
	};
}

static int max77663_ldo_hex2volt(int idx, int hex)
{
	if (hex > LDO_VOLT_MAX_HEX)
		return -EINVAL;

	switch (idx) {
	case 0:
	case 1:
		return (hex * 25000) + LDO_VOLT_BASE;
	case 4:
		return (hex * 12500) + LDO_VOLT_BASE;
	default:
		return (hex * 50000) + LDO_VOLT_BASE;
	};
}

static int max77663_ldo_val(struct udevice *dev, int op, int *uV)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	u32 adr = uc_pdata->ctrl_reg;
	int idx = dev->driver_data;
	int hex, val, ret;

	val = pmic_reg_read(dev->parent, adr);
	if (val < 0)
		return val;

	if (op == PMIC_OP_GET) {
		*uV = 0;

		ret = max77663_ldo_hex2volt(idx, val & LDO_VOLT_MASK);
		if (ret < 0)
			return ret;

		*uV = ret;
		return 0;
	}

	hex = max77663_ldo_volt2hex(idx, *uV);
	if (hex < 0)
		return hex;

	val &= ~LDO_VOLT_MASK;

	return pmic_reg_write(dev->parent, adr, val | hex);
}

static int max77663_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	int idx = dev->driver_data;

	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->ctrl_reg = max77663_ldo_reg[idx];

	return 0;
}

static int ldo_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = max77663_ldo_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int ldo_set_value(struct udevice *dev, int uV)
{
	return max77663_ldo_val(dev, PMIC_OP_SET, &uV);
}

static int ldo_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = max77663_ldo_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	return max77663_ldo_enable(dev, PMIC_OP_SET, &enable);
}

static const struct dm_regulator_ops max77663_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = ldo_get_enable,
	.set_enable = ldo_set_enable,
};

U_BOOT_DRIVER(max77663_ldo) = {
	.name = MAX77663_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &max77663_ldo_ops,
	.probe = max77663_ldo_probe,
};
