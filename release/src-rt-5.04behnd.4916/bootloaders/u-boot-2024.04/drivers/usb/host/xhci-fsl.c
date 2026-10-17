// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015,2016 Freescale Semiconductor, Inc.
 *
 * FSL USB HOST xHCI Controller
 *
 * Author: Ramneek Mehresh<ramneek.mehresh@freescale.com>
 */

#include <common.h>
#include <log.h>
#include <usb.h>
#include <linux/errno.h>
#include <linux/compat.h>
#include <linux/usb/xhci-fsl.h>
#include <linux/usb/dwc3.h>
#include <usb/xhci.h>
#include <fsl_errata.h>
#include <fsl_usb.h>
#include <dm.h>

/* Declare global data pointer */
struct xhci_fsl_priv {
	struct xhci_ctrl xhci;
	fdt_addr_t hcd_base;
	struct fsl_xhci ctx;
};

__weak int __board_usb_init(int index, enum usb_init_type init)
{
	return 0;
}

static int erratum_a008751(void)
{
#if defined(CONFIG_TARGET_LS2080AQDS) || defined(CONFIG_TARGET_LS2080ARDB) ||\
					defined(CONFIG_TARGET_LS2080AQDS)
	u32 __iomem *scfg = (u32 __iomem *)SCFG_BASE;
	writel(SCFG_USB3PRM1CR_INIT, scfg + SCFG_USB3PRM1CR / 4);
	return 0;
#endif
	return 1;
}

static void fsl_apply_xhci_errata(void)
{
	int ret;
	if (has_erratum_a008751()) {
		ret = erratum_a008751();
		if (ret != 0)
			puts("Failed to apply erratum a008751\n");
	}
}

static void fsl_xhci_set_beat_burst_length(struct dwc3 *dwc3_reg)
{
	clrsetbits_le32(&dwc3_reg->g_sbuscfg0, USB3_ENABLE_BEAT_BURST_MASK,
			USB3_ENABLE_BEAT_BURST);
	setbits_le32(&dwc3_reg->g_sbuscfg1, USB3_SET_BEAT_BURST_LIMIT);
}

static int fsl_xhci_core_init(struct fsl_xhci *fsl_xhci)
{
	int ret = 0;

	ret = dwc3_core_init(fsl_xhci->dwc3_reg);
	if (ret) {
		debug("%s:failed to initialize core\n", __func__);
		return ret;
	}

	/* We are hard-coding DWC3 core to Host Mode */
	dwc3_set_mode(fsl_xhci->dwc3_reg, DWC3_GCTL_PRTCAP_HOST);

	/* Set GFLADJ_30MHZ as 20h as per XHCI spec default value */
	dwc3_set_fladj(fsl_xhci->dwc3_reg, GFLADJ_30MHZ_DEFAULT);

	/* Change beat burst and outstanding pipelined transfers requests */
	fsl_xhci_set_beat_burst_length(fsl_xhci->dwc3_reg);

	/*
	 * A-010151: The dwc3 phy TSMC 28-nm HPM 0.9/1.8 V does not
	 * reliably support Rx Detect in P3 mode(P3 is the default
	 * setting). Therefore, some USB3.0 devices may not be detected
	 * reliably in Super Speed mode. So, USB controller to configure
	 * USB in P2 mode whenever the Receive Detect feature is required.
	 * whenever the Receive Detect feature is required.
	 */
	if (has_erratum_a010151())
		clrsetbits_le32(&fsl_xhci->dwc3_reg->g_usb3pipectl[0],
				DWC3_GUSB3PIPECTL_DISRXDETP3,
				DWC3_GUSB3PIPECTL_DISRXDETP3);

	return ret;
}

static int fsl_xhci_core_exit(struct fsl_xhci *fsl_xhci)
{
	/*
	 * Currently fsl socs do not support PHY shutdown from
	 * sw. But this support may be added in future socs.
	 */
	return 0;
}

static int xhci_fsl_probe(struct udevice *dev)
{
	struct xhci_fsl_priv *priv = dev_get_priv(dev);
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;

	int ret = 0;

	/*
	 * Get the base address for XHCI controller from the device node
	 */
	priv->hcd_base = dev_read_addr(dev);
	if (priv->hcd_base == FDT_ADDR_T_NONE) {
		debug("Can't get the XHCI register base address\n");
		return -ENXIO;
	}
	priv->ctx.hcd = (struct xhci_hccr *)priv->hcd_base;
	priv->ctx.dwc3_reg = (struct dwc3 *)((char *)(priv->hcd_base) +
			  DWC3_REG_OFFSET);

	fsl_apply_xhci_errata();

	ret = fsl_xhci_core_init(&priv->ctx);
	if (ret < 0) {
		puts("Failed to initialize xhci\n");
		return ret;
	}

	hccr = (struct xhci_hccr *)(priv->ctx.hcd);
	hcor = (struct xhci_hcor *)((uintptr_t) hccr
				+ HC_LENGTH(xhci_readl(&hccr->cr_capbase)));

	debug("xhci-fsl: init hccr %lx and hcor %lx hc_length %lx\n",
	      (uintptr_t)hccr, (uintptr_t)hcor,
	      (uintptr_t)HC_LENGTH(xhci_readl(&hccr->cr_capbase)));

	return xhci_register(dev, hccr, hcor);
}

static int xhci_fsl_remove(struct udevice *dev)
{
	struct xhci_fsl_priv *priv = dev_get_priv(dev);

	fsl_xhci_core_exit(&priv->ctx);

	return xhci_deregister(dev);
}

static const struct udevice_id xhci_usb_ids[] = {
	{ .compatible = "fsl,layerscape-dwc3", },
	{ .compatible = "fsl,ls1028a-dwc3", },
	{ }
};

U_BOOT_DRIVER(xhci_fsl) = {
	.name	= "xhci_fsl",
	.id	= UCLASS_USB,
	.of_match = xhci_usb_ids,
	.probe = xhci_fsl_probe,
	.remove = xhci_fsl_remove,
	.ops	= &xhci_usb_ops,
	.plat_auto	= sizeof(struct usb_plat),
	.priv_auto	= sizeof(struct xhci_fsl_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
