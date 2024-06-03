// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2018 Samsung Electronics
 *  Jaehoon Chung <jh80.chung@samsung.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/s2mps11.h>

#define MODE(_id, _val, _name) { \
	.id = _id, \
	.register_value = _val, \
	.name = _name, \
}

/* BUCK : 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 */
static struct dm_regulator_mode s2mps11_buck_modes[] = {
	MODE(OP_OFF, S2MPS11_BUCK_MODE_OFF, "OFF"),
	MODE(OP_STANDBY, S2MPS11_BUCK_MODE_STANDBY, "ON/OFF"),
	MODE(OP_ON, S2MPS11_BUCK_MODE_STANDBY, "ON"),
};

static struct dm_regulator_mode s2mps11_ldo_modes[] = {
	MODE(OP_OFF, S2MPS11_LDO_MODE_OFF, "OFF"),
	MODE(OP_STANDBY, S2MPS11_LDO_MODE_STANDBY, "ON/OFF"),
	MODE(OP_STANDBY_LPM, S2MPS11_LDO_MODE_STANDBY_LPM, "ON/LPM"),
	MODE(OP_ON, S2MPS11_LDO_MODE_ON, "ON"),
};

static const char s2mps11_buck_ctrl[] = {
	0xff, 0x25, 0x27, 0x29, 0x2b, 0x2d, 0x33, 0x35, 0x37, 0x39, 0x3b
};

static const char s2mps11_buck_out[] = {
	0xff, 0x26, 0x28, 0x2a, 0x2c, 0x2f, 0x34, 0x36, 0x38, 0x3a, 0x3c
};

static int s2mps11_buck_hex2volt(int buck, int hex)
{
	unsigned int uV = 0;

	if (hex < 0)
		goto bad;

	switch (buck) {
	case 7:
	case 8:
	case 10:
		if (hex > S2MPS11_BUCK7_8_10_VOLT_MAX_HEX)
			goto bad;

		uV = hex * S2MPS11_BUCK_HSTEP + S2MPS11_BUCK_UV_HMIN;
		break;
	case 9:
		if (hex > S2MPS11_BUCK9_VOLT_MAX_HEX)
			goto bad;
		uV = hex * S2MPS11_BUCK9_STEP * 2 + S2MPS11_BUCK9_UV_MIN;
		break;
	default:
		if (buck == 5 && hex > S2MPS11_BUCK5_VOLT_MAX_HEX)
			goto bad;
		else if (buck != 5 && hex > S2MPS11_BUCK_VOLT_MAX_HEX)
			goto bad;

		uV = hex * S2MPS11_BUCK_LSTEP + S2MPS11_BUCK_UV_MIN;
		break;
	}

	return uV;
bad:
	pr_err("Value: %#x is wrong for BUCK%d", hex, buck);
	return -EINVAL;
}

static int s2mps11_buck_volt2hex(int buck, int uV)
{
	int hex;

	switch (buck) {
	case 7:
	case 8:
	case 10:
		hex = (uV - S2MPS11_BUCK_UV_HMIN) / S2MPS11_BUCK_HSTEP;
		if (hex > S2MPS11_BUCK7_8_10_VOLT_MAX_HEX)
			goto bad;

		break;
	case 9:
		hex = (uV - S2MPS11_BUCK9_UV_MIN) / S2MPS11_BUCK9_STEP;
		if (hex > S2MPS11_BUCK9_VOLT_MAX_HEX)
			goto bad;
		break;
	default:
		hex = (uV - S2MPS11_BUCK_UV_MIN) / S2MPS11_BUCK_LSTEP;
		if (buck == 5 && hex > S2MPS11_BUCK5_VOLT_MAX_HEX)
			goto bad;
		else if (buck != 5 && hex > S2MPS11_BUCK_VOLT_MAX_HEX)
			goto bad;
		break;
	};

	if (hex >= 0)
		return hex;

bad:
	pr_err("Value: %d uV is wrong for BUCK%d", uV, buck);
	return -EINVAL;
}

static int s2mps11_buck_val(struct udevice *dev, int op, int *uV)
{
	int hex, buck, ret;
	u32 mask, addr;
	u8 val;

	buck = dev->driver_data;
	if (buck < 1 || buck > S2MPS11_BUCK_NUM) {
		pr_err("Wrong buck number: %d\n", buck);
		return -EINVAL;
	}

	if (op == PMIC_OP_GET)
		*uV = 0;

	addr = s2mps11_buck_out[buck];

	switch (buck) {
	case 9:
		mask = S2MPS11_BUCK9_VOLT_MASK;
		break;
	default:
		mask = S2MPS11_BUCK_VOLT_MASK;
		break;
	}

	ret = pmic_read(dev->parent, addr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		val &= mask;
		ret = s2mps11_buck_hex2volt(buck, val);
		if (ret < 0)
			return ret;
		*uV = ret;
		return 0;
	}

	hex = s2mps11_buck_volt2hex(buck, *uV);
	if (hex < 0)
		return hex;

	val &= ~mask;
	val |= hex;
	ret = pmic_write(dev->parent, addr, &val, 1);

	return ret;
}

static int s2mps11_buck_mode(struct udevice *dev, int op, int *opmode)
{
	unsigned int addr, mode;
	unsigned char val;
	int buck, ret;

	buck = dev->driver_data;
	if (buck < 1 || buck > S2MPS11_BUCK_NUM) {
		pr_err("Wrong buck number: %d\n", buck);
		return -EINVAL;
	}

	addr = s2mps11_buck_ctrl[buck];

	ret = pmic_read(dev->parent, addr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		val &= (S2MPS11_BUCK_MODE_MASK << S2MPS11_BUCK_MODE_SHIFT);
		switch (val) {
		case S2MPS11_BUCK_MODE_OFF:
			*opmode = OP_OFF;
			break;
		case S2MPS11_BUCK_MODE_STANDBY:
			*opmode = OP_STANDBY;
			break;
		case S2MPS11_BUCK_MODE_ON:
			*opmode = OP_ON;
			break;
		default:
			return -EINVAL;
		}
		return 0;
	}

	switch (*opmode) {
	case OP_OFF:
		mode = S2MPS11_BUCK_MODE_OFF;
		break;
	case OP_STANDBY:
		mode = S2MPS11_BUCK_MODE_STANDBY;
		break;
	case OP_ON:
		mode = S2MPS11_BUCK_MODE_ON;
		break;
	default:
		pr_err("Wrong mode: %d for buck: %d\n", *opmode, buck);
		return -EINVAL;
	}

	val &= ~(S2MPS11_BUCK_MODE_MASK << S2MPS11_BUCK_MODE_SHIFT);
	val |= mode;
	ret = pmic_write(dev->parent, addr, &val, 1);

	return ret;
}

static int s2mps11_buck_enable(struct udevice *dev, int op, bool *enable)
{
	int ret, on_off;

	if (op == PMIC_OP_GET) {
		ret = s2mps11_buck_mode(dev, op, &on_off);
		if (ret)
			return ret;
		switch (on_off) {
		case OP_OFF:
			*enable = false;
			break;
		case OP_ON:
			*enable = true;
			break;
		default:
			return -EINVAL;
		}
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			on_off = OP_ON;
		else
			on_off = OP_OFF;

		ret = s2mps11_buck_mode(dev, op, &on_off);
		if (ret)
			return ret;
	}

	return 0;
}

static int buck_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = s2mps11_buck_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;
	return uV;
}

static int buck_set_value(struct udevice *dev, int uV)
{
	return s2mps11_buck_val(dev, PMIC_OP_SET, &uV);
}

static int buck_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = s2mps11_buck_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;
	return enable;
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	return s2mps11_buck_enable(dev, PMIC_OP_SET, &enable);
}

static int buck_get_mode(struct udevice *dev)
{
	int mode;
	int ret;

	ret = s2mps11_buck_mode(dev, PMIC_OP_GET, &mode);
	if (ret)
		return ret;

	return mode;
}

static int buck_set_mode(struct udevice *dev, int mode)
{
	return s2mps11_buck_mode(dev, PMIC_OP_SET, &mode);
}

static int s2mps11_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode = s2mps11_buck_modes;
	uc_pdata->mode_count = ARRAY_SIZE(s2mps11_buck_modes);

	return 0;
}

static const struct dm_regulator_ops s2mps11_buck_ops = {
	.get_value	= buck_get_value,
	.set_value	= buck_set_value,
	.get_enable	= buck_get_enable,
	.set_enable	= buck_set_enable,
	.get_mode	= buck_get_mode,
	.set_mode	= buck_set_mode,
};

U_BOOT_DRIVER(s2mps11_buck) = {
	.name = S2MPS11_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &s2mps11_buck_ops,
	.probe = s2mps11_buck_probe,
};

static int s2mps11_ldo_hex2volt(int ldo, int hex)
{
	unsigned int uV = 0;

	if (hex > S2MPS11_LDO_VOLT_MAX_HEX) {
		pr_err("Value: %#x is wrong for LDO%d", hex, ldo);
		return -EINVAL;
	}

	switch (ldo) {
	case 1:
	case 6:
	case 11:
	case 22:
	case 23:
	case 27:
	case 35:
		uV = hex * S2MPS11_LDO_STEP + S2MPS11_LDO_UV_MIN;
		break;
	default:
		uV = hex * S2MPS11_LDO_STEP * 2 + S2MPS11_LDO_UV_MIN;
		break;
	}

	return uV;
}

static int s2mps11_ldo_volt2hex(int ldo, int uV)
{
	int hex = 0;

	switch (ldo) {
	case 1:
	case 6:
	case 11:
	case 22:
	case 23:
	case 27:
	case 35:
		hex = (uV - S2MPS11_LDO_UV_MIN) / S2MPS11_LDO_STEP;
		break;
	default:
		hex = (uV - S2MPS11_LDO_UV_MIN) / (S2MPS11_LDO_STEP * 2);
		break;
	}

	if (hex >= 0 && hex <= S2MPS11_LDO_VOLT_MAX_HEX)
		return hex;

	pr_err("Value: %d uV is wrong for LDO%d", uV, ldo);
	return -EINVAL;

	return 0;
}

static int s2mps11_ldo_val(struct udevice *dev, int op, int *uV)
{
	unsigned int addr;
	unsigned char val;
	int hex, ldo, ret;

	ldo = dev->driver_data;
	if (ldo < 1 || ldo > S2MPS11_LDO_NUM) {
		pr_err("Wrong ldo number: %d\n", ldo);
		return -EINVAL;
	}

	addr = S2MPS11_REG_L1CTRL + ldo - 1;

	ret = pmic_read(dev->parent, addr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		*uV = 0;
		val &= S2MPS11_LDO_VOLT_MASK;
		ret = s2mps11_ldo_hex2volt(ldo, val);
		if (ret < 0)
			return ret;

		*uV = ret;
		return 0;
	}

	hex = s2mps11_ldo_volt2hex(ldo, *uV);
	if (hex < 0)
		return hex;

	val &= ~S2MPS11_LDO_VOLT_MASK;
	val |= hex;
	ret = pmic_write(dev->parent, addr, &val, 1);

	return ret;
}

static int s2mps11_ldo_mode(struct udevice *dev, int op, int *opmode)
{
	unsigned int addr, mode;
	unsigned char val;
	int ldo, ret;

	ldo = dev->driver_data;
	if (ldo < 1 || ldo > S2MPS11_LDO_NUM) {
		pr_err("Wrong ldo number: %d\n", ldo);
		return -EINVAL;
	}
	addr = S2MPS11_REG_L1CTRL + ldo - 1;

	ret = pmic_read(dev->parent, addr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		val &= (S2MPS11_LDO_MODE_MASK << S2MPS11_LDO_MODE_SHIFT);
		switch (val) {
		case S2MPS11_LDO_MODE_OFF:
			*opmode = OP_OFF;
			break;
		case S2MPS11_LDO_MODE_STANDBY:
			*opmode = OP_STANDBY;
			break;
		case S2MPS11_LDO_MODE_STANDBY_LPM:
			*opmode = OP_STANDBY_LPM;
			break;
		case S2MPS11_LDO_MODE_ON:
			*opmode = OP_ON;
			break;
		default:
			return -EINVAL;
		}
		return 0;
	}

	switch (*opmode) {
	case OP_OFF:
		mode = S2MPS11_LDO_MODE_OFF;
		break;
	case OP_STANDBY:
		mode = S2MPS11_LDO_MODE_STANDBY;
		break;
	case OP_STANDBY_LPM:
		mode = S2MPS11_LDO_MODE_STANDBY_LPM;
		break;
	case OP_ON:
		mode = S2MPS11_LDO_MODE_ON;
		break;
	default:
		pr_err("Wrong mode: %d for ldo: %d\n", *opmode, ldo);
		return -EINVAL;
	}

	val &= ~(S2MPS11_LDO_MODE_MASK << S2MPS11_LDO_MODE_SHIFT);
	val |= mode;
	ret = pmic_write(dev->parent, addr, &val, 1);

	return ret;
}

static int s2mps11_ldo_enable(struct udevice *dev, int op, bool *enable)
{
	int ret, on_off;

	if (op == PMIC_OP_GET) {
		ret = s2mps11_ldo_mode(dev, op, &on_off);
		if (ret)
			return ret;
		switch (on_off) {
		case OP_OFF:
			*enable = false;
			break;
		case OP_ON:
			*enable = true;
			break;
		default:
			return -EINVAL;
		}
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			on_off = OP_ON;
		else
			on_off = OP_OFF;

		ret = s2mps11_ldo_mode(dev, op, &on_off);
		if (ret)
			return ret;
	}

	return 0;
}

static int ldo_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = s2mps11_ldo_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int ldo_set_value(struct udevice *dev, int uV)
{
	return s2mps11_ldo_val(dev, PMIC_OP_SET, &uV);
}

static int ldo_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = s2mps11_ldo_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;
	return enable;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	int ret;

	ret = s2mps11_ldo_enable(dev, PMIC_OP_SET, &enable);
	if (ret)
		return ret;

	/* Wait the "enable delay" for voltage to start to rise */
	udelay(15);

	return 0;
}

static int ldo_get_mode(struct udevice *dev)
{
	int mode, ret;

	ret = s2mps11_ldo_mode(dev, PMIC_OP_GET, &mode);
	if (ret)
		return ret;
	return mode;
}

static int ldo_set_mode(struct udevice *dev, int mode)
{
	return s2mps11_ldo_mode(dev, PMIC_OP_SET, &mode);
}

static int s2mps11_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);
	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->mode = s2mps11_ldo_modes;
	uc_pdata->mode_count = ARRAY_SIZE(s2mps11_ldo_modes);

	return 0;
}

static const struct dm_regulator_ops s2mps11_ldo_ops = {
	.get_value	= ldo_get_value,
	.set_value	= ldo_set_value,
	.get_enable	= ldo_get_enable,
	.set_enable	= ldo_set_enable,
	.get_mode	= ldo_get_mode,
	.set_mode	= ldo_set_mode,
};

U_BOOT_DRIVER(s2mps11_ldo) = {
	.name = S2MPS11_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &s2mps11_ldo_ops,
	.probe = s2mps11_ldo_probe,
};
