// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 *
 * Watchdog timer for MT7620 and earlier SoCs
 */

#include <div64.h>
#include <dm.h>
#include <reset.h>
#include <wdt.h>
#include <linux/bitops.h>
#include <linux/io.h>

struct mt7620_wdt {
	void __iomem *regs;
	u64 timeout;
};

#define TIMER_FREQ			40000000
#define TIMER_MASK			0xffff
#define TIMER_PRESCALE			65536

#define TIMER_LOAD			0x00
#define TIMER_CTL			0x08

#define TIMER_ENABLE			BIT(7)
#define TIMER_MODE_SHIFT		4
#define   TIMER_MODE_WDT		3
#define TIMER_PRESCALE_SHIFT		0
#define   TIMER_PRESCALE_65536		15

static void mt7620_wdt_ping(struct mt7620_wdt *priv)
{
	u64 val;

	val = (TIMER_FREQ / TIMER_PRESCALE) * priv->timeout;
	do_div(val, 1000);

	if (val > TIMER_MASK)
		val = TIMER_MASK;

	writel(val, priv->regs + TIMER_LOAD);
}

static int mt7620_wdt_start(struct udevice *dev, u64 ms, ulong flags)
{
	struct mt7620_wdt *priv = dev_get_priv(dev);

	priv->timeout = ms;
	mt7620_wdt_ping(priv);

	writel(TIMER_ENABLE | (TIMER_MODE_WDT << TIMER_MODE_SHIFT) |
	       (TIMER_PRESCALE_65536 << TIMER_PRESCALE_SHIFT),
	       priv->regs + TIMER_CTL);

	return 0;
}

static int mt7620_wdt_stop(struct udevice *dev)
{
	struct mt7620_wdt *priv = dev_get_priv(dev);

	mt7620_wdt_ping(priv);

	clrbits_32(priv->regs + TIMER_CTL, TIMER_ENABLE);

	return 0;
}

static int mt7620_wdt_reset(struct udevice *dev)
{
	struct mt7620_wdt *priv = dev_get_priv(dev);

	mt7620_wdt_ping(priv);

	return 0;
}

static int mt7620_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct mt7620_wdt *priv = dev_get_priv(dev);

	mt7620_wdt_start(dev, 1, flags);

	/*
	 * 0 will disable the timer directly, a positive number must be used
	 * instead. Since the timer is a countdown timer, 1 (tick) is used.
	 *
	 * For a timer with input clock = 40MHz, 1 timer tick is short
	 * enough to trigger a timeout immediately.
	 *
	 * Restore prescale to 1, and load timer with 1 to trigger timeout.
	 */
	writel(TIMER_ENABLE | (TIMER_MODE_WDT << TIMER_MODE_SHIFT),
	       priv->regs + TIMER_CTL);
	writel(1, priv->regs + TIMER_LOAD);

	return 0;
}

static int mt7620_wdt_probe(struct udevice *dev)
{
	struct mt7620_wdt *priv = dev_get_priv(dev);
	struct reset_ctl reset_wdt;
	int ret;

	ret = reset_get_by_index(dev, 0, &reset_wdt);
	if (!ret)
		reset_deassert(&reset_wdt);

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	mt7620_wdt_stop(dev);

	return 0;
}

static const struct wdt_ops mt7620_wdt_ops = {
	.start = mt7620_wdt_start,
	.reset = mt7620_wdt_reset,
	.stop = mt7620_wdt_stop,
	.expire_now = mt7620_wdt_expire_now,
};

static const struct udevice_id mt7620_wdt_ids[] = {
	{ .compatible = "mediatek,mt7620-wdt" },
	{}
};

U_BOOT_DRIVER(mt7620_wdt) = {
	.name = "mt7620_wdt",
	.id = UCLASS_WDT,
	.of_match = mt7620_wdt_ids,
	.probe = mt7620_wdt_probe,
	.priv_auto = sizeof(struct mt7620_wdt),
	.ops = &mt7620_wdt_ops,
};
