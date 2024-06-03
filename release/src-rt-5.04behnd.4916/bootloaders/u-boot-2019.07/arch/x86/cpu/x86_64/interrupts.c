// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <asm/processor-flags.h>

void enable_interrupts(void)
{
	asm("sti\n");
}

int disable_interrupts(void)
{
	long flags;

	asm volatile ("pushfq ; popq %0 ; cli\n" : "=g" (flags) : );

	return flags & X86_EFLAGS_IF;
}

int interrupt_init(void)
{
	/* Nothing to do - this was already done in SPL */
	return 0;
}
