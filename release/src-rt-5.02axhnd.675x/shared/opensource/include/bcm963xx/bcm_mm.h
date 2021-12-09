/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard

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
/* This file contains the Broadcom OS/CPU independent Memory Management API   */
/*                                                                            */
/******************************************************************************/

#ifndef _BCM_MM_H_
#define _BCM_MM_H_


#if defined(_CFE_)

#include "lib_malloc.h"
#include "cfe_iocb.h"

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)

#include "addrspace.h"

/* For CFE boot loader, cached addresses are mapped to physical addresses. Uncached addresses or OR'd with 0x40000000. */
#define CACHE_TO_NONCACHE(x)    			cache_to_uncache(((uintptr_t)(x)))
#define NONCACHE_TO_CACHE(x)    			uncache_to_cache(((uintptr_t)(x)))
#define CACHED_MALLOC_ATOMIC(_size) 			uncache_to_cache((uintptr_t)KMALLOC((_size),0))
#define CACHED_MALLOC(_size) 				uncache_to_cache((uintptr_t)KMALLOC((_size),0))
#define NONCACHED_MALLOC_ATOMIC(_size) 			cache_to_uncache((uintptr_t)KMALLOC((_size),0))
#define NONCACHED_MALLOC(_size) 			cache_to_uncache((uintptr_t)KMALLOC((_size),0))
#define NONCACHED_FREE(_ptr) 				KFREE(uncache_to_cache(_ptr))
#define CACHED_FREE(_ptr)				KFREE(_ptr)
#define VIRT_TO_PHYS(_addr)				uncache_to_cache((uintptr_t)_addr)
#define PHYS_TO_CACHED(_addr)				((uintptr_t)_addr)
#define PHYS_TO_UNCACHED(_addr)				cache_to_uncache((uintptr_t)_addr)
#if defined(BCM6858) || defined(CONFIG_BCM963148) || \
    defined(CONFIG_BCM96846) || defined(BCM6846) ||\
    defined(CONFIG_BCM96856) || defined(BCM6856) || defined(CONFIG_BCM96878) || defined(BCM6878)
#define DMA_CACHE_LINE					64
#else
#define DMA_CACHE_LINE					32
#endif
extern void _cfe_flushcache(int, uint8_t *, uint8_t *);
#if defined(BCM63138)
extern void l2cache_invalid_range(unsigned long start, unsigned long end);
extern void l2cache_clean_invalid_range(unsigned long start, unsigned long end);
#define FLUSH_RANGE(s,l) 				_cfe_flushcache(CFE_CACHE_FLUSH_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l)); l2cache_clean_invalid_range((unsigned long)s, (unsigned long)(s+l))
#define INV_RANGE(s,l) 					_cfe_flushcache(CFE_CACHE_INVAL_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l)); l2cache_invalid_range((unsigned long)s, (unsigned long)(s+l))
#else
#define FLUSH_RANGE(s,l) 				_cfe_flushcache(CFE_CACHE_FLUSH_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))
#define INV_RANGE(s,l) 					_cfe_flushcache(CFE_CACHE_INVAL_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))
#endif
#else /* !CONFIG_ARM */

#define CACHE_TO_NONCACHE(x)    			K0_TO_K1((uint32_t)x)
#define NONCACHE_TO_CACHE(x)    			K1_TO_K0((uint32_t)x)
#define CACHED_MALLOC_ATOMIC(_size) 			NONCACHE_TO_CACHE(KMALLOC(_size,0))
#define CACHED_MALLOC(_size) 				NONCACHE_TO_CACHE(KMALLOC(_size,0))
#define NONCACHED_MALLOC_ATOMIC(_size) 			CACHE_TO_NONCACHE(KMALLOC(_size,0))
#define NONCACHED_MALLOC(_size) 			CACHE_TO_NONCACHE(KMALLOC(_size,0))
#define NONCACHED_FREE(_ptr) 				KFREE(NONCACHE_TO_CACHE(_ptr))
#define CACHED_FREE(_ptr)				KFREE(_ptr)
#define VIRT_TO_PHYS(_addr)				K0_TO_PHYS((uint32_t)_addr)
#define PHYS_TO_CACHED(_addr)				PHYS_TO_K0((uint32_t)_addr)
#define PHYS_TO_UNCACHED(_addr)				PHYS_TO_K1((uint32_t)_addr)
#define DMA_CACHE_LINE					16
extern void _cfe_flushcache(int, uint8_t *, uint8_t *);
#define FLUSH_RANGE(s,l) 				_cfe_flushcache(CFE_CACHE_FLUSH_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))
#define INV_RANGE(s,l) 					_cfe_flushcache(CFE_CACHE_INVAL_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))

#endif /* !CONFIG_ARM */

#elif defined(__KERNEL__)

#if defined(CONFIG_MIPS)

#define CACHE_TO_NONCACHE(x)    			KSEG1ADDR(x)
#define NONCACHE_TO_CACHE(x)    			KSEG0ADDR(x)
#define CACHED_MALLOC_ATOMIC(_size) 			kmalloc(_size,GFP_ATOMIC)
#define CACHED_MALLOC(_size) 				kmalloc(_size,GFP_KERNEL)
#define CACHED_FREE(ptr) 				kfree((void*)ptr)
#define NONCACHED_MALLOC_ATOMIC(_size) 			CACHE_TO_NONCACHE(kmalloc(_size,GFP_ATOMIC|__GFP_DMA))
#define NONCACHED_MALLOC(_size) 			CACHE_TO_NONCACHE(kmalloc(_size,GFP_ATOMIC|__GFP_DMA))
#define NONCACHED_FREE(_ptr) 				kfree((void*)NONCACHE_TO_CACHE(_ptr))
#ifdef VIRT_TO_PHYS
#undef VIRT_TO_PHYS
#endif
#define VIRT_TO_PHYS(_addr)				CPHYSADDR(_addr)
#define PHYS_TO_CACHED(_addr)				KSEG0ADDR(_addr)
#define PHYS_TO_UNCACHED(_addr)				KSEG1ADDR(_addr)
#define FLUSH_RANGE(_addr, _size) 			blast_dcache_range(_addr, _addr+_size)
#define INV_RANGE(_addr, _size) 			blast_inv_dcache_range((uint32_t)_addr, (uint32_t)_addr+_size)

#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)

#define CACHE_TO_NONCACHE(x)    			(x)
#define NONCACHE_TO_CACHE(x)    			(x)
#define CACHED_MALLOC_ATOMIC(_size) 			kmalloc(_size,GFP_ATOMIC)
#define CACHED_MALLOC(_size) 				kmalloc(_size,GFP_KERNEL)
#define CACHED_FREE(ptr) 				kfree((void*)ptr)
#ifdef CONFIG_PLAT_BCM63XX_ACP
/* will allocate an additional 4 bytes at the end to store the original virtual pointer */
#define NONCACHED_MALLOC(_size, _phys_addr_ptr)		({void *_acp_addr_ptr = NULL;			\
		void *_org_addr_ptr = dma_alloc_coherent(NULL, _size + sizeof(void *), _phys_addr_ptr, GFP_KERNEL|GFP_ACP);	\
		if (_org_addr_ptr != NULL) {								\
			_acp_addr_ptr = (void *)ACP_ADDRESS(*_phys_addr_ptr);				\
			*(uintptr_t *)((uintptr_t)_acp_addr_ptr + _size) = (uintptr_t)_org_addr_ptr;	\
		}											\
		_acp_addr_ptr;		})
#define NONCACHED_MALLOC_ATOMIC(_size, _phys_addr_ptr)	({void *_acp_addr_ptr = NULL;			\
		void *_org_addr_ptr = dma_alloc_coherent(NULL, _size + sizeof(void *), _phys_addr_ptr, GFP_ATOMIC|GFP_ACP);	\
		if (_org_addr_ptr != NULL) {								\
			_acp_addr_ptr = (void *)ACP_ADDRESS(*_phys_addr_ptr);				\
			*(uintptr_t *)((uintptr_t)_acp_addr_ptr + _size) = (uintptr_t)_org_addr_ptr;	\
		}											\
		_acp_addr_ptr;		})
#define NONCACHED_FREE(_size, _ptr, _phys_addr)		({void *_org_addr_ptr;				\
			_org_addr_ptr = (void *)(*(uintptr_t *)((uintptr_t)_ptr + _size));	\
		dma_free_coherent(NULL, _size + 4, _org_addr_ptr, _phys_addr);	})
#else
#define NONCACHED_MALLOC_ATOMIC(_size, _phys_addr_ptr)	dma_alloc_coherent(NULL, _size, _phys_addr_ptr, GFP_ATOMIC)
#define NONCACHED_MALLOC(_size, _phys_addr_ptr)		dma_alloc_coherent(NULL, _size, _phys_addr_ptr, GFP_KERNEL)
#define NONCACHED_FREE(size, _ptr, _phys_addr)		dma_free_coherent(NULL, size, _ptr, _phys_addr)
#endif
#ifndef VIRT_TO_PHYS
#define VIRT_TO_PHYS(_addr)				virt_to_phys((const volatile void *)_addr)
#endif
#define PHYS_TO_CACHED(_addr)				phys_to_virt((phys_addr_t)_addr)
#define PHYS_TO_UNCACHED(_addr)				phys_to_virt((phys_addr_t)_addr)
#define FLUSH_RANGE(_addr, _size) 			dma_map_single(NULL, (void *)_addr, _size, DMA_TO_DEVICE)
#define INV_RANGE(_addr, _size) 			dma_map_single(NULL, (void *)_addr, _size, DMA_FROM_DEVICE)

#endif

#define DMA_CACHE_LINE					dma_get_cache_alignment()

#else
#define CACHE_TO_NONCACHE(x)    			KSEG1ADDR(x)
#define NONCACHE_TO_CACHE(x)    			KSEG0ADDR(x)
#define VIRT_TO_PHYS(_addr)				CPHYSADDR(_addr)
#define PHYS_TO_CACHED(_addr)				KSEG0ADDR(_addr)
#define PHYS_TO_UNCACHED(_addr)			KSEG1ADDR(_addr)
#endif

#endif /* _BCM_MM_H_ */
