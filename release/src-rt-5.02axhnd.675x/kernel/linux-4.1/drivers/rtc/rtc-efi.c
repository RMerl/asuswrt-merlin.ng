/*
 * rtc-efi: RTC Class Driver for EFI-based systems
 *
 * Copyright (C) 2009 Hewlett-Packard Development Company, L.P.
 *
 * Author: dann frazier <dannf@hp.com>
 * Based on efirtc.c by Stephane Eranian
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/stringify.h>
#include <linux/time.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/efi.h>

#define EFI_ISDST (EFI_TIME_ADJUST_DAYLIGHT|EFI_TIME_IN_DAYLIGHT)
/*
 * EFI Epoch is 1/1/1998
 */
#define EFI_RTC_EPOCH		1998

/*
 * returns day of the year [0-365]
 */
static inline int
compute_yday(efi_time_t *eft)
{
	/* efi_time_t.month is in the [1-12] so, we need -1 */
	return rtc_year_days(eft->day, eft->month - 1, eft->year);
}
/*
 * returns day of the week [0-6] 0=Sunday
 *
 * Don't try to provide a year that's before 1998, please !
 */
static int
compute_wday(efi_time_t *eft)
{
	int y;
	int ndays = 0;

	if (eft->year < EFI_RTC_EPOCH) {
		pr_err("EFI year < " __stringify(EFI_RTC_EPOCH) ", invalid date\n");
		return -1;
	}

	for (y = EFI_RTC_EPOCH; y < eft->year; y++)
		ndays += 365 + (is_leap_year(y) ? 1 : 0);

	ndays += compute_yday(eft);

	/*
	 * 4=1/1/1998 was a Thursday
	 */
	return (ndays + 4) % 7;
}

static void
convert_to_efi_time(struct rtc_time *wtime, efi_time_t *eft)
{
	eft->year	= wtime->tm_year + 1900;
	eft->month	= wtime->tm_mon + 1;
	eft->day	= wtime->tm_mday;
	eft->hour	= wtime->tm_hour;
	eft->minute	= wtime->tm_min;
	eft->second	= wtime->tm_sec;
	eft->nanosecond = 0;
	eft->daylight	= wtime->tm_isdst ? EFI_ISDST : 0;
	eft->timezone	= EFI_UNSPECIFIED_TIMEZONE;
}

static bool
convert_from_efi_time(efi_time_t *eft, struct rtc_time *wtime)
{
	memset(wtime, 0, sizeof(*wtime));

	if (eft->second >= 60)
		return false;
	wtime->tm_sec  = eft->second;

	if (eft->minute >= 60)
		return false;
	wtime->tm_min  = eft->minute;

	if (eft->hour >= 24)
		return false;
	wtime->tm_hour = eft->hour;

	if (!eft->day || eft->day > 31)
		return false;
	wtime->tm_mday = eft->day;

	if (!eft->month || eft->month > 12)
		return false;
	wtime->tm_mon  = eft->month - 1;
	wtime->tm_year = eft->year - 1900;

	/* day of the week [0-6], Sunday=0 */
	wtime->tm_wday = compute_wday(eft);
	if (wtime->tm_wday < 0)
		return false;

	/* day in the year [1-365]*/
	wtime->tm_yday = compute_yday(eft);


	switch (eft->daylight & EFI_ISDST) {
	case EFI_ISDST:
		wtime->tm_isdst = 1;
		break;
	case EFI_TIME_ADJUST_DAYLIGHT:
		wtime->tm_isdst = 0;
		break;
	default:
		wtime->tm_isdst = -1;
	}

	return true;
}

static int efi_read_alarm(struct device *dev, struct rtc_wkalrm *wkalrm)
{
	efi_time_t eft;
	efi_status_t status;

	/*
	 * As of EFI v1.10, this call always returns an unsupported status
	 */
	status = efi.get_wakeup_time((efi_bool_t *)&wkalrm->enabled,
				     (efi_bool_t *)&wkalrm->pending, &eft);

	if (status != EFI_SUCCESS)
		return -EINVAL;

	if (!convert_from_efi_time(&eft, &wkalrm->time))
		return -EIO;

	return rtc_valid_tm(&wkalrm->time);
}

static int efi_set_alarm(struct device *dev, struct rtc_wkalrm *wkalrm)
{
	efi_time_t eft;
	efi_status_t status;

	convert_to_efi_time(&wkalrm->time, &eft);

	/*
	 * XXX Fixme:
	 * As of EFI 0.92 with the firmware I have on my
	 * machine this call does not seem to work quite
	 * right
	 *
	 * As of v1.10, this call always returns an unsupported status
	 */
	status = efi.set_wakeup_time((efi_bool_t)wkalrm->enabled, &eft);

	dev_warn(dev, "write status is %d\n", (int)status);

	return status == EFI_SUCCESS ? 0 : -EINVAL;
}

static int efi_read_time(struct device *dev, struct rtc_time *tm)
{
	efi_status_t status;
	efi_time_t eft;
	efi_time_cap_t cap;

	status = efi.get_time(&eft, &cap);

	if (status != EFI_SUCCESS) {
		/* should never happen */
		dev_err(dev, "can't read time\n");
		return -EINVAL;
	}

	if (!convert_from_efi_time(&eft, tm))
		return -EIO;

	return rtc_valid_tm(tm);
}

static int efi_set_time(struct device *dev, struct rtc_time *tm)
{
	efi_status_t status;
	efi_time_t eft;

	convert_to_efi_time(tm, &eft);

	status = efi.set_time(&eft);

	return status == EFI_SUCCESS ? 0 : -EINVAL;
}

static const struct rtc_class_ops efi_rtc_ops = {
	.read_time = efi_read_time,
	.set_time = efi_set_time,
	.read_alarm = efi_read_alarm,
	.set_alarm = efi_set_alarm,
};

static int __init efi_rtc_probe(struct platform_device *dev)
{
	struct rtc_device *rtc;

	rtc = devm_rtc_device_register(&dev->dev, "rtc-efi", &efi_rtc_ops,
					THIS_MODULE);
	if (IS_ERR(rtc))
		return PTR_ERR(rtc);

	rtc->uie_unsupported = 1;
	platform_set_drvdata(dev, rtc);

	return 0;
}

static struct platform_driver efi_rtc_driver = {
	.driver = {
		.name = "rtc-efi",
	},
};

module_platform_driver_probe(efi_rtc_driver, efi_rtc_probe);

MODULE_ALIAS("platform:rtc-efi");
MODULE_AUTHOR("dann frazier <dannf@hp.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("EFI RTC driver");
MODULE_ALIAS("platform:rtc-efi");
