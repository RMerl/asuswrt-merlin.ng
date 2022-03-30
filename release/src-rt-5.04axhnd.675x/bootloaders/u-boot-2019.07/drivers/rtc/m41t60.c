// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Larry Johnson, lrj@acm.org
 *
 * based on rtc/m41t11.c which is ...
 *
 * (C) Copyright 2002
 * Andrew May, Viasat Inc, amay@viasat.com
 */

/*
 * STMicroelectronics M41T60 serial access real-time clock
 */

/* #define DEBUG 1 */

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <i2c.h>

/*
 * Convert between century and "century bits" (CB1 and CB0).  These routines
 * assume years are in the range 1900 - 2299.
 */

static unsigned char year2cb(unsigned const year)
{
	if (year < 1900 || year >= 2300)
		printf("M41T60 RTC: year %d out of range\n", year);

	return (year / 100) & 0x3;
}

static unsigned cb2year(unsigned const cb)
{
	return 1900 + 100 * ((cb + 1) & 0x3);
}

/*
 * These are simple defines for the chip local to here so they aren't too
 * verbose.  DAY/DATE aren't nice but that is how they are on the data sheet.
 */
#define RTC_SEC		0x0
#define RTC_MIN		0x1
#define RTC_HOUR	0x2
#define RTC_DAY		0x3
#define RTC_DATE	0x4
#define RTC_MONTH	0x5
#define RTC_YEAR	0x6

#define RTC_REG_CNT	7

#define RTC_CTRL	0x7

#if defined(DEBUG)
static void rtc_dump(char const *const label)
{
	uchar data[8];

	if (i2c_read(CONFIG_SYS_I2C_RTC_ADDR, 0, 1, data, sizeof(data))) {
		printf("I2C read failed in rtc_dump()\n");
		return;
	}
	printf("RTC dump %s: %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n",
	       label, data[0], data[1], data[2], data[3],
	       data[4], data[5], data[6], data[7]);
}
#else
#define rtc_dump(label)
#endif

static uchar *rtc_validate(void)
{
	/*
	 * This routine uses the OUT bit and the validity of the time values to
	 * determine whether there has been an initial power-up since the last
	 * time the routine was run.  It assumes that the OUT bit is not being
	 * used for any other purpose.
	 */
	static const uchar daysInMonth[0x13] = {
		0x00, 0x31, 0x29, 0x31, 0x30, 0x31, 0x30, 0x31,
		0x31, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x31, 0x30, 0x31
	};
	static uchar data[8];
	uchar min, date, month, years;

	rtc_dump("begin validate");
	if (i2c_read(CONFIG_SYS_I2C_RTC_ADDR, 0, 1, data, sizeof(data))) {
		printf("I2C read failed in rtc_validate()\n");
		return 0;
	}
	/*
	 * If the OUT bit is "1", there has been a loss of power, so stop the
	 * oscillator so it can be "kick-started" as per data sheet.
	 */
	if (0x00 != (data[RTC_CTRL] & 0x80)) {
		printf("M41T60 RTC clock lost power.\n");
		data[RTC_SEC] = 0x80;
		if (i2c_write(CONFIG_SYS_I2C_RTC_ADDR, RTC_SEC, 1, data, 1)) {
			printf("I2C write failed in rtc_validate()\n");
			return 0;
		}
	}
	/*
	 * If the oscillator is stopped or the date is invalid, then reset the
	 * OUT bit to "0", reset the date registers, and start the oscillator.
	 */
	min = data[RTC_MIN] & 0x7F;
	date = data[RTC_DATE];
	month = data[RTC_MONTH] & 0x3F;
	years = data[RTC_YEAR];
	if (0x59 < data[RTC_SEC] || 0x09 < (data[RTC_SEC] & 0x0F) ||
	    0x59 < min || 0x09 < (min & 0x0F) ||
	    0x23 < data[RTC_HOUR] || 0x09 < (data[RTC_HOUR] & 0x0F) ||
	    0x07 < data[RTC_DAY] || 0x00 == data[RTC_DAY] ||
	    0x12 < month ||
	    0x99 < years || 0x09 < (years & 0x0F) ||
	    daysInMonth[month] < date || 0x09 < (date & 0x0F) || 0x00 == date ||
	    (0x29 == date && 0x02 == month &&
	     ((0x00 != (years & 0x03)) ||
	      (0x00 == years && 0x00 != (data[RTC_MONTH] & 0xC0))))) {
		printf("Resetting M41T60 RTC clock.\n");
		/*
		 * Set to 00:00:00 1900-01-01 (Monday)
		 */
		data[RTC_SEC] = 0x00;
		data[RTC_MIN] &= 0x80;	/* preserve OFIE bit */
		data[RTC_HOUR] = 0x00;
		data[RTC_DAY] = 0x02;
		data[RTC_DATE] = 0x01;
		data[RTC_MONTH] = 0xC1;
		data[RTC_YEAR] = 0x00;
		data[RTC_CTRL] &= 0x7F;	/* reset OUT bit */

		if (i2c_write(CONFIG_SYS_I2C_RTC_ADDR, 0, 1, data, sizeof(data))) {
			printf("I2C write failed in rtc_validate()\n");
			return 0;
		}
	}
	return data;
}

int rtc_get(struct rtc_time *tmp)
{
	uchar const *const data = rtc_validate();

	if (!data)
		return -1;

	tmp->tm_sec = bcd2bin(data[RTC_SEC] & 0x7F);
	tmp->tm_min = bcd2bin(data[RTC_MIN] & 0x7F);
	tmp->tm_hour = bcd2bin(data[RTC_HOUR] & 0x3F);
	tmp->tm_mday = bcd2bin(data[RTC_DATE] & 0x3F);
	tmp->tm_mon = bcd2bin(data[RTC_MONTH] & 0x1F);
	tmp->tm_year = cb2year(data[RTC_MONTH] >> 6) + bcd2bin(data[RTC_YEAR]);
	tmp->tm_wday = bcd2bin(data[RTC_DAY] & 0x07) - 1;
	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

int rtc_set(struct rtc_time *tmp)
{
	uchar *const data = rtc_validate();

	if (!data)
		return -1;

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	data[RTC_SEC] = (data[RTC_SEC] & 0x80) | (bin2bcd(tmp->tm_sec) & 0x7F);
	data[RTC_MIN] = (data[RTC_MIN] & 0X80) | (bin2bcd(tmp->tm_min) & 0X7F);
	data[RTC_HOUR] = bin2bcd(tmp->tm_hour) & 0x3F;
	data[RTC_DATE] = bin2bcd(tmp->tm_mday) & 0x3F;
	data[RTC_MONTH] = bin2bcd(tmp->tm_mon) & 0x1F;
	data[RTC_YEAR] = bin2bcd(tmp->tm_year % 100);
	data[RTC_MONTH] |= year2cb(tmp->tm_year) << 6;
	data[RTC_DAY] = bin2bcd(tmp->tm_wday + 1) & 0x07;
	if (i2c_write(CONFIG_SYS_I2C_RTC_ADDR, 0, 1, data, RTC_REG_CNT)) {
		printf("I2C write failed in rtc_set()\n");
		return -1;
	}

	return 0;
}

void rtc_reset(void)
{
	uchar *const data = rtc_validate();
	char const *const s = env_get("rtccal");

	if (!data)
		return;

	rtc_dump("begin reset");
	/*
	 * If environmental variable "rtccal" is present, it must be a hex value
	 * between 0x00 and 0x3F, inclusive.  The five least-significan bits
	 * represent the calibration magnitude, and the sixth bit the sign bit.
	 * If these do not match the contents of the hardware register, that
	 * register is updated.  The value 0x00 imples no correction.  Consult
	 * the M41T60 documentation for further details.
	 */
	if (s) {
		unsigned long const l = simple_strtoul(s, 0, 16);

		if (l <= 0x3F) {
			if ((data[RTC_CTRL] & 0x3F) != l) {
				printf("Setting RTC calibration to 0x%02lX\n",
				       l);
				data[RTC_CTRL] &= 0xC0;
				data[RTC_CTRL] |= (uchar) l;
			}
		} else
			printf("environment parameter \"rtccal\" not valid: "
			       "ignoring\n");
	}
	/*
	 * Turn off frequency test.
	 */
	data[RTC_CTRL] &= 0xBF;
	if (i2c_write(CONFIG_SYS_I2C_RTC_ADDR, RTC_CTRL, 1, data + RTC_CTRL, 1)) {
		printf("I2C write failed in rtc_reset()\n");
		return;
	}
	rtc_dump("end reset");
}
