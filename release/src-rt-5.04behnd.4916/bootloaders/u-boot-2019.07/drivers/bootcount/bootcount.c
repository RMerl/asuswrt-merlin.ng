// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010-2012
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 */

#include <bootcount.h>
#include <linux/compiler.h>

/* Now implement the generic default functions */
__weak void bootcount_store(ulong a)
{
	void *reg = (void *)CONFIG_SYS_BOOTCOUNT_ADDR;
	uintptr_t flush_start = rounddown(CONFIG_SYS_BOOTCOUNT_ADDR,
					  CONFIG_SYS_CACHELINE_SIZE);
	uintptr_t flush_end;

#if defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD)
	raw_bootcount_store(reg, (CONFIG_SYS_BOOTCOUNT_MAGIC & 0xffff0000) | a);

	flush_end = roundup(CONFIG_SYS_BOOTCOUNT_ADDR + 4,
			    CONFIG_SYS_CACHELINE_SIZE);
#else
	raw_bootcount_store(reg, a);
	raw_bootcount_store(reg + 4, CONFIG_SYS_BOOTCOUNT_MAGIC);

	flush_end = roundup(CONFIG_SYS_BOOTCOUNT_ADDR + 8,
			    CONFIG_SYS_CACHELINE_SIZE);
#endif /* defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD */
	flush_dcache_range(flush_start, flush_end);
}

__weak ulong bootcount_load(void)
{
	void *reg = (void *)CONFIG_SYS_BOOTCOUNT_ADDR;

#if defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD)
	u32 tmp = raw_bootcount_load(reg);

	if ((tmp & 0xffff0000) != (CONFIG_SYS_BOOTCOUNT_MAGIC & 0xffff0000))
		return 0;
	else
		return (tmp & 0x0000ffff);
#else
	if (raw_bootcount_load(reg + 4) != CONFIG_SYS_BOOTCOUNT_MAGIC)
		return 0;
	else
		return raw_bootcount_load(reg);
#endif /* defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD) */
}
