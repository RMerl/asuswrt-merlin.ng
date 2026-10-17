// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Robert Baldyga <r.baldyga@samsung.com>
 *
 * based on arch/arm/cpu/armv7/omap3/cache.S
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/cache.h>

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	dcache_enable();
}

void disable_caches(void)
{
	dcache_disable();
}
#endif

#ifndef CONFIG_SYS_L2CACHE_OFF
void v7_outer_cache_enable(void)
{
	__asm(
		"push    {r0, r1, r2, lr}\n\t"
		"mrc     15, 0, r3, cr1, cr0, 1\n\t"
		"orr     r3, r3, #2\n\t"
		"mcr     15, 0, r3, cr1, cr0, 1\n\t"
		"pop     {r1, r2, r3, pc}"
	);
}

void v7_outer_cache_disable(void)
{
	__asm(
		"push    {r0, r1, r2, lr}\n\t"
		"mrc     15, 0, r3, cr1, cr0, 1\n\t"
		"bic     r3, r3, #2\n\t"
		"mcr     15, 0, r3, cr1, cr0, 1\n\t"
		"pop     {r1, r2, r3, pc}"
	);
}
#endif
