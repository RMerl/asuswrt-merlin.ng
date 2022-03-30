// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gururaja Hebbar gururajakr@sanyo.co.in
 *
 * reference linux-2.6.20.6/drivers/rtc/rtc-pl031.c
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <rtc.h>
#include <asm/io.h>
#include <asm/types.h>

/*
 * Register definitions
 */
#define	RTC_DR		0x00	/* Data read register */
#define	RTC_MR		0x04	/* Match register */
#define	RTC_LR		0x08	/* Data load register */
#define	RTC_CR		0x0c	/* Control register */
#define	RTC_IMSC	0x10	/* Interrupt mask and set register */
#define	RTC_RIS		0x14	/* Raw interrupt status register */
#define	RTC_MIS		0x18	/* Masked interrupt status register */
#define	RTC_ICR		0x1c	/* Interrupt clear register */

#define RTC_CR_START	(1 << 0)

struct pl031_platdata {
	phys_addr_t base;
};

static inline u32 pl031_read_reg(struct udevice *dev, int reg)
{
	struct pl031_platdata *pdata = dev_get_platdata(dev);

	return readl(pdata->base + reg);
}

static inline u32 pl031_write_reg(struct udevice *dev, int reg, u32 value)
{
	struct pl031_platdata *pdata = dev_get_platdata(dev);

	return writel(value, pdata->base + reg);
}

/*
 * Probe RTC device
 */
static int pl031_probe(struct udevice *dev)
{
	/* Enable RTC Start in Control register*/
	pl031_write_reg(dev, RTC_CR, RTC_CR_START);

	return 0;
}

/*
 * Get the current time from the RTC
 */
static int pl031_get(struct udevice *dev, struct rtc_time *tm)
{
	unsigned long tim;

	if (!tm)
		return -EINVAL;

	tim = pl031_read_reg(dev, RTC_DR);

	rtc_to_tm(tim, tm);

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

/*
 * Set the RTC
 */
static int pl031_set(struct udevice *dev, const struct rtc_time *tm)
{
	unsigned long tim;

	if (!tm)
		return -EINVAL;

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	/* Calculate number of seconds this incoming time represents */
	tim = rtc_mktime(tm);

	pl031_write_reg(dev, RTC_LR, tim);

	return 0;
}

/*
 * Reset the RTC. We set the date back to 1970-01-01.
 */
static int pl031_reset(struct udevice *dev)
{
	pl031_write_reg(dev, RTC_LR, 0);

	return 0;
}

static const struct rtc_ops pl031_ops = {
	.get = pl031_get,
	.set = pl031_set,
	.reset = pl031_reset,
};

static const struct udevice_id pl031_ids[] = {
	{ .compatible = "arm,pl031" },
	{ }
};

static int pl031_ofdata_to_platdata(struct udevice *dev)
{
	struct pl031_platdata *pdata = dev_get_platdata(dev);

	pdata->base = dev_read_addr(dev);

	return 0;
}

U_BOOT_DRIVER(rtc_pl031) = {
	.name	= "rtc-pl031",
	.id	= UCLASS_RTC,
	.of_match = pl031_ids,
	.probe	= pl031_probe,
	.ofdata_to_platdata = pl031_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct pl031_platdata),
	.ops	= &pl031_ops,
};
