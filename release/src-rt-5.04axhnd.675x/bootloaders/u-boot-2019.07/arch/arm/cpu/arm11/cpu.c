// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004 Texas Insturments
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

	/* turn off I/D-cache */
	icache_disable();
	dcache_disable();
	/* flush I/D-cache */
	cache_flush();

	return 0;
}

static void cache_flush(void)
{
	unsigned long i = 0;
	/* clean entire data cache */
	asm volatile("mcr p15, 0, %0, c7, c10, 0" : : "r" (i));
	/* invalidate both caches and flush btb */
	asm volatile("mcr p15, 0, %0, c7, c7, 0" : : "r" (i));
	/* mem barrier to sync things */
	asm volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (i));
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void invalidate_dcache_all(void)
{
	asm volatile("mcr p15, 0, %0, c7, c6, 0" : : "r" (0));
}

void flush_dcache_all(void)
{
	asm volatile("mcr p15, 0, %0, c7, c10, 0" : : "r" (0));
	asm volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (0));
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	if (!check_cache_range(start, stop))
		return;

	while (start < stop) {
		asm volatile("mcr p15, 0, %0, c7, c6, 1" : : "r" (start));
		start += CONFIG_SYS_CACHELINE_SIZE;
	}
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
	if (!check_cache_range(start, stop))
		return;

	while (start < stop) {
		asm volatile("mcr p15, 0, %0, c7, c14, 1" : : "r" (start));
		start += CONFIG_SYS_CACHELINE_SIZE;
	}

	asm volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (0));
}

#else /* #if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF) */
void invalidate_dcache_all(void)
{
}

void flush_dcache_all(void)
{
}
#endif /* #if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF) */

#if !(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
void enable_caches(void)
{
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	icache_enable();
#endif
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	dcache_enable();
#endif
}
#endif
