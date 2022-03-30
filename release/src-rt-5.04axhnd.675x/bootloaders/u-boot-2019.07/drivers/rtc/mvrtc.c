// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Jason Cooper <u-boot@lakedaemon.net>
 */

/*
 * Date & Time support for Marvell Integrated RTC
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <rtc.h>
#include <asm/io.h>
#include "mvrtc.h"

/* This RTC does not support century, so we assume 20 */
#define CENTURY 20

static int __mv_rtc_get(struct mvrtc_registers *regs, struct rtc_time *t)
{
	u32 time;
	u32 date;

	/* read the time register */
	time = readl(&regs->time);

	/* read the date register */
	date = readl(&regs->date);

	/* test for 12 hour clock (can't tell if it's am/pm) */
	if (time & MVRTC_HRFMT_MSK) {
		printf("Error: RTC in 12 hour mode, can't determine AM/PM.\n");
		return -1;
	}

	/* time */
	t->tm_sec  = bcd2bin((time >> MVRTC_SEC_SFT)  & MVRTC_SEC_MSK);
	t->tm_min  = bcd2bin((time >> MVRTC_MIN_SFT)  & MVRTC_MIN_MSK);
	t->tm_hour = bcd2bin((time >> MVRTC_HOUR_SFT) & MVRTC_HOUR_MSK);
	t->tm_wday = bcd2bin((time >> MVRTC_DAY_SFT)  & MVRTC_DAY_MSK);
	t->tm_wday--;

	/* date */
	t->tm_mday = bcd2bin((date >> MVRTC_DATE_SFT) & MVRTC_DATE_MSK);
	t->tm_mon  = bcd2bin((date >> MVRTC_MON_SFT)  & MVRTC_MON_MSK);
	t->tm_year = bcd2bin((date >> MVRTC_YEAR_SFT) & MVRTC_YEAR_MSK);
	t->tm_year += CENTURY * 100;

	/* not supported in this RTC */
	t->tm_yday  = 0;
	t->tm_isdst = 0;

	return 0;
}

#ifndef CONFIG_DM_RTC
int rtc_get(struct rtc_time *t)
{
	struct mvrtc_registers *regs;

	regs = (struct mvrtc_registers *)KW_RTC_BASE;
	return __mv_rtc_get(regs, t);
}
#endif /* !CONFIG_DM_RTC */

static int __mv_rtc_set(struct mvrtc_registers *regs, const struct rtc_time *t)
{
	u32 time = 0; /* sets hour format bit to zero, 24hr format. */
	u32 date = 0;

	/* check that this code isn't 80+ years old ;-) */
	if ((t->tm_year / 100) != CENTURY)
		printf("Warning: Only century %d supported.\n", CENTURY);

	/* time */
	time |= (bin2bcd(t->tm_sec)      & MVRTC_SEC_MSK)  << MVRTC_SEC_SFT;
	time |= (bin2bcd(t->tm_min)      & MVRTC_MIN_MSK)  << MVRTC_MIN_SFT;
	time |= (bin2bcd(t->tm_hour)     & MVRTC_HOUR_MSK) << MVRTC_HOUR_SFT;
	time |= (bin2bcd(t->tm_wday + 1) & MVRTC_DAY_MSK)  << MVRTC_DAY_SFT;

	/* date */
	date |= (bin2bcd(t->tm_mday)       & MVRTC_DATE_MSK) << MVRTC_DATE_SFT;
	date |= (bin2bcd(t->tm_mon)        & MVRTC_MON_MSK)  << MVRTC_MON_SFT;
	date |= (bin2bcd(t->tm_year % 100) & MVRTC_YEAR_MSK) << MVRTC_YEAR_SFT;

	/* write the time register */
	writel(time, &regs->time);

	/* write the date register */
	writel(date, &regs->date);

	return 0;
}

#ifndef CONFIG_DM_RTC
int rtc_set(struct rtc_time *t)
{
	struct mvrtc_registers *regs;

	regs = (struct mvrtc_registers *)KW_RTC_BASE;
	return __mv_rtc_set(regs, t);
}
#endif /* !CONFIG_DM_RTC */

static void __mv_rtc_reset(struct mvrtc_registers *regs)
{
	u32 time;
	u32 sec;

	/* no init routine for this RTC needed, just check that it's working */
	time = readl(&regs->time);
	sec  = bcd2bin((time >> MVRTC_SEC_SFT) & MVRTC_SEC_MSK);
	udelay(1000000);
	time = readl(&regs->time);

	if (sec == bcd2bin((time >> MVRTC_SEC_SFT) & MVRTC_SEC_MSK))
		printf("Error: RTC did not increment.\n");
}

#ifndef CONFIG_DM_RTC
void rtc_reset(void)
{
	struct mvrtc_registers *regs;

	regs = (struct mvrtc_registers *)KW_RTC_BASE;
	__mv_rtc_reset(regs);
}
#endif /* !CONFIG_DM_RTC */

#ifdef CONFIG_DM_RTC
static int mv_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	struct mvrtc_pdata *pdata = dev_get_platdata(dev);
	struct mvrtc_registers *regs = (struct mvrtc_registers *)pdata->iobase;

	return __mv_rtc_get(regs, tm);
}

static int mv_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	struct mvrtc_pdata *pdata = dev_get_platdata(dev);
	struct mvrtc_registers *regs = (struct mvrtc_registers *)pdata->iobase;

	return __mv_rtc_set(regs, tm);
}

static int mv_rtc_reset(struct udevice *dev)
{
	struct mvrtc_pdata *pdata = dev_get_platdata(dev);
	struct mvrtc_registers *regs = (struct mvrtc_registers *)pdata->iobase;

	__mv_rtc_reset(regs);
	return 0;
}

static const struct rtc_ops mv_rtc_ops = {
	.get = mv_rtc_get,
	.set = mv_rtc_set,
	.reset = mv_rtc_reset,
};

static const struct udevice_id mv_rtc_ids[] = {
	{ .compatible = "marvell,kirkwood-rtc" },
	{ .compatible = "marvell,orion-rtc" },
	{ }
};

static int mv_rtc_ofdata_to_platdata(struct udevice *dev)
{
	struct mvrtc_pdata *pdata = dev_get_platdata(dev);

	pdata->iobase = devfdt_get_addr(dev);
	return 0;
}

U_BOOT_DRIVER(rtc_mv) = {
	.name	= "rtc-mv",
	.id	= UCLASS_RTC,
	.ofdata_to_platdata = mv_rtc_ofdata_to_platdata,
	.of_match = mv_rtc_ids,
	.ops	= &mv_rtc_ops,
};
#endif /* CONFIG_DM_RTC */
