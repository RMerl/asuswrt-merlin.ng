// SPDX-License-Identifier: GPL-2.0+
/*
 * [origin: Linux kernel drivers/watchdog/at91sam9_wdt.c]
 *
 * Watchdog driver for AT91SAM9x processors.
 *
 * Copyright (C) 2008 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 * Copyright (C) 2008 Renaud CERRATO r.cerrato@til-technologies.fr
 */

/*
 * The Watchdog Timer Mode Register can be only written to once. If the
 * timeout need to be set from U-Boot, be sure that the bootstrap doesn't
 * write to this register. Inform Linux to it too
 */

#include <asm/io.h>
#include <asm/arch/at91_wdt.h>
#include <common.h>
#include <div64.h>
#include <dm.h>
#include <errno.h>
#include <wdt.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * AT91SAM9 watchdog runs a 12bit counter @ 256Hz,
 * use this to convert a watchdog
 * value from seconds.
 */
#define WDT_SEC2TICKS(s)	(((s) << 8) - 1)

/*
 * Set the watchdog time interval in 1/256Hz (write-once)
 * Counter is 12 bit.
 */
static int at91_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct at91_wdt_priv *priv = dev_get_priv(dev);
	u64 timeout;
	u32 ticks;

	/* Calculate timeout in seconds and the resulting ticks */
	timeout = timeout_ms;
	do_div(timeout, 1000);
	timeout = min_t(u64, timeout, WDT_MAX_TIMEOUT);
	ticks = WDT_SEC2TICKS(timeout);

	/* Check if disabled */
	if (readl(priv->regs + AT91_WDT_MR) & AT91_WDT_MR_WDDIS) {
		printf("sorry, watchdog is disabled\n");
		return -1;
	}

	/*
	 * All counting occurs at SLOW_CLOCK / 128 = 256 Hz
	 *
	 * Since WDV is a 12-bit counter, the maximum period is
	 * 4096 / 256 = 16 seconds.
	 */
	priv->regval = AT91_WDT_MR_WDRSTEN	/* causes watchdog reset */
		| AT91_WDT_MR_WDDBGHLT		/* disabled in debug mode */
		| AT91_WDT_MR_WDD(0xfff)	/* restart at any time */
		| AT91_WDT_MR_WDV(ticks);	/* timer value */
	writel(priv->regval, priv->regs + AT91_WDT_MR);

	return 0;
}

static int at91_wdt_stop(struct udevice *dev)
{
	struct at91_wdt_priv *priv = dev_get_priv(dev);

	/* Disable Watchdog Timer */
	priv->regval |= AT91_WDT_MR_WDDIS;
	writel(priv->regval, priv->regs + AT91_WDT_MR);

	return 0;
}

static int at91_wdt_reset(struct udevice *dev)
{
	struct at91_wdt_priv *priv = dev_get_priv(dev);

	writel(AT91_WDT_CR_WDRSTT | AT91_WDT_CR_KEY, priv->regs + AT91_WDT_CR);

	return 0;
}

static const struct wdt_ops at91_wdt_ops = {
	.start = at91_wdt_start,
	.stop = at91_wdt_stop,
	.reset = at91_wdt_reset,
};

static const struct udevice_id at91_wdt_ids[] = {
	{ .compatible = "atmel,at91sam9260-wdt" },
	{}
};

static int at91_wdt_probe(struct udevice *dev)
{
	struct at91_wdt_priv *priv = dev_get_priv(dev);

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	debug("%s: Probing wdt%u\n", __func__, dev->seq);

	return 0;
}

U_BOOT_DRIVER(at91_wdt) = {
	.name = "at91_wdt",
	.id = UCLASS_WDT,
	.of_match = at91_wdt_ids,
	.priv_auto_alloc_size = sizeof(struct at91_wdt_priv),
	.ops = &at91_wdt_ops,
	.probe = at91_wdt_probe,
};
