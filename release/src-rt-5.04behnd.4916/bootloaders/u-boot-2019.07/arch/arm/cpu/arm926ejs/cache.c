// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Ilya Yanok, EmCraft Systems
 */
#include <linux/types.h>
#include <common.h>

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void invalidate_dcache_all(void)
{
	asm volatile("mcr p15, 0, %0, c7, c6, 0\n" : : "r"(0));
}

void flush_dcache_all(void)
{
	asm volatile(
		"0:"
		"mrc p15, 0, r15, c7, c14, 3\n"
		"bne 0b\n"
		"mcr p15, 0, %0, c7, c10, 4\n"
		 : : "r"(0) : "memory"
	);
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	if (!check_cache_range(start, stop))
		return;

	while (start < stop) {
		asm volatile("mcr p15, 0, %0, c7, c6, 1\n" : : "r"(start));
		start += CONFIG_SYS_CACHELINE_SIZE;
	}
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
	if (!check_cache_range(start, stop))
		return;

	while (start < stop) {
		asm volatile("mcr p15, 0, %0, c7, c14, 1\n" : : "r"(start));
		start += CONFIG_SYS_CACHELINE_SIZE;
	}

	asm volatile("mcr p15, 0, %0, c7, c10, 4\n" : : "r"(0));
}
#else /* #if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF) */
void invalidate_dcache_all(void)
{
}

void flush_dcache_all(void)
{
}
#endif /* #if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF) */

/*
 * Stub implementations for l2 cache operations
 */

__weak void l2_cache_disable(void) {}

#if CONFIG_IS_ENABLED(SYS_THUMB_BUILD)
__weak void invalidate_l2_cache(void) {}
#endif

#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
/* Invalidate entire I-cache and branch predictor array */
void invalidate_icache_all(void)
{
	unsigned long i = 0;

	asm ("mcr p15, 0, %0, c7, c5, 0" : : "r" (i));
}
#else
void invalidate_icache_all(void) {}
#endif

void enable_caches(void)
{
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	icache_enable();
#endif
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	dcache_enable();
#endif
}

