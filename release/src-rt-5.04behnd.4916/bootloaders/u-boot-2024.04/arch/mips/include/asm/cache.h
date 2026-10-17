/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __MIPS_CACHE_H__
#define __MIPS_CACHE_H__

#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE

#ifndef __ASSEMBLY__
/**
 * mips_cache_probe() - Probe the properties of the caches
 *
 * Call this to probe the properties such as line sizes of the caches
 * present in the system, if any. This must be done before cache maintenance
 * functions such as flush_cache may be called.
 */
void mips_cache_probe(void);
#endif

#endif /* __MIPS_CACHE_H__ */
