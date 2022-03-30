// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Date & Time support for Philips PCF8563 RTC
 */

/* #define	DEBUG	*/

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <i2c.h>

static uchar rtc_read  (uchar reg);
static void  rtc_write (uchar reg, uchar val);

/* ------------------------------------------------------------------------- */

int rtc_get (struct rtc_time *tmp)
{
	int rel = 0;
	uchar sec, min, hour, mday, wday, mon_cent, year;

	sec	= rtc_read (0x02);
	min	= rtc_read (0x03);
	hour	= rtc_read (0x04);
	mday	= rtc_read (0x05);
	wday	= rtc_read (0x06);
	mon_cent= rtc_read (0x07);
	year	= rtc_read (0x08);

	debug ( "Get RTC year: %02x mon/cent: %02x mday: %02x wday: %02x "
		"hr: %02x min: %02x sec: %02x\n",
		year, mon_cent, mday, wday,
		hour, min, sec );
	debug ( "Alarms: wday: %02x day: %02x hour: %02x min: %02x\n",
		rtc_read (0x0C),
		rtc_read (0x0B),
		rtc_read (0x0A),
		rtc_read (0x09) );

	if (sec & 0x80) {
		puts ("### Warning: RTC Low Voltage - date/time not reliable\n");
		rel = -1;
	}

	tmp->tm_sec  = bcd2bin (sec  & 0x7F);
	tmp->tm_min  = bcd2bin (min  & 0x7F);
	tmp->tm_hour = bcd2bin (hour & 0x3F);
	tmp->tm_mday = bcd2bin (mday & 0x3F);
	tmp->tm_mon  = bcd2bin (mon_cent & 0x1F);
	tmp->tm_year = bcd2bin (year) + ((mon_cent & 0x80) ? 1900 : 2000);
	tmp->tm_wday = bcd2bin (wday & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;

	debug ( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return rel;
}

int rtc_set (struct rtc_time *tmp)
{
	uchar century;

	debug ( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	rtc_write (0x08, bin2bcd(tmp->tm_year % 100));

	century = (tmp->tm_year >= 2000) ? 0 : 0x80;
	rtc_write (0x07, bin2bcd(tmp->tm_mon) | century);

	rtc_write (0x06, bin2bcd(tmp->tm_wday));
	rtc_write (0x05, bin2bcd(tmp->tm_mday));
	rtc_write (0x04, bin2bcd(tmp->tm_hour));
	rtc_write (0x03, bin2bcd(tmp->tm_min ));
	rtc_write (0x02, bin2bcd(tmp->tm_sec ));

	return 0;
}

void rtc_reset (void)
{
	/* clear all control & status registers */
	rtc_write (0x00, 0x00);
	rtc_write (0x01, 0x00);
	rtc_write (0x0D, 0x00);

	/* clear Voltage Low bit */
	rtc_write (0x02, rtc_read (0x02) & 0x7F);

	/* reset all alarms */
	rtc_write (0x09, 0x00);
	rtc_write (0x0A, 0x00);
	rtc_write (0x0B, 0x00);
	rtc_write (0x0C, 0x00);
}

/* ------------------------------------------------------------------------- */

static uchar rtc_read (uchar reg)
{
	return (i2c_reg_read (CONFIG_SYS_I2C_RTC_ADDR, reg));
}

static void rtc_write (uchar reg, uchar val)
{
	i2c_reg_write (CONFIG_SYS_I2C_RTC_ADDR, reg, val);
}
