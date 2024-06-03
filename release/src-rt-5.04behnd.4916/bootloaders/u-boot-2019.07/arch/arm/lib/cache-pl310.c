// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 * Aneesh V <aneesh@ti.com>
 */
#include <linux/types.h>
#include <asm/io.h>
#include <asm/armv7.h>
#include <asm/pl310.h>
#include <config.h>
#include <common.h>

struct pl310_regs *const pl310 = (struct pl310_regs *)CONFIG_SYS_PL310_BASE;

static void pl310_cache_sync(void)
{
	writel(0, &pl310->pl310_cache_sync);
}

static void pl310_background_op_all_ways(u32 *op_reg)
{
	u32 assoc_16, associativity, way_mask;

	assoc_16 = readl(&pl310->pl310_aux_ctrl) &
			PL310_AUX_CTRL_ASSOCIATIVITY_MASK;
	if (assoc_16)
		associativity = 16;
	else
		associativity = 8;

	way_mask = (1 << associativity) - 1;
	/* Invalidate all ways */
	writel(way_mask, op_reg);
	/* Wait for all ways to be invalidated */
	while (readl(op_reg) & way_mask)
		;
	pl310_cache_sync();
}

void v7_outer_cache_inval_all(void)
{
	pl310_background_op_all_ways(&pl310->pl310_inv_way);
}

void v7_outer_cache_flush_all(void)
{
	pl310_background_op_all_ways(&pl310->pl310_clean_inv_way);
}

/* Flush(clean invalidate) memory from start to stop-1 */
void v7_outer_cache_flush_range(u32 start, u32 stop)
{
	/* PL310 currently supports only 32 bytes cache line */
	u32 pa, line_size = 32;

	/*
	 * Align to the beginning of cache-line - this ensures that
	 * the first 5 bits are 0 as required by PL310 TRM
	 */
	start &= ~(line_size - 1);

	for (pa = start; pa < stop; pa = pa + line_size)
		writel(pa, &pl310->pl310_clean_inv_line_pa);

	pl310_cache_sync();
}

/* invalidate memory from start to stop-1 */
void v7_outer_cache_inval_range(u32 start, u32 stop)
{
	/* PL310 currently supports only 32 bytes cache line */
	u32 pa, line_size = 32;

	/*
	 * If start address is not aligned to cache-line do not
	 * invalidate the first cache-line
	 */
	if (start & (line_size - 1)) {
		printf("ERROR: %s - start address is not aligned - 0x%08x\n",
			__func__, start);
		/* move to next cache line */
		start = (start + line_size - 1) & ~(line_size - 1);
	}

	/*
	 * If stop address is not aligned to cache-line do not
	 * invalidate the last cache-line
	 */
	if (stop & (line_size - 1)) {
		printf("ERROR: %s - stop address is not aligned - 0x%08x\n",
			__func__, stop);
		/* align to the beginning of this cache line */
		stop &= ~(line_size - 1);
	}

	for (pa = start; pa < stop; pa = pa + line_size)
		writel(pa, &pl310->pl310_inv_line_pa);

	pl310_cache_sync();
}
