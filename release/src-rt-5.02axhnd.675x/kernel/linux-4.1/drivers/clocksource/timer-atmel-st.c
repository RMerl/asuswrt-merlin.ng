/*
 * linux/arch/arm/mach-at91/at91rm9200_time.c
 *
 *  Copyright (C) 2003 SAN People
 *  Copyright (C) 2003 ATMEL
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/clockchips.h>
#include <linux/export.h>
#include <linux/mfd/syscon.h>
#include <linux/mfd/syscon/atmel-st.h>
#include <linux/of_irq.h>
#include <linux/regmap.h>

static unsigned long last_crtr;
static u32 irqmask;
static struct clock_event_device clkevt;
static struct regmap *regmap_st;

#define AT91_SLOW_CLOCK		32768
#define RM9200_TIMER_LATCH	((AT91_SLOW_CLOCK + HZ/2) / HZ)

/*
 * The ST_CRTR is updated asynchronously to the master clock ... but
 * the updates as seen by the CPU don't seem to be strictly monotonic.
 * Waiting until we read the same value twice avoids glitching.
 */
static inline unsigned long read_CRTR(void)
{
	unsigned int x1, x2;

	regmap_read(regmap_st, AT91_ST_CRTR, &x1);
	do {
		regmap_read(regmap_st, AT91_ST_CRTR, &x2);
		if (x1 == x2)
			break;
		x1 = x2;
	} while (1);
	return x1;
}

/*
 * IRQ handler for the timer.
 */
static irqreturn_t at91rm9200_timer_interrupt(int irq, void *dev_id)
{
	u32 sr;

	regmap_read(regmap_st, AT91_ST_SR, &sr);
	sr &= irqmask;

	/*
	 * irqs should be disabled here, but as the irq is shared they are only
	 * guaranteed to be off if the timer irq is registered first.
	 */
	WARN_ON_ONCE(!irqs_disabled());

	/* simulate "oneshot" timer with alarm */
	if (sr & AT91_ST_ALMS) {
		clkevt.event_handler(&clkevt);
		return IRQ_HANDLED;
	}

	/* periodic mode should handle delayed ticks */
	if (sr & AT91_ST_PITS) {
		u32	crtr = read_CRTR();

		while (((crtr - last_crtr) & AT91_ST_CRTV) >= RM9200_TIMER_LATCH) {
			last_crtr += RM9200_TIMER_LATCH;
			clkevt.event_handler(&clkevt);
		}
		return IRQ_HANDLED;
	}

	/* this irq is shared ... */
	return IRQ_NONE;
}

static cycle_t read_clk32k(struct clocksource *cs)
{
	return read_CRTR();
}

static struct clocksource clk32k = {
	.name		= "32k_counter",
	.rating		= 150,
	.read		= read_clk32k,
	.mask		= CLOCKSOURCE_MASK(20),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static void
clkevt32k_mode(enum clock_event_mode mode, struct clock_event_device *dev)
{
	unsigned int val;

	/* Disable and flush pending timer interrupts */
	regmap_write(regmap_st, AT91_ST_IDR, AT91_ST_PITS | AT91_ST_ALMS);
	regmap_read(regmap_st, AT91_ST_SR, &val);

	last_crtr = read_CRTR();
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		/* PIT for periodic irqs; fixed rate of 1/HZ */
		irqmask = AT91_ST_PITS;
		regmap_write(regmap_st, AT91_ST_PIMR, RM9200_TIMER_LATCH);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		/* ALM for oneshot irqs, set by next_event()
		 * before 32 seconds have passed
		 */
		irqmask = AT91_ST_ALMS;
		regmap_write(regmap_st, AT91_ST_RTAR, last_crtr);
		break;
	case CLOCK_EVT_MODE_SHUTDOWN:
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_RESUME:
		irqmask = 0;
		break;
	}
	regmap_write(regmap_st, AT91_ST_IER, irqmask);
}

static int
clkevt32k_next_event(unsigned long delta, struct clock_event_device *dev)
{
	u32		alm;
	int		status = 0;
	unsigned int	val;

	BUG_ON(delta < 2);

	/* The alarm IRQ uses absolute time (now+delta), not the relative
	 * time (delta) in our calling convention.  Like all clockevents
	 * using such "match" hardware, we have a race to defend against.
	 *
	 * Our defense here is to have set up the clockevent device so the
	 * delta is at least two.  That way we never end up writing RTAR
	 * with the value then held in CRTR ... which would mean the match
	 * wouldn't trigger until 32 seconds later, after CRTR wraps.
	 */
	alm = read_CRTR();

	/* Cancel any pending alarm; flush any pending IRQ */
	regmap_write(regmap_st, AT91_ST_RTAR, alm);
	regmap_read(regmap_st, AT91_ST_SR, &val);

	/* Schedule alarm by writing RTAR. */
	alm += delta;
	regmap_write(regmap_st, AT91_ST_RTAR, alm);

	return status;
}

static struct clock_event_device clkevt = {
	.name		= "at91_tick",
	.features	= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.rating		= 150,
	.set_next_event	= clkevt32k_next_event,
	.set_mode	= clkevt32k_mode,
};

/*
 * ST (system timer) module supports both clockevents and clocksource.
 */
static void __init atmel_st_timer_init(struct device_node *node)
{
	unsigned int val;
	int irq, ret;

	regmap_st = syscon_node_to_regmap(node);
	if (IS_ERR(regmap_st))
		panic(pr_fmt("Unable to get regmap\n"));

	/* Disable all timer interrupts, and clear any pending ones */
	regmap_write(regmap_st, AT91_ST_IDR,
		AT91_ST_PITS | AT91_ST_WDOVF | AT91_ST_RTTINC | AT91_ST_ALMS);
	regmap_read(regmap_st, AT91_ST_SR, &val);

	/* Get the interrupts property */
	irq  = irq_of_parse_and_map(node, 0);
	if (!irq)
		panic(pr_fmt("Unable to get IRQ from DT\n"));

	/* Make IRQs happen for the system timer */
	ret = request_irq(irq, at91rm9200_timer_interrupt,
			  IRQF_SHARED | IRQF_TIMER | IRQF_IRQPOLL,
			  "at91_tick", regmap_st);
	if (ret)
		panic(pr_fmt("Unable to setup IRQ\n"));

	/* The 32KiHz "Slow Clock" (tick every 30517.58 nanoseconds) is used
	 * directly for the clocksource and all clockevents, after adjusting
	 * its prescaler from the 1 Hz default.
	 */
	regmap_write(regmap_st, AT91_ST_RTMR, 1);

	/* Setup timer clockevent, with minimum of two ticks (important!!) */
	clkevt.cpumask = cpumask_of(0);
	clockevents_config_and_register(&clkevt, AT91_SLOW_CLOCK,
					2, AT91_ST_ALMV);

	/* register clocksource */
	clocksource_register_hz(&clk32k, AT91_SLOW_CLOCK);
}
CLOCKSOURCE_OF_DECLARE(atmel_st_timer, "atmel,at91rm9200-st",
		       atmel_st_timer_init);
