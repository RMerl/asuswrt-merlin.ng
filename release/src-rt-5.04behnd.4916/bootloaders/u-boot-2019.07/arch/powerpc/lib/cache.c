// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/cache.h>
#include <watchdog.h>

void flush_cache(ulong start_addr, ulong size)
{
#ifndef CONFIG_5xx
	ulong addr, start, end;

	start = start_addr & ~(CONFIG_SYS_CACHELINE_SIZE - 1);
	end = start_addr + size - 1;

	for (addr = start; (addr <= end) && (addr >= start);
			addr += CONFIG_SYS_CACHELINE_SIZE) {
		asm volatile("dcbst 0,%0" : : "r" (addr) : "memory");
		WATCHDOG_RESET();
	}
	/* wait for all dcbst to complete on bus */
	asm volatile("sync" : : : "memory");

	for (addr = start; (addr <= end) && (addr >= start);
			addr += CONFIG_SYS_CACHELINE_SIZE) {
		asm volatile("icbi 0,%0" : : "r" (addr) : "memory");
		WATCHDOG_RESET();
	}
	asm volatile("sync" : : : "memory");
	/* flush prefetch queue */
	asm volatile("isync" : : : "memory");
#endif
}
