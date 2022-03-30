// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * based on a the Linux rtc-x1207.c driver which is:
 *	Copyright 2004 Karen Spearel
 *	Copyright 2005 Alessandro Zummo
 *
 * Information and datasheet:
 * http://www.intersil.com/cda/deviceinfo/0,1477,X1205,00.html
 */

/*
 * Date & Time support for Xicor/Intersil X1205 RTC
 */

/* #define	DEBUG	*/

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <i2c.h>

#define CCR_SEC			0
#define CCR_MIN			1
#define CCR_HOUR		2
#define CCR_MDAY		3
#define CCR_MONTH		4
#define CCR_YEAR		5
#define CCR_WDAY		6
#define CCR_Y2K			7

#define X1205_REG_SR		0x3F	/* status register */
#define X1205_REG_Y2K		0x37
#define X1205_REG_DW		0x36
#define X1205_REG_YR		0x35
#define X1205_REG_MO		0x34
#define X1205_REG_DT		0x33
#define X1205_REG_HR		0x32
#define X1205_REG_MN		0x31
#define X1205_REG_SC		0x30
#define X1205_REG_DTR		0x13
#define X1205_REG_ATR		0x12
#define X1205_REG_INT		0x11
#define X1205_REG_0		0x10
#define X1205_REG_Y2K1		0x0F
#define X1205_REG_DWA1		0x0E
#define X1205_REG_YRA1		0x0D
#define X1205_REG_MOA1		0x0C
#define X1205_REG_DTA1		0x0B
#define X1205_REG_HRA1		0x0A
#define X1205_REG_MNA1		0x09
#define X1205_REG_SCA1		0x08
#define X1205_REG_Y2K0		0x07
#define X1205_REG_DWA0		0x06
#define X1205_REG_YRA0		0x05
#define X1205_REG_MOA0		0x04
#define X1205_REG_DTA0		0x03
#define X1205_REG_HRA0		0x02
#define X1205_REG_MNA0		0x01
#define X1205_REG_SCA0		0x00

#define X1205_CCR_BASE		0x30	/* Base address of CCR */
#define X1205_ALM0_BASE		0x00	/* Base address of ALARM0 */

#define X1205_SR_RTCF		0x01	/* Clock failure */
#define X1205_SR_WEL		0x02	/* Write Enable Latch */
#define X1205_SR_RWEL		0x04	/* Register Write Enable */

#define X1205_DTR_DTR0		0x01
#define X1205_DTR_DTR1		0x02
#define X1205_DTR_DTR2		0x04

#define X1205_HR_MIL		0x80	/* Set in ccr.hour for 24 hr mode */

static void rtc_write(int reg, u8 val)
{
	i2c_write(CONFIG_SYS_I2C_RTC_ADDR, reg, 2, &val, 1);
}

/*
 * In the routines that deal directly with the x1205 hardware, we use
 * rtc_time -- month 0-11, hour 0-23, yr = calendar year-epoch
 * Epoch is initialized as 2000. Time is set to UTC.
 */
int rtc_get(struct rtc_time *tm)
{
	u8 buf[8];

	i2c_read(CONFIG_SYS_I2C_RTC_ADDR, X1205_CCR_BASE, 2, buf, 8);

	debug("%s: raw read data - sec=%02x, min=%02x, hr=%02x, "
	      "mday=%02x, mon=%02x, year=%02x, wday=%02x, y2k=%02x\n",
	      __FUNCTION__,
	      buf[0], buf[1], buf[2], buf[3],
	      buf[4], buf[5], buf[6], buf[7]);

	tm->tm_sec = bcd2bin(buf[CCR_SEC]);
	tm->tm_min = bcd2bin(buf[CCR_MIN]);
	tm->tm_hour = bcd2bin(buf[CCR_HOUR] & 0x3F); /* hr is 0-23 */
	tm->tm_mday = bcd2bin(buf[CCR_MDAY]);
	tm->tm_mon = bcd2bin(buf[CCR_MONTH]); /* mon is 0-11 */
	tm->tm_year = bcd2bin(buf[CCR_YEAR])
		+ (bcd2bin(buf[CCR_Y2K]) * 100);
	tm->tm_wday = buf[CCR_WDAY];

	debug("%s: tm is secs=%d, mins=%d, hours=%d, "
	      "mday=%d, mon=%d, year=%d, wday=%d\n",
	      __FUNCTION__,
	      tm->tm_sec, tm->tm_min, tm->tm_hour,
	      tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

	return 0;
}

int rtc_set(struct rtc_time *tm)
{
	int i;
	u8 buf[8];

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	buf[CCR_SEC] = bin2bcd(tm->tm_sec);
	buf[CCR_MIN] = bin2bcd(tm->tm_min);

	/* set hour and 24hr bit */
	buf[CCR_HOUR] = bin2bcd(tm->tm_hour) | X1205_HR_MIL;

	buf[CCR_MDAY] = bin2bcd(tm->tm_mday);

	/* month, 1 - 12 */
	buf[CCR_MONTH] = bin2bcd(tm->tm_mon);

	/* year, since the rtc epoch*/
	buf[CCR_YEAR] = bin2bcd(tm->tm_year % 100);
	buf[CCR_WDAY] = tm->tm_wday & 0x07;
	buf[CCR_Y2K] = bin2bcd(tm->tm_year / 100);

	/* this sequence is required to unlock the chip */
	rtc_write(X1205_REG_SR, X1205_SR_WEL);
	rtc_write(X1205_REG_SR, X1205_SR_WEL | X1205_SR_RWEL);

	/* write register's data */
	for (i = 0; i < 8; i++)
		rtc_write(X1205_CCR_BASE + i, buf[i]);

	rtc_write(X1205_REG_SR, 0);

	return 0;
}

void rtc_reset(void)
{
	/*
	 * Nothing to do
	 */
}
