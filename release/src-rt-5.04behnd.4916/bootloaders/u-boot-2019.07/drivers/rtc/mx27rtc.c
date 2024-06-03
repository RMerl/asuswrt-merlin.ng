// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX27 RTC Driver
 *
 * Copyright (C) 2012 Philippe Reynes <tremyfr@yahoo.fr>
 */

#include <common.h>
#include <rtc.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>

#define HOUR_SHIFT 8
#define HOUR_MASK  0x1f
#define MIN_SHIFT  0
#define MIN_MASK   0x3f

int rtc_get(struct rtc_time *time)
{
	struct rtc_regs *rtc_regs = (struct rtc_regs *)IMX_RTC_BASE;
	uint32_t day, hour, min, sec;

	day  = readl(&rtc_regs->dayr);
	hour = readl(&rtc_regs->hourmin);
	sec  = readl(&rtc_regs->seconds);

	min  = (hour >> MIN_SHIFT) & MIN_MASK;
	hour = (hour >> HOUR_SHIFT) & HOUR_MASK;

	sec += min * 60 + hour * 3600 + day * 24 * 3600;

	rtc_to_tm(sec, time);

	return 0;
}

int rtc_set(struct rtc_time *time)
{
	struct rtc_regs *rtc_regs = (struct rtc_regs *)IMX_RTC_BASE;
	uint32_t day, hour, min, sec;

	sec = rtc_mktime(time);

	day  = sec / (24 * 3600);
	sec  = sec % (24 * 3600);
	hour = sec / 3600;
	sec  = sec % 3600;
	min  = sec / 60;
	sec  = sec % 60;

	hour  = (hour & HOUR_MASK) << HOUR_SHIFT;
	hour |= (min & MIN_MASK) << MIN_SHIFT;

	writel(day, &rtc_regs->dayr);
	writel(hour, &rtc_regs->hourmin);
	writel(sec, &rtc_regs->seconds);

	return 0;
}

void rtc_reset(void)
{
	/* nothing to do */
}
