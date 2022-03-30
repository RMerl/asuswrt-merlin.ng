// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001, 2002, 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Keith Outwater, keith_outwater@mvis.com`
 * Steven Scholz, steven.scholz@imc-berlin.de
 */

/*
 * Date & Time support (no alarms) for Dallas Semiconductor (now Maxim)
 * DS1307 and DS1338/9 Real Time Clock (RTC).
 *
 * based on ds1337.c
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <rtc.h>
#include <i2c.h>

enum ds_type {
	ds_1307,
	ds_1337,
	ds_1340,
	mcp794xx,
};

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
#define RTC_CTL_REG_ADDR	0x07

#define RTC_SEC_BIT_CH		0x80	/* Clock Halt (in Register 0)   */

#define RTC_CTL_BIT_RS0		0x01	/* Rate select 0                */
#define RTC_CTL_BIT_RS1		0x02	/* Rate select 1                */
#define RTC_CTL_BIT_SQWE	0x10	/* Square Wave Enable           */
#define RTC_CTL_BIT_OUT		0x80	/* Output Control               */

/* MCP7941X-specific bits */
#define MCP7941X_BIT_ST		0x80
#define MCP7941X_BIT_VBATEN	0x08

#ifndef CONFIG_DM_RTC

/*---------------------------------------------------------------------*/
#undef DEBUG_RTC

#ifdef DEBUG_RTC
#define DEBUGR(fmt, args...) printf(fmt, ##args)
#else
#define DEBUGR(fmt, args...)
#endif
/*---------------------------------------------------------------------*/

#ifndef CONFIG_SYS_I2C_RTC_ADDR
# define CONFIG_SYS_I2C_RTC_ADDR	0x68
#endif

#if defined(CONFIG_RTC_DS1307) && (CONFIG_SYS_I2C_SPEED > 100000)
# error The DS1307 is specified only up to 100kHz!
#endif

static uchar rtc_read (uchar reg);
static void rtc_write (uchar reg, uchar val);

/*
 * Get the current time from the RTC
 */
int rtc_get (struct rtc_time *tmp)
{
	int rel = 0;
	uchar sec, min, hour, mday, wday, mon, year;

#ifdef CONFIG_RTC_MCP79411
read_rtc:
#endif
	sec = rtc_read (RTC_SEC_REG_ADDR);
	min = rtc_read (RTC_MIN_REG_ADDR);
	hour = rtc_read (RTC_HR_REG_ADDR);
	wday = rtc_read (RTC_DAY_REG_ADDR);
	mday = rtc_read (RTC_DATE_REG_ADDR);
	mon = rtc_read (RTC_MON_REG_ADDR);
	year = rtc_read (RTC_YR_REG_ADDR);

	DEBUGR ("Get RTC year: %02x mon: %02x mday: %02x wday: %02x "
		"hr: %02x min: %02x sec: %02x\n",
		year, mon, mday, wday, hour, min, sec);

#ifdef CONFIG_RTC_DS1307
	if (sec & RTC_SEC_BIT_CH) {
		printf ("### Warning: RTC oscillator has stopped\n");
		/* clear the CH flag */
		rtc_write (RTC_SEC_REG_ADDR,
			   rtc_read (RTC_SEC_REG_ADDR) & ~RTC_SEC_BIT_CH);
		rel = -1;
	}
#endif

#ifdef CONFIG_RTC_MCP79411
	/* make sure that the backup battery is enabled */
	if (!(wday & MCP7941X_BIT_VBATEN)) {
		rtc_write(RTC_DAY_REG_ADDR,
			  wday | MCP7941X_BIT_VBATEN);
	}

	/* clock halted?  turn it on, so clock can tick. */
	if (!(sec & MCP7941X_BIT_ST)) {
		rtc_write(RTC_SEC_REG_ADDR, MCP7941X_BIT_ST);
		printf("Started RTC\n");
		goto read_rtc;
	}
#endif


	tmp->tm_sec  = bcd2bin (sec & 0x7F);
	tmp->tm_min  = bcd2bin (min & 0x7F);
	tmp->tm_hour = bcd2bin (hour & 0x3F);
	tmp->tm_mday = bcd2bin (mday & 0x3F);
	tmp->tm_mon  = bcd2bin (mon & 0x1F);
	tmp->tm_year = bcd2bin (year) + ( bcd2bin (year) >= 70 ? 1900 : 2000);
	tmp->tm_wday = bcd2bin ((wday - 1) & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;

	DEBUGR ("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return rel;
}


/*
 * Set the RTC
 */
int rtc_set (struct rtc_time *tmp)
{
	DEBUGR ("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	if (tmp->tm_year < 1970 || tmp->tm_year > 2069)
		printf("WARNING: year should be between 1970 and 2069!\n");

	rtc_write (RTC_YR_REG_ADDR, bin2bcd (tmp->tm_year % 100));
	rtc_write (RTC_MON_REG_ADDR, bin2bcd (tmp->tm_mon));
#ifdef CONFIG_RTC_MCP79411
	rtc_write (RTC_DAY_REG_ADDR,
		   bin2bcd (tmp->tm_wday + 1) | MCP7941X_BIT_VBATEN);
#else
	rtc_write (RTC_DAY_REG_ADDR, bin2bcd (tmp->tm_wday + 1));
#endif
	rtc_write (RTC_DATE_REG_ADDR, bin2bcd (tmp->tm_mday));
	rtc_write (RTC_HR_REG_ADDR, bin2bcd (tmp->tm_hour));
	rtc_write (RTC_MIN_REG_ADDR, bin2bcd (tmp->tm_min));
#ifdef CONFIG_RTC_MCP79411
	rtc_write (RTC_SEC_REG_ADDR, bin2bcd (tmp->tm_sec) | MCP7941X_BIT_ST);
#else
	rtc_write (RTC_SEC_REG_ADDR, bin2bcd (tmp->tm_sec));
#endif

	return 0;
}


/*
 * Reset the RTC. We setting the date back to 1970-01-01.
 * We also enable the oscillator output on the SQW/OUT pin and program
 * it for 32,768 Hz output. Note that according to the datasheet, turning
 * on the square wave output increases the current drain on the backup
 * battery to something between 480nA and 800nA.
 */
void rtc_reset (void)
{
	rtc_write (RTC_SEC_REG_ADDR, 0x00);	/* clearing Clock Halt	*/
	rtc_write (RTC_CTL_REG_ADDR, RTC_CTL_BIT_SQWE | RTC_CTL_BIT_RS1 | RTC_CTL_BIT_RS0);
}


/*
 * Helper functions
 */

static
uchar rtc_read (uchar reg)
{
	return (i2c_reg_read (CONFIG_SYS_I2C_RTC_ADDR, reg));
}


static void rtc_write (uchar reg, uchar val)
{
	i2c_reg_write (CONFIG_SYS_I2C_RTC_ADDR, reg, val);
}

#endif /* !CONFIG_DM_RTC */

#ifdef CONFIG_DM_RTC
static int ds1307_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	int ret;
	uchar buf[7];
	enum ds_type type = dev_get_driver_data(dev);

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	if (tm->tm_year < 1970 || tm->tm_year > 2069)
		printf("WARNING: year should be between 1970 and 2069!\n");

	buf[RTC_YR_REG_ADDR] = bin2bcd(tm->tm_year % 100);
	buf[RTC_MON_REG_ADDR] = bin2bcd(tm->tm_mon);
	buf[RTC_DAY_REG_ADDR] = bin2bcd(tm->tm_wday + 1);
	buf[RTC_DATE_REG_ADDR] = bin2bcd(tm->tm_mday);
	buf[RTC_HR_REG_ADDR] = bin2bcd(tm->tm_hour);
	buf[RTC_MIN_REG_ADDR] = bin2bcd(tm->tm_min);
	buf[RTC_SEC_REG_ADDR] = bin2bcd(tm->tm_sec);

	if (type == mcp794xx) {
		buf[RTC_DAY_REG_ADDR] |= MCP7941X_BIT_VBATEN;
		buf[RTC_SEC_REG_ADDR] |= MCP7941X_BIT_ST;
	}

	ret = dm_i2c_write(dev, 0, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	return 0;
}

static int ds1307_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	int ret;
	uchar buf[7];
	enum ds_type type = dev_get_driver_data(dev);

read_rtc:
	ret = dm_i2c_read(dev, 0, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	if (type == ds_1307) {
		if (buf[RTC_SEC_REG_ADDR] & RTC_SEC_BIT_CH) {
			printf("### Warning: RTC oscillator has stopped\n");
			/* clear the CH flag */
			buf[RTC_SEC_REG_ADDR] &= ~RTC_SEC_BIT_CH;
			dm_i2c_reg_write(dev, RTC_SEC_REG_ADDR,
					 buf[RTC_SEC_REG_ADDR]);
			return -1;
		}
	}

	if (type == mcp794xx) {
		/* make sure that the backup battery is enabled */
		if (!(buf[RTC_DAY_REG_ADDR] & MCP7941X_BIT_VBATEN)) {
			dm_i2c_reg_write(dev, RTC_DAY_REG_ADDR,
					 buf[RTC_DAY_REG_ADDR] |
					 MCP7941X_BIT_VBATEN);
		}

		/* clock halted?  turn it on, so clock can tick. */
		if (!(buf[RTC_SEC_REG_ADDR] & MCP7941X_BIT_ST)) {
			dm_i2c_reg_write(dev, RTC_SEC_REG_ADDR,
					 MCP7941X_BIT_ST);
			printf("Started RTC\n");
			goto read_rtc;
		}
	}

	tm->tm_sec  = bcd2bin(buf[RTC_SEC_REG_ADDR] & 0x7F);
	tm->tm_min  = bcd2bin(buf[RTC_MIN_REG_ADDR] & 0x7F);
	tm->tm_hour = bcd2bin(buf[RTC_HR_REG_ADDR] & 0x3F);
	tm->tm_mday = bcd2bin(buf[RTC_DATE_REG_ADDR] & 0x3F);
	tm->tm_mon  = bcd2bin(buf[RTC_MON_REG_ADDR] & 0x1F);
	tm->tm_year = bcd2bin(buf[RTC_YR_REG_ADDR]) +
			      (bcd2bin(buf[RTC_YR_REG_ADDR]) >= 70 ?
			       1900 : 2000);
	tm->tm_wday = bcd2bin((buf[RTC_DAY_REG_ADDR] - 1) & 0x07);
	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

static int ds1307_rtc_reset(struct udevice *dev)
{
	int ret;

	/* clear Clock Halt */
	ret = dm_i2c_reg_write(dev, RTC_SEC_REG_ADDR, 0x00);
	if (ret < 0)
		return ret;
	ret = dm_i2c_reg_write(dev, RTC_CTL_REG_ADDR,
			       RTC_CTL_BIT_SQWE | RTC_CTL_BIT_RS1 |
			       RTC_CTL_BIT_RS0);
	if (ret < 0)
		return ret;

	return 0;
}

static int ds1307_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS |
			   DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

static const struct rtc_ops ds1307_rtc_ops = {
	.get = ds1307_rtc_get,
	.set = ds1307_rtc_set,
	.reset = ds1307_rtc_reset,
};

static const struct udevice_id ds1307_rtc_ids[] = {
	{ .compatible = "dallas,ds1307", .data = ds_1307 },
	{ .compatible = "dallas,ds1337", .data = ds_1337 },
	{ .compatible = "dallas,ds1340", .data = ds_1340 },
	{ .compatible = "microchip,mcp7941x", .data = mcp794xx },
	{ }
};

U_BOOT_DRIVER(rtc_ds1307) = {
	.name	= "rtc-ds1307",
	.id	= UCLASS_RTC,
	.probe	= ds1307_probe,
	.of_match = ds1307_rtc_ids,
	.ops	= &ds1307_rtc_ops,
};
#endif /* CONFIG_DM_RTC */
