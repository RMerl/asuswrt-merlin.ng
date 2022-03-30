// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014 Google, Inc
 *
 * From Coreboot src/lib/ramtest.c
 */

#include <common.h>
#include <asm/io.h>
#include <asm/post.h>

static void write_phys(unsigned long addr, u32 value)
{
#if CONFIG_SSE2
	asm volatile(
		"movnti %1, (%0)"
		: /* outputs */
		: "r" (addr), "r" (value) /* inputs */
		: /* clobbers */
		);
#else
	writel(value, addr);
#endif
}

static u32 read_phys(unsigned long addr)
{
	return readl(addr);
}

static void phys_memory_barrier(void)
{
#if CONFIG_SSE2
	/* Needed for movnti */
	asm volatile(
		"sfence"
		:
		:
		: "memory"
	);
#else
	asm volatile(""
		:
		:
		: "memory");
#endif
}

void quick_ram_check(void)
{
	int fail = 0;
	u32 backup;

	backup = read_phys(CONFIG_RAMBASE);
	write_phys(CONFIG_RAMBASE, 0x55555555);
	phys_memory_barrier();
	if (read_phys(CONFIG_RAMBASE) != 0x55555555)
		fail = 1;
	write_phys(CONFIG_RAMBASE, 0xaaaaaaaa);
	phys_memory_barrier();
	if (read_phys(CONFIG_RAMBASE) != 0xaaaaaaaa)
		fail = 1;
	write_phys(CONFIG_RAMBASE, 0x00000000);
	phys_memory_barrier();
	if (read_phys(CONFIG_RAMBASE) != 0x00000000)
		fail = 1;
	write_phys(CONFIG_RAMBASE, 0xffffffff);
	phys_memory_barrier();
	if (read_phys(CONFIG_RAMBASE) != 0xffffffff)
		fail = 1;

	write_phys(CONFIG_RAMBASE, backup);
	if (fail) {
		post_code(POST_RAM_FAILURE);
		panic("RAM INIT FAILURE!\n");
	}
	phys_memory_barrier();
}
