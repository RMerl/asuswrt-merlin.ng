// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012-2015 Samsung Electronics
 *
 *  Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *  Przemyslaw Marczak <p.marczak@samsung.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/max77686_pmic.h>

#define MODE(_id, _val, _name) { \
	.id = _id, \
	.register_value = _val, \
	.name = _name, \
}

/* LDO: 1,3,4,5,9,17,18,19,20,21,22,23,24,26,26,27 */
static struct dm_regulator_mode max77686_ldo_mode_standby1[] = {
	MODE(OPMODE_OFF, MAX77686_LDO_MODE_OFF, "OFF"),
	MODE(OPMODE_LPM, MAX77686_LDO_MODE_LPM, "LPM"),
	MODE(OPMODE_STANDBY_LPM, MAX77686_LDO_MODE_STANDBY_LPM, "ON/LPM"),
	MODE(OPMODE_ON, MAX77686_LDO_MODE_ON, "ON"),
};

/* LDO: 2,6,7,8,10,11,12,14,15,16 */
static struct dm_regulator_mode max77686_ldo_mode_standby2[] = {
	MODE(OPMODE_OFF, MAX77686_LDO_MODE_OFF, "OFF"),
	MODE(OPMODE_STANDBY, MAX77686_LDO_MODE_STANDBY, "ON/OFF"),
	MODE(OPMODE_STANDBY_LPM, MAX77686_LDO_MODE_STANDBY_LPM, "ON/LPM"),
	MODE(OPMODE_ON, MAX77686_LDO_MODE_ON, "ON"),
};

/* Buck: 1 */
static struct dm_regulator_mode max77686_buck_mode_standby[] = {
	MODE(OPMODE_OFF, MAX77686_BUCK_MODE_OFF, "OFF"),
	MODE(OPMODE_STANDBY, MAX77686_BUCK_MODE_STANDBY, "ON/OFF"),
	MODE(OPMODE_ON, MAX77686_BUCK_MODE_ON, "ON"),
};

/* Buck: 2,3,4 */
static struct dm_regulator_mode max77686_buck_mode_lpm[] = {
	MODE(OPMODE_OFF, MAX77686_BUCK_MODE_OFF, "OFF"),
	MODE(OPMODE_STANDBY, MAX77686_BUCK_MODE_STANDBY, "ON/OFF"),
	MODE(OPMODE_LPM, MAX77686_BUCK_MODE_LPM, "LPM"),
	MODE(OPMODE_ON, MAX77686_BUCK_MODE_ON, "ON"),
};

/* Buck: 5,6,7,8,9 */
static struct dm_regulator_mode max77686_buck_mode_onoff[] = {
	MODE(OPMODE_OFF, MAX77686_BUCK_MODE_OFF, "OFF"),
	MODE(OPMODE_ON, MAX77686_BUCK_MODE_ON, "ON"),
};

static const char max77686_buck_ctrl[] = {
	0xff, 0x10, 0x12, 0x1c, 0x26, 0x30, 0x32, 0x34, 0x36, 0x38
};

static const char max77686_buck_out[] = {
	0xff, 0x11, 0x14, 0x1e, 0x28, 0x31, 0x33, 0x35, 0x37, 0x39
};

static int max77686_buck_volt2hex(int buck, int uV)
{
	int hex = 0;
	int hex_max = 0;

	switch (buck) {
	case 2:
	case 3:
	case 4:
		/* hex = (uV - 600000) / 12500; */
		hex = (uV - MAX77686_BUCK_UV_LMIN) / MAX77686_BUCK_UV_LSTEP;
		hex_max = MAX77686_BUCK234_VOLT_MAX_HEX;
		break;
	default:
		/*
		 * hex = (uV - 750000) / 50000. We assume that dynamic voltage
		 * scaling via GPIOs is not enabled and don't support that.
		 * If this is enabled then the driver will need to take that
		 * into account and check different registers depending on
		 * the current setting. See the datasheet for details.
		 */
		hex = (uV - MAX77686_BUCK_UV_HMIN) / MAX77686_BUCK_UV_HSTEP;
		hex_max = MAX77686_BUCK_VOLT_MAX_HEX;
		break;
	}

	if (hex >= 0 && hex <= hex_max)
		return hex;

	pr_err("Value: %d uV is wrong for BUCK%d", uV, buck);
	return -EINVAL;
}

static int max77686_buck_hex2volt(int buck, int hex)
{
	unsigned uV = 0;
	int hex_max = 0;

	if (hex < 0)
		goto bad_hex;

	switch (buck) {
	case 2:
	case 3:
	case 4:
		hex_max = MAX77686_BUCK234_VOLT_MAX_HEX;
		if (hex > hex_max)
			goto bad_hex;

		/* uV = hex * 12500 + 600000; */
		uV = hex * MAX77686_BUCK_UV_LSTEP + MAX77686_BUCK_UV_LMIN;
		break;
	default:
		hex_max = MAX77686_BUCK_VOLT_MAX_HEX;
		if (hex > hex_max)
			goto bad_hex;

		/* uV = hex * 50000 + 750000; */
		uV = hex * MAX77686_BUCK_UV_HSTEP + MAX77686_BUCK_UV_HMIN;
		break;
	}

	return uV;

bad_hex:
	pr_err("Value: %#x is wrong for BUCK%d", hex, buck);
	return -EINVAL;
}

static int max77686_ldo_volt2hex(int ldo, int uV)
{
	int hex = 0;

	switch (ldo) {
	case 1:
	case 2:
	case 6:
	case 7:
	case 8:
	case 15:
		hex = (uV - MAX77686_LDO_UV_MIN) / MAX77686_LDO_UV_LSTEP;
		/* hex = (uV - 800000) / 25000; */
		break;
	default:
		hex = (uV - MAX77686_LDO_UV_MIN) / MAX77686_LDO_UV_HSTEP;
		/* hex = (uV - 800000) / 50000; */
	}

	if (hex >= 0 && hex <= MAX77686_LDO_VOLT_MAX_HEX)
		return hex;

	pr_err("Value: %d uV is wrong for LDO%d", uV, ldo);
	return -EINVAL;
}

static int max77686_ldo_hex2volt(int ldo, int hex)
{
	unsigned int uV = 0;

	if (hex > MAX77686_LDO_VOLT_MAX_HEX)
		goto bad_hex;

	switch (ldo) {
	case 1:
	case 2:
	case 6:
	case 7:
	case 8:
	case 15:
		/* uV = hex * 25000 + 800000; */
		uV = hex * MAX77686_LDO_UV_LSTEP + MAX77686_LDO_UV_MIN;
		break;
	default:
		/* uV = hex * 50000 + 800000; */
		uV = hex * MAX77686_LDO_UV_HSTEP + MAX77686_LDO_UV_MIN;
	}

	return uV;

bad_hex:
	pr_err("Value: %#x is wrong for ldo%d", hex, ldo);
	return -EINVAL;
}

static int max77686_ldo_hex2mode(int ldo, int hex)
{
	if (hex > MAX77686_LDO_MODE_MASK)
		return -EINVAL;

	switch (hex) {
	case MAX77686_LDO_MODE_OFF:
		return OPMODE_OFF;
	case MAX77686_LDO_MODE_LPM: /* == MAX77686_LDO_MODE_STANDBY: */
		/* The same mode values but different meaning for each ldo */
		switch (ldo) {
		case 2:
		case 6:
		case 7:
		case 8:
		case 10:
		case 11:
		case 12:
		case 14:
		case 15:
		case 16:
			return OPMODE_STANDBY;
		default:
			return OPMODE_LPM;
		}
	case MAX77686_LDO_MODE_STANDBY_LPM:
		return OPMODE_STANDBY_LPM;
	case MAX77686_LDO_MODE_ON:
		return OPMODE_ON;
	default:
		return -EINVAL;
	}
}

static int max77686_buck_hex2mode(int buck, int hex)
{
	if (hex > MAX77686_BUCK_MODE_MASK)
		return -EINVAL;

	switch (hex) {
	case MAX77686_BUCK_MODE_OFF:
		return OPMODE_OFF;
	case MAX77686_BUCK_MODE_ON:
		return OPMODE_ON;
	case MAX77686_BUCK_MODE_STANDBY:
		switch (buck) {
		case 1:
		case 2:
		case 3:
		case 4:
			return OPMODE_STANDBY;
		default:
			return -EINVAL;
		}
	case MAX77686_BUCK_MODE_LPM:
		switch (buck) {
		case 2:
		case 3:
		case 4:
			return OPMODE_LPM;
		default:
			return -EINVAL;
		}
	default:
		return -EINVAL;
	}
}

static int max77686_buck_modes(int buck, struct dm_regulator_mode **modesp)
{
	int ret = -EINVAL;

	if (buck < 1 || buck > MAX77686_BUCK_NUM)
		return ret;

	switch (buck) {
	case 1:
		*modesp = max77686_buck_mode_standby;
		ret = ARRAY_SIZE(max77686_buck_mode_standby);
		break;
	case 2:
	case 3:
	case 4:
		*modesp = max77686_buck_mode_lpm;
		ret = ARRAY_SIZE(max77686_buck_mode_lpm);
		break;
	default:
		*modesp = max77686_buck_mode_onoff;
		ret = ARRAY_SIZE(max77686_buck_mode_onoff);
	}

	return ret;
}

static int max77686_ldo_modes(int ldo, struct dm_regulator_mode **modesp,
				struct udevice *dev)
{
	int ret = -EINVAL;

	if (ldo < 1 || ldo > MAX77686_LDO_NUM)
		return ret;

	switch (ldo) {
	case 2:
	case 6:
	case 7:
	case 8:
	case 10:
	case 11:
	case 12:
	case 14:
	case 15:
	case 16:
		*modesp = max77686_ldo_mode_standby2;
		ret = ARRAY_SIZE(max77686_ldo_mode_standby2);
		break;
	default:
		*modesp = max77686_ldo_mode_standby1;
		ret = ARRAY_SIZE(max77686_ldo_mode_standby1);
	}

	return ret;
}

static int max77686_ldo_val(struct udevice *dev, int op, int *uV)
{
	unsigned int adr;
	unsigned char val;
	int hex, ldo, ret;

	if (op == PMIC_OP_GET)
		*uV = 0;

	ldo = dev->driver_data;
	if (ldo < 1 || ldo > MAX77686_LDO_NUM) {
		pr_err("Wrong ldo number: %d", ldo);
		return -EINVAL;
	}

	adr = MAX77686_REG_PMIC_LDO1CTRL1 + ldo - 1;

	ret = pmic_read(dev->parent, adr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		val &= MAX77686_LDO_VOLT_MASK;
		ret = max77686_ldo_hex2volt(ldo, val);
		if (ret < 0)
			return ret;
		*uV = ret;
		return 0;
	}

	hex = max77686_ldo_volt2hex(ldo, *uV);
	if (hex < 0)
		return hex;

	val &= ~MAX77686_LDO_VOLT_MASK;
	val |= hex;
	ret = pmic_write(dev->parent, adr, &val, 1);

	return ret;
}

static int max77686_buck_val(struct udevice *dev, int op, int *uV)
{
	unsigned int mask, adr;
	unsigned char val;
	int hex, buck, ret;

	buck = dev->driver_data;
	if (buck < 1 || buck > MAX77686_BUCK_NUM) {
		pr_err("Wrong buck number: %d", buck);
		return -EINVAL;
	}

	if (op == PMIC_OP_GET)
		*uV = 0;

	/* &buck_out = ctrl + 1 */
	adr = max77686_buck_out[buck];

	/* mask */
	switch (buck) {
	case 2:
	case 3:
	case 4:
		mask = MAX77686_BUCK234_VOLT_MASK;
		break;
	default:
		mask = MAX77686_BUCK_VOLT_MASK;
		break;
	}

	ret = pmic_read(dev->parent, adr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		val &= mask;
		ret = max77686_buck_hex2volt(buck, val);
		if (ret < 0)
			return ret;
		*uV = ret;
		return 0;
	}

	hex = max77686_buck_volt2hex(buck, *uV);
	if (hex < 0)
		return hex;

	val &= ~mask;
	val |= hex;
	ret = pmic_write(dev->parent, adr, &val, 1);

	return ret;
}

static int max77686_ldo_mode(struct udevice *dev, int op, int *opmode)
{
	unsigned int adr, mode;
	unsigned char val;
	int ldo, ret;

	if (op == PMIC_OP_GET)
		*opmode = -EINVAL;

	ldo = dev->driver_data;
	if (ldo < 1 || ldo > MAX77686_LDO_NUM) {
		pr_err("Wrong ldo number: %d", ldo);
		return -EINVAL;
	}

	adr = MAX77686_REG_PMIC_LDO1CTRL1 + ldo - 1;

	ret = pmic_read(dev->parent, adr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		val &= MAX77686_LDO_MODE_MASK;
		ret = max77686_ldo_hex2mode(ldo, val);
		if (ret < 0)
			return ret;
		*opmode = ret;
		return 0;
	}

	/* mode */
	switch (*opmode) {
	case OPMODE_OFF:
		mode = MAX77686_LDO_MODE_OFF;
		break;
	case OPMODE_LPM:
		switch (ldo) {
		case 2:
		case 6:
		case 7:
		case 8:
		case 10:
		case 11:
		case 12:
		case 14:
		case 15:
		case 16:
			return -EINVAL;
		default:
			mode = MAX77686_LDO_MODE_LPM;
		}
		break;
	case OPMODE_STANDBY:
		switch (ldo) {
		case 2:
		case 6:
		case 7:
		case 8:
		case 10:
		case 11:
		case 12:
		case 14:
		case 15:
		case 16:
			mode = MAX77686_LDO_MODE_STANDBY;
			break;
		default:
			return -EINVAL;
		}
		break;
	case OPMODE_STANDBY_LPM:
		mode = MAX77686_LDO_MODE_STANDBY_LPM;
		break;
	case OPMODE_ON:
		mode = MAX77686_LDO_MODE_ON;
		break;
	default:
		mode = 0xff;
	}

	if (mode == 0xff) {
		pr_err("Wrong mode: %d for ldo%d", *opmode, ldo);
		return -EINVAL;
	}

	val &= ~MAX77686_LDO_MODE_MASK;
	val |= mode;
	ret = pmic_write(dev->parent, adr, &val, 1);

	return ret;
}

static int max77686_ldo_enable(struct udevice *dev, int op, bool *enable)
{
	int ret, on_off;

	if (op == PMIC_OP_GET) {
		ret = max77686_ldo_mode(dev, op, &on_off);
		if (ret)
			return ret;

		switch (on_off) {
		case OPMODE_OFF:
			*enable = false;
			break;
		case OPMODE_ON:
			*enable = true;
			break;
		default:
			return -EINVAL;
		}
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			on_off = OPMODE_ON;
		else
			on_off = OPMODE_OFF;

		ret = max77686_ldo_mode(dev, op, &on_off);
		if (ret)
			return ret;
	}

	return 0;
}

static int max77686_buck_mode(struct udevice *dev, int op, int *opmode)
{
	unsigned int mask, adr, mode, mode_shift;
	unsigned char val;
	int buck, ret;

	buck = dev->driver_data;
	if (buck < 1 || buck > MAX77686_BUCK_NUM) {
		pr_err("Wrong buck number: %d", buck);
		return -EINVAL;
	}

	adr = max77686_buck_ctrl[buck];

	/* mask */
	switch (buck) {
	case 2:
	case 3:
	case 4:
		mode_shift = MAX77686_BUCK_MODE_SHIFT_2;
		break;
	default:
		mode_shift = MAX77686_BUCK_MODE_SHIFT_1;
	}

	mask = MAX77686_BUCK_MODE_MASK << mode_shift;

	ret = pmic_read(dev->parent, adr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		val &= mask;
		val >>= mode_shift;
		ret = max77686_buck_hex2mode(buck, val);
		if (ret < 0)
			return ret;
		*opmode = ret;
		return 0;
	}

	/* mode */
	switch (*opmode) {
	case OPMODE_OFF:
		mode = MAX77686_BUCK_MODE_OFF;
		break;
	case OPMODE_STANDBY:
		switch (buck) {
		case 1:
		case 2:
		case 3:
		case 4:
			mode = MAX77686_BUCK_MODE_STANDBY << mode_shift;
			break;
		default:
			mode = 0xff;
		}
		break;
	case OPMODE_LPM:
		switch (buck) {
		case 2:
		case 3:
		case 4:
			mode = MAX77686_BUCK_MODE_LPM << mode_shift;
			break;
		default:
			mode = 0xff;
		}
		break;
	case OPMODE_ON:
		mode = MAX77686_BUCK_MODE_ON << mode_shift;
		break;
	default:
		mode = 0xff;
	}

	if (mode == 0xff) {
		pr_err("Wrong mode: %d for buck: %d\n", *opmode, buck);
		return -EINVAL;
	}

	val &= ~mask;
	val |= mode;
	ret = pmic_write(dev->parent, adr, &val, 1);

	return ret;
}

static int max77686_buck_enable(struct udevice *dev, int op, bool *enable)
{
	int ret, on_off;

	if (op == PMIC_OP_GET) {
		ret = max77686_buck_mode(dev, op, &on_off);
		if (ret)
			return ret;

		switch (on_off) {
		case OPMODE_OFF:
			*enable = false;
			break;
		case OPMODE_ON:
			*enable = true;
			break;
		default:
			return -EINVAL;
		}
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			on_off = OPMODE_ON;
		else
			on_off = OPMODE_OFF;

		ret = max77686_buck_mode(dev, op, &on_off);
		if (ret)
			return ret;
	}

	return 0;
}

static int max77686_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->mode_count = max77686_ldo_modes(dev->driver_data,
						  &uc_pdata->mode, dev);

	return 0;
}

static int ldo_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = max77686_ldo_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int ldo_set_value(struct udevice *dev, int uV)
{
	return max77686_ldo_val(dev, PMIC_OP_SET, &uV);
}

static int ldo_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = max77686_ldo_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	return max77686_ldo_enable(dev, PMIC_OP_SET, &enable);
}

static int ldo_get_mode(struct udevice *dev)
{
	int mode;
	int ret;

	ret = max77686_ldo_mode(dev, PMIC_OP_GET, &mode);
	if (ret)
		return ret;

	return mode;
}

static int ldo_set_mode(struct udevice *dev, int mode)
{
	return max77686_ldo_mode(dev, PMIC_OP_SET, &mode);
}

static int max77686_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode_count = max77686_buck_modes(dev->driver_data,
						   &uc_pdata->mode);

	return 0;
}

static int buck_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = max77686_buck_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int buck_set_value(struct udevice *dev, int uV)
{
	return max77686_buck_val(dev, PMIC_OP_SET, &uV);
}

static int buck_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = max77686_buck_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	return max77686_buck_enable(dev, PMIC_OP_SET, &enable);
}

static int buck_get_mode(struct udevice *dev)
{
	int mode;
	int ret;

	ret = max77686_buck_mode(dev, PMIC_OP_GET, &mode);
	if (ret)
		return ret;

	return mode;
}

static int buck_set_mode(struct udevice *dev, int mode)
{
	return max77686_buck_mode(dev, PMIC_OP_SET, &mode);
}

static const struct dm_regulator_ops max77686_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = ldo_get_enable,
	.set_enable = ldo_set_enable,
	.get_mode   = ldo_get_mode,
	.set_mode   = ldo_set_mode,
};

U_BOOT_DRIVER(max77686_ldo) = {
	.name = MAX77686_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &max77686_ldo_ops,
	.probe = max77686_ldo_probe,
};

static const struct dm_regulator_ops max77686_buck_ops = {
	.get_value  = buck_get_value,
	.set_value  = buck_set_value,
	.get_enable = buck_get_enable,
	.set_enable = buck_set_enable,
	.get_mode   = buck_get_mode,
	.set_mode   = buck_set_mode,
};

U_BOOT_DRIVER(max77686_buck) = {
	.name = MAX77686_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &max77686_buck_ops,
	.probe = max77686_buck_probe,
};
