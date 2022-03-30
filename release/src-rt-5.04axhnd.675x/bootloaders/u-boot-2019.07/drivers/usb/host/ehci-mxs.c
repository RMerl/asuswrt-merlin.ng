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

static const struct ehci_mxs_port mxs_port[] = {
#ifdef CONFIG_EHCI_MXS_PORT0
	{
		MXS_USBCTRL0_BASE,
		(struct mxs_usbphy_regs *)MXS_USBPHY0_BASE,
		(struct mxs_register_32 *)(MXS_CLKCTRL_BASE +
			offsetof(struct mxs_clkctrl_regs,
			hw_clkctrl_pll0ctrl0_reg)),
		CLKCTRL_PLL0CTRL0_EN_USB_CLKS | CLKCTRL_PLL0CTRL0_POWER,
		CLKCTRL_PLL0CTRL0_EN_USB_CLKS,
		HW_DIGCTL_CTRL_USB0_CLKGATE,
	},
#endif
#ifdef CONFIG_EHCI_MXS_PORT1
	{
		MXS_USBCTRL1_BASE,
		(struct mxs_usbphy_regs *)MXS_USBPHY1_BASE,
		(struct mxs_register_32 *)(MXS_CLKCTRL_BASE +
			offsetof(struct mxs_clkctrl_regs,
			hw_clkctrl_pll1ctrl0_reg)),
		CLKCTRL_PLL1CTRL0_EN_USB_CLKS | CLKCTRL_PLL1CTRL0_POWER,
		CLKCTRL_PLL1CTRL0_EN_USB_CLKS,
		HW_DIGCTL_CTRL_USB1_CLKGATE,
	},
#endif
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

int __weak board_ehci_hcd_init(int port)
{
	return 0;
}

int __weak board_ehci_hcd_exit(int port)
{
	return 0;
}

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{

	int ret;
	uint32_t usb_base, cap_base;
	const struct ehci_mxs_port *port;

	if ((index < 0) || (index >= ARRAY_SIZE(mxs_port))) {
		printf("Invalid port index (index = %d)!\n", index);
		return -EINVAL;
	}

	ret = board_ehci_hcd_init(index);
	if (ret)
		return ret;

	port = &mxs_port[index];

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

int ehci_hcd_stop(int index)
{
	int ret;
	uint32_t usb_base, cap_base, tmp;
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	const struct ehci_mxs_port *port;

	if ((index < 0) || (index >= ARRAY_SIZE(mxs_port))) {
		printf("Invalid port index (index = %d)!\n", index);
		return -EINVAL;
	}

	port = &mxs_port[index];

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
	ret = ehci_mxs_toggle_clock(port, 0);

	board_ehci_hcd_exit(index);

	return ret;
}
