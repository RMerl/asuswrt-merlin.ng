// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#define LOG_CATEGORY UCLASS_IOMMU

#include <common.h>
#include <dm.h>
#include <iommu.h>
#include <malloc.h>
#include <phys2bus.h>
#include <asm/io.h>

#if (CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA))

#if CONFIG_IS_ENABLED(PCI)
static int dev_pci_iommu_enable(struct udevice *dev)
{
	struct udevice *parent = dev->parent;
	struct udevice *dev_iommu;
	u32 *iommu_map;
	u32 iommu_map_mask, length, phandle, rid, rid_base;
	int i, count, len, ret;

	while (parent) {
		len = dev_read_size(parent, "iommu-map");
		if (len > 0)
			break;
		parent = parent->parent;
	}

	if (len <= 0)
		return 0;

	iommu_map = malloc(len);
	if (!iommu_map)
		return -ENOMEM;

	count = len / sizeof(u32);
	ret = dev_read_u32_array(parent, "iommu-map", iommu_map, count);
	if (ret < 0) {
		free(iommu_map);
		return 0;
	}

	iommu_map_mask = dev_read_u32_default(parent, "iommu-map-mask", ~0);
	rid = (dm_pci_get_bdf(dev) >> 8) & iommu_map_mask;

	/* Loop over entries until mapping is found. */
	for (i = 0; i < count; i += 4) {
		rid_base = iommu_map[i];
		phandle = iommu_map[i + 1];
		length = iommu_map[i + 3];

		if (rid < rid_base || rid >= rid_base + length)
			continue;

		ret = uclass_get_device_by_phandle_id(UCLASS_IOMMU, phandle,
						      &dev_iommu);
		if (ret) {
			debug("%s: uclass_get_device_by_ofnode failed: %d\n",
			      __func__, ret);
			free(iommu_map);
			return ret;
		}
		dev->iommu = dev_iommu;
		break;
	}

	free(iommu_map);
	return 0;
}
#endif

int dev_iommu_enable(struct udevice *dev)
{
	struct ofnode_phandle_args args;
	struct udevice *dev_iommu;
	const struct iommu_ops *ops;
	int i, count, ret = 0;

	count = dev_count_phandle_with_args(dev, "iommus",
					    "#iommu-cells", 0);
	for (i = 0; i < count; i++) {
		ret = dev_read_phandle_with_args(dev, "iommus",
						 "#iommu-cells", 0, i, &args);
		if (ret) {
			log_err("%s: Failed to parse 'iommus' property for '%s': %d\n",
				__func__, dev->name, ret);
			return ret;
		}

		ret = uclass_get_device_by_ofnode(UCLASS_IOMMU, args.node,
						  &dev_iommu);
		if (ret) {
			log_err("%s: Failed to find IOMMU device for '%s': %d\n",
				__func__, dev->name, ret);
			return ret;
		}
		dev->iommu = dev_iommu;

		if (dev->parent && dev->parent->iommu == dev_iommu)
			continue;

		ops = device_get_ops(dev->iommu);
		if (ops && ops->connect) {
			ret = ops->connect(dev);
			if (ret) {
				log_err("%s: Failed to connect '%s' to IOMMU '%s': %d\n",
					__func__, dev->name, dev->iommu->name, ret);
				return ret;
			}
		}
	}

#if CONFIG_IS_ENABLED(PCI)
	if (count < 0 && device_is_on_pci_bus(dev))
		return dev_pci_iommu_enable(dev);
#endif

	return 0;
}
#endif

dma_addr_t dev_iommu_dma_map(struct udevice *dev, void *addr, size_t size)
{
	const struct iommu_ops *ops;

	if (dev->iommu) {
		ops = device_get_ops(dev->iommu);
		if (ops && ops->map)
			return ops->map(dev->iommu, addr, size);
	}

	return dev_phys_to_bus(dev, virt_to_phys(addr));
}

void dev_iommu_dma_unmap(struct udevice *dev, dma_addr_t addr, size_t size)
{
	const struct iommu_ops *ops;

	if (dev->iommu) {
		ops = device_get_ops(dev->iommu);
		if (ops && ops->unmap)
			ops->unmap(dev->iommu, addr, size);
	}
}

UCLASS_DRIVER(iommu) = {
	.id		= UCLASS_IOMMU,
	.name		= "iommu",
};
