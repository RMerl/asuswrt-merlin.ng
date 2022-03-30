#ifndef __ASM_NIOS2_DMA_MAPPING_H
#define __ASM_NIOS2_DMA_MAPPING_H

#include <memalign.h>
#include <asm/io.h>

/*
 * dma_alloc_coherent() return cache-line aligned allocation which is mapped
 * to uncached io region.
 */
static inline void *dma_alloc_coherent(size_t len, unsigned long *handle)
{
	unsigned long addr = (unsigned long)malloc_cache_aligned(len);

	if (!addr)
		return NULL;

	invalidate_dcache_range(addr, addr + len);
	if (handle)
		*handle = addr;

	return map_physmem(addr, len, MAP_NOCACHE);
}
#endif /* __ASM_NIOS2_DMA_MAPPING_H */
