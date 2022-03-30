// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    
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

#if defined(__UBOOT__)

#include <common.h>

#define VIRT_TO_PHYS(_addr)				((unsigned int)_addr)
#define PHYS_TO_CACHED(_addr)				((void *)((unsigned int)_addr))
#define PHYS_TO_UNCACHED(_addr)				((void *)((unsigned int)_addr))

#define CACHED_MALLOC_ATOMIC(_size) 			malloc(_size)
#define CACHED_MALLOC(_size) 				malloc(_size)
#define CACHED_FREE(_ptr)				free(_ptr)
#define NONCACHED_MALLOC_ATOMIC(_size) 			noncached_alloc(_size, 64)
#define NONCACHED_MALLOC(_size) 			noncached_alloc(_size, 64)
#define NONCACHED_FREE(_ptr) 				(_ptr)

#define KMALLOC(_size, _align)				memalign(_align, _size)
#define KFREE(_ptr)					free(_ptr)

#if defined(CONFIG_BCM6858) || defined(CONFIG_BCM63148) || \
    defined(CONFIG_BCM6846) || defined(CONFIG_BCM6856) || defined(CONFIG_BCM6878)
#define DMA_CACHE_LINE					64
#else
#define DMA_CACHE_LINE					32
#endif

#define FLUSH_RANGE(s,l) 				({unsigned long start, end;	\
				start = ((unsigned long)(s))&~(DMA_CACHE_LINE-1);			\
				end = (((unsigned long)(s)+(l)) + DMA_CACHE_LINE - 1)&~(DMA_CACHE_LINE-1); \
				flush_dcache_range(start, end);	})

#define INV_RANGE(s,l) 					({unsigned long start, end;	\
				start = ((unsigned long)(s))&~(DMA_CACHE_LINE-1); \
				end = (((unsigned long)(s)+(l)) + DMA_CACHE_LINE - 1)&~(DMA_CACHE_LINE-1); \
				invalidate_dcache_range(start, end);	})
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

#else /* __KERNEL__ */
#define CACHE_TO_NONCACHE(x)    			KSEG1ADDR(x)
#define NONCACHE_TO_CACHE(x)    			KSEG0ADDR(x)
#define VIRT_TO_PHYS(_addr)				CPHYSADDR(_addr)
#define PHYS_TO_CACHED(_addr)				KSEG0ADDR(_addr)
#define PHYS_TO_UNCACHED(_addr)				KSEG1ADDR(_addr)
#endif

#endif /* _BCM_MM_H_ */
