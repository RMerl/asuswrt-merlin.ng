// SPDX-License-Identifier: GPL-2.0+
/*
 * Date & Time support for Micro Crystal RV-8803-C7.
 *
 * based on ds1307.c which is
 *   (C) Copyright 2001, 2002, 2003
 *   Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *   Keith Outwater, keith_outwater@mvis.com`
 *   Steven Scholz, steven.scholz@imc-berlin.de
 *
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <log.h>
#include <rtc.h>
#include <i2c.h>
#include <linux/bitops.h>

/*
 * RTC register addresses
 */
#define RTC_SEC_REG_ADDR	0x00
#define RTC_MIN_REG_ADDR	0x01
#define RTC_HR_REG_ADDR		0x02
#define RTC_DAY_REG_ADDR	0x03
#define RTC_DATE_REG_ADDR	0x04
#define RTC_MON_REG_ADDR	0x05
#define RTC_YR_REG_ADDR		0x06

#define RTC_FLAG_REG_ADDR	0x0E
#define RTC_FLAG_BIT_V1F	BIT(0)
#define RTC_FLAG_BIT_V2F	BIT(1)

#define RTC_CTL_REG_ADDR	0x0F
#define RTC_CTL_BIT_RST		BIT(0)

static int rv8803_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	int ret;
	u8 buf[7];

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	if (tm->tm_year < 2000 || tm->tm_year > 2099)
		printf("WARNING: year should be between 2000 and 2099!\n");

	buf[RTC_YR_REG_ADDR] = bin2bcd(tm->tm_year % 100);
	buf[RTC_MON_REG_ADDR] = bin2bcd(tm->tm_mon + 1);
	buf[RTC_DAY_REG_ADDR] = 1 << (tm->tm_wday & 0x7);
	buf[RTC_DATE_REG_ADDR] = bin2bcd(tm->tm_mday);
	buf[RTC_HR_REG_ADDR] = bin2bcd(tm->tm_hour);
	buf[RTC_MIN_REG_ADDR] = bin2bcd(tm->tm_min);
	buf[RTC_SEC_REG_ADDR] = bin2bcd(tm->tm_sec);

	ret = dm_i2c_write(dev, 0, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	return 0;
}

static int rv8803_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	int ret;
	u8 buf[7];
	int flags;

	flags = dm_i2c_reg_read(dev, RTC_FLAG_REG_ADDR);
	if (flags < 0)
		return flags;
	debug("%s: flags=%Xh\n", __func__, flags);

	if (flags & RTC_FLAG_BIT_V1F)
		printf("### Warning: temperature compensation has stopped\n");

	if (flags & RTC_FLAG_BIT_V2F) {
		printf("### Warning: Voltage low, data is invalid\n");
		return -1;
	}

	ret = dm_i2c_read(dev, 0, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	tm->tm_sec  = bcd2bin(buf[RTC_SEC_REG_ADDR] & 0x7F);
	tm->tm_min  = bcd2bin(buf[RTC_MIN_REG_ADDR] & 0x7F);
	tm->tm_hour = bcd2bin(buf[RTC_HR_REG_ADDR] & 0x3F);
	tm->tm_mday = bcd2bin(buf[RTC_DATE_REG_ADDR] & 0x3F);
	tm->tm_mon  = bcd2bin(buf[RTC_MON_REG_ADDR] & 0x1F) - 1;
	tm->tm_year = bcd2bin(buf[RTC_YR_REG_ADDR]) + 2000;
	tm->tm_wday = fls(buf[RTC_DAY_REG_ADDR] & 0x7F) - 1;
	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

static int rv8803_rtc_reset(struct udevice *dev)
{
	int ret;
	struct rtc_time tmp = {
		.tm_year = 2000,
		.tm_mon = 1,
		.tm_mday = 1,
		.tm_hour = 0,
		.tm_min = 0,
		.tm_sec = 0,
	};

	/* assert reset */
	ret = dm_i2c_reg_write(dev, RTC_CTL_REG_ADDR, RTC_CTL_BIT_RST);
	if (ret < 0)
		return ret;

	/* clear all flags */
	ret = dm_i2c_reg_write(dev, RTC_FLAG_REG_ADDR, 0);
	if (ret < 0)
		return ret;

	ret = rv8803_rtc_set(dev, &tmp);
	if (ret < 0)
		return ret;

	/* clear reset */
	ret = dm_i2c_reg_write(dev, RTC_CTL_REG_ADDR, 0);
	if (ret < 0)
		return ret;

	debug("RTC:   %4d-%02d-%02d %2d:%02d:%02d UTC\n",
	      tmp.tm_year, tmp.tm_mon, tmp.tm_mday,
	      tmp.tm_hour, tmp.tm_min, tmp.tm_sec);

	return 0;
}

static int rv8803_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS |
			   DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

static const struct rtc_ops rv8803_rtc_ops = {
	.get = rv8803_rtc_get,
	.set = rv8803_rtc_set,
	.reset = rv8803_rtc_reset,
};

static const struct udevice_id rv8803_rtc_ids[] = {
	{ .compatible = "microcrystal,rv8803", },
	{ .compatible = "epson,rx8803" },
	{ .compatible = "epson,rx8900" },
	{ }
};

U_BOOT_DRIVER(rtc_rv8803) = {
	.name	= "rtc-rv8803",
	.id	= UCLASS_RTC,
	.probe	= rv8803_probe,
	.of_match = rv8803_rtc_ids,
	.ops	= &rv8803_rtc_ops,
};
