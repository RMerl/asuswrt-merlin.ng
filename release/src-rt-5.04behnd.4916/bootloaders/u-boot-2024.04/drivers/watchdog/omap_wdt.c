// SPDX-License-Identifier: GPL-2.0
/*
 * omap_wdt.c
 *
 * (C) Copyright 2013
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 *
 * Watchdog driver for the TI OMAP 16xx & 24xx/34xx 32KHz (non-secure) watchdog
 *
 * commit 2d991a164a61858012651e13c59521975504e260
 * Author: Bill Pemberton <wfp5p@virginia.edu>
 * Date:   Mon Nov 19 13:21:41 2012 -0500
 *
 * watchdog: remove use of __devinit
 *
 * CONFIG_HOTPLUG is going away as an option so __devinit is no longer
 * needed.
 *
 * Author: MontaVista Software, Inc.
 *	 <gdavis@mvista.com> or <source@mvista.com>
 *
 * History:
 *
 * 20030527: George G. Davis <gdavis@mvista.com>
 *	Initially based on linux-2.4.19-rmk7-pxa1/drivers/char/sa1100_wdt.c
 *	(c) Copyright 2000 Oleg Drokin <green@crimea.edu>
 *	Based on SoftDog driver by Alan Cox <alan@lxorguk.ukuu.org.uk>
 *
 * Copyright (c) 2004 Texas Instruments.
 *	1. Modified to support OMAP1610 32-KHz watchdog timer
 *	2. Ported to 2.6 kernel
 *
 * Copyright (c) 2005 David Brownell
 *	Use the driver model and standard identifiers; handle bigger timeouts.
 */

#include <common.h>
#include <log.h>
#include <watchdog.h>
#include <asm/ti-common/omap_wdt.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/arch/cpu.h>
#include <wdt.h>
#include <dm.h>
#include <errno.h>

/* Hardware timeout in seconds */
#define WDT_HW_TIMEOUT 60

#if !CONFIG_IS_ENABLED(WDT)
static unsigned int wdt_trgr_pattern = 0x1234;

void hw_watchdog_reset(void)
{
	struct wd_timer *wdt = (struct wd_timer *)WDT_BASE;

	/*
	 * Somebody just triggered watchdog reset and write to WTGR register
	 * is in progress. It is resetting right now, no need to trigger it
	 * again
	 */
	if ((readl(&wdt->wdtwwps)) & WDT_WWPS_PEND_WTGR)
		return;

	wdt_trgr_pattern = ~wdt_trgr_pattern;
	writel(wdt_trgr_pattern, &wdt->wdtwtgr);

	/*
	 * Don't wait for posted write to complete, i.e. don't check
	 * WDT_WWPS_PEND_WTGR bit in WWPS register. There is no writes to
	 * WTGR register outside of this func, and if entering it
	 * we see WDT_WWPS_PEND_WTGR bit set, it means watchdog reset
	 * was just triggered. This prevents us from wasting time in busy
	 * polling of WDT_WWPS_PEND_WTGR bit.
	 */
}

static int omap_wdt_set_timeout(unsigned int timeout)
{
	struct wd_timer *wdt = (struct wd_timer *)WDT_BASE;
	u32 pre_margin = GET_WLDR_VAL(timeout);

	/* just count up at 32 KHz */
	while (readl(&wdt->wdtwwps) & WDT_WWPS_PEND_WLDR)
		;

	writel(pre_margin, &wdt->wdtwldr);
	while (readl(&wdt->wdtwwps) & WDT_WWPS_PEND_WLDR)
		;

	return 0;
}

void hw_watchdog_disable(void)
{
	struct wd_timer *wdt = (struct wd_timer *)WDT_BASE;

	/*
	 * Disable watchdog
	 */
	writel(0xAAAA, &wdt->wdtwspr);
	while (readl(&wdt->wdtwwps) != 0x0)
		;
	writel(0x5555, &wdt->wdtwspr);
	while (readl(&wdt->wdtwwps) != 0x0)
		;
}

void hw_watchdog_init(void)
{
	struct wd_timer *wdt = (struct wd_timer *)WDT_BASE;

	/*
	 * Make sure the watchdog is disabled. This is unfortunately required
	 * because writing to various registers with the watchdog running has no
	 * effect.
	 */
	hw_watchdog_disable();

	/* initialize prescaler */
	while (readl(&wdt->wdtwwps) & WDT_WWPS_PEND_WCLR)
		;

	writel(WDT_WCLR_PRE | (PTV << WDT_WCLR_PTV_OFF), &wdt->wdtwclr);
	while (readl(&wdt->wdtwwps) & WDT_WWPS_PEND_WCLR)
		;

	omap_wdt_set_timeout(WDT_HW_TIMEOUT);

	/* Sequence to enable the watchdog */
	writel(0xBBBB, &wdt->wdtwspr);
	while ((readl(&wdt->wdtwwps)) & WDT_WWPS_PEND_WSPR)
		;

	writel(0x4444, &wdt->wdtwspr);
	while ((readl(&wdt->wdtwwps)) & WDT_WWPS_PEND_WSPR)
		;
}

void watchdog_reset(void)
{
	hw_watchdog_reset();
}

#else

static int omap3_wdt_reset(struct udevice *dev)
{
	struct omap3_wdt_priv *priv = dev_get_priv(dev);

	/*
	 * Somebody just triggered watchdog reset and write to WTGR register
	 * is in progress. It is resetting right now, no need to trigger it
	 * again
	 */
	if ((readl(&priv->regs->wdtwwps)) & WDT_WWPS_PEND_WTGR)
		return 0;

	priv->wdt_trgr_pattern = ~(priv->wdt_trgr_pattern);
	writel(priv->wdt_trgr_pattern, &priv->regs->wdtwtgr);
	/*
	 * Don't wait for posted write to complete, i.e. don't check
	 * WDT_WWPS_PEND_WTGR bit in WWPS register. There is no writes to
	 * WTGR register outside of this func, and if entering it
	 * we see WDT_WWPS_PEND_WTGR bit set, it means watchdog reset
	 * was just triggered. This prevents us from wasting time in busy
	 * polling of WDT_WWPS_PEND_WTGR bit.
	 */
	return 0;
}

static int omap3_wdt_stop(struct udevice *dev)
{
	struct omap3_wdt_priv *priv = dev_get_priv(dev);

	/* disable watchdog */
	writel(0xAAAA, &priv->regs->wdtwspr);
	while (readl(&priv->regs->wdtwwps) != 0x0)
		;
	writel(0x5555, &priv->regs->wdtwspr);
	while (readl(&priv->regs->wdtwwps) != 0x0)
		;
	return 0;
}

static int omap3_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct omap3_wdt_priv *priv = dev_get_priv(dev);
	u32 pre_margin = GET_WLDR_VAL(timeout_ms / 1000);
	/*
	 * Make sure the watchdog is disabled. This is unfortunately required
	 * because writing to various registers with the watchdog running has
	 * no effect.
	 */
	omap3_wdt_stop(dev);

	/* initialize prescaler */
	while (readl(&priv->regs->wdtwwps) & WDT_WWPS_PEND_WCLR)
		;

	writel(WDT_WCLR_PRE | (PTV << WDT_WCLR_PTV_OFF), &priv->regs->wdtwclr);
	while (readl(&priv->regs->wdtwwps) & WDT_WWPS_PEND_WCLR)
		;
	/* just count up at 32 KHz */
	while (readl(&priv->regs->wdtwwps) & WDT_WWPS_PEND_WLDR)
		;

	writel(pre_margin, &priv->regs->wdtwldr);
	while (readl(&priv->regs->wdtwwps) & WDT_WWPS_PEND_WLDR)
		;
	/* Sequence to enable the watchdog */
	writel(0xBBBB, &priv->regs->wdtwspr);
	while ((readl(&priv->regs->wdtwwps)) & WDT_WWPS_PEND_WSPR)
		;

	writel(0x4444, &priv->regs->wdtwspr);
	while ((readl(&priv->regs->wdtwwps)) & WDT_WWPS_PEND_WSPR)
		;

	/* Trigger the watchdog to actually reload the counter. */
	while ((readl(&priv->regs->wdtwwps)) & WDT_WWPS_PEND_WTGR)
		;

	priv->wdt_trgr_pattern = ~(priv->wdt_trgr_pattern);
	writel(priv->wdt_trgr_pattern, &priv->regs->wdtwtgr);

	while ((readl(&priv->regs->wdtwwps)) & WDT_WWPS_PEND_WTGR)
		;

	return 0;
}

static int omap3_wdt_probe(struct udevice *dev)
{
	struct omap3_wdt_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	priv->wdt_trgr_pattern = 0x1234;
	debug("%s: Probing wdt%u\n", __func__, dev_seq(dev));
	return 0;
}

static const struct wdt_ops omap3_wdt_ops = {
	.start = omap3_wdt_start,
	.stop = omap3_wdt_stop,
	.reset = omap3_wdt_reset,
};

static const struct udevice_id omap3_wdt_ids[] = {
	{ .compatible = "ti,omap3-wdt" },
	{ }
};

U_BOOT_DRIVER(omap3_wdt) = {
	.name = "omap3_wdt",
	.id = UCLASS_WDT,
	.of_match = omap3_wdt_ids,
	.ops = &omap3_wdt_ops,
	.probe = omap3_wdt_probe,
	.priv_auto	= sizeof(struct omap3_wdt_priv),
};
#endif /* !CONFIG_IS_ENABLED(WDT) */
