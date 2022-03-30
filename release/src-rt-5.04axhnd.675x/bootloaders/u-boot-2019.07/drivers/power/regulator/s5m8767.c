// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/s5m8767.h>

static const struct sec_voltage_desc buck_v1 = {
	.max = 2225000,
	.min =  650000,
	.step =   6250,
};

static const struct sec_voltage_desc buck_v2 = {
	.max = 1600000,
	.min =  600000,
	.step =   6250,
};

static const struct sec_voltage_desc buck_v3 = {
	.max = 3000000,
	.min =  750000,
	.step =  12500,
};

static const struct sec_voltage_desc ldo_v1 = {
	.max = 3950000,
	.min =  800000,
	.step =  50000,
};

static const struct sec_voltage_desc ldo_v2 = {
	.max = 2375000,
	.min =  800000,
	.step =  25000,
};

static const struct s5m8767_para buck_param[] = {
	/*
	 *            | voltage ----|  | enable -|   voltage
	 * regnum       addr  bpos mask  addr  on     desc
	 */
	{S5M8767_BUCK1, 0x33, 0x0, 0xff, 0x32, 0x3, &buck_v1},
	{S5M8767_BUCK2, 0x35, 0x0, 0xff, 0x34, 0x1, &buck_v2},
	{S5M8767_BUCK3, 0x3e, 0x0, 0xff, 0x3d, 0x1, &buck_v2},
	{S5M8767_BUCK4, 0x47, 0x0, 0xff, 0x46, 0x1, &buck_v2},
	{S5M8767_BUCK5, 0x50, 0x0, 0xff, 0x4f, 0x3, &buck_v1},
	{S5M8767_BUCK6, 0x55, 0x0, 0xff, 0x54, 0x3, &buck_v1},
	{S5M8767_BUCK7, 0x57, 0x0, 0xff, 0x56, 0x3, &buck_v3},
	{S5M8767_BUCK8, 0x59, 0x0, 0xff, 0x58, 0x3, &buck_v3},
	{S5M8767_BUCK9, 0x5b, 0x0, 0xff, 0x5a, 0x3, &buck_v3},
};

static const struct s5m8767_para ldo_param[] = {
	{S5M8767_LDO1,  0x5c, 0x0, 0x3f, 0x5c, 0x3, &ldo_v2},
	{S5M8767_LDO2,  0x5d, 0x0, 0x3f, 0x5d, 0x1, &ldo_v2},
	{S5M8767_LDO3,  0x61, 0x0, 0x3f, 0x61, 0x3, &ldo_v1},
	{S5M8767_LDO4,  0x62, 0x0, 0x3f, 0x62, 0x3, &ldo_v1},
	{S5M8767_LDO5,  0x63, 0x0, 0x3f, 0x63, 0x3, &ldo_v1},
	{S5M8767_LDO6,  0x64, 0x0, 0x3f, 0x64, 0x1, &ldo_v2},
	{S5M8767_LDO7,  0x65, 0x0, 0x3f, 0x65, 0x1, &ldo_v2},
	{S5M8767_LDO8,  0x66, 0x0, 0x3f, 0x66, 0x1, &ldo_v2},
	{S5M8767_LDO9,  0x67, 0x0, 0x3f, 0x67, 0x3, &ldo_v1},
	{S5M8767_LDO10, 0x68, 0x0, 0x3f, 0x68, 0x1, &ldo_v1},
	{S5M8767_LDO11, 0x69, 0x0, 0x3f, 0x69, 0x1, &ldo_v1},
	{S5M8767_LDO12, 0x6a, 0x0, 0x3f, 0x6a, 0x1, &ldo_v1},
	{S5M8767_LDO13, 0x6b, 0x0, 0x3f, 0x6b, 0x3, &ldo_v1},
	{S5M8767_LDO14, 0x6c, 0x0, 0x3f, 0x6c, 0x1, &ldo_v1},
	{S5M8767_LDO15, 0x6d, 0x0, 0x3f, 0x6d, 0x1, &ldo_v2},
	{S5M8767_LDO16, 0x6e, 0x0, 0x3f, 0x6e, 0x1, &ldo_v1},
	{S5M8767_LDO17, 0x6f, 0x0, 0x3f, 0x6f, 0x3, &ldo_v1},
	{S5M8767_LDO18, 0x70, 0x0, 0x3f, 0x70, 0x3, &ldo_v1},
	{S5M8767_LDO19, 0x71, 0x0, 0x3f, 0x71, 0x3, &ldo_v1},
	{S5M8767_LDO20, 0x72, 0x0, 0x3f, 0x72, 0x3, &ldo_v1},
	{S5M8767_LDO21, 0x73, 0x0, 0x3f, 0x73, 0x3, &ldo_v1},
	{S5M8767_LDO22, 0x74, 0x0, 0x3f, 0x74, 0x3, &ldo_v1},
	{S5M8767_LDO23, 0x75, 0x0, 0x3f, 0x75, 0x3, &ldo_v1},
	{S5M8767_LDO24, 0x76, 0x0, 0x3f, 0x76, 0x3, &ldo_v1},
	{S5M8767_LDO25, 0x77, 0x0, 0x3f, 0x77, 0x3, &ldo_v1},
	{S5M8767_LDO26, 0x78, 0x0, 0x3f, 0x78, 0x3, &ldo_v1},
	{S5M8767_LDO27, 0x79, 0x0, 0x3f, 0x79, 0x3, &ldo_v1},
	{S5M8767_LDO28, 0x7a, 0x0, 0x3f, 0x7a, 0x3, &ldo_v1},
};

enum {
	ENABLE_SHIFT	= 6,
	ENABLE_MASK	= 3,
};

static int reg_get_value(struct udevice *dev, const struct s5m8767_para *param)
{
	const struct sec_voltage_desc *desc;
	int ret, uv, val;

	ret = pmic_reg_read(dev->parent, param->vol_addr);
	if (ret < 0)
		return ret;

	desc = param->vol;
	val = (ret >> param->vol_bitpos) & param->vol_bitmask;
	uv = desc->min + val * desc->step;

	return uv;
}

static int reg_set_value(struct udevice *dev, const struct s5m8767_para *param,
			 int uv)
{
	const struct sec_voltage_desc *desc;
	int ret, val;

	desc = param->vol;
	if (uv < desc->min || uv > desc->max)
		return -EINVAL;
	val = (uv - desc->min) / desc->step;
	val = (val & param->vol_bitmask) << param->vol_bitpos;
	ret = pmic_clrsetbits(dev->parent, param->vol_addr,
			      param->vol_bitmask << param->vol_bitpos,
			      val);

	return ret;
}

static int s5m8767_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->mode_count = 0;

	return 0;
}
static int ldo_get_value(struct udevice *dev)
{
	int ldo = dev->driver_data;

	return reg_get_value(dev, &ldo_param[ldo]);
}

static int ldo_set_value(struct udevice *dev, int uv)
{
	int ldo = dev->driver_data;

	return reg_set_value(dev, &ldo_param[ldo], uv);
}

static int reg_get_enable(struct udevice *dev, const struct s5m8767_para *param)
{
	bool enable;
	int ret;

	ret = pmic_reg_read(dev->parent, param->reg_enaddr);
	if (ret < 0)
		return ret;

	enable = (ret >> ENABLE_SHIFT) & ENABLE_MASK;

	return enable;
}

static int reg_set_enable(struct udevice *dev, const struct s5m8767_para *param,
			  bool enable)
{
	int ret;

	ret = pmic_reg_read(dev->parent, param->reg_enaddr);
	if (ret < 0)
		return ret;

	ret = pmic_clrsetbits(dev->parent, param->reg_enaddr,
			      ENABLE_MASK << ENABLE_SHIFT,
			      enable ? param->reg_enbiton << ENABLE_SHIFT : 0);

	return ret;
}

static int ldo_get_enable(struct udevice *dev)
{
	int ldo = dev->driver_data;

	return reg_get_enable(dev, &ldo_param[ldo]);
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	int ldo = dev->driver_data;

	return reg_set_enable(dev, &ldo_param[ldo], enable);
}

static int s5m8767_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode_count = 0;

	return 0;
}

static int buck_get_value(struct udevice *dev)
{
	int buck = dev->driver_data;

	return reg_get_value(dev, &buck_param[buck]);
}

static int buck_set_value(struct udevice *dev, int uv)
{
	int buck = dev->driver_data;

	return reg_set_value(dev, &buck_param[buck], uv);
}

static int buck_get_enable(struct udevice *dev)
{
	int buck = dev->driver_data;

	return reg_get_enable(dev, &buck_param[buck]);
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	int buck = dev->driver_data;

	return reg_set_enable(dev, &buck_param[buck], enable);
}

static const struct dm_regulator_ops s5m8767_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = ldo_get_enable,
	.set_enable = ldo_set_enable,
};

U_BOOT_DRIVER(s5m8767_ldo) = {
	.name = S5M8767_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &s5m8767_ldo_ops,
	.probe = s5m8767_ldo_probe,
};

static const struct dm_regulator_ops s5m8767_buck_ops = {
	.get_value  = buck_get_value,
	.set_value  = buck_set_value,
	.get_enable = buck_get_enable,
	.set_enable = buck_set_enable,
};

U_BOOT_DRIVER(s5m8767_buck) = {
	.name = S5M8767_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &s5m8767_buck_ops,
	.probe = s5m8767_buck_probe,
};
