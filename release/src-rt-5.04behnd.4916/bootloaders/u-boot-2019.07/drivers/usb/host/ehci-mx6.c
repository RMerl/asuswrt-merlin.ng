// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2009 Daniel Mack <daniel@caiaq.de>
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <usb.h>
#include <errno.h>
#include <wait_bit.h>
#include <linux/compiler.h>
#include <usb/ehci-ci.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/sys_proto.h>
#include <dm.h>
#include <asm/mach-types.h>
#include <power/regulator.h>
#include <linux/usb/otg.h>

#include "ehci.h"

DECLARE_GLOBAL_DATA_PTR;

#define USB_OTGREGS_OFFSET	0x000
#define USB_H1REGS_OFFSET	0x200
#define USB_H2REGS_OFFSET	0x400
#define USB_H3REGS_OFFSET	0x600
#define USB_OTHERREGS_OFFSET	0x800

#define USB_H1_CTRL_OFFSET	0x04

#define USBPHY_CTRL				0x00000030
#define USBPHY_CTRL_SET				0x00000034
#define USBPHY_CTRL_CLR				0x00000038
#define USBPHY_CTRL_TOG				0x0000003c

#define USBPHY_PWD				0x00000000
#define USBPHY_CTRL_SFTRST			0x80000000
#define USBPHY_CTRL_CLKGATE			0x40000000
#define USBPHY_CTRL_ENUTMILEVEL3		0x00008000
#define USBPHY_CTRL_ENUTMILEVEL2		0x00004000
#define USBPHY_CTRL_OTG_ID			0x08000000

#define ANADIG_USB2_CHRG_DETECT_EN_B		0x00100000
#define ANADIG_USB2_CHRG_DETECT_CHK_CHRG_B	0x00080000

#define ANADIG_USB2_PLL_480_CTRL_BYPASS		0x00010000
#define ANADIG_USB2_PLL_480_CTRL_ENABLE		0x00002000
#define ANADIG_USB2_PLL_480_CTRL_POWER		0x00001000
#define ANADIG_USB2_PLL_480_CTRL_EN_USB_CLKS	0x00000040

#define USBNC_OFFSET		0x200
#define USBNC_PHY_STATUS_OFFSET	0x23C
#define USBNC_PHYSTATUS_ID_DIG	(1 << 4) /* otg_id status */
#define USBNC_PHYCFG2_ACAENB	(1 << 4) /* otg_id detection enable */
#define UCTRL_PWR_POL		(1 << 9) /* OTG Polarity of Power Pin */
#define UCTRL_OVER_CUR_POL	(1 << 8) /* OTG Polarity of Overcurrent */
#define UCTRL_OVER_CUR_DIS	(1 << 7) /* Disable OTG Overcurrent Detection */

/* USBCMD */
#define UCMD_RUN_STOP           (1 << 0) /* controller run/stop */
#define UCMD_RESET		(1 << 1) /* controller reset */

#if defined(CONFIG_MX6)
static const unsigned phy_bases[] = {
	USB_PHY0_BASE_ADDR,
	USB_PHY1_BASE_ADDR,
};

static void usb_internal_phy_clock_gate(int index, int on)
{
	void __iomem *phy_reg;

	if (index >= ARRAY_SIZE(phy_bases))
		return;

	phy_reg = (void __iomem *)phy_bases[index];
	phy_reg += on ? USBPHY_CTRL_CLR : USBPHY_CTRL_SET;
	writel(USBPHY_CTRL_CLKGATE, phy_reg);
}

static void usb_power_config(int index)
{
	struct anatop_regs __iomem *anatop =
		(struct anatop_regs __iomem *)ANATOP_BASE_ADDR;
	void __iomem *chrg_detect;
	void __iomem *pll_480_ctrl_clr;
	void __iomem *pll_480_ctrl_set;

	switch (index) {
	case 0:
		chrg_detect = &anatop->usb1_chrg_detect;
		pll_480_ctrl_clr = &anatop->usb1_pll_480_ctrl_clr;
		pll_480_ctrl_set = &anatop->usb1_pll_480_ctrl_set;
		break;
	case 1:
		chrg_detect = &anatop->usb2_chrg_detect;
		pll_480_ctrl_clr = &anatop->usb2_pll_480_ctrl_clr;
		pll_480_ctrl_set = &anatop->usb2_pll_480_ctrl_set;
		break;
	default:
		return;
	}
	/*
	 * Some phy and power's special controls
	 * 1. The external charger detector needs to be disabled
	 * or the signal at DP will be poor
	 * 2. The PLL's power and output to usb
	 * is totally controlled by IC, so the Software only needs
	 * to enable them at initializtion.
	 */
	writel(ANADIG_USB2_CHRG_DETECT_EN_B |
		     ANADIG_USB2_CHRG_DETECT_CHK_CHRG_B,
		     chrg_detect);

	writel(ANADIG_USB2_PLL_480_CTRL_BYPASS,
		     pll_480_ctrl_clr);

	writel(ANADIG_USB2_PLL_480_CTRL_ENABLE |
		     ANADIG_USB2_PLL_480_CTRL_POWER |
		     ANADIG_USB2_PLL_480_CTRL_EN_USB_CLKS,
		     pll_480_ctrl_set);
}

/* Return 0 : host node, <>0 : device mode */
static int usb_phy_enable(int index, struct usb_ehci *ehci)
{
	void __iomem *phy_reg;
	void __iomem *phy_ctrl;
	void __iomem *usb_cmd;
	int ret;

	if (index >= ARRAY_SIZE(phy_bases))
		return 0;

	phy_reg = (void __iomem *)phy_bases[index];
	phy_ctrl = (void __iomem *)(phy_reg + USBPHY_CTRL);
	usb_cmd = (void __iomem *)&ehci->usbcmd;

	/* Stop then Reset */
	clrbits_le32(usb_cmd, UCMD_RUN_STOP);
	ret = wait_for_bit_le32(usb_cmd, UCMD_RUN_STOP, false, 10000, false);
	if (ret)
		return ret;

	setbits_le32(usb_cmd, UCMD_RESET);
	ret = wait_for_bit_le32(usb_cmd, UCMD_RESET, false, 10000, false);
	if (ret)
		return ret;

	/* Reset USBPHY module */
	setbits_le32(phy_ctrl, USBPHY_CTRL_SFTRST);
	udelay(10);

	/* Remove CLKGATE and SFTRST */
	clrbits_le32(phy_ctrl, USBPHY_CTRL_CLKGATE | USBPHY_CTRL_SFTRST);
	udelay(10);

	/* Power up the PHY */
	writel(0, phy_reg + USBPHY_PWD);
	/* enable FS/LS device */
	setbits_le32(phy_ctrl, USBPHY_CTRL_ENUTMILEVEL2 |
			USBPHY_CTRL_ENUTMILEVEL3);

	return 0;
}

int usb_phy_mode(int port)
{
	void __iomem *phy_reg;
	void __iomem *phy_ctrl;
	u32 val;

	phy_reg = (void __iomem *)phy_bases[port];
	phy_ctrl = (void __iomem *)(phy_reg + USBPHY_CTRL);

	val = readl(phy_ctrl);

	if (val & USBPHY_CTRL_OTG_ID)
		return USB_INIT_DEVICE;
	else
		return USB_INIT_HOST;
}

/* Base address for this IP block is 0x02184800 */
struct usbnc_regs {
	u32	ctrl[4];	/* otg/host1-3 */
	u32	uh2_hsic_ctrl;
	u32	uh3_hsic_ctrl;
	u32	otg_phy_ctrl_0;
	u32	uh1_phy_ctrl_0;
};
#elif defined(CONFIG_MX7)
struct usbnc_regs {
	u32 ctrl1;
	u32 ctrl2;
	u32 reserve1[10];
	u32 phy_cfg1;
	u32 phy_cfg2;
	u32 reserve2;
	u32 phy_status;
	u32 reserve3[4];
	u32 adp_cfg1;
	u32 adp_cfg2;
	u32 adp_status;
};

static void usb_power_config(int index)
{
	struct usbnc_regs *usbnc = (struct usbnc_regs *)(USB_BASE_ADDR +
			(0x10000 * index) + USBNC_OFFSET);
	void __iomem *phy_cfg2 = (void __iomem *)(&usbnc->phy_cfg2);
	void __iomem *ctrl = (void __iomem *)(&usbnc->ctrl1);

	/*
	 * Clear the ACAENB to enable usb_otg_id detection,
	 * otherwise it is the ACA detection enabled.
	 */
	clrbits_le32(phy_cfg2, USBNC_PHYCFG2_ACAENB);

	/* Set power polarity to high active */
#ifdef CONFIG_MXC_USB_OTG_HACTIVE
	setbits_le32(ctrl, UCTRL_PWR_POL);
#else
	clrbits_le32(ctrl, UCTRL_PWR_POL);
#endif
}

int usb_phy_mode(int port)
{
	struct usbnc_regs *usbnc = (struct usbnc_regs *)(USB_BASE_ADDR +
			(0x10000 * port) + USBNC_OFFSET);
	void __iomem *status = (void __iomem *)(&usbnc->phy_status);
	u32 val;

	val = readl(status);

	if (val & USBNC_PHYSTATUS_ID_DIG)
		return USB_INIT_DEVICE;
	else
		return USB_INIT_HOST;
}
#endif

static void usb_oc_config(int index)
{
#if defined(CONFIG_MX6)
	struct usbnc_regs *usbnc = (struct usbnc_regs *)(USB_BASE_ADDR +
			USB_OTHERREGS_OFFSET);
	void __iomem *ctrl = (void __iomem *)(&usbnc->ctrl[index]);
#elif defined(CONFIG_MX7)
	struct usbnc_regs *usbnc = (struct usbnc_regs *)(USB_BASE_ADDR +
			(0x10000 * index) + USBNC_OFFSET);
	void __iomem *ctrl = (void __iomem *)(&usbnc->ctrl1);
#endif

#if CONFIG_MACH_TYPE == MACH_TYPE_MX6Q_ARM2
	/* mx6qarm2 seems to required a different setting*/
	clrbits_le32(ctrl, UCTRL_OVER_CUR_POL);
#else
	setbits_le32(ctrl, UCTRL_OVER_CUR_POL);
#endif

	setbits_le32(ctrl, UCTRL_OVER_CUR_DIS);
}

/**
 * board_usb_phy_mode - override usb phy mode
 * @port:	usb host/otg port
 *
 * Target board specific, override usb_phy_mode.
 * When usb-otg is used as usb host port, iomux pad usb_otg_id can be
 * left disconnected in this case usb_phy_mode will not be able to identify
 * the phy mode that usb port is used.
 * Machine file overrides board_usb_phy_mode.
 *
 * Return: USB_INIT_DEVICE or USB_INIT_HOST
 */
int __weak board_usb_phy_mode(int port)
{
	return usb_phy_mode(port);
}

/**
 * board_ehci_hcd_init - set usb vbus voltage
 * @port:      usb otg port
 *
 * Target board specific, setup iomux pad to setup supply vbus voltage
 * for usb otg port. Machine board file overrides board_ehci_hcd_init
 *
 * Return: 0 Success
 */
int __weak board_ehci_hcd_init(int port)
{
	return 0;
}

/**
 * board_ehci_power - enables/disables usb vbus voltage
 * @port:      usb otg port
 * @on:        on/off vbus voltage
 *
 * Enables/disables supply vbus voltage for usb otg port.
 * Machine board file overrides board_ehci_power
 *
 * Return: 0 Success
 */
int __weak board_ehci_power(int port, int on)
{
	return 0;
}

int ehci_mx6_common_init(struct usb_ehci *ehci, int index)
{
	int ret;

	enable_usboh3_clk(1);
	mdelay(1);

	/* Do board specific initialization */
	ret = board_ehci_hcd_init(index);
	if (ret)
		return ret;

	usb_power_config(index);
	usb_oc_config(index);

#if defined(CONFIG_MX6)
	usb_internal_phy_clock_gate(index, 1);
	usb_phy_enable(index, ehci);
#endif

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_USB)
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	enum usb_init_type type;
#if defined(CONFIG_MX6)
	u32 controller_spacing = 0x200;
#elif defined(CONFIG_MX7)
	u32 controller_spacing = 0x10000;
#endif
	struct usb_ehci *ehci = (struct usb_ehci *)(USB_BASE_ADDR +
		(controller_spacing * index));
	int ret;

	if (index > 3)
		return -EINVAL;

	ret = ehci_mx6_common_init(ehci, index);
	if (ret)
		return ret;

	type = board_usb_phy_mode(index);

	if (hccr && hcor) {
		*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
		*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
				HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
	}

	if ((type == init) || (type == USB_INIT_DEVICE))
		board_ehci_power(index, (type == USB_INIT_DEVICE) ? 0 : 1);
	if (type != init)
		return -ENODEV;
	if (type == USB_INIT_DEVICE)
		return 0;

	setbits_le32(&ehci->usbmode, CM_HOST);
	writel(CONFIG_MXC_USB_PORTSC, &ehci->portsc);
	setbits_le32(&ehci->portsc, USB_EN);

	mdelay(10);

	return 0;
}

int ehci_hcd_stop(int index)
{
	return 0;
}
#else
struct ehci_mx6_priv_data {
	struct ehci_ctrl ctrl;
	struct usb_ehci *ehci;
	struct udevice *vbus_supply;
	enum usb_init_type init_type;
	int portnr;
};

static int mx6_init_after_reset(struct ehci_ctrl *dev)
{
	struct ehci_mx6_priv_data *priv = dev->priv;
	enum usb_init_type type = priv->init_type;
	struct usb_ehci *ehci = priv->ehci;
	int ret;

	ret = ehci_mx6_common_init(priv->ehci, priv->portnr);
	if (ret)
		return ret;

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	if (priv->vbus_supply) {
		ret = regulator_set_enable(priv->vbus_supply,
					   (type == USB_INIT_DEVICE) ?
					   false : true);
		if (ret) {
			puts("Error enabling VBUS supply\n");
			return ret;
		}
	}
#endif

	if (type == USB_INIT_DEVICE)
		return 0;

	setbits_le32(&ehci->usbmode, CM_HOST);
	writel(CONFIG_MXC_USB_PORTSC, &ehci->portsc);
	setbits_le32(&ehci->portsc, USB_EN);

	mdelay(10);

	return 0;
}

static const struct ehci_ops mx6_ehci_ops = {
	.init_after_reset = mx6_init_after_reset
};

static int ehci_usb_phy_mode(struct udevice *dev)
{
	struct usb_platdata *plat = dev_get_platdata(dev);
	void *__iomem addr = (void *__iomem)devfdt_get_addr(dev);
	void *__iomem phy_ctrl, *__iomem phy_status;
	const void *blob = gd->fdt_blob;
	int offset = dev_of_offset(dev), phy_off;
	u32 val;

	/*
	 * About fsl,usbphy, Refer to
	 * Documentation/devicetree/bindings/usb/ci-hdrc-usb2.txt.
	 */
	if (is_mx6()) {
		phy_off = fdtdec_lookup_phandle(blob,
						offset,
						"fsl,usbphy");
		if (phy_off < 0)
			return -EINVAL;

		addr = (void __iomem *)fdtdec_get_addr(blob, phy_off,
						       "reg");
		if ((fdt_addr_t)addr == FDT_ADDR_T_NONE)
			return -EINVAL;

		phy_ctrl = (void __iomem *)(addr + USBPHY_CTRL);
		val = readl(phy_ctrl);

		if (val & USBPHY_CTRL_OTG_ID)
			plat->init_type = USB_INIT_DEVICE;
		else
			plat->init_type = USB_INIT_HOST;
	} else if (is_mx7()) {
		phy_status = (void __iomem *)(addr +
					      USBNC_PHY_STATUS_OFFSET);
		val = readl(phy_status);

		if (val & USBNC_PHYSTATUS_ID_DIG)
			plat->init_type = USB_INIT_DEVICE;
		else
			plat->init_type = USB_INIT_HOST;
	} else {
		return -EINVAL;
	}

	return 0;
}

static int ehci_usb_ofdata_to_platdata(struct udevice *dev)
{
	struct usb_platdata *plat = dev_get_platdata(dev);
	enum usb_dr_mode dr_mode;

	dr_mode = usb_get_dr_mode(dev_of_offset(dev));

	switch (dr_mode) {
	case USB_DR_MODE_HOST:
		plat->init_type = USB_INIT_HOST;
		break;
	case USB_DR_MODE_PERIPHERAL:
		plat->init_type = USB_INIT_DEVICE;
		break;
	case USB_DR_MODE_OTG:
	case USB_DR_MODE_UNKNOWN:
		return ehci_usb_phy_mode(dev);
	};

	return 0;
}

static int ehci_usb_bind(struct udevice *dev)
{
	/*
	 * TODO:
	 * This driver is only partly converted to DT probing and still uses
	 * a tremendous amount of hard-coded addresses. To make things worse,
	 * the driver depends on specific sequential indexing of controllers,
	 * from which it derives offsets in the PHY and ANATOP register sets.
	 *
	 * Here we attempt to calculate these indexes from DT information as
	 * well as we can. The USB controllers on all existing iMX6/iMX7 SoCs
	 * are placed next to each other, at addresses incremented by 0x200.
	 * Thus, the index is derived from the multiple of 0x200 offset from
	 * the first controller address.
	 *
	 * However, to complete conversion of this driver to DT probing, the
	 * following has to be done:
	 * - DM clock framework support for iMX must be implemented
	 * - usb_power_config() has to be converted to clock framework
	 *   -> Thus, the ad-hoc "index" variable goes away.
	 * - USB PHY handling has to be factored out into separate driver
	 *   -> Thus, the ad-hoc "index" variable goes away from the PHY
	 *      code, the PHY driver must parse it's address from DT. This
	 *      USB driver must find the PHY driver via DT phandle.
	 *   -> usb_power_config() shall be moved to PHY driver
	 * With these changes in place, the ad-hoc indexing goes away and
	 * the driver is fully converted to DT probing.
	 */
	fdt_size_t size;
	fdt_addr_t addr = devfdt_get_addr_size_index(dev, 0, &size);

	dev->req_seq = (addr - USB_BASE_ADDR) / size;

	return 0;
}

static int ehci_usb_probe(struct udevice *dev)
{
	struct usb_platdata *plat = dev_get_platdata(dev);
	struct usb_ehci *ehci = (struct usb_ehci *)devfdt_get_addr(dev);
	struct ehci_mx6_priv_data *priv = dev_get_priv(dev);
	enum usb_init_type type = plat->init_type;
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int ret;

	priv->ehci = ehci;
	priv->portnr = dev->seq;
	priv->init_type = type;

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	ret = device_get_supply_regulator(dev, "vbus-supply",
					  &priv->vbus_supply);
	if (ret)
		debug("%s: No vbus supply\n", dev->name);
#endif
	ret = ehci_mx6_common_init(ehci, priv->portnr);
	if (ret)
		return ret;

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	if (priv->vbus_supply) {
		ret = regulator_set_enable(priv->vbus_supply,
					   (type == USB_INIT_DEVICE) ?
					   false : true);
		if (ret) {
			puts("Error enabling VBUS supply\n");
			return ret;
		}
	}
#endif

	if (priv->init_type == USB_INIT_HOST) {
		setbits_le32(&ehci->usbmode, CM_HOST);
		writel(CONFIG_MXC_USB_PORTSC, &ehci->portsc);
		setbits_le32(&ehci->portsc, USB_EN);
	}

	mdelay(10);

	hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	hcor = (struct ehci_hcor *)((uint32_t)hccr +
			HC_LENGTH(ehci_readl(&(hccr)->cr_capbase)));

	return ehci_register(dev, hccr, hcor, &mx6_ehci_ops, 0, priv->init_type);
}

static const struct udevice_id mx6_usb_ids[] = {
	{ .compatible = "fsl,imx27-usb" },
	{ }
};

U_BOOT_DRIVER(usb_mx6) = {
	.name	= "ehci_mx6",
	.id	= UCLASS_USB,
	.of_match = mx6_usb_ids,
	.ofdata_to_platdata = ehci_usb_ofdata_to_platdata,
	.bind	= ehci_usb_bind,
	.probe	= ehci_usb_probe,
	.remove = ehci_deregister,
	.ops	= &ehci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct ehci_mx6_priv_data),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
#endif
