// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 - 2013 Tensilica Inc.
 * (C) Copyright 2014 - 2016 Cadence Design Systems Inc.
 */

/*
 * Exception handling.
 *  We currently don't handle any exception and force a reset.
 *  (Note that alloca is a special case and handled in start.S)
 */

#include <common.h>
#include <command.h>
#include <irq_func.h>
#include <asm/ptrace.h>
#include <asm/string.h>
#include <asm/regs.h>

typedef void (*handler_t)(struct pt_regs *);

void unhandled_exception(struct pt_regs *regs)
{
	printf("Unhandled Exception: EXCCAUSE = %ld, EXCVADDR = %lx, pc = %lx\n",
	       regs->exccause, regs->excvaddr, regs->pc);
	panic("*** PANIC\n");
}

handler_t exc_table[EXCCAUSE_LAST] = {
	[0 ... EXCCAUSE_LAST-1]			= unhandled_exception,
};

int interrupt_init(void)
{
	return 0;
}

void enable_interrupts(void)
{
}

int disable_interrupts(void)
{
	return 0;
}
