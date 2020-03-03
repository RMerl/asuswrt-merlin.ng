/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * vineetg: Jan 1011
 *  -sched_clock( ) no longer jiffies based. Uses the same clocksource
 *   as gtod
 *
 * Rajeshwarr/Vineetg: Mar 2008
 *  -Implemented CONFIG_GENERIC_TIME (rather deleted arch specific code)
 *   for arch independent gettimeofday()
 *  -Implemented CONFIG_GENERIC_CLOCKEVENTS as base for hrtimers
 *
 * Vineetg: Mar 2008: Forked off from time.c which now is time-jiff.c
 */

/* ARC700 has two 32bit independent prog Timers: TIMER0 and TIMER1
 * Each can programmed to go from @count to @limit and optionally
 * interrupt when that happens.
 * A write to Control Register clears the Interrupt
 *
 * We've designated TIMER0 for events (clockevents)
 * while TIMER1 for free running (clocksource)
 *
 * Newer ARC700 cores have 64bit clk fetching RTSC insn, preferred over TIMER1
 */

#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/timex.h>
#include <linux/profile.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <asm/irq.h>
#include <asm/arcregs.h>
#include <asm/clk.h>
#include <asm/mach_desc.h>

/* Timer related Aux registers */
#define ARC_REG_TIMER0_LIMIT	0x23	/* timer 0 limit */
#define ARC_REG_TIMER0_CTRL	0x22	/* timer 0 control */
#define ARC_REG_TIMER0_CNT	0x21	/* timer 0 count */
#define ARC_REG_TIMER1_LIMIT	0x102	/* timer 1 limit */
#define ARC_REG_TIMER1_CTRL	0x101	/* timer 1 control */
#define ARC_REG_TIMER1_CNT	0x100	/* timer 1 count */

#define TIMER_CTRL_IE		(1 << 0) /* Interupt when Count reachs limit */
#define TIMER_CTRL_NH		(1 << 1) /* Count only when CPU NOT halted */

#define ARC_TIMER_MAX	0xFFFFFFFF

/********** Clock Source Device *********/

#ifdef CONFIG_ARC_HAS_RTSC

int arc_counter_setup(void)
{
	/*
	 * For SMP this needs to be 0. However Kconfig glue doesn't
	 * enable this option for SMP configs
	 */
	return 1;
}

static cycle_t arc_counter_read(struct clocksource *cs)
{
	unsigned long flags;
	union {
#ifdef CONFIG_CPU_BIG_ENDIAN
		struct { u32 high, low; };
#else
		struct { u32 low, high; };
#endif
		cycle_t  full;
	} stamp;

	flags = arch_local_irq_save();

	__asm__ __volatile(
	"	.extCoreRegister tsch, 58,  r, cannot_shortcut	\n"
	"	rtsc %0, 0	\n"
	"	mov  %1, 0	\n"
	: "=r" (stamp.low), "=r" (stamp.high));

	arch_local_irq_restore(flags);

	return stamp.full;
}

static struct clocksource arc_counter = {
	.name   = "ARC RTSC",
	.rating = 300,
	.read   = arc_counter_read,
	.mask   = CLOCKSOURCE_MASK(32),
	.flags  = CLOCK_SOURCE_IS_CONTINUOUS,
};

#else /* !CONFIG_ARC_HAS_RTSC */

static bool is_usable_as_clocksource(void)
{
#ifdef CONFIG_SMP
	return 0;
#else
	return 1;
#endif
}

/*
 * set 32bit TIMER1 to keep counting monotonically and wraparound
 */
int arc_counter_setup(void)
{
	write_aux_reg(ARC_REG_TIMER1_LIMIT, ARC_TIMER_MAX);
	write_aux_reg(ARC_REG_TIMER1_CNT, 0);
	write_aux_reg(ARC_REG_TIMER1_CTRL, TIMER_CTRL_NH);

	return is_usable_as_clocksource();
}

static cycle_t arc_counter_read(struct clocksource *cs)
{
	return (cycle_t) read_aux_reg(ARC_REG_TIMER1_CNT);
}

static struct clocksource arc_counter = {
	.name   = "ARC Timer1",
	.rating = 300,
	.read   = arc_counter_read,
	.mask   = CLOCKSOURCE_MASK(32),
	.flags  = CLOCK_SOURCE_IS_CONTINUOUS,
};

#endif

/********** Clock Event Device *********/

/*
 * Arm the timer to interrupt after @cycles
 * The distinction for oneshot/periodic is done in arc_event_timer_ack() below
 */
static void arc_timer_event_setup(unsigned int cycles)
{
	write_aux_reg(ARC_REG_TIMER0_LIMIT, cycles);
	write_aux_reg(ARC_REG_TIMER0_CNT, 0);	/* start from 0 */

	write_aux_reg(ARC_REG_TIMER0_CTRL, TIMER_CTRL_IE | TIMER_CTRL_NH);
}


static int arc_clkevent_set_next_event(unsigned long delta,
				       struct clock_event_device *dev)
{
	arc_timer_event_setup(delta);
	return 0;
}

static void arc_clkevent_set_mode(enum clock_event_mode mode,
				  struct clock_event_device *dev)
{
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
                /*
                 * At X Hz, 1 sec = 1000ms -> X cycles;
                 *                    10ms -> X / 100 cycles
                 */
		arc_timer_event_setup(arc_get_core_freq() / HZ);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		break;
	default:
		break;
	}

	return;
}

static DEFINE_PER_CPU(struct clock_event_device, arc_clockevent_device) = {
	.name		= "ARC Timer0",
	.features	= CLOCK_EVT_FEAT_ONESHOT | CLOCK_EVT_FEAT_PERIODIC,
	.mode		= CLOCK_EVT_MODE_UNUSED,
	.rating		= 300,
	.irq		= TIMER0_IRQ,	/* hardwired, no need for resources */
	.set_next_event = arc_clkevent_set_next_event,
	.set_mode	= arc_clkevent_set_mode,
};

static irqreturn_t timer_irq_handler(int irq, void *dev_id)
{
	/*
	 * Note that generic IRQ core could have passed @evt for @dev_id if
	 * irq_set_chip_and_handler() asked for handle_percpu_devid_irq()
	 */
	struct clock_event_device *evt = this_cpu_ptr(&arc_clockevent_device);
	int irq_reenable = evt->mode == CLOCK_EVT_MODE_PERIODIC;

	/*
	 * Any write to CTRL reg ACks the interrupt, we rewrite the
	 * Count when [N]ot [H]alted bit.
	 * And re-arm it if perioid by [I]nterrupt [E]nable bit
	 */
	write_aux_reg(ARC_REG_TIMER0_CTRL, irq_reenable | TIMER_CTRL_NH);

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

/*
 * Setup the local event timer for @cpu
 */
void arc_local_timer_setup()
{
	struct clock_event_device *evt = this_cpu_ptr(&arc_clockevent_device);
	int cpu = smp_processor_id();

	evt->cpumask = cpumask_of(cpu);
	clockevents_config_and_register(evt, arc_get_core_freq(),
					0, ARC_TIMER_MAX);

	/* setup the per-cpu timer IRQ handler - for all cpus */
	arc_request_percpu_irq(TIMER0_IRQ, cpu, timer_irq_handler,
			       "Timer0 (per-cpu-tick)", evt);
}

/*
 * Called from start_kernel() - boot CPU only
 *
 * -Sets up h/w timers as applicable on boot cpu
 * -Also sets up any global state needed for timer subsystem:
 *    - for "counting" timer, registers a clocksource, usable across CPUs
 *      (provided that underlying counter h/w is synchronized across cores)
 *    - for "event" timer, sets up TIMER0 IRQ (as that is platform agnostic)
 */
void __init time_init(void)
{
	/*
	 * sets up the timekeeping free-flowing counter which also returns
	 * whether the counter is usable as clocksource
	 */
	if (arc_counter_setup())
		/*
		 * CLK upto 4.29 GHz can be safely represented in 32 bits
		 * because Max 32 bit number is 4,294,967,295
		 */
		clocksource_register_hz(&arc_counter, arc_get_core_freq());

	/* sets up the periodic event timer */
	arc_local_timer_setup();

	if (machine_desc->init_time)
		machine_desc->init_time();
}
