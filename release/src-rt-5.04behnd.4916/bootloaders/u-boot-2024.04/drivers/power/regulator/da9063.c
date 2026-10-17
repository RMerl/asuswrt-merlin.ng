// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2018 Flowbird
 *  Martin Fuzzey  <martin.fuzzey@flowbird.group>
 */

#include <common.h>
#include <dm.h>
#include <linux/bitops.h>
#include <power/da9063_pmic.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define	DA9063_BUCK_EN		0x01
#define	DA9063_LDO_EN		0x01
#define DA9063_VBUCK_MASK	0x7F
#define DA9063_BUCK_SL		0x80
#define DA9063_LDO_SL		0x80

#define DA9063_VLDO1_MASK	0x3F
#define DA9063_VLDO2_MASK	0x3F
#define DA9063_VLDO3_MASK	0x7F
#define DA9063_VLDO4_MASK	0x7F
#define DA9063_VLDO5_MASK	0x3F
#define DA9063_VLDO6_MASK	0x3F
#define DA9063_VLDO7_MASK	0x3F
#define DA9063_VLDO8_MASK	0x3F
#define DA9063_VLDO9_MASK	0x3F
#define DA9063_VLDO10_MASK	0x3F
#define DA9063_VLDO11_MASK	0x3F

#define DA9063_BUCK_MODE_MASK	0xC0
#define	DA9063_BUCK_MODE_MANUAL	0x00
#define	DA9063_BUCK_MODE_SLEEP	0x40
#define	DA9063_BUCK_MODE_SYNC	0x80
#define	DA9063_BUCK_MODE_AUTO	0xC0

#define DA9063_BIO_ILIM_MASK	0x0F
#define DA9063_BMEM_ILIM_MASK	0xF0
#define DA9063_BPRO_ILIM_MASK	0x0F
#define DA9063_BPERI_ILIM_MASK	0xF0
#define DA9063_BCORE1_ILIM_MASK	0x0F
#define DA9063_BCORE2_ILIM_MASK	0xF0

struct da9063_reg_info {
	uint min_uV;
	uint step_uV;
	uint max_uV;
	uint min_uA;
	uint step_uA;
	uint max_uA;
	uint en_reg;
	uint vsel_reg;
	uint mode_reg;
	uint ilim_reg;
	u8 en_mask;
	u8 vsel_mask;
	u8 ilim_mask;
	const char *dt_node_name;
	const int *current_limits;
};

struct da9063_priv {
	const struct da9063_reg_info *reg_info;
};

static struct dm_regulator_mode da9063_ldo_modes[] = {
	{ .id = DA9063_LDOMODE_SLEEP,
		.register_value = DA9063_LDO_SL, .name = "SLEEP" },
	{ .id = DA9063_LDOMODE_NORMAL,
		.register_value = 0, .name = "NORMAL" },
};

#define DA9063_LDO(regl_name, min_mV, step_mV, max_mV) \
	.min_uV = (min_mV) * 1000, \
	.step_uV = (step_mV) * 1000, \
	.max_uV = (max_mV) * 1000, \
	.en_reg = DA9063_REG_##regl_name##_CONT, \
	.en_mask = DA9063_LDO_EN, \
	.vsel_reg = DA9063_REG_V##regl_name##_A, \
	.vsel_mask = DA9063_V##regl_name##_MASK, \
	.mode_reg = DA9063_REG_V##regl_name##_A \

/* This array is directly indexed so must stay in numerical order */
static const struct da9063_reg_info da9063_ldo_info[] = {
	{ DA9063_LDO(LDO1, 600, 20, 1860) },
	{ DA9063_LDO(LDO2, 600, 20, 1860) },
	{ DA9063_LDO(LDO3, 900, 20, 3440) },
	{ DA9063_LDO(LDO4, 900, 20, 3440) },
	{ DA9063_LDO(LDO5, 900, 50, 3600) },
	{ DA9063_LDO(LDO6, 900, 50, 3600) },
	{ DA9063_LDO(LDO7, 900, 50, 3600) },
	{ DA9063_LDO(LDO8, 900, 50, 3600) },
	{ DA9063_LDO(LDO9, 950, 50, 3600) },
	{ DA9063_LDO(LDO10, 900, 50, 3600) },
	{ DA9063_LDO(LDO11, 900, 50, 3600) },
};

static struct dm_regulator_mode da9063_buck_modes[] = {
	{ .id = DA9063_BUCKMODE_SLEEP,
		.register_value = DA9063_BUCK_MODE_SLEEP, .name = "SLEEP" },
	{ .id = DA9063_BUCKMODE_SYNC,
		.register_value = DA9063_BUCK_MODE_SYNC, .name = "SYNC" },
	{ .id = DA9063_BUCKMODE_AUTO,
		.register_value = DA9063_BUCK_MODE_AUTO, .name = "AUTO" },
};

#define DA9063_BUCK(regl_name, dt_name, \
		    min_mV, step_mV, max_mV, \
		    min_mA, step_mA, max_mA, _ilim_reg) \
	.dt_node_name = dt_name, \
	.min_uV = (min_mV) * 1000, \
	.step_uV = (step_mV) * 1000, \
	.max_uV = (max_mV) * 1000, \
	.min_uA = (min_mA) * 1000, \
	.step_uA = (step_mA) * 1000, \
	.max_uA = (max_mA) * 1000, \
	.en_reg = DA9063_REG_##regl_name##_CONT, \
	.en_mask = DA9063_BUCK_EN, \
	.vsel_reg = DA9063_REG_V##regl_name##_A, \
	.vsel_mask = DA9063_VBUCK_MASK, \
	.mode_reg = DA9063_REG_##regl_name##_CFG, \
	.ilim_reg = DA9063_REG_BUCK_ILIM_##_ilim_reg, \
	.ilim_mask = DA9063_##regl_name##_ILIM_MASK

static const struct da9063_reg_info da9063_buck_info[] = {
	/*				mV		mA */
	{ DA9063_BUCK(BCORE1, "bcore1",	300, 10, 1570,	500, 100, 2000,	C) },
	{ DA9063_BUCK(BCORE2, "bcore2", 300, 10, 1570,	500, 100, 2000, C) },
	{ DA9063_BUCK(BPRO, "bpro",	530, 10, 1800,	500, 100, 2000, B) },
	{ DA9063_BUCK(BMEM, "bmem",	800, 20, 3340,	1500, 100, 3000, A) },
	{ DA9063_BUCK(BIO, "bio",	800, 20, 3340,	1500, 100, 3000, A) },
	{ DA9063_BUCK(BPERI, "bperi",	800, 20, 3340,	1500, 100, 3000, B) },
};

static int da9063_get_enable(struct udevice *dev)
{
	const struct da9063_priv *priv = dev_get_priv(dev);
	const struct da9063_reg_info *info = priv->reg_info;
	int ret;

	ret = pmic_reg_read(dev->parent, info->en_reg);
	if (ret < 0)
		return ret;

	return ret & info->en_mask ? true : false;
}

static int da9063_set_enable(struct udevice *dev, bool enable)
{
	const struct da9063_priv *priv = dev_get_priv(dev);
	const struct da9063_reg_info *info = priv->reg_info;

	return pmic_clrsetbits(dev->parent, info->en_reg,
			       info->en_mask, enable ? info->en_mask : 0);
}

static int da9063_get_voltage(struct udevice *dev)
{
	const struct da9063_priv *priv = dev_get_priv(dev);
	const struct da9063_reg_info *info = priv->reg_info;
	int ret;

	ret = pmic_reg_read(dev->parent, info->vsel_reg);
	if (ret < 0)
		return ret;

	return info->min_uV + (ret & info->vsel_mask) * info->step_uV;
}

static int da9063_set_voltage(struct udevice *dev, int uV)
{
	const struct da9063_priv *priv = dev_get_priv(dev);
	const struct da9063_reg_info *info = priv->reg_info;
	uint sel;

	if (uV < info->min_uV || uV > info->max_uV)
		return -EINVAL;

	sel = (uV - info->min_uV) / info->step_uV;

	return pmic_clrsetbits(dev->parent, info->vsel_reg,
			       info->vsel_mask, sel);
}

static const struct dm_regulator_mode
	*da9063_find_mode_by_id(int id,
				const struct dm_regulator_mode *modes,
				uint mode_count)
{
	for (; mode_count; mode_count--) {
		if (modes->id == id)
			return modes;
		modes++;
	}
	return NULL;
}

static int ldo_get_mode(struct udevice *dev)
{
	const struct da9063_priv *priv = dev_get_priv(dev);
	const struct da9063_reg_info *info = priv->reg_info;
	int val;

	val = pmic_reg_read(dev->parent, info->mode_reg);
	if (val < 0)
		return val;

	if (val & DA9063_LDO_SL)
		return DA9063_LDOMODE_SLEEP;
	else
		return DA9063_LDOMODE_NORMAL;
}

static int ldo_set_mode(struct udevice *dev, int mode_id)
{
	const struct da9063_priv *priv = dev_get_priv(dev);
	const struct da9063_reg_info *info = priv->reg_info;
	const struct dm_regulator_mode *mode;

	mode = da9063_find_mode_by_id(mode_id,
				      da9063_ldo_modes,
				      ARRAY_SIZE(da9063_ldo_modes));
	if (!mode)
		return -EINVAL;

	return pmic_clrsetbits(dev->parent, info->mode_reg,
			       DA9063_LDO_SL, mode->register_value);
}

static int buck_get_mode(struct udevice *dev)
{
	const struct da9063_priv *priv = dev_get_priv(dev);
	const struct da9063_reg_info *info = priv->reg_info;
	int i;
	int val;

	val = pmic_reg_read(dev->parent, info->mode_reg);
	if (val < 0)
		return val;

	val &= DA9063_BUCK_MODE_MASK;
	if (val == DA9063_BUCK_MODE_MANUAL) {
		val = pmic_reg_read(dev->parent, info->vsel_reg);
		if (val < 0)
			return val;

		if (val & DA9063_BUCK_SL)
			return DA9063_BUCKMODE_SLEEP;
		else
			return DA9063_BUCKMODE_SYNC;
	}

	for (i = 0; i < ARRAY_SIZE(da9063_buck_modes); i++) {
		if (da9063_buck_modes[i].register_value == val)
			return da9063_buck_modes[i].id;
	}

	return -EINVAL;
}

static int buck_set_mode(struct udevice *dev, int mode_id)
{
	const struct da9063_priv *priv = dev_get_priv(dev);
	const struct da9063_reg_info *info = priv->reg_info;
	const struct dm_regulator_mode *mode;

	mode = da9063_find_mode_by_id(mode_id,
				      da9063_buck_modes,
				      ARRAY_SIZE(da9063_buck_modes));
	if (!mode)
		return -EINVAL;

	return pmic_clrsetbits(dev->parent, info->mode_reg,
			       DA9063_BUCK_MODE_MASK, mode->register_value);
}

static int buck_get_current_limit(struct udevice *dev)
{
	const struct da9063_priv *priv = dev_get_priv(dev);
	const struct da9063_reg_info *info = priv->reg_info;
	int val;

	val = pmic_reg_read(dev->parent, info->ilim_reg);
	if (val < 0)
		return val;

	val &= info->ilim_mask;
	val >>= (ffs(info->ilim_mask) - 1);

	return info->min_uA + val * info->step_uA;
}

static int buck_set_current_limit(struct udevice *dev, int uA)
{
	const struct da9063_priv *priv = dev_get_priv(dev);
	const struct da9063_reg_info *info = priv->reg_info;
	int val;

	if (uA < info->min_uA || uA > info->max_uA)
		return -EINVAL;

	val = (uA - info->min_uA) / info->step_uA;
	val <<= (ffs(info->ilim_mask) - 1);

	return pmic_clrsetbits(dev->parent, info->ilim_reg,
			       info->ilim_mask, val);
}

static int da9063_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct da9063_priv *priv = dev_get_priv(dev);

	/* LDOs are named numerically in DT so can directly index */
	if (dev->driver_data < 1 ||
	    dev->driver_data > ARRAY_SIZE(da9063_ldo_info))
		return -EINVAL;
	priv->reg_info = &da9063_ldo_info[dev->driver_data - 1];

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->mode = da9063_ldo_modes;
	uc_pdata->mode_count = ARRAY_SIZE(da9063_ldo_modes);

	return 0;
}

static int da9063_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct da9063_priv *priv = dev_get_priv(dev);
	int i;

	/* Bucks have names rather than numbers so need to match with DT */
	for (i = 0; i < ARRAY_SIZE(da9063_buck_info); i++) {
		const struct da9063_reg_info *info = &da9063_buck_info[i];

		if (!strcmp(info->dt_node_name, dev->name)) {
			priv->reg_info = info;
			break;
		}
	}
	if (!priv->reg_info)
		return -ENODEV;

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode = da9063_buck_modes;
	uc_pdata->mode_count = ARRAY_SIZE(da9063_buck_modes);

	return 0;
}

static const struct dm_regulator_ops da9063_ldo_ops = {
	.get_value  = da9063_get_voltage,
	.set_value  = da9063_set_voltage,
	.get_enable = da9063_get_enable,
	.set_enable = da9063_set_enable,
	.get_mode   = ldo_get_mode,
	.set_mode   = ldo_set_mode,
};

U_BOOT_DRIVER(da9063_ldo) = {
	.name = DA9063_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &da9063_ldo_ops,
	.probe = da9063_ldo_probe,
	.priv_auto	= sizeof(struct da9063_priv),
};

static const struct dm_regulator_ops da9063_buck_ops = {
	.get_value  = da9063_get_voltage,
	.set_value  = da9063_set_voltage,
	.get_enable = da9063_get_enable,
	.set_enable = da9063_set_enable,
	.get_mode   = buck_get_mode,
	.set_mode   = buck_set_mode,
	.get_current = buck_get_current_limit,
	.set_current = buck_set_current_limit,
};

U_BOOT_DRIVER(da9063_buck) = {
	.name = DA9063_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &da9063_buck_ops,
	.probe = da9063_buck_probe,
	.priv_auto	= sizeof(struct da9063_priv),
};
