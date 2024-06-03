/*
 * rs5c372.c
 *
 * Device driver for Ricoh's Real Time Controller RS5C372A.
 *
 * Copyright (C) 2004 Gary Jennejohn garyj@denx.de
 *
 * Based in part in ds1307.c -
 * (C) Copyright 2001, 2002, 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Keith Outwater, keith_outwater@mvis.com`
 * Steven Scholz, steven.scholz@imc-berlin.de
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <i2c.h>

/*
 * Reads are always done starting with register 15, which requires some
 * jumping-through-hoops to access the data correctly.
 *
 * Writes are always done starting with register 0.
 */

#define DEBUG 0

#if DEBUG
static unsigned int rtc_debug = DEBUG;
#else
#define rtc_debug 0	/* gcc will remove all the debug code for us */
#endif

#ifndef CONFIG_SYS_I2C_RTC_ADDR
#define CONFIG_SYS_I2C_RTC_ADDR 0x32
#endif

#define RS5C372_RAM_SIZE 0x10
#define RATE_32000HZ	0x80	/* Rate Select 32.000KHz */
#define RATE_32768HZ	0x00	/* Rate Select 32.768KHz */

#define STATUS_XPT  0x10    /* data invalid because voltage was 0 */

#define USE_24HOUR_MODE 0x20
#define TWELVE_HOUR_MODE(n) ((((n) >> 5) & 1) == 0)
#define HOURS_AP(n)	(((n) >> 5) & 1)
#define HOURS_12(n)	bcd2bin((n) & 0x1F)
#define HOURS_24(n)	bcd2bin((n) & 0x3F)


static int setup_done = 0;

static int
rs5c372_readram(unsigned char *buf, int len)
{
	int ret;

	ret = i2c_read(CONFIG_SYS_I2C_RTC_ADDR, 0, 0, buf, len);
	if (ret != 0) {
		printf("%s: failed to read\n", __FUNCTION__);
		return ret;
	}

	if (buf[0] & STATUS_XPT)
		printf("### Warning: RTC lost power\n");

	return ret;
}

static void
rs5c372_enable(void)
{
	unsigned char buf[RS5C372_RAM_SIZE + 1];
	int ret;

	/* note that this returns reg. 15 in buf[1] */
	ret = rs5c372_readram(&buf[1], RS5C372_RAM_SIZE);
	if (ret != 0) {
		printf("%s: failed\n", __FUNCTION__);
		return;
	}

	buf[0] = 0;
	/* we want to start writing at register 0 so we have to copy the */
	/* register contents up one slot */
	for (ret = 2; ret < 9; ret++)
		buf[ret - 1] = buf[ret];
	/* registers 0 to 6 (time values) are not touched */
	buf[8] = RATE_32768HZ; /* reg. 7 */
	buf[9] = 0; /* reg. 8 */
	buf[10] = 0; /* reg. 9 */
	buf[11] = 0; /* reg. 10 */
	buf[12] = 0; /* reg. 11 */
	buf[13] = 0; /* reg. 12 */
	buf[14] = 0; /* reg. 13 */
	buf[15] = 0; /* reg. 14 */
	buf[16] = USE_24HOUR_MODE; /* reg. 15 */
	ret = i2c_write(CONFIG_SYS_I2C_RTC_ADDR, 0, 0, buf, RS5C372_RAM_SIZE+1);
	if (ret != 0) {
		printf("%s: failed\n", __FUNCTION__);
		return;
	}
	setup_done = 1;

	return;
}

static void
rs5c372_convert_to_time(struct rtc_time *dt, unsigned char *buf)
{
	/* buf[0] is register 15 */
	dt->tm_sec = bcd2bin(buf[1]);
	dt->tm_min = bcd2bin(buf[2]);

	if (TWELVE_HOUR_MODE(buf[0])) {
		dt->tm_hour = HOURS_12(buf[3]);
		if (HOURS_AP(buf[3])) /* PM */
			dt->tm_hour += 12;
	} else /* 24-hour-mode */
		dt->tm_hour = HOURS_24(buf[3]);

	dt->tm_mday = bcd2bin(buf[5]);
	dt->tm_mon = bcd2bin(buf[6]);
	dt->tm_year = bcd2bin(buf[7]);
	if (dt->tm_year >= 70)
		dt->tm_year += 1900;
	else
		dt->tm_year += 2000;
	/* 0 is Sunday */
	dt->tm_wday = bcd2bin(buf[4] & 0x07);
	dt->tm_yday = 0;
	dt->tm_isdst= 0;

	if(rtc_debug > 2) {
		printf("rs5c372_convert_to_time: year = %d\n", dt->tm_year);
		printf("rs5c372_convert_to_time: mon  = %d\n", dt->tm_mon);
		printf("rs5c372_convert_to_time: mday = %d\n", dt->tm_mday);
		printf("rs5c372_convert_to_time: hour = %d\n", dt->tm_hour);
		printf("rs5c372_convert_to_time: min  = %d\n", dt->tm_min);
		printf("rs5c372_convert_to_time: sec  = %d\n", dt->tm_sec);
	}
}

/*
 * Get the current time from the RTC
 */
int
rtc_get (struct rtc_time *tmp)
{
	unsigned char buf[RS5C372_RAM_SIZE];
	int ret;

	if (!setup_done)
		rs5c372_enable();

	if (!setup_done)
		return -1;

	memset(buf, 0, sizeof(buf));

	/* note that this returns reg. 15 in buf[0] */
	ret = rs5c372_readram(buf, RS5C372_RAM_SIZE);
	if (ret != 0) {
		printf("%s: failed\n", __FUNCTION__);
		return -1;
	}

	rs5c372_convert_to_time(tmp, buf);

	return 0;
}

/*
 * Set the RTC
 */
int rtc_set (struct rtc_time *tmp)
{
	unsigned char buf[8], reg15;
	int ret;

	if (!setup_done)
		rs5c372_enable();

	if (!setup_done)
		return -1;

	if(rtc_debug > 2) {
		printf("rtc_set: tm_year = %d\n", tmp->tm_year);
		printf("rtc_set: tm_mon	 = %d\n", tmp->tm_mon);
		printf("rtc_set: tm_mday = %d\n", tmp->tm_mday);
		printf("rtc_set: tm_hour = %d\n", tmp->tm_hour);
		printf("rtc_set: tm_min	 = %d\n", tmp->tm_min);
		printf("rtc_set: tm_sec	 = %d\n", tmp->tm_sec);
	}

	memset(buf, 0, sizeof(buf));

	/* only read register 15 */
	ret = i2c_read(CONFIG_SYS_I2C_RTC_ADDR, 0, 0, buf, 1);

	if (ret == 0) {
		/* need to save register 15 */
		reg15 = buf[0];
		buf[0] = 0;	/* register address on RS5C372 */
		buf[1] = bin2bcd(tmp->tm_sec);
		buf[2] = bin2bcd(tmp->tm_min);
		/* need to handle 12 hour mode */
		if (TWELVE_HOUR_MODE(reg15)) {
			if (tmp->tm_hour >= 12) { /* PM */
				/* 12 PM is a special case */
				if (tmp->tm_hour == 12)
					buf[3] = bin2bcd(tmp->tm_hour);
				else
					buf[3] = bin2bcd(tmp->tm_hour - 12);
				buf[3] |= 0x20;
			}
		} else {
			buf[3] = bin2bcd(tmp->tm_hour);
		}

		buf[4] = bin2bcd(tmp->tm_wday);
		buf[5] = bin2bcd(tmp->tm_mday);
		buf[6] = bin2bcd(tmp->tm_mon);
		if (tmp->tm_year < 1970 || tmp->tm_year > 2069)
			printf("WARNING: year should be between 1970 and 2069!\n");
		buf[7] = bin2bcd(tmp->tm_year % 100);

		ret = i2c_write(CONFIG_SYS_I2C_RTC_ADDR, 0, 0, buf, 8);
		if (ret != 0) {
			printf("rs5c372_set_datetime(), i2c_master_send() returned %d\n",ret);
			return -1;
		}
	} else {
		return -1;
	}

	return 0;
}

/*
 * Reset the RTC.
 */
void
rtc_reset (void)
{
	if (!setup_done)
		rs5c372_enable();
}
