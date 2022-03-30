// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* for now: just dummy functions to satisfy the linker */

#include <common.h>
#include <malloc.h>

/*
 * Flush range from all levels of d-cache/unified-cache.
 * Affects the range [start, start + size - 1].
 */
__weak void flush_cache(unsigned long start, unsigned long size)
{
	flush_dcache_range(start, start + size);
}

/*
 * Default implementation:
 * do a range flush for the entire range
 */
__weak void flush_dcache_all(void)
{
	flush_cache(0, ~0);
}

/*
 * Default implementation of enable_caches()
 * Real implementation should be in platform code
 */
__weak void enable_caches(void)
{
	puts("WARNING: Caches not enabled\n");
}

__weak void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	/* An empty stub, real implementation should be in platform code */
}
__weak void flush_dcache_range(unsigned long start, unsigned long stop)
{
	/* An empty stub, real implementation should be in platform code */
}

int check_cache_range(unsigned long start, unsigned long stop)
{
	int ok = 1;

	if (start & (CONFIG_SYS_CACHELINE_SIZE - 1))
		ok = 0;

	if (stop & (CONFIG_SYS_CACHELINE_SIZE - 1))
		ok = 0;

	if (!ok) {
		warn_non_spl("CACHE: Misaligned operation at range [%08lx, %08lx]\n",
			     start, stop);
	}

	return ok;
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
/*
 * Reserve one MMU section worth of address space below the malloc() area that
 * will be mapped uncached.
 */
static unsigned long noncached_start;
static unsigned long noncached_end;
static unsigned long noncached_next;

void noncached_init(void)
{
	phys_addr_t start, end;
	size_t size;

	/* If this calculation changes, update board_f.c:reserve_noncached() */
	end = ALIGN(mem_malloc_start, MMU_SECTION_SIZE) - MMU_SECTION_SIZE;
	size = ALIGN(CONFIG_SYS_NONCACHED_MEMORY, MMU_SECTION_SIZE);
	start = end - size;

	debug("mapping memory %pa-%pa non-cached\n", &start, &end);

	noncached_start = start;
	noncached_end = end;
	noncached_next = start;

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	mmu_set_region_dcache_behaviour(noncached_start, size, DCACHE_OFF);
#endif
}

phys_addr_t noncached_alloc(size_t size, size_t align)
{
	phys_addr_t next = ALIGN(noncached_next, align);

	if (next >= noncached_end || (noncached_end - next) < size)
		return 0;

	debug("allocated %zu bytes of uncached memory @%pa\n", size, &next);
	noncached_next = next + size;

	return next;
}
#endif /* CONFIG_SYS_NONCACHED_MEMORY */

#if CONFIG_IS_ENABLED(SYS_THUMB_BUILD)
void invalidate_l2_cache(void)
{
	unsigned int val = 0;

	asm volatile("mcr p15, 1, %0, c15, c11, 0 @ invl l2 cache"
		: : "r" (val) : "cc");
	isb();
}
#endif
