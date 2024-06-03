// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#include <common.h>
#if !(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
static inline unsigned long CACHE_SET(unsigned char cache)
{
	if (cache == ICACHE)
		return 64 << ((GET_ICM_CFG() & ICM_CFG_MSK_ISET) \
			>> ICM_CFG_OFF_ISET);
	else
		return 64 << ((GET_DCM_CFG() & DCM_CFG_MSK_DSET) \
			>> DCM_CFG_OFF_DSET);
}

static inline unsigned long CACHE_WAY(unsigned char cache)
{
	if (cache == ICACHE)
		return 1 + ((GET_ICM_CFG() & ICM_CFG_MSK_IWAY) \
			>> ICM_CFG_OFF_IWAY);
	else
		return 1 + ((GET_DCM_CFG() & DCM_CFG_MSK_DWAY) \
			>> DCM_CFG_OFF_DWAY);
}

static inline unsigned long CACHE_LINE_SIZE(enum cache_t cache)
{
	if (cache == ICACHE)
		return 8 << (((GET_ICM_CFG() & ICM_CFG_MSK_ISZ) \
			>> ICM_CFG_OFF_ISZ) - 1);
	else
		return 8 << (((GET_DCM_CFG() & DCM_CFG_MSK_DSZ) \
			>> DCM_CFG_OFF_DSZ) - 1);
}
#endif

#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
void invalidate_icache_all(void)
{
	unsigned long end, line_size;
	line_size = CACHE_LINE_SIZE(ICACHE);
	end = line_size * CACHE_WAY(ICACHE) * CACHE_SET(ICACHE);
	do {
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL" : : "r" (end));

		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL" : : "r" (end));

		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL" : : "r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL" : : "r" (end));
	} while (end > 0);
}

void invalidate_icache_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(ICACHE);
	while (end > start) {
		asm volatile (
			"\n\tcctl %0, L1I_VA_INVAL"
			:
			: "r"(start)
		);
		start += line_size;
	}
}

void icache_enable(void)
{
	asm volatile (
		"mfsr	$p0, $mr8\n\t"
		"ori	$p0, $p0, 0x01\n\t"
		"mtsr	$p0, $mr8\n\t"
		"isb\n\t"
	);
}

void icache_disable(void)
{
	asm volatile (
		"mfsr	$p0, $mr8\n\t"
		"li	$p1, ~0x01\n\t"
		"and	$p0, $p0, $p1\n\t"
		"mtsr	$p0, $mr8\n\t"
		"isb\n\t"
	);
}

int icache_status(void)
{
	int ret;

	asm volatile (
		"mfsr	$p0, $mr8\n\t"
		"andi	%0,  $p0, 0x01\n\t"
		: "=r" (ret)
		:
		: "memory"
	);

	return ret;
}

#else
void invalidate_icache_all(void)
{
}

void invalidate_icache_range(unsigned long start, unsigned long end)
{
}

void icache_enable(void)
{
}

void icache_disable(void)
{
}

int icache_status(void)
{
	return 0;
}

#endif

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void dcache_wbinval_all(void)
{
	unsigned long end, line_size;
	line_size = CACHE_LINE_SIZE(DCACHE);
	end = line_size * CACHE_WAY(DCACHE) * CACHE_SET(DCACHE);
	do {
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_WB" : : "r" (end));
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" : : "r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_WB" : : "r" (end));
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" : : "r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_WB" : : "r" (end));
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" : : "r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_WB" : : "r" (end));
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" : : "r" (end));

	} while (end > 0);
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);

	while (end > start) {
		asm volatile (
			"\n\tcctl %0, L1D_VA_WB"
			"\n\tcctl %0, L1D_VA_INVAL" : : "r" (start)
		);
		start += line_size;
	}
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);
	while (end > start) {
		asm volatile (
			"\n\tcctl %0, L1D_VA_INVAL" : : "r"(start)
		);
		start += line_size;
	}
}

void dcache_enable(void)
{
	asm volatile (
		"mfsr	$p0, $mr8\n\t"
		"ori	$p0, $p0, 0x02\n\t"
		"mtsr	$p0, $mr8\n\t"
		"isb\n\t"
	);
}

void dcache_disable(void)
{
	asm volatile (
		"mfsr	$p0, $mr8\n\t"
		"li	$p1, ~0x02\n\t"
		"and	$p0, $p0, $p1\n\t"
		"mtsr	$p0, $mr8\n\t"
		"isb\n\t"
	);
}

int dcache_status(void)
{
	int ret;
	asm volatile (
		"mfsr	$p0, $mr8\n\t"
		"andi	%0, $p0, 0x02\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
	return ret;
}

#else
void dcache_wbinval_all(void)
{
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
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

#endif


void flush_dcache_all(void)
{
	dcache_wbinval_all();
}

void cache_flush(void)
{
	flush_dcache_all();
	invalidate_icache_all();
}


void flush_cache(unsigned long addr, unsigned long size)
{
	flush_dcache_range(addr, addr + size);
	invalidate_icache_range(addr, addr + size);
}
