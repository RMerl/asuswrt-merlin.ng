// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Broadcom.
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <usb.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <usb/xhci.h>

#define DRD2U3H_XHC_REGS_AXIWRA	0xC08
#define DRD2U3H_XHC_REGS_AXIRDA	0xC0C

#define USBAXI_CACHE		0xF
#define USBAXI_PROT		0x8
#define USBAXI_SA_MASK		0x1FF
#define USBAXI_UA_MASK		(0x1FF << 16)
#define USBAXI_SA_VAL		((USBAXI_CACHE << 4) | USBAXI_PROT)
#define USBAXI_UA_VAL		(USBAXI_SA_VAL << 16)
#define USBAXI_SA_UA_MASK	(USBAXI_UA_MASK | USBAXI_SA_MASK)
#define USBAXI_SA_UA_VAL	(USBAXI_UA_VAL | USBAXI_SA_VAL)

struct brcm_xhci_plat {
	unsigned int arcache;
	unsigned int awcache;
	void __iomem *hc_base;
};

static int xhci_brcm_probe(struct udevice *dev)
{
	struct brcm_xhci_plat *plat = dev_get_plat(dev);
	struct xhci_hcor *hcor;
	struct xhci_hccr *hcd;
	int len, ret = 0;

	if (!plat) {
		dev_err(dev, "Can't get xHCI Plat data\n");
		return -ENOMEM;
	}

	hcd = dev_read_addr_ptr(dev);
	if (!hcd) {
		dev_err(dev, "Can't get the xHCI register base address\n");
		return -ENXIO;
	}

	plat->hc_base = hcd;
	len = HC_LENGTH(xhci_readl(&hcd->cr_capbase));
	hcor = (struct xhci_hcor *)(plat->hc_base + len);

	/* Save the default values of AXI read and write attributes */
	plat->awcache = readl(plat->hc_base + DRD2U3H_XHC_REGS_AXIWRA);
	plat->arcache = readl(plat->hc_base + DRD2U3H_XHC_REGS_AXIRDA);

	/* Enable AXI write attributes */
	clrsetbits_le32(plat->hc_base + DRD2U3H_XHC_REGS_AXIWRA,
			USBAXI_SA_UA_MASK, USBAXI_SA_UA_VAL);

	/* Enable AXI read attributes */
	clrsetbits_le32(plat->hc_base + DRD2U3H_XHC_REGS_AXIRDA,
			USBAXI_SA_UA_MASK, USBAXI_SA_UA_VAL);

	ret = xhci_register(dev, hcd, hcor);
	if (ret)
		dev_err(dev, "Failed to register xHCI\n");

	return ret;
}

static int xhci_brcm_deregister(struct udevice *dev)
{
	struct brcm_xhci_plat *plat = dev_get_plat(dev);

	/* Restore the default values for AXI read and write attributes */
	writel(plat->awcache, plat->hc_base + DRD2U3H_XHC_REGS_AXIWRA);
	writel(plat->arcache, plat->hc_base + DRD2U3H_XHC_REGS_AXIRDA);

	return xhci_deregister(dev);
}

static const struct udevice_id xhci_brcm_ids[] = {
	{ .compatible = "brcm,generic-xhci" },
	{ }
};

U_BOOT_DRIVER(usb_xhci) = {
	.name				= "xhci_brcm",
	.id				= UCLASS_USB,
	.probe				= xhci_brcm_probe,
	.remove				= xhci_brcm_deregister,
	.ops				= &xhci_usb_ops,
	.of_match			= xhci_brcm_ids,
	.plat_auto	= sizeof(struct brcm_xhci_plat),
	.priv_auto		= sizeof(struct xhci_ctrl),
	.flags				= DM_FLAG_ALLOC_PRIV_DMA,
};
