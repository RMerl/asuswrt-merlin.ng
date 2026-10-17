// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019
 * Written by Ramon Fried <ramon.fried@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <pci_ep.h>
#include <asm/global_data.h>
#include <linux/sizes.h>
#include <linux/log2.h>
#include "pcie-cadence.h"

DECLARE_GLOBAL_DATA_PTR;

static int cdns_write_header(struct udevice *dev, uint fn,
			     struct pci_ep_header *hdr)
{
	struct cdns_pcie *pcie = dev_get_priv(dev);

	cdns_pcie_ep_fn_writew(pcie, fn, PCI_DEVICE_ID, hdr->deviceid);
	cdns_pcie_ep_fn_writeb(pcie, fn, PCI_REVISION_ID, hdr->revid);
	cdns_pcie_ep_fn_writeb(pcie, fn, PCI_CLASS_PROG,
			       hdr->progif_code);
	cdns_pcie_ep_fn_writew(pcie, fn, PCI_CLASS_DEVICE,
			       hdr->subclass_code |
			       hdr->baseclass_code << 8);
	cdns_pcie_ep_fn_writeb(pcie, fn, PCI_CACHE_LINE_SIZE,
			       hdr->cache_line_size);
	cdns_pcie_ep_fn_writew(pcie, fn, PCI_SUBSYSTEM_ID,
			       hdr->subsys_id);
	cdns_pcie_ep_fn_writeb(pcie, fn, PCI_INTERRUPT_PIN,
			       hdr->interrupt_pin);

	/*
	 * Vendor ID can only be modified from function 0, all other functions
	 * use the same vendor ID as function 0.
	 */
	if (fn == 0) {
		/* Update the vendor IDs. */
		u32 id = CDNS_PCIE_LM_ID_VENDOR(hdr->vendorid) |
			 CDNS_PCIE_LM_ID_SUBSYS(hdr->subsys_vendor_id);

		cdns_pcie_writel(pcie, CDNS_PCIE_LM_ID, id);
	}

	return 0;
}

static int cdns_set_bar(struct udevice *dev, uint fn, struct pci_bar *ep_bar)
{
	struct cdns_pcie *pcie = dev_get_priv(dev);
	dma_addr_t bar_phys = ep_bar->phys_addr;
	enum pci_barno bar = ep_bar->barno;
	int flags = ep_bar->flags;
	u32 addr0, addr1, reg, cfg, b, aperture, ctrl;
	u64 sz;

	/* BAR size is 2^(aperture + 7) */
	sz = max_t(size_t, ep_bar->size, CDNS_PCIE_EP_MIN_APERTURE);
	/*
	 * roundup_pow_of_two() returns an unsigned long, which is not suited
	 * for 64bit values.
	 */
	sz = 1ULL << fls64(sz - 1);
	aperture = ilog2(sz) - 7; /* 128B -> 0, 256B -> 1, 512B -> 2, ... */

	if ((flags & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO) {
		ctrl = CDNS_PCIE_LM_BAR_CFG_CTRL_IO_32BITS;
	} else {
		bool is_prefetch = !!(flags & PCI_BASE_ADDRESS_MEM_PREFETCH);
		bool is_64bits = (sz > SZ_2G) |
			!!(ep_bar->flags & PCI_BASE_ADDRESS_MEM_TYPE_64);

		if (is_64bits && (bar & 1))
			return -EINVAL;

		if (is_64bits && !(flags & PCI_BASE_ADDRESS_MEM_TYPE_64))
			ep_bar->flags |= PCI_BASE_ADDRESS_MEM_TYPE_64;

		if (is_64bits && is_prefetch)
			ctrl = CDNS_PCIE_LM_BAR_CFG_CTRL_PREFETCH_MEM_64BITS;
		else if (is_prefetch)
			ctrl = CDNS_PCIE_LM_BAR_CFG_CTRL_PREFETCH_MEM_32BITS;
		else if (is_64bits)
			ctrl = CDNS_PCIE_LM_BAR_CFG_CTRL_MEM_64BITS;
		else
			ctrl = CDNS_PCIE_LM_BAR_CFG_CTRL_MEM_32BITS;
	}

	addr0 = lower_32_bits(bar_phys);
	addr1 = upper_32_bits(bar_phys);
	cdns_pcie_writel(pcie, CDNS_PCIE_AT_IB_EP_FUNC_BAR_ADDR0(fn, bar),
			 addr0);
	cdns_pcie_writel(pcie, CDNS_PCIE_AT_IB_EP_FUNC_BAR_ADDR1(fn, bar),
			 addr1);

	if (bar < BAR_4) {
		reg = CDNS_PCIE_LM_EP_FUNC_BAR_CFG0(fn);
		b = bar;
	} else {
		reg = CDNS_PCIE_LM_EP_FUNC_BAR_CFG1(fn);
		b = bar - BAR_4;
	}

	cfg = cdns_pcie_readl(pcie, reg);
	cfg &= ~(CDNS_PCIE_LM_EP_FUNC_BAR_CFG_BAR_APERTURE_MASK(b) |
		 CDNS_PCIE_LM_EP_FUNC_BAR_CFG_BAR_CTRL_MASK(b));
	cfg |= (CDNS_PCIE_LM_EP_FUNC_BAR_CFG_BAR_APERTURE(b, aperture) |
		CDNS_PCIE_LM_EP_FUNC_BAR_CFG_BAR_CTRL(b, ctrl));
	cdns_pcie_writel(pcie, reg, cfg);

	return 0;
}

static int cdns_set_msi(struct udevice *dev, uint fn, uint mmc)
{
	struct cdns_pcie *pcie = dev_get_priv(dev);
	u32 cap = CDNS_PCIE_EP_FUNC_MSI_CAP_OFFSET;

	/*
	 * Set the Multiple Message Capable bitfield into the Message Control
	 * register.
	 */
	u16 flags;

	flags = cdns_pcie_ep_fn_readw(pcie, fn, cap + PCI_MSI_FLAGS);
	flags = (flags & ~PCI_MSI_FLAGS_QMASK) | (mmc << 1);
	flags |= PCI_MSI_FLAGS_64BIT;
	flags &= ~PCI_MSI_FLAGS_MASKBIT;
	cdns_pcie_ep_fn_writew(pcie, fn, cap + PCI_MSI_FLAGS, flags);

	return 0;
}

static struct pci_ep_ops cdns_pci_ep_ops = {
	.write_header = cdns_write_header,
	.set_bar = cdns_set_bar,
	.set_msi = cdns_set_msi,
};

static int cdns_pci_ep_probe(struct udevice *dev)
{
	struct cdns_pcie *pdata = dev_get_priv(dev);

	pdata->reg_base = dev_read_addr_ptr(dev);
	if (!pdata->reg_base)
		return -ENOMEM;

	pdata->max_functions = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					      "max-functions", 1);
	pdata->max_regions = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					    "cdns,max-outbound-regions", 8);

	return 0;
}

static int cdns_pci_ep_remove(struct udevice *dev)
{
	return 0;
}

const struct udevice_id cadence_pci_ep_of_match[] = {
	{ .compatible = "cdns,cdns-pcie-ep" },
	{ }
};

U_BOOT_DRIVER(cdns_pcie) = {
	.name	= "cdns,pcie-ep",
	.id	= UCLASS_PCI_EP,
	.of_match = cadence_pci_ep_of_match,
	.ops = &cdns_pci_ep_ops,
	.probe = cdns_pci_ep_probe,
	.remove = cdns_pci_ep_remove,
	.priv_auto	= sizeof(struct cdns_pcie),
};
