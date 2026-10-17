/*
 * watchdog.c - driver for i.mx on-chip watchdog
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <hang.h>
#include <asm/io.h>
#include <wdt.h>
#include <watchdog.h>
#include <asm/arch/imx-regs.h>
#ifdef CONFIG_FSL_LSCH2
#include <asm/arch/immap_lsch2.h>
#endif
#include <fsl_wdog.h>
#include <div64.h>

#define TIMEOUT_MAX	128000
#define TIMEOUT_MIN	500

static void imx_watchdog_expire_now(struct watchdog_regs *wdog, bool ext_reset)
{
	u16 wcr = WCR_WDE;

	if (ext_reset)
		wcr |= WCR_SRS; /* do not assert internal reset */
	else
		wcr |= WCR_WDA; /* do not assert external reset */

	/* Write 3 times to ensure it works, due to IMX6Q errata ERR004346 */
	writew(wcr, &wdog->wcr);
	writew(wcr, &wdog->wcr);
	writew(wcr, &wdog->wcr);

	while (1) {
		/*
		 * spin before reset
		 */
	}
}

#if !defined(CONFIG_IMX_WATCHDOG) || \
    (defined(CONFIG_IMX_WATCHDOG) && !CONFIG_IS_ENABLED(WDT))
void __attribute__((weak)) reset_cpu(void)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	imx_watchdog_expire_now(wdog, true);
}
#endif

#if defined(CONFIG_IMX_WATCHDOG)
static void imx_watchdog_reset(struct watchdog_regs *wdog)
{
#ifndef CONFIG_WATCHDOG_RESET_DISABLE
	writew(0x5555, &wdog->wsr);
	writew(0xaaaa, &wdog->wsr);
#endif /* CONFIG_WATCHDOG_RESET_DISABLE*/
}

static void imx_watchdog_init(struct watchdog_regs *wdog, bool ext_reset,
			      u64 timeout)
{
	u16 wcr;

	/*
	 * The timer watchdog can be set between
	 * 0.5 and 128 Seconds.
	 */
	timeout = max_t(u64, timeout, TIMEOUT_MIN);
	timeout = min_t(u64, timeout, TIMEOUT_MAX);
	timeout = lldiv(timeout, 500) - 1;

#ifdef CONFIG_FSL_LSCH2
	wcr = (WCR_WDA | WCR_SRS | WCR_WDE) << 8 | timeout;
#else
	wcr = WCR_WDZST | WCR_WDBG | WCR_WDE | WCR_SRS |
		WCR_WDA | SET_WCR_WT(timeout);
	if (ext_reset)
		wcr |= WCR_WDT;
#endif /* CONFIG_FSL_LSCH2*/
	writew(wcr, &wdog->wcr);
	imx_watchdog_reset(wdog);
}

#if !CONFIG_IS_ENABLED(WDT)
void hw_watchdog_reset(void)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	imx_watchdog_reset(wdog);
}

void hw_watchdog_init(void)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	imx_watchdog_init(wdog, true, CONFIG_WATCHDOG_TIMEOUT_MSECS);
}
#else
struct imx_wdt_priv {
	void __iomem *base;
	bool ext_reset;
};

static int imx_wdt_reset(struct udevice *dev)
{
	struct imx_wdt_priv *priv = dev_get_priv(dev);

	imx_watchdog_reset(priv->base);

	return 0;
}

static int imx_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct imx_wdt_priv *priv = dev_get_priv(dev);

	imx_watchdog_expire_now(priv->base, priv->ext_reset);
	hang();

	return 0;
}

static int imx_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct imx_wdt_priv *priv = dev_get_priv(dev);

	imx_watchdog_init(priv->base, priv->ext_reset, timeout);

	return 0;
}

static int imx_wdt_probe(struct udevice *dev)
{
	struct imx_wdt_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENOENT;

	priv->ext_reset = dev_read_bool(dev, "fsl,ext-reset-output");

	return 0;
}

static const struct wdt_ops imx_wdt_ops = {
	.start		= imx_wdt_start,
	.reset		= imx_wdt_reset,
	.expire_now	= imx_wdt_expire_now,
};

static const struct udevice_id imx_wdt_ids[] = {
	{ .compatible = "fsl,imx21-wdt" },
	{}
};

U_BOOT_DRIVER(imx_wdt) = {
	.name		= "imx_wdt",
	.id		= UCLASS_WDT,
	.of_match	= imx_wdt_ids,
	.probe		= imx_wdt_probe,
	.ops		= &imx_wdt_ops,
	.priv_auto	= sizeof(struct imx_wdt_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};
#endif
#endif
