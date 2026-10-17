/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __X86_CACHE_H__
#define __X86_CACHE_H__

/*
 * Use CONFIG_SYS_CACHELINE_SIZE (which is set to 64-bytes) for DMA alignment.
 */
#define ARCH_DMA_MINALIGN		CONFIG_SYS_CACHELINE_SIZE

static inline void wbinvd(void)
{
	asm volatile ("wbinvd" : : : "memory");
}

static inline void invd(void)
{
	asm volatile("invd" : : : "memory");
}

/* Enable caches and write buffer */
void enable_caches(void);

/* Disable caches and write buffer */
void disable_caches(void);

#endif /* __X86_CACHE_H__ */
