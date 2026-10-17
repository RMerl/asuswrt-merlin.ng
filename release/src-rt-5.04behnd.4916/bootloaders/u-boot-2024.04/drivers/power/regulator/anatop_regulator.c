// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright (C) 2021 Linaro
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <regmap.h>
#include <syscon.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define LDO_RAMP_UP_UNIT_IN_CYCLES      64 /* 64 cycles per step */
#define LDO_RAMP_UP_FREQ_IN_MHZ         24 /* cycle based on 24M OSC */

#define LDO_POWER_GATE			0x00
#define LDO_FET_FULL_ON			0x1f

#define BIT_WIDTH_MAX			32

#define ANATOP_REGULATOR_STEP		25000
#define MIN_DROPOUT_UV			125000

struct anatop_regulator {
	const char *name;
	struct regmap *regmap;
	struct udevice *supply;
	u32 control_reg;
	u32 vol_bit_shift;
	u32 vol_bit_width;
	u32 min_bit_val;
	u32 min_voltage;
	u32 max_voltage;
	u32 delay_reg;
	u32 delay_bit_shift;
	u32 delay_bit_width;
};

static u32 anatop_get_bits(struct udevice *dev, u32 addr, int bit_shift,
			   int bit_width)
{
	const struct anatop_regulator *anatop_reg = dev_get_plat(dev);
	int err;
	u32 val, mask;

	if (bit_width == BIT_WIDTH_MAX)
		mask = ~0;
	else
		mask = (1 << bit_width) - 1;

	err = regmap_read(anatop_reg->regmap, addr, &val);
	if (err) {
		dev_dbg(dev, "cannot read reg (%d)\n", err);
		return err;
	}

	val = (val >> bit_shift) & mask;

	return val;
}

static int anatop_set_bits(struct udevice *dev, u32 addr, int bit_shift,
			   int bit_width, u32 data)
{
	const struct anatop_regulator *anatop_reg = dev_get_plat(dev);
	int err;
	u32 val, mask;

	if (bit_width == 32)
		mask = ~0;
	else
		mask = (1 << bit_width) - 1;

	err = regmap_read(anatop_reg->regmap, addr, &val);
	if (err) {
		dev_dbg(dev, "cannot read reg (%d)\n", err);
		return err;
	}
	val = val & ~(mask << bit_shift);
	err = regmap_write(anatop_reg->regmap,
			   addr, (data << bit_shift) | val);
	if (err) {
		dev_dbg(dev, "cannot write reg (%d)\n", err);
		return err;
	}

	return 0;
}

static int anatop_get_voltage(struct udevice *dev)
{
	const struct anatop_regulator *anatop_reg = dev_get_plat(dev);
	u32 sel;
	u32 val;

	if (!anatop_reg->control_reg)
		return -ENOSYS;

	val = anatop_get_bits(dev,
			      anatop_reg->control_reg,
			      anatop_reg->vol_bit_shift,
			      anatop_reg->vol_bit_width);

	sel = val - anatop_reg->min_bit_val;

	return sel * ANATOP_REGULATOR_STEP + anatop_reg->min_voltage;
}

static int anatop_set_voltage(struct udevice *dev, int uV)
{
	const struct anatop_regulator *anatop_reg = dev_get_plat(dev);
	u32 val;
	u32 sel;
	int ret;

	dev_dbg(dev, "uv %d, min %d, max %d\n", uV, anatop_reg->min_voltage,
		anatop_reg->max_voltage);

	if (uV < anatop_reg->min_voltage)
		return -EINVAL;

	if (!anatop_reg->control_reg)
		return -ENOSYS;

	sel = DIV_ROUND_UP(uV - anatop_reg->min_voltage,
			   ANATOP_REGULATOR_STEP);
	if (sel * ANATOP_REGULATOR_STEP + anatop_reg->min_voltage >
	    anatop_reg->max_voltage)
		return -EINVAL;
	val = anatop_reg->min_bit_val + sel;
	dev_dbg(dev, "calculated val %d\n", val);

	if (anatop_reg->supply) {
		ret = regulator_set_value(anatop_reg->supply,
					  uV + MIN_DROPOUT_UV);
		if (ret)
			return ret;
	}

	ret = anatop_set_bits(dev,
			      anatop_reg->control_reg,
			      anatop_reg->vol_bit_shift,
			      anatop_reg->vol_bit_width,
			      val);

	return ret;
}

static const struct dm_regulator_ops anatop_regulator_ops = {
	.set_value = anatop_set_voltage,
	.get_value = anatop_get_voltage,
};

static int anatop_regulator_probe(struct udevice *dev)
{
	struct anatop_regulator *anatop_reg;
	struct dm_regulator_uclass_plat *uc_pdata;
	struct udevice *syscon;
	int ret = 0;
	u32 val;

	anatop_reg = dev_get_plat(dev);
	uc_pdata = dev_get_uclass_plat(dev);

	anatop_reg->name = ofnode_read_string(dev_ofnode(dev),
					      "regulator-name");
	if (!anatop_reg->name)
		return log_msg_ret("regulator-name", -EINVAL);

	ret = device_get_supply_regulator(dev, "vin-supply",
					  &anatop_reg->supply);
	if (ret != -ENODEV) {
		if (ret)
			return log_msg_ret("get vin-supply", ret);

		ret = regulator_set_enable(anatop_reg->supply, true);
		if (ret)
			return ret;
	}

	ret = dev_read_u32(dev,
			   "anatop-reg-offset",
			   &anatop_reg->control_reg);
	if (ret)
		return log_msg_ret("anatop-reg-offset", ret);

	ret = dev_read_u32(dev,
			   "anatop-vol-bit-width",
			   &anatop_reg->vol_bit_width);
	if (ret)
		return log_msg_ret("anatop-vol-bit-width", ret);

	ret = dev_read_u32(dev,
			   "anatop-vol-bit-shift",
			   &anatop_reg->vol_bit_shift);
	if (ret)
		return log_msg_ret("anatop-vol-bit-shift", ret);

	ret = dev_read_u32(dev,
			   "anatop-min-bit-val",
			   &anatop_reg->min_bit_val);
	if (ret)
		return log_msg_ret("anatop-min-bit-val", ret);

	ret = dev_read_u32(dev,
			   "anatop-min-voltage",
			   &anatop_reg->min_voltage);
	if (ret)
		return log_msg_ret("anatop-min-voltage", ret);

	ret = dev_read_u32(dev,
			   "anatop-max-voltage",
			   &anatop_reg->max_voltage);
	if (ret)
		return log_msg_ret("anatop-max-voltage", ret);

	/* read LDO ramp up setting, only for core reg */
	dev_read_u32(dev, "anatop-delay-reg-offset",
		     &anatop_reg->delay_reg);
	dev_read_u32(dev, "anatop-delay-bit-width",
		     &anatop_reg->delay_bit_width);
	dev_read_u32(dev, "anatop-delay-bit-shift",
		     &anatop_reg->delay_bit_shift);

	syscon = dev_get_parent(dev);
	if (!syscon) {
		dev_dbg(dev, "unable to find syscon device\n");
		return -ENOENT;
	}

	anatop_reg->regmap = syscon_get_regmap(syscon);
	if (IS_ERR(anatop_reg->regmap)) {
		dev_dbg(dev, "unable to find regmap (%ld)\n",
			PTR_ERR(anatop_reg->regmap));
		return -ENOENT;
	}

	/* check whether need to care about LDO ramp up speed */
	if (anatop_reg->delay_bit_width) {
		/*
		 * the delay for LDO ramp up time is
		 * based on the register setting, we need
		 * to calculate how many steps LDO need to
		 * ramp up, and how much delay needed. (us)
		 */
		val = anatop_get_bits(dev,
				      anatop_reg->delay_reg,
				      anatop_reg->delay_bit_shift,
				      anatop_reg->delay_bit_width);
		uc_pdata->ramp_delay = (LDO_RAMP_UP_UNIT_IN_CYCLES << val)
			/ LDO_RAMP_UP_FREQ_IN_MHZ + 1;
	}

	return 0;
}

static const struct udevice_id of_anatop_regulator_match_tbl[] = {
	{ .compatible = "fsl,anatop-regulator", },
	{ /* end */ }
};

U_BOOT_DRIVER(anatop_regulator) = {
	.name = "anatop_regulator",
	.id = UCLASS_REGULATOR,
	.ops = &anatop_regulator_ops,
	.of_match = of_anatop_regulator_match_tbl,
	.plat_auto = sizeof(struct anatop_regulator),
	.probe = anatop_regulator_probe,
};
