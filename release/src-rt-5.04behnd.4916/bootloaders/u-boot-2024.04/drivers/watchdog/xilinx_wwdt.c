// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx window watchdog timer driver.
 *
 * Author(s):	Michal Simek <michal.simek@amd.com>
 *		Ashok Reddy Soma <ashok.reddy.soma@xilinx.com>
 *
 * Copyright (c) 2020, Xilinx Inc.
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <wdt.h>
#include <linux/compat.h>
#include <dm/device_compat.h>
#include <linux/io.h>

/* Refresh Register Masks */
#define XWT_WWREF_GWRR_MASK	BIT(0) /* Refresh and start new period */

/* Generic Control/Status Register Masks */
#define XWT_WWCSR_GWEN_MASK	BIT(0) /* Enable Bit */

/* Register offsets for the WWDT device */
#define XWT_WWDT_MWR_OFFSET	0x00
#define XWT_WWDT_ESR_OFFSET	0x04
#define XWT_WWDT_FCR_OFFSET	0x08
#define XWT_WWDT_FWR_OFFSET	0x0c
#define XWT_WWDT_SWR_OFFSET	0x10
#define XWT_WWDT_CNT_MIN	1
#define XWT_WWDT_CNT_MAX	0xffffffff

/* Master Write Control Register Masks */
#define XWT_WWDT_MWR_MASK	BIT(0)

/* Enable and Status Register Masks */
#define XWT_WWDT_ESR_WINT_MASK	BIT(16)
#define XWT_WWDT_ESR_WSW_MASK	BIT(8)
#define XWT_WWDT_ESR_WEN_MASK	BIT(0)

struct xlnx_wwdt_priv {
	bool enable_once;
	struct regmap *regs;
	struct clk clk;
};

struct xlnx_wwdt_plat {
	bool enable_once;
};

static int xlnx_wwdt_reset(struct udevice *dev)
{
	u32 esr;
	struct xlnx_wwdt_priv *wdt = dev_get_priv(dev);

	regmap_write(wdt->regs, XWT_WWDT_MWR_OFFSET, XWT_WWDT_MWR_MASK);
	regmap_read(wdt->regs, XWT_WWDT_ESR_OFFSET, &esr);
	esr |= XWT_WWDT_ESR_WINT_MASK;
	esr &= ~XWT_WWDT_ESR_WSW_MASK;
	regmap_write(wdt->regs, XWT_WWDT_ESR_OFFSET, esr);
	regmap_read(wdt->regs, XWT_WWDT_ESR_OFFSET, &esr);
	esr |= XWT_WWDT_ESR_WSW_MASK;
	regmap_write(wdt->regs, XWT_WWDT_ESR_OFFSET, esr);

	return 0;
}

static int xlnx_wwdt_stop(struct udevice *dev)
{
	struct xlnx_wwdt_priv *wdt = dev_get_priv(dev);

	if (wdt->enable_once) {
		dev_warn(dev, "Can't stop Xilinx watchdog.\n");
		return -EBUSY;
	}

	/* Disable the  window watchdog timer */
	regmap_write(wdt->regs, XWT_WWDT_MWR_OFFSET, XWT_WWDT_MWR_MASK);
	regmap_write(wdt->regs, XWT_WWDT_ESR_OFFSET, ~(u32)XWT_WWDT_ESR_WEN_MASK);

	clk_disable(&wdt->clk);

	dev_dbg(dev, "Watchdog disabled!\n");

	return 0;
}

static int xlnx_wwdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	int ret;
	u32 esr;
	u64 count, timeout;
	unsigned long clock_f;
	struct xlnx_wwdt_priv *wdt = dev_get_priv(dev);

	clock_f = clk_get_rate(&wdt->clk);
	if (IS_ERR_VALUE(clock_f)) {
		dev_err(dev, "failed to get rate\n");
		return clock_f;
	}

	dev_dbg(dev, "%s: CLK %ld\n", __func__, clock_f);

	/* Convert timeout from msec to sec */
	timeout = timeout_ms / 1000;

	/* Calculate timeout count */
	count = timeout * clock_f;

	/* Count should be at least 1 */
	if (count < XWT_WWDT_CNT_MIN) {
		debug("%s: watchdog won't fire with 0 ticks\n", __func__);
		count = XWT_WWDT_CNT_MIN;
	}

	/* Limit the count to maximum possible value */
	if (count > XWT_WWDT_CNT_MAX) {
		debug("%s: maximum watchdog timeout exceeded\n", __func__);
		count = XWT_WWDT_CNT_MAX;
	}

	ret = clk_enable(&wdt->clk);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	/* Disable the window watchdog timer */
	regmap_write(wdt->regs, XWT_WWDT_MWR_OFFSET, XWT_WWDT_MWR_MASK);
	regmap_write(wdt->regs, XWT_WWDT_ESR_OFFSET, ~(u32)XWT_WWDT_ESR_WEN_MASK);

	/* Set first window and second window registers with timeout */
	regmap_write(wdt->regs, XWT_WWDT_FWR_OFFSET, 0); /* No pre-timeout */
	regmap_write(wdt->regs, XWT_WWDT_SWR_OFFSET, (u32)count);
	regmap_write(wdt->regs, XWT_WWDT_FCR_OFFSET, 0);

	/* Enable the window watchdog timer */
	regmap_read(wdt->regs, XWT_WWDT_ESR_OFFSET, &esr);
	esr |= XWT_WWDT_ESR_WEN_MASK;
	regmap_write(wdt->regs, XWT_WWDT_ESR_OFFSET, esr);

	return 0;
}

static int xlnx_wwdt_expire_now(struct udevice *dev, ulong flags)
{
	return xlnx_wwdt_start(dev, XWT_WWDT_CNT_MIN, flags);
}

static int xlnx_wwdt_probe(struct udevice *dev)
{
	int ret;
	struct xlnx_wwdt_plat *plat = dev_get_plat(dev);
	struct xlnx_wwdt_priv *wdt = dev_get_priv(dev);

	dev_dbg(dev, "%s: Probing wdt%u\n", __func__, dev_seq(dev));

	ret = regmap_init_mem(dev_ofnode(dev), &wdt->regs);
	if (ret) {
		dev_dbg(dev, "failed to get regbase of wwdt\n");
		return ret;
	}

	wdt->enable_once = plat->enable_once;

	ret = clk_get_by_index(dev, 0, &wdt->clk);
	if (ret < 0)
		dev_err(dev, "failed to get clock\n");

	return ret;
}

static int xlnx_wwdt_of_to_plat(struct udevice *dev)
{
	struct xlnx_wwdt_plat *plat = dev_get_plat(dev);

	plat->enable_once = dev_read_u32_default(dev, "xlnx,wdt-enable-once",
						 0);
	dev_dbg(dev, "wdt-enable-once %d\n", plat->enable_once);

	return 0;
}

static const struct wdt_ops xlnx_wwdt_ops = {
	.start = xlnx_wwdt_start,
	.reset = xlnx_wwdt_reset,
	.stop = xlnx_wwdt_stop,
	.expire_now = xlnx_wwdt_expire_now,
};

static const struct udevice_id xlnx_wwdt_ids[] = {
	{ .compatible = "xlnx,versal-wwdt", },
	{ .compatible = "xlnx,versal-wwdt-1.0", }, /* deprecated */
	{},
};

U_BOOT_DRIVER(xlnx_wwdt) = {
	.name = "xlnx_wwdt",
	.id = UCLASS_WDT,
	.of_match = xlnx_wwdt_ids,
	.probe = xlnx_wwdt_probe,
	.priv_auto	= sizeof(struct xlnx_wwdt_priv),
	.plat_auto	= sizeof(struct xlnx_wwdt_plat),
	.of_to_plat = xlnx_wwdt_of_to_plat,
	.ops = &xlnx_wwdt_ops,
};
