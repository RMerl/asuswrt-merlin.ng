// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <rtc.h>
#include <asm/rtc.h>

#define REG_COUNT 0x80

static int sandbox_rtc_get(struct udevice *dev, struct rtc_time *time)
{
	time->tm_sec = dm_i2c_reg_read(dev, REG_SEC);
	if (time->tm_sec < 0)
		return time->tm_sec;
	time->tm_min = dm_i2c_reg_read(dev, REG_MIN);
	if (time->tm_min < 0)
		return time->tm_min;
	time->tm_hour = dm_i2c_reg_read(dev, REG_HOUR);
	if (time->tm_hour < 0)
		return time->tm_hour;
	time->tm_mday = dm_i2c_reg_read(dev, REG_MDAY);
	if (time->tm_mday < 0)
		return time->tm_mday;
	time->tm_mon = dm_i2c_reg_read(dev, REG_MON);
	if (time->tm_mon < 0)
		return time->tm_mon;
	time->tm_year = dm_i2c_reg_read(dev, REG_YEAR);
	if (time->tm_year < 0)
		return time->tm_year;
	time->tm_year += 1900;
	time->tm_wday = dm_i2c_reg_read(dev, REG_WDAY);
	if (time->tm_wday < 0)
		return time->tm_wday;

	return 0;
}

static int sandbox_rtc_set(struct udevice *dev, const struct rtc_time *time)
{
	int ret;

	ret = dm_i2c_reg_write(dev, REG_SEC, time->tm_sec);
	if (ret < 0)
		return ret;
	ret = dm_i2c_reg_write(dev, REG_MIN, time->tm_min);
	if (ret < 0)
		return ret;
	ret = dm_i2c_reg_write(dev, REG_HOUR, time->tm_hour);
	if (ret < 0)
		return ret;
	ret = dm_i2c_reg_write(dev, REG_MDAY, time->tm_mday);
	if (ret < 0)
		return ret;
	ret = dm_i2c_reg_write(dev, REG_MON, time->tm_mon);
	if (ret < 0)
		return ret;
	ret = dm_i2c_reg_write(dev, REG_YEAR, time->tm_year - 1900);
	if (ret < 0)
		return ret;
	ret = dm_i2c_reg_write(dev, REG_WDAY, time->tm_wday);
	if (ret < 0)
		return ret;

	return 0;
}

static int sandbox_rtc_reset(struct udevice *dev)
{
	return dm_i2c_reg_write(dev, REG_RESET, 0);
}

static int sandbox_rtc_read8(struct udevice *dev, unsigned int reg)
{
	return dm_i2c_reg_read(dev, reg);
}

static int sandbox_rtc_write8(struct udevice *dev, unsigned int reg, int val)
{
	return dm_i2c_reg_write(dev, reg, val);
}

static const struct rtc_ops sandbox_rtc_ops = {
	.get = sandbox_rtc_get,
	.set = sandbox_rtc_set,
	.reset = sandbox_rtc_reset,
	.read8 = sandbox_rtc_read8,
	.write8 = sandbox_rtc_write8,
};

static const struct udevice_id sandbox_rtc_ids[] = {
	{ .compatible = "sandbox-rtc" },
	{ }
};

U_BOOT_DRIVER(rtc_sandbox) = {
	.name	= "rtc-sandbox",
	.id	= UCLASS_RTC,
	.of_match = sandbox_rtc_ids,
	.ops	= &sandbox_rtc_ops,
};
