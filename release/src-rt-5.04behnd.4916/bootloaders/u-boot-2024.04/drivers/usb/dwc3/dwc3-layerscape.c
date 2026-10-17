// SPDX-License-Identifier: GPL-2.0
/*
 * Layerscape DWC3 Glue layer
 *
 * Copyright (C) 2021 Michael Walle <michael@walle.cc>
 *
 * Based on dwc3-generic.c.
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dwc3-uboot.h>
#include <linux/usb/gadget.h>
#include <usb.h>
#include "core.h"
#include "gadget.h"
#include <usb/xhci.h>

struct dwc3_layerscape_plat {
	fdt_addr_t base;
	u32 maximum_speed;
	enum usb_dr_mode dr_mode;
};

struct dwc3_layerscape_priv {
	void *base;
	struct dwc3 dwc3;
	struct phy_bulk phys;
};

struct dwc3_layerscape_host_priv {
	struct xhci_ctrl xhci_ctrl;
	struct dwc3_layerscape_priv gen_priv;
};

static int dwc3_layerscape_probe(struct udevice *dev,
				 struct dwc3_layerscape_priv *priv)
{
	int rc;
	struct dwc3_layerscape_plat *plat = dev_get_plat(dev);
	struct dwc3 *dwc3 = &priv->dwc3;

	dwc3->dev = dev;
	dwc3->maximum_speed = plat->maximum_speed;
	dwc3->dr_mode = plat->dr_mode;
	if (CONFIG_IS_ENABLED(OF_CONTROL))
		dwc3_of_parse(dwc3);

	rc = dwc3_setup_phy(dev, &priv->phys);
	if (rc && rc != -ENOTSUPP)
		return rc;

	priv->base = map_physmem(plat->base, DWC3_OTG_REGS_END, MAP_NOCACHE);
	dwc3->regs = priv->base + DWC3_GLOBALS_REGS_START;

	rc =  dwc3_init(dwc3);
	if (rc) {
		unmap_physmem(priv->base, MAP_NOCACHE);
		return rc;
	}

	return 0;
}

static int dwc3_layerscape_remove(struct udevice *dev,
				  struct dwc3_layerscape_priv *priv)
{
	struct dwc3 *dwc3 = &priv->dwc3;

	dwc3_remove(dwc3);
	dwc3_shutdown_phy(dev, &priv->phys);
	unmap_physmem(dwc3->regs, MAP_NOCACHE);

	return 0;
}

static int dwc3_layerscape_of_to_plat(struct udevice *dev)
{
	struct dwc3_layerscape_plat *plat = dev_get_plat(dev);
	ofnode node = dev_ofnode(dev);

	plat->base = dev_read_addr(dev);

	plat->maximum_speed = usb_get_maximum_speed(node);
	if (plat->maximum_speed == USB_SPEED_UNKNOWN) {
		dev_dbg(dev, "No USB maximum speed specified. Using super speed\n");
		plat->maximum_speed = USB_SPEED_SUPER;
	}

	plat->dr_mode = usb_get_dr_mode(node);
	if (plat->dr_mode == USB_DR_MODE_UNKNOWN) {
		dev_err(dev, "Invalid usb mode setup\n");
		return -ENODEV;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(DM_USB_GADGET)
int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	struct dwc3_layerscape_priv *priv = dev_get_priv(dev);

	dwc3_gadget_uboot_handle_interrupt(&priv->dwc3);

	return 0;
}

static int dwc3_layerscape_peripheral_probe(struct udevice *dev)
{
	struct dwc3_layerscape_priv *priv = dev_get_priv(dev);

	return dwc3_layerscape_probe(dev, priv);
}

static int dwc3_layerscape_peripheral_remove(struct udevice *dev)
{
	struct dwc3_layerscape_priv *priv = dev_get_priv(dev);

	return dwc3_layerscape_remove(dev, priv);
}

U_BOOT_DRIVER(dwc3_layerscape_peripheral) = {
	.name	= "dwc3-layerscape-peripheral",
	.id	= UCLASS_USB_GADGET_GENERIC,
	.of_to_plat = dwc3_layerscape_of_to_plat,
	.probe = dwc3_layerscape_peripheral_probe,
	.remove = dwc3_layerscape_peripheral_remove,
	.priv_auto	= sizeof(struct dwc3_layerscape_priv),
	.plat_auto	= sizeof(struct dwc3_layerscape_plat),
};
#endif

#if CONFIG_IS_ENABLED(USB_HOST)
static int dwc3_layerscape_host_probe(struct udevice *dev)
{
	struct xhci_hcor *hcor;
	struct xhci_hccr *hccr;
	struct dwc3_layerscape_host_priv *priv = dev_get_priv(dev);
	int rc;

	rc = dwc3_layerscape_probe(dev, &priv->gen_priv);
	if (rc)
		return rc;

	hccr = priv->gen_priv.base;
	hcor = priv->gen_priv.base + HC_LENGTH(xhci_readl(&hccr->cr_capbase));

	return xhci_register(dev, hccr, hcor);
}

static int dwc3_layerscape_host_remove(struct udevice *dev)
{
	struct dwc3_layerscape_host_priv *priv = dev_get_priv(dev);
	int rc;

	rc = xhci_deregister(dev);
	if (rc)
		return rc;

	return dwc3_layerscape_remove(dev, &priv->gen_priv);
}

U_BOOT_DRIVER(dwc3_layerscape_host) = {
	.name	= "dwc3-layerscape-host",
	.id	= UCLASS_USB,
	.of_to_plat = dwc3_layerscape_of_to_plat,
	.probe = dwc3_layerscape_host_probe,
	.remove = dwc3_layerscape_host_remove,
	.priv_auto	= sizeof(struct dwc3_layerscape_host_priv),
	.plat_auto	= sizeof(struct dwc3_layerscape_plat),
	.ops = &xhci_usb_ops,
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
#endif

static int dwc3_layerscape_bind(struct udevice *dev)
{
	ofnode node = dev_ofnode(dev);
	const char *name = ofnode_get_name(node);
	enum usb_dr_mode dr_mode;
	char *driver;

	dr_mode = usb_get_dr_mode(node);

	switch (dr_mode) {
#if CONFIG_IS_ENABLED(DM_USB_GADGET)
	case USB_DR_MODE_PERIPHERAL:
		dev_dbg(dev, "Using peripheral mode\n");
		driver = "dwc3-layerscape-peripheral";
		break;
#endif
#if CONFIG_IS_ENABLED(USB_HOST)
	case USB_DR_MODE_HOST:
		dev_dbg(dev, "Using host mode\n");
		driver = "dwc3-layerscape-host";
		break;
#endif
	default:
		dev_dbg(dev, "Unsupported dr_mode\n");
		return -ENODEV;
	};

	return device_bind_driver_to_node(dev, driver, name, node, NULL);
}

static const struct udevice_id dwc3_layerscape_ids[] = {
	{ .compatible = "fsl,layerscape-dwc3" },
	{ .compatible = "fsl,ls1028a-dwc3" },
	{ }
};

U_BOOT_DRIVER(dwc3_layerscape_wrapper) = {
	.name	= "dwc3-layerscape-wrapper",
	.id	= UCLASS_NOP,
	.of_match = dwc3_layerscape_ids,
	.bind = dwc3_layerscape_bind,
};
