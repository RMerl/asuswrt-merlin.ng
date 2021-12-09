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

/*
 * BCM63xx SoC timer implementation based on external PERIPH Timer
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/cpu.h>
#include <linux/percpu.h>
#include <linux/cpumask.h>
#include <linux/sched_clock.h>

#include <plat/bcm63xx_timer.h>
#include <bcm_ext_timer.h>

/*
 * timer implementations for clocksource and clockevent
 * We will use 2 PERIPH timers, one for clocksource and one for
 * clockevent.
 */
int timer_cs_used = -1;			/* for clock source */
DEFINE_PER_CPU(int, timer_ce_used) = -1;/* for clock event */
DEFINE_PER_CPU(struct clock_event_device, clockevent_timer);

#define PERIPH_TIMER_CLK_FREQ	50000	/* in KHz, value is 50MHz */
/* the below timer value will convert into the larger timercount supported
 * in PERIPH_TIMER: 30-bit PERIPH_TIMER at 50MHz support up 0x40000000/50
 * = 21474836us */
#define PERIPH_TIMER_PERIOD_MAX	(TIMER_CNT_MAX/(PERIPH_TIMER_CLK_FREQ/1000)) 


static notrace cycle_t bcm63xx_read_timer_count(struct clocksource *cs)
{
	int ret;
	uint64_t count; /* cycle_t is 64 bit integer */

	if (timer_cs_used == -1)
		return 0;

	ret = ext_timer_read_count(timer_cs_used, &count);
	if (ret == 0)
		return (cycle_t)count;
	else
		return 0;
}

notrace u64 bcm63xx_sched_clock_read(void)
{
	int ret;
	uint64_t count;

	if (timer_cs_used == -1)
		return 0;

	ret = ext_timer_read_count(timer_cs_used, &count);
	if (ret == 0)
		return count;
	else
		return 0;
}

static struct clocksource bcm63xx_clocksource = {
	.name = "timer_cs",
	.rating = 350,
	.read = bcm63xx_read_timer_count,
	.mask = CLOCKSOURCE_MASK(30), 
	.flags = CLOCK_SOURCE_IS_CONTINUOUS,

};

static void __init periph_timer_clocksource_init(void)
{

	if (timer_cs_used != -1)
		return;

	timer_cs_used = ext_timer_alloc(-1, PERIPH_TIMER_PERIOD_MAX, NULL, 0);

	/* cannot allocate timer, just quit.  Shouldn't happen! */
	if (timer_cs_used == -1)
		return;

	ext_timer_start(timer_cs_used);

	/* bcm63xx_clocksource->shift/mult will be computed by the following
	 * register function */
	clocksource_register_khz(&bcm63xx_clocksource, PERIPH_TIMER_CLK_FREQ);

        /* register with sched clock */
	sched_clock_register(bcm63xx_sched_clock_read, 30, PERIPH_TIMER_CLK_FREQ*1000);
}

static void timer_set_mode(enum clock_event_mode mode,
		struct clock_event_device *clk)
{
	int *timer_ce = this_cpu_ptr(&timer_ce_used);

	if (*timer_ce == -1)
		return;

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		ext_timer_stop(*timer_ce);

		/* set up timer based on HZ given, unit is microseconds */
		ext_timer_set_period(*timer_ce, 1000000/HZ);

		ext_timer_set_mode(*timer_ce, EXT_TIMER_MODE_PERIODIC);

		ext_timer_start(*timer_ce);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		/* timer is set and enabled in 'set_next_event' hook */
		break;
	case CLOCK_EVT_MODE_RESUME:
		ext_timer_start(*timer_ce);
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		ext_timer_stop(*timer_ce);
	default:
		break;
	}
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

	ext_timer_set_mode(*timer_ce, EXT_TIMER_MODE_ONESHOT);

	ext_timer_start(*timer_ce);

	return 0;
}

static void clock_event_callback(unsigned int param)
{
	struct clock_event_device *evt = (struct clock_event_device *)param;
	evt->event_handler(evt);
}

static void periph_timer_clockevent_init(void)
{
	int cpu = smp_processor_id();
	int *timer_ce = this_cpu_ptr(&timer_ce_used);
	struct clock_event_device *evt = this_cpu_ptr(&clockevent_timer);
	int ret = -1;

	if (*timer_ce == -1)
	{
		*timer_ce = ext_timer_alloc_only(-1,
			(ExtTimerHandler)&clock_event_callback,
			(unsigned int)evt);

		/* cannot allocate timer, just quit.  Shouldn't happen! */
		if (*timer_ce == -1)
			return;
	}

	/* in cpu1 case, cpu1 is not fully up yet at this port, set force to true 
	   to force irq affinity to this cpu */
	ret = ext_timer_set_affinity(*timer_ce, cpu, true);

	evt->name = "timer_ce";
	evt->features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT |
		CLOCK_EVT_FEAT_PERCPU;
	evt->set_mode = timer_set_mode,
	evt->set_next_event = timer_set_next_event,
	evt->rating = 250,
	evt->cpumask = cpumask_of(cpu);

	/* clockevents_config_and_register(dev, freq, min_delta, max_delta)
	 * freq is in the unit of Hz
	 * min_delta: minimum clock tick to program in oneshot mode
	 * max_delta: maximum clock tick to program in oneshot mode */
	clockevents_config_and_register(evt,
			PERIPH_TIMER_CLK_FREQ * 1000, 0, 0x3fffffff);
}

static void periph_timer_clockevent_stop(void)
{
	struct clock_event_device *evt = this_cpu_ptr(&clockevent_timer);

	timer_set_mode(CLOCK_EVT_MODE_UNUSED, evt);
}

static int periph_timer_cpu_notify(struct notifier_block *self, unsigned long action,
			 void *hcpu)
{
	switch (action & ~CPU_TASKS_FROZEN) {
	case CPU_STARTING:
		periph_timer_clockevent_init();
		break;
	case CPU_DYING:
		periph_timer_clockevent_stop();
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block periph_timer_cpu_nb = {
	.notifier_call = periph_timer_cpu_notify,
};

void __init bcm63xx_timer_init(void)
{
	init_hw_timers();

	register_cpu_notifier(&periph_timer_cpu_nb);
	
	/* Immediately configure the timer on the boot CPU */
	periph_timer_clocksource_init();
	periph_timer_clockevent_init();
}

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
