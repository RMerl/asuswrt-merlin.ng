// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com.
 */

/*
 * Epson RX8025 RTC driver.
 */

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <i2c.h>

/*---------------------------------------------------------------------*/
#undef DEBUG_RTC

#ifdef DEBUG_RTC
#define DEBUGR(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGR(fmt,args...)
#endif
/*---------------------------------------------------------------------*/

#ifndef CONFIG_SYS_I2C_RTC_ADDR
# define CONFIG_SYS_I2C_RTC_ADDR	0x32
#endif

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

#define RTC_CTL1_REG_ADDR	0x0e
#define RTC_CTL2_REG_ADDR	0x0f

/*
 * Control register 1 bits
 */
#define RTC_CTL1_BIT_2412	0x20

/*
 * Control register 2 bits
 */
#define RTC_CTL2_BIT_PON	0x10
#define RTC_CTL2_BIT_VDET	0x40
#define RTC_CTL2_BIT_XST	0x20
#define RTC_CTL2_BIT_VDSL	0x80

/*
 * Note: the RX8025 I2C RTC requires register
 * reads and write to consist of a single bus
 * cycle. It is not allowed to write the register
 * address in a first cycle that is terminated by
 * a STOP condition. The chips needs a 'restart'
 * sequence (start sequence without a prior stop).
 * This driver has been written for a 4xx board.
 * U-Boot's 4xx i2c driver is currently not capable
 * to generate such cycles to some work arounds
 * are used.
 */

/* static uchar rtc_read (uchar reg); */
#define rtc_read(reg) buf[((reg) + 1) & 0xf]

static void rtc_write (uchar reg, uchar val);

/*
 * Get the current time from the RTC
 */
int rtc_get (struct rtc_time *tmp)
{
	int rel = 0;
	uchar sec, min, hour, mday, wday, mon, year, ctl2;
	uchar buf[16];

	if (i2c_read(CONFIG_SYS_I2C_RTC_ADDR, 0, 0, buf, 16))
		printf("Error reading from RTC\n");

	sec = rtc_read(RTC_SEC_REG_ADDR);
	min = rtc_read(RTC_MIN_REG_ADDR);
	hour = rtc_read(RTC_HR_REG_ADDR);
	wday = rtc_read(RTC_DAY_REG_ADDR);
	mday = rtc_read(RTC_DATE_REG_ADDR);
	mon = rtc_read(RTC_MON_REG_ADDR);
	year = rtc_read(RTC_YR_REG_ADDR);

	DEBUGR ("Get RTC year: %02x mon: %02x mday: %02x wday: %02x "
		"hr: %02x min: %02x sec: %02x\n",
		year, mon, mday, wday, hour, min, sec);

	/* dump status */
	ctl2 = rtc_read(RTC_CTL2_REG_ADDR);
	if (ctl2 & RTC_CTL2_BIT_PON) {
		printf("RTC: power-on detected\n");
		rel = -1;
	}

	if (ctl2 & RTC_CTL2_BIT_VDET) {
		printf("RTC: voltage drop detected\n");
		rel = -1;
	}

	if (!(ctl2 & RTC_CTL2_BIT_XST)) {
		printf("RTC: oscillator stop detected\n");
		rel = -1;
	}

	tmp->tm_sec  = bcd2bin (sec & 0x7F);
	tmp->tm_min  = bcd2bin (min & 0x7F);
	if (rtc_read(RTC_CTL1_REG_ADDR) & RTC_CTL1_BIT_2412)
		tmp->tm_hour = bcd2bin (hour & 0x3F);
	else
		tmp->tm_hour = bcd2bin (hour & 0x1F) % 12 +
			       ((hour & 0x20) ? 12 : 0);
	tmp->tm_mday = bcd2bin (mday & 0x3F);
	tmp->tm_mon  = bcd2bin (mon & 0x1F);
	tmp->tm_year = bcd2bin (year) + ( bcd2bin (year) >= 70 ? 1900 : 2000);
	tmp->tm_wday = bcd2bin (wday & 0x07);
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
	rtc_write (RTC_DAY_REG_ADDR, bin2bcd (tmp->tm_wday));
	rtc_write (RTC_DATE_REG_ADDR, bin2bcd (tmp->tm_mday));
	rtc_write (RTC_HR_REG_ADDR, bin2bcd (tmp->tm_hour));
	rtc_write (RTC_MIN_REG_ADDR, bin2bcd (tmp->tm_min));
	rtc_write (RTC_SEC_REG_ADDR, bin2bcd (tmp->tm_sec));

	rtc_write (RTC_CTL1_REG_ADDR, RTC_CTL1_BIT_2412);

	return 0;
}

/*
 * Reset the RTC
 */
void rtc_reset (void)
{
	uchar buf[16];
	uchar ctl2;

	if (i2c_read(CONFIG_SYS_I2C_RTC_ADDR, 0,    0,   buf, 16))
		printf("Error reading from RTC\n");

	ctl2 = rtc_read(RTC_CTL2_REG_ADDR);
	ctl2 &= ~(RTC_CTL2_BIT_PON | RTC_CTL2_BIT_VDET);
	ctl2 |= RTC_CTL2_BIT_XST | RTC_CTL2_BIT_VDSL;
	rtc_write (RTC_CTL2_REG_ADDR, ctl2);
}

/*
 * Helper functions
 */
static void rtc_write (uchar reg, uchar val)
{
	uchar buf[2];
	buf[0] = reg << 4;
	buf[1] = val;
	if (i2c_write(CONFIG_SYS_I2C_RTC_ADDR, 0, 0, buf, 2) != 0)
		printf("Error writing to RTC\n");

}
