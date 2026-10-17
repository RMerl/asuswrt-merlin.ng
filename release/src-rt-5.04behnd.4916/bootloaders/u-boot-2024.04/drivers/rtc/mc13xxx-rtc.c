// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2008, Guennadi Liakhovetski <lg@denx.de>
 */

#include <common.h>
#include <rtc.h>
#include <spi.h>
#include <power/pmic.h>
#include <fsl_pmic.h>

int rtc_get(struct rtc_time *rtc)
{
	u32 day1, day2, time;
	int tim, i = 0;
	struct pmic *p = pmic_get("FSL_PMIC");
	int ret;

	if (!p)
		return -1;
	do {
		ret = pmic_reg_read(p, REG_RTC_DAY, &day1);
		if (ret < 0)
			return -1;

		ret = pmic_reg_read(p, REG_RTC_TIME, &time);
		if (ret < 0)
			return -1;

		ret = pmic_reg_read(p, REG_RTC_DAY, &day2);
		if (ret < 0)
			return -1;

	} while (day1 != day2 && i++ < 3);

	tim = day1 * 86400 + time;

	rtc_to_tm(tim, rtc);

	rtc->tm_yday = 0;
	rtc->tm_isdst = 0;

	return 0;
}

int rtc_set(struct rtc_time *rtc)
{
	u32 time, day;
	struct pmic *p = pmic_get("FSL_PMIC");
	if (!p)
		return -1;

	time = rtc_mktime(rtc);
	day = time / 86400;
	time %= 86400;

	pmic_reg_write(p, REG_RTC_DAY, day);
	pmic_reg_write(p, REG_RTC_TIME, time);

	return 0;
}

void rtc_reset(void)
{
}
