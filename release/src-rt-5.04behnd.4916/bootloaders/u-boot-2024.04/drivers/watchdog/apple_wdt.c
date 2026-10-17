// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <clk.h>
#include <dm.h>
#include <wdt.h>
#include <asm/io.h>
#include <linux/delay.h>

#define APPLE_WDT_CUR_TIME		0x10
#define APPLE_WDT_BARK_TIME		0x14
#define APPLE_WDT_CTRL			0x1c
#define  APPLE_WDT_CTRL_RESET_EN	BIT(2)

struct apple_wdt_priv {
	void *base;
	ulong clk_rate;
};

static int apple_wdt_reset(struct udevice *dev)
{
	struct apple_wdt_priv *priv = dev_get_priv(dev);

	writel(0, priv->base + APPLE_WDT_CUR_TIME);

	return 0;
}

static int apple_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct apple_wdt_priv *priv = dev_get_priv(dev);
	u64 timeout;

	timeout = (timeout_ms * priv->clk_rate) / 1000;
	if (timeout > U32_MAX)
		return -EINVAL;

	writel(0, priv->base + APPLE_WDT_CUR_TIME);
	writel(timeout, priv->base + APPLE_WDT_BARK_TIME);
	writel(APPLE_WDT_CTRL_RESET_EN, priv->base + APPLE_WDT_CTRL);

	return 0;
}

static int apple_wdt_stop(struct udevice *dev)
{
	struct apple_wdt_priv *priv = dev_get_priv(dev);

	writel(0, priv->base + APPLE_WDT_CTRL);

	return 0;
}

static int apple_wdt_expire_now(struct udevice *dev, ulong flags)
{
	int ret;

	ret = apple_wdt_start(dev, 0, flags);
	if (ret)
		return ret;

	/*
	 * It can take up to 25ms until the SoC actually resets, so
	 * wait 50ms just to be sure.
	 */
	mdelay(50);

	return 0;
}

static const struct wdt_ops apple_wdt_ops = {
	.reset = apple_wdt_reset,
	.start = apple_wdt_start,
	.stop = apple_wdt_stop,
	.expire_now = apple_wdt_expire_now,
};

static const struct udevice_id apple_wdt_ids[] = {
	{ .compatible = "apple,wdt" },
	{ /* sentinel */ }
};

static int apple_wdt_probe(struct udevice *dev)
{
	struct apple_wdt_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	priv->clk_rate = clk_get_rate(&clk);

	return 0;
}

U_BOOT_DRIVER(apple_wdt) = {
	.name = "apple_wdt",
	.id = UCLASS_WDT,
	.of_match = apple_wdt_ids,
	.priv_auto = sizeof(struct apple_wdt_priv),
	.probe = apple_wdt_probe,
	.ops = &apple_wdt_ops,
};
