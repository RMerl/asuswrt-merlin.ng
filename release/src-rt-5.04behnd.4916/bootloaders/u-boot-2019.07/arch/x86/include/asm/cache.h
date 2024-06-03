/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __X86_CACHE_H__
#define __X86_CACHE_H__

/*
 * If CONFIG_SYS_CACHELINE_SIZE is defined use it for DMA alignment.  Otherwise
 * use 64-bytes, a safe default for x86.
 */
#ifndef CONFIG_SYS_CACHELINE_SIZE
#define CONFIG_SYS_CACHELINE_SIZE	64
#endif

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
