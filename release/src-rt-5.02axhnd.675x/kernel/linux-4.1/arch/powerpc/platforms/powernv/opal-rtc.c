/*
 * PowerNV Real Time Clock.
 *
 * Copyright 2011 IBM Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */


#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>

#include <asm/opal.h>
#include <asm/firmware.h>
#include <asm/machdep.h>

static void opal_to_tm(u32 y_m_d, u64 h_m_s_ms, struct rtc_time *tm)
{
	tm->tm_year	= ((bcd2bin(y_m_d >> 24) * 100) +
			   bcd2bin((y_m_d >> 16) & 0xff)) - 1900;
	tm->tm_mon	= bcd2bin((y_m_d >> 8) & 0xff) - 1;
	tm->tm_mday	= bcd2bin(y_m_d & 0xff);
	tm->tm_hour	= bcd2bin((h_m_s_ms >> 56) & 0xff);
	tm->tm_min	= bcd2bin((h_m_s_ms >> 48) & 0xff);
	tm->tm_sec	= bcd2bin((h_m_s_ms >> 40) & 0xff);

        GregorianDay(tm);
}

unsigned long __init opal_get_boot_time(void)
{
	struct rtc_time tm;
	u32 y_m_d;
	u64 h_m_s_ms;
	__be32 __y_m_d;
	__be64 __h_m_s_ms;
	long rc = OPAL_BUSY;

	if (!opal_check_token(OPAL_RTC_READ))
		return 0;

	while (rc == OPAL_BUSY || rc == OPAL_BUSY_EVENT) {
		rc = opal_rtc_read(&__y_m_d, &__h_m_s_ms);
		if (rc == OPAL_BUSY_EVENT)
			opal_poll_events(NULL);
		else
			mdelay(10);
	}
	if (rc != OPAL_SUCCESS)
		return 0;

	y_m_d = be32_to_cpu(__y_m_d);
	h_m_s_ms = be64_to_cpu(__h_m_s_ms);
	opal_to_tm(y_m_d, h_m_s_ms, &tm);
	return mktime(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		      tm.tm_hour, tm.tm_min, tm.tm_sec);
}

static __init int opal_time_init(void)
{
	struct platform_device *pdev;
	struct device_node *rtc;

	rtc = of_find_node_by_path("/ibm,opal/rtc");
	if (rtc) {
		pdev = of_platform_device_create(rtc, "opal-rtc", NULL);
		of_node_put(rtc);
	} else {
		if (opal_check_token(OPAL_RTC_READ) ||
		    opal_check_token(OPAL_READ_TPO))
			pdev = platform_device_register_simple("opal-rtc", -1,
							       NULL, 0);
		else
			return -ENODEV;
	}

	return PTR_ERR_OR_ZERO(pdev);
}
machine_subsys_initcall(powernv, opal_time_init);
