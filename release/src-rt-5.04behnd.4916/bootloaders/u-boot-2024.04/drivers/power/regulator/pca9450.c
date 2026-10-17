// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * NXP PCA9450 regulator driver
 * Copyright (C) 2022 Marek Vasut <marex@denx.de>
 *
 * Largely based on:
 * ROHM BD71837 regulator driver
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <linux/bitops.h>
#include <power/pca9450.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define HW_STATE_CONTROL 0
#define DEBUG

/**
 * struct pca9450_vrange - describe linear range of voltages
 *
 * @min_volt:	smallest voltage in range
 * @step:	how much voltage changes at each selector step
 * @min_sel:	smallest selector in the range
 * @max_sel:	maximum selector in the range
 */
struct pca9450_vrange {
	unsigned int	min_volt;
	unsigned int	step;
	u8		min_sel;
	u8		max_sel;
};

/**
 * struct pca9450_plat - describe regulator control registers
 *
 * @name:	name of the regulator. Used for matching the dt-entry
 * @enable_reg:	register address used to enable/disable regulator
 * @enablemask:	register mask used to enable/disable regulator
 * @volt_reg:	register address used to configure regulator voltage
 * @volt_mask:	register mask used to configure regulator voltage
 * @ranges:	pointer to ranges of regulator voltages and matching register
 *		values
 * @numranges:	number of voltage ranges pointed by ranges
 */
struct pca9450_plat {
	const char		*name;
	u8			enable_reg;
	u8			enablemask;
	u8			volt_reg;
	u8			volt_mask;
	struct pca9450_vrange	*ranges;
	unsigned int		numranges;
};

#define PCA_RANGE(_min, _vstep, _sel_low, _sel_hi) \
{ \
	.min_volt = (_min), .step = (_vstep), \
	.min_sel = (_sel_low), .max_sel = (_sel_hi), \
}

#define PCA_DATA(_name, enreg, enmask, vreg, vmask, _range) \
{ \
	.name = (_name), .enable_reg = (enreg), .enablemask = (enmask), \
	.volt_reg = (vreg), .volt_mask = (vmask), .ranges = (_range), \
	.numranges = ARRAY_SIZE(_range) \
}

static struct pca9450_vrange pca9450_buck123_vranges[] = {
	PCA_RANGE(600000, 12500, 0, 0x7f),
};

static struct pca9450_vrange pca9450_buck456_vranges[] = {
	PCA_RANGE(600000, 25000, 0, 0x70),
	PCA_RANGE(3400000, 0, 0x71, 0x7f),
};

static struct pca9450_vrange pca9450_ldo1_vranges[] = {
	PCA_RANGE(1600000, 100000, 0x0, 0x3),
	PCA_RANGE(3000000, 100000, 0x4, 0x7),
};

static struct pca9450_vrange pca9450_ldo2_vranges[] = {
	PCA_RANGE(800000, 50000, 0x0, 0x7),
};

static struct pca9450_vrange pca9450_ldo34_vranges[] = {
	PCA_RANGE(800000, 100000, 0x0, 0x19),
	PCA_RANGE(3300000, 0, 0x1a, 0x1f),
};

static struct pca9450_vrange pca9450_ldo5_vranges[] = {
	PCA_RANGE(1800000, 100000, 0x0, 0xf),
};

/*
 * We use enable mask 'HW_STATE_CONTROL' to indicate that this regulator
 * must not be enabled or disabled by SW. The typical use-case for PCA9450
 * is powering NXP i.MX8. In this use-case we (for now) only allow control
 * for BUCK4, BUCK5, BUCK6 which are not boot critical.
 */
static struct pca9450_plat pca9450_reg_data[] = {
	/* Bucks 1-3 which support dynamic voltage scaling */
	PCA_DATA("BUCK1", PCA9450_BUCK1CTRL, HW_STATE_CONTROL,
		 PCA9450_BUCK1OUT_DVS0, PCA9450_DVS_BUCK_RUN_MASK,
		 pca9450_buck123_vranges),
	PCA_DATA("BUCK2", PCA9450_BUCK2CTRL, HW_STATE_CONTROL,
		 PCA9450_BUCK2OUT_DVS0, PCA9450_DVS_BUCK_RUN_MASK,
		 pca9450_buck123_vranges),
	PCA_DATA("BUCK3", PCA9450_BUCK3CTRL, HW_STATE_CONTROL,
		 PCA9450_BUCK3OUT_DVS0, PCA9450_DVS_BUCK_RUN_MASK,
		 pca9450_buck123_vranges),
	/* Bucks 4-6 which do not support dynamic voltage scaling */
	PCA_DATA("BUCK4", PCA9450_BUCK4CTRL, HW_STATE_CONTROL,
		 PCA9450_BUCK4OUT, PCA9450_DVS_BUCK_RUN_MASK,
		 pca9450_buck456_vranges),
	PCA_DATA("BUCK5", PCA9450_BUCK5CTRL, HW_STATE_CONTROL,
		 PCA9450_BUCK5OUT, PCA9450_DVS_BUCK_RUN_MASK,
		 pca9450_buck456_vranges),
	PCA_DATA("BUCK6", PCA9450_BUCK6CTRL, HW_STATE_CONTROL,
		 PCA9450_BUCK6OUT, PCA9450_DVS_BUCK_RUN_MASK,
		 pca9450_buck456_vranges),
	/* LDOs */
	PCA_DATA("LDO1", PCA9450_LDO1CTRL, HW_STATE_CONTROL,
		 PCA9450_LDO1CTRL, PCA9450_LDO12_MASK,
		 pca9450_ldo1_vranges),
	PCA_DATA("LDO2", PCA9450_LDO2CTRL, HW_STATE_CONTROL,
		 PCA9450_LDO2CTRL, PCA9450_LDO12_MASK,
		 pca9450_ldo2_vranges),
	PCA_DATA("LDO3", PCA9450_LDO3CTRL, HW_STATE_CONTROL,
		 PCA9450_LDO3CTRL, PCA9450_LDO34_MASK,
		 pca9450_ldo34_vranges),
	PCA_DATA("LDO4", PCA9450_LDO4CTRL, HW_STATE_CONTROL,
		 PCA9450_LDO4CTRL, PCA9450_LDO34_MASK,
		 pca9450_ldo34_vranges),
	PCA_DATA("LDO5", PCA9450_LDO5CTRL_H, HW_STATE_CONTROL,
		 PCA9450_LDO5CTRL_H, PCA9450_LDO5_MASK,
		 pca9450_ldo5_vranges),
};

static int vrange_find_value(struct pca9450_vrange *r, unsigned int sel,
			     unsigned int *val)
{
	if (!val || sel < r->min_sel || sel > r->max_sel)
		return -EINVAL;

	*val = r->min_volt + r->step * (sel - r->min_sel);
	return 0;
}

static int vrange_find_selector(struct pca9450_vrange *r, int val,
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

static int pca9450_get_enable(struct udevice *dev)
{
	struct pca9450_plat *plat = dev_get_plat(dev);
	int val;

	/*
	 * boot critical regulators on pca9450 must not be controlled by sw
	 * due to the 'feature' which leaves power rails down if pca9450 is
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

static int pca9450_set_enable(struct udevice *dev, bool enable)
{
	int val = 0;
	struct pca9450_plat *plat = dev_get_plat(dev);

	/*
	 * boot critical regulators on pca9450 must not be controlled by sw
	 * due to the 'feature' which leaves power rails down if pca9450 is
	 * reseted to snvs state. Hence we can't set the state here.
	 */
	if (plat->enablemask == HW_STATE_CONTROL)
		return enable ? 0 : -EINVAL;

	if (enable)
		val = plat->enablemask;

	return pmic_clrsetbits(dev->parent, plat->enable_reg, plat->enablemask,
			       val);
}

static int pca9450_get_value(struct udevice *dev)
{
	struct pca9450_plat *plat = dev_get_plat(dev);
	unsigned int reg, tmp;
	int i, ret;

	ret = pmic_reg_read(dev->parent, plat->volt_reg);
	if (ret < 0)
		return ret;

	reg = ret;
	reg &= plat->volt_mask;

	for (i = 0; i < plat->numranges; i++) {
		struct pca9450_vrange *r = &plat->ranges[i];

		if (!vrange_find_value(r, reg, &tmp))
			return tmp;
	}

	pr_err("Unknown voltage value read from pmic\n");

	return -EINVAL;
}

static int pca9450_set_value(struct udevice *dev, int uvolt)
{
	struct pca9450_plat *plat = dev_get_plat(dev);
	unsigned int sel;
	int i, found = 0;

	for (i = 0; i < plat->numranges; i++) {
		struct pca9450_vrange *r = &plat->ranges[i];

		found = !vrange_find_selector(r, uvolt, &sel);
		if (found) {
			unsigned int tmp;

			/*
			 * We require exactly the requested value to be
			 * supported - this can be changed later if needed
			 */
			found = !vrange_find_value(r, sel, &tmp);
			if (found && tmp == uvolt)
				break;
			found = 0;
		}
	}

	if (!found)
		return -EINVAL;

	return pmic_clrsetbits(dev->parent, plat->volt_reg,
			       plat->volt_mask, sel);
}

static int pca9450_regulator_probe(struct udevice *dev)
{
	struct pca9450_plat *plat = dev_get_plat(dev);
	int i, type;

	type = dev_get_driver_data(dev_get_parent(dev));

	if (type != NXP_CHIP_TYPE_PCA9450A && type != NXP_CHIP_TYPE_PCA9450BC &&
	    type != NXP_CHIP_TYPE_PCA9451A) {
		debug("Unknown PMIC type\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(pca9450_reg_data); i++) {
		if (strcmp(dev->name, pca9450_reg_data[i].name))
			continue;

		/* PCA9450B/PCA9450C uses BUCK1 and BUCK3 in dual-phase */
		if (type == NXP_CHIP_TYPE_PCA9450BC &&
		    !strcmp(pca9450_reg_data[i].name, "BUCK3")) {
			continue;
		}

		/* PCA9451A uses BUCK3 in dual-phase and don't have LDO2 and LDO3 */
		if (type == NXP_CHIP_TYPE_PCA9451A &&
		    (!strcmp(pca9450_reg_data[i].name, "BUCK3") ||
		    !strcmp(pca9450_reg_data[i].name, "LDO2") ||
		    !strcmp(pca9450_reg_data[i].name, "LDO3"))) {
			continue;
		}

		*plat = pca9450_reg_data[i];

		return 0;
	}

	pr_err("Unknown regulator '%s'\n", dev->name);

	return -ENOENT;
}

static const struct dm_regulator_ops pca9450_regulator_ops = {
	.get_value	= pca9450_get_value,
	.set_value	= pca9450_set_value,
	.get_enable	= pca9450_get_enable,
	.set_enable	= pca9450_set_enable,
};

U_BOOT_DRIVER(pca9450_regulator) = {
	.name		= PCA9450_REGULATOR_DRIVER,
	.id		= UCLASS_REGULATOR,
	.ops		= &pca9450_regulator_ops,
	.probe		= pca9450_regulator_probe,
	.plat_auto	= sizeof(struct pca9450_plat),
};
