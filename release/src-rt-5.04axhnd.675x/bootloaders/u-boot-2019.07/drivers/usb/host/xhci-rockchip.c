// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Rockchip, Inc.
 * Authors: Daniel Meng <daniel.meng@rock-chips.com>
 */
#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <usb.h>
#include <watchdog.h>
#include <linux/errno.h>
#include <linux/compat.h>
#include <linux/usb/dwc3.h>
#include <power/regulator.h>

#include "xhci.h"

struct rockchip_xhci_platdata {
	fdt_addr_t hcd_base;
	struct udevice *vbus_supply;
};

/*
 * Contains pointers to register base addresses
 * for the usb controller.
 */
struct rockchip_xhci {
	struct usb_platdata usb_plat;
	struct xhci_ctrl ctrl;
	struct xhci_hccr *hcd;
	struct dwc3 *dwc3_reg;
};

static int xhci_usb_ofdata_to_platdata(struct udevice *dev)
{
	struct rockchip_xhci_platdata *plat = dev_get_platdata(dev);
	int ret = 0;

	/*
	 * Get the base address for XHCI controller from the device node
	 */
	plat->hcd_base = dev_read_addr(dev);
	if (plat->hcd_base == FDT_ADDR_T_NONE) {
		pr_err("Can't get the XHCI register base address\n");
		return -ENXIO;
	}

	/* Vbus regulator */
	ret = device_get_supply_regulator(dev, "vbus-supply",
					  &plat->vbus_supply);
	if (ret)
		debug("Can't get VBus regulator!\n");

	return 0;
}

/*
 * rockchip_dwc3_phy_setup() - Configure USB PHY Interface of DWC3 Core
 * @dwc: Pointer to our controller context structure
 * @dev: Pointer to ulcass device
 */
static void rockchip_dwc3_phy_setup(struct dwc3 *dwc3_reg,
				    struct udevice *dev)
{
	u32 reg;
	u32 utmi_bits;

	/* Set dwc3 usb2 phy config */
	reg = readl(&dwc3_reg->g_usb2phycfg[0]);

	if (dev_read_bool(dev, "snps,dis-enblslpm-quirk"))
		reg &= ~DWC3_GUSB2PHYCFG_ENBLSLPM;

	utmi_bits = dev_read_u32_default(dev, "snps,phyif-utmi-bits", -1);
	if (utmi_bits == 16) {
		reg |= DWC3_GUSB2PHYCFG_PHYIF;
		reg &= ~DWC3_GUSB2PHYCFG_USBTRDTIM_MASK;
		reg |= DWC3_GUSB2PHYCFG_USBTRDTIM_16BIT;
	} else if (utmi_bits == 8) {
		reg &= ~DWC3_GUSB2PHYCFG_PHYIF;
		reg &= ~DWC3_GUSB2PHYCFG_USBTRDTIM_MASK;
		reg |= DWC3_GUSB2PHYCFG_USBTRDTIM_8BIT;
	}

	if (dev_read_bool(dev, "snps,dis-u2-freeclk-exists-quirk"))
		reg &= ~DWC3_GUSB2PHYCFG_U2_FREECLK_EXISTS;

	if (dev_read_bool(dev, "snps,dis-u2-susphy-quirk"))
		reg &= ~DWC3_GUSB2PHYCFG_SUSPHY;

	writel(reg, &dwc3_reg->g_usb2phycfg[0]);
}

static int rockchip_xhci_core_init(struct rockchip_xhci *rkxhci,
				   struct udevice *dev)
{
	int ret;

	ret = dwc3_core_init(rkxhci->dwc3_reg);
	if (ret) {
		pr_err("failed to initialize core\n");
		return ret;
	}

	rockchip_dwc3_phy_setup(rkxhci->dwc3_reg, dev);

	/* We are hard-coding DWC3 core to Host Mode */
	dwc3_set_mode(rkxhci->dwc3_reg, DWC3_GCTL_PRTCAP_HOST);

	return 0;
}

static int rockchip_xhci_core_exit(struct rockchip_xhci *rkxhci)
{
	return 0;
}

static int xhci_usb_probe(struct udevice *dev)
{
	struct rockchip_xhci_platdata *plat = dev_get_platdata(dev);
	struct rockchip_xhci *ctx = dev_get_priv(dev);
	struct xhci_hcor *hcor;
	int ret;

	ctx->hcd = (struct xhci_hccr *)plat->hcd_base;
	ctx->dwc3_reg = (struct dwc3 *)((char *)(ctx->hcd) + DWC3_REG_OFFSET);
	hcor = (struct xhci_hcor *)((uint64_t)ctx->hcd +
			HC_LENGTH(xhci_readl(&ctx->hcd->cr_capbase)));

	if (plat->vbus_supply) {
		ret = regulator_set_enable(plat->vbus_supply, true);
		if (ret) {
			pr_err("XHCI: failed to set VBus supply\n");
			return ret;
		}
	}

	ret = rockchip_xhci_core_init(ctx, dev);
	if (ret) {
		pr_err("XHCI: failed to initialize controller\n");
		return ret;
	}

	return xhci_register(dev, ctx->hcd, hcor);
}

static int xhci_usb_remove(struct udevice *dev)
{
	struct rockchip_xhci_platdata *plat = dev_get_platdata(dev);
	struct rockchip_xhci *ctx = dev_get_priv(dev);
	int ret;

	ret = xhci_deregister(dev);
	if (ret)
		return ret;
	ret = rockchip_xhci_core_exit(ctx);
	if (ret)
		return ret;

	if (plat->vbus_supply) {
		ret = regulator_set_enable(plat->vbus_supply, false);
		if (ret)
			pr_err("XHCI: failed to set VBus supply\n");
	}

	return ret;
}

static const struct udevice_id xhci_usb_ids[] = {
	{ .compatible = "rockchip,rk3399-xhci" },
	{ .compatible = "rockchip,rk3328-xhci" },
	{ }
};

U_BOOT_DRIVER(usb_xhci) = {
	.name	= "xhci_rockchip",
	.id	= UCLASS_USB,
	.of_match = xhci_usb_ids,
	.ofdata_to_platdata = xhci_usb_ofdata_to_platdata,
	.probe = xhci_usb_probe,
	.remove = xhci_usb_remove,
	.ops	= &xhci_usb_ops,
	.bind	= dm_scan_fdt_dev,
	.platdata_auto_alloc_size = sizeof(struct rockchip_xhci_platdata),
	.priv_auto_alloc_size = sizeof(struct rockchip_xhci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

static const struct udevice_id usb_phy_ids[] = {
	{ .compatible = "rockchip,rk3399-usb3-phy" },
	{ .compatible = "rockchip,rk3328-usb3-phy" },
	{ }
};

U_BOOT_DRIVER(usb_phy) = {
	.name = "usb_phy_rockchip",
	.of_match = usb_phy_ids,
};
