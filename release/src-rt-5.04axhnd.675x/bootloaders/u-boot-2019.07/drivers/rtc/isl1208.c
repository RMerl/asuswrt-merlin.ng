// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Tor Krill, Excito Elektronik i Skåne , tor@excito.com
 *
 * Modelled after the ds1337 driver
 */

/*
 * Date & Time support (no alarms) for Intersil
 * ISL1208 Real Time Clock (RTC).
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <rtc.h>
#include <i2c.h>

/*---------------------------------------------------------------------*/
#ifdef DEBUG_RTC
#define DEBUGR(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGR(fmt,args...)
#endif
/*---------------------------------------------------------------------*/

/*
 * RTC register addresses
 */

#define RTC_SEC_REG_ADDR	0x0
#define RTC_MIN_REG_ADDR	0x1
#define RTC_HR_REG_ADDR		0x2
#define RTC_DATE_REG_ADDR	0x3
#define RTC_MON_REG_ADDR	0x4
#define RTC_YR_REG_ADDR		0x5
#define RTC_DAY_REG_ADDR	0x6
#define RTC_STAT_REG_ADDR	0x7
/*
 * RTC control register bits
 */

/*
 * RTC status register bits
 */
#define RTC_STAT_BIT_ARST	0x80	/* AUTO RESET ENABLE BIT */
#define RTC_STAT_BIT_XTOSCB	0x40	/* CRYSTAL OSCILLATOR ENABLE BIT */
#define RTC_STAT_BIT_WRTC	0x10	/* WRITE RTC ENABLE BIT */
#define RTC_STAT_BIT_ALM	0x04	/* ALARM BIT */
#define RTC_STAT_BIT_BAT	0x02	/* BATTERY BIT */
#define RTC_STAT_BIT_RTCF	0x01	/* REAL TIME CLOCK FAIL BIT */

/*
 * Read an RTC register
 */

static int isl1208_rtc_read8(struct udevice *dev, unsigned int reg)
{
	return dm_i2c_reg_read(dev, reg);
}

/*
 * Write an RTC register
 */

static int isl1208_rtc_write8(struct udevice *dev, unsigned int reg, int val)
{
	return dm_i2c_reg_write(dev, reg, val);
}

/*
 * Get the current time from the RTC
 */

static int isl1208_rtc_get(struct udevice *dev, struct rtc_time *tmp)
{
	int ret;
	uchar buf[8], val;

	ret = dm_i2c_read(dev, 0, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	if (buf[RTC_STAT_REG_ADDR] & RTC_STAT_BIT_RTCF) {
		printf ("### Warning: RTC oscillator has stopped\n");
		ret = dm_i2c_read(dev, RTC_STAT_REG_ADDR, &val, sizeof(val));
		if (ret < 0)
			return ret;

		val = val & ~(RTC_STAT_BIT_BAT | RTC_STAT_BIT_RTCF);
		ret = dm_i2c_write(dev, RTC_STAT_REG_ADDR, &val, sizeof(val));
		if (ret < 0)
			return ret;
	}

	tmp->tm_sec  = bcd2bin(buf[RTC_SEC_REG_ADDR] & 0x7F);
	tmp->tm_min  = bcd2bin(buf[RTC_MIN_REG_ADDR] & 0x7F);
	tmp->tm_hour = bcd2bin(buf[RTC_HR_REG_ADDR] & 0x3F);
	tmp->tm_mday = bcd2bin(buf[RTC_DATE_REG_ADDR] & 0x3F);
	tmp->tm_mon  = bcd2bin(buf[RTC_MON_REG_ADDR] & 0x1F);
	tmp->tm_year = bcd2bin(buf[RTC_YR_REG_ADDR]) + 2000;
	tmp->tm_wday = bcd2bin(buf[RTC_DAY_REG_ADDR] & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;

	DEBUGR ("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

/*
 * Set the RTC
 */
static int isl1208_rtc_set(struct udevice *dev, const struct rtc_time *tmp)
{
	int ret;
	uchar val, buf[7];

	DEBUGR ("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	if (tmp->tm_year < 2000 || tmp->tm_year > 2099)
		printf("WARNING: year should be between 2000 and 2099!\n");

	/* enable write */
	ret = dm_i2c_read(dev, RTC_STAT_REG_ADDR, &val, sizeof(val));
	if (ret < 0)
		return ret;

	val = val | RTC_STAT_BIT_WRTC;

	ret = dm_i2c_write(dev, RTC_STAT_REG_ADDR, &val, sizeof(val));
	if (ret < 0)
		return ret;

	buf[RTC_YR_REG_ADDR] = bin2bcd(tmp->tm_year % 100);
	buf[RTC_MON_REG_ADDR] = bin2bcd(tmp->tm_mon);
	buf[RTC_DAY_REG_ADDR] = bin2bcd(tmp->tm_wday);
	buf[RTC_DATE_REG_ADDR] = bin2bcd(tmp->tm_mday);
	buf[RTC_HR_REG_ADDR] = bin2bcd(tmp->tm_hour) | 0x80; /* 24h clock */
	buf[RTC_MIN_REG_ADDR] = bin2bcd(tmp->tm_min);
	buf[RTC_SEC_REG_ADDR] = bin2bcd(tmp->tm_sec);

	ret = dm_i2c_write(dev, 0, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	/* disable write */
	ret = dm_i2c_read(dev, RTC_STAT_REG_ADDR, &val, sizeof(val));
	if (ret < 0)
		return ret;

	val = val & ~RTC_STAT_BIT_WRTC;
	ret = dm_i2c_write(dev, RTC_STAT_REG_ADDR, &val, sizeof(val));
	if (ret < 0)
		return ret;

	return 0;
}

static int isl1208_rtc_reset(struct udevice *dev)
{
	return 0;
}

static int isl1208_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS |
			   DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

static const struct rtc_ops isl1208_rtc_ops = {
	.get = isl1208_rtc_get,
	.set = isl1208_rtc_set,
	.reset = isl1208_rtc_reset,
	.read8 = isl1208_rtc_read8,
	.write8 = isl1208_rtc_write8,
};

static const struct udevice_id isl1208_rtc_ids[] = {
	{ .compatible = "isil,isl1208" },
	{ }
};

U_BOOT_DRIVER(rtc_isl1208) = {
	.name	= "rtc-isl1208",
	.id	= UCLASS_RTC,
	.probe	= isl1208_probe,
	.of_match = isl1208_rtc_ids,
	.ops	= &isl1208_rtc_ops,
};
