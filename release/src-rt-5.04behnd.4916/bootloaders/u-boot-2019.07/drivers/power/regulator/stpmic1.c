// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 * Author: Christophe Kerello <christophe.kerello@st.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/stpmic1.h>

struct stpmic1_range {
	int min_uv;
	int min_sel;
	int max_sel;
	int step;
};

struct stpmic1_output {
	const struct stpmic1_range *ranges;
	int nbranges;
};

#define STPMIC1_MODE(_id, _val, _name) { \
	.id = _id,			\
	.register_value = _val,		\
	.name = _name,			\
}

#define STPMIC1_RANGE(_min_uv, _min_sel, _max_sel, _step) { \
	.min_uv = _min_uv,		\
	.min_sel = _min_sel,		\
	.max_sel = _max_sel,		\
	.step = _step,			\
}

#define STPMIC1_OUTPUT(_ranges, _nbranges) { \
	.ranges = _ranges,		\
	.nbranges = _nbranges,		\
}

static int stpmic1_output_find_uv(int sel,
				  const struct stpmic1_output *output)
{
	const struct stpmic1_range *range;
	int i;

	for (i = 0, range = output->ranges;
	     i < output->nbranges; i++, range++) {
		if (sel >= range->min_sel && sel <= range->max_sel)
			return range->min_uv +
			       (sel - range->min_sel) * range->step;
	}

	return -EINVAL;
}

static int stpmic1_output_find_sel(int uv,
				   const struct stpmic1_output *output)
{
	const struct stpmic1_range *range;
	int i;

	for (i = 0, range = output->ranges;
	     i < output->nbranges; i++, range++) {
		if (uv == range->min_uv && !range->step)
			return range->min_sel;

		if (uv >= range->min_uv &&
		    uv <= range->min_uv +
			  (range->max_sel - range->min_sel) * range->step)
			return range->min_sel +
			       (uv - range->min_uv) / range->step;
	}

	return -EINVAL;
}

/*
 * BUCK regulators
 */

static const struct stpmic1_range buck1_ranges[] = {
	STPMIC1_RANGE(725000, 0, 4, 0),
	STPMIC1_RANGE(725000, 5, 36, 25000),
	STPMIC1_RANGE(1500000, 37, 63, 0),
};

static const struct stpmic1_range buck2_ranges[] = {
	STPMIC1_RANGE(1000000, 0, 17, 0),
	STPMIC1_RANGE(1050000, 18, 19, 0),
	STPMIC1_RANGE(1100000, 20, 21, 0),
	STPMIC1_RANGE(1150000, 22, 23, 0),
	STPMIC1_RANGE(1200000, 24, 25, 0),
	STPMIC1_RANGE(1250000, 26, 27, 0),
	STPMIC1_RANGE(1300000, 28, 29, 0),
	STPMIC1_RANGE(1350000, 30, 31, 0),
	STPMIC1_RANGE(1400000, 32, 33, 0),
	STPMIC1_RANGE(1450000, 34, 35, 0),
	STPMIC1_RANGE(1500000, 36, 63, 0),
};

static const struct stpmic1_range buck3_ranges[] = {
	STPMIC1_RANGE(1000000, 0, 19, 0),
	STPMIC1_RANGE(1100000, 20, 23, 0),
	STPMIC1_RANGE(1200000, 24, 27, 0),
	STPMIC1_RANGE(1300000, 28, 31, 0),
	STPMIC1_RANGE(1400000, 32, 35, 0),
	STPMIC1_RANGE(1500000, 36, 55, 100000),
	STPMIC1_RANGE(3400000, 56, 63, 0),
};

static const struct stpmic1_range buck4_ranges[] = {
	STPMIC1_RANGE(600000, 0, 27, 25000),
	STPMIC1_RANGE(1300000, 28, 29, 0),
	STPMIC1_RANGE(1350000, 30, 31, 0),
	STPMIC1_RANGE(1400000, 32, 33, 0),
	STPMIC1_RANGE(1450000, 34, 35, 0),
	STPMIC1_RANGE(1500000, 36, 60, 100000),
	STPMIC1_RANGE(3900000, 61, 63, 0),
};

/* BUCK: 1,2,3,4 - voltage ranges */
static const struct stpmic1_output buck_voltage_range[] = {
	STPMIC1_OUTPUT(buck1_ranges, ARRAY_SIZE(buck1_ranges)),
	STPMIC1_OUTPUT(buck2_ranges, ARRAY_SIZE(buck2_ranges)),
	STPMIC1_OUTPUT(buck3_ranges, ARRAY_SIZE(buck3_ranges)),
	STPMIC1_OUTPUT(buck4_ranges, ARRAY_SIZE(buck4_ranges)),
};

/* BUCK modes */
static const struct dm_regulator_mode buck_modes[] = {
	STPMIC1_MODE(STPMIC1_PREG_MODE_HP, STPMIC1_PREG_MODE_HP, "HP"),
	STPMIC1_MODE(STPMIC1_PREG_MODE_LP, STPMIC1_PREG_MODE_LP, "LP"),
};

static int stpmic1_buck_get_uv(struct udevice *dev, int buck)
{
	int sel;

	sel = pmic_reg_read(dev, STPMIC1_BUCKX_MAIN_CR(buck));
	if (sel < 0)
		return sel;

	sel &= STPMIC1_BUCK_VOUT_MASK;
	sel >>= STPMIC1_BUCK_VOUT_SHIFT;

	return stpmic1_output_find_uv(sel, &buck_voltage_range[buck]);
}

static int stpmic1_buck_get_value(struct udevice *dev)
{
	return stpmic1_buck_get_uv(dev->parent, dev->driver_data - 1);
}

static int stpmic1_buck_set_value(struct udevice *dev, int uv)
{
	int sel, buck = dev->driver_data - 1;

	sel = stpmic1_output_find_sel(uv, &buck_voltage_range[buck]);
	if (sel < 0)
		return sel;

	return pmic_clrsetbits(dev->parent,
			       STPMIC1_BUCKX_MAIN_CR(buck),
			       STPMIC1_BUCK_VOUT_MASK,
			       sel << STPMIC1_BUCK_VOUT_SHIFT);
}

static int stpmic1_buck_get_enable(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent,
			    STPMIC1_BUCKX_MAIN_CR(dev->driver_data - 1));
	if (ret < 0)
		return false;

	return ret & STPMIC1_BUCK_ENA ? true : false;
}

static int stpmic1_buck_set_enable(struct udevice *dev, bool enable)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	int delay = enable ? STPMIC1_DEFAULT_START_UP_DELAY_MS :
			     STPMIC1_DEFAULT_STOP_DELAY_MS;
	int ret, uv;

	/* if regulator is already in the wanted state, nothing to do */
	if (stpmic1_buck_get_enable(dev) == enable)
		return 0;

	if (enable) {
		uc_pdata = dev_get_uclass_platdata(dev);
		uv = stpmic1_buck_get_value(dev);
		if (uv < uc_pdata->min_uV || uv > uc_pdata->max_uV)
			stpmic1_buck_set_value(dev, uc_pdata->min_uV);
	}

	ret = pmic_clrsetbits(dev->parent,
			      STPMIC1_BUCKX_MAIN_CR(dev->driver_data - 1),
			      STPMIC1_BUCK_ENA, enable ? STPMIC1_BUCK_ENA : 0);
	mdelay(delay);

	return ret;
}

static int stpmic1_buck_get_mode(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent,
			    STPMIC1_BUCKX_MAIN_CR(dev->driver_data - 1));
	if (ret < 0)
		return ret;

	return ret & STPMIC1_BUCK_PREG_MODE ? STPMIC1_PREG_MODE_LP :
					      STPMIC1_PREG_MODE_HP;
}

static int stpmic1_buck_set_mode(struct udevice *dev, int mode)
{
	return pmic_clrsetbits(dev->parent,
			       STPMIC1_BUCKX_MAIN_CR(dev->driver_data - 1),
			       STPMIC1_BUCK_PREG_MODE,
			       mode ? STPMIC1_BUCK_PREG_MODE : 0);
}

static int stpmic1_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	if (!dev->driver_data || dev->driver_data > STPMIC1_MAX_BUCK)
		return -EINVAL;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode = (struct dm_regulator_mode *)buck_modes;
	uc_pdata->mode_count = ARRAY_SIZE(buck_modes);

	return 0;
}

static const struct dm_regulator_ops stpmic1_buck_ops = {
	.get_value  = stpmic1_buck_get_value,
	.set_value  = stpmic1_buck_set_value,
	.get_enable = stpmic1_buck_get_enable,
	.set_enable = stpmic1_buck_set_enable,
	.get_mode   = stpmic1_buck_get_mode,
	.set_mode   = stpmic1_buck_set_mode,
};

U_BOOT_DRIVER(stpmic1_buck) = {
	.name = "stpmic1_buck",
	.id = UCLASS_REGULATOR,
	.ops = &stpmic1_buck_ops,
	.probe = stpmic1_buck_probe,
};

/*
 * LDO regulators
 */

static const struct stpmic1_range ldo12_ranges[] = {
	STPMIC1_RANGE(1700000, 0, 7, 0),
	STPMIC1_RANGE(1700000, 8, 24, 100000),
	STPMIC1_RANGE(3300000, 25, 31, 0),
};

static const struct stpmic1_range ldo3_ranges[] = {
	STPMIC1_RANGE(1700000, 0, 7, 0),
	STPMIC1_RANGE(1700000, 8, 24, 100000),
	STPMIC1_RANGE(3300000, 25, 30, 0),
	/* Sel 31 is special case when LDO3 is in mode sync_source (BUCK2/2) */
};

static const struct stpmic1_range ldo5_ranges[] = {
	STPMIC1_RANGE(1700000, 0, 7, 0),
	STPMIC1_RANGE(1700000, 8, 30, 100000),
	STPMIC1_RANGE(3900000, 31, 31, 0),
};

static const struct stpmic1_range ldo6_ranges[] = {
	STPMIC1_RANGE(900000, 0, 24, 100000),
	STPMIC1_RANGE(3300000, 25, 31, 0),
};

/* LDO: 1,2,3,4,5,6 - voltage ranges */
static const struct stpmic1_output ldo_voltage_range[] = {
	STPMIC1_OUTPUT(ldo12_ranges, ARRAY_SIZE(ldo12_ranges)),
	STPMIC1_OUTPUT(ldo12_ranges, ARRAY_SIZE(ldo12_ranges)),
	STPMIC1_OUTPUT(ldo3_ranges, ARRAY_SIZE(ldo3_ranges)),
	STPMIC1_OUTPUT(NULL, 0),
	STPMIC1_OUTPUT(ldo5_ranges, ARRAY_SIZE(ldo5_ranges)),
	STPMIC1_OUTPUT(ldo6_ranges, ARRAY_SIZE(ldo6_ranges)),
};

/* LDO modes */
static const struct dm_regulator_mode ldo_modes[] = {
	STPMIC1_MODE(STPMIC1_LDO_MODE_NORMAL,
		     STPMIC1_LDO_MODE_NORMAL, "NORMAL"),
	STPMIC1_MODE(STPMIC1_LDO_MODE_BYPASS,
		     STPMIC1_LDO_MODE_BYPASS, "BYPASS"),
	STPMIC1_MODE(STPMIC1_LDO_MODE_SINK_SOURCE,
		     STPMIC1_LDO_MODE_SINK_SOURCE, "SINK SOURCE"),
};

static int stpmic1_ldo_get_value(struct udevice *dev)
{
	int sel, ldo = dev->driver_data - 1;

	sel = pmic_reg_read(dev->parent, STPMIC1_LDOX_MAIN_CR(ldo));
	if (sel < 0)
		return sel;

	/* ldo4 => 3,3V */
	if (ldo == STPMIC1_LDO4)
		return STPMIC1_LDO4_UV;

	sel &= STPMIC1_LDO12356_VOUT_MASK;
	sel >>= STPMIC1_LDO12356_VOUT_SHIFT;

	/* ldo3, sel = 31 => BUCK2/2 */
	if (ldo == STPMIC1_LDO3 && sel == STPMIC1_LDO3_DDR_SEL)
		return stpmic1_buck_get_uv(dev->parent, STPMIC1_BUCK2) / 2;

	return stpmic1_output_find_uv(sel, &ldo_voltage_range[ldo]);
}

static int stpmic1_ldo_set_value(struct udevice *dev, int uv)
{
	int sel, ldo = dev->driver_data - 1;

	/* ldo4 => not possible */
	if (ldo == STPMIC1_LDO4)
		return -EINVAL;

	sel = stpmic1_output_find_sel(uv, &ldo_voltage_range[ldo]);
	if (sel < 0)
		return sel;

	return pmic_clrsetbits(dev->parent,
			       STPMIC1_LDOX_MAIN_CR(ldo),
			       STPMIC1_LDO12356_VOUT_MASK,
			       sel << STPMIC1_LDO12356_VOUT_SHIFT);
}

static int stpmic1_ldo_get_enable(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent,
			    STPMIC1_LDOX_MAIN_CR(dev->driver_data - 1));
	if (ret < 0)
		return false;

	return ret & STPMIC1_LDO_ENA ? true : false;
}

static int stpmic1_ldo_set_enable(struct udevice *dev, bool enable)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	int delay = enable ? STPMIC1_DEFAULT_START_UP_DELAY_MS :
			     STPMIC1_DEFAULT_STOP_DELAY_MS;
	int ret, uv;

	/* if regulator is already in the wanted state, nothing to do */
	if (stpmic1_ldo_get_enable(dev) == enable)
		return 0;

	if (enable) {
		uc_pdata = dev_get_uclass_platdata(dev);
		uv = stpmic1_ldo_get_value(dev);
		if (uv < uc_pdata->min_uV || uv > uc_pdata->max_uV)
			stpmic1_ldo_set_value(dev, uc_pdata->min_uV);
	}

	ret = pmic_clrsetbits(dev->parent,
			      STPMIC1_LDOX_MAIN_CR(dev->driver_data - 1),
			      STPMIC1_LDO_ENA, enable ? STPMIC1_LDO_ENA : 0);
	mdelay(delay);

	return ret;
}

static int stpmic1_ldo_get_mode(struct udevice *dev)
{
	int ret, ldo = dev->driver_data - 1;

	if (ldo != STPMIC1_LDO3)
		return -EINVAL;

	ret = pmic_reg_read(dev->parent, STPMIC1_LDOX_MAIN_CR(ldo));
	if (ret < 0)
		return ret;

	if (ret & STPMIC1_LDO3_MODE)
		return STPMIC1_LDO_MODE_BYPASS;

	ret &= STPMIC1_LDO12356_VOUT_MASK;
	ret >>= STPMIC1_LDO12356_VOUT_SHIFT;

	return ret == STPMIC1_LDO3_DDR_SEL ? STPMIC1_LDO_MODE_SINK_SOURCE :
					     STPMIC1_LDO_MODE_NORMAL;
}

static int stpmic1_ldo_set_mode(struct udevice *dev, int mode)
{
	int ret, ldo = dev->driver_data - 1;

	if (ldo != STPMIC1_LDO3)
		return -EINVAL;

	ret = pmic_reg_read(dev->parent, STPMIC1_LDOX_MAIN_CR(ldo));
	if (ret < 0)
		return ret;

	switch (mode) {
	case STPMIC1_LDO_MODE_SINK_SOURCE:
		ret &= ~STPMIC1_LDO12356_VOUT_MASK;
		ret |= STPMIC1_LDO3_DDR_SEL << STPMIC1_LDO12356_VOUT_SHIFT;
	case STPMIC1_LDO_MODE_NORMAL:
		ret &= ~STPMIC1_LDO3_MODE;
		break;
	case STPMIC1_LDO_MODE_BYPASS:
		ret |= STPMIC1_LDO3_MODE;
		break;
	}

	return pmic_reg_write(dev->parent, STPMIC1_LDOX_MAIN_CR(ldo), ret);
}

static int stpmic1_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	if (!dev->driver_data || dev->driver_data > STPMIC1_MAX_LDO)
		return -EINVAL;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_LDO;
	if (dev->driver_data - 1 == STPMIC1_LDO3) {
		uc_pdata->mode = (struct dm_regulator_mode *)ldo_modes;
		uc_pdata->mode_count = ARRAY_SIZE(ldo_modes);
	} else {
		uc_pdata->mode_count = 0;
	}

	return 0;
}

static const struct dm_regulator_ops stpmic1_ldo_ops = {
	.get_value  = stpmic1_ldo_get_value,
	.set_value  = stpmic1_ldo_set_value,
	.get_enable = stpmic1_ldo_get_enable,
	.set_enable = stpmic1_ldo_set_enable,
	.get_mode   = stpmic1_ldo_get_mode,
	.set_mode   = stpmic1_ldo_set_mode,
};

U_BOOT_DRIVER(stpmic1_ldo) = {
	.name = "stpmic1_ldo",
	.id = UCLASS_REGULATOR,
	.ops = &stpmic1_ldo_ops,
	.probe = stpmic1_ldo_probe,
};

/*
 * VREF DDR regulator
 */

static int stpmic1_vref_ddr_get_value(struct udevice *dev)
{
	/* BUCK2/2 */
	return stpmic1_buck_get_uv(dev->parent, STPMIC1_BUCK2) / 2;
}

static int stpmic1_vref_ddr_get_enable(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent, STPMIC1_REFDDR_MAIN_CR);
	if (ret < 0)
		return false;

	return ret & STPMIC1_VREF_ENA ? true : false;
}

static int stpmic1_vref_ddr_set_enable(struct udevice *dev, bool enable)
{
	int delay = enable ? STPMIC1_DEFAULT_START_UP_DELAY_MS :
			     STPMIC1_DEFAULT_STOP_DELAY_MS;
	int ret;

	/* if regulator is already in the wanted state, nothing to do */
	if (stpmic1_vref_ddr_get_enable(dev) == enable)
		return 0;

	ret = pmic_clrsetbits(dev->parent, STPMIC1_REFDDR_MAIN_CR,
			      STPMIC1_VREF_ENA, enable ? STPMIC1_VREF_ENA : 0);
	mdelay(delay);

	return ret;
}

static int stpmic1_vref_ddr_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_FIXED;
	uc_pdata->mode_count = 0;

	return 0;
}

static const struct dm_regulator_ops stpmic1_vref_ddr_ops = {
	.get_value  = stpmic1_vref_ddr_get_value,
	.get_enable = stpmic1_vref_ddr_get_enable,
	.set_enable = stpmic1_vref_ddr_set_enable,
};

U_BOOT_DRIVER(stpmic1_vref_ddr) = {
	.name = "stpmic1_vref_ddr",
	.id = UCLASS_REGULATOR,
	.ops = &stpmic1_vref_ddr_ops,
	.probe = stpmic1_vref_ddr_probe,
};

/*
 * BOOST regulator
 */

static int stpmic1_boost_get_enable(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent, STPMIC1_BST_SW_CR);
	if (ret < 0)
		return false;

	return ret & STPMIC1_BST_ON ? true : false;
}

static int stpmic1_boost_set_enable(struct udevice *dev, bool enable)
{
	int ret;

	ret = pmic_reg_read(dev->parent, STPMIC1_BST_SW_CR);
	if (ret < 0)
		return ret;

	if (!enable && ret & STPMIC1_PWR_SW_ON)
		return -EINVAL;

	/* if regulator is already in the wanted state, nothing to do */
	if (!!(ret & STPMIC1_BST_ON) == enable)
		return 0;

	ret = pmic_clrsetbits(dev->parent, STPMIC1_BST_SW_CR,
			      STPMIC1_BST_ON,
			      enable ? STPMIC1_BST_ON : 0);
	if (enable)
		mdelay(STPMIC1_USB_BOOST_START_UP_DELAY_MS);

	return ret;
}

static int stpmic1_boost_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_FIXED;
	uc_pdata->mode_count = 0;

	return 0;
}

static const struct dm_regulator_ops stpmic1_boost_ops = {
	.get_enable = stpmic1_boost_get_enable,
	.set_enable = stpmic1_boost_set_enable,
};

U_BOOT_DRIVER(stpmic1_boost) = {
	.name = "stpmic1_boost",
	.id = UCLASS_REGULATOR,
	.ops = &stpmic1_boost_ops,
	.probe = stpmic1_boost_probe,
};

/*
 * USB power switch
 */

static int stpmic1_pwr_sw_get_enable(struct udevice *dev)
{
	uint mask = 1 << dev->driver_data;
	int ret;

	ret = pmic_reg_read(dev->parent, STPMIC1_BST_SW_CR);
	if (ret < 0)
		return false;

	return ret & mask ? true : false;
}

static int stpmic1_pwr_sw_set_enable(struct udevice *dev, bool enable)
{
	uint mask = 1 << dev->driver_data;
	int delay = enable ? STPMIC1_DEFAULT_START_UP_DELAY_MS :
			     STPMIC1_DEFAULT_STOP_DELAY_MS;
	int ret;

	ret = pmic_reg_read(dev->parent, STPMIC1_BST_SW_CR);
	if (ret < 0)
		return ret;

	/* if regulator is already in the wanted state, nothing to do */
	if (!!(ret & mask) == enable)
		return 0;

	/* Boost management */
	if (enable && !(ret & STPMIC1_BST_ON)) {
		pmic_clrsetbits(dev->parent, STPMIC1_BST_SW_CR,
				STPMIC1_BST_ON, STPMIC1_BST_ON);
		mdelay(STPMIC1_USB_BOOST_START_UP_DELAY_MS);
	} else if (!enable && ret & STPMIC1_BST_ON &&
		   (ret & STPMIC1_PWR_SW_ON) != STPMIC1_PWR_SW_ON) {
		pmic_clrsetbits(dev->parent, STPMIC1_BST_SW_CR,
				STPMIC1_BST_ON, 0);
	}

	ret = pmic_clrsetbits(dev->parent, STPMIC1_BST_SW_CR,
			      mask, enable ? mask : 0);
	mdelay(delay);

	return ret;
}

static int stpmic1_pwr_sw_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	if (!dev->driver_data || dev->driver_data > STPMIC1_MAX_PWR_SW)
		return -EINVAL;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_FIXED;
	uc_pdata->mode_count = 0;

	return 0;
}

static const struct dm_regulator_ops stpmic1_pwr_sw_ops = {
	.get_enable = stpmic1_pwr_sw_get_enable,
	.set_enable = stpmic1_pwr_sw_set_enable,
};

U_BOOT_DRIVER(stpmic1_pwr_sw) = {
	.name = "stpmic1_pwr_sw",
	.id = UCLASS_REGULATOR,
	.ops = &stpmic1_pwr_sw_ops,
	.probe = stpmic1_pwr_sw_probe,
};
