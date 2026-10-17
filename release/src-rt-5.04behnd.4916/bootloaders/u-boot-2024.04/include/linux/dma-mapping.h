/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_DMA_MAPPING_H
#define _LINUX_DMA_MAPPING_H

#include <asm/cache.h>
#include <linux/dma-direction.h>
#include <linux/types.h>
#include <asm/dma-mapping.h>
#include <cpu_func.h>

#define dma_mapping_error(x, y)	0

/**
 * Map a buffer to make it available to the DMA device
 *
 * Linux-like DMA API that is intended to be used from drivers. This hides the
 * underlying cache operation from drivers. Call this before starting the DMA
 * transfer. In most of architectures in U-Boot, the virtual address matches to
 * the physical address (but we have exceptions like sandbox). U-Boot does not
 * support iommu at the driver level, so it also matches to the DMA address.
 * Hence, this helper currently just performs the cache operation, then returns
 * straight-mapped dma_address, which is intended to be set to the register of
 * the DMA device.
 *
 * @vaddr: address of the buffer
 * @len: length of the buffer
 * @dir: the direction of DMA
 */
static inline dma_addr_t dma_map_single(void *vaddr, size_t len,
					enum dma_data_direction dir)
{
	unsigned long addr = (unsigned long)vaddr;

	len = ALIGN(len, ARCH_DMA_MINALIGN);

	if (dir == DMA_FROM_DEVICE)
		invalidate_dcache_range(addr, addr + len);
	else
		flush_dcache_range(addr, addr + len);

	return addr;
}

/**
 * Unmap a buffer to make it available to CPU
 *
 * Linux-like DMA API that is intended to be used from drivers. This hides the
 * underlying cache operation from drivers. Call this after finishin the DMA
 * transfer.
 *
 * @addr: DMA address
 * @len: length of the buffer
 * @dir: the direction of DMA
 */
static inline void dma_unmap_single(dma_addr_t addr, size_t len,
				    enum dma_data_direction dir)
{
	len = ALIGN(len, ARCH_DMA_MINALIGN);

	if (dir != DMA_TO_DEVICE)
		invalidate_dcache_range(addr, addr + len);
}

#endif
