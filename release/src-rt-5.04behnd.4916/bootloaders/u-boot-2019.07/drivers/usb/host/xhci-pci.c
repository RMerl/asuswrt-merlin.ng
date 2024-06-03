// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015, Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * All rights reserved.
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <usb.h>
#include "xhci.h"

static void xhci_pci_init(struct udevice *dev, struct xhci_hccr **ret_hccr,
			  struct xhci_hcor **ret_hcor)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	u32 cmd;

	hccr = (struct xhci_hccr *)dm_pci_map_bar(dev,
			PCI_BASE_ADDRESS_0, PCI_REGION_MEM);
	hcor = (struct xhci_hcor *)((uintptr_t) hccr +
			HC_LENGTH(xhci_readl(&hccr->cr_capbase)));

	debug("XHCI-PCI init hccr %p and hcor %p hc_length %d\n",
	      hccr, hcor, (u32)HC_LENGTH(xhci_readl(&hccr->cr_capbase)));

	*ret_hccr = hccr;
	*ret_hcor = hcor;

	/* enable busmaster */
	dm_pci_read_config32(dev, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MASTER;
	dm_pci_write_config32(dev, PCI_COMMAND, cmd);
}

static int xhci_pci_probe(struct udevice *dev)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;

	xhci_pci_init(dev, &hccr, &hcor);

	return xhci_register(dev, hccr, hcor);
}

static const struct udevice_id xhci_pci_ids[] = {
	{ .compatible = "xhci-pci" },
	{ }
};

U_BOOT_DRIVER(xhci_pci) = {
	.name	= "xhci_pci",
	.id	= UCLASS_USB,
	.probe = xhci_pci_probe,
	.remove = xhci_deregister,
	.of_match = xhci_pci_ids,
	.ops	= &xhci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct xhci_ctrl),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

static struct pci_device_id xhci_pci_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_SERIAL_USB_XHCI, ~0) },
	{},
};

U_BOOT_PCI_DEVICE(xhci_pci, xhci_pci_supported);
