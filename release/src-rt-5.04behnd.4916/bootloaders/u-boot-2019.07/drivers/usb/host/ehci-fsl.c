// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009, 2011, 2016 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2008, Excito Elektronik i Sk=E5ne AB
 *
 * Author: Tor Krill tor@excito.com
 */

#include <common.h>
#include <pci.h>
#include <usb.h>
#include <asm/io.h>
#include <usb/ehci-ci.h>
#include <hwconfig.h>
#include <fsl_usb.h>
#include <fdt_support.h>
#include <dm.h>

#include "ehci.h"

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#endif

#if CONFIG_IS_ENABLED(DM_USB)
struct ehci_fsl_priv {
	struct ehci_ctrl ehci;
	fdt_addr_t hcd_base;
	char *phy_type;
};
#endif

static void set_txfifothresh(struct usb_ehci *, u32);
#if CONFIG_IS_ENABLED(DM_USB)
static int ehci_fsl_init(struct ehci_fsl_priv *priv, struct usb_ehci *ehci,
		  struct ehci_hccr *hccr, struct ehci_hcor *hcor);
#else
static int ehci_fsl_init(int index, struct usb_ehci *ehci,
			 struct ehci_hccr *hccr, struct ehci_hcor *hcor);
#endif

/* Check USB PHY clock valid */
static int usb_phy_clk_valid(struct usb_ehci *ehci)
{
	if (!((in_be32(&ehci->control) & PHY_CLK_VALID) ||
			in_be32(&ehci->prictrl))) {
		printf("USB PHY clock invalid!\n");
		return 0;
	} else {
		return 1;
	}
}

#if CONFIG_IS_ENABLED(DM_USB)
static int ehci_fsl_ofdata_to_platdata(struct udevice *dev)
{
	struct ehci_fsl_priv *priv = dev_get_priv(dev);
	const void *prop;

	prop = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "phy_type",
			   NULL);
	if (prop) {
		priv->phy_type = (char *)prop;
		debug("phy_type %s\n", priv->phy_type);
	}

	return 0;
}

static int ehci_fsl_init_after_reset(struct ehci_ctrl *ctrl)
{
	struct usb_ehci *ehci = NULL;
	struct ehci_fsl_priv *priv = container_of(ctrl, struct ehci_fsl_priv,
						   ehci);
#ifdef CONFIG_PPC
	ehci = (struct usb_ehci *)lower_32_bits(priv->hcd_base);
#else
	ehci = (struct usb_ehci *)priv->hcd_base;
#endif

	if (ehci_fsl_init(priv, ehci, priv->ehci.hccr, priv->ehci.hcor) < 0)
		return -ENXIO;

	return 0;
}

static const struct ehci_ops fsl_ehci_ops = {
	.init_after_reset = ehci_fsl_init_after_reset,
};

static int ehci_fsl_probe(struct udevice *dev)
{
	struct ehci_fsl_priv *priv = dev_get_priv(dev);
	struct usb_ehci *ehci = NULL;
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	struct ehci_ctrl *ehci_ctrl = &priv->ehci;

	/*
	 * Get the base address for EHCI controller from the device node
	 */
	priv->hcd_base = devfdt_get_addr(dev);
	if (priv->hcd_base == FDT_ADDR_T_NONE) {
		debug("Can't get the EHCI register base address\n");
		return -ENXIO;
	}
#ifdef CONFIG_PPC
	ehci = (struct usb_ehci *)lower_32_bits(priv->hcd_base);
#else
	ehci = (struct usb_ehci *)priv->hcd_base;
#endif
	hccr = (struct ehci_hccr *)(&ehci->caplength);
	hcor = (struct ehci_hcor *)
		((void *)hccr + HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	ehci_ctrl->has_fsl_erratum_a005275 = has_erratum_a005275();

	if (ehci_fsl_init(priv, ehci, hccr, hcor) < 0)
		return -ENXIO;

	debug("ehci-fsl: init hccr %p and hcor %p hc_length %d\n",
	      (void *)hccr, (void *)hcor,
	      HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	return ehci_register(dev, hccr, hcor, &fsl_ehci_ops, 0, USB_INIT_HOST);
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "fsl-usb2-mph", },
	{ .compatible = "fsl-usb2-dr", },
	{ }
};

U_BOOT_DRIVER(ehci_fsl) = {
	.name	= "ehci_fsl",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.ofdata_to_platdata = ehci_fsl_ofdata_to_platdata,
	.probe = ehci_fsl_probe,
	.remove = ehci_deregister,
	.ops	= &ehci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct ehci_fsl_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
#else
/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 *
 * Excerpts from linux ehci fsl driver.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct ehci_ctrl *ehci_ctrl = container_of(hccr,
					struct ehci_ctrl, hccr);
	struct usb_ehci *ehci = NULL;

	switch (index) {
	case 0:
		ehci = (struct usb_ehci *)CONFIG_SYS_FSL_USB1_ADDR;
		break;
	case 1:
		ehci = (struct usb_ehci *)CONFIG_SYS_FSL_USB2_ADDR;
		break;
	default:
		printf("ERROR: wrong controller index!!\n");
		return -EINVAL;
	};

	*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	ehci_ctrl->has_fsl_erratum_a005275 = has_erratum_a005275();

	return ehci_fsl_init(index, ehci, *hccr, *hcor);
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	return 0;
}
#endif

#if CONFIG_IS_ENABLED(DM_USB)
static int ehci_fsl_init(struct ehci_fsl_priv *priv, struct usb_ehci *ehci,
		  struct ehci_hccr *hccr, struct ehci_hcor *hcor)
#else
static int ehci_fsl_init(int index, struct usb_ehci *ehci,
			 struct ehci_hccr *hccr, struct ehci_hcor *hcor)
#endif
{
	const char *phy_type = NULL;
#if !CONFIG_IS_ENABLED(DM_USB)
	size_t len;
	char current_usb_controller[5];
#endif
#ifdef CONFIG_SYS_FSL_USB_INTERNAL_UTMI_PHY
	char usb_phy[5];

	usb_phy[0] = '\0';
#endif
	if (has_erratum_a007075()) {
		/*
		 * A 5ms delay is needed after applying soft-reset to the
		 * controller to let external ULPI phy come out of reset.
		 * This delay needs to be added before re-initializing
		 * the controller after soft-resetting completes
		 */
		mdelay(5);
	}

	/* Set to Host mode */
	setbits_le32(&ehci->usbmode, CM_HOST);

	out_be32(&ehci->snoop1, SNOOP_SIZE_2GB);
	out_be32(&ehci->snoop2, 0x80000000 | SNOOP_SIZE_2GB);

	/* Init phy */
#if CONFIG_IS_ENABLED(DM_USB)
	if (priv->phy_type)
		phy_type = priv->phy_type;
#else
	memset(current_usb_controller, '\0', 5);
	snprintf(current_usb_controller, sizeof(current_usb_controller),
		 "usb%d", index+1);

	if (hwconfig_sub(current_usb_controller, "phy_type"))
		phy_type = hwconfig_subarg(current_usb_controller,
				"phy_type", &len);
#endif
	else
		phy_type = env_get("usb_phy_type");

	if (!phy_type) {
#ifdef CONFIG_SYS_FSL_USB_INTERNAL_UTMI_PHY
		/* if none specified assume internal UTMI */
		strcpy(usb_phy, "utmi");
		phy_type = usb_phy;
#else
		printf("WARNING: USB phy type not defined !!\n");
		return -1;
#endif
	}

	if (!strncmp(phy_type, "utmi", 4)) {
#if defined(CONFIG_SYS_FSL_USB_INTERNAL_UTMI_PHY)
		clrsetbits_be32(&ehci->control, CONTROL_REGISTER_W1C_MASK,
				PHY_CLK_SEL_UTMI);
		clrsetbits_be32(&ehci->control, CONTROL_REGISTER_W1C_MASK,
				UTMI_PHY_EN);
		udelay(1000); /* delay required for PHY Clk to appear */
#endif
		out_le32(&(hcor)->or_portsc[0], PORT_PTS_UTMI);
		clrsetbits_be32(&ehci->control, CONTROL_REGISTER_W1C_MASK,
				USB_EN);
	} else {
		clrsetbits_be32(&ehci->control, CONTROL_REGISTER_W1C_MASK,
				PHY_CLK_SEL_ULPI);
		clrsetbits_be32(&ehci->control, UTMI_PHY_EN |
				CONTROL_REGISTER_W1C_MASK, USB_EN);
		udelay(1000); /* delay required for PHY Clk to appear */
		if (!usb_phy_clk_valid(ehci))
			return -EINVAL;
		out_le32(&(hcor)->or_portsc[0], PORT_PTS_ULPI);
	}

	out_be32(&ehci->prictrl, 0x0000000c);
	out_be32(&ehci->age_cnt_limit, 0x00000040);
	out_be32(&ehci->sictrl, 0x00000001);

	in_le32(&ehci->usbmode);

	if (has_erratum_a007798())
		set_txfifothresh(ehci, TXFIFOTHRESH);

	if (has_erratum_a004477()) {
		/*
		 * When reset is issued while any ULPI transaction is ongoing
		 * then it may result to corruption of ULPI Function Control
		 * Register which eventually causes phy clock to enter low
		 * power mode which stops the clock. Thus delay is required
		 * before reset to let ongoing ULPI transaction complete.
		 */
		udelay(1);
	}
	return 0;
}

/*
 * Setting the value of TXFIFO_THRESH field in TXFILLTUNING register
 * to counter DDR latencies in writing data into Tx buffer.
 * This prevents Tx buffer from getting underrun
 */
static void set_txfifothresh(struct usb_ehci *ehci, u32 txfifo_thresh)
{
	u32 cmd;
	cmd = ehci_readl(&ehci->txfilltuning);
	cmd &= ~TXFIFO_THRESH_MASK;
	cmd |= TXFIFO_THRESH(txfifo_thresh);
	ehci_writel(&ehci->txfilltuning, cmd);
}
