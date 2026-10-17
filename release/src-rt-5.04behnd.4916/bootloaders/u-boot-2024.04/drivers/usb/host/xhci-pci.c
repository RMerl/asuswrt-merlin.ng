// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015, Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * All rights reserved.
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <init.h>
#include <log.h>
#include <pci.h>
#include <reset.h>
#include <usb.h>
#include <usb/xhci.h>

struct xhci_pci_plat {
	struct reset_ctl reset;
};

static int xhci_pci_init(struct udevice *dev, struct xhci_hccr **ret_hccr,
			 struct xhci_hcor **ret_hcor)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	u32 cmd;

	hccr = (struct xhci_hccr *)dm_pci_map_bar(dev,
			PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE,
			PCI_REGION_MEM);
	if (!hccr) {
		printf("xhci-pci init cannot map PCI mem bar\n");
		return -EIO;
	}

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
	return 0;
}

static int xhci_pci_probe(struct udevice *dev)
{
	struct xhci_pci_plat *plat = dev_get_plat(dev);
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	int ret;

	ret = reset_get_by_index(dev, 0, &plat->reset);
	if (ret && ret != -ENOENT && ret != -ENOTSUPP) {
		dev_err(dev, "failed to get reset\n");
		return ret;
	}

	if (reset_valid(&plat->reset)) {
		ret = reset_assert(&plat->reset);
		if (ret)
			goto err_reset;

		ret = reset_deassert(&plat->reset);
		if (ret)
			goto err_reset;
	}

	ret = xhci_pci_init(dev, &hccr, &hcor);
	if (ret)
		goto err_reset;

	ret = xhci_register(dev, hccr, hcor);
	if (ret)
		goto err_reset;

	return 0;

err_reset:
	if (reset_valid(&plat->reset))
		reset_free(&plat->reset);

	return ret;
}

static int xhci_pci_remove(struct udevice *dev)
{
	struct xhci_pci_plat *plat = dev_get_plat(dev);

	xhci_deregister(dev);
	if (reset_valid(&plat->reset))
		reset_free(&plat->reset);

	return 0;
}

static const struct udevice_id xhci_pci_ids[] = {
	{ .compatible = "xhci-pci" },
	{ }
};

U_BOOT_DRIVER(xhci_pci) = {
	.name	= "xhci_pci",
	.id	= UCLASS_USB,
	.probe = xhci_pci_probe,
	.remove	= xhci_pci_remove,
	.of_match = xhci_pci_ids,
	.ops	= &xhci_usb_ops,
	.plat_auto	= sizeof(struct xhci_pci_plat),
	.priv_auto	= sizeof(struct xhci_ctrl),
	.flags	= DM_FLAG_OS_PREPARE | DM_FLAG_ALLOC_PRIV_DMA,
};

static struct pci_device_id xhci_pci_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_SERIAL_USB_XHCI, ~0) },
	{},
};

U_BOOT_PCI_DEVICE(xhci_pci, xhci_pci_supported);
