/*
    <:copyright-BRCM:2014-2016:DUAL/GPL:standard
    
       Copyright (c) 2014-2016 Broadcom 
       All Rights Reserved
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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
#include "bdmf_data_types.h"

#define rdp_mm_aligned_alloc(_size, _phy_addr_p) __rdp_mm_aligned_alloc((_size), (_phy_addr_p), GFP_DMA)
#define rdp_mm_aligned_alloc_atomic(_size, _phy_addr_p) __rdp_mm_aligned_alloc((_size), (_phy_addr_p), GFP_ATOMIC)

#if defined(__UBOOT__)
#undef rdp_mm_aligned_alloc
#undef rdp_mm_aligned_alloc_atomic
#define rdp_mm_aligned_alloc(_size, _phy_addr_p) __rdp_mm_aligned_alloc((_size), (_phy_addr_p), 0)
#define rdp_mm_aligned_alloc_atomic(_size, _phy_addr_p) __rdp_mm_aligned_alloc((_size), (_phy_addr_p), 0)
#endif

static inline void *__rdp_mm_aligned_alloc(uint32_t size,
					   bdmf_phys_addr_t * phy_addr_p,
					   gfp_t gfp)
{
#if !defined(__UBOOT__) && defined(__KERNEL__) && (defined(CONFIG_ARM) || defined(CONFIG_ARM64))
	dma_addr_t phy_addr;
	uint32_t size_padded_aligned = (size + (sizeof(dma_addr_t) << 1) - 1) & ~(sizeof(dma_addr_t) - 1);	/* must be multiple of pointer size */
	dma_addr_t *mem;

	if (rdp_dummy_dev == NULL)
		return NULL;

	/* memory allocation of dma_alloc_coherent for ARM is aligned to page size which is aligned to cache */
	mem =
	    (dma_addr_t *) dma_alloc_coherent(rdp_dummy_dev,
					      size_padded_aligned, &phy_addr,
					      gfp);
	if (unlikely(mem == NULL))
		return NULL;
	/*  printk("\n\tsize %u, size32 %u, mem %p, &mem[size] %p, phy_addr 0x%08x\n\n", size, size32, mem, &mem[(size32-sizeof(void *))>>2], phy_addr); */
	mem[(size_padded_aligned / sizeof(dma_addr_t)) - 1] = phy_addr;
	*phy_addr_p = (bdmf_phys_addr_t) phy_addr;
	return (void *)mem;
#else
	/* we are trying to make sure the start of the ring is cache line aligned,
	 * and we need an additional memory to hold (void *) just right before the
	 * start of the ring. So we are allocating 1 more cache line + sizeof(void *) */
	unsigned long cache_line = DMA_CACHE_LINE;
	void *mem =
	    (void *)NONCACHED_MALLOC_ATOMIC(size + cache_line + sizeof(void *));
	void **ptr =
	    (void **)(((uintptr_t) mem + cache_line + (sizeof(void *))) &
		      ~(cache_line - 1));
	ptr[-1] = mem;
	*phy_addr_p = VIRT_TO_PHYS(ptr);

	return (void *)ptr;
#endif
}

static inline void rdp_mm_aligned_free(void *ptr, uint32_t size)
{
#if !defined(__UBOOT__) && defined(__KERNEL__) && (defined(CONFIG_ARM) || defined(CONFIG_ARM64))
	uint32_t size_padded_aligned = (size + (sizeof(void *) << 1) - 1) & ~(sizeof(void *) - 1);	/* must be multiple of pointer size */
	dma_addr_t *mem = ptr;
	dma_addr_t phy_addr =
	    mem[(size_padded_aligned / sizeof(dma_addr_t)) - 1];
	dma_free_coherent(rdp_dummy_dev, size_padded_aligned, ptr, phy_addr);
#else
	NONCACHED_FREE(((void **)ptr)[-1]);
#endif
}

static inline void rdp_mm_setl_context(void *__to, unsigned int __val,
				       unsigned int __n)
{
	volatile unsigned int *dst = (volatile unsigned int *)__to;
	int i;

	for (i = 0; i < (__n / 4); i++, dst++) {
		if ((i & 0x3) == 3)
			continue;

		*dst = __val;	/* DSL */
	}
}

static inline void rdp_mm_setl(void *__to, unsigned int __val, unsigned int __n)
{
	volatile unsigned int *dst = (volatile unsigned int *)__to;
	int i;

	for (i = 0; i < (__n / 4); i++, dst++) {
		*dst = __val;	/* DSL */
	}
}

static inline void rdp_mm_cpyl_context(void *__to, void *__from,
				       unsigned int __n)
{
	volatile unsigned int *src = (unsigned int *)__from;
	volatile unsigned int *dst = (unsigned int *)__to;
	int i, n = __n / 4;

	for (i = 0; i < n; i++, src++, dst++) {
		if ((i & 0x3) == 3)
			continue;

		*dst = swap4bytes(*src);
	}
}

#endif
