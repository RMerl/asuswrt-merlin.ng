// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Marvell International Ltd.
 *
 * MVEBU USB HOST xHCI Controller
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <usb.h>
#include <power/regulator.h>
#include <asm/gpio.h>

#include "xhci.h"

struct mvebu_xhci_platdata {
	fdt_addr_t hcd_base;
};

/**
 * Contains pointers to register base addresses
 * for the usb controller.
 */
struct mvebu_xhci {
	struct xhci_ctrl ctrl;	/* Needs to come first in this struct! */
	struct usb_platdata usb_plat;
	struct xhci_hccr *hcd;
};

/*
 * Dummy implementation that can be overwritten by a board
 * specific function
 */
__weak int board_xhci_enable(fdt_addr_t base)
{
	return 0;
}

static int xhci_usb_probe(struct udevice *dev)
{
	struct mvebu_xhci_platdata *plat = dev_get_platdata(dev);
	struct mvebu_xhci *ctx = dev_get_priv(dev);
	struct xhci_hcor *hcor;
	int len, ret;
	struct udevice *regulator;

	ctx->hcd = (struct xhci_hccr *)plat->hcd_base;
	len = HC_LENGTH(xhci_readl(&ctx->hcd->cr_capbase));
	hcor = (struct xhci_hcor *)((uintptr_t)ctx->hcd + len);

	ret = device_get_supply_regulator(dev, "vbus-supply", &regulator);
	if (!ret) {
		ret = regulator_set_enable(regulator, true);
		if (ret) {
			printf("Failed to turn ON the VBUS regulator\n");
			return ret;
		}
	}

	/* Enable USB xHCI (VBUS, reset etc) in board specific code */
	board_xhci_enable(devfdt_get_addr_index(dev, 1));

	return xhci_register(dev, ctx->hcd, hcor);
}

static int xhci_usb_ofdata_to_platdata(struct udevice *dev)
{
	struct mvebu_xhci_platdata *plat = dev_get_platdata(dev);

	/*
	 * Get the base address for XHCI controller from the device node
	 */
	plat->hcd_base = devfdt_get_addr(dev);
	if (plat->hcd_base == FDT_ADDR_T_NONE) {
		debug("Can't get the XHCI register base address\n");
		return -ENXIO;
	}

	return 0;
}

static const struct udevice_id xhci_usb_ids[] = {
	{ .compatible = "marvell,armada3700-xhci" },
	{ .compatible = "marvell,armada-380-xhci" },
	{ .compatible = "marvell,armada-8k-xhci" },
	{ }
};

U_BOOT_DRIVER(usb_xhci) = {
	.name	= "xhci_mvebu",
	.id	= UCLASS_USB,
	.of_match = xhci_usb_ids,
	.ofdata_to_platdata = xhci_usb_ofdata_to_platdata,
	.probe = xhci_usb_probe,
	.remove = xhci_deregister,
	.ops	= &xhci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct mvebu_xhci_platdata),
	.priv_auto_alloc_size = sizeof(struct mvebu_xhci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
