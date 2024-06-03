// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2012 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 */

#include <common.h>

int arch_cpu_init(void)
{
	icache_enable();

	return 0;
}

#ifdef CONFIG_ARMV7_LPAE
void enable_caches(void)
{
	dcache_enable();
}
#endif
