// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 */

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <asm/system.h>
#include <asm/io.h>

static void cache_flush(void);

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * we turn off caches etc ...
	 */

	disable_interrupts ();

	/* ARM926E-S needs the protection unit enabled for the icache to have
	 * been enabled	 - left for possible later use
	 * should turn off the protection unit as well....
	 */
	/* turn off I/D-cache */
	icache_disable();
	dcache_disable();
	/* flush I/D-cache */
	cache_flush();

	return 0;
}

/* flush I/D-cache */
static void cache_flush (void)
{
	unsigned long i = 0;

	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (i));
	asm ("mcr p15, 0, %0, c7, c6, 0": :"r" (i));
}

#ifndef CONFIG_ARCH_INTEGRATOR

__attribute__((noreturn)) void reset_cpu(ulong addr __attribute__((unused)))
{
	writew(0x0, 0xfffece10);
	writew(0x8, 0xfffece10);
	for (;;)
		;
}

#endif	/* #ifdef CONFIG_ARCH_INTEGRATOR */
