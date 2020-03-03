/*
 * Integrator/AP timer driver
 * Copyright (C) 2000-2003 Deep Blue Solutions Ltd
 * Copyright (c) 2014, Linaro Limited
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

#include <linux/clk.h>
#include <linux/clocksource.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/sched_clock.h>
#include <asm/hardware/arm_timer.h>

static void __iomem * sched_clk_base;

static u64 notrace integrator_read_sched_clock(void)
{
	return -readl(sched_clk_base + TIMER_VALUE);
}

static void integrator_clocksource_init(unsigned long inrate,
					void __iomem *base)
{
	u32 ctrl = TIMER_CTRL_ENABLE | TIMER_CTRL_PERIODIC;
	unsigned long rate = inrate;

	if (rate >= 1500000) {
		rate /= 16;
		ctrl |= TIMER_CTRL_DIV16;
	}

	writel(0xffff, base + TIMER_LOAD);
	writel(ctrl, base + TIMER_CTRL);

	clocksource_mmio_init(base + TIMER_VALUE, "timer2",
			rate, 200, 16, clocksource_mmio_readl_down);

	sched_clk_base = base;
	sched_clock_register(integrator_read_sched_clock, 16, rate);
}

static unsigned long timer_reload;
static void __iomem * clkevt_base;

/*
 * IRQ handler for the timer
 */
static irqreturn_t integrator_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = dev_id;

	/* clear the interrupt */
	writel(1, clkevt_base + TIMER_INTCLR);

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static void clkevt_set_mode(enum clock_event_mode mode, struct clock_event_device *evt)
{
	u32 ctrl = readl(clkevt_base + TIMER_CTRL) & ~TIMER_CTRL_ENABLE;

	/* Disable timer */
	writel(ctrl, clkevt_base + TIMER_CTRL);

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		/* Enable the timer and start the periodic tick */
		writel(timer_reload, clkevt_base + TIMER_LOAD);
		ctrl |= TIMER_CTRL_PERIODIC | TIMER_CTRL_ENABLE;
		writel(ctrl, clkevt_base + TIMER_CTRL);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		/* Leave the timer disabled, .set_next_event will enable it */
		ctrl &= ~TIMER_CTRL_PERIODIC;
		writel(ctrl, clkevt_base + TIMER_CTRL);
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	case CLOCK_EVT_MODE_RESUME:
	default:
		/* Just leave in disabled state */
		break;
	}

}

static int clkevt_set_next_event(unsigned long next, struct clock_event_device *evt)
{
	unsigned long ctrl = readl(clkevt_base + TIMER_CTRL);

	writel(ctrl & ~TIMER_CTRL_ENABLE, clkevt_base + TIMER_CTRL);
	writel(next, clkevt_base + TIMER_LOAD);
	writel(ctrl | TIMER_CTRL_ENABLE, clkevt_base + TIMER_CTRL);

	return 0;
}

static struct clock_event_device integrator_clockevent = {
	.name		= "timer1",
	.features	= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= clkevt_set_mode,
	.set_next_event	= clkevt_set_next_event,
	.rating		= 300,
};

static struct irqaction integrator_timer_irq = {
	.name		= "timer",
	.flags		= IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= integrator_timer_interrupt,
	.dev_id		= &integrator_clockevent,
};

static void integrator_clockevent_init(unsigned long inrate,
				void __iomem *base, int irq)
{
	unsigned long rate = inrate;
	unsigned int ctrl = 0;

	clkevt_base = base;
	/* Calculate and program a divisor */
	if (rate > 0x100000 * HZ) {
		rate /= 256;
		ctrl |= TIMER_CTRL_DIV256;
	} else if (rate > 0x10000 * HZ) {
		rate /= 16;
		ctrl |= TIMER_CTRL_DIV16;
	}
	timer_reload = rate / HZ;
	writel(ctrl, clkevt_base + TIMER_CTRL);

	setup_irq(irq, &integrator_timer_irq);
	clockevents_config_and_register(&integrator_clockevent,
					rate,
					1,
					0xffffU);
}

static void __init integrator_ap_timer_init_of(struct device_node *node)
{
	const char *path;
	void __iomem *base;
	int err;
	int irq;
	struct clk *clk;
	unsigned long rate;
	struct device_node *pri_node;
	struct device_node *sec_node;

	base = of_io_request_and_map(node, 0, "integrator-timer");
	if (!base)
		return;

	clk = of_clk_get(node, 0);
	if (IS_ERR(clk)) {
		pr_err("No clock for %s\n", node->name);
		return;
	}
	clk_prepare_enable(clk);
	rate = clk_get_rate(clk);
	writel(0, base + TIMER_CTRL);

	err = of_property_read_string(of_aliases,
				"arm,timer-primary", &path);
	if (WARN_ON(err))
		return;
	pri_node = of_find_node_by_path(path);
	err = of_property_read_string(of_aliases,
				"arm,timer-secondary", &path);
	if (WARN_ON(err))
		return;
	sec_node = of_find_node_by_path(path);

	if (node == pri_node) {
		/* The primary timer lacks IRQ, use as clocksource */
		integrator_clocksource_init(rate, base);
		return;
	}

	if (node == sec_node) {
		/* The secondary timer will drive the clock event */
		irq = irq_of_parse_and_map(node, 0);
		integrator_clockevent_init(rate, base, irq);
		return;
	}

	pr_info("Timer @%p unused\n", base);
	clk_disable_unprepare(clk);
}

CLOCKSOURCE_OF_DECLARE(integrator_ap_timer, "arm,integrator-timer",
		       integrator_ap_timer_init_of);
