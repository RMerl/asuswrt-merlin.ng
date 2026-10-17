/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2018 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __ASM_RISCV_DMA_MAPPING_H
#define __ASM_RISCV_DMA_MAPPING_H

#include <linux/types.h>
#include <asm/cache.h>
#include <cpu_func.h>
#include <linux/dma-direction.h>
#include <malloc.h>

static inline void *dma_alloc_coherent(size_t len, unsigned long *handle)
{
	*handle = (unsigned long)memalign(ARCH_DMA_MINALIGN, len);
	return (void *)*handle;
}

static inline void dma_free_coherent(void *addr)
{
	free(addr);
}

#endif /* __ASM_RISCV_DMA_MAPPING_H */
