// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

const ulong patterns[]      = {	0x00000000,
				0xFFFFFFFF,
				0xFF00FF00,
				0x0F0F0F0F,
				0xF0F0F0F0};
const ulong NBR_OF_PATTERNS = sizeof(patterns) / sizeof(*patterns);
const ulong OFFS_PATTERN    = 3;
const ulong REPEAT_PATTERN  = 1000;

void bootcount_store(ulong a)
{
	ulong *save_addr;
	ulong size = 0;
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		size += gd->bd->bi_dram[i].size;
	save_addr = (ulong *)(size - BOOTCOUNT_ADDR);
	writel(a, save_addr);
	writel(CONFIG_SYS_BOOTCOUNT_MAGIC, &save_addr[1]);

	for (i = 0; i < REPEAT_PATTERN; i++)
		writel(patterns[i % NBR_OF_PATTERNS],
			&save_addr[i + OFFS_PATTERN]);

	/* Make sure the data is written to RAM */
	flush_dcache_range((ulong)&save_addr[0],
			   (((ulong)&save_addr[REPEAT_PATTERN + OFFS_PATTERN] &
			     ~(ARCH_DMA_MINALIGN - 1)) + ARCH_DMA_MINALIGN));
}

ulong bootcount_load(void)
{
	ulong *save_addr;
	ulong size = 0;
	ulong counter = 0;
	int i, tmp;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		size += gd->bd->bi_dram[i].size;
	save_addr = (ulong *)(size - BOOTCOUNT_ADDR);

	counter = readl(&save_addr[0]);

	/* Is the counter reliable, check in the big pattern for bit errors */
	for (i = 0; (i < REPEAT_PATTERN) && (counter != 0); i++) {
		tmp = readl(&save_addr[i + OFFS_PATTERN]);
		if (tmp != patterns[i % NBR_OF_PATTERNS])
			counter = 0;
	}
	return counter;
}
