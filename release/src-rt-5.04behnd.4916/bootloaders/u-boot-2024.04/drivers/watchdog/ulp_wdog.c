// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <dm.h>
#include <wdt.h>

/*
 * MX7ULP WDOG Register Map
 */
struct wdog_regs {
	u32 cs;
	u32 cnt;
	u32 toval;
	u32 win;
};

struct ulp_wdt_priv {
	struct wdog_regs *wdog;
	u32 clk_rate;
};

#define REFRESH_WORD0 0xA602 /* 1st refresh word */
#define REFRESH_WORD1 0xB480 /* 2nd refresh word */

#define UNLOCK_WORD0 0xC520 /* 1st unlock word */
#define UNLOCK_WORD1 0xD928 /* 2nd unlock word */

#define UNLOCK_WORD 0xD928C520 /* unlock word */
#define REFRESH_WORD 0xB480A602 /* refresh word */

#define WDGCS_WDGE                      BIT(7)
#define WDGCS_WDGUPDATE                 BIT(5)

#define WDGCS_RCS                       BIT(10)
#define WDGCS_ULK                       BIT(11)
#define WDOG_CS_PRES                    BIT(12)
#define WDGCS_CMD32EN                   BIT(13)
#define WDGCS_FLG                       BIT(14)
#define WDGCS_INT			BIT(6)

#define WDG_BUS_CLK                      (0x0)
#define WDG_LPO_CLK                      (0x1)
#define WDG_32KHZ_CLK                    (0x2)
#define WDG_EXT_CLK                      (0x3)

#define CLK_RATE_1KHZ			1000
#define CLK_RATE_32KHZ			125

void hw_watchdog_set_timeout(u16 val)
{
	/* setting timeout value */
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG_BASE_ADDR;

	writel(val, &wdog->toval);
}

void ulp_watchdog_reset(struct wdog_regs *wdog)
{
	if (readl(&wdog->cs) & WDGCS_CMD32EN) {
		writel(REFRESH_WORD, &wdog->cnt);
	} else {
		dmb();
		__raw_writel(REFRESH_WORD0, &wdog->cnt);
		__raw_writel(REFRESH_WORD1, &wdog->cnt);
		dmb();
	}
}

void ulp_watchdog_init(struct wdog_regs *wdog, u16 timeout)
{
	u32 cmd32 = 0;

	if (readl(&wdog->cs) & WDGCS_CMD32EN) {
		writel(UNLOCK_WORD, &wdog->cnt);
		cmd32 = WDGCS_CMD32EN;
	} else {
		dmb();
		__raw_writel(UNLOCK_WORD0, &wdog->cnt);
		__raw_writel(UNLOCK_WORD1, &wdog->cnt);
		dmb();
	}

	/* Wait WDOG Unlock */
	while (!(readl(&wdog->cs) & WDGCS_ULK))
		;

	hw_watchdog_set_timeout(timeout);
	writel(0, &wdog->win);

	/* setting 1-kHz clock source, enable counter running, and clear interrupt */
	if (IS_ENABLED(CONFIG_ARCH_IMX9))
		writel((cmd32 | WDGCS_WDGE | WDGCS_WDGUPDATE | (WDG_LPO_CLK << 8) |
		       WDGCS_FLG | WDOG_CS_PRES | WDGCS_INT), &wdog->cs);
	else
		writel((cmd32 | WDGCS_WDGE | WDGCS_WDGUPDATE | (WDG_LPO_CLK << 8) |
		       WDGCS_FLG), &wdog->cs);

	/* Wait WDOG reconfiguration */
	while (!(readl(&wdog->cs) & WDGCS_RCS))
		;

	ulp_watchdog_reset(wdog);
}

void hw_watchdog_reset(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG_BASE_ADDR;

	ulp_watchdog_reset(wdog);
}

void hw_watchdog_init(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG_BASE_ADDR;

	ulp_watchdog_init(wdog, CONFIG_WATCHDOG_TIMEOUT_MSECS);
}

#if !CONFIG_IS_ENABLED(SYSRESET)
void reset_cpu(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG_BASE_ADDR;
	u32 cmd32 = 0;

	if (readl(&wdog->cs) & WDGCS_CMD32EN) {
		writel(UNLOCK_WORD, &wdog->cnt);
		cmd32 = WDGCS_CMD32EN;
	} else {
		dmb();
		__raw_writel(UNLOCK_WORD0, &wdog->cnt);
		__raw_writel(UNLOCK_WORD1, &wdog->cnt);
		dmb();
	}

	/* Wait WDOG Unlock */
	while (!(readl(&wdog->cs) & WDGCS_ULK))
		;

	hw_watchdog_set_timeout(5); /* 5ms timeout for general; 40ms timeout for imx93 */
	writel(0, &wdog->win);

	/* enable counter running */
	if (IS_ENABLED(CONFIG_ARCH_IMX9))
		writel((cmd32 | WDGCS_WDGE | (WDG_LPO_CLK << 8) | WDOG_CS_PRES |
		       WDGCS_INT), &wdog->cs);
	else
		writel((cmd32 | WDGCS_WDGE | (WDG_LPO_CLK << 8)), &wdog->cs);

	/* Wait WDOG reconfiguration */
	while (!(readl(&wdog->cs) & WDGCS_RCS))
		;

	hw_watchdog_reset();

	while (1);
}
#endif

static int ulp_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct ulp_wdt_priv *priv = dev_get_priv(dev);
	u64 timeout = 0;

	timeout = (timeout_ms * priv->clk_rate) / 1000;
	if (timeout > U16_MAX)
		return -EINVAL;

	ulp_watchdog_init(priv->wdog, (u16)timeout);

	return 0;
}

static int ulp_wdt_reset(struct udevice *dev)
{
	struct ulp_wdt_priv *priv = dev_get_priv(dev);

	ulp_watchdog_reset(priv->wdog);

	return 0;
}

static int ulp_wdt_probe(struct udevice *dev)
{
	struct ulp_wdt_priv *priv = dev_get_priv(dev);

	priv->wdog = dev_read_addr_ptr(dev);
	if (!priv->wdog)
		return -EINVAL;

	priv->clk_rate = (u32)dev_get_driver_data(dev);
	if (!priv->clk_rate)
		return -EINVAL;

	return 0;
}

static const struct wdt_ops ulp_wdt_ops = {
	.start = ulp_wdt_start,
	.reset = ulp_wdt_reset,
};

static const struct udevice_id ulp_wdt_ids[] = {
	{ .compatible = "fsl,imx7ulp-wdt", .data = CLK_RATE_1KHZ },
	{ .compatible = "fsl,imx8ulp-wdt", .data = CLK_RATE_1KHZ },
	{ .compatible = "fsl,imx93-wdt", .data = CLK_RATE_32KHZ },
	{}
};

U_BOOT_DRIVER(ulp_wdt) = {
	.name	= "ulp_wdt",
	.id	= UCLASS_WDT,
	.of_match	= ulp_wdt_ids,
	.priv_auto	= sizeof(struct ulp_wdt_priv),
	.probe		= ulp_wdt_probe,
	.ops	= &ulp_wdt_ops,
};
