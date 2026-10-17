// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010,2011 NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <log.h>
#include <tps6586x.h>
#include <asm/io.h>
#include <i2c.h>
#include <linux/delay.h>

static struct udevice *tps6586x_dev;

enum {
	/* Registers that we access */
	SUPPLY_CONTROL1		= 0x20,
	SUPPLY_CONTROL2,
	SM1_VOLTAGE_V1		= 0x23,
	SM1_VOLTAGE_V2,
	SM0_VOLTAGE_V1		= 0x26,
	SM0_VOLTAGE_V2,
	PFM_MODE		= 0x47,

	/* Bits in the supply control registers */
	CTRL_SM1_RAMP		= 0x01,
	CTRL_SM1_SUPPLY2	= 0x02,
	CTRL_SM0_RAMP		= 0x04,
	CTRL_SM0_SUPPLY2	= 0x08,
};

#define MAX_I2C_RETRY	3
static int tps6586x_read(int reg)
{
	int	i;
	uchar	data;
	int	retval = -1;

	for (i = 0; i < MAX_I2C_RETRY; ++i) {
		if (!dm_i2c_read(tps6586x_dev, reg,  &data, 1)) {
			retval = (int)data;
			goto exit;
		}

		/* i2c access failed, retry */
		udelay(100);
	}

exit:
	debug("pmu_read %x=%x\n", reg, retval);
	if (retval < 0)
		debug("%s: failed to read register %#x: %d\n", __func__, reg,
		      retval);
	return retval;
}

static int tps6586x_write(int reg, uchar *data, uint len)
{
	int	i;
	int	retval = -1;

	for (i = 0; i < MAX_I2C_RETRY; ++i) {
		if (!dm_i2c_write(tps6586x_dev, reg, data, len)) {
			retval = 0;
			goto exit;
		}

		/* i2c access failed, retry */
		udelay(100);
	}

exit:
	debug("pmu_write %x=%x: ", reg, retval);
	for (i = 0; i < len; i++)
		debug("%x ", data[i]);
	if (retval)
		debug("%s: failed to write register %#x\n", __func__, reg);
	return retval;
}

/*
 * Get current voltage of SM0 and SM1
 *
 * @param sm0	Place to put SM0 voltage
 * @param sm1	Place to put SM1 voltage
 * Return: 0 if ok, -1 on error
 */
static int read_voltages(int *sm0, int *sm1)
{
	int ctrl1, ctrl2;
	int is_v2;

	/*
	 * Each vdd has two supply sources, ie, v1 and v2.
	 * The supply control reg1 and reg2 determine the current selection.
	 */
	ctrl1 = tps6586x_read(SUPPLY_CONTROL1);
	ctrl2 = tps6586x_read(SUPPLY_CONTROL2);
	if (ctrl1 == -1 || ctrl2 == -1)
		return -ENOTSUPP;

	/* Figure out whether V1 or V2 is selected */
	is_v2 = (ctrl1 | ctrl2) & CTRL_SM0_SUPPLY2;
	*sm0 = tps6586x_read(is_v2 ? SM0_VOLTAGE_V2 : SM0_VOLTAGE_V1);
	*sm1 = tps6586x_read(is_v2 ? SM1_VOLTAGE_V2 : SM1_VOLTAGE_V1);
	if (*sm0 == -1 || *sm1 == -1)
		return -ENOTSUPP;

	return 0;
}

static int set_voltage(int reg, int data, int rate)
{
	uchar control_bit;
	uchar buff[3];

	control_bit = (reg == SM0_VOLTAGE_V1 ? CTRL_SM0_RAMP : CTRL_SM1_RAMP);

	/*
	 * Only one supply is needed in u-boot. set both v1 and v2 to
	 * same value.
	 *
	 * When both v1 and v2 are set to same value, we just need to set
	 * control1 reg to trigger the supply selection.
	 */
	buff[0] = buff[1] = (uchar)data;
	buff[2] = rate;

	/* write v1, v2 and rate, then trigger */
	if (tps6586x_write(reg, buff, 3) ||
	    tps6586x_write(SUPPLY_CONTROL1, &control_bit, 1))
		return -ENOTSUPP;

	return 0;
}

static int calculate_next_voltage(int voltage, int target, int step)
{
	int diff = voltage < target ? step : -step;

	if (abs(target - voltage) > step)
		voltage += diff;
	else
		voltage = target;

	return voltage;
}

int tps6586x_set_pwm_mode(int mask)
{
	uchar val;
	int ret;

	assert(tps6586x_dev);
	ret = tps6586x_read(PFM_MODE);
	if (ret != -1) {
		val = (uchar)ret;
		val |= mask;

		ret = tps6586x_write(PFM_MODE, &val, 1);
	}

	if (ret == -1)
		debug("%s: Failed to read/write PWM mode reg\n", __func__);

	return ret;
}

int tps6586x_adjust_sm0_sm1(int sm0_target, int sm1_target, int step, int rate,
			    int min_sm0_over_sm1)
{
	int sm0, sm1;
	int bad;

	assert(tps6586x_dev);

	/* get current voltage settings */
	if (read_voltages(&sm0, &sm1)) {
		debug("%s: Cannot read voltage settings\n", __func__);
		return -EINVAL;
	}

	/*
	 * if vdd_core < vdd_cpu + rel
	 *    skip
	 *
	 * This condition may happen when system reboots due to kernel crash.
	 */
	if (min_sm0_over_sm1 != -1 && sm0 < sm1 + min_sm0_over_sm1) {
		debug("%s: SM0 is %d, SM1 is %d, but min_sm0_over_sm1 is %d\n",
		      __func__, sm0, sm1, min_sm0_over_sm1);
		return -EINVAL;
	}

	/*
	 * Since vdd_core and vdd_cpu may both stand at either greater or less
	 * than their nominal voltage, the adjustment may go either directions.
	 *
	 * Make sure vdd_core is always higher than vdd_cpu with certain margin.
	 * So, find out which vdd to adjust first in each step.
	 *
	 * case 1: both sm0 and sm1 need to move up
	 *              adjust sm0 before sm1
	 *
	 * case 2: both sm0 and sm1 need to move down
	 *              adjust sm1 before sm0
	 *
	 * case 3: sm0 moves down and sm1 moves up
	 *              adjusting either one first is fine.
	 *
	 * Adjust vdd_core and vdd_cpu one step at a time until they reach
	 * their nominal values.
	 */
	bad = 0;
	while (!bad && (sm0 != sm0_target || sm1 != sm1_target)) {
		int adjust_sm0_late = 0; /* flag to adjust vdd_core later */

		debug("%d-%d   %d-%d   ", sm0, sm0_target, sm1, sm1_target);

		if (sm0 != sm0_target) {
			/*
			 * if case 1 and case 3, set new sm0 first.
			 * otherwise, hold down until new sm1 is set.
			 */
			sm0 = calculate_next_voltage(sm0, sm0_target, step);
			if (sm1 < sm1_target)
				bad |= set_voltage(SM0_VOLTAGE_V1, sm0, rate);
			else
				adjust_sm0_late = 1;
		}

		if (sm1 != sm1_target) {
			sm1 = calculate_next_voltage(sm1, sm1_target, step);
			bad |= set_voltage(SM1_VOLTAGE_V1, sm1, rate);
		}

		if (adjust_sm0_late)
			bad |= set_voltage(SM0_VOLTAGE_V1, sm0, rate);
		debug("%d\n", adjust_sm0_late);
	}
	debug("%d-%d   %d-%d   done\n", sm0, sm0_target, sm1, sm1_target);

	return bad ? -EINVAL : 0;
}

int tps6586x_init(struct udevice *dev)
{
	tps6586x_dev = dev;

	return 0;
}
