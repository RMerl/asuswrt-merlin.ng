/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Andes Technology Corporation
 * Ken Kuo, Andes Technology Corporation <ken_kuo@andestech.com>
 */
#ifndef __ASM_NDS_DMA_MAPPING_H
#define __ASM_NDS_DMA_MAPPING_H

#include <linux/dma-direction.h>

static void *dma_alloc_coherent(size_t len, unsigned long *handle)
{
	*handle = (unsigned long)memalign(ARCH_DMA_MINALIGN, len);
	return (void *)*handle;
}

static inline unsigned long dma_map_single(volatile void *vaddr, size_t len,
					   enum dma_data_direction dir)
{
	return (unsigned long)vaddr;
}

static inline void dma_unmap_single(volatile void *vaddr, size_t len,
				    unsigned long paddr)
{
}

#endif /* __ASM_NDS_DMA_MAPPING_H */
