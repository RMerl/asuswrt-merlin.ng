// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Vladimir Zapolskiy <vz@mleia.com>
 * (C) Copyright 2007 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/system.h>

#define CACHE_VALID       1
#define CACHE_UPDATED     2

static inline void cache_wback_all(void)
{
	unsigned long addr, data, i, j;

	for (i = 0; i < CACHE_OC_NUM_ENTRIES; i++) {
		for (j = 0; j < CACHE_OC_NUM_WAYS; j++) {
			addr = CACHE_OC_ADDRESS_ARRAY
				| (j << CACHE_OC_WAY_SHIFT)
				| (i << CACHE_OC_ENTRY_SHIFT);
			data = inl(addr);
			if (data & CACHE_UPDATED) {
				data &= ~CACHE_UPDATED;
				outl(data, addr);
			}
		}
	}
}

#define CACHE_ENABLE      0
#define CACHE_DISABLE     1

static int cache_control(unsigned int cmd)
{
	unsigned long ccr;

	jump_to_P2();
	ccr = inl(CCR);

	if (ccr & CCR_CACHE_ENABLE)
		cache_wback_all();

	if (cmd == CACHE_DISABLE)
		outl(CCR_CACHE_STOP, CCR);
	else
		outl(CCR_CACHE_INIT, CCR);
	back_to_P1();

	return 0;
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	u32 v;

	start &= ~(L1_CACHE_BYTES - 1);
	for (v = start; v < end; v += L1_CACHE_BYTES) {
		asm volatile ("ocbp     %0" :	/* no output */
			      : "m" (__m(v)));
	}
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	u32 v;

	start &= ~(L1_CACHE_BYTES - 1);
	for (v = start; v < end; v += L1_CACHE_BYTES) {
		asm volatile ("ocbi     %0" :	/* no output */
			      : "m" (__m(v)));
	}
}

void flush_cache(unsigned long addr, unsigned long size)
{
	flush_dcache_range(addr , addr + size);
}

void icache_enable(void)
{
	cache_control(CACHE_ENABLE);
}

void icache_disable(void)
{
	cache_control(CACHE_DISABLE);
}

int icache_status(void)
{
	return 0;
}

void dcache_enable(void)
{
}

void dcache_disable(void)
{
}

int dcache_status(void)
{
	return 0;
}
