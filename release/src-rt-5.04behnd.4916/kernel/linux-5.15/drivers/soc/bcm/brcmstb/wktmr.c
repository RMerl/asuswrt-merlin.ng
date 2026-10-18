/*
 * Copyright © 2014-2023 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/suspend.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/pm.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/pm_wakeup.h>
#include <linux/reboot.h>
#include <linux/rtc.h>
#ifdef CONFIG_MIPS
#include <linux/clocksource.h>
#endif

#define DRV_NAME	"brcm-waketimer"

static struct brcmstb_waketmr {
	struct rtc_device *rtc;
	struct device *dev;
	void __iomem *base;
	unsigned int wake_irq;
	unsigned int timer_irq;
	unsigned long rtc_alarm;
	int wake_timeout;
	struct notifier_block reboot_notifier;
	bool timer_irq_en;
} wktimer;

/* No timeout */
#define BRCMSTB_WKTMR_DEFAULT_TIMEOUT	(-1)

#define BRCMSTB_WKTMR_EVENT		0x00
#define  WKTMR_ALARM_EVENT		BIT(0)
#define BRCMSTB_WKTMR_COUNTER		0x04
#define BRCMSTB_WKTMR_ALARM		0x08
#define BRCMSTB_WKTMR_PRESCALER		0x0C
#define BRCMSTB_WKTMR_PRESCALER_VAL	0x10

/* Fixed 27Mhz frequency since WKTMR is in the UPG clock domain. This
 * information should come from Device Tree eventually
 */
#define WKTMR_FREQ		27000000

static inline bool brcmstb_waketmr_is_pending(struct brcmstb_waketmr *timer)
{
	u32 reg;

	reg = readl_relaxed(timer->base + BRCMSTB_WKTMR_EVENT);
	return !!(reg & WKTMR_ALARM_EVENT);
}

static inline void brcmstb_waketmr_clear_alarm(struct brcmstb_waketmr *timer)
{
	u32 reg;

	reg = readl_relaxed(timer->base + BRCMSTB_WKTMR_COUNTER);
	writel_relaxed(reg - 1, timer->base + BRCMSTB_WKTMR_ALARM);
	writel_relaxed(WKTMR_ALARM_EVENT, timer->base + BRCMSTB_WKTMR_EVENT);
	(void)readl_relaxed(timer->base + BRCMSTB_WKTMR_EVENT);
}

static void brcmstb_waketmr_set_alarm(struct brcmstb_waketmr *timer,
		unsigned int secs)
{
	unsigned int now;

	brcmstb_waketmr_clear_alarm(timer);

/*
 * We use the waketimer as a clocksource for SMP on MIPS. So we do not want
 * to reset the prescaler here, or it will screw up the timing.
 */
#ifndef CONFIG_MIPS
	/* Make sure we are actually counting in seconds */
	writel_relaxed(WKTMR_FREQ, timer->base + BRCMSTB_WKTMR_PRESCALER);
#endif

	writel_relaxed(secs, timer->base + BRCMSTB_WKTMR_ALARM);
	now = readl_relaxed(timer->base + BRCMSTB_WKTMR_COUNTER);

	while ((int)(secs - now) <= 0 &&
		!brcmstb_waketmr_is_pending(timer)) {
		secs = now + 1;
		writel_relaxed(secs, timer->base + BRCMSTB_WKTMR_ALARM);
		now = readl_relaxed(timer->base + BRCMSTB_WKTMR_COUNTER);
	}
}

static irqreturn_t brcmstb_waketmr_irq(int irq, void *data)
{
	struct brcmstb_waketmr *timer = data;
	if (!timer->timer_irq_en)
		pm_wakeup_event(timer->dev, 0);
	return IRQ_HANDLED;
}

#ifdef CONFIG_RTC_CLASS
static irqreturn_t brcmstb_timer_irq(int irq, void *data)
{
	struct brcmstb_waketmr *timer = data;

	/* Ignore spurious interrupts */
	if(!brcmstb_waketmr_is_pending(timer))
		return IRQ_HANDLED;

	writel_relaxed(WKTMR_ALARM_EVENT, timer->base + BRCMSTB_WKTMR_EVENT);
	disable_irq_nosync(irq);
	timer->timer_irq_en = false;

	rtc_update_irq(timer->rtc, 1, RTC_IRQF | RTC_AF);
	return IRQ_HANDLED;
}
#endif

struct wktmr_time {
	u32			sec;
	u32			pre;
};

static void wktmr_read(struct wktmr_time *t)
{
	u32 tmp;

	do {
		t->sec = readl_relaxed(wktimer.base + BRCMSTB_WKTMR_COUNTER);
		tmp = readl_relaxed(wktimer.base + BRCMSTB_WKTMR_PRESCALER_VAL);
	} while (tmp >= WKTMR_FREQ);

	t->pre = WKTMR_FREQ - tmp;
}

/* Use this wrapper if on MIPS */
#ifdef CONFIG_MIPS
static void brcmstb_waketmr_read_persistent_clock(struct timespec64 *ts)
{
	struct wktmr_time now;

	wktmr_read(&now);

	ts->tv_sec = now.sec;
	ts->tv_nsec = now.pre * (NSEC_PER_SEC / WKTMR_FREQ);
}

void read_persistent_clock64(struct timespec64 *ts)
{
	/*
	 * read_persistent_clock64 gets called before wake
	 * timer can be initialized. If we are not initialized
	 * yet, default to 0.
	 */
	if (wktimer.base) {
		brcmstb_waketmr_read_persistent_clock(ts);
	} else {
		ts->tv_sec = 0;
		ts->tv_nsec = 0;
	}
}
#endif /* CONFIG_MIPS */

static ssize_t brcmstb_waketmr_timeout_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct brcmstb_waketmr *timer = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", timer->wake_timeout);
}

static ssize_t brcmstb_waketmr_timeout_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct brcmstb_waketmr *timer = dev_get_drvdata(dev);
	int timeout;
	int ret;

#ifdef CONFIG_RTC_CLASS
	dev_warn(dev, "Using sysfs attributes, consider using 'rtcwake'\n");
#endif
	ret = kstrtoint(buf, 0, &timeout);
	if (ret < 0)
		return ret;

	/* Allow -1 as "no timeout" */
	if (timeout < -1)
		return -EINVAL;

	timer->wake_timeout = timeout;

	return count;
}

static const DEVICE_ATTR(timeout, S_IRUGO | S_IWUSR,
		brcmstb_waketmr_timeout_show,
		brcmstb_waketmr_timeout_store);

static inline bool brcmstb_waketmr_wakeup_en(struct brcmstb_waketmr *timer)
{
	return (device_may_wakeup(timer->dev) &&
		(timer->timer_irq_en || timer->wake_timeout >= 0));
}

static int brcmstb_waketmr_prepare_suspend(struct brcmstb_waketmr *timer)
{
	struct device *dev = timer->dev;
	unsigned int t;
	int ret;

	if (timer->timer_irq)
		disable_irq(timer->timer_irq);

	if (brcmstb_waketmr_wakeup_en(timer)) {
		dev_dbg(dev, "enable wake IRQ\n");
		ret = enable_irq_wake(timer->wake_irq);
		if (ret) {
			dev_err(dev, "failed to enable wake-up interrupt\n");
			if (timer->timer_irq)
				enable_irq(timer->timer_irq);
			return ret;
		}
		if (timer->timer_irq_en) {
			ret = enable_irq_wake(timer->timer_irq);
			if (ret) {
				dev_err(dev, "failed to enable rtc interrupt\n");
				disable_irq_wake(timer->wake_irq);
				enable_irq(timer->timer_irq);
				return ret;
			}
		}

		/*
		 * Using the ioctl() interface, the alarm timer will have
		 * already been enabled in brcmstb_waketmr_setalarm().
		 * However, using the sysfs interface, we have to enable it
		 * here, because brcmstb_waketmr_setalarm() isn't called.
		 */
		if (!timer->timer_irq_en) {
			t = readl_relaxed(timer->base + BRCMSTB_WKTMR_COUNTER);
			t += timer->wake_timeout + 1;
			brcmstb_waketmr_set_alarm(timer, t);
		}
	} else {
		dev_dbg(dev, "nothing to do: wake_timeout: %d\n",
				timer->wake_timeout);
	}
	return 0;
}

/* If enabled as a wakeup-source, arm the timer when powering off */
static int brcmstb_waketmr_reboot(struct notifier_block *nb,
		unsigned long action, void *data)
{
	struct brcmstb_waketmr *timer;
	timer = container_of(nb, struct brcmstb_waketmr, reboot_notifier);

	/* Set timer for cold boot */
	if (action == SYS_POWER_OFF)
		brcmstb_waketmr_prepare_suspend(timer);

	return NOTIFY_DONE;
}

#ifdef CONFIG_RTC_CLASS
static int brcmstb_waketmr_gettime(struct device *dev,
				   struct rtc_time *tm)
{
	struct wktmr_time now;

	wktmr_read(&now);

	rtc_time64_to_tm(now.sec, tm);

	return 0;
}

static int brcmstb_waketmr_settime(struct device *dev,
				   struct rtc_time *tm)
{
	struct brcmstb_waketmr *timer = dev_get_drvdata(dev);
	time64_t sec;
	int ret;

	ret = rtc_valid_tm(tm);
	if (ret)
		return ret;

	sec = rtc_tm_to_time64(tm);

	dev_dbg(dev, "%s: sec=%lld\n", __FUNCTION__, sec);
	writel_relaxed(sec, timer->base + BRCMSTB_WKTMR_COUNTER);

	return 0;
}

static int brcmstb_waketmr_getalarm(struct device *dev,
				    struct rtc_wkalrm *alarm)
{
	struct brcmstb_waketmr *timer = dev_get_drvdata(dev);

	rtc_time64_to_tm(timer->rtc_alarm, &alarm->time);
	alarm->enabled = timer->timer_irq_en;
	alarm->pending = brcmstb_waketmr_is_pending(timer);
	dev_dbg(dev, "%s: alarm pending=%d\n", __FUNCTION__, alarm->pending);

	return 0;
}

static int brcmstb_waketmr_alarm_enable(struct device *dev,
					unsigned int enabled)
{
	struct brcmstb_waketmr *timer = dev_get_drvdata(dev);

	if (!timer->timer_irq)
		return 0;

	if (enabled && !timer->timer_irq_en) {
		if ((int)(readl_relaxed(timer->base + BRCMSTB_WKTMR_COUNTER) -
		    readl_relaxed(timer->base + BRCMSTB_WKTMR_ALARM)) >= 0 &&
		    !brcmstb_waketmr_is_pending(timer))
			return -1;
		timer->timer_irq_en = true;
		enable_irq(timer->timer_irq);
	} else if (!enabled && timer->timer_irq_en) {
		disable_irq(timer->timer_irq);
		timer->timer_irq_en = false;
	}

	dev_dbg(dev, "%s: enabled=%d\n", __FUNCTION__, enabled);

	return 0;
}

static int brcmstb_waketmr_setalarm(struct device *dev,
				     struct rtc_wkalrm *alarm)
{
	struct brcmstb_waketmr *timer = dev_get_drvdata(dev);
	time64_t sec;

	sec = rtc_tm_to_time64(&alarm->time);
	timer->rtc_alarm = sec;

	if (timer->timer_irq_en) {
		disable_irq(timer->timer_irq);
		timer->timer_irq_en = false;
	}
	dev_dbg(dev, "%s: timeout=%lld\n", __FUNCTION__, sec);
	brcmstb_waketmr_set_alarm(timer, sec);

	if (alarm->enabled && timer->timer_irq) {
		timer->timer_irq_en = true;
		enable_irq(timer->timer_irq);
	}

	return 0;
}

static const struct rtc_class_ops brcmstb_waketmr_ops = {
	.read_time	= brcmstb_waketmr_gettime,
	.set_time	= brcmstb_waketmr_settime,
	.read_alarm	= brcmstb_waketmr_getalarm,
	.set_alarm	= brcmstb_waketmr_setalarm,
	.alarm_irq_enable = brcmstb_waketmr_alarm_enable,
};
#endif /* CONFIG_RTC_CLASS */

/*
 * MIPS uses the Wake Timer as the clocksource instead of the
 * MIPS counter/compare registers in each core because of issues
 * synchronizing multiple counters on SMP systems.
 */
#ifdef CONFIG_MIPS

static DEFINE_SPINLOCK(wktmr_lock);

static u64 wktmr_cs_read(struct clocksource *cs)
{
	struct wktmr_time t;
	unsigned long flags;

	spin_lock_irqsave(&wktmr_lock, flags);
	wktmr_read(&t);
	spin_unlock_irqrestore(&wktmr_lock, flags);

	return (t.sec * (u64)WKTMR_FREQ) + t.pre;
}

static struct clocksource clocksource_wktmr = {
	.name		= "wktmr",
	.read		= wktmr_cs_read,
	.mask		= CLOCKSOURCE_MASK(64),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static inline void __init init_wktmr_clocksource(void)
{
	clocksource_wktmr.rating = 250;
	clocksource_register_hz(&clocksource_wktmr, WKTMR_FREQ);
}
#else
static inline void __init init_wktmr_clocksource(void)
{
}
#endif /* CONFIG_MIPS */

static int __init brcmstb_waketmr_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct brcmstb_waketmr *timer = &wktimer;
	struct resource *res;
	int ret;

	platform_set_drvdata(pdev, timer);

	if (timer->dev)
		return -EBUSY;

	timer->dev = dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	timer->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(timer->base))
		return PTR_ERR(timer->base);

	/*
	 * Set wakeup capability before requesting wakeup interrupt, so we can
	 * process boot-time "wakeups" (e.g., from S5 soft-off)
	 */
	device_set_wakeup_capable(dev, true);
	device_wakeup_enable(dev);

	timer->wake_irq = platform_get_irq(pdev, 0);
	ret = devm_request_irq(dev, timer->wake_irq, brcmstb_waketmr_irq, 0,
			       DRV_NAME, timer);
	if (ret < 0) {
		dev_err(dev, "Unabled to initialize wake IRQ\n");
		return ret;
	}
	dev_info(dev, "wake IRQ found: %d\n", timer->wake_irq);

	brcmstb_waketmr_clear_alarm(timer);
#ifdef CONFIG_RTC_CLASS
	/* Attempt to initialize non-wake irq */
	timer->timer_irq = platform_get_irq(pdev, 1);
	ret = devm_request_irq(dev, timer->timer_irq, brcmstb_timer_irq, 0,
			 DRV_NAME"-rtc", timer);
	if (ret < 0) {
		timer->timer_irq = 0;
		dev_warn(dev, "Unable to initialize timer IRQ\n");
	} else {
		disable_irq(timer->timer_irq);
		dev_info(dev, "timer IRQ found: %d\n", timer->timer_irq);
	}

	timer->rtc = devm_rtc_device_register(dev, "brcmstb-waketmr",
					 &brcmstb_waketmr_ops, THIS_MODULE);
	if (IS_ERR(timer->rtc)) {
		dev_err(dev, "unable to register device\n");
		return PTR_ERR(timer->rtc);
	}
#endif
	timer->reboot_notifier.notifier_call = brcmstb_waketmr_reboot;
	register_reboot_notifier(&timer->reboot_notifier);

	timer->wake_timeout = BRCMSTB_WKTMR_DEFAULT_TIMEOUT;

	ret = device_create_file(dev, &dev_attr_timeout);
	if (ret) {
		unregister_reboot_notifier(&timer->reboot_notifier);
		return ret;
	}

	init_wktmr_clocksource();
	dev_info(dev, "registered wake timer\n");
	return ret;
}

static int brcmstb_waketmr_remove(struct platform_device *pdev)
{
	struct brcmstb_waketmr *timer = dev_get_drvdata(&pdev->dev);

	device_remove_file(&pdev->dev, &dev_attr_timeout);
	unregister_reboot_notifier(&timer->reboot_notifier);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int brcmstb_waketmr_suspend(struct device *dev)
{
	struct brcmstb_waketmr *timer = dev_get_drvdata(dev);

	return brcmstb_waketmr_prepare_suspend(timer);
}

static int brcmstb_waketmr_resume(struct device *dev)
{
	struct brcmstb_waketmr *timer = dev_get_drvdata(dev);
	int ret = 0;

	if (brcmstb_waketmr_wakeup_en(timer)) {
		ret = disable_irq_wake(timer->wake_irq);
		if (timer->timer_irq && timer->timer_irq_en)
			disable_irq_wake(timer->timer_irq);
	}

	if (timer->timer_irq)
		enable_irq(timer->timer_irq);

	return ret;
}
#endif /* CONFIG_PM_SLEEP */

static SIMPLE_DEV_PM_OPS(brcmstb_waketmr_pm_ops, brcmstb_waketmr_suspend,
		brcmstb_waketmr_resume);

static const struct of_device_id brcmstb_waketmr_of_match[] = {
	{ .compatible = "brcm,brcmstb-waketimer" },
	{},
};

static struct platform_driver brcmstb_waketmr_driver = {
	.remove			= brcmstb_waketmr_remove,
	.driver = {
		.name		= DRV_NAME,
		.pm		= &brcmstb_waketmr_pm_ops,
		.of_match_table	= of_match_ptr(brcmstb_waketmr_of_match),
	}
};

static int __init brcmstb_waketmr_init(void)
{
	return platform_driver_probe(&brcmstb_waketmr_driver,
				     brcmstb_waketmr_probe);
}

static void __exit brcmstb_waketmr_exit(void)
{
	platform_driver_unregister(&brcmstb_waketmr_driver);
}

module_init(brcmstb_waketmr_init);
module_exit(brcmstb_waketmr_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Brian Norris");
MODULE_DESCRIPTION("Wake-up timer driver for STB chips");
