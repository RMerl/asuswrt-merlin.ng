// SPDX-License-Identifier: GPL-2.0+
/*
 * Watchdog driver for the FTWDT010 Watch Dog Driver
 *
 * (c) Copyright 2004 Faraday Technology Corp. (www.faraday-tech.com)
 * Based on sa1100_wdt.c by Oleg Drokin <green@crimea.edu>
 * Based on SoftDog driver by Alan Cox <alan@redhat.com>
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 *
 * 27/11/2004 Initial release, Faraday.
 * 12/01/2011 Port to u-boot, Macpaul Lin.
 * 22/08/2022 Port to DM
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <log.h>
#include <asm/io.h>
#include <faraday/ftwdt010_wdt.h>

struct ftwdt010_wdt_priv {
	struct ftwdt010_wdt __iomem *regs;
};

static int ftwdt010_wdt_reset(struct udevice *dev)
{
	struct ftwdt010_wdt_priv *priv = dev_get_priv(dev);
	struct ftwdt010_wdt *wd = priv->regs;

	debug("Reset WDT..\n");

	/* clear control register */
	writel(0, &wd->wdcr);

	/* Write Magic number */
	writel(FTWDT010_WDRESTART_MAGIC, &wd->wdrestart);

	/* Enable WDT */
	writel(FTWDT010_WDCR_RST | FTWDT010_WDCR_ENABLE, &wd->wdcr);

	return 0;
}

/*
 * Set the watchdog time interval and start the timer.
 * Counter is 32 bit.
 */
static int ftwdt010_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct ftwdt010_wdt_priv *priv = dev_get_priv(dev);
	struct ftwdt010_wdt *wd = priv->regs;
	unsigned int reg;

	debug("Activating WDT %llu ms\n", timeout_ms);

	/* Check if disabled */
	if (readl(&wd->wdcr) & ~FTWDT010_WDCR_ENABLE) {
		printf("sorry, watchdog is disabled\n");
		return -1;
	}

	/*
	 * In a 66MHz system,
	 * if you set WDLOAD as 0x03EF1480 (66000000)
	 * the reset timer is 1 second.
	 */
	reg = FTWDT010_WDLOAD(timeout_ms * FTWDT010_TIMEOUT_FACTOR);

	writel(reg, &wd->wdload);

	return ftwdt010_wdt_reset(dev);
}

static int ftwdt010_wdt_stop(struct udevice *dev)
{
	struct ftwdt010_wdt_priv *priv = dev_get_priv(dev);
	struct ftwdt010_wdt *wd = priv->regs;

	debug("Deactivating WDT..\n");

	/*
	 * It was defined with CONFIG_WATCHDOG_NOWAYOUT in Linux
	 *
	 * Shut off the timer.
	 * Lock it in if it's a module and we defined ...NOWAYOUT
	 */
	writel(0, &wd->wdcr);
	return 0;
}

static int ftwdt010_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct ftwdt010_wdt_priv *priv = dev_get_priv(dev);
	struct ftwdt010_wdt *wd = priv->regs;

	debug("Expiring WDT..\n");
	writel(FTWDT010_WDLOAD(0), &wd->wdload);
	return ftwdt010_wdt_reset(dev);
}

static int ftwdt010_wdt_probe(struct udevice *dev)
{
	struct ftwdt010_wdt_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	return 0;
}

static const struct wdt_ops ftwdt010_wdt_ops = {
	.start = ftwdt010_wdt_start,
	.reset = ftwdt010_wdt_reset,
	.stop = ftwdt010_wdt_stop,
	.expire_now = ftwdt010_wdt_expire_now,
};

static const struct udevice_id ftwdt010_wdt_ids[] = {
	{ .compatible = "faraday,ftwdt010" },
	{}
};

U_BOOT_DRIVER(ftwdt010_wdt) = {
	.name = "ftwdt010_wdt",
	.id = UCLASS_WDT,
	.of_match = ftwdt010_wdt_ids,
	.ops = &ftwdt010_wdt_ops,
	.probe = ftwdt010_wdt_probe,
	.priv_auto = sizeof(struct ftwdt010_wdt_priv),
};
