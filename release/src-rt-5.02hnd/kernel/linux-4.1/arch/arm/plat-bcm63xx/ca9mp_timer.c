#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>

#include <asm/sched_clock.h>
#include <asm/localtimer.h>
#include <asm/smp_twd.h>

#include <plat/ca9mpcore.h>
#include <bcm_map_part.h>

/*
 * The ARM9 MPCORE Global Timer is a continously-running 64-bit timer,
 * which is used both as a "clock source" and as a "clock event" -
 * there is a banked per-cpu compare and reload registers that are
 * used to generated either one-shot or periodic interrupts on the cpu
 * that calls the mode_set function.
 *
 * NOTE: This code does not support dynamic change of the source clock
 * frequency. The interrupt interval is only calculated once during
 * initialization.
 */

/*
 * Global Timer Registers
 */
#define	GTIMER_COUNT_LO		0x00	/* Lower 32 of 64 bits counter */
#define	GTIMER_COUNT_HI		0x04	/* Higher 32 of 64 bits counter */
#define	GTIMER_CTRL		0x08	/* Control (partially banked) */
#define	GTIMER_CTRL_EN		(1<<0)	/* Timer enable bit */
#define	GTIMER_CTRL_CMP_EN	(1<<1)	/* Comparator enable */
#define	GTIMER_CTRL_IRQ_EN	(1<<2)	/* Interrupt enable */
#define	GTIMER_CTRL_AUTO_EN	(1<<3)	/* Auto-increment enable */
#define	GTIMER_INT_STAT		0x0C	/* Interrupt Status (banked) */
#define	GTIMER_COMP_LO		0x10	/* Lower half comparator (banked) */
#define	GTIMER_COMP_HI		0x14	/* Upper half comparator (banked) */
#define	GTIMER_RELOAD		0x18	/* Auto-increment (banked) */

#define	GTIMER_MIN_RANGE	30	/* Minimum wrap-around time in sec */

#define GTIMER_VIRT_ADDR	(IO_ADDRESS(SCU_PHYS_BASE) + CA9MP_GTIMER_OFF)
#define LTIMER_PHY_ADDR		(SCU_PHYS_BASE + CA9MP_LTIMER_OFF)

/* Gobal variables */
static u32 ticks_per_jiffy;

static cycle_t gptimer_count_read(struct clocksource *cs)
{
	u32 count_hi, count_ho, count_lo;
	u64 count;

	/* Avoid unexpected rollover with double-read of upper half */
	do {
		count_hi = readl_relaxed(GTIMER_VIRT_ADDR + GTIMER_COUNT_HI);
		count_lo = readl_relaxed(GTIMER_VIRT_ADDR + GTIMER_COUNT_LO);
		count_ho = readl_relaxed(GTIMER_VIRT_ADDR + GTIMER_COUNT_HI);
	} while (count_hi != count_ho);

	count = (u64)count_hi << 32 | count_lo;
	return count;
}

static struct clocksource clocksource_gptimer = {
	.name		= "ca9mp_gtimer",
	.rating		= 300,
	.read		= gptimer_count_read,
	.mask		= CLOCKSOURCE_MASK(64),
//	.shift		= 20,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static notrace u32 brcm_sched_clock_read(void)
{
	return clocksource_gptimer.read(&clocksource_gptimer);
}

/*
 * IRQ handler for the global timer
 * This interrupt is banked per CPU so is handled identically
 */
static irqreturn_t gtimer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = *(struct clock_event_device **)dev_id;

	if (evt->mode == CLOCK_EVT_MODE_ONESHOT) {
		u32 ctrl = readl_relaxed(GTIMER_VIRT_ADDR + GTIMER_CTRL);
		ctrl &= ~GTIMER_CTRL_EN;
		writel_relaxed(ctrl, GTIMER_VIRT_ADDR + GTIMER_CTRL);
	}
	/* clear the interrupt */
	writel_relaxed(1, GTIMER_VIRT_ADDR + GTIMER_INT_STAT);

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static void gtimer_set_mode(enum clock_event_mode mode,
		struct clock_event_device *evt)
{
	u32 ctrl = 0, period;
	u64 count;

	/* By default, when we enter this function, we can just stop
	 * the timer completely, once a mode is selected, then we
	 * can start the timer at that point. */

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		period = ticks_per_jiffy;
		count = gptimer_count_read(NULL);
		count += period;
		writel_relaxed(ctrl, GTIMER_VIRT_ADDR + GTIMER_CTRL);
		writel_relaxed(count & 0xffffffffUL, GTIMER_VIRT_ADDR + GTIMER_COMP_LO);
		writel_relaxed(count >> 32, GTIMER_VIRT_ADDR + GTIMER_COMP_HI);
		writel_relaxed(period, GTIMER_VIRT_ADDR + GTIMER_RELOAD);
		ctrl = GTIMER_CTRL_EN | GTIMER_CTRL_CMP_EN |
				GTIMER_CTRL_IRQ_EN | GTIMER_CTRL_AUTO_EN;
		break;

	case CLOCK_EVT_MODE_ONESHOT:
		/* period set, and timer enabled in 'next_event' hook */
		break;

	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		break;
	}
	/* Apply the new mode */
	writel_relaxed(ctrl, GTIMER_VIRT_ADDR + GTIMER_CTRL);
}

static int gtimer_set_next_event(unsigned long next,
		struct clock_event_device *evt)
{
	u32 ctrl = readl_relaxed(GTIMER_VIRT_ADDR + GTIMER_CTRL);
	u64 count = gptimer_count_read(NULL);

	ctrl &= ~GTIMER_CTRL_CMP_EN;
	writel_relaxed(ctrl, GTIMER_VIRT_ADDR + GTIMER_CTRL);

	count += next;

	writel_relaxed(count & 0xffffffffUL, GTIMER_VIRT_ADDR + GTIMER_COMP_LO);
	writel_relaxed(count >> 32, GTIMER_VIRT_ADDR + GTIMER_COMP_HI);

	/* enable IRQ for the same cpu that loaded comparator */
	ctrl |= GTIMER_CTRL_EN | GTIMER_CTRL_CMP_EN | GTIMER_CTRL_IRQ_EN;

	writel_relaxed(ctrl, GTIMER_VIRT_ADDR + GTIMER_CTRL);

	return 0;
}

static struct clock_event_device gtimer_clockevent = {
	.name		= "ca9mp_gtimer",
	.shift		= 20,
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= gtimer_set_mode,
	.set_next_event	= gtimer_set_next_event,
	.rating		= 300,
};

static union {
	struct clock_event_device *evt;
	struct clock_event_device __percpu **percpu_evt;
} brcm_evt;


static void __init gtimer_clockevents_init(u32 rate)
{
	struct clock_event_device *evt = &gtimer_clockevent;
	int res;

	evt->irq = CA9MP_IRQ_GLOBALTIMER;
	evt->cpumask = cpumask_of(0);

#ifdef CONFIG_BCM63138_SIM
        ticks_per_jiffy = DIV_ROUND_CLOSEST(rate, HZ) / 20;
#else
        ticks_per_jiffy = DIV_ROUND_CLOSEST(rate, HZ);
#endif

        clockevents_calc_mult_shift(evt, rate, GTIMER_MIN_RANGE);

	evt->max_delta_ns = clockevent_delta2ns(0xffffffff, evt);
	evt->min_delta_ns = clockevent_delta2ns(0xf, evt);

	/* Register the device to install handler before enabing IRQ */
	clockevents_register_device(evt);

	brcm_evt.percpu_evt = alloc_percpu(struct clock_event_device *);
	if (!brcm_evt.percpu_evt) {
		pr_err("alloc_percpu failed for %s\n", evt->name);
	}
	*__this_cpu_ptr(brcm_evt.percpu_evt) = evt;
	res = request_percpu_irq(evt->irq, gtimer_interrupt, evt->name, 
			brcm_evt.percpu_evt);
	if (!res) {
		pr_err("request_percpu_irq succeeds for %s\n", evt->name);
		enable_percpu_irq(evt->irq, 0);
	} else
		pr_err("request_percpu_irq fails! for %s\n", evt->name);
}

static void inline gtimer_clockevents_updatefreq_hz(u32 rate)
{
	struct clock_event_device *evt = &gtimer_clockevent;

	/* there is an API called clockevents_update_freq which does
	 * almost identical task as what we do here */
#ifdef CONFIG_BCM63138_SIM
        ticks_per_jiffy = DIV_ROUND_CLOSEST(rate, HZ) / 20;
#else
        ticks_per_jiffy = DIV_ROUND_CLOSEST(rate, HZ);
#endif

        clockevents_calc_mult_shift(evt, rate, GTIMER_MIN_RANGE);

	evt->max_delta_ns = clockevent_delta2ns(0xffffffff, evt);
	evt->min_delta_ns = clockevent_delta2ns(0xf, evt);
}

/*
 * MPCORE Global Timer initialization function
 */
static void __init ca9mp_gtimer_init(unsigned long rate)
{
	u64 count;
	int res;

	printk(KERN_INFO "MPCORE Global Timer Clock %luHz\n", rate);

	/* Register as system timer */
	gtimer_clockevents_init(rate);

	/* Self-test the timer is running */
	count = gptimer_count_read(NULL);

	/* Register as time source */
	res = clocksource_register_hz(&clocksource_gptimer, rate);
	if (res)
		printk("%s:clocksource_register failed!\n", __func__);
	setup_sched_clock(brcm_sched_clock_read, 32, rate);

	count = gptimer_count_read(NULL) - count;
	if (count == 0)
		printk(KERN_CRIT "MPCORE Global Timer Dead!!\n");
}

#ifdef CONFIG_HAVE_ARM_TWD
static DEFINE_TWD_LOCAL_TIMER(twd_local_timer, LTIMER_PHY_ADDR,
		CA9MP_IRQ_LOCALTIMER);

static void __init ca9mp_twd_init(void)
{
	int err = twd_local_timer_register(&twd_local_timer);
	if (err)
		pr_err("twd_local_timer_register failed %d\n", err);
}
#else
#define ca9mp_twd_init()	do {} while(0)
#endif

void ca9mp_timer_update_freq(unsigned long rate)
{
	printk(KERN_INFO "MPCORE Global Timer Clock update to %luHz\n", rate);

	gtimer_clockevents_updatefreq_hz(rate);

	__clocksource_updatefreq_hz(&clocksource_gptimer, rate);
}

void __init ca9mp_timer_init(unsigned long rate)
{
	/* init global timer */
	ca9mp_gtimer_init(rate);

	/* init TWD / local timer */
	ca9mp_twd_init();

}

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
