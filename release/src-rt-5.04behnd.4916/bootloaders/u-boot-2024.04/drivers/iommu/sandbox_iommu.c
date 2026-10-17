// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <dm.h>
#include <iommu.h>
#include <lmb.h>
#include <asm/io.h>
#include <linux/sizes.h>

#define IOMMU_PAGE_SIZE		SZ_4K

struct sandbox_iommu_priv {
	struct lmb lmb;
};

static dma_addr_t sandbox_iommu_map(struct udevice *dev, void *addr,
				    size_t size)
{
	struct sandbox_iommu_priv *priv = dev_get_priv(dev);
	phys_addr_t paddr, dva;
	phys_size_t psize, off;

	paddr = ALIGN_DOWN(virt_to_phys(addr), IOMMU_PAGE_SIZE);
	off = virt_to_phys(addr) - paddr;
	psize = ALIGN(size + off, IOMMU_PAGE_SIZE);

	dva = lmb_alloc(&priv->lmb, psize, IOMMU_PAGE_SIZE);

	return dva + off;
}

static void sandbox_iommu_unmap(struct udevice *dev, dma_addr_t addr,
				size_t size)
{
	struct sandbox_iommu_priv *priv = dev_get_priv(dev);
	phys_addr_t dva;
	phys_size_t psize;

	dva = ALIGN_DOWN(addr, IOMMU_PAGE_SIZE);
	psize = size + (addr - dva);
	psize = ALIGN(psize, IOMMU_PAGE_SIZE);

	lmb_free(&priv->lmb, dva, psize);
}

static struct iommu_ops sandbox_iommu_ops = {
	.map = sandbox_iommu_map,
	.unmap = sandbox_iommu_unmap,
};

static int sandbox_iommu_probe(struct udevice *dev)
{
	struct sandbox_iommu_priv *priv = dev_get_priv(dev);

	lmb_init(&priv->lmb);
	lmb_add(&priv->lmb, 0x89abc000, SZ_16K);

	return 0;
}

static const struct udevice_id sandbox_iommu_ids[] = {
	{ .compatible = "sandbox,iommu" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sandbox_iommu) = {
	.name = "sandbox_iommu",
	.id = UCLASS_IOMMU,
	.of_match = sandbox_iommu_ids,
	.priv_auto = sizeof(struct sandbox_iommu_priv),
	.ops = &sandbox_iommu_ops,
	.probe = sandbox_iommu_probe,
};
