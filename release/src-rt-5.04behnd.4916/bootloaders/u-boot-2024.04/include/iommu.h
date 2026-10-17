#ifndef _IOMMU_H
#define _IOMMU_H

struct udevice;

struct iommu_ops {
	/**
	 * init() - Connect a device to it's IOMMU, called before probe()
	 * The iommu device can be fetched through dev->iommu
	 *
	 * @iommu_dev:	IOMMU device
	 * @dev:	Device to connect
	 * @return 0 if OK, -errno on error
	 */
	int (*connect)(struct udevice *dev);
	/**
	 * map() - map DMA memory
	 *
	 * @dev:	device for which to map DMA memory
	 * @addr:	CPU address of the memory
	 * @size:	size of the memory
	 * @return DMA address for the device
	 */
	dma_addr_t (*map)(struct udevice *dev, void *addr, size_t size);

	/**
	 * unmap() - unmap DMA memory
	 *
	 * @dev:	device for which to unmap DMA memory
	 * @addr:	DMA address of the memory
	 * @size:	size of the memory
	 */
	void (*unmap)(struct udevice *dev, dma_addr_t addr, size_t size);
};

#if (CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)) && \
	CONFIG_IS_ENABLED(IOMMU)
int dev_iommu_enable(struct udevice *dev);
#else
static inline int dev_iommu_enable(struct udevice *dev)
{
	return 0;
}
#endif

dma_addr_t dev_iommu_dma_map(struct udevice *dev, void *addr, size_t size);
void dev_iommu_dma_unmap(struct udevice *dev, dma_addr_t addr, size_t size);

#endif
