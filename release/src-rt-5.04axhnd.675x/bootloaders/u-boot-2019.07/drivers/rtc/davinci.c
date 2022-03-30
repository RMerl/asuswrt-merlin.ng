// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 DENX Software Engineering GmbH
 * Heiko Schocher <hs@denx.de>
 */
#include <common.h>
#include <command.h>
#include <rtc.h>
#include <asm/io.h>
#include <asm/davinci_rtc.h>

int rtc_get(struct rtc_time *tmp)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)DAVINCI_RTC_BASE;
	unsigned long sec, min, hour, mday, wday, mon_cent, year;
	unsigned long status;

	status = readl(&rtc->status);
	if ((status & RTC_STATE_RUN) != RTC_STATE_RUN) {
		printf("RTC doesn't run\n");
		return -1;
	}
	if ((status & RTC_STATE_BUSY) == RTC_STATE_BUSY)
		udelay(20);

	sec	= readl(&rtc->second);
	min	= readl(&rtc->minutes);
	hour	= readl(&rtc->hours);
	mday	= readl(&rtc->day);
	wday	= readl(&rtc->dotw);
	mon_cent = readl(&rtc->month);
	year	= readl(&rtc->year);

	debug("Get RTC year: %02lx mon/cent: %02lx mday: %02lx wday: %02lx "
		"hr: %02lx min: %02lx sec: %02lx\n",
		year, mon_cent, mday, wday,
		hour, min, sec);

	tmp->tm_sec  = bcd2bin(sec  & 0x7F);
	tmp->tm_min  = bcd2bin(min  & 0x7F);
	tmp->tm_hour = bcd2bin(hour & 0x3F);
	tmp->tm_mday = bcd2bin(mday & 0x3F);
	tmp->tm_mon  = bcd2bin(mon_cent & 0x1F);
	tmp->tm_year = bcd2bin(year) + 2000;
	tmp->tm_wday = bcd2bin(wday & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

int rtc_set(struct rtc_time *tmp)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)DAVINCI_RTC_BASE;

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
	writel(bin2bcd(tmp->tm_year % 100), &rtc->year);
	writel(bin2bcd(tmp->tm_mon), &rtc->month);

	writel(bin2bcd(tmp->tm_wday), &rtc->dotw);
	writel(bin2bcd(tmp->tm_mday), &rtc->day);
	writel(bin2bcd(tmp->tm_hour), &rtc->hours);
	writel(bin2bcd(tmp->tm_min), &rtc->minutes);
	writel(bin2bcd(tmp->tm_sec), &rtc->second);
	return 0;
}

void rtc_reset(void)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)DAVINCI_RTC_BASE;

	/* run RTC counter */
	writel(0x01, &rtc->ctrl);
}
