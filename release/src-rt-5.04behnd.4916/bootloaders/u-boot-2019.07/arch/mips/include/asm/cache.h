/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __MIPS_CACHE_H__
#define __MIPS_CACHE_H__

#define L1_CACHE_SHIFT		CONFIG_MIPS_L1_CACHE_SHIFT
#define L1_CACHE_BYTES		(1 << L1_CACHE_SHIFT)

#define ARCH_DMA_MINALIGN	(L1_CACHE_BYTES)

/*
 * CONFIG_SYS_CACHELINE_SIZE is still used in various drivers primarily for
 * DMA buffer alignment. Satisfy those drivers by providing it as a synonym
 * of ARCH_DMA_MINALIGN for now.
 */
#define CONFIG_SYS_CACHELINE_SIZE ARCH_DMA_MINALIGN

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
