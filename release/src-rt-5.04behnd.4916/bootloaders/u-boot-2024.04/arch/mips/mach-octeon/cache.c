// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#include <cpu_func.h>

/* Octeon memory write barrier */
#define CVMX_SYNCW	asm volatile ("syncw\nsyncw\n" : : : "memory")

void flush_dcache_range(ulong start_addr, ulong stop)
{
	/* Flush all pending writes */
	CVMX_SYNCW;
}

void flush_cache(ulong start_addr, ulong size)
{
}

void invalidate_dcache_range(ulong start_addr, ulong stop)
{
	/* Don't need to do anything for OCTEON */
}
