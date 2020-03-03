/*
 *	Copyright (C) 1992, 1998 Linus Torvalds, Ingo Molnar
 *
 * This file contains the lowest level x86_64-specific interrupt
 * entry and irq statistics code. All the remaining irq logic is
 * done by the generic kernel/irq/ code and in the
 * x86_64-specific irq controller code. (e.g. i8259.c and
 * io_apic.c.)
 */

#include <linux/kernel_stat.h>
#include <linux/interrupt.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/ftrace.h>
#include <linux/uaccess.h>
#include <linux/smp.h>
#include <asm/io_apic.h>
#include <asm/idle.h>
#include <asm/apic.h>

DEFINE_PER_CPU_SHARED_ALIGNED(irq_cpustat_t, irq_stat);
EXPORT_PER_CPU_SYMBOL(irq_stat);

DEFINE_PER_CPU(struct pt_regs *, irq_regs);
EXPORT_PER_CPU_SYMBOL(irq_regs);

int sysctl_panic_on_stackoverflow;

/*
 * Probabilistic stack overflow check:
 *
 * Only check the stack in process context, because everything else
 * runs on the big interrupt stacks. Checking reliably is too expensive,
 * so we just check from interrupts.
 */
static inline void stack_overflow_check(struct pt_regs *regs)
{
#ifdef CONFIG_DEBUG_STACKOVERFLOW
#define STACK_TOP_MARGIN	128
	struct orig_ist *oist;
	u64 irq_stack_top, irq_stack_bottom;
	u64 estack_top, estack_bottom;
	u64 curbase = (u64)task_stack_page(current);

	if (user_mode(regs))
		return;

	if (regs->sp >= curbase + sizeof(struct thread_info) +
				  sizeof(struct pt_regs) + STACK_TOP_MARGIN &&
	    regs->sp <= curbase + THREAD_SIZE)
		return;

	irq_stack_top = (u64)this_cpu_ptr(irq_stack_union.irq_stack) +
			STACK_TOP_MARGIN;
	irq_stack_bottom = (u64)__this_cpu_read(irq_stack_ptr);
	if (regs->sp >= irq_stack_top && regs->sp <= irq_stack_bottom)
		return;

	oist = this_cpu_ptr(&orig_ist);
	estack_top = (u64)oist->ist[0] - EXCEPTION_STKSZ + STACK_TOP_MARGIN;
	estack_bottom = (u64)oist->ist[N_EXCEPTION_STACKS - 1];
	if (regs->sp >= estack_top && regs->sp <= estack_bottom)
		return;

	WARN_ONCE(1, "do_IRQ(): %s has overflown the kernel stack (cur:%Lx,sp:%lx,irq stk top-bottom:%Lx-%Lx,exception stk top-bottom:%Lx-%Lx)\n",
		current->comm, curbase, regs->sp,
		irq_stack_top, irq_stack_bottom,
		estack_top, estack_bottom);

	if (sysctl_panic_on_stackoverflow)
		panic("low stack detected by irq handler - check messages\n");
#endif
}

bool handle_irq(unsigned irq, struct pt_regs *regs)
{
	struct irq_desc *desc;

	stack_overflow_check(regs);

	desc = irq_to_desc(irq);
	if (unlikely(!desc))
		return false;

	generic_handle_irq_desc(irq, desc);
	return true;
}
