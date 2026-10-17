// SPDX-License-Identifier: GPL-2.0+
/*
 * Ralink / Mediatek RT288x/RT3xxx/MT76xx built-in hardware watchdog timer
 *
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 *
 * Based on the Linux driver version which is:
 *   Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *   Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct mt762x_wdt {
	void __iomem *regs;
};

#define TIMER_REG_TMRSTAT		0x00
#define TIMER_REG_TMR1CTL		0x20
#define TIMER_REG_TMR1LOAD		0x24

#define TMR1CTL_ENABLE			BIT(7)
#define TMR1CTL_RESTART			BIT(9)
#define TMR1CTL_PRESCALE_SHIFT		16

static int mt762x_wdt_ping(struct mt762x_wdt *priv)
{
	writel(TMR1CTL_RESTART, priv->regs + TIMER_REG_TMRSTAT);

	return 0;
}

static int mt762x_wdt_start(struct udevice *dev, u64 ms, ulong flags)
{
	struct mt762x_wdt *priv = dev_get_priv(dev);

	/* set the prescaler to 1ms == 1000us */
	writel(1000 << TMR1CTL_PRESCALE_SHIFT, priv->regs + TIMER_REG_TMR1CTL);
	writel(ms, priv->regs + TIMER_REG_TMR1LOAD);

	setbits_le32(priv->regs + TIMER_REG_TMR1CTL, TMR1CTL_ENABLE);

	return 0;
}

static int mt762x_wdt_stop(struct udevice *dev)
{
	struct mt762x_wdt *priv = dev_get_priv(dev);

	mt762x_wdt_ping(priv);

	clrbits_le32(priv->regs + TIMER_REG_TMR1CTL, TMR1CTL_ENABLE);

	return 0;
}

static int mt762x_wdt_reset(struct udevice *dev)
{
	struct mt762x_wdt *priv = dev_get_priv(dev);

	mt762x_wdt_ping(priv);

	return 0;
}

static int mt762x_wdt_probe(struct udevice *dev)
{
	struct mt762x_wdt *priv = dev_get_priv(dev);

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	mt762x_wdt_stop(dev);

	return 0;
}

static const struct wdt_ops mt762x_wdt_ops = {
	.start = mt762x_wdt_start,
	.reset = mt762x_wdt_reset,
	.stop = mt762x_wdt_stop,
};

static const struct udevice_id mt762x_wdt_ids[] = {
	{ .compatible = "mediatek,mt7621-wdt" },
	{}
};

U_BOOT_DRIVER(mt762x_wdt) = {
	.name = "mt762x_wdt",
	.id = UCLASS_WDT,
	.of_match = mt762x_wdt_ids,
	.probe = mt762x_wdt_probe,
	.priv_auto	= sizeof(struct mt762x_wdt),
	.ops = &mt762x_wdt_ops,
};
