// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023, Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 *
 * This driver emulates a real time clock based on timer ticks.
 */

#include <div64.h>
#include <dm.h>
#include <mapmem.h>
#include <rtc.h>
#include <linux/io.h>

/**
 * struct goldfish_rtc - private data for RTC driver
 */
struct goldfish_rtc {
	/**
	 * @base: base address for register file
	 */
	void __iomem *base;
	/**
	 * @isdst: daylight saving time
	 */
	int isdst;
};

/* Register offsets */
#define GOLDFISH_TIME_LOW	0x00
#define GOLDFISH_TIME_HIGH	0x04

static int goldfish_rtc_get(struct udevice *dev, struct rtc_time *time)
{
	struct goldfish_rtc *priv = dev_get_priv(dev);
	void __iomem *base = priv->base;
	u64 time_high;
	u64 time_low;
	u64 now;

	time_low = ioread32(base + GOLDFISH_TIME_LOW);
	time_high = ioread32(base + GOLDFISH_TIME_HIGH);
	now = (time_high << 32) | time_low;

	do_div(now, 1000000000U);

	rtc_to_tm(now, time);
	time->tm_isdst = priv->isdst;

	return 0;
}

static int goldfish_rtc_set(struct udevice *dev, const struct rtc_time *time)
{
	struct goldfish_rtc *priv = dev_get_priv(dev);
	void __iomem *base = priv->base;
	u64 now;

	if (time->tm_year < 1970)
		return -EINVAL;

	now = rtc_mktime(time) * 1000000000ULL;
	iowrite32(now >> 32, base + GOLDFISH_TIME_HIGH);
	iowrite32(now, base + GOLDFISH_TIME_LOW);

	if (time->tm_isdst > 0)
		priv->isdst = 1;
	else if (time->tm_isdst < 0)
		priv->isdst = -1;
	else
		priv->isdst = 0;

	return 0;
}

int goldfish_rtc_probe(struct udevice *dev)
{
	struct goldfish_rtc *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	priv->base = map_sysmem(addr, 0x20);

	return 0;
}

static const struct rtc_ops goldfish_rtc_ops = {
	.get = goldfish_rtc_get,
	.set = goldfish_rtc_set,
};

static const struct udevice_id goldfish_rtc_of_match[] = {
	{ .compatible = "google,goldfish-rtc", },
	{},
};

U_BOOT_DRIVER(rtc_goldfish) = {
	.name		= "rtc_goldfish",
	.id		= UCLASS_RTC,
	.ops		= &goldfish_rtc_ops,
	.probe		= goldfish_rtc_probe,
	.of_match	= goldfish_rtc_of_match,
	.priv_auto	= sizeof(struct goldfish_rtc),
};
