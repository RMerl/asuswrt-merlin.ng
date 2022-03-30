// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2015 Samsung Electronics
 *  Przemyslaw Marczak  <p.marczak@samsung.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/sandbox_pmic.h>

#define MODE(_id, _val, _name) [_id] = {  \
	.id = _id,                \
	.register_value = _val,   \
	.name = _name,            \
}

#define RANGE(_min, _max, _step) { \
	.min = _min,               \
	.max = _max,               \
	.step = _step,             \
}

/*
 * struct output_range - helper structure type to define the range of output
 * operating values (current/voltage), limited by the PMIC IC design.
 *
 * @min  - minimum value
 * @max  - maximum value
 * @step - step value
*/
struct output_range {
	int min;
	int max;
	int step;
};

/* BUCK: 1,2 - voltage range */
static struct output_range buck_voltage_range[] = {
	RANGE(OUT_BUCK1_UV_MIN, OUT_BUCK1_UV_MAX, OUT_BUCK1_UV_STEP),
	RANGE(OUT_BUCK2_UV_MIN, OUT_BUCK2_UV_MAX, OUT_BUCK2_UV_STEP),
};

/* BUCK: 1 - current range */
static struct output_range buck_current_range[] = {
	RANGE(OUT_BUCK1_UA_MIN, OUT_BUCK1_UA_MAX, OUT_BUCK1_UA_STEP),
};

/* BUCK operating modes */
static struct dm_regulator_mode sandbox_buck_modes[] = {
	MODE(BUCK_OM_OFF, OM2REG(BUCK_OM_OFF), "OFF"),
	MODE(BUCK_OM_ON, OM2REG(BUCK_OM_ON), "ON"),
	MODE(BUCK_OM_PWM, OM2REG(BUCK_OM_PWM), "PWM"),
};

/* LDO: 1,2 - voltage range */
static struct output_range ldo_voltage_range[] = {
	RANGE(OUT_LDO1_UV_MIN, OUT_LDO1_UV_MAX, OUT_LDO1_UV_STEP),
	RANGE(OUT_LDO2_UV_MIN, OUT_LDO2_UV_MAX, OUT_LDO2_UV_STEP),
};

/* LDO: 1 - current range */
static struct output_range ldo_current_range[] = {
	RANGE(OUT_LDO1_UA_MIN, OUT_LDO1_UA_MAX, OUT_LDO1_UA_STEP),
};

/* LDO operating modes */
static struct dm_regulator_mode sandbox_ldo_modes[] = {
	MODE(LDO_OM_OFF, OM2REG(LDO_OM_OFF), "OFF"),
	MODE(LDO_OM_ON, OM2REG(LDO_OM_ON), "ON"),
	MODE(LDO_OM_SLEEP, OM2REG(LDO_OM_SLEEP), "SLEEP"),
	MODE(LDO_OM_STANDBY, OM2REG(LDO_OM_STANDBY), "STANDBY"),
};

int out_get_value(struct udevice *dev, int output_count, int reg_type,
		  struct output_range *range)
{
	uint8_t reg_val;
	uint reg;
	int ret;

	if (dev->driver_data > output_count) {
		pr_err("Unknown regulator number: %lu for PMIC %s!",
		      dev->driver_data, dev->name);
		return -EINVAL;
	}

	reg = (dev->driver_data - 1) * OUT_REG_COUNT + reg_type;
	ret = pmic_read(dev->parent, reg, &reg_val, 1);
	if (ret) {
		pr_err("PMIC read failed: %d\n",  ret);
		return ret;
	}

	ret =  REG2VAL(range[dev->driver_data - 1].min,
		       range[dev->driver_data - 1].step,
		       reg_val);

	return ret;
}

static int out_set_value(struct udevice *dev, int output_count, int reg_type,
			 struct output_range *range, int value)
{
	uint8_t reg_val;
	uint reg;
	int ret;
	int max_value;

	if (dev->driver_data > output_count) {
		pr_err("Unknown regulator number: %lu for PMIC %s!",
		      dev->driver_data, dev->name);
		return -EINVAL;
	}

	max_value = range[dev->driver_data - 1].max;
	if (value > max_value) {
		pr_err("Wrong value for %s: %lu. Max is: %d.",
		      dev->name, dev->driver_data, max_value);
		return -EINVAL;
	}

	reg_val = VAL2REG(range[dev->driver_data - 1].min,
			  range[dev->driver_data - 1].step,
			  value);

	reg = (dev->driver_data - 1) * OUT_REG_COUNT + reg_type;
	ret = pmic_write(dev->parent, reg, &reg_val, 1);
	if (ret) {
		pr_err("PMIC write failed: %d\n",  ret);
		return ret;
	}

	return 0;
}

static int out_get_mode(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	uint8_t reg_val;
	uint reg;
	int ret;
	int i;

	uc_pdata = dev_get_uclass_platdata(dev);

	reg = (dev->driver_data - 1) * OUT_REG_COUNT + OUT_REG_OM;
	ret = pmic_read(dev->parent, reg, &reg_val, 1);
	if (ret) {
		pr_err("PMIC read failed: %d\n",  ret);
		return ret;
	}

	for (i = 0; i < uc_pdata->mode_count; i++) {
		if (reg_val == uc_pdata->mode[i].register_value)
			return uc_pdata->mode[i].id;
	}

	pr_err("Unknown operation mode for %s!", dev->name);
	return -EINVAL;
}

static int out_set_mode(struct udevice *dev, int mode)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	int reg_val = -1;
	uint reg;
	int ret;
	int i;

	uc_pdata = dev_get_uclass_platdata(dev);

	if (mode >= uc_pdata->mode_count)
		return -EINVAL;

	for (i = 0; i < uc_pdata->mode_count; i++) {
		if (mode == uc_pdata->mode[i].id) {
			reg_val = uc_pdata->mode[i].register_value;
			break;
		}
	}

	if (reg_val == -1) {
		pr_err("Unknown operation mode for %s!", dev->name);
		return -EINVAL;
	}

	reg = (dev->driver_data - 1) * OUT_REG_COUNT + OUT_REG_OM;
	ret = pmic_write(dev->parent, reg, (uint8_t *)&reg_val, 1);
	if (ret) {
		pr_err("PMIC write failed: %d\n",  ret);
		return ret;
	}

	return 0;
}

static int buck_get_voltage(struct udevice *dev)
{
	return out_get_value(dev, SANDBOX_BUCK_COUNT, OUT_REG_UV,
			      buck_voltage_range);
}

static int buck_set_voltage(struct udevice *dev, int uV)
{
	return out_set_value(dev, SANDBOX_BUCK_COUNT, OUT_REG_UV,
			      buck_voltage_range, uV);
}

static int buck_get_current(struct udevice *dev)
{
	/* BUCK2 - unsupported */
	if (dev->driver_data == 2)
		return -ENOSYS;

	return out_get_value(dev, SANDBOX_BUCK_COUNT, OUT_REG_UA,
			      buck_current_range);
}

static int buck_set_current(struct udevice *dev, int uA)
{
	/* BUCK2 - unsupported */
	if (dev->driver_data == 2)
		return -ENOSYS;

	return out_set_value(dev, SANDBOX_BUCK_COUNT, OUT_REG_UA,
			      buck_current_range, uA);
}

static int buck_get_enable(struct udevice *dev)
{
	if (out_get_mode(dev) == BUCK_OM_OFF)
		return false;

	return true;
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	return out_set_mode(dev, enable ? BUCK_OM_ON : BUCK_OM_OFF);
}

static int sandbox_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode = sandbox_buck_modes;
	uc_pdata->mode_count = ARRAY_SIZE(sandbox_buck_modes);

	return 0;
}

static const struct dm_regulator_ops sandbox_buck_ops = {
	.get_value   = buck_get_voltage,
	.set_value   = buck_set_voltage,
	.get_current = buck_get_current,
	.set_current = buck_set_current,
	.get_enable  = buck_get_enable,
	.set_enable  = buck_set_enable,
	.get_mode    = out_get_mode,
	.set_mode    = out_set_mode,
};

U_BOOT_DRIVER(sandbox_buck) = {
	.name = SANDBOX_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &sandbox_buck_ops,
	.probe = sandbox_buck_probe,
};

static int ldo_get_voltage(struct udevice *dev)
{
	return out_get_value(dev, SANDBOX_LDO_COUNT, OUT_REG_UV,
			     ldo_voltage_range);
}

static int ldo_set_voltage(struct udevice *dev, int uV)
{
	return out_set_value(dev, SANDBOX_LDO_COUNT, OUT_REG_UV,
			     ldo_voltage_range, uV);
}

static int ldo_get_current(struct udevice *dev)
{
	/* LDO2 - unsupported */
	if (dev->driver_data == 2)
		return -ENOSYS;

	return out_get_value(dev, SANDBOX_LDO_COUNT, OUT_REG_UA,
			     ldo_current_range);
}

static int ldo_set_current(struct udevice *dev, int uA)
{
	/* LDO2 - unsupported */
	if (dev->driver_data == 2)
		return -ENOSYS;

	return out_set_value(dev, SANDBOX_LDO_COUNT, OUT_REG_UA,
			     ldo_current_range, uA);
}

static int ldo_get_enable(struct udevice *dev)
{
	if (out_get_mode(dev) == LDO_OM_OFF)
		return false;

	return true;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	return out_set_mode(dev, enable ? LDO_OM_ON : LDO_OM_OFF);
}

static int sandbox_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->mode = sandbox_ldo_modes;
	uc_pdata->mode_count = ARRAY_SIZE(sandbox_ldo_modes);

	return 0;
}

static const struct dm_regulator_ops sandbox_ldo_ops = {
	.get_value   = ldo_get_voltage,
	.set_value   = ldo_set_voltage,
	.get_current = ldo_get_current,
	.set_current = ldo_set_current,
	.get_enable  = ldo_get_enable,
	.set_enable  = ldo_set_enable,
	.get_mode    = out_get_mode,
	.set_mode    = out_set_mode,
};

U_BOOT_DRIVER(sandbox_ldo) = {
	.name = SANDBOX_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &sandbox_ldo_ops,
	.probe = sandbox_ldo_probe,
};
