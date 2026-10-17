// SPDX-License-Identifier: GPL-2.0
/*
 * mcf_wdt.c - driver for ColdFire on-chip watchdog
 *
 * Author: Angelo Dureghello <angelo@kernel-space.org>
 *
 */

#include <common.h>
#include <dm.h>
#include <hang.h>
#include <asm/io.h>
#include <wdt.h>
#include <linux/bitops.h>

#define DIVIDER_5XXX	4096
#define DIVIDER_5282	8192

#define WCR_EN          BIT(0)
#define WCR_HALTED      BIT(1)
#define WCR_DOZE        BIT(2)
#define WCR_WAIT        BIT(3)

struct watchdog_regs {
	u16 wcr;        /* Control */
	u16 wmr;        /* Service */
	u16 wcntr;      /* Counter */
	u16 wsr;        /* Reset Status */
};

static void mcf_watchdog_reset(struct watchdog_regs *wdog)
{
	if (!IS_ENABLED(CONFIG_WATCHDOG_RESET_DISABLE)) {
		writew(0x5555, &wdog->wsr);
		writew(0xaaaa, &wdog->wsr);
	}
}

static void mcf_watchdog_init(struct watchdog_regs *wdog, u32 fixed_divider,
			      u64 timeout_msecs)
{
	u32 wdog_module, cycles_per_sec;

	cycles_per_sec = CFG_SYS_CLK / fixed_divider;

	wdog_module = cycles_per_sec * ((u32)timeout_msecs / 1000);
	wdog_module += (cycles_per_sec / 1000) * ((u32)timeout_msecs % 1000);

	/* Limit check, max 16 bits */
	if (wdog_module > 0xffff)
		wdog_module = 0xffff;

	/* Set timeout and enable watchdog */
	writew((u16)wdog_module, &wdog->wmr);
	writew(WCR_EN, &wdog->wcr);

	mcf_watchdog_reset(wdog);
}

struct mcf_wdt_priv {
	void __iomem *base;
	u32 fixed_divider;
};

static int mcf_wdt_expire_now(struct udevice *dev, ulong flags)
{
	hang();

	return 0;
}

static int mcf_wdt_reset(struct udevice *dev)
{
	struct mcf_wdt_priv *priv = dev_get_priv(dev);

	mcf_watchdog_reset(priv->base);

	return 0;
}

static int mcf_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct mcf_wdt_priv *priv = dev_get_priv(dev);

	/* Timeout from fdt (timeout) comes in milliseconds */
	mcf_watchdog_init(priv->base, priv->fixed_divider, timeout);

	return 0;
}

static int mcf_wdt_stop(struct udevice *dev)
{
	struct mcf_wdt_priv *priv = dev_get_priv(dev);
	struct watchdog_regs *wdog = (struct watchdog_regs *)priv->base;

	setbits_be16(&wdog->wcr, WCR_HALTED);

	return 0;
}

static int mcf_wdt_probe(struct udevice *dev)
{
	struct mcf_wdt_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENOENT;

	priv->fixed_divider = (u32)dev_get_driver_data(dev);

	return 0;
}

static const struct wdt_ops mcf_wdt_ops = {
	.start		= mcf_wdt_start,
	.stop		= mcf_wdt_stop,
	.reset		= mcf_wdt_reset,
	.expire_now	= mcf_wdt_expire_now,
};

static const struct udevice_id mcf_wdt_ids[] = {
	{ .compatible = "fsl,mcf5208-wdt", .data = DIVIDER_5XXX },
	{ .compatible = "fsl,mcf5282-wdt", .data = DIVIDER_5282 },
	{}
};

U_BOOT_DRIVER(mcf_wdt) = {
	.name		= "mcf_wdt",
	.id		= UCLASS_WDT,
	.of_match	= mcf_wdt_ids,
	.probe		= mcf_wdt_probe,
	.ops		= &mcf_wdt_ops,
	.priv_auto	= sizeof(struct mcf_wdt_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};
