// SPDX-License-Identifier: GPL-2.0+
/*
 * OMAP USB HOST xHCI Controller
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Author: Dan Murphy <dmurphy@ti.com>
 */

#include <common.h>
#include <log.h>
#include <usb.h>
#include <linux/errno.h>
#include <asm/omap_common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>

#include <linux/compat.h>
#include <linux/usb/dwc3.h>
#include <linux/usb/xhci-omap.h>

#include <usb/xhci.h>

/* Declare global data pointer */
static struct omap_xhci omap;

static int omap_xhci_core_init(struct omap_xhci *omap)
{
	int ret = 0;

	usb_phy_power(1);
	omap_enable_phy(omap);

	ret = dwc3_core_init(omap->dwc3_reg);
	if (ret) {
		debug("%s:failed to initialize core\n", __func__);
		return ret;
	}

	/* We are hard-coding DWC3 core to Host Mode */
	dwc3_set_mode(omap->dwc3_reg, DWC3_GCTL_PRTCAP_HOST);

	return ret;
}

static void omap_xhci_core_exit(struct omap_xhci *omap)
{
	usb_phy_power(0);
}

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	struct omap_xhci *ctx = &omap;
	int ret = 0;

	ctx->hcd = (struct xhci_hccr *)OMAP_XHCI_BASE;
	ctx->dwc3_reg = (struct dwc3 *)((char *)(ctx->hcd) + DWC3_REG_OFFSET);
	ctx->usb3_phy = (struct omap_usb3_phy *)OMAP_OCP1_SCP_BASE;
	ctx->otg_wrapper = (struct omap_dwc_wrapper *)OMAP_OTG_WRAPPER_BASE;

	ret = board_usb_init(index, USB_INIT_HOST);
	if (ret != 0) {
		puts("Failed to initialize board for USB\n");
		return ret;
	}

	ret = omap_xhci_core_init(ctx);
	if (ret < 0) {
		puts("Failed to initialize xhci\n");
		return ret;
	}

	*hccr = (struct xhci_hccr *)(OMAP_XHCI_BASE);
	*hcor = (struct xhci_hcor *)((uint32_t) *hccr
				+ HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	debug("omap-xhci: init hccr %x and hcor %x hc_length %d\n",
	      (uint32_t)*hccr, (uint32_t)*hcor,
	      (uint32_t)HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return ret;
}

void xhci_hcd_stop(int index)
{
	struct omap_xhci *ctx = &omap;

	omap_xhci_core_exit(ctx);
	board_usb_cleanup(index, USB_INIT_HOST);
}
