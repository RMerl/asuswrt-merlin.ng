// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2023 Samuel Holland <samuel@sholland.org>
 */

#include <axp_pmic.h>
#include <dm.h>
#include <errno.h>
#include <dm/device-internal.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define NA 0xff

struct axp_regulator_plat {
	const char	*name;
	u8		enable_reg;
	u8		enable_mask;
	u8		volt_reg;
	u8		volt_mask;
	u16		min_mV;
	u16		max_mV;
	u8		step_mV;
	u8		split;
	const u16	*table;
};

static int axp_regulator_get_value(struct udevice *dev)
{
	const struct axp_regulator_plat *plat = dev_get_plat(dev);
	int mV, sel;

	if (plat->volt_reg == NA)
		return -EINVAL;

	sel = pmic_reg_read(dev->parent, plat->volt_reg);
	if (sel < 0)
		return sel;

	sel &= plat->volt_mask;
	sel >>= ffs(plat->volt_mask) - 1;

	if (plat->table) {
		mV = plat->table[sel];
	} else {
		if (sel > plat->split)
			sel = plat->split + (sel - plat->split) * 2;
		mV = plat->min_mV + sel * plat->step_mV;
	}

	return mV * 1000;
}

static int axp_regulator_set_value(struct udevice *dev, int uV)
{
	const struct axp_regulator_plat *plat = dev_get_plat(dev);
	int mV = uV / 1000;
	uint sel, shift;

	if (plat->volt_reg == NA)
		return -EINVAL;
	if (mV < plat->min_mV || mV > plat->max_mV)
		return -EINVAL;

	shift = ffs(plat->volt_mask) - 1;

	if (plat->table) {
		/*
		 * The table must be monotonically increasing and
		 * have an entry for each possible field value.
		 */
		sel = plat->volt_mask >> shift;
		while (sel && plat->table[sel] > mV)
			sel--;
	} else {
		sel = (mV - plat->min_mV) / plat->step_mV;
		if (sel > plat->split)
			sel = plat->split + (sel - plat->split) / 2;
	}

	return pmic_clrsetbits(dev->parent, plat->volt_reg,
			       plat->volt_mask, sel << shift);
}

static int axp_regulator_get_enable(struct udevice *dev)
{
	const struct axp_regulator_plat *plat = dev_get_plat(dev);
	int reg;

	reg = pmic_reg_read(dev->parent, plat->enable_reg);
	if (reg < 0)
		return reg;

	return (reg & plat->enable_mask) == plat->enable_mask;
}

static int axp_regulator_set_enable(struct udevice *dev, bool enable)
{
	const struct axp_regulator_plat *plat = dev_get_plat(dev);

	return pmic_clrsetbits(dev->parent, plat->enable_reg,
			       plat->enable_mask,
			       enable ? plat->enable_mask : 0);
}

static const struct dm_regulator_ops axp_regulator_ops = {
	.get_value		= axp_regulator_get_value,
	.set_value		= axp_regulator_set_value,
	.get_enable		= axp_regulator_get_enable,
	.set_enable		= axp_regulator_set_enable,
};

static const u16 axp152_dcdc1_table[] = {
	1700, 1800, 1900, 2000, 2100, 2400, 2500, 2600,
	2700, 2800, 3000, 3100, 3200, 3300, 3400, 3500,
};

static const u16 axp152_aldo12_table[] = {
	1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900,
	2000, 2500, 2700, 2800, 3000, 3100, 3200, 3300,
};

static const u16 axp152_ldo0_table[] = {
	5000, 3300, 2800, 2500,
};

static const struct axp_regulator_plat axp152_regulators[] = {
	{ "dcdc1", 0x12, BIT(7), 0x26, 0x0f, .table = axp152_dcdc1_table },
	{ "dcdc2", 0x12, BIT(6), 0x23, 0x3f,  700, 2275,  25, NA },
	{ "dcdc3", 0x12, BIT(5), 0x27, 0x3f,  700, 3500,  50, NA },
	{ "dcdc4", 0x12, BIT(4), 0x2b, 0x7f,  700, 3500,  25, NA },
	{ "aldo1", 0x12, BIT(3), 0x28, 0xf0, .table = axp152_aldo12_table },
	{ "aldo2", 0x12, BIT(2), 0x28, 0x0f, .table = axp152_aldo12_table },
	{ "dldo1", 0x12, BIT(1), 0x29, 0x1f,  700, 3500, 100, NA },
	{ "dldo2", 0x12, BIT(0), 0x2a, 0x1f,  700, 3500, 100, NA },
	{ "ldo0",  0x15, BIT(7), 0x15, 0x30, .table = axp152_ldo0_table },
	{ }
};

static const u16 axp20x_ldo4_table[] = {
	1250, 1300, 1400, 1500, 1600, 1700, 1800, 1900,
	2000, 2500, 2700, 2800, 3000, 3100, 3200, 3300,
};

static const struct axp_regulator_plat axp20x_regulators[] = {
	{ "dcdc2", 0x12, BIT(4), 0x23, 0x3f,  700, 2275,  25, NA },
	{ "dcdc3", 0x12, BIT(1), 0x27, 0x7f,  700, 3500,  25, NA },
	{ "ldo2",  0x12, BIT(2), 0x28, 0xf0, 1800, 3300, 100, NA },
	{ "ldo3",  0x12, BIT(6), 0x29, 0x7f,  700, 2275,  25, NA },
	{ "ldo4",  0x12, BIT(3), 0x28, 0x0f, .table = axp20x_ldo4_table },
	{ }
};

static const struct axp_regulator_plat axp22x_regulators[] = {
	{"dc5ldo", 0x10, BIT(0), 0x1c, 0x07,  700, 1400, 100, NA },
	{ "dcdc1", 0x10, BIT(1), 0x21, 0x1f, 1600, 3400, 100, NA },
	{ "dcdc2", 0x10, BIT(2), 0x22, 0x3f,  600, 1540,  20, NA },
	{ "dcdc3", 0x10, BIT(3), 0x23, 0x3f,  600, 1860,  20, NA },
	{ "dcdc4", 0x10, BIT(4), 0x24, 0x3f,  600, 1540,  20, NA },
	{ "dcdc5", 0x10, BIT(5), 0x25, 0x1f, 1000, 2550,  50, NA },
	{ "aldo1", 0x10, BIT(6), 0x28, 0x1f,  700, 3300, 100, NA },
	{ "aldo2", 0x10, BIT(7), 0x29, 0x1f,  700, 3300, 100, NA },
	{ "aldo3", 0x13, BIT(7), 0x2a, 0x1f,  700, 3300, 100, NA },
	{ "dldo1", 0x12, BIT(3), 0x15, 0x1f,  700, 3300, 100, NA },
	{ "dldo2", 0x12, BIT(4), 0x16, 0x1f,  700, 3300, 100, NA },
	{ "dldo3", 0x12, BIT(5), 0x17, 0x1f,  700, 3300, 100, NA },
	{ "dldo4", 0x12, BIT(6), 0x18, 0x1f,  700, 3300, 100, NA },
	{ "eldo1", 0x12, BIT(0), 0x19, 0x1f,  700, 3300, 100, NA },
	{ "eldo2", 0x12, BIT(1), 0x1a, 0x1f,  700, 3300, 100, NA },
	{ "eldo3", 0x12, BIT(2), 0x1b, 0x1f,  700, 3300, 100, NA },
	{ "dc1sw", 0x12, BIT(7),   NA,   NA,   NA,   NA,  NA, NA },
	{ }
};

/*
 * The "dcdc1" regulator has another range, beyond 1.54V up to 3.4V, in
 * steps of 100mV. We cannot model this easily, but also don't need that,
 * since it's typically only used for ~1.1V anyway, so just ignore it.
 * Also the DCDC3 regulator is described wrongly in the (available) manual,
 * experiments show that the split point is at 1200mV, as for DCDC1/2.
 */
static const struct axp_regulator_plat axp313_regulators[] = {
	{ "dcdc1", 0x10, BIT(0), 0x13, 0x7f,  500, 1540,  10, 70 },
	{ "dcdc2", 0x10, BIT(1), 0x14, 0x7f,  500, 1540,  10, 70 },
	{ "dcdc3", 0x10, BIT(2), 0x15, 0x7f,  500, 1840,  10, 70 },
	{ "aldo1", 0x10, BIT(3), 0x16, 0x1f,  500, 3500, 100, NA },
	{ "dldo1", 0x10, BIT(4), 0x17, 0x1f,  500, 3500, 100, NA },
	{ }
};

static const struct axp_regulator_plat axp803_regulators[] = {
	{ "dcdc1", 0x10, BIT(0), 0x20, 0x1f, 1600, 3400, 100, NA },
	{ "dcdc2", 0x10, BIT(1), 0x21, 0x7f,  500, 1300,  10, 70 },
	{ "dcdc3", 0x10, BIT(2), 0x22, 0x7f,  500, 1300,  10, 70 },
	{ "dcdc4", 0x10, BIT(3), 0x23, 0x7f,  500, 1300,  10, 70 },
	{ "dcdc5", 0x10, BIT(4), 0x24, 0x7f,  800, 1840,  10, 32 },
	{ "dcdc6", 0x10, BIT(5), 0x25, 0x7f,  600, 1520,  10, 50 },
	{ "aldo1", 0x13, BIT(5), 0x28, 0x1f,  700, 3300, 100, NA },
	{ "aldo2", 0x13, BIT(6), 0x29, 0x1f,  700, 3300, 100, NA },
	{ "aldo3", 0x13, BIT(7), 0x2a, 0x1f,  700, 3300, 100, NA },
	{ "dldo1", 0x12, BIT(3), 0x15, 0x1f,  700, 3300, 100, NA },
	{ "dldo2", 0x12, BIT(4), 0x16, 0x1f,  700, 4200, 100, 27 },
	{ "dldo3", 0x12, BIT(5), 0x17, 0x1f,  700, 3300, 100, NA },
	{ "dldo4", 0x12, BIT(6), 0x18, 0x1f,  700, 3300, 100, NA },
	{ "eldo1", 0x12, BIT(0), 0x19, 0x1f,  700, 1900,  50, NA },
	{ "eldo2", 0x12, BIT(1), 0x1a, 0x1f,  700, 1900,  50, NA },
	{ "eldo3", 0x12, BIT(2), 0x1b, 0x1f,  700, 1900,  50, NA },
	{ "fldo1", 0x13, BIT(2), 0x1c, 0x0f,  700, 1450,  50, NA },
	{ "fldo2", 0x13, BIT(3), 0x1d, 0x0f,  700, 1450,  50, NA },
	{ "dc1sw", 0x12, BIT(7),   NA,   NA,   NA,   NA,  NA, NA },
	{ }
};

/*
 * The "dcdcd" split changes the step size by a factor of 5, not 2;
 * disallow values above the split to maintain accuracy.
 */
static const struct axp_regulator_plat axp806_regulators[] = {
	{ "dcdca", 0x10, BIT(0), 0x12, 0x7f,  600, 1520,  10, 50 },
	{ "dcdcb", 0x10, BIT(1), 0x13, 0x1f, 1000, 2550,  50, NA },
	{ "dcdcc", 0x10, BIT(2), 0x14, 0x7f,  600, 1520,  10, 50 },
	{ "dcdcd", 0x10, BIT(3), 0x15, 0x3f,  600, 1500,  20, NA },
	{ "dcdce", 0x10, BIT(4), 0x16, 0x1f, 1100, 3400, 100, NA },
	{ "aldo1", 0x10, BIT(5), 0x17, 0x1f,  700, 3300, 100, NA },
	{ "aldo2", 0x10, BIT(6), 0x18, 0x1f,  700, 3300, 100, NA },
	{ "aldo3", 0x10, BIT(7), 0x19, 0x1f,  700, 3300, 100, NA },
	{ "bldo1", 0x11, BIT(0), 0x20, 0x0f,  700, 1900, 100, NA },
	{ "bldo2", 0x11, BIT(1), 0x21, 0x0f,  700, 1900, 100, NA },
	{ "bldo3", 0x11, BIT(2), 0x22, 0x0f,  700, 1900, 100, NA },
	{ "bldo4", 0x11, BIT(3), 0x23, 0x0f,  700, 1900, 100, NA },
	{ "cldo1", 0x11, BIT(4), 0x24, 0x1f,  700, 3300, 100, NA },
	{ "cldo2", 0x11, BIT(5), 0x25, 0x1f,  700, 4200, 100, 27 },
	{ "cldo3", 0x11, BIT(6), 0x26, 0x1f,  700, 3300, 100, NA },
	{ "sw",    0x11, BIT(7),   NA,   NA,   NA,   NA,  NA, NA },
	{ }
};

/*
 * The "dcdc4" split changes the step size by a factor of 5, not 2;
 * disallow values above the split to maintain accuracy.
 */
static const struct axp_regulator_plat axp809_regulators[] = {
	{"dc5ldo", 0x10, BIT(0), 0x1c, 0x07,  700, 1400, 100, NA },
	{ "dcdc1", 0x10, BIT(1), 0x21, 0x1f, 1600, 3400, 100, NA },
	{ "dcdc2", 0x10, BIT(2), 0x22, 0x3f,  600, 1540,  20, NA },
	{ "dcdc3", 0x10, BIT(3), 0x23, 0x3f,  600, 1860,  20, NA },
	{ "dcdc4", 0x10, BIT(4), 0x24, 0x3f,  600, 1540,  20, NA },
	{ "dcdc5", 0x10, BIT(5), 0x25, 0x1f, 1000, 2550,  50, NA },
	{ "aldo1", 0x10, BIT(6), 0x28, 0x1f,  700, 3300, 100, NA },
	{ "aldo2", 0x10, BIT(7), 0x29, 0x1f,  700, 3300, 100, NA },
	{ "aldo3", 0x12, BIT(5), 0x2a, 0x1f,  700, 3300, 100, NA },
	{ "dldo1", 0x12, BIT(3), 0x15, 0x1f,  700, 3300, 100, NA },
	{ "dldo2", 0x12, BIT(4), 0x16, 0x1f,  700, 3300, 100, NA },
	{ "eldo1", 0x12, BIT(0), 0x19, 0x1f,  700, 3300, 100, NA },
	{ "eldo2", 0x12, BIT(1), 0x1a, 0x1f,  700, 3300, 100, NA },
	{ "eldo3", 0x12, BIT(2), 0x1b, 0x1f,  700, 3300, 100, NA },
	{ "sw",    0x12, BIT(6),   NA,   NA,   NA,   NA,  NA, NA },
	{ "dc1sw", 0x12, BIT(7),   NA,   NA,   NA,   NA,  NA, NA },
	{ }
};

static const struct axp_regulator_plat axp813_regulators[] = {
	{ "dcdc1", 0x10, BIT(0), 0x20, 0x1f, 1600, 3400, 100, NA },
	{ "dcdc2", 0x10, BIT(1), 0x21, 0x7f,  500, 1300,  10, 70 },
	{ "dcdc3", 0x10, BIT(2), 0x22, 0x7f,  500, 1300,  10, 70 },
	{ "dcdc4", 0x10, BIT(3), 0x23, 0x7f,  500, 1300,  10, 70 },
	{ "dcdc5", 0x10, BIT(4), 0x24, 0x7f,  800, 1840,  10, 32 },
	{ "dcdc6", 0x10, BIT(5), 0x25, 0x7f,  600, 1520,  10, 50 },
	{ "dcdc7", 0x10, BIT(6), 0x26, 0x7f,  600, 1520,  10, 50 },
	{ "aldo1", 0x13, BIT(5), 0x28, 0x1f,  700, 3300, 100, NA },
	{ "aldo2", 0x13, BIT(6), 0x29, 0x1f,  700, 3300, 100, NA },
	{ "aldo3", 0x13, BIT(7), 0x2a, 0x1f,  700, 3300, 100, NA },
	{ "dldo1", 0x12, BIT(3), 0x15, 0x1f,  700, 3300, 100, NA },
	{ "dldo2", 0x12, BIT(4), 0x16, 0x1f,  700, 4200, 100, 27 },
	{ "dldo3", 0x12, BIT(5), 0x17, 0x1f,  700, 3300, 100, NA },
	{ "dldo4", 0x12, BIT(6), 0x18, 0x1f,  700, 3300, 100, NA },
	{ "eldo1", 0x12, BIT(0), 0x19, 0x1f,  700, 1900,  50, NA },
	{ "eldo2", 0x12, BIT(1), 0x1a, 0x1f,  700, 1900,  50, NA },
	{ "eldo3", 0x12, BIT(2), 0x1b, 0x1f,  700, 1900,  50, NA },
	{ "fldo1", 0x13, BIT(2), 0x1c, 0x0f,  700, 1450,  50, NA },
	{ "fldo2", 0x13, BIT(3), 0x1d, 0x0f,  700, 1450,  50, NA },
	{ "fldo3", 0x13, BIT(4),   NA,   NA,   NA,   NA,  NA, NA },
	{ }
};

static const struct axp_regulator_plat *const axp_regulators[] = {
	[AXP152_ID]	= axp152_regulators,
	[AXP202_ID]	= axp20x_regulators,
	[AXP209_ID]	= axp20x_regulators,
	[AXP221_ID]	= axp22x_regulators,
	[AXP223_ID]	= axp22x_regulators,
	[AXP313_ID]	= axp313_regulators,
	[AXP803_ID]	= axp803_regulators,
	[AXP806_ID]	= axp806_regulators,
	[AXP809_ID]	= axp809_regulators,
	[AXP813_ID]	= axp813_regulators,
};

static int axp_regulator_bind(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_plat = dev_get_uclass_plat(dev);
	ulong id = dev_get_driver_data(dev->parent);
	const struct axp_regulator_plat *plat;

	for (plat = axp_regulators[id]; plat && plat->name; plat++)
		if (!strcmp(plat->name, dev->name))
			break;
	if (!plat || !plat->name)
		return -ENODEV;

	dev_set_plat(dev, (void *)plat);

	if (plat->volt_reg == NA)
		uc_plat->type = REGULATOR_TYPE_FIXED;
	else if (!strncmp(plat->name, "dcdc", strlen("dcdc")))
		uc_plat->type = REGULATOR_TYPE_BUCK;
	else
		uc_plat->type = REGULATOR_TYPE_LDO;

	return 0;
}

U_BOOT_DRIVER(axp_regulator) = {
	.name		= "axp_regulator",
	.id		= UCLASS_REGULATOR,
	.bind		= axp_regulator_bind,
	.ops		= &axp_regulator_ops,
};
