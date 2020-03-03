/*
 * Based on arm clockevents implementation and old bfin time tick.
 *
 * Copyright 2008-2009 Analog Devics Inc.
 *                2008 GeoTechnologies
 *                     Vitja Makarov
 *
 * Licensed under the GPL-2
 */

#include <linux/module.h>
#include <linux/profile.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/irq.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/cpufreq.h>

#include <asm/blackfin.h>
#include <asm/time.h>
#include <asm/gptimers.h>
#include <asm/nmi.h>


#if defined(CONFIG_CYCLES_CLOCKSOURCE)

static notrace cycle_t bfin_read_cycles(struct clocksource *cs)
{
#ifdef CONFIG_CPU_FREQ
	return __bfin_cycles_off + (get_cycles() << __bfin_cycles_mod);
#else
	return get_cycles();
#endif
}

static struct clocksource bfin_cs_cycles = {
	.name		= "bfin_cs_cycles",
	.rating		= 400,
	.read		= bfin_read_cycles,
	.mask		= CLOCKSOURCE_MASK(64),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static inline unsigned long long bfin_cs_cycles_sched_clock(void)
{
	return clocksource_cyc2ns(bfin_read_cycles(&bfin_cs_cycles),
		bfin_cs_cycles.mult, bfin_cs_cycles.shift);
}

static int __init bfin_cs_cycles_init(void)
{
	if (clocksource_register_hz(&bfin_cs_cycles, get_cclk()))
		panic("failed to register clocksource");

	return 0;
}
#else
# define bfin_cs_cycles_init()
#endif

#ifdef CONFIG_GPTMR0_CLOCKSOURCE

void __init setup_gptimer0(void)
{
	disable_gptimers(TIMER0bit);

#ifdef CONFIG_BF60x
	bfin_write16(TIMER_DATA_IMSK, 0);
	set_gptimer_config(TIMER0_id,  TIMER_OUT_DIS
		| TIMER_MODE_PWM_CONT | TIMER_PULSE_HI | TIMER_IRQ_PER);
#else
	set_gptimer_config(TIMER0_id, \
		TIMER_OUT_DIS | TIMER_PERIOD_CNT | TIMER_MODE_PWM);
#endif
	set_gptimer_period(TIMER0_id, -1);
	set_gptimer_pwidth(TIMER0_id, -2);
	SSYNC();
	enable_gptimers(TIMER0bit);
}

static cycle_t bfin_read_gptimer0(struct clocksource *cs)
{
	return bfin_read_TIMER0_COUNTER();
}

static struct clocksource bfin_cs_gptimer0 = {
	.name		= "bfin_cs_gptimer0",
	.rating		= 350,
	.read		= bfin_read_gptimer0,
	.mask		= CLOCKSOURCE_MASK(32),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static inline unsigned long long bfin_cs_gptimer0_sched_clock(void)
{
	return clocksource_cyc2ns(bfin_read_TIMER0_COUNTER(),
		bfin_cs_gptimer0.mult, bfin_cs_gptimer0.shift);
}

static int __init bfin_cs_gptimer0_init(void)
{
	setup_gptimer0();

	if (clocksource_register_hz(&bfin_cs_gptimer0, get_sclk()))
		panic("failed to register clocksource");

	return 0;
}
#else
# define bfin_cs_gptimer0_init()
#endif

#if defined(CONFIG_GPTMR0_CLOCKSOURCE) || defined(CONFIG_CYCLES_CLOCKSOURCE)
/* prefer to use cycles since it has higher rating */
notrace unsigned long long sched_clock(void)
{
#if defined(CONFIG_CYCLES_CLOCKSOURCE)
	return bfin_cs_cycles_sched_clock();
#else
	return bfin_cs_gptimer0_sched_clock();
#endif
}
#endif

#if defined(CONFIG_TICKSOURCE_GPTMR0)
static int bfin_gptmr0_set_next_event(unsigned long cycles,
                                     struct clock_event_device *evt)
{
	disable_gptimers(TIMER0bit);

	/* it starts counting three SCLK cycles after the TIMENx bit is set */
	set_gptimer_pwidth(TIMER0_id, cycles - 3);
	enable_gptimers(TIMER0bit);
	return 0;
}

static void bfin_gptmr0_set_mode(enum clock_event_mode mode,
				struct clock_event_device *evt)
{
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC: {
#ifndef CONFIG_BF60x
		set_gptimer_config(TIMER0_id, \
			TIMER_OUT_DIS | TIMER_IRQ_ENA | \
			TIMER_PERIOD_CNT | TIMER_MODE_PWM);
#else
		set_gptimer_config(TIMER0_id,  TIMER_OUT_DIS
			| TIMER_MODE_PWM_CONT | TIMER_PULSE_HI | TIMER_IRQ_PER);
#endif

		set_gptimer_period(TIMER0_id, get_sclk() / HZ);
		set_gptimer_pwidth(TIMER0_id, get_sclk() / HZ - 1);
		enable_gptimers(TIMER0bit);
		break;
	}
	case CLOCK_EVT_MODE_ONESHOT:
		disable_gptimers(TIMER0bit);
#ifndef CONFIG_BF60x
		set_gptimer_config(TIMER0_id, \
			TIMER_OUT_DIS | TIMER_IRQ_ENA | TIMER_MODE_PWM);
#else
		set_gptimer_config(TIMER0_id, TIMER_OUT_DIS | TIMER_MODE_PWM
			| TIMER_PULSE_HI | TIMER_IRQ_WID_DLY);
#endif

		set_gptimer_period(TIMER0_id, 0);
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		disable_gptimers(TIMER0bit);
		break;
	case CLOCK_EVT_MODE_RESUME:
		break;
	}
}

static void bfin_gptmr0_ack(void)
{
	clear_gptimer_intr(TIMER0_id);
}

static void __init bfin_gptmr0_init(void)
{
	disable_gptimers(TIMER0bit);
}

#ifdef CONFIG_CORE_TIMER_IRQ_L1
__attribute__((l1_text))
#endif
irqreturn_t bfin_gptmr0_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = dev_id;
	smp_mb();
	/*
	 * We want to ACK before we handle so that we can handle smaller timer
	 * intervals.  This way if the timer expires again while we're handling
	 * things, we're more likely to see that 2nd int rather than swallowing
	 * it by ACKing the int at the end of this handler.
	 */
	bfin_gptmr0_ack();
	evt->event_handler(evt);
	return IRQ_HANDLED;
}

static struct irqaction gptmr0_irq = {
	.name		= "Blackfin GPTimer0",
	.flags		= IRQF_TIMER | IRQF_IRQPOLL | IRQF_PERCPU,
	.handler	= bfin_gptmr0_interrupt,
};

static struct clock_event_device clockevent_gptmr0 = {
	.name		= "bfin_gptimer0",
	.rating		= 300,
	.irq		= IRQ_TIMER0,
	.shift		= 32,
	.features 	= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_next_event = bfin_gptmr0_set_next_event,
	.set_mode	= bfin_gptmr0_set_mode,
};

static void __init bfin_gptmr0_clockevent_init(struct clock_event_device *evt)
{
	unsigned long clock_tick;

	clock_tick = get_sclk();
	evt->mult = div_sc(clock_tick, NSEC_PER_SEC, evt->shift);
	evt->max_delta_ns = clockevent_delta2ns(-1, evt);
	evt->min_delta_ns = clockevent_delta2ns(100, evt);

	evt->cpumask = cpumask_of(0);

	clockevents_register_device(evt);
}
#endif /* CONFIG_TICKSOURCE_GPTMR0 */

#if defined(CONFIG_TICKSOURCE_CORETMR)
/* per-cpu local core timer */
DEFINE_PER_CPU(struct clock_event_device, coretmr_events);

static int bfin_coretmr_set_next_event(unsigned long cycles,
				struct clock_event_device *evt)
{
	bfin_write_TCNTL(TMPWR);
	CSYNC();
	bfin_write_TCOUNT(cycles);
	CSYNC();
	bfin_write_TCNTL(TMPWR | TMREN);
	return 0;
}

static void bfin_coretmr_set_mode(enum clock_event_mode mode,
				struct clock_event_device *evt)
{
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC: {
		unsigned long tcount = ((get_cclk() / (HZ * TIME_SCALE)) - 1);
		bfin_write_TCNTL(TMPWR);
		CSYNC();
		bfin_write_TSCALE(TIME_SCALE - 1);
		bfin_write_TPERIOD(tcount);
		bfin_write_TCOUNT(tcount);
		CSYNC();
		bfin_write_TCNTL(TMPWR | TMREN | TAUTORLD);
		break;
	}
	case CLOCK_EVT_MODE_ONESHOT:
		bfin_write_TCNTL(TMPWR);
		CSYNC();
		bfin_write_TSCALE(TIME_SCALE - 1);
		bfin_write_TPERIOD(0);
		bfin_write_TCOUNT(0);
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		bfin_write_TCNTL(0);
		CSYNC();
		break;
	case CLOCK_EVT_MODE_RESUME:
		break;
	}
}

void bfin_coretmr_init(void)
{
	/* power up the timer, but don't enable it just yet */
	bfin_write_TCNTL(TMPWR);
	CSYNC();

	/* the TSCALE prescaler counter. */
	bfin_write_TSCALE(TIME_SCALE - 1);
	bfin_write_TPERIOD(0);
	bfin_write_TCOUNT(0);

	CSYNC();
}

#ifdef CONFIG_CORE_TIMER_IRQ_L1
__attribute__((l1_text))
#endif

irqreturn_t bfin_coretmr_interrupt(int irq, void *dev_id)
{
	int cpu = smp_processor_id();
	struct clock_event_device *evt = &per_cpu(coretmr_events, cpu);

	smp_mb();
	evt->event_handler(evt);

	touch_nmi_watchdog();

	return IRQ_HANDLED;
}

static struct irqaction coretmr_irq = {
	.name		= "Blackfin CoreTimer",
	.flags		= IRQF_TIMER | IRQF_IRQPOLL | IRQF_PERCPU,
	.handler	= bfin_coretmr_interrupt,
};

void bfin_coretmr_clockevent_init(void)
{
	unsigned long clock_tick;
	unsigned int cpu = smp_processor_id();
	struct clock_event_device *evt = &per_cpu(coretmr_events, cpu);

#ifdef CONFIG_SMP
	evt->broadcast = smp_timer_broadcast;
#endif

	evt->name = "bfin_core_timer";
	evt->rating = 350;
	evt->irq = -1;
	evt->shift = 32;
	evt->features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT;
	evt->set_next_event = bfin_coretmr_set_next_event;
	evt->set_mode = bfin_coretmr_set_mode;

	clock_tick = get_cclk() / TIME_SCALE;
	evt->mult = div_sc(clock_tick, NSEC_PER_SEC, evt->shift);
	evt->max_delta_ns = clockevent_delta2ns(-1, evt);
	evt->min_delta_ns = clockevent_delta2ns(100, evt);

	evt->cpumask = cpumask_of(cpu);

	clockevents_register_device(evt);
}
#endif /* CONFIG_TICKSOURCE_CORETMR */


void read_persistent_clock(struct timespec *ts)
{
	time_t secs_since_1970 = (365 * 37 + 9) * 24 * 60 * 60;	/* 1 Jan 2007 */
	ts->tv_sec = secs_since_1970;
	ts->tv_nsec = 0;
}

void __init time_init(void)
{

#ifdef CONFIG_RTC_DRV_BFIN
	/* [#2663] hack to filter junk RTC values that would cause
	 * userspace to have to deal with time values greater than
	 * 2^31 seconds (which uClibc cannot cope with yet)
	 */
	if ((bfin_read_RTC_STAT() & 0xC0000000) == 0xC0000000) {
		printk(KERN_NOTICE "bfin-rtc: invalid date; resetting\n");
		bfin_write_RTC_STAT(0);
	}
#endif

	bfin_cs_cycles_init();
	bfin_cs_gptimer0_init();

#if defined(CONFIG_TICKSOURCE_CORETMR)
	bfin_coretmr_init();
	setup_irq(IRQ_CORETMR, &coretmr_irq);
	bfin_coretmr_clockevent_init();
#endif

#if defined(CONFIG_TICKSOURCE_GPTMR0)
	bfin_gptmr0_init();
	setup_irq(IRQ_TIMER0, &gptmr0_irq);
	gptmr0_irq.dev_id = &clockevent_gptmr0;
	bfin_gptmr0_clockevent_init(&clockevent_gptmr0);
#endif

#if !defined(CONFIG_TICKSOURCE_CORETMR) && !defined(CONFIG_TICKSOURCE_GPTMR0)
# error at least one clock event device is required
#endif
}
