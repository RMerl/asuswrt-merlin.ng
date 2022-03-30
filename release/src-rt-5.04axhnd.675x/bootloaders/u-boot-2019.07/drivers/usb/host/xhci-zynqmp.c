// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Xilinx, Inc.
 *
 * Zynq USB HOST xHCI Controller
 *
 * Author: Siva Durga Prasad Paladugu<sivadur@xilinx.com>
 *
 * This file was reused from Freescale USB xHCI
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <linux/errno.h>
#include <asm/arch/hardware.h>
#include <linux/compat.h>
#include <linux/usb/dwc3.h>
#include "xhci.h"

/* Declare global data pointer */
/* Default to the ZYNQMP XHCI defines */
#define USB3_PWRCTL_CLK_CMD_MASK	0x3FE000
#define USB3_PWRCTL_CLK_FREQ_MASK	0xFFC
#define USB3_PHY_PARTIAL_RX_POWERON     BIT(6)
#define USB3_PHY_RX_POWERON		BIT(14)
#define USB3_PHY_TX_POWERON		BIT(15)
#define USB3_PHY_TX_RX_POWERON	(USB3_PHY_RX_POWERON | USB3_PHY_TX_POWERON)
#define USB3_PWRCTL_CLK_CMD_SHIFT   14
#define USB3_PWRCTL_CLK_FREQ_SHIFT	22

/* USBOTGSS_WRAPPER definitions */
#define USBOTGSS_WRAPRESET	BIT(17)
#define USBOTGSS_DMADISABLE BIT(16)
#define USBOTGSS_STANDBYMODE_NO_STANDBY BIT(4)
#define USBOTGSS_STANDBYMODE_SMRT		BIT(5)
#define USBOTGSS_STANDBYMODE_SMRT_WKUP (0x3 << 4)
#define USBOTGSS_IDLEMODE_NOIDLE BIT(2)
#define USBOTGSS_IDLEMODE_SMRT BIT(3)
#define USBOTGSS_IDLEMODE_SMRT_WKUP (0x3 << 2)

/* USBOTGSS_IRQENABLE_SET_0 bit */
#define USBOTGSS_COREIRQ_EN	BIT(1)

/* USBOTGSS_IRQENABLE_SET_1 bits */
#define USBOTGSS_IRQ_SET_1_IDPULLUP_FALL_EN	BIT(1)
#define USBOTGSS_IRQ_SET_1_DISCHRGVBUS_FALL_EN	BIT(3)
#define USBOTGSS_IRQ_SET_1_CHRGVBUS_FALL_EN	BIT(4)
#define USBOTGSS_IRQ_SET_1_DRVVBUS_FALL_EN	BIT(5)
#define USBOTGSS_IRQ_SET_1_IDPULLUP_RISE_EN	BIT(8)
#define USBOTGSS_IRQ_SET_1_DISCHRGVBUS_RISE_EN	BIT(11)
#define USBOTGSS_IRQ_SET_1_CHRGVBUS_RISE_EN	BIT(12)
#define USBOTGSS_IRQ_SET_1_DRVVBUS_RISE_EN	BIT(13)
#define USBOTGSS_IRQ_SET_1_OEVT_EN		BIT(16)
#define USBOTGSS_IRQ_SET_1_DMADISABLECLR_EN	BIT(17)

struct zynqmp_xhci {
	struct usb_platdata usb_plat;
	struct xhci_ctrl ctrl;
	struct xhci_hccr *hcd;
	struct dwc3 *dwc3_reg;
};

struct zynqmp_xhci_platdata {
	fdt_addr_t hcd_base;
};

static int zynqmp_xhci_core_init(struct zynqmp_xhci *zynqmp_xhci)
{
	int ret = 0;

	ret = dwc3_core_init(zynqmp_xhci->dwc3_reg);
	if (ret) {
		debug("%s:failed to initialize core\n", __func__);
		return ret;
	}

	/* We are hard-coding DWC3 core to Host Mode */
	dwc3_set_mode(zynqmp_xhci->dwc3_reg, DWC3_GCTL_PRTCAP_HOST);

	return ret;
}

void xhci_hcd_stop(int index)
{
	/*
	 * Currently zynqmp socs do not support PHY shutdown from
	 * sw. But this support may be added in future socs.
	 */

	return;
}

static int xhci_usb_probe(struct udevice *dev)
{
	struct zynqmp_xhci_platdata *plat = dev_get_platdata(dev);
	struct zynqmp_xhci *ctx = dev_get_priv(dev);
	struct xhci_hcor *hcor;
	int ret;

	ctx->hcd = (struct xhci_hccr *)plat->hcd_base;
	ctx->dwc3_reg = (struct dwc3 *)((char *)(ctx->hcd) + DWC3_REG_OFFSET);

	ret = zynqmp_xhci_core_init(ctx);
	if (ret) {
		puts("XHCI: failed to initialize controller\n");
		return -EINVAL;
	}

	hcor = (struct xhci_hcor *)((ulong)ctx->hcd +
				  HC_LENGTH(xhci_readl(&ctx->hcd->cr_capbase)));

	return xhci_register(dev, ctx->hcd, hcor);
}

static int xhci_usb_remove(struct udevice *dev)
{
	return xhci_deregister(dev);
}

static int xhci_usb_ofdata_to_platdata(struct udevice *dev)
{
	struct zynqmp_xhci_platdata *plat = dev_get_platdata(dev);
	const void *blob = gd->fdt_blob;

	/* Get the base address for XHCI controller from the device node */
	plat->hcd_base = fdtdec_get_addr(blob, dev_of_offset(dev), "reg");
	if (plat->hcd_base == FDT_ADDR_T_NONE) {
		debug("Can't get the XHCI register base address\n");
		return -ENXIO;
	}

	return 0;
}

U_BOOT_DRIVER(dwc3_generic_host) = {
	.name = "dwc3-generic-host",
	.id = UCLASS_USB,
	.ofdata_to_platdata = xhci_usb_ofdata_to_platdata,
	.probe = xhci_usb_probe,
	.remove = xhci_usb_remove,
	.ops = &xhci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct zynqmp_xhci_platdata),
	.priv_auto_alloc_size = sizeof(struct zynqmp_xhci),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
