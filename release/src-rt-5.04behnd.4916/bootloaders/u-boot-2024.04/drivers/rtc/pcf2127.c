/*
 * Copyright (C) 2016 by NXP Semiconductors Inc.
 * Date & Time support for PCF2127 RTC
 */

/*	#define	DEBUG	*/

#include <common.h>
#include <command.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <rtc.h>

#define PCF2127_REG_CTRL1	0x00
#define PCF2127_REG_CTRL2	0x01
#define PCF2127_REG_CTRL3	0x02
#define PCF2127_REG_SC		0x03
#define PCF2127_REG_MN		0x04
#define PCF2127_REG_HR		0x05
#define PCF2127_REG_DM		0x06
#define PCF2127_REG_DW		0x07
#define PCF2127_REG_MO		0x08
#define PCF2127_REG_YR		0x09

static int pcf2127_rtc_read(struct udevice *dev, uint offset, u8 *buffer, uint len)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	struct i2c_msg msg;
	int ret;

	/* Set the address of the start register to be read */
	ret = dm_i2c_write(dev, offset, NULL, 0);
	if (ret < 0)
		return ret;

	/* Read register's data */
	msg.addr = chip->chip_addr;
	msg.flags |= I2C_M_RD;
	msg.len = len;
	msg.buf = buffer;

	return dm_i2c_xfer(dev, &msg, 1);
}

static int pcf2127_rtc_write(struct udevice *dev, uint offset,
			     const u8 *buffer, uint len)
{
	return dm_i2c_write(dev, offset, buffer, len);
}

static int pcf2127_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	uchar buf[7] = {0};
	int i = 0, ret;

	/* hours, minutes and seconds */
	buf[i++] = bin2bcd(tm->tm_sec);
	buf[i++] = bin2bcd(tm->tm_min);
	buf[i++] = bin2bcd(tm->tm_hour);
	buf[i++] = bin2bcd(tm->tm_mday);
	buf[i++] = tm->tm_wday & 0x07;

	/* month, 1 - 12 */
	buf[i++] = bin2bcd(tm->tm_mon);

	/* year */
	buf[i++] = bin2bcd(tm->tm_year % 100);

	/* write register's data */
	ret = dm_i2c_write(dev, PCF2127_REG_SC, buf, i);

	return ret;
}

static int pcf2127_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	int ret = 0;
	uchar buf[10] = { PCF2127_REG_CTRL1 };

	ret = pcf2127_rtc_read(dev, PCF2127_REG_CTRL1, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	if (buf[PCF2127_REG_CTRL3] & 0x04)
		puts("### Warning: RTC Low Voltage - date/time not reliable\n");

	tm->tm_sec  = bcd2bin(buf[PCF2127_REG_SC] & 0x7F);
	tm->tm_min  = bcd2bin(buf[PCF2127_REG_MN] & 0x7F);
	tm->tm_hour = bcd2bin(buf[PCF2127_REG_HR] & 0x3F);
	tm->tm_mday = bcd2bin(buf[PCF2127_REG_DM] & 0x3F);
	tm->tm_mon  = bcd2bin(buf[PCF2127_REG_MO] & 0x1F);
	tm->tm_year = bcd2bin(buf[PCF2127_REG_YR]) + 1900;
	if (tm->tm_year < 1970)
		tm->tm_year += 100;	/* assume we are in 1970...2069 */
	tm->tm_wday = buf[PCF2127_REG_DW] & 0x07;
	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	return ret;
}

static int pcf2127_rtc_reset(struct udevice *dev)
{
	/*Doing nothing here*/

	return 0;
}

static const struct rtc_ops pcf2127_rtc_ops = {
	.get = pcf2127_rtc_get,
	.set = pcf2127_rtc_set,
	.reset = pcf2127_rtc_reset,
	.read = pcf2127_rtc_read,
	.write = pcf2127_rtc_write,
};

static const struct udevice_id pcf2127_rtc_ids[] = {
	{ .compatible = "nxp,pcf2127" },
	{ .compatible = "nxp,pcf2129" },
	{ .compatible = "nxp,pca2129" },
	{ }
};

U_BOOT_DRIVER(rtc_pcf2127) = {
	.name	= "rtc-pcf2127",
	.id	= UCLASS_RTC,
	.of_match = pcf2127_rtc_ids,
	.ops	= &pcf2127_rtc_ops,
};
