/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __MICROBLAZE_CACHE_H__
#define __MICROBLAZE_CACHE_H__

/*
 * The microblaze can have either a 4 or 16 byte cacheline depending on whether
 * you are using OPB(4) or CacheLink(16).  If the board config has not specified
 * a cacheline size we assume the larger value of 16 bytes for DMA buffer
 * alignment.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN	16
#endif

/**
 * flush_cache_all - flush the entire instruction/data caches
 */
void flush_cache_all(void);

#endif /* __MICROBLAZE_CACHE_H__ */
