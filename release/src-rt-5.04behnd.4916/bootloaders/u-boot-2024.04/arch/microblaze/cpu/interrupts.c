// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Michal Simek
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Michal  SIMEK <monstr@monstr.eu>
 * Yasushi SHOJI <yashi@atmark-techno.com>
 */

#include <log.h>
#include <vsprintf.h>
#include <asm/asm.h>

void enable_interrupts(void)
{
	debug("Enable interrupts for the whole CPU\n");
	MSRSET(0x2);
}

int disable_interrupts(void)
{
	unsigned int msr;

	MFS(msr, rmsr);
	MSRCLR(0x2);
	return (msr & 0x2) != 0;
}

int interrupt_init(void)
{
	return 0;
}

void interrupt_handler(void)
{
	panic("Interrupt occurred\n");
}
