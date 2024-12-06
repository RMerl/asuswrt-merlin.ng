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

/*
 * This is hack version of the original bcm63xx timer driver using external
 * periph timer for 6766 only to replace the non functional default arch
 * timer. This driver use all four periph timers for per-cpu clockevent for
 * high resolution timer support and the arch timer physical counter for
 * clocksource and schedule clock
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/cpu.h>
#include <linux/percpu.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/cpumask.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/sched_clock.h>
#include <bcm_ext_timer.h>

int ext_timer_early_init(struct device_node *np);

int timer_cs_used = -1;			/* for clock source */
DEFINE_PER_CPU(int, timer_ce_used) = -1;/* for clock event */
DEFINE_PER_CPU(struct clock_event_device, clockevent_timer);

#define ARCH_TIMER_FREQ                80*1000*1000
#define PERIPH_TIMER_CNT_MAX           ((0x1 << 30) - 1)

uint32_t perclk_freq = 50000;	/* in KHz, value is 50MHz */
/* the below timer value will convert into the larger timercount supported
 * in PERIPH_TIMER: 30-bit PERIPH_TIMER at 50MHz support up 0x40000000/50
 * = 21474836us */
#define PERIPH_TIMER_PERIOD_MAX	(PERIPH_IMER_CNT_MAX/(perclk_freq/1000))

static u64 arch_timer_read_phy_counter(void)
{
	u64 count;

	/* v7 only for 6766 */
	asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (count));

	return count;
}


static u64 arch_timer_clocksource_read(struct clocksource *cs)
{
	return arch_timer_read_phy_counter();
}

static struct clocksource arch_timer_clocksource = {
	.name = "timer_cs",
	.rating = 350,
	.read = arch_timer_clocksource_read,
	.mask = CLOCKSOURCE_MASK(56),
	.flags = CLOCK_SOURCE_IS_CONTINUOUS,
};

static void __init arch_timer_clocksource_init(void)
{
	clocksource_register_hz(&arch_timer_clocksource, ARCH_TIMER_FREQ);

	sched_clock_register(arch_timer_read_phy_counter, 56, ARCH_TIMER_FREQ);
}


static int  timer_set_shutdown( struct clock_event_device *clk)
{
	int *timer_ce = this_cpu_ptr(&timer_ce_used);
	ext_timer_stop(*timer_ce);
	return 0;
}

static int timer_set_periodic(struct clock_event_device *evt)
{
	int *timer_ce = this_cpu_ptr(&timer_ce_used);


	if (*timer_ce == -1)
		return -ENODEV;

	ext_timer_stop(*timer_ce);

	/* set up timer based on HZ given, unit is microseconds */
	ext_timer_set_period(*timer_ce, 1000000/HZ);

	ext_timer_set_mode(*timer_ce, EXT_TIMER_MODE_PERIODIC);

	ext_timer_start(*timer_ce);

	return 0;

}

static int timer_set_next_event(unsigned long cycle,
		struct clock_event_device *unused)
{
	int *timer_ce = this_cpu_ptr(&timer_ce_used);

	if (*timer_ce == -1)
		return -ENODEV;

	/* stop the timer will clear the residual counter */
	ext_timer_stop(*timer_ce);

	ext_timer_set_count(*timer_ce, cycle);

	ext_timer_start(*timer_ce);

	return 0;
}

static void clock_event_callback(unsigned int param)
{
	struct clock_event_device *evt = (struct clock_event_device *)param;
	evt->event_handler(evt);
}

static int timer_starting_cpu(unsigned int cpu)
{
	int *timer_ce = this_cpu_ptr(&timer_ce_used);
	struct clock_event_device *evt = this_cpu_ptr(&clockevent_timer);
	int ret = -1;

	if (*timer_ce == -1)
	{
		*timer_ce = ext_timer_alloc_only(-1,
			(ExtTimerHandler)&clock_event_callback,
			(unsigned int)evt);

		/* cannot allocate timer, just quit.  Shouldn't happen! */
		if (*timer_ce == -1) {
			pr_err("Failed to allocated ext timer for cpu %d\n", cpu);
			return -1;
		}
	}

	/* in cpu1 case, cpu1 is not fully up yet at this port, set force to true
	   to force irq affinity to this cpu */
	ret = ext_timer_set_affinity(*timer_ce, cpu, true);

	evt->name = "timer_ce";
	evt->features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT |
		CLOCK_EVT_FEAT_PERCPU;
	evt->set_state_shutdown = timer_set_shutdown,
	evt->set_state_oneshot = timer_set_shutdown,
	evt->set_next_event = timer_set_next_event,
	evt->set_state_periodic = timer_set_periodic,
	evt->rating = 250,
	evt->cpumask = cpumask_of(cpu);


	timer_set_shutdown(evt);

	/* clockevents_config_and_register(dev, freq, min_delta, max_delta)
	 * freq is in the unit of Hz
	 * min_delta: minimum clock tick to program in oneshot mode
	 * max_delta: maximum clock tick to program in oneshot mode */
	clockevents_config_and_register(evt,
			perclk_freq * 1000, 0xf, 0x3fffffff);

	return 0;
}

static int timer_dying_cpu(unsigned int cpu)
{
	struct clock_event_device *evt = this_cpu_ptr(&clockevent_timer);

	timer_set_shutdown(evt);

	return 0;
}

static int __init bcm63xx_timer_of_register(struct device_node *np)
{
	struct clk* clk;
	int err = 0;

	err = ext_timer_early_init(np);
	if( err != 0 ) {
		pr_err("bcm63xx_timer_of_register failed to initialize HW timers %d\n",
			err);
		return err;
	}

	clk = of_clk_get(np, 0);
	if( !IS_ERR_OR_NULL(clk) ) {
		clk_prepare_enable(clk);
		perclk_freq = clk_get_rate(clk)/1000;
	} else
		pr_err("failed to get clk %p, use default perclk rate %d\n",
			clk, perclk_freq);

	/* Register and immediately configure the timer on the boot CPU */
	err = cpuhp_setup_state(CPUHP_AP_ARM_ARCH_TIMER_STARTING,
		"clockevents/bcm63xx_timer:starting",
		timer_starting_cpu, timer_dying_cpu);

	if(!err)
	{
		/* Immediately configure the timer on the boot CPU */
		arch_timer_clocksource_init();
	}
	else
		printk("bcm63xx_timer_init: cpuhp_setup_state failed %d\n", err);

	return err;
}

TIMER_OF_DECLARE(bcm63xx, "brcm,bcm-timers", bcm63xx_timer_of_register);
