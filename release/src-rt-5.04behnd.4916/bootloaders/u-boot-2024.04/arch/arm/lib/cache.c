// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* for now: just dummy functions to satisfy the linker */

#include <common.h>
#include <cpu_func.h>
#include <log.h>
#include <malloc.h>
#include <asm/cache.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

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

void noncached_set_region(void)
{
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	mmu_set_region_dcache_behaviour(noncached_start,
					noncached_end - noncached_start,
					DCACHE_OFF);
#endif
}

int noncached_init(void)
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

	noncached_set_region();

	return 0;
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

int arch_reserve_mmu(void)
{
	return arm_reserve_mmu();
}

__weak int arm_reserve_mmu(void)
{
#if !(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
	/* reserve TLB table */
	gd->arch.tlb_size = PGTABLE_SIZE;
	gd->relocaddr -= gd->arch.tlb_size;

	/* round down to next 64 kB limit */
	gd->relocaddr &= ~(0x10000 - 1);

	gd->arch.tlb_addr = gd->relocaddr;
	debug("TLB table from %08lx to %08lx\n", gd->arch.tlb_addr,
	      gd->arch.tlb_addr + gd->arch.tlb_size);

#ifdef CFG_SYS_MEM_RESERVE_SECURE
	/*
	 * Record allocated tlb_addr in case gd->tlb_addr to be overwritten
	 * with location within secure ram.
	 */
	gd->arch.tlb_allocated = gd->arch.tlb_addr;
#endif

	if (IS_ENABLED(CONFIG_CMO_BY_VA_ONLY)) {
		/*
		 * As invalidate_dcache_all() will be called before
		 * mmu_setup(), we should make sure that the PTs are
		 * already in a valid state.
		 */
		memset((void *)gd->arch.tlb_addr, 0, gd->arch.tlb_size);
	}
#endif

	return 0;
}
