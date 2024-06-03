// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) EETS GmbH, 2017, Felix Brack <f.brack@eets.ch>
 */

#include <common.h>
#include <dm.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps65910_pmic.h>

#define VOUT_CHOICE_COUNT 4

/*
 * struct regulator_props - Properties of a LDO and VIO SMPS regulator
 *
 * All of these regulators allow setting one out of four output voltages.
 * These output voltages are only achievable when supplying the regulator
 * with a minimum input voltage.
 *
 * @vin_min[]: minimum supply input voltage in uV required to achieve the
 *             corresponding vout[] voltage
 * @vout[]:    regulator output voltage in uV
 * @reg:       I2C register used to set regulator voltage
 */
struct regulator_props {
	int vin_min[VOUT_CHOICE_COUNT];
	int vout[VOUT_CHOICE_COUNT];
	int reg;
};

static const struct regulator_props ldo_props_vdig1 = {
	.vin_min = { 1700000, 2100000, 2700000, 3200000 },
	.vout = { 1200000, 1500000, 1800000, 2700000 },
	.reg = TPS65910_REG_VDIG1
};

static const struct regulator_props ldo_props_vdig2 = {
	.vin_min = { 1700000, 1700000, 1700000, 2700000 },
	.vout = { 1000000, 1100000, 1200000, 1800000 },
	.reg = TPS65910_REG_VDIG2
};

static const struct regulator_props ldo_props_vpll = {
	.vin_min = { 2700000, 2700000, 2700000, 3000000 },
	.vout = { 1000000, 1100000, 1800000, 2500000 },
	.reg = TPS65910_REG_VPLL
};

static const struct regulator_props ldo_props_vdac = {
	.vin_min = { 2700000, 3000000, 3200000, 3200000 },
	.vout = { 1800000, 2600000, 2800000, 2850000 },
	.reg = TPS65910_REG_VDAC
};

static const struct regulator_props ldo_props_vaux1 = {
	.vin_min = { 2700000, 3200000, 3200000, 3200000 },
	.vout = { 1800000, 2500000, 2800000, 2850000 },
	.reg = TPS65910_REG_VAUX1
};

static const struct regulator_props ldo_props_vaux2 = {
	.vin_min = { 2700000, 3200000, 3200000, 3600000 },
	.vout = { 1800000, 2800000, 2900000, 3300000 },
	.reg = TPS65910_REG_VAUX2
};

static const struct regulator_props ldo_props_vaux33 = {
	.vin_min = { 2700000, 2700000, 3200000, 3600000 },
	.vout = { 1800000, 2000000, 2800000, 3300000 },
	.reg = TPS65910_REG_VAUX33
};

static const struct regulator_props ldo_props_vmmc = {
	.vin_min = { 2700000, 3200000, 3200000, 3600000 },
	.vout = { 1800000, 2800000, 3000000, 3300000 },
	.reg = TPS65910_REG_VMMC
};

static const struct regulator_props smps_props_vio = {
	.vin_min = { 3200000, 3200000, 4000000, 4400000 },
	.vout = { 1500000, 1800000, 2500000, 3300000 },
	.reg = TPS65910_REG_VIO
};

/* lookup table of control registers indexed by regulator unit number */
static const int ctrl_regs[] = {
	TPS65910_REG_VRTC,
	TPS65910_REG_VIO,
	TPS65910_REG_VDD1,
	TPS65910_REG_VDD2,
	TPS65910_REG_VDD3,
	TPS65910_REG_VDIG1,
	TPS65910_REG_VDIG2,
	TPS65910_REG_VPLL,
	TPS65910_REG_VDAC,
	TPS65910_REG_VAUX1,
	TPS65910_REG_VAUX2,
	TPS65910_REG_VAUX33,
	TPS65910_REG_VMMC
};

/* supply names as used in DT */
static const char * const supply_names[] = {
	"vccio-supply",
	"vcc1-supply",
	"vcc2-supply",
	"vcc3-supply",
	"vcc4-supply",
	"vcc5-supply",
	"vcc6-supply",
	"vcc7-supply"
};

/* lookup table of regulator supplies indexed by regulator unit number */
static const int regulator_supplies[] = {
	TPS65910_SUPPLY_VCC7,
	TPS65910_SUPPLY_VCCIO,
	TPS65910_SUPPLY_VCC1,
	TPS65910_SUPPLY_VCC2,
	TPS65910_SUPPLY_VCC7,
	TPS65910_SUPPLY_VCC6,
	TPS65910_SUPPLY_VCC6,
	TPS65910_SUPPLY_VCC5,
	TPS65910_SUPPLY_VCC5,
	TPS65910_SUPPLY_VCC4,
	TPS65910_SUPPLY_VCC4,
	TPS65910_SUPPLY_VCC3,
	TPS65910_SUPPLY_VCC3
};

static int get_ctrl_reg_from_unit_addr(const uint unit_addr)
{
	if (unit_addr < ARRAY_SIZE(ctrl_regs))
		return ctrl_regs[unit_addr];
	return -ENXIO;
}

static int tps65910_regulator_get_value(struct udevice *dev,
					const struct regulator_props *rgp)
{
	int sel, val, vout;
	struct tps65910_regulator_pdata *pdata = dev_get_platdata(dev);
	int vin = pdata->supply;

	val = pmic_reg_read(dev->parent, rgp->reg);
	if (val < 0)
		return val;
	sel = (val & TPS65910_SEL_MASK) >> 2;
	vout = (vin >= *(rgp->vin_min + sel)) ? *(rgp->vout + sel) : 0;
	vout = ((val & TPS65910_SUPPLY_STATE_MASK) == 1) ? vout : 0;

	return vout;
}

static int tps65910_ldo_get_value(struct udevice *dev)
{
	struct tps65910_regulator_pdata *pdata = dev_get_platdata(dev);
	int vin;

	if (!pdata)
		return 0;
	vin = pdata->supply;

	switch (pdata->unit) {
	case TPS65910_UNIT_VRTC:
		/* VRTC is fixed and can't be turned off */
		return (vin >= 2500000) ? 1830000 : 0;
	case TPS65910_UNIT_VDIG1:
		return tps65910_regulator_get_value(dev, &ldo_props_vdig1);
	case TPS65910_UNIT_VDIG2:
		return tps65910_regulator_get_value(dev, &ldo_props_vdig2);
	case TPS65910_UNIT_VPLL:
		return tps65910_regulator_get_value(dev, &ldo_props_vpll);
	case TPS65910_UNIT_VDAC:
		return tps65910_regulator_get_value(dev, &ldo_props_vdac);
	case TPS65910_UNIT_VAUX1:
		return tps65910_regulator_get_value(dev, &ldo_props_vaux1);
	case TPS65910_UNIT_VAUX2:
		return tps65910_regulator_get_value(dev, &ldo_props_vaux2);
	case TPS65910_UNIT_VAUX33:
		return tps65910_regulator_get_value(dev, &ldo_props_vaux33);
	case TPS65910_UNIT_VMMC:
		return tps65910_regulator_get_value(dev, &ldo_props_vmmc);
	default:
		return 0;
	}
}

static int tps65910_regulator_set_value(struct udevice *dev,
					const struct regulator_props *ldo,
					int uV)
{
	int val;
	int sel = 0;
	struct tps65910_regulator_pdata *pdata = dev_get_platdata(dev);

	do {
		/* we only allow exact voltage matches */
		if (uV == *(ldo->vout + sel))
			break;
	} while (++sel < VOUT_CHOICE_COUNT);
	if (sel == VOUT_CHOICE_COUNT)
		return -EINVAL;
	if (pdata->supply < *(ldo->vin_min + sel))
		return -EINVAL;

	val = pmic_reg_read(dev->parent, ldo->reg);
	if (val < 0)
		return val;
	val &= ~TPS65910_SEL_MASK;
	val |= sel << 2;
	return pmic_reg_write(dev->parent, ldo->reg, val);
}

static int tps65910_ldo_set_value(struct udevice *dev, int uV)
{
	struct tps65910_regulator_pdata *pdata = dev_get_platdata(dev);
	int vin = pdata->supply;

	switch (pdata->unit) {
	case TPS65910_UNIT_VRTC:
		/* VRTC is fixed to 1.83V and can't be turned off */
		if (vin < 2500000)
			return -EINVAL;
		return 0;
	case TPS65910_UNIT_VDIG1:
		return tps65910_regulator_set_value(dev, &ldo_props_vdig1, uV);
	case TPS65910_UNIT_VDIG2:
		return tps65910_regulator_set_value(dev, &ldo_props_vdig2, uV);
	case TPS65910_UNIT_VPLL:
		return tps65910_regulator_set_value(dev, &ldo_props_vpll, uV);
	case TPS65910_UNIT_VDAC:
		return tps65910_regulator_set_value(dev, &ldo_props_vdac, uV);
	case TPS65910_UNIT_VAUX1:
		return tps65910_regulator_set_value(dev, &ldo_props_vaux1, uV);
	case TPS65910_UNIT_VAUX2:
		return tps65910_regulator_set_value(dev, &ldo_props_vaux2, uV);
	case TPS65910_UNIT_VAUX33:
		return tps65910_regulator_set_value(dev, &ldo_props_vaux33, uV);
	case TPS65910_UNIT_VMMC:
		return tps65910_regulator_set_value(dev, &ldo_props_vmmc, uV);
	default:
		return 0;
	}
}

static int tps65910_get_enable(struct udevice *dev)
{
	int reg, val;
	struct tps65910_regulator_pdata *pdata = dev_get_platdata(dev);

	reg = get_ctrl_reg_from_unit_addr(pdata->unit);
	if (reg < 0)
		return reg;

	val = pmic_reg_read(dev->parent, reg);
	if (val < 0)
		return val;

	/* bits 1:0 of regulator control register define state */
	return ((val & TPS65910_SUPPLY_STATE_MASK) == 1);
}

static int tps65910_set_enable(struct udevice *dev, bool enable)
{
	int reg;
	uint clr, set;
	struct tps65910_regulator_pdata *pdata = dev_get_platdata(dev);

	reg = get_ctrl_reg_from_unit_addr(pdata->unit);
	if (reg < 0)
		return reg;

	if (enable) {
		clr = TPS65910_SUPPLY_STATE_MASK & ~TPS65910_SUPPLY_STATE_ON;
		set = TPS65910_SUPPLY_STATE_MASK & TPS65910_SUPPLY_STATE_ON;
	} else {
		clr = TPS65910_SUPPLY_STATE_MASK & ~TPS65910_SUPPLY_STATE_OFF;
		set = TPS65910_SUPPLY_STATE_MASK & TPS65910_SUPPLY_STATE_OFF;
	}
	return pmic_clrsetbits(dev->parent, reg, clr, set);
}

static int buck_get_vdd1_vdd2_value(struct udevice *dev, int reg_vdd)
{
	int gain;
	int val = pmic_reg_read(dev, reg_vdd);

	if (val < 0)
		return val;
	gain = (val & TPS65910_GAIN_SEL_MASK) >> 6;
	gain = (gain == 0) ? 1 : gain;
	val = pmic_reg_read(dev, reg_vdd + 1);
	if (val < 0)
		return val;
	if (val & TPS65910_VDD_SR_MASK)
		/* use smart reflex value instead */
		val = pmic_reg_read(dev, reg_vdd + 2);
	if (val < 0)
		return val;
	return (562500 + (val & TPS65910_VDD_SEL_MASK) * 12500) * gain;
}

static int tps65910_buck_get_value(struct udevice *dev)
{
	struct tps65910_regulator_pdata *pdata = dev_get_platdata(dev);

	switch (pdata->unit) {
	case TPS65910_UNIT_VIO:
		return tps65910_regulator_get_value(dev, &smps_props_vio);
	case TPS65910_UNIT_VDD1:
		return buck_get_vdd1_vdd2_value(dev->parent, TPS65910_REG_VDD1);
	case TPS65910_UNIT_VDD2:
		return buck_get_vdd1_vdd2_value(dev->parent, TPS65910_REG_VDD2);
	default:
		return 0;
	}
}

static int buck_set_vdd1_vdd2_value(struct udevice *dev, int uV)
{
	int ret, reg_vdd, gain;
	int val;
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct tps65910_regulator_pdata *pdata = dev_get_platdata(dev);

	switch (pdata->unit) {
	case TPS65910_UNIT_VDD1:
		reg_vdd = TPS65910_REG_VDD1;
		break;
	case TPS65910_UNIT_VDD2:
		reg_vdd = TPS65910_REG_VDD2;
		break;
	default:
		return -EINVAL;
	}
	uc_pdata = dev_get_uclass_platdata(dev);

	/* check setpoint is within limits */
	if (uV < uc_pdata->min_uV) {
		pr_err("voltage %duV for %s too low\n", uV, dev->name);
		return -EINVAL;
	}
	if (uV > uc_pdata->max_uV) {
		pr_err("voltage %duV for %s too high\n", uV, dev->name);
		return -EINVAL;
	}

	val = pmic_reg_read(dev->parent, reg_vdd);
	if (val < 0)
		return val;
	gain = (val & TPS65910_GAIN_SEL_MASK) >> 6;
	gain = (gain == 0) ? 1 : gain;
	val = ((uV / gain) - 562500) / 12500;
	if (val < TPS65910_VDD_SEL_MIN || val > TPS65910_VDD_SEL_MAX)
		/*
		 * Neither do we change the gain, nor do we allow shutdown or
		 * any approximate value (for now)
		 */
		return -EPERM;
	val &= TPS65910_VDD_SEL_MASK;
	ret = pmic_reg_write(dev->parent, reg_vdd + 1, val);
	if (ret)
		return ret;
	return 0;
}

static int tps65910_buck_set_value(struct udevice *dev, int uV)
{
	struct tps65910_regulator_pdata *pdata = dev_get_platdata(dev);

	if (pdata->unit == TPS65910_UNIT_VIO)
		return tps65910_regulator_set_value(dev, &smps_props_vio, uV);

	return buck_set_vdd1_vdd2_value(dev, uV);
}

static int tps65910_boost_get_value(struct udevice *dev)
{
	int vout;
	struct tps65910_regulator_pdata *pdata = dev_get_platdata(dev);

	vout = (pdata->supply >= 3000000) ? 5000000 : 0;
	return vout;
}

static int tps65910_regulator_ofdata_to_platdata(struct udevice *dev)
{
	struct udevice *supply;
	int ret;
	const char *supply_name;
	struct tps65910_regulator_pdata *pdata = dev_get_platdata(dev);

	pdata->unit = dev_get_driver_data(dev);
	if (pdata->unit > TPS65910_UNIT_VMMC)
		return -EINVAL;
	supply_name = supply_names[regulator_supplies[pdata->unit]];

	debug("Looking up supply power %s\n", supply_name);
	ret = device_get_supply_regulator(dev->parent, supply_name, &supply);
	if (ret) {
		debug("  missing supply power %s\n", supply_name);
		return ret;
	}
	pdata->supply = regulator_get_value(supply);
	if (pdata->supply < 0) {
		debug("  invalid supply voltage for regulator %s\n",
		      supply->name);
		return -EINVAL;
	}

	return 0;
}

static const struct dm_regulator_ops tps65910_boost_ops = {
	.get_value  = tps65910_boost_get_value,
	.get_enable = tps65910_get_enable,
	.set_enable = tps65910_set_enable,
};

U_BOOT_DRIVER(tps65910_boost) = {
	.name = TPS65910_BOOST_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65910_boost_ops,
	.platdata_auto_alloc_size = sizeof(struct tps65910_regulator_pdata),
	.ofdata_to_platdata = tps65910_regulator_ofdata_to_platdata,
};

static const struct dm_regulator_ops tps65910_buck_ops = {
	.get_value  = tps65910_buck_get_value,
	.set_value  = tps65910_buck_set_value,
	.get_enable = tps65910_get_enable,
	.set_enable = tps65910_set_enable,
};

U_BOOT_DRIVER(tps65910_buck) = {
	.name = TPS65910_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65910_buck_ops,
	.platdata_auto_alloc_size = sizeof(struct tps65910_regulator_pdata),
	.ofdata_to_platdata = tps65910_regulator_ofdata_to_platdata,
};

static const struct dm_regulator_ops tps65910_ldo_ops = {
	.get_value  = tps65910_ldo_get_value,
	.set_value  = tps65910_ldo_set_value,
	.get_enable = tps65910_get_enable,
	.set_enable = tps65910_set_enable,
};

U_BOOT_DRIVER(tps65910_ldo) = {
	.name = TPS65910_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65910_ldo_ops,
	.platdata_auto_alloc_size = sizeof(struct tps65910_regulator_pdata),
	.ofdata_to_platdata = tps65910_regulator_ofdata_to_platdata,
};
