// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 - 2013 Tensilica Inc.
 * (C) Copyright 2014 - 2016 Cadence Design Systems Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/cache.h>

/*
 * We currently run always with caches enabled when running from memory.
 * Xtensa version D or later will support changing cache behavior, so
 * we could implement it if necessary.
 */

int dcache_status(void)
{
	return 1;
}

void dcache_enable(void)
{
}

void dcache_disable(void)
{
}

void flush_cache(ulong start_addr, ulong size)
{
	__flush_invalidate_dcache_range(start_addr, size);
	__invalidate_icache_range(start_addr, size);
}

void flush_dcache_all(void)
{
	__flush_dcache_all();
	__invalidate_icache_all();
}

void flush_dcache_range(ulong start_addr, ulong end_addr)
{
	__flush_invalidate_dcache_range(start_addr, end_addr - start_addr);
}

void invalidate_dcache_range(ulong start, ulong stop)
{
	__invalidate_dcache_range(start, stop - start);
}

void invalidate_dcache_all(void)
{
	__invalidate_dcache_all();
}

void invalidate_icache_all(void)
{
	__invalidate_icache_all();
}
