#if defined(CONFIG_BCM_KF_WDT)
/**************************************************************
 * bcm96xxx_wdt.c Watchdog driver for Broadcom BCM96xxx
 *
 * Author: Farhan Ali <fali@broadcom.com>
 * Based on bcm2835_wdt.c
 *
 * Copyright (c) 2014 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2016:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 *
 ************************************************************/
#include <linux/types.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>

/* Watchdog default count register */
#define WDT_DEFVAL_REG			0x0

/* Watchdog control register */
#define WDT_CTL_REG			0x4

/* Watchdog control register constants */
#define WDT_START_1			(0xff00)
#define WDT_START_2			(0x00ff)
#define WDT_STOP_1			(0xee00)
#define WDT_STOP_2			(0x00ee)

/* Watchdog reset length register */
#define WDT_RSTLEN_REG			0x8

/* Watchdog soft reset register (BCM6328 only) */
#define WDT_SOFTRESET_REG		0xc

#define WDT_HZ				50000000 /* Fclk */
#define WDT_MAX_TIME_TICKS		0xffffffff
#define WDT_DEFAULT_TIME_SECS		10

#define SECS_TO_WDOG_TICKS(x) ((uint32_t)((x) * WDT_HZ))
#define WDOG_TICKS_TO_SECS(x) ((uint32_t)((x) / WDT_HZ))

struct bcm96xxx_wdt {
	void __iomem		*base;
	spinlock_t		lock;
};

static unsigned int heartbeat;
static bool nowayout = WATCHDOG_NOWAYOUT;

static int bcm96xxx_wdt_start(struct watchdog_device *wdog)
{
	struct bcm96xxx_wdt *wdt = watchdog_get_drvdata(wdog);
        unsigned long flags;
	spin_lock_irqsave(&wdt->lock, flags);

	writel_relaxed(SECS_TO_WDOG_TICKS(wdog->timeout),  wdt->base + WDT_DEFVAL_REG);
	writel_relaxed(WDT_START_1, wdt->base + WDT_CTL_REG);
	writel_relaxed(WDT_START_2, wdt->base + WDT_CTL_REG);

	spin_unlock_irqrestore(&wdt->lock, flags);

	return 0;
}

static int bcm96xxx_wdt_stop(struct watchdog_device *wdog)
{
	struct bcm96xxx_wdt *wdt = watchdog_get_drvdata(wdog);

	writel_relaxed(WDT_STOP_1, wdt->base + WDT_CTL_REG);
	writel_relaxed(WDT_STOP_2, wdt->base + WDT_CTL_REG);

	dev_info(wdog->dev, "Watchdog timer stopped");
	return 0;
}

static int bcm96xxx_wdt_set_timeout(struct watchdog_device *wdog, unsigned int t)
{
	wdog->timeout = t;
	return 0;
}

static unsigned int bcm96xxx_wdt_get_timeleft(struct watchdog_device *wdog)
{
	struct bcm96xxx_wdt *wdt = watchdog_get_drvdata(wdog);

	uint32_t ret = readl_relaxed(wdt->base + WDT_CTL_REG);
	return WDOG_TICKS_TO_SECS(ret);
}

static struct watchdog_ops bcm96xxx_wdt_ops = {
	.owner =	THIS_MODULE,
	.start =	bcm96xxx_wdt_start,
	.stop =		bcm96xxx_wdt_stop,
	.set_timeout =	bcm96xxx_wdt_set_timeout,
	.get_timeleft =	bcm96xxx_wdt_get_timeleft,
};

static struct watchdog_info bcm96xxx_wdt_info = {
	.options =	WDIOF_SETTIMEOUT | WDIOF_MAGICCLOSE |
			WDIOF_KEEPALIVEPING,
	.identity =	"BCM96xxx Watchdog timer",
};

static struct watchdog_device bcm96xxx_wdt_wdd = {
	.info =		&bcm96xxx_wdt_info,
	.ops =		&bcm96xxx_wdt_ops,
	.min_timeout =	1,
	.max_timeout =	WDOG_TICKS_TO_SECS(WDT_MAX_TIME_TICKS),
	.timeout =	WDOG_TICKS_TO_SECS(WDT_MAX_TIME_TICKS),
};

static int bcm96xxx_wdt_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct bcm96xxx_wdt *wdt;
	int err;

	wdt = devm_kzalloc(dev, sizeof(struct bcm96xxx_wdt), GFP_KERNEL);
	if (!wdt)
		return -ENOMEM;
	platform_set_drvdata(pdev, wdt);

	spin_lock_init(&wdt->lock);

	wdt->base = of_iomap(np, 0);
	if (!wdt->base) {
		dev_err(dev, "Failed to remap watchdog regs");
		return -ENODEV;
	}

	watchdog_set_drvdata(&bcm96xxx_wdt_wdd, wdt);
	watchdog_init_timeout(&bcm96xxx_wdt_wdd, heartbeat, dev);
	watchdog_set_nowayout(&bcm96xxx_wdt_wdd, nowayout);
	err = watchdog_register_device(&bcm96xxx_wdt_wdd);
	if (err) {
		dev_err(dev, "Failed to register watchdog device");
		iounmap(wdt->base);
		return err;
	}

	dev_info(dev, "Broadcom BCM96xxx watchdog timer");
	return 0;
}

static int bcm96xxx_wdt_remove(struct platform_device *pdev)
{
	struct bcm96xxx_wdt *wdt = platform_get_drvdata(pdev);
	bcm96xxx_wdt_stop(&bcm96xxx_wdt_wdd);
	watchdog_unregister_device(&bcm96xxx_wdt_wdd);
	iounmap(wdt->base);

	return 0;
}

static void bcm96xxx_wdt_shutdown(struct platform_device *pdev)
{
	bcm96xxx_wdt_stop(&bcm96xxx_wdt_wdd);
}

static const struct of_device_id bcm96xxx_wdt_of_match[] = {
	{ .compatible = "brcm,bcm96xxx-wdt", },
	{},
};
MODULE_DEVICE_TABLE(of, bcm96xxx_wdt_of_match);

static struct platform_driver bcm96xxx_wdt_driver = {
	.probe		= bcm96xxx_wdt_probe,
	.remove		= bcm96xxx_wdt_remove,
	.shutdown	= bcm96xxx_wdt_shutdown,
	.driver = {
                .name =         "bcm96xxx-wdt",           
		.of_match_table = bcm96xxx_wdt_of_match,
	},
};
module_platform_driver(bcm96xxx_wdt_driver);

module_param(heartbeat, uint, 0);
MODULE_PARM_DESC(heartbeat, "Initial watchdog heartbeat in seconds");

module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default="
				__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

MODULE_AUTHOR("Farhan Ali <fali@broadcom.com>");
MODULE_DESCRIPTION("Driver for Broadcom BCM96xxx watchdog timer");
MODULE_LICENSE("GPL");
#endif
