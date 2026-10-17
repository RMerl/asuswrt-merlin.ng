// SPDX-License-Identifier: GPL-2.0
/*
 * SII Semiconductor Corporation S35392A RTC driver.
 *
 * Copyright (c) 2017, General Electric Company
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <command.h>
#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <linux/bitrev.h>
#include <rtc.h>
#include <linux/delay.h>

#define S35390A_CHIP_ADDR	0x30

#define S35390A_CMD_STATUS1	0x0
#define S35390A_CMD_STATUS2	0x1
#define S35390A_CMD_TIME1	0x2
#define S35390A_CMD_TIME2	0x3
#define S35390A_CMD_INT2_REG1	0x5

#define S35390A_BYTE_YEAR	0
#define S35390A_BYTE_MONTH	1
#define S35390A_BYTE_DAY	2
#define S35390A_BYTE_WDAY	3
#define S35390A_BYTE_HOURS	4
#define S35390A_BYTE_MINS	5
#define S35390A_BYTE_SECS	6

/* flags for STATUS1 */
#define S35390A_FLAG_POC	0x01
#define S35390A_FLAG_BLD	0x02
#define S35390A_FLAG_INT2	0x04
#define S35390A_FLAG_24H	0x40
#define S35390A_FLAG_RESET	0x80

/*
 * If either BLD or POC is set, then the chip has lost power long enough for
 * the time value to become invalid.
 */
#define S35390A_LOW_VOLTAGE (S35390A_FLAG_POC | S35390A_FLAG_BLD)

/*---------------------------------------------------------------------*/
#undef DEBUG_RTC

#ifdef DEBUG_RTC
#define DEBUGR(fmt, args...) printf(fmt, ##args)
#else
#define DEBUGR(fmt, args...)
#endif
/*---------------------------------------------------------------------*/

#ifdef CONFIG_DM_RTC
#define DEV_TYPE struct udevice
#else
/* Local udevice */
struct ludevice {
	u8 chip;
};

#define DEV_TYPE struct ludevice
struct ludevice dev;

#endif

#define msleep(a) udelay(a * 1000)

int lowvoltage;

static int s35392a_rtc_reset(DEV_TYPE *dev);

static int s35392a_rtc_read(DEV_TYPE *dev, u8 reg, u8 *buf, int len)
{
	int ret;

#ifdef CONFIG_DM_RTC
	ret = dm_i2c_read(dev, reg, buf, len);
#else
	(void)dev;
	ret = i2c_read(S35390A_CHIP_ADDR | reg, 0, -1, buf, len);
#endif

	return ret;
}

static int s35392a_rtc_write(DEV_TYPE *dev, u8 reg, u8 *buf, int len)
{
	int ret;

#ifdef CONFIG_DM_RTC
	ret = dm_i2c_write(dev, reg, buf, len);
#else
	(void)dev;
	ret = i2c_write(S35390A_CHIP_ADDR | reg, 0, 0, buf, len);
#endif

	return ret;
}

static int s35392a_rtc_read8(DEV_TYPE *dev, unsigned int reg)
{
	u8 val;
	int ret;

	ret = s35392a_rtc_read(dev, reg, &val, sizeof(val));
	return ret < 0 ? ret : val;
}

static int s35392a_rtc_write8(DEV_TYPE *dev, unsigned int reg, int val)
{
	int ret;
	u8 lval = val;

	ret = s35392a_rtc_write(dev, reg, &lval, sizeof(lval));
	return ret < 0 ? ret : 0;
}

static int validate_time(const struct rtc_time *tm)
{
	if ((tm->tm_year < 2000) || (tm->tm_year > 2099))
		return -EINVAL;

	if ((tm->tm_mon < 1) || (tm->tm_mon > 12))
		return -EINVAL;

	if ((tm->tm_mday < 1) || (tm->tm_mday > 31))
		return -EINVAL;

	if ((tm->tm_wday < 0) || (tm->tm_wday > 6))
		return -EINVAL;

	if ((tm->tm_hour < 0) || (tm->tm_hour > 23))
		return -EINVAL;

	if ((tm->tm_min < 0) || (tm->tm_min > 59))
		return -EINVAL;

	if ((tm->tm_sec < 0) || (tm->tm_sec > 59))
		return -EINVAL;

	return 0;
}

void s35392a_rtc_init(DEV_TYPE *dev)
{
	int status;

	status = s35392a_rtc_read8(dev, S35390A_CMD_STATUS1);
	if (status < 0)
		goto error;

	DEBUGR("init: S35390A_CMD_STATUS1: 0x%x\n", status);

	lowvoltage = status & S35390A_LOW_VOLTAGE ? 1 : 0;

	if (status & S35390A_FLAG_POC)
		/*
		 * Do not communicate for 0.5 seconds since the power-on
		 * detection circuit is in operation.
		 */
		msleep(500);

	else if (!lowvoltage)
		/*
		 * If both POC and BLD are unset everything is fine.
		 */
		return;

	if (lowvoltage)
		printf("RTC low voltage detected\n");

	if (!s35392a_rtc_reset(dev))
		return;

error:
	printf("Error RTC init.\n");
}

/* Get the current time from the RTC */
static int s35392a_rtc_get(DEV_TYPE *dev, struct rtc_time *tm)
{
	u8 date[7];
	int ret, i;

	if (lowvoltage) {
		DEBUGR("RTC low voltage detected\n");
		return -EINVAL;
	}

	ret = s35392a_rtc_read(dev, S35390A_CMD_TIME1, date, sizeof(date));
	if (ret < 0) {
		DEBUGR("Error reading date from RTC\n");
		return -EIO;
	}

	/* This chip returns the bits of each byte in reverse order */
	for (i = 0; i < 7; ++i)
		date[i] = bitrev8(date[i]);

	tm->tm_sec  = bcd2bin(date[S35390A_BYTE_SECS]);
	tm->tm_min  = bcd2bin(date[S35390A_BYTE_MINS]);
	tm->tm_hour = bcd2bin(date[S35390A_BYTE_HOURS] & ~S35390A_FLAG_24H);
	tm->tm_wday = bcd2bin(date[S35390A_BYTE_WDAY]);
	tm->tm_mday = bcd2bin(date[S35390A_BYTE_DAY]);
	tm->tm_mon  = bcd2bin(date[S35390A_BYTE_MONTH]);
	tm->tm_year = bcd2bin(date[S35390A_BYTE_YEAR]) + 2000;

	DEBUGR("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	       tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

/* Set the RTC */
static int s35392a_rtc_set(DEV_TYPE *dev, const struct rtc_time *tm)
{
	int i, ret;
	int status;
	u8 date[7];

	DEBUGR("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	       tm->tm_hour, tm->tm_min, tm->tm_sec);

	ret = validate_time(tm);
	if (ret < 0)
		return -EINVAL;

	/* We support only 24h mode */
	ret = s35392a_rtc_read8(dev, S35390A_CMD_STATUS1);
	if (ret < 0)
		return -EIO;
	status = ret;

	ret = s35392a_rtc_write8(dev, S35390A_CMD_STATUS1,
				 status | S35390A_FLAG_24H);
	if (ret < 0)
		return -EIO;

	date[S35390A_BYTE_YEAR]  = bin2bcd(tm->tm_year - 2000);
	date[S35390A_BYTE_MONTH] = bin2bcd(tm->tm_mon);
	date[S35390A_BYTE_DAY]   = bin2bcd(tm->tm_mday);
	date[S35390A_BYTE_WDAY]  = bin2bcd(tm->tm_wday);
	date[S35390A_BYTE_HOURS] = bin2bcd(tm->tm_hour);
	date[S35390A_BYTE_MINS]  = bin2bcd(tm->tm_min);
	date[S35390A_BYTE_SECS]  = bin2bcd(tm->tm_sec);

	/* This chip expects the bits of each byte to be in reverse order */
	for (i = 0; i < 7; ++i)
		date[i] = bitrev8(date[i]);

	ret = s35392a_rtc_write(dev, S35390A_CMD_TIME1, date, sizeof(date));
	if (ret < 0) {
		DEBUGR("Error writing date to RTC\n");
		return -EIO;
	}

	/* Now we have time. Reset the low voltage status */
	lowvoltage = 0;

	return 0;
}

/* Reset the RTC. */
static int s35392a_rtc_reset(DEV_TYPE *dev)
{
	int buf;
	int ret;
	unsigned int initcount = 0;

	buf = S35390A_FLAG_RESET;

initialize:
	ret = s35392a_rtc_write8(dev, S35390A_CMD_STATUS1, buf);
	if (ret < 0)
		return -EIO;

	ret = s35392a_rtc_read8(dev, S35390A_CMD_STATUS1);
	if (ret < 0)
		return -EIO;
	buf = ret;

	if (!lowvoltage)
		lowvoltage = buf & S35390A_LOW_VOLTAGE ? 1 : 0;

	if (buf & S35390A_LOW_VOLTAGE) {
		/* Try up to five times to reset the chip */
		if (initcount < 5) {
			++initcount;
			goto initialize;
		} else {
			return -EIO;
		}
	}

	return 0;
}

#ifndef CONFIG_DM_RTC

int rtc_get(struct rtc_time *tm)
{
	return s35392a_rtc_get(&dev, tm);
}

int rtc_set(struct rtc_time *tm)
{
	return s35392a_rtc_set(&dev, tm);
}

void rtc_reset(void)
{
	s35392a_rtc_reset(&dev);
}

void rtc_init(void)
{
	s35392a_rtc_init(&dev);
}

#else

static int s35392a_probe(struct udevice *dev)
{
#if defined(CONFIG_DM_RTC)
	/* 3-bit "command", or register, is encoded within the device address.
	 */
	i2c_set_chip_offset_len(dev, 0);
	i2c_set_chip_addr_offset_mask(dev, 0x7);
#endif

	s35392a_rtc_init(dev);
	return 0;
}

static const struct rtc_ops s35392a_rtc_ops = {
	.get = s35392a_rtc_get,
	.set = s35392a_rtc_set,
	.read8 = s35392a_rtc_read8,
	.write8 = s35392a_rtc_write8,
	.reset = s35392a_rtc_reset,
};

static const struct udevice_id s35392a_rtc_ids[] = {
	{ .compatible = "sii,s35392a-rtc" },
	{ .compatible = "sii,s35392a" },
	{ .compatible = "s35392a" },
	{ }
};

U_BOOT_DRIVER(s35392a_rtc) = {
	.name	  = "s35392a_rtc",
	.id	      = UCLASS_RTC,
	.probe    = s35392a_probe,
	.of_match = s35392a_rtc_ids,
	.ops	  = &s35392a_rtc_ops,
};

#endif
