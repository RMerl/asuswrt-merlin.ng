// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/arch/mips/bcm63xx/usb-common.c:
 *	Copyright 2008 Maxime Bizon <mbizon@freebox.fr>
 *	Copyright 2013 Florian Fainelli <florian@openwrt.org>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <power-domain.h>
#include <reset.h>
#include <asm/io.h>
#include <dm/device.h>

/* USBH PLL Control register */
#define USBH_PLL_REG		0x18
#define USBH_PLL_IDDQ_PWRDN	BIT(9)
#define USBH_PLL_PWRDN_DELAY	BIT(10)

/* USBH Swap Control register */
#define USBH_SWAP_REG		0x1c
#define USBH_SWAP_OHCI_DATA	BIT(0)
#define USBH_SWAP_OHCI_ENDIAN	BIT(1)
#define USBH_SWAP_EHCI_DATA	BIT(3)
#define USBH_SWAP_EHCI_ENDIAN	BIT(4)

/* USBH Setup register */
#define USBH_SETUP_REG		0x28
#define USBH_SETUP_IOC		BIT(4)
#define USBH_SETUP_IPP		BIT(5)

struct bcm6368_usbh_hw {
	uint32_t setup_clr;
	uint32_t pll_clr;
};

struct bcm6368_usbh_priv {
	const struct bcm6368_usbh_hw *hw;
	void __iomem *regs;
};

static int bcm6368_usbh_init(struct phy *phy)
{
	struct bcm6368_usbh_priv *priv = dev_get_priv(phy->dev);
	const struct bcm6368_usbh_hw *hw = priv->hw;

	/* configure to work in native cpu endian */
	clrsetbits_be32(priv->regs + USBH_SWAP_REG,
			USBH_SWAP_EHCI_ENDIAN | USBH_SWAP_OHCI_ENDIAN,
			USBH_SWAP_EHCI_DATA | USBH_SWAP_OHCI_DATA);

	/* setup config */
	if (hw->setup_clr)
		clrbits_be32(priv->regs + USBH_SETUP_REG, hw->setup_clr);

	setbits_be32(priv->regs + USBH_SETUP_REG, USBH_SETUP_IOC);

	/* enable pll control */
	if (hw->pll_clr)
		clrbits_be32(priv->regs + USBH_PLL_REG, hw->pll_clr);

	return 0;
}

static struct phy_ops bcm6368_usbh_ops = {
	.init = bcm6368_usbh_init,
};

static const struct bcm6368_usbh_hw bcm6328_hw = {
	.pll_clr = USBH_PLL_IDDQ_PWRDN | USBH_PLL_PWRDN_DELAY,
	.setup_clr = 0,
};

static const struct bcm6368_usbh_hw bcm6362_hw = {
	.pll_clr = 0,
	.setup_clr = 0,
};

static const struct bcm6368_usbh_hw bcm6368_hw = {
	.pll_clr = 0,
	.setup_clr = 0,
};

static const struct bcm6368_usbh_hw bcm63268_hw = {
	.pll_clr = USBH_PLL_IDDQ_PWRDN | USBH_PLL_PWRDN_DELAY,
	.setup_clr = USBH_SETUP_IPP,
};

static const struct udevice_id bcm6368_usbh_ids[] = {
	{
		.compatible = "brcm,bcm6328-usbh",
		.data = (ulong)&bcm6328_hw,
	}, {
		.compatible = "brcm,bcm6362-usbh",
		.data = (ulong)&bcm6362_hw,
	}, {
		.compatible = "brcm,bcm6368-usbh",
		.data = (ulong)&bcm6368_hw,
	}, {
		.compatible = "brcm,bcm63268-usbh",
		.data = (ulong)&bcm63268_hw,
	}, { /* sentinel */ }
};

static int bcm6368_usbh_probe(struct udevice *dev)
{
	struct bcm6368_usbh_priv *priv = dev_get_priv(dev);
	const struct bcm6368_usbh_hw *hw =
		(const struct bcm6368_usbh_hw *)dev_get_driver_data(dev);
#if defined(CONFIG_POWER_DOMAIN)
	struct power_domain pwr_dom;
#endif
	struct reset_ctl rst_ctl;
	struct clk clk;
	int ret;

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	priv->hw = hw;

	/* enable usbh clock */
	ret = clk_get_by_name(dev, "usbh", &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret < 0)
		return ret;

	ret = clk_free(&clk);
	if (ret < 0)
		return ret;

#if defined(CONFIG_POWER_DOMAIN)
	/* enable power domain */
	ret = power_domain_get(dev, &pwr_dom);
	if (ret < 0)
		return ret;

	ret = power_domain_on(&pwr_dom);
	if (ret < 0)
		return ret;

	ret = power_domain_free(&pwr_dom);
	if (ret < 0)
		return ret;
#endif

	/* perform reset */
	ret = reset_get_by_index(dev, 0, &rst_ctl);
	if (ret < 0)
		return ret;

	ret = reset_deassert(&rst_ctl);
	if (ret < 0)
		return ret;

	ret = reset_free(&rst_ctl);
	if (ret < 0)
		return ret;

	/* enable usb_ref clock */
	ret = clk_get_by_name(dev, "usb_ref", &clk);
	if (!ret) {
		ret = clk_enable(&clk);
		if (ret < 0)
			return ret;

		ret = clk_free(&clk);
		if (ret < 0)
			return ret;
	}

	mdelay(100);

	return 0;
}

U_BOOT_DRIVER(bcm6368_usbh) = {
	.name = "bcm6368-usbh",
	.id = UCLASS_PHY,
	.of_match = bcm6368_usbh_ids,
	.ops = &bcm6368_usbh_ops,
	.priv_auto_alloc_size = sizeof(struct bcm6368_usbh_priv),
	.probe = bcm6368_usbh_probe,
};
