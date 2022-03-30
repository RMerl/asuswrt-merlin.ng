/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Tensilica Inc.
 */
#ifndef _XTENSA_CACHE_H
#define _XTENSA_CACHE_H

#include <asm/arch/core.h>

#define ARCH_DMA_MINALIGN	XCHAL_DCACHE_LINESIZE

#ifndef __ASSEMBLY__

void __flush_dcache_all(void);
void __flush_invalidate_dcache_range(unsigned long addr, unsigned long size);
void __invalidate_dcache_all(void);
void __invalidate_dcache_range(unsigned long addr, unsigned long size);

void __invalidate_icache_all(void);
void __invalidate_icache_range(unsigned long addr, unsigned long size);

#endif

#endif	/* _XTENSA_CACHE_H */
