// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Texas Insturments
 *
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
#include <asm/cache.h>
#include <asm/armv7.h>
#include <linux/compiler.h>

void __weak cpu_cache_initialization(void){}

int cleanup_before_linux_select(int flags)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * we turn off caches etc ...
	 */
#ifndef CONFIG_SPL_BUILD
	disable_interrupts();
#endif

	if (flags & CBL_DISABLE_CACHES) {
		/*
		* turn off D-cache
		* dcache_disable() in turn flushes the d-cache and disables MMU
		*/
		dcache_disable();
		v7_outer_cache_disable();

		/*
		* After D-cache is flushed and before it is disabled there may
		* be some new valid entries brought into the cache. We are
		* sure that these lines are not dirty and will not affect our
		* execution. (because unwinding the call-stack and setting a
		* bit in CP15 SCTRL is all we did during this. We have not
		* pushed anything on to the stack. Neither have we affected
		* any static data) So just invalidate the entire d-cache again
		* to avoid coherency problems for kernel
		*/
		invalidate_dcache_all();

		icache_disable();
		invalidate_icache_all();
	} else {
		/*
		 * Turn off I-cache and invalidate it
		 */
		icache_disable();
		invalidate_icache_all();

		flush_dcache_all();
		invalidate_icache_all();
		icache_enable();
	}

	/*
	 * Some CPU need more cache attention before starting the kernel.
	 */
	cpu_cache_initialization();

	return 0;
}

int cleanup_before_linux(void)
{
	return cleanup_before_linux_select(CBL_ALL);
}
