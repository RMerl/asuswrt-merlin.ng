// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/drivers/watchdog/bcm63xx_wdt.c:
 *	Copyright (C) 2007 Miguel Gaio <miguel.gaio@efixo.com>
 *	Copyright (C) 2008 Florian Fainelli <florian@openwrt.org>
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <clk.h>
#include <asm/io.h>

/* WDT Value register */
#define WDT_VAL_REG		0x0
#define WDT_VAL_MIN		0x00000002
#define WDT_VAL_MAX		0xfffffffe

/* WDT Control register */
#define WDT_CTL_REG		0x4
#define WDT_CTL_START1_MASK	0x0000ff00
#define WDT_CTL_START2_MASK	0x000000ff
#define WDT_CTL_STOP1_MASK	0x0000ee00
#define WDT_CTL_STOP2_MASK	0x000000ee

struct bcm6345_wdt_priv {
	void __iomem *regs;
	unsigned long clk_rate;
};

static int bcm6345_wdt_reset(struct udevice *dev)
{
	struct bcm6345_wdt_priv *priv = dev_get_priv(dev);

	writel(WDT_CTL_START1_MASK, priv->regs + WDT_CTL_REG);
	writel(WDT_CTL_START2_MASK, priv->regs + WDT_CTL_REG);

	return 0;
}

static int bcm6345_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct bcm6345_wdt_priv *priv = dev_get_priv(dev);
	u32 val = priv->clk_rate / 1000 * timeout;

	if (val < WDT_VAL_MIN) {
		debug("watchdog won't fire with less than 2 ticks\n");
		val = WDT_VAL_MIN;
	} else if (val > WDT_VAL_MAX) {
		debug("maximum watchdog timeout exceeded\n");
		val = WDT_VAL_MAX;
	}

	writel(val, priv->regs + WDT_VAL_REG);

	return bcm6345_wdt_reset(dev);
}

static int bcm6345_wdt_expire_now(struct udevice *dev, ulong flags)
{
	return bcm6345_wdt_start(dev, WDT_VAL_MIN, flags);
}

static int bcm6345_wdt_stop(struct udevice *dev)
{
	struct bcm6345_wdt_priv *priv = dev_get_priv(dev);

	writel(WDT_CTL_STOP1_MASK, priv->regs + WDT_CTL_REG);
	writel(WDT_CTL_STOP2_MASK, priv->regs + WDT_CTL_REG);

	return 0;
}

static const struct wdt_ops bcm6345_wdt_ops = {
	.expire_now = bcm6345_wdt_expire_now,
	.reset = bcm6345_wdt_reset,
	.start = bcm6345_wdt_start,
	.stop = bcm6345_wdt_stop,
};

static const struct udevice_id bcm6345_wdt_ids[] = {
	{ .compatible = "brcm,bcm6345-wdt" },
	{ /* sentinel */ }
};

static int bcm6345_wdt_probe(struct udevice *dev)
{
	struct bcm6345_wdt_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (!ret)
		priv->clk_rate = clk_get_rate(&clk);
	else
		return -EINVAL;

	bcm6345_wdt_stop(dev);

	return 0;
}

U_BOOT_DRIVER(wdt_bcm6345) = {
	.name = "wdt_bcm6345",
	.id = UCLASS_WDT,
	.of_match = bcm6345_wdt_ids,
	.ops = &bcm6345_wdt_ops,
	.priv_auto_alloc_size = sizeof(struct bcm6345_wdt_priv),
	.probe = bcm6345_wdt_probe,
};
