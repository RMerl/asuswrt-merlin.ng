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
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/irq.h>

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

/* Set MAX hw timeout to max even no. of seconds in hw timer */
#define MAX_HW_TIMEOUT			(WDOG_TICKS_TO_SECS(WDT_MAX_TIME_TICKS) & (u32)~1U)

/* MAX pre timeout is the maximum wdt expiry we can set without 
 * requireing a half-way down interrupt to continue the count, since
 * after pre-timeout is hit we will disable the interrupt and let it
 * count to 0 unless interrupted by user space . */
#define MAX_PRE_TIMEOUT			(MAX_HW_TIMEOUT)

/* Maximum half-way down interrupt interval */
#define MAX_INT_INTERVAL		((MAX_HW_TIMEOUT)/2)

/* For devices which do not report proper watchdog time left */
#define QUIRK_BCM96XXX_WDT_BROKEN_TIME_LEFT	0x1

struct bcm96xxx_wdt {
	void __iomem		*base;
	uint32_t		curr_pretimeout;
	uint32_t		curr_timeleft;
	uint32_t		elapsed_time;
	uint32_t		hw_timeout;
	int 			wdt_irq;
	int			irq_enabled;
	spinlock_t		lock;
	uint32_t		quirks;
	struct watchdog_device * wdog;
};

static unsigned int heartbeat;
static bool nowayout = WATCHDOG_NOWAYOUT;

//////////// KERNEL API
#define MICROSECS_TO_WDOG_TICKS(x) ((uint32_t)((x) * (WDT_HZ/1000000)))
static struct bcm96xxx_wdt *bcmbca_wdt = NULL;

int bcmbca_wd_start(unsigned int timeout)
{
	unsigned long flags;

	if (!bcmbca_wdt)
		return -1;

	spin_lock_irqsave(&bcmbca_wdt->lock, flags);

	/* Disable IRQ based count as we are overriding everything here */
	if( bcmbca_wdt->irq_enabled )
	{
		disable_irq(bcmbca_wdt->wdt_irq);
		bcmbca_wdt->irq_enabled = 0;
	}

	/* Notify if pretimeout was configured */
	if( bcmbca_wdt->curr_pretimeout )
	{
		watchdog_notify_pretimeout(bcmbca_wdt->wdog);
		bcmbca_wdt->curr_pretimeout = 0;
	}

	writel(MICROSECS_TO_WDOG_TICKS(timeout),  bcmbca_wdt->base + WDT_DEFVAL_REG);
	writel(WDT_START_1, bcmbca_wdt->base + WDT_CTL_REG);
	writel(WDT_START_2, bcmbca_wdt->base + WDT_CTL_REG);

	spin_unlock_irqrestore(&bcmbca_wdt->lock, flags);

	return 0;
}
EXPORT_SYMBOL(bcmbca_wd_start);
////////////////////////

/* Get timeout to be programmed in hw regs for the int enabled implementation */
static int  bcm96xxx_wdt_get_int_based_hw_timeout( struct bcm96xxx_wdt *wdt )
{
	int duration;
	uint32_t hw_timeout;

	/* If we have it pretimeout, return timeleft */
	if( wdt->curr_timeleft <= wdt->curr_pretimeout )
		hw_timeout = wdt->curr_timeleft;
	else
	{
		if( wdt->quirks & QUIRK_BCM96XXX_WDT_BROKEN_TIME_LEFT )
		{
			/* IRQ interval is always 1 sec. Since we cannot readback 
			 * timeleft from hw registers register, we will use the 
			 * 1 second interrupt resolution to determine time left */
			hw_timeout = 2;
		}
		else
		{
			/* If timeleft is > pretimeout */
			duration = wdt->curr_timeleft - wdt->curr_pretimeout;
			if ( duration > MAX_INT_INTERVAL )
			{
				/* Duration is larger than maximum interrupt interval, we will
				 * need multiple IRQs to service the entire timeout period. So
				 * we program hw_timeout for maximum irq interval time * 2 */
				hw_timeout = MAX_INT_INTERVAL * 2;
			}
			else
			{
				/* If wdt->curr_pretimeout is configured, we need a final interrupt at 
				 * wdt->curr_pretimeout point, therefore we configure hw_timout as 
				 * 2*duration. If pretimeout is not configured, then we dont
				 * need any more interrupts anymore, and we configure hw_timeout
				 * as the duration */
				if( wdt->curr_pretimeout )
					hw_timeout = duration * 2;	
				else
					hw_timeout = duration;
			}
		}
	}

	return hw_timeout;
}

static int bcm96xxx_wdt_start(struct watchdog_device *wdog)
{
	struct bcm96xxx_wdt *wdt = watchdog_get_drvdata(wdog);
        unsigned long flags;
	spin_lock_irqsave(&wdt->lock, flags);

	if(!wdt->wdt_irq)
	{
		/* If no IRQ is configured, we ignore pretimeout */
		wdog->pretimeout = 0;

		/* Disable half-way down interrupt if enabled */
		if( wdt->irq_enabled )
		{
			disable_irq(wdt->wdt_irq);
			wdt->irq_enabled = 0;
		}

		/* If no IRQ, scale down timeout to hw timer maximum */
		if( wdog->timeout > WDOG_TICKS_TO_SECS(WDT_MAX_TIME_TICKS ) )
			wdog->timeout = WDOG_TICKS_TO_SECS(WDT_MAX_TIME_TICKS);
	}

	wdt->curr_pretimeout = wdog->pretimeout;
	wdt->elapsed_time = 0;
	wdt->curr_timeleft = wdog->timeout;

	if(!wdt->wdt_irq) 
	{
		wdt->hw_timeout = wdog->timeout;
		pr_debug("wdt_start: hw_timeout:%d set\n", wdt->hw_timeout);
	}
	else
	{
		wdt->hw_timeout = bcm96xxx_wdt_get_int_based_hw_timeout(wdt);
		pr_debug("wdt_start: Arming for %d sec irq\n", wdt->hw_timeout/2);
	}

	writel(SECS_TO_WDOG_TICKS(wdt->hw_timeout),  wdt->base + WDT_DEFVAL_REG);
	writel(WDT_START_1, wdt->base + WDT_CTL_REG);
	writel(WDT_START_2, wdt->base + WDT_CTL_REG);

	/* Enable half-way down interrupt if pretimeout is configured */
	if( !wdt->irq_enabled && wdt->curr_pretimeout ) 
	{
		enable_irq(wdt->wdt_irq);
		wdt->irq_enabled = 1;
	}

	spin_unlock_irqrestore(&wdt->lock, flags);

	return 0;
}

static int bcm96xxx_wdt_stop(struct watchdog_device *wdog)
{
	struct bcm96xxx_wdt *wdt = watchdog_get_drvdata(wdog);

	writel(WDT_STOP_1, wdt->base + WDT_CTL_REG);
	writel(WDT_STOP_2, wdt->base + WDT_CTL_REG);

	return 0;
}

static int bcm96xxx_wdt_set_timeout(struct watchdog_device *wdog, unsigned int t)
{
	pr_debug("wdt_set_timeout:%d set\n", t);
	wdog->timeout = t;
	return 0;
}

static int bcm96xxx_wdt_set_pretimeout(struct watchdog_device *wdog, unsigned int t)
{
	pr_debug("wdt_set_pretimeout:%d set\n", t);
	if( t > MAX_PRE_TIMEOUT )
	{
		printk(KERN_ERR "Invalid pretimeout %d. Max: %d\n",
			t, MAX_PRE_TIMEOUT);
		return -EINVAL;
	}

	wdog->pretimeout= t;
	return 0;
}

static unsigned int bcm96xxx_wdt_get_timeleft(struct watchdog_device *wdog)
{
	struct bcm96xxx_wdt *wdt = watchdog_get_drvdata(wdog);
	unsigned long flags;
	uint32_t total_timeleft;
	volatile uint32_t hw_timeleft;
	uint32_t hw_sec_elapsed;

	spin_lock_irqsave(&wdt->lock, flags);

	/* Calculate instantaneous time left */
	if( wdt->quirks & QUIRK_BCM96XXX_WDT_BROKEN_TIME_LEFT )
	{
		/* IRQ resolution is 1sec, just return time left */
		total_timeleft = wdt->curr_timeleft;
	}
	else
	{
		/* Calculate actual time left based on register value */
		hw_timeleft = readl_relaxed(wdt->base + WDT_CTL_REG);	
		hw_sec_elapsed = wdt->hw_timeout - WDOG_TICKS_TO_SECS(hw_timeleft);
		total_timeleft = wdt->curr_timeleft - hw_sec_elapsed;
	}

	spin_unlock_irqrestore(&wdt->lock, flags);
	return total_timeleft;
}

static struct watchdog_ops bcm96xxx_wdt_ops = {
	.owner =	THIS_MODULE,
	.start =	bcm96xxx_wdt_start,
	.stop =		bcm96xxx_wdt_stop,
	.set_timeout =	bcm96xxx_wdt_set_timeout,
	.set_pretimeout = bcm96xxx_wdt_set_pretimeout,
	.get_timeleft =	bcm96xxx_wdt_get_timeleft,
};

static struct watchdog_info bcm96xxx_wdt_info = {
	.options =	WDIOF_SETTIMEOUT | WDIOF_MAGICCLOSE |
			WDIOF_KEEPALIVEPING | WDIOF_PRETIMEOUT,
	.identity =	"BCM96xxx Watchdog timer",
};

static struct watchdog_device bcm96xxx_wdt_wdd = {
	.info =		&bcm96xxx_wdt_info,
	.ops =		&bcm96xxx_wdt_ops,
	.min_timeout =	0,
	.max_timeout =	0,
	.timeout =	WDOG_TICKS_TO_SECS(WDT_MAX_TIME_TICKS),
	.pretimeout =	1,
};

static irqreturn_t bcm96xxx_wdt_irq(int irq, void *wdog)
{
	uint32_t hw_timeleft;
	uint32_t hw_sec_elapsed;
	uint32_t irq_int_sec;
	struct bcm96xxx_wdt *wdt = watchdog_get_drvdata((struct watchdog_device *)wdog);

	/* Adjust total timeleft and elapsed time */
	if( wdt->quirks & QUIRK_BCM96XXX_WDT_BROKEN_TIME_LEFT )
	{
		/* Broken Time left: Interrupt resolution is always 1 sec */
		wdt->curr_timeleft--;
		wdt->elapsed_time++;
	}
	else
	{
		/* Update timeleft based on actual IRQ interval */

		/* Calculate actual IRQ interval */
		hw_timeleft = readl_relaxed(wdt->base + WDT_CTL_REG);
		hw_sec_elapsed = wdt->hw_timeout - WDOG_TICKS_TO_SECS(hw_timeleft);

		/* If our expected irq interval and the actual irq interval differ
		 * by more than 1 second, we probably have a missed IRQ situation.
		 * In that case, use the actual irq interval as the elapsed time.
		 * If not, then use the expected irq interval as the elasped time 
		 */
		pr_debug("wdt_irq: IRQ int exp:%d actual:%d\n", wdt->hw_timeout/2, hw_sec_elapsed);
		if( (hw_sec_elapsed - (wdt->hw_timeout/2)) > 1 )
			irq_int_sec = hw_sec_elapsed;
		else
			irq_int_sec = wdt->hw_timeout/2;

		wdt->curr_timeleft -= irq_int_sec;
		wdt->elapsed_time  += irq_int_sec;
	}

	pr_debug("wdt_irq: [left/elap/tout] [%d/%d/%d] sec! \n", 
		wdt->curr_timeleft, wdt->elapsed_time,
		((struct watchdog_device *)wdog)->timeout);

	/* Compute next hw_timeout to be programmed */
	wdt->hw_timeout = bcm96xxx_wdt_get_int_based_hw_timeout(wdt);
		
	/* Disable interrupts if pre-timeout has been hit OR if there is no pre-timeout
	 * and our computed hw_timeout is equal to the timeleft */
	if( (!wdt->curr_pretimeout && (wdt->curr_timeleft == wdt->hw_timeout ))
	 || ( wdt->curr_timeleft <= wdt->curr_pretimeout ) )
	{
		if( wdt->curr_pretimeout )
		{
			watchdog_notify_pretimeout((struct watchdog_device *) wdog);
			wdt->curr_pretimeout = 0;
		}
		wdt->irq_enabled = 0;
	}

	if(!wdt->irq_enabled)
		printk(KERN_CRIT "wdt_irq: Expiring in %d! \n", wdt->hw_timeout);
	else
		pr_debug("wdt_irq: Arming for %d sec irq\n", wdt->hw_timeout/2);

	writel(SECS_TO_WDOG_TICKS(wdt->hw_timeout), wdt->base + WDT_DEFVAL_REG);
	writel(WDT_START_1, wdt->base + WDT_CTL_REG);
	writel(WDT_START_2, wdt->base + WDT_CTL_REG);

	if( wdt->quirks & QUIRK_BCM96XXX_WDT_BROKEN_TIME_LEFT )
	{
		/* If timeleft is broken, then we are entirely dependent
		 * on an accurate number of interrupts to determine
		 * the time left. To make sure we dont get duplicate irqs
		 * when the WDT hardware takes a bit longer to de-assert
		 * the IRQ line, we do another read to ensure that our
		 * previous writes actually get flushed to the hardware
		 * and take affect.
		 */
		hw_timeleft = readl_relaxed(wdt->base + WDT_CTL_REG);
	 }

	/* Disable interrupt if wd is expiring after pretimeout */
	if( !wdt->irq_enabled )
		disable_irq_nosync(wdt->wdt_irq);

	return IRQ_HANDLED;
}

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

	wdt->irq_enabled = 0;
	wdt->wdt_irq = platform_get_irq(pdev, 0);

	if (wdt->wdt_irq > 0) 
	{
		/*
		 * Not all supported platforms specify an interrupt for the
		 * watchdog, so let's make it optional.
		 */
		err = devm_request_irq(&pdev->dev, wdt->wdt_irq,
				       bcm96xxx_wdt_irq, 0, pdev->name, &bcm96xxx_wdt_wdd);
		if (err < 0)
		{
			wdt->wdt_irq = 0; 
			dev_warn(&pdev->dev, "failed to request IRQ\n");
		}
		else
			wdt->irq_enabled = 1;
	}
	else
	{
		wdt->wdt_irq = 0;
	}

	wdt->base = of_iomap(np, 0);
	if (!wdt->base) {
		dev_err(dev, "Failed to remap watchdog regs");
		return -ENODEV;
	}

	/* Store parent watchdog device pointer in drv data */
	wdt->wdog = &bcm96xxx_wdt_wdd;

	/* Get quirks */
	if (of_get_property(np, "broken-reg-time-left", NULL))
		wdt->quirks |= QUIRK_BCM96XXX_WDT_BROKEN_TIME_LEFT;

	/* Register watchdog */
	watchdog_set_drvdata(&bcm96xxx_wdt_wdd, wdt);
	watchdog_init_timeout(&bcm96xxx_wdt_wdd, heartbeat, dev);
	watchdog_set_nowayout(&bcm96xxx_wdt_wdd, nowayout);
	err = watchdog_register_device(&bcm96xxx_wdt_wdd);
	if (err) {
		dev_err(dev, "Failed to register watchdog device");
		iounmap(wdt->base);
		return err;
	}

	if (!bcmbca_wdt)
		bcmbca_wdt = wdt;

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
