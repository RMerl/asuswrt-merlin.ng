// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 ROHM Semiconductors
 *
 * ROHM BD71837 regulator driver
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <linux/bitops.h>
#include <linux/printk.h>
#include <power/bd71837.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define HW_STATE_CONTROL 0
#define DEBUG

/**
 * struct bd71837_vrange - describe linear range of voltages
 *
 * @min_volt:	smallest voltage in range
 * @step:	how much voltage changes at each selector step
 * @min_sel:	smallest selector in the range
 * @max_sel:	maximum selector in the range
 * @rangeval:	register value used to select this range if selectible
 *		ranges are supported
 */
struct bd71837_vrange {
	unsigned int	min_volt;
	unsigned int	step;
	u8		min_sel;
	u8		max_sel;
	u8		rangeval;
};

/**
 * struct bd71837_plat - describe regulator control registers
 *
 * @name:	name of the regulator. Used for matching the dt-entry
 * @enable_reg:	register address used to enable/disable regulator
 * @enablemask:	register mask used to enable/disable regulator
 * @volt_reg:	register address used to configure regulator voltage
 * @volt_mask:	register mask used to configure regulator voltage
 * @ranges:	pointer to ranges of regulator voltages and matching register
 *		values
 * @numranges:	number of voltage ranges pointed by ranges
 * @rangemask:	mask for selecting used ranges if multiple ranges are supported
 * @sel_mask:	bit to toggle in order to transfer the register control to SW
 * @dvs:	whether the voltage can be changed when regulator is enabled
 */
struct bd71837_plat {
	const char		*name;
	u8			enable_reg;
	u8			enablemask;
	u8			volt_reg;
	u8			volt_mask;
	struct bd71837_vrange	*ranges;
	unsigned int		numranges;
	u8			rangemask;
	u8			sel_mask;
	bool			dvs;
};

#define BD_RANGE(_min, _vstep, _sel_low, _sel_hi, _range_sel) \
{ \
	.min_volt = (_min), .step = (_vstep), .min_sel = (_sel_low), \
	.max_sel = (_sel_hi), .rangeval = (_range_sel) \
}

#define BD_DATA(_name, enreg, enmask, vreg, vmask, _range, rmask, _dvs, sel) \
{ \
	.name = (_name), .enable_reg = (enreg), .enablemask = (enmask), \
	.volt_reg = (vreg), .volt_mask = (vmask), .ranges = (_range), \
	.numranges = ARRAY_SIZE(_range), .rangemask = (rmask), .dvs = (_dvs), \
	.sel_mask = (sel) \
}

static struct bd71837_vrange dvs_buck_vranges[] = {
	BD_RANGE(700000, 10000, 0, 0x3c, 0),
	BD_RANGE(1300000, 0, 0x3d, 0x3f, 0),
};

static struct bd71837_vrange bd71847_buck3_vranges[] = {
	BD_RANGE(700000, 100000, 0x00, 0x03, 0),
	BD_RANGE(1050000, 50000, 0x04, 0x05, 0),
	BD_RANGE(1200000, 150000, 0x06, 0x07, 0),
	BD_RANGE(550000, 50000, 0x0, 0x7, 0x40),
	BD_RANGE(675000, 100000, 0x0, 0x3, 0x80),
	BD_RANGE(1025000, 50000, 0x4, 0x5, 0x80),
	BD_RANGE(1175000, 150000, 0x6, 0x7, 0x80),
};

static struct bd71837_vrange bd71847_buck4_vranges[] = {
	BD_RANGE(3000000, 100000, 0x00, 0x03, 0),
	BD_RANGE(2600000, 100000, 0x00, 0x03, 40),
};

static struct bd71837_vrange bd71837_buck5_vranges[] = {
	BD_RANGE(700000, 100000, 0, 0x3, 0),
	BD_RANGE(1050000, 50000, 0x04, 0x05, 0),
	BD_RANGE(1200000, 150000, 0x06, 0x07, 0),
	BD_RANGE(675000, 100000, 0x0, 0x3, 0x80),
	BD_RANGE(1025000, 50000, 0x04, 0x05, 0x80),
	BD_RANGE(1175000, 150000, 0x06, 0x07, 0x80),
};

static struct bd71837_vrange bd71837_buck6_vranges[] = {
	BD_RANGE(3000000, 100000, 0x00, 0x03, 0),
};

static struct bd71837_vrange nodvs_buck3_vranges[] = {
	BD_RANGE(1605000, 90000, 0, 1, 0),
	BD_RANGE(1755000, 45000, 2, 4, 0),
	BD_RANGE(1905000, 45000, 5, 7, 0),
};

static struct bd71837_vrange nodvs_buck4_vranges[] = {
	BD_RANGE(800000, 10000, 0x00, 0x3C, 0),
};

static struct bd71837_vrange ldo1_vranges[] = {
	BD_RANGE(3000000, 100000, 0x00, 0x03, 0),
	BD_RANGE(1600000, 100000, 0x00, 0x03, 0x20),
};

static struct bd71837_vrange ldo2_vranges[] = {
	BD_RANGE(900000, 0, 0, 0, 0),
	BD_RANGE(800000, 0, 1, 1, 0),
};

static struct bd71837_vrange ldo3_vranges[] = {
	BD_RANGE(1800000, 100000, 0x00, 0x0f, 0),
};

static struct bd71837_vrange ldo4_vranges[] = {
	BD_RANGE(900000, 100000, 0x00, 0x09, 0),
};

static struct bd71837_vrange bd71837_ldo5_vranges[] = {
	BD_RANGE(1800000, 100000, 0x00, 0x0f, 0),
};

static struct bd71837_vrange bd71847_ldo5_vranges[] = {
	BD_RANGE(1800000, 100000, 0x00, 0x0f, 0),
	BD_RANGE(800000, 100000, 0x00, 0x0f, 0x20),
};

static struct bd71837_vrange ldo6_vranges[] = {
	BD_RANGE(900000, 100000, 0x00, 0x09, 0),
};

static struct bd71837_vrange ldo7_vranges[] = {
	BD_RANGE(1800000, 100000, 0x00, 0x0f, 0),
};

/*
 * We use enable mask 'HW_STATE_CONTROL' to indicate that this regulator
 * must not be enabled or disabled by SW. The typical use-case for BD71837
 * is powering NXP i.MX8. In this use-case we (for now) only allow control
 * for BUCK3 and BUCK4 which are not boot critical.
 */
static struct bd71837_plat bd71837_reg_data[] = {
/* Bucks 1-4 which support dynamic voltage scaling */
	BD_DATA("BUCK1", BD718XX_BUCK1_CTRL, HW_STATE_CONTROL,
		BD718XX_BUCK1_VOLT_RUN, DVS_BUCK_RUN_MASK, dvs_buck_vranges, 0,
		true, BD718XX_BUCK_SEL),
	BD_DATA("BUCK2", BD718XX_BUCK2_CTRL, HW_STATE_CONTROL,
		BD718XX_BUCK2_VOLT_RUN, DVS_BUCK_RUN_MASK, dvs_buck_vranges, 0,
		true, BD718XX_BUCK_SEL),
	BD_DATA("BUCK3", BD71837_BUCK3_CTRL, BD718XX_BUCK_EN,
		BD71837_BUCK3_VOLT_RUN, DVS_BUCK_RUN_MASK, dvs_buck_vranges, 0,
		true, BD718XX_BUCK_SEL),
	BD_DATA("BUCK4", BD71837_BUCK4_CTRL, BD718XX_BUCK_EN,
		BD71837_BUCK4_VOLT_RUN, DVS_BUCK_RUN_MASK, dvs_buck_vranges, 0,
		true, BD718XX_BUCK_SEL),
/* Bucks 5-8 which do not support dynamic voltage scaling */
	BD_DATA("BUCK5", BD718XX_1ST_NODVS_BUCK_CTRL, HW_STATE_CONTROL,
		BD718XX_1ST_NODVS_BUCK_VOLT, BD718XX_1ST_NODVS_BUCK_MASK,
		bd71837_buck5_vranges, 0x80, false, BD718XX_BUCK_SEL),
	BD_DATA("BUCK6", BD718XX_2ND_NODVS_BUCK_CTRL, HW_STATE_CONTROL,
		BD718XX_2ND_NODVS_BUCK_VOLT, BD71837_BUCK6_MASK,
		bd71837_buck6_vranges, 0, false, BD718XX_BUCK_SEL),
	BD_DATA("BUCK7", BD718XX_3RD_NODVS_BUCK_CTRL, HW_STATE_CONTROL,
		BD718XX_3RD_NODVS_BUCK_VOLT, BD718XX_3RD_NODVS_BUCK_MASK,
		nodvs_buck3_vranges, 0, false, BD718XX_BUCK_SEL),
	BD_DATA("BUCK8", BD718XX_4TH_NODVS_BUCK_CTRL, HW_STATE_CONTROL,
		BD718XX_4TH_NODVS_BUCK_VOLT, BD718XX_4TH_NODVS_BUCK_MASK,
		nodvs_buck4_vranges, 0, false, BD718XX_BUCK_SEL),
/* LDOs */
	BD_DATA("LDO1", BD718XX_LDO1_VOLT, HW_STATE_CONTROL, BD718XX_LDO1_VOLT,
		BD718XX_LDO1_MASK, ldo1_vranges, 0x20, false, BD718XX_LDO_SEL),
	BD_DATA("LDO2", BD718XX_LDO2_VOLT, HW_STATE_CONTROL, BD718XX_LDO2_VOLT,
		BD718XX_LDO2_MASK, ldo2_vranges, 0, false, BD718XX_LDO_SEL),
	BD_DATA("LDO3", BD718XX_LDO3_VOLT, HW_STATE_CONTROL, BD718XX_LDO3_VOLT,
		BD718XX_LDO3_MASK, ldo3_vranges, 0, false, BD718XX_LDO_SEL),
	BD_DATA("LDO4", BD718XX_LDO4_VOLT, HW_STATE_CONTROL, BD718XX_LDO4_VOLT,
		BD718XX_LDO4_MASK, ldo4_vranges, 0, false, BD718XX_LDO_SEL),
	BD_DATA("LDO5", BD718XX_LDO5_VOLT, HW_STATE_CONTROL, BD718XX_LDO5_VOLT,
		BD71837_LDO5_MASK, bd71837_ldo5_vranges, 0, false,
		BD718XX_LDO_SEL),
	BD_DATA("LDO6", BD718XX_LDO6_VOLT, HW_STATE_CONTROL, BD718XX_LDO6_VOLT,
		BD718XX_LDO6_MASK, ldo6_vranges, 0, false, BD718XX_LDO_SEL),
	BD_DATA("LDO7", BD71837_LDO7_VOLT, HW_STATE_CONTROL, BD71837_LDO7_VOLT,
		BD71837_LDO7_MASK, ldo7_vranges, 0, false, BD718XX_LDO_SEL),
};

static struct bd71837_plat bd71847_reg_data[] = {
/* Bucks 1 and 2 which support dynamic voltage scaling */
	BD_DATA("BUCK1", BD718XX_BUCK1_CTRL, HW_STATE_CONTROL,
		BD718XX_BUCK1_VOLT_RUN, DVS_BUCK_RUN_MASK, dvs_buck_vranges, 0,
		true, BD718XX_BUCK_SEL),
	BD_DATA("BUCK2", BD718XX_BUCK2_CTRL, HW_STATE_CONTROL,
		BD718XX_BUCK2_VOLT_RUN, DVS_BUCK_RUN_MASK, dvs_buck_vranges, 0,
		true, BD718XX_BUCK_SEL),
/* Bucks 3-6 which do not support dynamic voltage scaling */
	BD_DATA("BUCK3", BD718XX_1ST_NODVS_BUCK_CTRL, HW_STATE_CONTROL,
		BD718XX_1ST_NODVS_BUCK_VOLT, BD718XX_1ST_NODVS_BUCK_MASK,
		bd71847_buck3_vranges, 0xc0, false, BD718XX_BUCK_SEL),
	BD_DATA("BUCK4", BD718XX_2ND_NODVS_BUCK_CTRL, HW_STATE_CONTROL,
		BD718XX_2ND_NODVS_BUCK_VOLT, BD71837_BUCK6_MASK,
		bd71847_buck4_vranges, 0x40, false, BD718XX_BUCK_SEL),
	BD_DATA("BUCK5", BD718XX_3RD_NODVS_BUCK_CTRL, HW_STATE_CONTROL,
		BD718XX_3RD_NODVS_BUCK_VOLT, BD718XX_3RD_NODVS_BUCK_MASK,
		nodvs_buck3_vranges, 0, false, BD718XX_BUCK_SEL),
	BD_DATA("BUCK6", BD718XX_4TH_NODVS_BUCK_CTRL, HW_STATE_CONTROL,
		BD718XX_4TH_NODVS_BUCK_VOLT, BD718XX_4TH_NODVS_BUCK_MASK,
		nodvs_buck4_vranges, 0, false, BD718XX_BUCK_SEL),
/* LDOs */
	BD_DATA("LDO1", BD718XX_LDO1_VOLT, HW_STATE_CONTROL, BD718XX_LDO1_VOLT,
		BD718XX_LDO1_MASK, ldo1_vranges, 0x20, false, BD718XX_LDO_SEL),
	BD_DATA("LDO2", BD718XX_LDO2_VOLT, HW_STATE_CONTROL, BD718XX_LDO2_VOLT,
		BD718XX_LDO2_MASK, ldo2_vranges, 0, false, BD718XX_LDO_SEL),
	BD_DATA("LDO3", BD718XX_LDO3_VOLT, HW_STATE_CONTROL, BD718XX_LDO3_VOLT,
		BD718XX_LDO3_MASK, ldo3_vranges, 0, false, BD718XX_LDO_SEL),
	BD_DATA("LDO4", BD718XX_LDO4_VOLT, HW_STATE_CONTROL, BD718XX_LDO4_VOLT,
		BD718XX_LDO4_MASK, ldo4_vranges, 0, false, BD718XX_LDO_SEL),
	BD_DATA("LDO5", BD718XX_LDO5_VOLT, HW_STATE_CONTROL, BD718XX_LDO5_VOLT,
		BD71847_LDO5_MASK, bd71847_ldo5_vranges, 0x20, false,
		BD718XX_LDO_SEL),
	BD_DATA("LDO6", BD718XX_LDO6_VOLT, HW_STATE_CONTROL, BD718XX_LDO6_VOLT,
		BD718XX_LDO6_MASK, ldo6_vranges, 0, false, BD718XX_LDO_SEL),
};

static int vrange_find_value(struct bd71837_vrange *r, unsigned int sel,
			     unsigned int *val)
{
	if (!val || sel < r->min_sel || sel > r->max_sel)
		return -EINVAL;

	*val = r->min_volt + r->step * (sel - r->min_sel);
	return 0;
}

static int vrange_find_selector(struct bd71837_vrange *r, int val,
				unsigned int *sel)
{
	int ret = -EINVAL;
	int num_vals = r->max_sel - r->min_sel + 1;

	if (val >= r->min_volt &&
	    val <= r->min_volt + r->step * (num_vals - 1)) {
		if (r->step) {
			*sel = r->min_sel + ((val - r->min_volt) / r->step);
			ret = 0;
		} else {
			*sel = r->min_sel;
			ret = 0;
		}
	}
	return ret;
}

static int bd71837_get_enable(struct udevice *dev)
{
	int val;
	struct bd71837_plat *plat = dev_get_plat(dev);

	/*
	 * boot critical regulators on bd71837 must not be controlled by sw
	 * due to the 'feature' which leaves power rails down if bd71837 is
	 * reseted to snvs state. hence we can't get the state here.
	 *
	 * if we are alive it means we probably are on run state and
	 * if the regulator can't be controlled we can assume it is
	 * enabled.
	 */
	if (plat->enablemask == HW_STATE_CONTROL)
		return 1;

	val = pmic_reg_read(dev->parent, plat->enable_reg);
	if (val < 0)
		return val;

	return (val & plat->enablemask);
}

static int bd71837_set_enable(struct udevice *dev, bool enable)
{
	int val = 0;
	struct bd71837_plat *plat = dev_get_plat(dev);

	/*
	 * boot critical regulators on bd71837 must not be controlled by sw
	 * due to the 'feature' which leaves power rails down if bd71837 is
	 * reseted to snvs state. Hence we can't set the state here.
	 */
	if (plat->enablemask == HW_STATE_CONTROL)
		return enable ? 0 : -EINVAL;

	if (enable)
		val = plat->enablemask;

	return pmic_clrsetbits(dev->parent, plat->enable_reg, plat->enablemask,
			       val);
}

static int bd71837_get_value(struct udevice *dev)
{
	unsigned int reg, range;
	unsigned int tmp;
	struct bd71837_plat *plat = dev_get_plat(dev);
	int i;

	reg = pmic_reg_read(dev->parent, plat->volt_reg);
	if (((int)reg) < 0)
		return reg;

	range = reg & plat->rangemask;

	reg &= plat->volt_mask;
	reg >>= ffs(plat->volt_mask) - 1;

	for (i = 0; i < plat->numranges; i++) {
		struct bd71837_vrange *r = &plat->ranges[i];

		if (plat->rangemask && ((plat->rangemask & range) !=
		    r->rangeval))
			continue;

		if (!vrange_find_value(r, reg, &tmp))
			return tmp;
	}

	pr_err("Unknown voltage value read from pmic\n");

	return -EINVAL;
}

static int bd71837_set_value(struct udevice *dev, int uvolt)
{
	unsigned int sel;
	unsigned int range;
	int i;
	int found = 0;
	struct bd71837_plat *plat = dev_get_plat(dev);

	/*
	 * An under/overshooting may occur if voltage is changed for other
	 * regulators but buck 1,2,3 or 4 when regulator is enabled. Prevent
	 * change to protect the HW
	 */
	if (!plat->dvs)
		if (bd71837_get_enable(dev)) {
			/* If the value is already set, skip the warning. */
			if (bd71837_get_value(dev) == uvolt)
				return 0;
			pr_err("Only DVS bucks can be changed when enabled\n");
			return -EINVAL;
		}

	for (i = 0; i < plat->numranges; i++) {
		struct bd71837_vrange *r = &plat->ranges[i];

		found = !vrange_find_selector(r, uvolt, &sel);
		if (found) {
			unsigned int tmp;

			/*
			 * We require exactly the requested value to be
			 * supported - this can be changed later if needed
			 */
			range = r->rangeval;
			found = !vrange_find_value(r, sel, &tmp);
			if (found && tmp == uvolt)
				break;
			found = 0;
		}
	}

	if (!found)
		return -EINVAL;

	sel <<= ffs(plat->volt_mask) - 1;

	if (plat->rangemask)
		sel |= range;

	return pmic_clrsetbits(dev->parent, plat->volt_reg, plat->volt_mask |
			       plat->rangemask, sel);
}

static int bd71837_regulator_probe(struct udevice *dev)
{
	struct bd71837_plat *plat = dev_get_plat(dev);
	int i, ret;
	struct dm_regulator_uclass_plat *uc_pdata;
	int type;
	struct bd71837_plat *init_data;
	int data_amnt;

	type = dev_get_driver_data(dev_get_parent(dev));

	switch (type) {
	case ROHM_CHIP_TYPE_BD71837:
		init_data = bd71837_reg_data;
		data_amnt = ARRAY_SIZE(bd71837_reg_data);
		break;
	case ROHM_CHIP_TYPE_BD71847:
		init_data = bd71847_reg_data;
		data_amnt = ARRAY_SIZE(bd71847_reg_data);
		break;
	default:
		debug("Unknown PMIC type\n");
		init_data = NULL;
		data_amnt = 0;
		break;
	}

	for (i = 0; i < data_amnt; i++) {
		if (!strcmp(dev->name, init_data[i].name)) {
			*plat = init_data[i];
			if (plat->enablemask != HW_STATE_CONTROL) {
				/*
				 * Take the regulator under SW control. Ensure
				 * the initial state matches dt flags and then
				 * write the SEL bit
				 */
				uc_pdata = dev_get_uclass_plat(dev);
				ret = bd71837_set_enable(dev,
							 !!(uc_pdata->boot_on ||
							 uc_pdata->always_on));
				if (ret)
					return ret;

				return pmic_clrsetbits(dev->parent,
						      plat->enable_reg,
						      plat->sel_mask,
						      plat->sel_mask);
			}
			return 0;
		}
	}

	pr_err("Unknown regulator '%s'\n", dev->name);

	return -ENOENT;
}

static const struct dm_regulator_ops bd71837_regulator_ops = {
	.get_value  = bd71837_get_value,
	.set_value  = bd71837_set_value,
	.get_enable = bd71837_get_enable,
	.set_enable = bd71837_set_enable,
};

U_BOOT_DRIVER(bd71837_regulator) = {
	.name = BD718XX_REGULATOR_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &bd71837_regulator_ops,
	.probe = bd71837_regulator_probe,
	.plat_auto	= sizeof(struct bd71837_plat),
};
