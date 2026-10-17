// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2003
 * Gleb Natapov <gnatapov@mrv.com>
 */

#include <common.h>
#include <irq_func.h>
#include <asm/processor.h>
#include <watchdog.h>
#ifdef CONFIG_LED_STATUS
#include <status_led.h>
#endif
#include <asm/ptrace.h>

#ifndef CONFIG_MPC83XX_TIMER
#ifndef CFG_SYS_WATCHDOG_FREQ
#define CFG_SYS_WATCHDOG_FREQ (CONFIG_SYS_HZ / 2)
#endif

static unsigned decrementer_count; /* count value for 1e6/HZ microseconds */

static __inline__ unsigned long get_dec (void)
{
	unsigned long val;

	asm volatile ("mfdec %0":"=r" (val):);

	return val;
}


static __inline__ void set_dec (unsigned long val)
{
	if (val)
		asm volatile ("mtdec %0"::"r" (val));
}
#endif /* !CONFIG_MPC83XX_TIMER */

void enable_interrupts(void)
{
	set_msr (get_msr () | MSR_EE);
}

/* returns flag if MSR_EE was set before */
int disable_interrupts(void)
{
	ulong msr = get_msr ();

	set_msr (msr & ~MSR_EE);
	return ((msr & MSR_EE) != 0);
}

#ifndef CONFIG_MPC83XX_TIMER
int interrupt_init(void)
{
	/* call cpu specific function from $(CPU)/interrupts.c */
	interrupt_init_cpu (&decrementer_count);

	set_dec (decrementer_count);

	set_msr (get_msr () | MSR_EE);

	return (0);
}

static volatile ulong timestamp = 0;

void timer_interrupt(struct pt_regs *regs)
{
	/* call cpu specific function from $(CPU)/interrupts.c */
	timer_interrupt_cpu (regs);

	/* Restore Decrementer Count */
	set_dec (decrementer_count);

	timestamp++;

#if defined(CONFIG_WATCHDOG) || defined (CONFIG_HW_WATCHDOG)
	if (CFG_SYS_WATCHDOG_FREQ && (timestamp % (CFG_SYS_WATCHDOG_FREQ)) == 0)
		schedule();
#endif    /* CONFIG_WATCHDOG || CONFIG_HW_WATCHDOG */

#ifdef CONFIG_LED_STATUS
	status_led_tick(timestamp);
#endif /* CONFIG_LED_STATUS */
}

ulong get_timer (ulong base)
{
	return (timestamp - base);
}
#endif /* !CONFIG_MPC83XX_TIMER */
