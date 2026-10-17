// SPDX-License-Identifier: GPL-2.0
/*
 * Watchdog driver for MediaTek SoCs
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <hang.h>
#include <wdt.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define MTK_WDT_MODE			0x00
#define MTK_WDT_LENGTH			0x04
#define MTK_WDT_RESTART			0x08
#define MTK_WDT_STATUS			0x0c
#define MTK_WDT_INTERVAL		0x10
#define MTK_WDT_SWRST			0x14
#define MTK_WDT_REQ_MODE		0x30
#define MTK_WDT_DEBUG_CTL		0x40

#define WDT_MODE_KEY			(0x22 << 24)
#define WDT_MODE_EN			BIT(0)
#define WDT_MODE_EXTPOL			BIT(1)
#define WDT_MODE_EXTEN			BIT(2)
#define WDT_MODE_IRQ_EN			BIT(3)
#define WDT_MODE_DUAL_EN		BIT(6)

#define WDT_LENGTH_KEY			0x8
#define WDT_LENGTH_TIMEOUT(n)		((n) << 5)

#define WDT_RESTART_KEY			0x1971
#define WDT_SWRST_KEY			0x1209

struct mtk_wdt_priv {
	void __iomem *base;
};

static int mtk_wdt_reset(struct udevice *dev)
{
	struct mtk_wdt_priv *priv = dev_get_priv(dev);

	/* Reload watchdog duration */
	writel(WDT_RESTART_KEY, priv->base + MTK_WDT_RESTART);

	return 0;
}

static int mtk_wdt_stop(struct udevice *dev)
{
	struct mtk_wdt_priv *priv = dev_get_priv(dev);

	clrsetbits_le32(priv->base + MTK_WDT_MODE, WDT_MODE_EN, WDT_MODE_KEY);

	return 0;
}

static int mtk_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct mtk_wdt_priv *priv = dev_get_priv(dev);

	/* Kick watchdog to prevent counter == 0 */
	writel(WDT_RESTART_KEY, priv->base + MTK_WDT_RESTART);

	/* Reset */
	writel(WDT_SWRST_KEY, priv->base + MTK_WDT_SWRST);
	hang();

	return 0;
}

static void mtk_wdt_set_timeout(struct udevice *dev, u64 timeout_ms)
{
	struct mtk_wdt_priv *priv = dev_get_priv(dev);
	u64 timeout_us;
	u32 timeout_cc;
	u32 length;

	/*
	 * One WDT_LENGTH count is 512 ticks of the wdt clock
	 * Clock runs at 32768 Hz
	 * e.g. 15.625 ms per count (nominal)
	 * We want the ceiling after dividing timeout_ms by 15.625 ms
	 * We add 15624 prior to the divide to implement the ceiling
	 * We prevent over-flow by clamping the timeout_ms value here
	 *  as the maximum WDT_LENGTH counts is 1023 -> 15.984375 sec
	 * We also enforce a minimum of 1 count
	 * Many watchdog peripherals have a self-imposed count of 1
	 *  that is added to the register counts.
	 *  The MediaTek docs lack details to know if this is the case here.
	 *  So we enforce a minimum of 1 to guarantee operation.
	 */
	if (timeout_ms > 15984)
		timeout_ms = 15984;

	timeout_us = timeout_ms * 1000;
	timeout_cc = (15624 + timeout_us) / 15625;
	if (timeout_cc == 0)
		timeout_cc = 1;

	length = WDT_LENGTH_TIMEOUT(timeout_cc) | WDT_LENGTH_KEY;
	writel(length, priv->base + MTK_WDT_LENGTH);
}

static int mtk_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct mtk_wdt_priv *priv = dev_get_priv(dev);

	mtk_wdt_set_timeout(dev, timeout_ms);

	mtk_wdt_reset(dev);

	/* Enable watchdog reset signal */
	setbits_le32(priv->base + MTK_WDT_MODE,
		     WDT_MODE_EN | WDT_MODE_KEY | WDT_MODE_EXTEN);

	return 0;
}

static int mtk_wdt_probe(struct udevice *dev)
{
	struct mtk_wdt_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENOENT;

	/* Clear status */
	clrsetbits_le32(priv->base + MTK_WDT_MODE,
			WDT_MODE_IRQ_EN | WDT_MODE_EXTPOL, WDT_MODE_KEY);

	return mtk_wdt_stop(dev);
}

static const struct wdt_ops mtk_wdt_ops = {
	.start = mtk_wdt_start,
	.reset = mtk_wdt_reset,
	.stop = mtk_wdt_stop,
	.expire_now = mtk_wdt_expire_now,
};

static const struct udevice_id mtk_wdt_ids[] = {
	{ .compatible = "mediatek,wdt"},
	{ .compatible = "mediatek,mt6589-wdt"},
	{ .compatible = "mediatek,mt7986-wdt" },
	{}
};

U_BOOT_DRIVER(mtk_wdt) = {
	.name = "mtk_wdt",
	.id = UCLASS_WDT,
	.of_match = mtk_wdt_ids,
	.priv_auto	= sizeof(struct mtk_wdt_priv),
	.probe = mtk_wdt_probe,
	.ops = &mtk_wdt_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
