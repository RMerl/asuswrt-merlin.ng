/*
    <:copyright-BRCM:2014-2016:DUAL/GPL:standard
    
       Copyright (c) 2014-2016 Broadcom 
       All Rights Reserved
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>
*/

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains inline functions for runner host memory management      */
/*                                                                            */
/******************************************************************************/
#ifndef _RDP_MM_H_
#define _RDP_MM_H_
#include <common.h>
#include "bdmf_data_types.h"

#define VIRT_TO_PHYS(_addr)		((uintptr_t)_addr)
#define PHYS_TO_CACHED(_addr)		((void *)((uintptr_t)_addr))
#define PHYS_TO_UNCACHED(_addr)		((void *)((uintptr_t)_addr))

#define CACHED_FREE(_ptr)		free(_ptr)

#define KMALLOC(_size, _align)		memalign(_align, _size)
#define KFREE(_ptr)			free(_ptr)

#if defined(CONFIG_BCM6858) || defined(CONFIG_BCM63146) || \
    defined(CONFIG_BCM6846) || defined(CONFIG_BCM6856) || \
    defined(CONFIG_BCM6878)
#define DMA_CACHE_LINE 64
#else
#define DMA_CACHE_LINE 32
#endif

#define FLUSH_RANGE(s,l) ({ \
	unsigned long start, end;	\
	start = ((unsigned long)(s))&~(DMA_CACHE_LINE-1); \
	end = (((unsigned long)(s)+(l)) + \
		DMA_CACHE_LINE - 1)&~(DMA_CACHE_LINE-1); \
	flush_dcache_range(start, end);	})

#define INV_RANGE(s,l) ({ \
	unsigned long start, end;	\
	start = ((unsigned long)(s))&~(DMA_CACHE_LINE-1); \
	end = (((unsigned long)(s)+(l)) + \
		DMA_CACHE_LINE - 1)&~(DMA_CACHE_LINE-1); \
	invalidate_dcache_range(start, end);	})

#define BCM_PKTBUF_SIZE 2048

#define rdp_mm_aligned_alloc(_size, _phy_addr_p) \
	__rdp_mm_aligned_alloc((_size), (_phy_addr_p), GFP_DMA)
#define rdp_mm_aligned_alloc_atomic(_size, _phy_addr_p) \
	__rdp_mm_aligned_alloc((_size), (_phy_addr_p), GFP_ATOMIC)

static inline void *__rdp_mm_aligned_alloc(uint32_t size,
					   bdmf_phys_addr_t *phy_addr_p,
					   gfp_t gfp)
{
	dma_addr_t phy_addr;
	uint32_t size_padded_aligned;
	dma_addr_t *mem;

	/* must be multiple of pointer size */
	size_padded_aligned = (size + (sizeof(dma_addr_t) << 1) - 1) &
			~(sizeof(dma_addr_t) - 1);
	mem = (dma_addr_t *)consistent_alloc(gfp, size_padded_aligned, &phy_addr);
	if (unlikely(mem == NULL))
		return NULL;

	mem[(size_padded_aligned / sizeof(dma_addr_t)) - 1] = phy_addr;
	*phy_addr_p = (bdmf_phys_addr_t)phy_addr;
	return (void *)mem;
}

static inline void rdp_mm_aligned_free(void *ptr, uint32_t size)
{
	uint32_t size_padded_aligned;
	dma_addr_t *mem = ptr, phy_addr;

	size_padded_aligned = (size + (sizeof(dma_addr_t) << 1) - 1) &
			~(sizeof(dma_addr_t) - 1);
	phy_addr = mem[(size_padded_aligned / sizeof(dma_addr_t)) - 1];
	consistent_free(ptr, size_padded_aligned, phy_addr);
}

static inline void rdp_mm_setl_context(void *__to, unsigned int __val,
				       unsigned int __n)
{
	volatile unsigned int *dst = (volatile unsigned int *)__to;
	int i;

	for (i = 0; i < (__n / 4); i++, dst++) {
		if ((i & 0x3) == 3)
			continue;
		*dst = __val; /* DSL */
	}
}

static inline void rdp_mm_setl(void *__to, unsigned int __val, unsigned int __n)
{
	volatile unsigned int *dst = (volatile unsigned int *)__to;
	int i;

	for (i = 0; i < (__n / 4); i++, dst++)
		*dst = __val; /* DSL */
}

static inline void rdp_mm_cpyl_context(void *__to, void *__from,
				       unsigned int __n)
{
	volatile unsigned int * src = (unsigned int *)__from;
	volatile unsigned int * dst = (unsigned int *)__to;
	int i, n = __n / 4;

	for (i = 0; i < n; i++, src++, dst++) {
		if ((i & 0x3) == 3)
			continue;

		*dst = swap4bytes(*src);
	}
}

#endif
