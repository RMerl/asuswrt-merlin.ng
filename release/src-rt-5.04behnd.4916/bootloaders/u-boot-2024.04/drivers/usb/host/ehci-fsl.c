// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009, 2011, 2016 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2008, Excito Elektronik i Sk=E5ne AB
 *
 * Author: Tor Krill tor@excito.com
 */

#include <common.h>
#include <env.h>
#include <log.h>
#include <pci.h>
#include <usb.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <usb/ehci-ci.h>
#include <hwconfig.h>
#include <fsl_usb.h>
#include <fdt_support.h>
#include <dm.h>

#include "ehci.h"

DECLARE_GLOBAL_DATA_PTR;

struct ehci_fsl_priv {
	struct ehci_ctrl ehci;
	fdt_addr_t hcd_base;
	char *phy_type;
};

static void set_txfifothresh(struct usb_ehci *, u32);
static int ehci_fsl_init(struct ehci_fsl_priv *priv, struct usb_ehci *ehci,
		  struct ehci_hccr *hccr, struct ehci_hcor *hcor);

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

static int ehci_fsl_of_to_plat(struct udevice *dev)
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
	priv->hcd_base = dev_read_addr(dev);
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
	.of_to_plat = ehci_fsl_of_to_plat,
	.probe = ehci_fsl_probe,
	.remove = ehci_deregister,
	.ops	= &ehci_usb_ops,
	.plat_auto	= sizeof(struct usb_plat),
	.priv_auto	= sizeof(struct ehci_fsl_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

static int ehci_fsl_init(struct ehci_fsl_priv *priv, struct usb_ehci *ehci,
		  struct ehci_hccr *hccr, struct ehci_hcor *hcor)
{
	const char *phy_type = NULL;
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
	if (priv->phy_type)
		phy_type = priv->phy_type;
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
