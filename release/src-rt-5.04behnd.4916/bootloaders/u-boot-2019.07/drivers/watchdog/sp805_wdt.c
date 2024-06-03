// SPDX-License-Identifier: GPL-2.0+
/*
 * Watchdog driver for SP805 on some Layerscape SoC
 *
 * Copyright 2019 NXP
 */

#include <asm/io.h>
#include <common.h>
#include <dm/device.h>
#include <dm/fdtaddr.h>
#include <dm/read.h>
#include <linux/bitops.h>
#include <watchdog.h>
#include <wdt.h>

#define WDTLOAD			0x000
#define WDTCONTROL		0x008
#define WDTINTCLR		0x00C
#define WDTLOCK			0xC00

#define TIME_OUT_MIN_MSECS	1
#define TIME_OUT_MAX_MSECS	120000
#define SYS_FSL_WDT_CLK_DIV	16
#define INT_ENABLE		BIT(0)
#define RESET_ENABLE		BIT(1)
#define DISABLE			0
#define UNLOCK			0x1ACCE551
#define LOCK			0x00000001
#define INT_MASK		BIT(0)

DECLARE_GLOBAL_DATA_PTR;

struct sp805_wdt_priv {
	void __iomem *reg;
};

static int sp805_wdt_reset(struct udevice *dev)
{
	struct sp805_wdt_priv *priv = dev_get_priv(dev);

	writel(UNLOCK, priv->reg + WDTLOCK);
	writel(INT_MASK, priv->reg + WDTINTCLR);
	writel(LOCK, priv->reg + WDTLOCK);
	readl(priv->reg + WDTLOCK);

	return 0;
}

static int sp805_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	u32 load_value;
	u32 load_time;
	struct sp805_wdt_priv *priv = dev_get_priv(dev);

	load_time = (u32)timeout;
	if (timeout < TIME_OUT_MIN_MSECS)
		load_time = TIME_OUT_MIN_MSECS;
	else if (timeout > TIME_OUT_MAX_MSECS)
		load_time = TIME_OUT_MAX_MSECS;
	/* sp805 runs counter with given value twice, so when the max timeout is
	 * set 120s, the gd->bus_clk is less than 1145MHz, the load_value will
	 * not overflow.
	 */
	load_value = (gd->bus_clk) /
		(2 * 1000 * SYS_FSL_WDT_CLK_DIV) * load_time;

	writel(UNLOCK, priv->reg + WDTLOCK);
	writel(load_value, priv->reg + WDTLOAD);
	writel(INT_MASK, priv->reg + WDTINTCLR);
	writel(INT_ENABLE | RESET_ENABLE, priv->reg + WDTCONTROL);
	writel(LOCK, priv->reg + WDTLOCK);
	readl(priv->reg + WDTLOCK);

	return 0;
}

static int sp805_wdt_stop(struct udevice *dev)
{
	struct sp805_wdt_priv *priv = dev_get_priv(dev);

	writel(UNLOCK, priv->reg + WDTLOCK);
	writel(DISABLE, priv->reg + WDTCONTROL);
	writel(LOCK, priv->reg + WDTLOCK);
	readl(priv->reg + WDTLOCK);

	return 0;
}

static int sp805_wdt_probe(struct udevice *dev)
{
	debug("%s: Probing wdt%u\n", __func__, dev->seq);

	return 0;
}

static int sp805_wdt_ofdata_to_platdata(struct udevice *dev)
{
	struct sp805_wdt_priv *priv = dev_get_priv(dev);

	priv->reg = (void __iomem *)dev_read_addr(dev);
	if (IS_ERR(priv->reg))
		return PTR_ERR(priv->reg);

	return 0;
}

static const struct wdt_ops sp805_wdt_ops = {
	.start = sp805_wdt_start,
	.reset = sp805_wdt_reset,
	.stop = sp805_wdt_stop,
};

static const struct udevice_id sp805_wdt_ids[] = {
	{ .compatible = "arm,sp805-wdt" },
	{}
};

U_BOOT_DRIVER(sp805_wdt) = {
	.name = "sp805_wdt",
	.id = UCLASS_WDT,
	.of_match = sp805_wdt_ids,
	.probe = sp805_wdt_probe,
	.priv_auto_alloc_size = sizeof(struct sp805_wdt_priv),
	.ofdata_to_platdata = sp805_wdt_ofdata_to_platdata,
	.ops = &sp805_wdt_ops,
};
