// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@mips.com>
 */

#include <dm.h>
#include <init.h>
#include <msc01.h>
#include <pci.h>
#include <pci_msc01.h>
#include <asm/io.h>

#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1

struct msc01_pci_controller {
	struct pci_controller hose;
	void *base;
};

static inline struct msc01_pci_controller *
hose_to_msc01(struct pci_controller *hose)
{
	return container_of(hose, struct msc01_pci_controller, hose);
}

static int msc01_config_access(struct msc01_pci_controller *msc01,
			       unsigned char access_type, pci_dev_t bdf,
			       int where, u32 *data)
{
	const u32 aborts = MSC01_PCI_INTSTAT_MA_MSK | MSC01_PCI_INTSTAT_TA_MSK;
	void *intstat = msc01->base + MSC01_PCI_INTSTAT_OFS;
	void *cfgdata = msc01->base + MSC01_PCI_CFGDATA_OFS;
	unsigned int bus = PCI_BUS(bdf);
	unsigned int dev = PCI_DEV(bdf);
	unsigned int func = PCI_FUNC(bdf);

	/* clear abort status */
	__raw_writel(aborts, intstat);

	/* setup address */
	__raw_writel((PCI_CONF1_ADDRESS(bus, dev, func, where) & ~PCI_CONF1_ENABLE),
		     msc01->base + MSC01_PCI_CFGADDR_OFS);

	/* perform access */
	if (access_type == PCI_ACCESS_WRITE)
		__raw_writel(*data, cfgdata);
	else
		*data = __raw_readl(cfgdata);

	/* check for aborts */
	if (__raw_readl(intstat) & aborts) {
		/* clear abort status */
		__raw_writel(aborts, intstat);
		return -1;
	}

	return 0;
}

static int msc01_pci_read_config(const struct udevice *dev, pci_dev_t bdf,
				 uint where, ulong *val, enum pci_size_t size)
{
	struct msc01_pci_controller *msc01 = dev_get_priv(dev);
	u32 data = 0;

	if (msc01_config_access(msc01, PCI_ACCESS_READ, bdf, where, &data)) {
		*val = pci_get_ff(size);
		return 0;
	}

	*val = pci_conv_32_to_size(data, where, size);

	return 0;
}

static int msc01_pci_write_config(struct udevice *dev, pci_dev_t bdf,
				  uint where, ulong val, enum pci_size_t size)
{
	struct msc01_pci_controller *msc01 = dev_get_priv(dev);
	u32 data = 0;

	if (size == PCI_SIZE_32) {
		data = val;
	} else {
		u32 old;

		if (msc01_config_access(msc01, PCI_ACCESS_READ, bdf, where, &old))
			return 0;

		data = pci_conv_size_to_32(old, val, where, size);
	}

	msc01_config_access(msc01, PCI_ACCESS_WRITE, bdf, where, &data);

	return 0;
}

static int msc01_pci_probe(struct udevice *dev)
{
	struct msc01_pci_controller *msc01 = dev_get_priv(dev);

	msc01->base = dev_remap_addr(dev);
	if (!msc01->base)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops msc01_pci_ops = {
	.read_config	= msc01_pci_read_config,
	.write_config	= msc01_pci_write_config,
};

static const struct udevice_id msc01_pci_ids[] = {
	{ .compatible = "mips,pci-msc01" },
	{ }
};

U_BOOT_DRIVER(msc01_pci) = {
	.name		= "msc01_pci",
	.id		= UCLASS_PCI,
	.of_match	= msc01_pci_ids,
	.ops		= &msc01_pci_ops,
	.probe		= msc01_pci_probe,
	.priv_auto	= sizeof(struct msc01_pci_controller),
};
