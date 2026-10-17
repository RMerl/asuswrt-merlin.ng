// SPDX-License-Identifier: GPL-2.0+
/*
 * RTC driver for the Armada 38x Marvell SoCs
 *
 * Copyright (C) 2021 Marek Behún <kabel@kernel.org>
 *
 * Based on Linux' driver by Gregory Clement and Marvell
 */

#include <asm/io.h>
#include <dm.h>
#include <linux/delay.h>
#include <rtc.h>

#define RTC_STATUS			0x0
#define RTC_TIME			0xC
#define RTC_CONF_TEST			0x1C

/* Armada38x SoC registers  */
#define RTC_38X_BRIDGE_TIMING_CTL	0x0
#define RTC_38X_PERIOD_OFFS		0
#define RTC_38X_PERIOD_MASK		(0x3FF << RTC_38X_PERIOD_OFFS)
#define RTC_38X_READ_DELAY_OFFS		26
#define RTC_38X_READ_DELAY_MASK		(0x1F << RTC_38X_READ_DELAY_OFFS)

#define SAMPLE_NR			100

struct armada38x_rtc {
	void __iomem *regs;
	void __iomem *regs_soc;
};

/*
 * According to Erratum RES-3124064 we have to do some configuration in MBUS.
 * To read an RTC register we need to read it 100 times and return the most
 * frequent value.
 * To write an RTC register we need to write 2x zero into STATUS register,
 * followed by the proper write. Linux adds an 5 us delay after this, so we do
 * it here as well.
 */
static void update_38x_mbus_timing_params(struct armada38x_rtc *rtc)
{
	u32 reg;

	reg = readl(rtc->regs_soc + RTC_38X_BRIDGE_TIMING_CTL);
	reg &= ~RTC_38X_PERIOD_MASK;
	reg |= 0x3FF << RTC_38X_PERIOD_OFFS; /* Maximum value */
	reg &= ~RTC_38X_READ_DELAY_MASK;
	reg |= 0x1F << RTC_38X_READ_DELAY_OFFS; /* Maximum value */
	writel(reg, rtc->regs_soc + RTC_38X_BRIDGE_TIMING_CTL);
}

static void armada38x_rtc_write(u32 val, struct armada38x_rtc *rtc, u8 reg)
{
	writel(0, rtc->regs + RTC_STATUS);
	writel(0, rtc->regs + RTC_STATUS);
	writel(val, rtc->regs + reg);
	udelay(5);
}

static u32 armada38x_rtc_read(struct armada38x_rtc *rtc, u8 reg)
{
	u8 counts[SAMPLE_NR], max_idx;
	u32 samples[SAMPLE_NR], max;
	int i, j, last;

	for (i = 0, last = 0; i < SAMPLE_NR; ++i) {
		u32 sample = readl(rtc->regs + reg);

		/* find if this value was already read */
		for (j = 0; j < last; ++j) {
			if (samples[j] == sample)
				break;
		}

		if (j < last) {
			/* if yes, increment count */
			++counts[j];
		} else {
			/* if not, add */
			samples[last] = sample;
			counts[last] = 1;
			++last;
		}
	}

	/* finally find the sample that was read the most */
	max = 0;
	max_idx = 0;

	for (i = 0; i < last; ++i) {
		if (counts[i] > max) {
			max = counts[i];
			max_idx = i;
		}
	}

	return samples[max_idx];
}

static int armada38x_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	struct armada38x_rtc *rtc = dev_get_priv(dev);
	u32 time;

	time = armada38x_rtc_read(rtc, RTC_TIME);

	rtc_to_tm(time, tm);

	return 0;
}

static int armada38x_rtc_reset(struct udevice *dev)
{
	struct armada38x_rtc *rtc = dev_get_priv(dev);
	u32 reg;

	reg = armada38x_rtc_read(rtc, RTC_CONF_TEST);

	if (reg & 0xff) {
		armada38x_rtc_write(0, rtc, RTC_CONF_TEST);
		mdelay(500);
		armada38x_rtc_write(0, rtc, RTC_TIME);
		armada38x_rtc_write(BIT(0) | BIT(1), rtc, RTC_STATUS);
	}

	return 0;
}

static int armada38x_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	struct armada38x_rtc *rtc = dev_get_priv(dev);
	unsigned long time;

	time = rtc_mktime(tm);

	if (time > U32_MAX)
		printf("%s: requested time to set will overflow\n", dev->name);

	armada38x_rtc_reset(dev);
	armada38x_rtc_write(time, rtc, RTC_TIME);

	return 0;
}

static int armada38x_probe(struct udevice *dev)
{
	struct armada38x_rtc *rtc = dev_get_priv(dev);

	rtc->regs = dev_remap_addr_name(dev, "rtc");
	if (!rtc->regs)
		goto err;

	rtc->regs_soc = dev_remap_addr_name(dev, "rtc-soc");
	if (!rtc->regs_soc)
		goto err;

	update_38x_mbus_timing_params(rtc);

	return 0;
err:
	printf("%s: io address missing\n", dev->name);
	return -ENODEV;
}

static const struct rtc_ops armada38x_rtc_ops = {
	.get = armada38x_rtc_get,
	.set = armada38x_rtc_set,
	.reset = armada38x_rtc_reset,
};

static const struct udevice_id armada38x_rtc_ids[] = {
	{ .compatible = "marvell,armada-380-rtc", .data = 0 },
	{ }
};

U_BOOT_DRIVER(rtc_armada38x) = {
	.name		= "rtc-armada38x",
	.id		= UCLASS_RTC,
	.of_match	= armada38x_rtc_ids,
	.probe		= armada38x_probe,
	.priv_auto	= sizeof(struct armada38x_rtc),
	.ops		= &armada38x_rtc_ops,
};
