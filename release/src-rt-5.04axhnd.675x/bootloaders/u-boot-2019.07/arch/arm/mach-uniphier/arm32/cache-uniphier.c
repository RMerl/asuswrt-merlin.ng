// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <asm/armv7.h>
#include <asm/processor.h>

#include "cache-uniphier.h"

/* control registers */
#define UNIPHIER_SSCC		0x500c0000	/* Control Register */
#define    UNIPHIER_SSCC_BST			(0x1 << 20)	/* UCWG burst read */
#define    UNIPHIER_SSCC_ACT			(0x1 << 19)	/* Inst-Data separate */
#define    UNIPHIER_SSCC_WTG			(0x1 << 18)	/* WT gathering on */
#define    UNIPHIER_SSCC_PRD			(0x1 << 17)	/* enable pre-fetch */
#define    UNIPHIER_SSCC_ON			(0x1 <<  0)	/* enable cache */
#define UNIPHIER_SSCLPDAWCR	0x500c0030	/* Unified/Data Active Way Control */
#define UNIPHIER_SSCLPIAWCR	0x500c0034	/* Instruction Active Way Control */

/* revision registers */
#define UNIPHIER_SSCID		0x503c0100	/* ID Register */

/* operation registers */
#define UNIPHIER_SSCOPE		0x506c0244	/* Cache Operation Primitive Entry */
#define    UNIPHIER_SSCOPE_CM_INV		0x0	/* invalidate */
#define    UNIPHIER_SSCOPE_CM_CLEAN		0x1	/* clean */
#define    UNIPHIER_SSCOPE_CM_FLUSH		0x2	/* flush */
#define    UNIPHIER_SSCOPE_CM_SYNC		0x8	/* sync (drain bufs) */
#define    UNIPHIER_SSCOPE_CM_FLUSH_PREFETCH	0x9	/* flush p-fetch buf */
#define UNIPHIER_SSCOQM		0x506c0248
#define    UNIPHIER_SSCOQM_TID_MASK		(0x3 << 21)
#define    UNIPHIER_SSCOQM_TID_LRU_DATA		(0x0 << 21)
#define    UNIPHIER_SSCOQM_TID_LRU_INST		(0x1 << 21)
#define    UNIPHIER_SSCOQM_TID_WAY		(0x2 << 21)
#define    UNIPHIER_SSCOQM_S_MASK		(0x3 << 17)
#define    UNIPHIER_SSCOQM_S_RANGE		(0x0 << 17)
#define    UNIPHIER_SSCOQM_S_ALL		(0x1 << 17)
#define    UNIPHIER_SSCOQM_S_WAY		(0x2 << 17)
#define    UNIPHIER_SSCOQM_CE			(0x1 << 15)	/* notify completion */
#define    UNIPHIER_SSCOQM_CW			(0x1 << 14)
#define    UNIPHIER_SSCOQM_CM_MASK		(0x7)
#define    UNIPHIER_SSCOQM_CM_INV		0x0	/* invalidate */
#define    UNIPHIER_SSCOQM_CM_CLEAN		0x1	/* clean */
#define    UNIPHIER_SSCOQM_CM_FLUSH		0x2	/* flush */
#define    UNIPHIER_SSCOQM_CM_PREFETCH		0x3	/* prefetch to cache */
#define    UNIPHIER_SSCOQM_CM_PREFETCH_BUF	0x4	/* prefetch to pf-buf */
#define    UNIPHIER_SSCOQM_CM_TOUCH		0x5	/* touch */
#define    UNIPHIER_SSCOQM_CM_TOUCH_ZERO	0x6	/* touch to zero */
#define    UNIPHIER_SSCOQM_CM_TOUCH_DIRTY	0x7	/* touch with dirty */
#define UNIPHIER_SSCOQAD	0x506c024c	/* Cache Operation Queue Address */
#define UNIPHIER_SSCOQSZ	0x506c0250	/* Cache Operation Queue Size */
#define UNIPHIER_SSCOQMASK	0x506c0254	/* Cache Operation Queue Address Mask */
#define UNIPHIER_SSCOQWN	0x506c0258	/* Cache Operation Queue Way Number */
#define UNIPHIER_SSCOPPQSEF	0x506c025c	/* Cache Operation Queue Set Complete */
#define    UNIPHIER_SSCOPPQSEF_FE		(0x1 << 1)
#define    UNIPHIER_SSCOPPQSEF_OE		(0x1 << 0)
#define UNIPHIER_SSCOLPQS	0x506c0260	/* Cache Operation Queue Status */
#define    UNIPHIER_SSCOLPQS_EF			(0x1 << 2)
#define    UNIPHIER_SSCOLPQS_EST		(0x1 << 1)
#define    UNIPHIER_SSCOLPQS_QST		(0x1 << 0)

#define UNIPHIER_SSC_LINE_SIZE		128
#define UNIPHIER_SSC_RANGE_OP_MAX_SIZE	(0x00400000 - (UNIPHIER_SSC_LINE_SIZE))

#define UNIPHIER_SSCOQAD_IS_NEEDED(op) \
		((op & UNIPHIER_SSCOQM_S_MASK) == UNIPHIER_SSCOQM_S_RANGE)
#define UNIPHIER_SSCOQWM_IS_NEEDED(op) \
		(((op & UNIPHIER_SSCOQM_S_MASK) == UNIPHIER_SSCOQM_S_WAY) || \
		 ((op & UNIPHIER_SSCOQM_TID_MASK) == UNIPHIER_SSCOQM_TID_WAY))

/* uniphier_cache_sync - perform a sync point for a particular cache level */
static void uniphier_cache_sync(void)
{
	/* drain internal buffers */
	writel(UNIPHIER_SSCOPE_CM_SYNC, UNIPHIER_SSCOPE);
	/* need a read back to confirm */
	readl(UNIPHIER_SSCOPE);
}

/**
 * uniphier_cache_maint_common - run a queue operation
 *
 * @start: start address of range operation (don't care for "all" operation)
 * @size: data size of range operation (don't care for "all" operation)
 * @ways: target ways (don't care for operations other than pre-fetch, touch
 * @operation: flags to specify the desired cache operation
 */
static void uniphier_cache_maint_common(u32 start, u32 size, u32 ways,
					u32 operation)
{
	/* clear the complete notification flag */
	writel(UNIPHIER_SSCOLPQS_EF, UNIPHIER_SSCOLPQS);

	do {
		/* set cache operation */
		writel(UNIPHIER_SSCOQM_CE | operation, UNIPHIER_SSCOQM);

		/* set address range if needed */
		if (likely(UNIPHIER_SSCOQAD_IS_NEEDED(operation))) {
			writel(start, UNIPHIER_SSCOQAD);
			writel(size, UNIPHIER_SSCOQSZ);
		}

		/* set target ways if needed */
		if (unlikely(UNIPHIER_SSCOQWM_IS_NEEDED(operation)))
			writel(ways, UNIPHIER_SSCOQWN);
	} while (unlikely(readl(UNIPHIER_SSCOPPQSEF) &
			  (UNIPHIER_SSCOPPQSEF_FE | UNIPHIER_SSCOPPQSEF_OE)));

	/* wait until the operation is completed */
	while (likely(readl(UNIPHIER_SSCOLPQS) != UNIPHIER_SSCOLPQS_EF))
		cpu_relax();
}

static void uniphier_cache_maint_all(u32 operation)
{
	uniphier_cache_maint_common(0, 0, 0, UNIPHIER_SSCOQM_S_ALL | operation);

	uniphier_cache_sync();
}

static void uniphier_cache_maint_range(u32 start, u32 end, u32 ways,
				       u32 operation)
{
	u32 size;

	/*
	 * If the start address is not aligned,
	 * perform a cache operation for the first cache-line
	 */
	start = start & ~(UNIPHIER_SSC_LINE_SIZE - 1);

	size = end - start;

	if (unlikely(size >= (u32)(-UNIPHIER_SSC_LINE_SIZE))) {
		/* this means cache operation for all range */
		uniphier_cache_maint_all(operation);
		return;
	}

	/*
	 * If the end address is not aligned,
	 * perform a cache operation for the last cache-line
	 */
	size = ALIGN(size, UNIPHIER_SSC_LINE_SIZE);

	while (size) {
		u32 chunk_size = min_t(u32, size, UNIPHIER_SSC_RANGE_OP_MAX_SIZE);

		uniphier_cache_maint_common(start, chunk_size, ways,
					    UNIPHIER_SSCOQM_S_RANGE | operation);

		start += chunk_size;
		size -= chunk_size;
	}

	uniphier_cache_sync();
}

void uniphier_cache_prefetch_range(u32 start, u32 end, u32 ways)
{
	uniphier_cache_maint_range(start, end, ways,
				   UNIPHIER_SSCOQM_TID_WAY |
				   UNIPHIER_SSCOQM_CM_PREFETCH);
}

void uniphier_cache_touch_range(u32 start, u32 end, u32 ways)
{
	uniphier_cache_maint_range(start, end, ways,
				   UNIPHIER_SSCOQM_TID_WAY |
				   UNIPHIER_SSCOQM_CM_TOUCH);
}

void uniphier_cache_touch_zero_range(u32 start, u32 end, u32 ways)
{
	uniphier_cache_maint_range(start, end, ways,
				   UNIPHIER_SSCOQM_TID_WAY |
				   UNIPHIER_SSCOQM_CM_TOUCH_ZERO);
}

void uniphier_cache_inv_way(u32 ways)
{
	uniphier_cache_maint_common(0, 0, ways,
				    UNIPHIER_SSCOQM_S_WAY |
				    UNIPHIER_SSCOQM_CM_INV);
}

void uniphier_cache_set_active_ways(int cpu, u32 active_ways)
{
	void __iomem *base = (void __iomem *)UNIPHIER_SSCC + 0xc00;

	switch (readl(UNIPHIER_SSCID)) { /* revision */
	case 0x12:	/* LD4 */
	case 0x16:	/* sld8 */
		base = (void __iomem *)UNIPHIER_SSCC + 0x840;
		break;
	default:
		base = (void __iomem *)UNIPHIER_SSCC + 0xc00;
		break;
	}

	writel(active_ways, base + 4 * cpu);
}

static void uniphier_cache_endisable(int enable)
{
	u32 tmp;

	tmp = readl(UNIPHIER_SSCC);
	if (enable)
		tmp |= UNIPHIER_SSCC_ON;
	else
		tmp &= ~UNIPHIER_SSCC_ON;
	writel(tmp, UNIPHIER_SSCC);
}

void uniphier_cache_enable(void)
{
	uniphier_cache_endisable(1);
}

void uniphier_cache_disable(void)
{
	uniphier_cache_endisable(0);
}

#ifdef CONFIG_CACHE_UNIPHIER
void v7_outer_cache_flush_all(void)
{
	uniphier_cache_maint_all(UNIPHIER_SSCOQM_CM_FLUSH);
}

void v7_outer_cache_inval_all(void)
{
	uniphier_cache_maint_all(UNIPHIER_SSCOQM_CM_INV);
}

void v7_outer_cache_flush_range(u32 start, u32 end)
{
	uniphier_cache_maint_range(start, end, 0, UNIPHIER_SSCOQM_CM_FLUSH);
}

void v7_outer_cache_inval_range(u32 start, u32 end)
{
	if (start & (UNIPHIER_SSC_LINE_SIZE - 1)) {
		start &= ~(UNIPHIER_SSC_LINE_SIZE - 1);
		uniphier_cache_maint_range(start, UNIPHIER_SSC_LINE_SIZE, 0,
					   UNIPHIER_SSCOQM_CM_FLUSH);
		start += UNIPHIER_SSC_LINE_SIZE;
	}

	if (start >= end) {
		uniphier_cache_sync();
		return;
	}

	if (end & (UNIPHIER_SSC_LINE_SIZE - 1)) {
		end &= ~(UNIPHIER_SSC_LINE_SIZE - 1);
		uniphier_cache_maint_range(end, UNIPHIER_SSC_LINE_SIZE, 0,
					   UNIPHIER_SSCOQM_CM_FLUSH);
	}

	if (start >= end) {
		uniphier_cache_sync();
		return;
	}

	uniphier_cache_maint_range(start, end, 0, UNIPHIER_SSCOQM_CM_INV);
}

void v7_outer_cache_enable(void)
{
	uniphier_cache_set_active_ways(0, U32_MAX);	/* activate all ways */
	uniphier_cache_enable();
}

void v7_outer_cache_disable(void)
{
	uniphier_cache_disable();
}
#endif

void enable_caches(void)
{
	dcache_enable();
}
