// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Sanchayan Maity <sanchayan.maity@toradex.com>
 * Copyright (C) 2015 Toradex AG
 *
 * Based on ehci-mx6 driver
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <errno.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/regs-usbphy.h>
#include <usb/ehci-ci.h>
#include <linux/libfdt.h>
#include <fdtdec.h>

#include "ehci.h"

#define USB_NC_REG_OFFSET				0x00000800

#define ANADIG_PLL_CTRL_EN_USB_CLKS		(1 << 6)

#define UCTRL_OVER_CUR_POL	(1 << 8) /* OTG Polarity of Overcurrent */
#define UCTRL_OVER_CUR_DIS	(1 << 7) /* Disable OTG Overcurrent Detection */

/* USBCMD */
#define UCMD_RUN_STOP		(1 << 0) /* controller run/stop */
#define UCMD_RESET			(1 << 1) /* controller reset */

DECLARE_GLOBAL_DATA_PTR;

static const unsigned phy_bases[] = {
	USB_PHY0_BASE_ADDR,
	USB_PHY1_BASE_ADDR,
};

static const unsigned nc_reg_bases[] = {
	USBC0_BASE_ADDR,
	USBC1_BASE_ADDR,
};

static void usb_internal_phy_clock_gate(int index)
{
	void __iomem *phy_reg;

	phy_reg = (void __iomem *)phy_bases[index];
	clrbits_le32(phy_reg + USBPHY_CTRL, USBPHY_CTRL_CLKGATE);
}

static void usb_power_config(int index)
{
	struct anadig_reg __iomem *anadig =
		(struct anadig_reg __iomem *)ANADIG_BASE_ADDR;
	void __iomem *pll_ctrl;

	switch (index) {
	case 0:
		pll_ctrl = &anadig->pll3_ctrl;
		clrbits_le32(pll_ctrl, ANADIG_PLL3_CTRL_BYPASS);
		setbits_le32(pll_ctrl, ANADIG_PLL3_CTRL_ENABLE
			 | ANADIG_PLL3_CTRL_POWERDOWN
			 | ANADIG_PLL_CTRL_EN_USB_CLKS);
		break;
	case 1:
		pll_ctrl = &anadig->pll7_ctrl;
		clrbits_le32(pll_ctrl, ANADIG_PLL7_CTRL_BYPASS);
		setbits_le32(pll_ctrl, ANADIG_PLL7_CTRL_ENABLE
			 | ANADIG_PLL7_CTRL_POWERDOWN
			 | ANADIG_PLL_CTRL_EN_USB_CLKS);
		break;
	default:
		return;
	}
}

static void usb_phy_enable(int index, struct usb_ehci *ehci)
{
	void __iomem *phy_reg;
	void __iomem *phy_ctrl;
	void __iomem *usb_cmd;

	phy_reg = (void __iomem *)phy_bases[index];
	phy_ctrl = (void __iomem *)(phy_reg + USBPHY_CTRL);
	usb_cmd = (void __iomem *)&ehci->usbcmd;

	/* Stop then Reset */
	clrbits_le32(usb_cmd, UCMD_RUN_STOP);
	while (readl(usb_cmd) & UCMD_RUN_STOP)
		;

	setbits_le32(usb_cmd, UCMD_RESET);
	while (readl(usb_cmd) & UCMD_RESET)
		;

	/* Reset USBPHY module */
	setbits_le32(phy_ctrl, USBPHY_CTRL_SFTRST);
	udelay(10);

	/* Remove CLKGATE and SFTRST */
	clrbits_le32(phy_ctrl, USBPHY_CTRL_CLKGATE | USBPHY_CTRL_SFTRST);
	udelay(10);

	/* Power up the PHY */
	writel(0, phy_reg + USBPHY_PWD);

	/* Enable FS/LS device */
	setbits_le32(phy_ctrl, USBPHY_CTRL_ENUTMILEVEL2 |
		 USBPHY_CTRL_ENUTMILEVEL3);
}

static void usb_oc_config(int index)
{
	void __iomem *ctrl;

	ctrl = (void __iomem *)(nc_reg_bases[index] + USB_NC_REG_OFFSET);

	setbits_le32(ctrl, UCTRL_OVER_CUR_POL);
	setbits_le32(ctrl, UCTRL_OVER_CUR_DIS);
}

int __weak board_usb_phy_mode(int port)
{
	return 0;
}

int __weak board_ehci_hcd_init(int port)
{
	return 0;
}

int ehci_vf_common_init(struct usb_ehci *ehci, int index)
{
	int ret;

	/* Do board specific initialisation */
	ret = board_ehci_hcd_init(index);
	if (ret)
		return ret;

	usb_power_config(index);
	usb_oc_config(index);
	usb_internal_phy_clock_gate(index);
	usb_phy_enable(index, ehci);

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_USB)
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct usb_ehci *ehci;
	enum usb_init_type type;
	int ret;

	if (index >= ARRAY_SIZE(nc_reg_bases))
		return -EINVAL;

	ehci = (struct usb_ehci *)nc_reg_bases[index];

	ret = ehci_vf_common_init(index);
	if (ret)
		return ret;

	*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	type = board_usb_phy_mode(index);
	if (type != init)
		return -ENODEV;

	if (init == USB_INIT_DEVICE) {
		setbits_le32(&ehci->usbmode, CM_DEVICE);
		writel((PORT_PTS_UTMI | PORT_PTS_PTW), &ehci->portsc);
		setbits_le32(&ehci->portsc, USB_EN);
	} else if (init == USB_INIT_HOST) {
		setbits_le32(&ehci->usbmode, CM_HOST);
		writel((PORT_PTS_UTMI | PORT_PTS_PTW), &ehci->portsc);
		setbits_le32(&ehci->portsc, USB_EN);
	}

	return 0;
}

int ehci_hcd_stop(int index)
{
	return 0;
}
#else
/* Possible port types (dual role mode) */
enum dr_mode {
	DR_MODE_NONE = 0,
	DR_MODE_HOST,		/* supports host operation */
	DR_MODE_DEVICE,		/* supports device operation */
	DR_MODE_OTG,		/* supports both */
};

struct ehci_vf_priv_data {
	struct ehci_ctrl ctrl;
	struct usb_ehci *ehci;
	struct gpio_desc cdet_gpio;
	enum usb_init_type init_type;
	enum dr_mode dr_mode;
	u32 portnr;
};

static int vf_usb_ofdata_to_platdata(struct udevice *dev)
{
	struct ehci_vf_priv_data *priv = dev_get_priv(dev);
	const void *dt_blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	const char *mode;

	priv->portnr = dev->seq;

	priv->ehci = (struct usb_ehci *)devfdt_get_addr(dev);
	mode = fdt_getprop(dt_blob, node, "dr_mode", NULL);
	if (mode) {
		if (0 == strcmp(mode, "host")) {
			priv->dr_mode = DR_MODE_HOST;
			priv->init_type = USB_INIT_HOST;
		} else if (0 == strcmp(mode, "peripheral")) {
			priv->dr_mode = DR_MODE_DEVICE;
			priv->init_type = USB_INIT_DEVICE;
		} else if (0 == strcmp(mode, "otg")) {
			priv->dr_mode = DR_MODE_OTG;
			/*
			 * We set init_type to device by default when OTG
			 * mode is requested. If a valid gpio is provided
			 * we will switch the init_type based on the state
			 * of the gpio pin.
			 */
			priv->init_type = USB_INIT_DEVICE;
		} else {
			debug("%s: Cannot decode dr_mode '%s'\n",
			      __func__, mode);
			return -EINVAL;
		}
	} else {
		priv->dr_mode = DR_MODE_HOST;
		priv->init_type = USB_INIT_HOST;
	}

	if (priv->dr_mode == DR_MODE_OTG) {
		gpio_request_by_name_nodev(offset_to_ofnode(node),
					   "fsl,cdet-gpio", 0, &priv->cdet_gpio,
					   GPIOD_IS_IN);
		if (dm_gpio_is_valid(&priv->cdet_gpio)) {
			if (dm_gpio_get_value(&priv->cdet_gpio))
				priv->init_type = USB_INIT_DEVICE;
			else
				priv->init_type = USB_INIT_HOST;
		}
	}

	return 0;
}

static int vf_init_after_reset(struct ehci_ctrl *dev)
{
	struct ehci_vf_priv_data *priv = dev->priv;
	enum usb_init_type type = priv->init_type;
	struct usb_ehci *ehci = priv->ehci;
	int ret;

	ret = ehci_vf_common_init(priv->ehci, priv->portnr);
	if (ret)
		return ret;

	if (type == USB_INIT_DEVICE)
		return 0;

	setbits_le32(&ehci->usbmode, CM_HOST);
	writel((PORT_PTS_UTMI | PORT_PTS_PTW), &ehci->portsc);
	setbits_le32(&ehci->portsc, USB_EN);

	mdelay(10);

	return 0;
}

static const struct ehci_ops vf_ehci_ops = {
	.init_after_reset = vf_init_after_reset
};

static int vf_usb_bind(struct udevice *dev)
{
	static int num_controllers;

	/*
	 * Without this hack, if we return ENODEV for USB Controller 0, on
	 * probe for the next controller, USB Controller 1 will be given a
	 * sequence number of 0. This conflicts with our requirement of
	 * sequence numbers while initialising the peripherals.
	 */
	dev->req_seq = num_controllers;
	num_controllers++;

	return 0;
}

static int ehci_usb_probe(struct udevice *dev)
{
	struct usb_platdata *plat = dev_get_platdata(dev);
	struct ehci_vf_priv_data *priv = dev_get_priv(dev);
	struct usb_ehci *ehci = priv->ehci;
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int ret;

	ret = ehci_vf_common_init(ehci, priv->portnr);
	if (ret)
		return ret;

	if (priv->init_type != plat->init_type)
		return -ENODEV;

	if (priv->init_type == USB_INIT_HOST) {
		setbits_le32(&ehci->usbmode, CM_HOST);
		writel((PORT_PTS_UTMI | PORT_PTS_PTW), &ehci->portsc);
		setbits_le32(&ehci->portsc, USB_EN);
	}

	mdelay(10);

	hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	hcor = (struct ehci_hcor *)((uint32_t)hccr +
				HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	return ehci_register(dev, hccr, hcor, &vf_ehci_ops, 0, priv->init_type);
}

static const struct udevice_id vf_usb_ids[] = {
	{ .compatible = "fsl,vf610-usb" },
	{ }
};

U_BOOT_DRIVER(usb_ehci) = {
	.name = "ehci_vf",
	.id = UCLASS_USB,
	.of_match = vf_usb_ids,
	.bind = vf_usb_bind,
	.probe = ehci_usb_probe,
	.remove = ehci_deregister,
	.ops = &ehci_usb_ops,
	.ofdata_to_platdata = vf_usb_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct ehci_vf_priv_data),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
#endif
