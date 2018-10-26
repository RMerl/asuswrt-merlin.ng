/*
 * linux/kernel/irq/handle.c
 *
 * Copyright (C) 1992, 1998-2006 Linus Torvalds, Ingo Molnar
 * Copyright (C) 2005-2006, Thomas Gleixner, Russell King
 *
 * This file contains the core interrupt handling code.
 *
 * Detailed information is available in Documentation/DocBook/genericirq
 *
 */

#include <linux/irq.h>
#include <linux/random.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>

#include <trace/events/irq.h>
#if defined(CONFIG_BCM_KF_BUZZZ) && defined(CONFIG_BUZZZ_KEVT)
#include <linux/buzzz.h>
#endif

#include "internals.h"

/**
 * handle_bad_irq - handle spurious and unhandled irqs
 * @irq:       the interrupt number
 * @desc:      description of the interrupt
 *
 * Handles spurious and unhandled IRQ's. It also prints a debugmessage.
 */
void handle_bad_irq(unsigned int irq, struct irq_desc *desc)
{
#if defined(CONFIG_BCM_KF_BUZZZ) && defined(CONFIG_BUZZZ_KEVT)
	BUZZZ_KNL3(IRQ_BAD, irq, 0);
#endif
	print_irq_desc(irq, desc);
	kstat_incr_irqs_this_cpu(irq, desc);
	ack_bad_irq(irq);
}

/*
 * Special, empty irq handler:
 */
irqreturn_t no_action(int cpl, void *dev_id)
{
	return IRQ_NONE;
}
EXPORT_SYMBOL_GPL(no_action);

static void warn_no_thread(unsigned int irq, struct irqaction *action)
{
	if (test_and_set_bit(IRQTF_WARNED, &action->thread_flags))
		return;

	printk(KERN_WARNING "IRQ %d device %s returned IRQ_WAKE_THREAD "
	       "but no thread function available.", irq, action->name);
}

void __irq_wake_thread(struct irq_desc *desc, struct irqaction *action)
{
	/*
	 * In case the thread crashed and was killed we just pretend that
	 * we handled the interrupt. The hardirq handler has disabled the
	 * device interrupt, so no irq storm is lurking.
	 */
	if (action->thread->flags & PF_EXITING)
		return;

	/*
	 * Wake up the handler thread for this action. If the
	 * RUNTHREAD bit is already set, nothing to do.
	 */
	if (test_and_set_bit(IRQTF_RUNTHREAD, &action->thread_flags))
		return;

	/*
	 * It's safe to OR the mask lockless here. We have only two
	 * places which write to threads_oneshot: This code and the
	 * irq thread.
	 *
	 * This code is the hard irq context and can never run on two
	 * cpus in parallel. If it ever does we have more serious
	 * problems than this bitmask.
	 *
	 * The irq threads of this irq which clear their "running" bit
	 * in threads_oneshot are serialized via desc->lock against
	 * each other and they are serialized against this code by
	 * IRQS_INPROGRESS.
	 *
	 * Hard irq handler:
	 *
	 *	spin_lock(desc->lock);
	 *	desc->state |= IRQS_INPROGRESS;
	 *	spin_unlock(desc->lock);
	 *	set_bit(IRQTF_RUNTHREAD, &action->thread_flags);
	 *	desc->threads_oneshot |= mask;
	 *	spin_lock(desc->lock);
	 *	desc->state &= ~IRQS_INPROGRESS;
	 *	spin_unlock(desc->lock);
	 *
	 * irq thread:
	 *
	 * again:
	 *	spin_lock(desc->lock);
	 *	if (desc->state & IRQS_INPROGRESS) {
	 *		spin_unlock(desc->lock);
	 *		while(desc->state & IRQS_INPROGRESS)
	 *			cpu_relax();
	 *		goto again;
	 *	}
	 *	if (!test_bit(IRQTF_RUNTHREAD, &action->thread_flags))
	 *		desc->threads_oneshot &= ~mask;
	 *	spin_unlock(desc->lock);
	 *
	 * So either the thread waits for us to clear IRQS_INPROGRESS
	 * or we are waiting in the flow handler for desc->lock to be
	 * released before we reach this point. The thread also checks
	 * IRQTF_RUNTHREAD under desc->lock. If set it leaves
	 * threads_oneshot untouched and runs the thread another time.
	 */
	desc->threads_oneshot |= action->thread_mask;

	/*
	 * We increment the threads_active counter in case we wake up
	 * the irq thread. The irq thread decrements the counter when
	 * it returns from the handler or in the exit path and wakes
	 * up waiters which are stuck in synchronize_irq() when the
	 * active count becomes zero. synchronize_irq() is serialized
	 * against this code (hard irq handler) via IRQS_INPROGRESS
	 * like the finalize_oneshot() code. See comment above.
	 */
	atomic_inc(&desc->threads_active);

	wake_up_process(action->thread);
}

#if defined(CONFIG_BCM_KF_HARDIRQ_CYCLES)
/* see the description in arch/mips/bcm963xx/Kconfig */
struct kernel_stat_shadow {
	struct cpu_usage_stat last_cpustat;  /* cpustat when we started accumulating */
	unsigned int start_cnt;            /**< c0 count when starting hardirq */
	unsigned int accumulated_cnt;      /**< cycles accumulated so far */
	unsigned int intrs;     /**< debug only, how many intrs accumulate whole tick */
	/* we could even expand this structure to keep track of cycle counts on a
	 * per interrupt basis and find out which interrupt is using too many
	 * cycles.  Surprisingly, the timer interrupt seems to take about 10-15us.
	 */
};

DEFINE_PER_CPU(struct kernel_stat_shadow, kstat_shadow);
static unsigned int cycles_per_tick;
extern unsigned int mips_hpt_frequency;

static void start_hardirq_count(void)
{
	struct kernel_stat_shadow *ks_shadow = &per_cpu(kstat_shadow, smp_processor_id());
	ks_shadow->start_cnt = read_c0_count();
}

static void stop_hardirq_count(void)
{
	unsigned int end_cnt = read_c0_count();
	struct kernel_stat_shadow *ks_shadow;
	ks_shadow = &per_cpu(kstat_shadow, smp_processor_id());
	ks_shadow->intrs++;
	if (end_cnt > ks_shadow->start_cnt)
		ks_shadow->accumulated_cnt += end_cnt - ks_shadow->start_cnt;
	else
		//counter rolled over
		ks_shadow->accumulated_cnt += (UINT_MAX - ks_shadow->start_cnt) + end_cnt;

	if (cycles_per_tick == 0) {
		cycles_per_tick = mips_hpt_frequency/HZ;
	}

	// See if we have accumulated a whole tick
	if (ks_shadow->accumulated_cnt >= cycles_per_tick) {
		struct cpu_usage_stat *cpustat = &kstat_this_cpu.cpustat;
		cputime64_t user_delta = cpustat->user - ks_shadow->last_cpustat.user;
		cputime64_t system_delta = cpustat->system - ks_shadow->last_cpustat.system;
		cputime64_t softirq_delta = cpustat->softirq - ks_shadow->last_cpustat.softirq;
		cputime64_t idle_delta = cpustat->idle - ks_shadow->last_cpustat.idle;

//		printk("TICK on %d in %d intrs!\n", smp_processor_id(), ks_shadow->intrs);
		cpustat->irq++;
		// subtract 1 tick from the field that has incremented the most
		if (user_delta > system_delta && user_delta > softirq_delta && user_delta > idle_delta)
			cpustat->user--;
		else if (system_delta > user_delta && system_delta > softirq_delta && system_delta > idle_delta)
			cpustat->system--;
		else if (softirq_delta > user_delta && softirq_delta > system_delta && softirq_delta > idle_delta)
			cpustat->softirq--;
		else
			cpustat->idle--;

		ks_shadow->accumulated_cnt -= cycles_per_tick;
		ks_shadow->intrs = 0;
		ks_shadow->last_cpustat = *cpustat;
	}
}
#endif


irqreturn_t
handle_irq_event_percpu(struct irq_desc *desc, struct irqaction *action)
{
	irqreturn_t retval = IRQ_NONE;
	unsigned int flags = 0, irq = desc->irq_data.irq;

#if defined(CONFIG_BCM_KF_HARDIRQ_CYCLES)
	start_hardirq_count();
#endif

	do {
		irqreturn_t res;

		trace_irq_handler_entry(irq, action);
#if defined(CONFIG_BCM_KF_BUZZZ) && defined(CONFIG_BUZZZ_KEVT)
		BUZZZ_KNL3(IRQ_ENT, irq, action->handler);
#endif
		res = action->handler(irq, action->dev_id);
#if defined(CONFIG_BCM_KF_BUZZZ) && defined(CONFIG_BUZZZ_KEVT)
		BUZZZ_KNL3(IRQ_EXT, irq, action->handler);
#endif
		trace_irq_handler_exit(irq, action, res);

		if (WARN_ONCE(!irqs_disabled(),"irq %u handler %pF enabled interrupts\n",
			      irq, action->handler))
			local_irq_disable();

		switch (res) {
		case IRQ_WAKE_THREAD:
			/*
			 * Catch drivers which return WAKE_THREAD but
			 * did not set up a thread function
			 */
			if (unlikely(!action->thread_fn)) {
				warn_no_thread(irq, action);
				break;
			}

			__irq_wake_thread(desc, action);

			/* Fall through to add to randomness */
		case IRQ_HANDLED:
			flags |= action->flags;
			break;

		default:
			break;
		}

		retval |= res;
		action = action->next;
	} while (action);

	add_interrupt_randomness(irq, flags);

	if (!noirqdebug)
		note_interrupt(irq, desc, retval);

#if defined(CONFIG_BCM_KF_HARDIRQ_CYCLES)
	stop_hardirq_count();
#endif
		
	return retval;
}

irqreturn_t handle_irq_event(struct irq_desc *desc)
{
	struct irqaction *action = desc->action;
	irqreturn_t ret;

	desc->istate &= ~IRQS_PENDING;
	irqd_set(&desc->irq_data, IRQD_IRQ_INPROGRESS);
	raw_spin_unlock(&desc->lock);

	ret = handle_irq_event_percpu(desc, action);

	raw_spin_lock(&desc->lock);
	irqd_clear(&desc->irq_data, IRQD_IRQ_INPROGRESS);
	return ret;
}
