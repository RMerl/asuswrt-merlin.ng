// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 *
 * Author:	Priyanka Jain <Priyanka.Jain@freescale.com>
 */

/*
 * This file provides Date & Time support (no alarms) for PT7C4338 chip.
 *
 * This file is based on drivers/rtc/ds1337.c
 *
 * PT7C4338 chip is manufactured by Pericom Technology Inc.
 * It is a serial real-time clock which provides
 * 1)Low-power clock/calendar.
 * 2)Programmable square-wave output.
 * It has 56 bytes of nonvolatile RAM.
 */

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <i2c.h>

/* RTC register addresses */
#define RTC_SEC_REG_ADDR        0x0
#define RTC_MIN_REG_ADDR        0x1
#define RTC_HR_REG_ADDR         0x2
#define RTC_DAY_REG_ADDR        0x3
#define RTC_DATE_REG_ADDR       0x4
#define RTC_MON_REG_ADDR        0x5
#define RTC_YR_REG_ADDR         0x6
#define RTC_CTL_STAT_REG_ADDR   0x7

/* RTC second register address bit */
#define RTC_SEC_BIT_CH		0x80	/* Clock Halt (in Register 0) */

/* RTC control and status register bits */
#define RTC_CTL_STAT_BIT_RS0    0x1	/* Rate select 0 */
#define RTC_CTL_STAT_BIT_RS1    0x2	/* Rate select 1 */
#define RTC_CTL_STAT_BIT_SQWE   0x10	/* Square Wave Enable */
#define RTC_CTL_STAT_BIT_OSF    0x20	/* Oscillator Stop Flag */
#define RTC_CTL_STAT_BIT_OUT    0x80	/* Output Level Control */

/* RTC reset value */
#define RTC_PT7C4338_RESET_VAL \
	(RTC_CTL_STAT_BIT_RS0 | RTC_CTL_STAT_BIT_RS1 | RTC_CTL_STAT_BIT_OUT)

/****** Helper functions ****************************************/
static u8 rtc_read(u8 reg)
{
	return i2c_reg_read(CONFIG_SYS_I2C_RTC_ADDR, reg);
}

static void rtc_write(u8 reg, u8 val)
{
	i2c_reg_write(CONFIG_SYS_I2C_RTC_ADDR, reg, val);
}
/****************************************************************/

/* Get the current time from the RTC */
int rtc_get(struct rtc_time *tmp)
{
	int ret = 0;
	u8 sec, min, hour, mday, wday, mon, year, ctl_stat;

	ctl_stat = rtc_read(RTC_CTL_STAT_REG_ADDR);
	sec = rtc_read(RTC_SEC_REG_ADDR);
	min = rtc_read(RTC_MIN_REG_ADDR);
	hour = rtc_read(RTC_HR_REG_ADDR);
	wday = rtc_read(RTC_DAY_REG_ADDR);
	mday = rtc_read(RTC_DATE_REG_ADDR);
	mon = rtc_read(RTC_MON_REG_ADDR);
	year = rtc_read(RTC_YR_REG_ADDR);
	debug("Get RTC year: %02x mon: %02x mday: %02x wday: %02x "
		"hr: %02x min: %02x sec: %02x control_status: %02x\n",
		year, mon, mday, wday, hour, min, sec, ctl_stat);

	if (ctl_stat & RTC_CTL_STAT_BIT_OSF) {
		printf("### Warning: RTC oscillator has stopped\n");
		/* clear the OSF flag */
		rtc_write(RTC_CTL_STAT_REG_ADDR,
			rtc_read(RTC_CTL_STAT_REG_ADDR)\
			& ~RTC_CTL_STAT_BIT_OSF);
		ret = -1;
	}

	tmp->tm_sec = bcd2bin(sec & 0x7F);
	tmp->tm_min = bcd2bin(min & 0x7F);
	tmp->tm_hour = bcd2bin(hour & 0x3F);
	tmp->tm_mday = bcd2bin(mday & 0x3F);
	tmp->tm_mon = bcd2bin(mon & 0x1F);
	tmp->tm_year = bcd2bin(year) + 2000;
	tmp->tm_wday = bcd2bin((wday - 1) & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;
	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return ret;
}

/* Set the RTC */
int rtc_set(struct rtc_time *tmp)
{
	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	rtc_write(RTC_YR_REG_ADDR, bin2bcd(tmp->tm_year % 100));
	rtc_write(RTC_MON_REG_ADDR, bin2bcd(tmp->tm_mon));
	rtc_write(RTC_DAY_REG_ADDR, bin2bcd(tmp->tm_wday + 1));
	rtc_write(RTC_DATE_REG_ADDR, bin2bcd(tmp->tm_mday));
	rtc_write(RTC_HR_REG_ADDR, bin2bcd(tmp->tm_hour));
	rtc_write(RTC_MIN_REG_ADDR, bin2bcd(tmp->tm_min));
	rtc_write(RTC_SEC_REG_ADDR, bin2bcd(tmp->tm_sec));

	return 0;
}

/* Reset the RTC */
void rtc_reset(void)
{
	rtc_write(RTC_SEC_REG_ADDR, 0x00);	/* clearing Clock Halt	*/
	rtc_write(RTC_CTL_STAT_REG_ADDR, RTC_PT7C4338_RESET_VAL);
}
