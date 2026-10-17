/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */
#ifndef __ASM_ARM_DMA_MAPPING_H
#define __ASM_ARM_DMA_MAPPING_H

#include <asm/cache.h>
#include <cpu_func.h>
#include <linux/dma-direction.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <malloc.h>

static inline void *dma_alloc_coherent(size_t len, unsigned long *handle)
{
	*handle = (unsigned long)memalign(ARCH_DMA_MINALIGN, ROUND(len, ARCH_DMA_MINALIGN));
	return (void *)*handle;
}

static inline void dma_free_coherent(void *addr)
{
	free(addr);
}

#endif /* __ASM_ARM_DMA_MAPPING_H */
