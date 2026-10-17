// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 USB Host driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <errno.h>
#include <linux/delay.h>
#include <dm.h>
#include <power/regulator.h>

#include "ehci.h"

/* This DIGCTL register ungates clock to USB */
#define	HW_DIGCTL_CTRL			0x8001c000
#define	HW_DIGCTL_CTRL_USB0_CLKGATE	(1 << 2)
#define	HW_DIGCTL_CTRL_USB1_CLKGATE	(1 << 16)

struct ehci_mxs_port {
	uint32_t		usb_regs;
	struct mxs_usbphy_regs	*phy_regs;

	struct mxs_register_32	*pll;
	uint32_t		pll_en_bits;
	uint32_t		pll_dis_bits;
	uint32_t		gate_bits;
};

static int ehci_mxs_toggle_clock(const struct ehci_mxs_port *port, int enable)
{
	struct mxs_register_32 *digctl_ctrl =
		(struct mxs_register_32 *)HW_DIGCTL_CTRL;
	int pll_offset, dig_offset;

	if (enable) {
		pll_offset = offsetof(struct mxs_register_32, reg_set);
		dig_offset = offsetof(struct mxs_register_32, reg_clr);
		writel(port->gate_bits, (u32)&digctl_ctrl->reg + dig_offset);
		writel(port->pll_en_bits, (u32)port->pll + pll_offset);
	} else {
		pll_offset = offsetof(struct mxs_register_32, reg_clr);
		dig_offset = offsetof(struct mxs_register_32, reg_set);
		writel(port->pll_dis_bits, (u32)port->pll + pll_offset);
		writel(port->gate_bits, (u32)&digctl_ctrl->reg + dig_offset);
	}

	return 0;
}

static int __ehci_hcd_init(struct ehci_mxs_port *port, enum usb_init_type init,
			   struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	u32 usb_base, cap_base;
	int ret;

	/* Reset the PHY block */
	writel(USBPHY_CTRL_SFTRST, &port->phy_regs->hw_usbphy_ctrl_set);
	udelay(10);
	writel(USBPHY_CTRL_SFTRST | USBPHY_CTRL_CLKGATE,
	       &port->phy_regs->hw_usbphy_ctrl_clr);

	/* Enable USB clock */
	ret = ehci_mxs_toggle_clock(port, 1);
	if (ret)
		return ret;

	/* Start USB PHY */
	writel(0, &port->phy_regs->hw_usbphy_pwd);

	/* Enable UTMI+ Level 2 and Level 3 compatibility */
	writel(USBPHY_CTRL_ENUTMILEVEL3 | USBPHY_CTRL_ENUTMILEVEL2 | 1,
	       &port->phy_regs->hw_usbphy_ctrl_set);

	usb_base = port->usb_regs + 0x100;
	*hccr = (struct ehci_hccr *)usb_base;

	cap_base = ehci_readl(&(*hccr)->cr_capbase);
	*hcor = (struct ehci_hcor *)(usb_base + HC_LENGTH(cap_base));

	return 0;
}

static int __ehci_hcd_stop(struct ehci_mxs_port *port)
{
	u32 usb_base, cap_base, tmp;
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;

	/* Stop the USB port */
	usb_base = port->usb_regs + 0x100;
	hccr = (struct ehci_hccr *)usb_base;
	cap_base = ehci_readl(&hccr->cr_capbase);
	hcor = (struct ehci_hcor *)(usb_base + HC_LENGTH(cap_base));

	tmp = ehci_readl(&hcor->or_usbcmd);
	tmp &= ~CMD_RUN;
	ehci_writel(&hcor->or_usbcmd, tmp);

	/* Disable the PHY */
	tmp = USBPHY_PWD_RXPWDRX | USBPHY_PWD_RXPWDDIFF |
		USBPHY_PWD_RXPWD1PT1 | USBPHY_PWD_RXPWDENV |
		USBPHY_PWD_TXPWDV2I | USBPHY_PWD_TXPWDIBIAS |
		USBPHY_PWD_TXPWDFS;
	writel(tmp, &port->phy_regs->hw_usbphy_pwd);

	/* Disable USB clock */
	return ehci_mxs_toggle_clock(port, 0);
}

struct ehci_mxs_priv_data {
	struct ehci_ctrl ctrl;
	struct usb_ehci *ehci;
	struct udevice *vbus_supply;
	struct ehci_mxs_port port;
	enum usb_init_type init_type;
};

/*
 * Below defines correspond to imx28 clk Linux (v5.15.y)
 * clock driver to provide proper offset for PHY[01]
 * devices.
 */
#define CLK_USB_PHY0 62
#define CLK_USB_PHY1 63
#define PLL0CTRL0(base) ((base) + 0x0000)
#define PLL1CTRL0(base) ((base) + 0x0020)

static int ehci_usb_ofdata_to_platdata(struct udevice *dev)
{
	struct ehci_mxs_priv_data *priv = dev_get_priv(dev);
	struct usb_plat *plat = dev_get_plat(dev);
	struct ehci_mxs_port *port = &priv->port;
	u32 phandle, phy_reg, clk_reg, clk_id;
	ofnode np = dev_ofnode(dev);
	ofnode phy_node, clk_node;
	const char *mode;
	int ret;

	mode = ofnode_read_string(np, "dr_mode");
	if (mode) {
		if (strcmp(mode, "peripheral") == 0)
			plat->init_type = USB_INIT_DEVICE;
		else if (strcmp(mode, "host") == 0)
			plat->init_type = USB_INIT_HOST;
		else
			return -EINVAL;
	}

	/* Read base address of the USB IP block */
	ret = ofnode_read_u32(np, "reg", &port->usb_regs);
	if (ret)
		return ret;

	/* Read base address of the USB PHY IP block */
	ret = ofnode_read_u32(np, "fsl,usbphy", &phandle);
	if (ret)
		return ret;

	phy_node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(phy_node))
		return -ENODEV;

	ret = ofnode_read_u32(phy_node, "reg", &phy_reg);
	if (ret)
		return ret;

	port->phy_regs = (struct mxs_usbphy_regs *)phy_reg;

	/* Read base address of the CLK IP block and proper ID */
	ret = ofnode_read_u32_index(phy_node, "clocks", 0, &phandle);
	if (ret)
		return ret;

	ret = ofnode_read_u32_index(phy_node, "clocks", 1, &clk_id);
	if (ret)
		return ret;

	clk_node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(clk_node))
		return -ENODEV;

	ret = ofnode_read_u32(clk_node, "reg", &clk_reg);
	if (ret)
		return ret;

	port->pll = (struct mxs_register_32 *)clk_reg;

	/* Provide proper offset for USB PHY clocks */
	if (clk_id == CLK_USB_PHY0)
		port->pll = PLL0CTRL0(port->pll);

	if (clk_id == CLK_USB_PHY1)
		port->pll = PLL1CTRL0(port->pll);

	debug("%s: pll_reg: 0x%p clk_id: %d\n", __func__, port->pll, clk_id);
	/*
	 * On the imx28 the values provided by CLKCTRL_PLL0* defines to are the
	 * same as ones for CLKCTRL_PLL1*. As a result the former can be used
	 * for both ports - i.e. (usb[01]).
	 */
	port->pll_en_bits = CLKCTRL_PLL0CTRL0_EN_USB_CLKS |
		CLKCTRL_PLL0CTRL0_POWER;
	port->pll_dis_bits = CLKCTRL_PLL0CTRL0_EN_USB_CLKS;
	port->gate_bits = HW_DIGCTL_CTRL_USB0_CLKGATE;

	return 0;
}

static int ehci_usb_probe(struct udevice *dev)
{
	struct usb_plat *plat = dev_get_plat(dev);
	struct usb_ehci *ehci = dev_read_addr_ptr(dev);
	struct ehci_mxs_priv_data *priv = dev_get_priv(dev);
	struct ehci_mxs_port *port = &priv->port;
	enum usb_init_type type = plat->init_type;
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int ret;

	priv->ehci = ehci;
	priv->init_type = type;

	debug("%s: USB type: %s  reg: 0x%x phy_reg 0x%p\n", __func__,
	      type == USB_INIT_HOST ? "HOST" : "DEVICE", port->usb_regs,
	      (uint32_t *)port->phy_regs);

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	ret = device_get_supply_regulator(dev, "vbus-supply",
					  &priv->vbus_supply);
	if (ret)
		debug("%s: No vbus supply\n", dev->name);

	if (!ret && priv->vbus_supply) {
		ret = regulator_set_enable_if_allowed(priv->vbus_supply,
						      (type == USB_INIT_DEVICE) ?
						      false : true);
		if (ret) {
			puts("Error enabling VBUS supply\n");
			return ret;
		}
	}
#endif
	ret = __ehci_hcd_init(port, type, &hccr, &hcor);
	if (ret)
		return ret;

	mdelay(10);
	return ehci_register(dev, hccr, hcor, NULL, 0, priv->init_type);
}

static int ehci_usb_remove(struct udevice *dev)
{
	struct ehci_mxs_priv_data *priv = dev_get_priv(dev);
	struct ehci_mxs_port *port = &priv->port;
	int ret;

	ret =  ehci_deregister(dev);
	if (ret)
		return ret;

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	if (priv->vbus_supply) {
		ret = regulator_set_enable_if_allowed(priv->vbus_supply, false);
		if (ret) {
			puts("Error disabling VBUS supply\n");
			return ret;
		}
	}
#endif
	return __ehci_hcd_stop(port);
}

static const struct udevice_id mxs_usb_ids[] = {
	{ .compatible = "fsl,imx28-usb" },
	{ }
};

U_BOOT_DRIVER(usb_mxs) = {
	.name	= "ehci_mxs",
	.id	= UCLASS_USB,
	.of_match = mxs_usb_ids,
	.of_to_plat = ehci_usb_ofdata_to_platdata,
	.probe	= ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.plat_auto = sizeof(struct usb_plat),
	.priv_auto = sizeof(struct ehci_mxs_priv_data),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
