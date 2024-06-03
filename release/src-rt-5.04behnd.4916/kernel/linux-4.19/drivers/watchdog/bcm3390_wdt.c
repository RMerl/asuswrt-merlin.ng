#if defined(CONFIG_BCM_KF_WDT)
/*
 * Copyright (C) 2019 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/nmi.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/watchdog.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <asm/irq_regs.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/sched/loadavg.h>
#include <linux/sched/stat.h>
#include <linux/mm.h>
#include <linux/kmsg_dump.h>
#include <asm/ptrace.h>
#include <linux/printk.h>
#include <linux/console.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <linux/console.h>
#include <linux/reboot.h>

#define WDT_START_1		0xff00
#define WDT_START_2		0x00ff
#define WDT_STOP_1		0xee00
#define WDT_STOP_2		0x00ee

#define WDT_TIMEOUT_REG		0x28
#define WDT_CMD_REG		0x2c
#define WDT_CTRL_REG		0x3c
#define WDT_CTRL_MODE_RST	0x0
#define WDT_CTRL_MODE_NMI	0x1
#define WDT_CTRL_MODE_NMI_RST	0x2
#define WDT_INT_STATUS_REG	0x0
#define WDT_INT_REG		0x4
#define WDT_PRETIMEOUT_CTL_REG	0x8
#define WDT_PRETIMEOUT_STS_REG	0x18
#define WDT_HTIMEOUT_INT_MASK	0x10
#define WDT_PRETIMEOUT_INT_MASK	0x01
#define WDT_PRETIMEOUT_ENABLE	0xC0000000
#define WDT_PRETIMEOUT_VAL_MASK	0x3fffffff
#define WDT_CTRL_MODE_VAL_MASK	0x3

#define WDT_MIN_TIMEOUT		1 /* seconds */
#define WDT_DEFAULT_TIMEOUT	30 /* seconds */
#define WDT_DEFAULT_RATE	27000000
#define WDT_DEFAULT_PRETIMEOUT	(WDT_DEFAULT_TIMEOUT/2)
#define WDT_MAX_TIMEOUT		159 /* Maximux Timeout value in seconds */
#define WDT_TIMER_MAX_TIMEOUT 39 /* Maximux Timeout value in seconds */

struct bcm3390_watchdog {
	struct list_head	list;
	void __iomem		*base;
	struct watchdog_device	wdd;
	u32			rate;
	struct clk		*clk;
	u32			ping_stat;
	u32			ping_min;
	u32			ping_avg;
	u32			ping_max;
	u64			ping_count;
	ktime_t		ping_tstamp;
	bool		start;
	u32			half_cnt;
	u32			pretimeout_left;
};

static LIST_HEAD(wdts);
static spinlock_t wdts_lock;

static bool wdt_running;
static bool nowayout = WATCHDOG_NOWAYOUT;

#ifdef CONFIG_BCM3390_WDT_NMI
static atomic_t nmi_cpu_cnt = ATOMIC_INIT(0);
static spinlock_t nmi_lock;
#endif

#define MICROSECS_TO_WDOG_TICKS(x) ((uint32_t)((x) * (WDT_DEFAULT_RATE/1000000)))

int bcmbca_wd_start(unsigned int timeout)
{
	unsigned long flags;
    struct bcm3390_watchdog *wdt;
    
    wdt = list_first_entry(&wdts, struct bcm3390_watchdog, list);

	if (!wdt)
        return -1;
 
	spin_lock_irqsave(&wdts_lock, flags);

    writel(WDT_STOP_1, wdt->base + WDT_CMD_REG);
	writel(WDT_STOP_2, wdt->base + WDT_CMD_REG);

    writel(MICROSECS_TO_WDOG_TICKS(timeout), wdt->base + WDT_TIMEOUT_REG);

    writel(WDT_START_1, wdt->base + WDT_CMD_REG);
	writel(WDT_START_2, wdt->base + WDT_CMD_REG);

    spin_unlock_irqrestore(&wdts_lock, flags);

	return 0;
}
EXPORT_SYMBOL(bcmbca_wd_start);

static void bcm3390_wdt_set_timeout_reg(struct watchdog_device *wdog)
{
	struct bcm3390_watchdog *wdt = watchdog_get_drvdata(wdog);
	u32 timeout;

	timeout = wdt->rate * wdog->timeout;

	writel(timeout, wdt->base + WDT_TIMEOUT_REG);
}

void bcm3390_wdt_ping_stats(struct bcm3390_watchdog *wdt)
{
	u32 latency;

	if (!wdt->ping_stat)
		return;

	wdt->ping_count++;
	latency = ktime_to_ms(ktime_sub(ktime_get_boottime(), wdt->ping_tstamp));
	if (wdt->ping_count > 2) {
		/* Exponential moving average: 31 period */
		wdt->ping_avg =  wdt->ping_avg + (latency>>4) - (wdt->ping_avg>>4);
		if (latency > wdt->ping_max)
			wdt->ping_max = latency;
		if (latency < wdt->ping_min)
			wdt->ping_min = latency;
	}
	wdt->ping_tstamp = ktime_get_boottime();
	pr_debug("WDT Reset stats: Count %lld | Avg %u ms | Min %u ms | Max %u ms | Cur %u\n",
		 wdt->ping_count, wdt->ping_avg, wdt->ping_min, wdt->ping_max, latency);
}

static int bcm3390_wdt_ping(struct watchdog_device *wdog)
{
	struct bcm3390_watchdog *wdt = watchdog_get_drvdata(wdog);
	u32 pretimeout;

	bcm3390_wdt_ping_stats(wdt);
	writel(WDT_START_1, wdt->base + WDT_CMD_REG);
	writel(WDT_START_2, wdt->base + WDT_CMD_REG);

	if (wdog->pretimeout) {
		/* stop pretimeout timer */ 
		writel(0, wdt->base + WDT_PRETIMEOUT_CTL_REG);
		writel(0, wdt->base + WDT_INT_REG);

		/* start pretimeout timer */ 
		pretimeout = wdog->timeout - wdog->pretimeout;

		wdt->pretimeout_left = 0;

		if(pretimeout > WDT_TIMER_MAX_TIMEOUT){
		    wdt->pretimeout_left = pretimeout - WDT_TIMER_MAX_TIMEOUT;
		    pretimeout = WDT_TIMER_MAX_TIMEOUT;
		}

		pretimeout = (wdt->rate * pretimeout) & WDT_PRETIMEOUT_VAL_MASK;
		writel(WDT_PRETIMEOUT_ENABLE | pretimeout, wdt->base + WDT_PRETIMEOUT_CTL_REG);
		writel(WDT_PRETIMEOUT_INT_MASK, wdt->base + WDT_INT_REG);
	} else {
		writel(0, wdt->base + WDT_PRETIMEOUT_CTL_REG);
		writel(0, wdt->base + WDT_INT_REG);
	}
	return 0;
}

static int bcm3390_wdt_start(struct watchdog_device *wdog)
{
	bcm3390_wdt_set_timeout_reg(wdog);
	bcm3390_wdt_ping(wdog);
	return 0;
}

static int bcm3390_wdt_stop(struct watchdog_device *wdog)
{
	struct bcm3390_watchdog *wdt = watchdog_get_drvdata(wdog);

	writel(WDT_STOP_1, wdt->base + WDT_CMD_REG);
	writel(WDT_STOP_2, wdt->base + WDT_CMD_REG);

	writel(0, wdt->base + WDT_PRETIMEOUT_CTL_REG);
	writel(0, wdt->base + WDT_INT_REG);

	return 0;
}

static int bcm3390_wdt_set_timeout(struct watchdog_device *wdog,
				   unsigned int t)
{
	struct bcm3390_watchdog *wdt = watchdog_get_drvdata(wdog);
	/* Can't modify timeout value if watchdog timer is running */
	bcm3390_wdt_stop(wdog);
	wdog->timeout = t;
	wdt->ping_stat = 0;
	bcm3390_wdt_start(wdog);
	wdt->ping_count = 0;
	wdt->ping_avg = 0;
	wdt->ping_min = 0xFFFFFFFF;
	wdt->ping_max = 0;
	wdt->ping_tstamp = ktime_get_boottime();
	wdt->ping_stat = 1;
	return 0;
}

static unsigned int bcm3390_wdt_get_timeleft(struct watchdog_device *wdog)
{
	struct bcm3390_watchdog *wdt = watchdog_get_drvdata(wdog);
	u32 time_left;

	time_left = readl(wdt->base + WDT_CMD_REG);

	return time_left / wdt->rate;
}

static struct watchdog_info bcm3390_wdt_info = {
	.identity	= "Broadcom Watchdog Timer",
	.options	= WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING |
			  WDIOF_MAGICCLOSE | WDIOF_PRETIMEOUT
};

extern void show_regs(struct pt_regs *);

static void bcm3390_wdt_showallcpus(void)
{
	if (!trigger_all_cpu_backtrace()) {
		struct pt_regs *regs = NULL;
		regs = get_irq_regs();
		if (regs) {
			pr_emerg("CPU%d:\n", smp_processor_id());
			show_regs(regs);
		}
	}
}

#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)
static int bcm3390_wdt_loadavg_show(void)
{
	unsigned long avnrun[3];

	get_avenrun(avnrun, FIXED_1/200, 0);

	pr_emerg("%lu.%02lu %lu.%02lu %lu.%02lu %ld/%d\n",
		LOAD_INT(avnrun[0]), LOAD_FRAC(avnrun[0]),
		LOAD_INT(avnrun[1]), LOAD_FRAC(avnrun[1]),
		LOAD_INT(avnrun[2]), LOAD_FRAC(avnrun[2]),
		nr_running(), nr_threads);
	return 0;
}

#ifdef BCM_WDT_TASKLET
struct tasklet_struct kmsg_task;
void bcm3390_wdt_kmsg_work(unsigned long data)
{
	pr_emerg("---[ Call Kmsg Dump ]---\n");
	kmsg_dump(KMSG_DUMP_OOPS);
}
#else
struct work_struct kmsg_work;
void bcm3390_wdt_kmsg_work(struct work_struct *work)
{
	pr_emerg("---[ Call Kmsg Dump ]---\n");
	kmsg_dump(KMSG_DUMP_OOPS);
}
#endif

static void bcm3390_wdt_custom_show(struct bcm3390_watchdog *wdt)
{
	nodemask_t mask;

	if (wdt) {
		pr_emerg("---[ WDT Reset stats ]---\n");
		pr_emerg("Count %lld | Avg %u ms | Min %u ms | Max %u ms\n",
			 wdt->ping_count, wdt->ping_avg, wdt->ping_min,
			 wdt->ping_max);
	}

	pr_emerg("\n---[ CPU Avg Load ]---\n");
	bcm3390_wdt_loadavg_show();

	pr_emerg("\n---[ Show Memory ]---\n");
	nodes_setall(mask);
	show_mem(0, &mask);

	atomic_notifier_call_chain(&panic_notifier_list, 0, "watchdog");
#ifdef CONFIG_BCM3390_WDT_NMI
	pr_emerg("\n---[ Call Kmsg Dump ]---\n");
	kmsg_dump(KMSG_DUMP_OOPS);
#else
#ifdef BCM_WDT_TASKLET
	tasklet_schedule(&kmsg_task);
#else
	schedule_work(&kmsg_work);
#endif
#endif
}

static void bcm3390_wdt_isr_show(void)
{
	nodemask_t mask;

	/* prevent preemption from any other possible sources */
	preempt_disable_notrace();
	smp_send_stop();

	/* local_irq_enable(); */
	console_verbose();
	bust_spinlocks(1);
	debug_locks_off();

	pr_emerg("\n******************** WDT IRQ ********************\n");
	pr_emerg("\n---[ CPU Avg Load ]---\n");
	bcm3390_wdt_loadavg_show();
	pr_emerg("\n---[ Show Memory ]---\n");
	nodes_setall(mask);
	show_mem(0, &mask);
	/* trigger FPM & DQM dump */
	atomic_notifier_call_chain(&panic_notifier_list, 0, "watchdog");
	pr_emerg("\n---[ Backtrace ]---\n");

	/* dump regs & backtrace */
	dump_stack();

	pr_emerg("\n---[ Call Kmsg Dump ]---\n");
	printk_safe_flush_on_panic();

	/* local_irq_disable(); */

	/* trigger persistent storage dump */
	kmsg_dump(KMSG_DUMP_PANIC);
	/* kick klogd */
	bust_spinlocks(0);

	/* hang until WD reset kicks in */
	while (1)
		cpu_relax();
}

static irqreturn_t bcm3390_wdt_isr(int irq, void *arg)
{
	struct bcm3390_watchdog *wdt = arg;
	u32 status = readl(wdt->base + WDT_INT_STATUS_REG);
	u32 mask = readl(wdt->base + WDT_INT_REG);
	u32 pretimeout;

	if (status & WDT_PRETIMEOUT_INT_MASK) {
		/* Clear status */
		writel(status, wdt->base + WDT_INT_STATUS_REG);
        
		if (!wdt->pretimeout_left) {
			pr_emerg("\nWATCHDOG(pre-timeout)\n");
			mask &= ~WDT_PRETIMEOUT_INT_MASK;
			watchdog_notify_pretimeout(&wdt->wdd);
		} else {
		    /* stop pretimeout timer */ 
			writel(0, wdt->base + WDT_PRETIMEOUT_CTL_REG);
			writel(0, wdt->base + WDT_INT_REG);

			if(wdt->pretimeout_left > WDT_TIMER_MAX_TIMEOUT) {
		    	wdt->pretimeout_left -= WDT_TIMER_MAX_TIMEOUT; 
		    	pretimeout = (wdt->rate * WDT_TIMER_MAX_TIMEOUT) & WDT_PRETIMEOUT_VAL_MASK;
			} else {
				pretimeout = (wdt->rate * wdt->pretimeout_left) & WDT_PRETIMEOUT_VAL_MASK;
				wdt->pretimeout_left = 0;
			}
		    writel(WDT_PRETIMEOUT_ENABLE | pretimeout, wdt->base + WDT_PRETIMEOUT_CTL_REG);
		    mask = WDT_PRETIMEOUT_INT_MASK;
		}
		writel(mask, wdt->base + WDT_INT_REG);
	} else {
		pr_emerg("\nWATCHDOG(pre-timeout): wrong status\n");
	}
	return IRQ_HANDLED;
}

#ifdef CONFIG_BCM3390_WDT_NMI
void bcm3390_wdt_nmi(struct pt_regs *regs)
{
	int cpu_cnt;
	nodemask_t mask;

	/*
	 * The chip HW ORs together all of the NMI signals in the chip (e.g.
	 * WD, external pin, ...) and drives this signal directly to the
	 * CPU core(s) FIQ input. Thus as long as a device drives its NMI
	 * output the FIQ to the CPU will be held active and we'll just
	 * come back here again (millions of times). We want to return
	 * so that any printk's can come out. Masking the FIQ in the
	 * SPSR_<mode prior to exception> register still results in coming
	 * back here 2x for each core because they are all signalled and as
	 * soon as we switch to SVC mode (where FIQ gets unmasked) another FIQ
	 * gets triggered, so we must also disable FIQ locally.
	 */
	regs->ARM_cpsr |= PSR_F_BIT;
	local_fiq_disable();
	local_irq_disable();
	spin_lock(&nmi_lock); /* serialize output from each CPU */
	cpu_cnt = atomic_add_return(1, &nmi_cpu_cnt);
	if (cpu_cnt == 1) {
		/*
		 * Enable IRQs on 1 CPU to get NMI bufs to dump on all
		 * CPUs.
		 */
		local_irq_enable();
		console_verbose();
		bust_spinlocks(1);
		debug_locks_off();
		pr_emerg("\n******************** NMI ********************\n");
		pr_emerg("\n---[ CPU Avg Load ]---\n");
		bcm3390_wdt_loadavg_show();
		pr_emerg("\n---[ Show Memory ]---\n");
		nodes_setall(mask);
		show_mem(0, &mask);
		/* trigger FPM & DQM dump */
		atomic_notifier_call_chain(&panic_notifier_list, 0, "watchdog");
		pr_emerg("\n---[ Backtrace ]---\n");
	}
	/* dump regs & backtrace */
	show_regs(regs);
	if (cpu_cnt == num_online_cpus()) {
		pr_emerg("\n---[ Call Kmsg Dump ]---\n");
		printk_safe_flush();
		console_flush_on_panic(CONSOLE_FLUSH_PENDING);
		/* trigger persistent storage dump */
		kmsg_dump(KMSG_DUMP_PANIC);
		/* kick klogd */
		bust_spinlocks(0);
		/* Wait here for watchdog to initiate reboot */
		/* The wait time is Maximum Watchdog time the register
		   supports + default time (Grace time) of 30 seconds */
		msleep ((WDT_MAX_TIMEOUT + WDT_DEFAULT_TIMEOUT)*1000);
		do_kernel_restart("reboot");
	}
	spin_unlock(&nmi_lock);
}
#endif

static const struct watchdog_ops bcm3390_wdt_ops = {
	.owner		= THIS_MODULE,
	.start		= bcm3390_wdt_start,
	.stop		= bcm3390_wdt_stop,
	.set_timeout	= bcm3390_wdt_set_timeout,
	.get_timeleft	= bcm3390_wdt_get_timeleft,
};

static int bcm3390_wdt_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct bcm3390_watchdog *wdt;
	struct resource *res;
	int err, irq;
	const char *str;
	u32 mode = WDT_CTRL_MODE_RST;
	u32 timeout = WDT_DEFAULT_TIMEOUT;
	u32 t1, t2, tmp;
	unsigned long flags;
	bool timeout_config = false, mode_config = false;

	wdt = devm_kzalloc(dev, sizeof(*wdt), GFP_KERNEL);
	if (!wdt)
		return -ENOMEM;

	platform_set_drvdata(pdev, wdt);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	wdt->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(wdt->base))
		return PTR_ERR(wdt->base);

	wdt->clk = devm_clk_get(dev, NULL);
	/* If unable to get clock, use default frequency */
	if (!IS_ERR(wdt->clk)) {
		clk_prepare_enable(wdt->clk);
		wdt->rate = clk_get_rate(wdt->clk);
		/* Prevent divide-by-zero exception */
		if (!wdt->rate)
			wdt->rate = WDT_DEFAULT_RATE;
	} else {
		wdt->rate = WDT_DEFAULT_RATE;
		wdt->clk = NULL;
	}

	/* Is this WDT running? */
	t1 = readl(wdt->base + WDT_CMD_REG);
	udelay((1000000 / wdt->rate) + 1);
	t2 = readl(wdt->base + WDT_CMD_REG);
	wdt_running = (t2 < t1);

    if (of_property_read_bool(np, "start"))
		wdt->start = true;

	err = of_property_read_u32(np, "timeout-ms", &timeout);
	if (!err)
    	timeout_config = true;

    err = of_property_read_string(np, "mode", &str);
	if (!err) {
		mode_config = true;
		if (!strcmp(str, "reset"))
			mode = WDT_CTRL_MODE_RST;
		else if (!strcmp(str, "nmi"))
			mode = WDT_CTRL_MODE_NMI;
		else if (!strcmp(str, "nmi-reset"))
			mode = WDT_CTRL_MODE_NMI_RST;
		else
			pr_err("Invalid mode specified. Using reset mode.\n");
	}
	else
		mode = readl(wdt->base + WDT_CTRL_REG) &
			WDT_CTRL_MODE_VAL_MASK;

	irq = platform_get_irq(pdev, 0);
	/* Only use IRQ if we're not using NMI */
	if ((irq > 0) && (mode != WDT_CTRL_MODE_NMI) && 
	    (mode != WDT_CTRL_MODE_NMI_RST)) {
		if (!devm_request_irq(&pdev->dev, irq, bcm3390_wdt_isr, 0,
				      dev_name(&pdev->dev), wdt)) {
#ifdef BCM_WDT_TASKLET
			tasklet_init(&kmsg_task, bcm3390_wdt_kmsg_work,
				     (unsigned long) wdt);
#else
			INIT_WORK(&kmsg_work, bcm3390_wdt_kmsg_work);
#endif
		}
	}

	wdt->wdd.info		= &bcm3390_wdt_info;
	wdt->wdd.ops		= &bcm3390_wdt_ops;
	wdt->wdd.min_timeout	= WDT_MIN_TIMEOUT;
	wdt->wdd.timeout	= timeout;
	wdt->wdd.max_timeout	= 0xffffffff / wdt->rate;
	wdt->wdd.parent		= dev;
	wdt->ping_min		= 0xFFFFFFFF;
	watchdog_set_drvdata(&wdt->wdd, wdt);
	err = watchdog_register_device(&wdt->wdd);
	if (err) {
		dev_err(dev, "Failed to register watchdog device\n");
		clk_disable_unprepare(wdt->clk);
		return err;
	}

	/* If already running then stop it and configure
	   the required settings and restart it */
	if (timeout_config || mode_config) {
		if (wdt_running) {
			err = bcm3390_wdt_stop(&wdt->wdd);
			if (err) {
				dev_err(dev, "Failed to stop watchdog\n");
				clk_disable_unprepare(wdt->clk);
				return err;
			}
			wdt->start = true;
		}
		if (mode_config) {
			dev_dbg(dev, "Configure mode : %u\n", mode);
			tmp = readl(wdt->base + WDT_CTRL_REG);
			tmp &= ~WDT_CTRL_MODE_VAL_MASK;
			__raw_writel((tmp | (mode & WDT_CTRL_MODE_VAL_MASK)),
				wdt->base + WDT_CTRL_REG);
		}
	}

	/* If stop property is set then stop the watchdog */
	if (of_property_read_bool(np, "stop")) {
		err = bcm3390_wdt_stop(&wdt->wdd);
		if (err) {
			dev_err(dev, "Failed to stop watchdog\n");
			clk_disable_unprepare(wdt->clk);
			return err;
		}
		dev_dbg(dev, "Stoped watchdog as per settings in dt\n");
	}

	spin_lock_irqsave(&wdts_lock, flags);
	list_add(&wdt->list, &wdts);
	spin_unlock_irqrestore(&wdts_lock, flags);

	dev_info(dev, "Registered bcm3390 watchdog\n");

	return 0;
}

static int bcm3390_wdt_remove(struct platform_device *pdev)
{
	struct bcm3390_watchdog *wdt = platform_get_drvdata(pdev);
	unsigned long flags;

	if (!nowayout)
		bcm3390_wdt_stop(&wdt->wdd);
	spin_lock_irqsave(&wdts_lock, flags);
	list_del(&wdt->list);
	spin_unlock_irqrestore(&wdts_lock, flags);
	watchdog_unregister_device(&wdt->wdd);
	clk_disable_unprepare(wdt->clk);
#ifdef BCM_WDT_TASKLET
	tasklet_kill(&kmsg_task);
#else
	flush_work(&kmsg_work);
#endif

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int bcm3390_wdt_suspend(struct device *dev)
{
	struct bcm3390_watchdog *wdt = dev_get_drvdata(dev);

	if (watchdog_active(&wdt->wdd))
		return bcm3390_wdt_stop(&wdt->wdd);

	return 0;
}

static int bcm3390_wdt_resume(struct device *dev)
{
	struct bcm3390_watchdog *wdt = dev_get_drvdata(dev);

	if (watchdog_active(&wdt->wdd))
		return bcm3390_wdt_start(&wdt->wdd);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(bcm3390_wdt_pm_ops, bcm3390_wdt_suspend,
			 bcm3390_wdt_resume);

static void bcm3390_wdt_shutdown(struct platform_device *pdev)
{
	struct bcm3390_watchdog *wdt = platform_get_drvdata(pdev);

	if (watchdog_active(&wdt->wdd) && !nowayout)
		bcm3390_wdt_stop(&wdt->wdd);
}

static const struct of_device_id bcm3390_wdt_match[] = {
	{ .compatible = "brcm,bcm3390-wdt" },
	{},
};

static struct platform_driver bcm3390_wdt_driver = {
	.probe		= bcm3390_wdt_probe,
	.remove		= bcm3390_wdt_remove,
	.shutdown	= bcm3390_wdt_shutdown,
	.driver		= {
		.name		= "bcm3390-wdt",
		.of_match_table	= bcm3390_wdt_match,
		.pm		= &bcm3390_wdt_pm_ops,
	}
};

static int __init bcm3390_wdt_init(void)
{
	int err;
	struct bcm3390_watchdog *wdt;
	unsigned long flags;

#ifdef CONFIG_BCM3390_WDT_NMI
	spin_lock_init(&nmi_lock);
#endif
	spin_lock_init(&wdts_lock);
	err = platform_driver_register(&bcm3390_wdt_driver);
	if (err)
		return err;
	if (wdt_running) {
		spin_lock_irqsave(&wdts_lock, flags);
		list_for_each_entry(wdt, &wdts, list) {
			if (wdt->start) {
				err = bcm3390_wdt_start(&wdt->wdd);
				if (err)
					break;
			}
		}
		spin_unlock_irqrestore(&wdts_lock, flags);
	}

	return err;
}
module_init(bcm3390_wdt_init);

static void __exit bcm3390_wdt_exit(void)
{
	platform_driver_unregister(&bcm3390_wdt_driver);
}
module_exit(bcm3390_wdt_exit);

module_param(nowayout, bool, 0644);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default="
	__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Driver for Broadcom 3390 CM SoC Watchdog");
MODULE_AUTHOR("Justin Chen, Jayesh Patel, Tim Ross");
#endif
