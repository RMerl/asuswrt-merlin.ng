// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This driver emulates a real time clock based on timer ticks.
 */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <env.h>
#include <rtc.h>
#include <timestamp.h>

/**
 * struct emul_rtc - private data for emulated RTC driver
 */
struct emul_rtc {
	/**
	 * @offset_us: microseconds from 1970-01-01 to timer_get_us() base
	 */
	u64 offset_us;
	/**
	 * @isdst: daylight saving time
	 */
	int isdst;
};

static int emul_rtc_get(struct udevice *dev, struct rtc_time *time)
{
	struct emul_rtc *priv = dev_get_priv(dev);
	u64 now;

	now = timer_get_us() + priv->offset_us;
	do_div(now, 1000000);
	rtc_to_tm(now, time);
	time->tm_isdst = priv->isdst;

	return 0;
}

static int emul_rtc_set(struct udevice *dev, const struct rtc_time *time)
{
	struct emul_rtc *priv = dev_get_priv(dev);

	if (time->tm_year < 1970)
		return -EINVAL;

	priv->offset_us = rtc_mktime(time) * 1000000ULL - timer_get_us();

	if (time->tm_isdst > 0)
		priv->isdst = 1;
	else if (time->tm_isdst < 0)
		priv->isdst = -1;
	else
		priv->isdst = 0;

	return 0;
}

int emul_rtc_probe(struct udevice *dev)
{
	struct emul_rtc *priv = dev_get_priv(dev);
	const char *epoch_str;
	u64 epoch;

	epoch_str = env_get("rtc_emul_epoch");

	if (epoch_str) {
		epoch = simple_strtoull(epoch_str, NULL, 10);
	} else {
		/* Use the build date as initial time */
		epoch = U_BOOT_EPOCH;
	}
	priv->offset_us = epoch * 1000000ULL - timer_get_us();
	priv->isdst = -1;

	return 0;
}

static const struct rtc_ops emul_rtc_ops = {
	.get = emul_rtc_get,
	.set = emul_rtc_set,
};

U_BOOT_DRIVER(rtc_emul) = {
	.name	= "rtc_emul",
	.id	= UCLASS_RTC,
	.ops	= &emul_rtc_ops,
	.probe	= emul_rtc_probe,
	.priv_auto	= sizeof(struct emul_rtc),
};

U_BOOT_DRVINFO(rtc_emul) = {
	.name	= "rtc_emul",
};
