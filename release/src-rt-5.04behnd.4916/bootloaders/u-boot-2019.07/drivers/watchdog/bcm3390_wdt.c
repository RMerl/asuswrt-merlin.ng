/*
 * Copyright (C) 2022 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <clk.h>
#include <asm/io.h>

#define WDT_START_1		0xff00
#define WDT_START_2		0x00ff
#define WDT_STOP_1		0xee00
#define WDT_STOP_2		0x00ee

#define WDT_TIMEOUT_REG		0x28
#define WDT_CMD_REG		0x2c
#define WDT_CTRL_REG		0x3c
#define WDT_CTRL_MODE_RST	0x0
#define WDT_CTRL_MODE_NMI	0x1
#define WDT_CTRL_MODE_NMI_RST	0x2
#define WDT_INT_STATUS_REG	0x0
#define WDT_INT_REG		0x4
#define WDT_PRETIMEOUT_CTL_REG	0x8
#define WDT_PRETIMEOUT_STS_REG	0x18
#define WDT_HTIMEOUT_INT_MASK	0x10
#define WDT_PRETIMEOUT_INT_MASK	0x01
#define WDT_PRETIMEOUT_ENABLE	0xC0000000
#define WDT_PRETIMEOUT_VAL_MASK	0x3fffffff
#define WDT_CTRL_MODE_VAL_MASK	0x3

#define WDT_MIN_TIMEOUT		1 /* seconds */
#define WDT_DEFAULT_TIMEOUT	30 /* seconds */
#define WDT_DEFAULT_RATE	27000000
#define WDT_DEFAULT_PRETIMEOUT	(WDT_DEFAULT_TIMEOUT/2)
#define WDT_MAX_TIMEOUT		159 /* Maximux Timeout value in seconds */

struct bcm3390_watchdog {
	void __iomem *base;
	unsigned long clk_rate;
};

static int bcm3390_wdt_reset(struct udevice *dev)
{
	struct bcm3390_watchdog *wdt = dev_get_priv(dev);

	writel(WDT_START_1, wdt->base + WDT_CMD_REG);
	writel(WDT_START_2, wdt->base + WDT_CMD_REG);

	return 0;
}

static int bcm3390_wdt_stop(struct udevice *dev)
{
	struct bcm3390_watchdog *wdt = dev_get_priv(dev);

	writel(WDT_STOP_1, wdt->base + WDT_CMD_REG);
	writel(WDT_STOP_2, wdt->base + WDT_CMD_REG);

	return 0;
}

static int bcm3390_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct bcm3390_watchdog *wdt = dev_get_priv(dev);
	u32 val = wdt->clk_rate / 1000 * timeout;

    bcm3390_wdt_stop(dev);

    writel(val, wdt->base + WDT_TIMEOUT_REG);

	return bcm3390_wdt_reset(dev);
}

static int bcm3390_wdt_expire_now(struct udevice *dev, ulong flags)
{
   return bcm3390_wdt_start(dev, 1, flags);
}

static const struct wdt_ops bcm3390_wdt_ops = {
	.expire_now = bcm3390_wdt_expire_now,
	.reset = bcm3390_wdt_reset,
	.start = bcm3390_wdt_start,
	.stop = bcm3390_wdt_stop,
};

static const struct udevice_id bcm3390_wdt_ids[] = {
	{ .compatible = "brcm,bcm3390-wdt" },
	{ }
};

static int bcm3390_wdt_probe(struct udevice *dev)
{
	struct bcm3390_watchdog *wdt = dev_get_priv(dev);
	struct clk clk;
	int ret;

	wdt->base = dev_remap_addr(dev);
	if (!wdt->base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (!ret)
		wdt->clk_rate = clk_get_rate(&clk);
	else
		wdt->clk_rate = WDT_DEFAULT_RATE;

	bcm3390_wdt_stop(dev);

	return 0;
}

U_BOOT_DRIVER(wdt_bcm3390) = {
	.name = "wdt_bcm3390",
	.id = UCLASS_WDT,
	.of_match = bcm3390_wdt_ids,
	.ops = &bcm3390_wdt_ops,
	.priv_auto_alloc_size = sizeof(struct bcm3390_watchdog),
	.probe = bcm3390_wdt_probe,
};

MODULE_LICENSE("GPL v2");
