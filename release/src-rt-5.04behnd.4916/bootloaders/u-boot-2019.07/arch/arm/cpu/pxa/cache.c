// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <linux/types.h>
#include <common.h>

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void invalidate_dcache_all(void)
{
	/* Flush/Invalidate I cache */
	asm volatile("mcr p15, 0, %0, c7, c5, 0\n" : : "r"(0));
	/* Flush/Invalidate D cache */
	asm volatile("mcr p15, 0, %0, c7, c6, 0\n" : : "r"(0));
}

void flush_dcache_all(void)
{
	return invalidate_dcache_all();
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	start &= ~(CONFIG_SYS_CACHELINE_SIZE - 1);
	stop &= ~(CONFIG_SYS_CACHELINE_SIZE - 1);

	while (start <= stop) {
		asm volatile("mcr p15, 0, %0, c7, c6, 1\n" : : "r"(start));
		start += CONFIG_SYS_CACHELINE_SIZE;
	}
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
	return invalidate_dcache_range(start, stop);
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
