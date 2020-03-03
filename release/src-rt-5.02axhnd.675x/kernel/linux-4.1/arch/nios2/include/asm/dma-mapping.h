/*
 * Copyright (C) 2011 Tobias Klauser <tklauser@distanz.ch>
 * Copyright (C) 2009 Wind River Systems Inc
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 */

#ifndef _ASM_NIOS2_DMA_MAPPING_H
#define _ASM_NIOS2_DMA_MAPPING_H

#include <linux/scatterlist.h>
#include <linux/cache.h>
#include <asm/cacheflush.h>

static inline void __dma_sync_for_device(void *vaddr, size_t size,
			      enum dma_data_direction direction)
{
	switch (direction) {
	case DMA_FROM_DEVICE:
		invalidate_dcache_range((unsigned long)vaddr,
			(unsigned long)(vaddr + size));
		break;
	case DMA_TO_DEVICE:
		/*
		 * We just need to flush the caches here , but Nios2 flush
		 * instruction will do both writeback and invalidate.
		 */
	case DMA_BIDIRECTIONAL: /* flush and invalidate */
		flush_dcache_range((unsigned long)vaddr,
			(unsigned long)(vaddr + size));
		break;
	default:
		BUG();
	}
}

static inline void __dma_sync_for_cpu(void *vaddr, size_t size,
			      enum dma_data_direction direction)
{
	switch (direction) {
	case DMA_BIDIRECTIONAL:
	case DMA_FROM_DEVICE:
		invalidate_dcache_range((unsigned long)vaddr,
			(unsigned long)(vaddr + size));
		break;
	case DMA_TO_DEVICE:
		break;
	default:
		BUG();
	}
}

#define dma_alloc_noncoherent(d, s, h, f) dma_alloc_coherent(d, s, h, f)
#define dma_free_noncoherent(d, s, v, h) dma_free_coherent(d, s, v, h)

void *dma_alloc_coherent(struct device *dev, size_t size,
			   dma_addr_t *dma_handle, gfp_t flag);

void dma_free_coherent(struct device *dev, size_t size,
			 void *vaddr, dma_addr_t dma_handle);

static inline dma_addr_t dma_map_single(struct device *dev, void *ptr,
					size_t size,
					enum dma_data_direction direction)
{
	BUG_ON(!valid_dma_direction(direction));
	__dma_sync_for_device(ptr, size, direction);
	return virt_to_phys(ptr);
}

static inline void dma_unmap_single(struct device *dev, dma_addr_t dma_addr,
				size_t size, enum dma_data_direction direction)
{
}

extern int dma_map_sg(struct device *dev, struct scatterlist *sg, int nents,
	enum dma_data_direction direction);
extern dma_addr_t dma_map_page(struct device *dev, struct page *page,
	unsigned long offset, size_t size, enum dma_data_direction direction);
extern void dma_unmap_page(struct device *dev, dma_addr_t dma_address,
	size_t size, enum dma_data_direction direction);
extern void dma_unmap_sg(struct device *dev, struct scatterlist *sg,
	int nhwentries, enum dma_data_direction direction);
extern void dma_sync_single_for_cpu(struct device *dev, dma_addr_t dma_handle,
	size_t size, enum dma_data_direction direction);
extern void dma_sync_single_for_device(struct device *dev,
	dma_addr_t dma_handle, size_t size, enum dma_data_direction direction);
extern void dma_sync_single_range_for_cpu(struct device *dev,
	dma_addr_t dma_handle, unsigned long offset, size_t size,
	enum dma_data_direction direction);
extern void dma_sync_single_range_for_device(struct device *dev,
	dma_addr_t dma_handle, unsigned long offset, size_t size,
	enum dma_data_direction direction);
extern void dma_sync_sg_for_cpu(struct device *dev, struct scatterlist *sg,
	int nelems, enum dma_data_direction direction);
extern void dma_sync_sg_for_device(struct device *dev, struct scatterlist *sg,
	int nelems, enum dma_data_direction direction);

static inline int dma_supported(struct device *dev, u64 mask)
{
	return 1;
}

static inline int dma_set_mask(struct device *dev, u64 mask)
{
	if (!dev->dma_mask || !dma_supported(dev, mask))
		return -EIO;

	*dev->dma_mask = mask;

	return 0;
}

static inline int dma_mapping_error(struct device *dev, dma_addr_t dma_addr)
{
	return 0;
}

/*
* dma_alloc_noncoherent() returns non-cacheable memory, so there's no need to
* do any flushing here.
*/
static inline void dma_cache_sync(struct device *dev, void *vaddr, size_t size,
				  enum dma_data_direction direction)
{
}

/* drivers/base/dma-mapping.c */
extern int dma_common_mmap(struct device *dev, struct vm_area_struct *vma,
		void *cpu_addr, dma_addr_t dma_addr, size_t size);
extern int dma_common_get_sgtable(struct device *dev, struct sg_table *sgt,
		void *cpu_addr, dma_addr_t dma_addr,
		size_t size);

#define dma_mmap_coherent(d, v, c, h, s) dma_common_mmap(d, v, c, h, s)
#define dma_get_sgtable(d, t, v, h, s) dma_common_get_sgtable(d, t, v, h, s)

#endif /* _ASM_NIOS2_DMA_MAPPING_H */
