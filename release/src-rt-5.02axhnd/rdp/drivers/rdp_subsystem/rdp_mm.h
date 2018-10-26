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
#include "bdmf_data_types.h"

#if defined(__KERNEL__) && (defined(CONFIG_ARM) || defined(CONFIG_ARM64))
#include <linux/device.h>

extern struct device *rdp_dummy_dev;
#endif


#if !defined(RDP_SIM)
#define rdp_mm_aligned_alloc(_size, _phy_addr_p) __rdp_mm_aligned_alloc((_size), (_phy_addr_p), GFP_DMA)
#define rdp_mm_aligned_alloc_atomic(_size, _phy_addr_p) __rdp_mm_aligned_alloc((_size), (_phy_addr_p), GFP_ATOMIC)

#if defined(_CFE_)
typedef int gfp_t;
#undef rdp_mm_aligned_alloc
#undef rdp_mm_aligned_alloc_atomic
#define rdp_mm_aligned_alloc(_size, _phy_addr_p) __rdp_mm_aligned_alloc((_size), (_phy_addr_p), 0)
#define rdp_mm_aligned_alloc_atomic(_size, _phy_addr_p) __rdp_mm_aligned_alloc((_size), (_phy_addr_p), 0)
#endif

static inline void *__rdp_mm_aligned_alloc(uint32_t size, bdmf_phys_addr_t *phy_addr_p, gfp_t gfp)
{
#if defined(__KERNEL__) && (defined(CONFIG_ARM) || defined(CONFIG_ARM64))
    dma_addr_t phy_addr;
    uint32_t size_padded_aligned = (size + (sizeof(dma_addr_t) << 1) - 1) & ~(sizeof(dma_addr_t) - 1); /* must be multiple of pointer size */
    dma_addr_t *mem;

    if (rdp_dummy_dev == NULL)
        return NULL;

    /* memory allocation of dma_alloc_coherent for ARM is aligned to page size which is aligned to cache */
    mem = (dma_addr_t *)dma_alloc_coherent(rdp_dummy_dev, size_padded_aligned, &phy_addr, gfp);
    if (unlikely(mem == NULL))
        return NULL;
    /*	printk("\n\tsize %u, size32 %u, mem %p, &mem[size] %p, phy_addr 0x%08x\n\n", size, size32, mem, &mem[(size32-sizeof(void *))>>2], phy_addr); */
    mem[(size_padded_aligned / sizeof(dma_addr_t)) - 1] = phy_addr;
    *phy_addr_p = (bdmf_phys_addr_t)phy_addr;
    return (void *)mem;
#else
    /* we are trying to make sure the start of the ring is cache line aligned,
     * and we need an additional memory to hold (void *) just right before the
     * start of the ring. So we are allocating 1 more cache line + sizeof(void *) */
    unsigned long cache_line = DMA_CACHE_LINE;
    void *mem = (void *)CACHED_MALLOC_ATOMIC(size + cache_line + sizeof(void *));
    void **ptr = (void **)(((uintptr_t)mem + cache_line + (sizeof(void *))) & ~(cache_line - 1));
    ptr[-1] = mem;
    *phy_addr_p = VIRT_TO_PHYS(ptr);
    INV_RANGE((uintptr_t)ptr, size);
    return (void *)CACHE_TO_NONCACHE(ptr);
#endif
}

static inline void rdp_mm_aligned_free(void *ptr, uint32_t size)
{
#if defined(__KERNEL__) && (defined(CONFIG_ARM) || defined(CONFIG_ARM64))
    uint32_t size_padded_aligned = (size + (sizeof(void *) << 1) - 1) & ~(sizeof(void *) - 1); /* must be multiple of pointer size */
    dma_addr_t *mem = ptr;
    dma_addr_t phy_addr =  mem[(size_padded_aligned / sizeof(dma_addr_t)) - 1];
    dma_free_coherent(rdp_dummy_dev, size_padded_aligned, ptr, phy_addr);
#else
    CACHED_FREE(((void**)ptr)[-1]);
#endif
}

#else /* for simulator */

#define rdp_mm_aligned_alloc(_size, _phy_addr_p) __rdp_mm_aligned_alloc((_size), (_phy_addr_p))
#define rdp_mm_aligned_alloc_atomic(_size, _phy_addr_p) __rdp_mm_aligned_alloc((_size), (_phy_addr_p))
static inline void *__rdp_mm_aligned_alloc(uint32_t size, bdmf_phys_addr_t *phy_addr_p)
{
#ifdef XRDP
    void *mem;
    void **ptr;
    uint32_t size_padded_aligned;

    size_padded_aligned = (2 * size) + sizeof(void *);
    mem = bdmf_alloc_rsv(size_padded_aligned, phy_addr_p);
    if (!mem)
        return NULL;
    *phy_addr_p += (size + sizeof(void *));
    *phy_addr_p &= ~(size - 1);
    ptr = rsv_phys_to_virt(*phy_addr_p);
    ptr[-1] = mem;
    return ptr;
#else
    void *phys_addr = malloc(size);

    *phy_addr_p = (bdmf_phys_addr_t)phys_addr;
    return phys_addr;
#endif
}

static inline void rdp_mm_aligned_free(void *ptr, uint32_t size)
{
#ifdef XRDP
/* in case of simulator we don't free the memory because we don't manage shared memory segments */
#ifdef XRDP_EMULATION
    bdmf_free(((void**)ptr)[-1]);
#endif
#else
    free(ptr);
#endif
}
#endif

static inline void rdp_mm_setl_context(void *__to, unsigned int __val, unsigned int __n)
{
    volatile unsigned int *dst = (volatile unsigned int *)__to;
    int i;

    for (i = 0; i < (__n / 4); i++, dst++)
    {
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
    {
        *dst = __val; /* DSL */
    }
}

static inline void rdp_mm_cpyl_context(void *__to, void *__from, unsigned int __n)
{
    volatile unsigned int * src = (unsigned int *)__from;
    volatile unsigned int * dst = (unsigned int *)__to;
    int i, n = __n / 4;

    for (i = 0; i < n; i++, src++, dst++)
    {
        if ((i & 0x3) == 3)
            continue;

        *dst = swap4bytes(*src);
    }
}

#endif
