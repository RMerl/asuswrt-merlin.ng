// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014, Xilinx, Inc
 *
 * USB Low level initialization(Specific to zynq)
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <usb/ehci-ci.h>
#include <usb/ulpi.h>

#include "ehci.h"

struct zynq_ehci_priv {
	struct ehci_ctrl ehcictrl;
	struct usb_ehci *ehci;
};

static int ehci_zynq_of_to_plat(struct udevice *dev)
{
	struct zynq_ehci_priv *priv = dev_get_priv(dev);

	priv->ehci = dev_read_addr_ptr(dev);
	if (!priv->ehci)
		return -EINVAL;

	return 0;
}

static int ehci_zynq_probe(struct udevice *dev)
{
	struct usb_plat *plat = dev_get_plat(dev);
	struct zynq_ehci_priv *priv = dev_get_priv(dev);
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	struct ulpi_viewport ulpi_vp;
	/* Used for writing the ULPI data address */
	struct ulpi_regs *ulpi = (struct ulpi_regs *)0;
	int ret;

	hccr = (struct ehci_hccr *)((uint32_t)&priv->ehci->caplength);
	hcor = (struct ehci_hcor *)((uint32_t) hccr +
			HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	ulpi_vp.viewport_addr = (u32)&priv->ehci->ulpi_viewpoint;
	ulpi_vp.port_num = 0;

	ret = ulpi_init(&ulpi_vp);
	if (ret) {
		puts("zynq ULPI viewport init failed\n");
		return -1;
	}

	/* ULPI set flags */
	ulpi_write(&ulpi_vp, &ulpi->otg_ctrl,
		   ULPI_OTG_DP_PULLDOWN | ULPI_OTG_DM_PULLDOWN |
		   ULPI_OTG_EXTVBUSIND);
	ulpi_write(&ulpi_vp, &ulpi->function_ctrl,
		   ULPI_FC_FULL_SPEED | ULPI_FC_OPMODE_NORMAL |
		   ULPI_FC_SUSPENDM);
	ulpi_write(&ulpi_vp, &ulpi->iface_ctrl, 0);

	/* Set VBus */
	ulpi_write(&ulpi_vp, &ulpi->otg_ctrl_set,
		   ULPI_OTG_DRVVBUS | ULPI_OTG_DRVVBUS_EXT);

	return ehci_register(dev, hccr, hcor, NULL, 0, plat->init_type);
}

static const struct udevice_id ehci_zynq_ids[] = {
	{ .compatible = "xlnx,zynq-usb-2.20a" },
	{ }
};

U_BOOT_DRIVER(ehci_zynq) = {
	.name	= "ehci_zynq",
	.id	= UCLASS_USB,
	.of_match = ehci_zynq_ids,
	.of_to_plat = ehci_zynq_of_to_plat,
	.probe = ehci_zynq_probe,
	.remove = ehci_deregister,
	.ops	= &ehci_usb_ops,
	.plat_auto	= sizeof(struct usb_plat),
	.priv_auto	= sizeof(struct zynq_ehci_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
