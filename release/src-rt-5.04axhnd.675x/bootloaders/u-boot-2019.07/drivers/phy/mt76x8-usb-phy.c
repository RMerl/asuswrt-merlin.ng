// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Stefan Roese <sr@denx.de>
 *
 * Derived from linux/drivers/phy/ralink/phy-ralink-usb.c
 *     Copyright (C) 2017 John Crispin <john@phrozen.org>
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <regmap.h>
#include <reset-uclass.h>
#include <syscon.h>
#include <asm/io.h>

#define RT_SYSC_REG_SYSCFG1		0x014
#define RT_SYSC_REG_CLKCFG1		0x030
#define RT_SYSC_REG_USB_PHY_CFG		0x05c

#define OFS_U2_PHY_AC0			0x800
#define OFS_U2_PHY_AC1			0x804
#define OFS_U2_PHY_AC2			0x808
#define OFS_U2_PHY_ACR0			0x810
#define OFS_U2_PHY_ACR1			0x814
#define OFS_U2_PHY_ACR2			0x818
#define OFS_U2_PHY_ACR3			0x81C
#define OFS_U2_PHY_ACR4			0x820
#define OFS_U2_PHY_AMON0		0x824
#define OFS_U2_PHY_DCR0			0x860
#define OFS_U2_PHY_DCR1			0x864
#define OFS_U2_PHY_DTM0			0x868
#define OFS_U2_PHY_DTM1			0x86C

#define RT_RSTCTRL_UDEV			BIT(25)
#define RT_RSTCTRL_UHST			BIT(22)
#define RT_SYSCFG1_USB0_HOST_MODE	BIT(10)

#define MT7620_CLKCFG1_UPHY0_CLK_EN	BIT(25)
#define MT7620_CLKCFG1_UPHY1_CLK_EN	BIT(22)
#define RT_CLKCFG1_UPHY1_CLK_EN		BIT(20)
#define RT_CLKCFG1_UPHY0_CLK_EN		BIT(18)

#define USB_PHY_UTMI_8B60M		BIT(1)
#define UDEV_WAKEUP			BIT(0)

struct mt76x8_usb_phy {
	u32			clk;
	void __iomem		*base;
	struct regmap		*sysctl;
};

static void u2_phy_w32(struct mt76x8_usb_phy *phy, u32 val, u32 reg)
{
	writel(val, phy->base + reg);
}

static u32 u2_phy_r32(struct mt76x8_usb_phy *phy, u32 reg)
{
	return readl(phy->base + reg);
}

static void mt76x8_usb_phy_init(struct mt76x8_usb_phy *phy)
{
	u2_phy_r32(phy, OFS_U2_PHY_AC2);
	u2_phy_r32(phy, OFS_U2_PHY_ACR0);
	u2_phy_r32(phy, OFS_U2_PHY_DCR0);

	u2_phy_w32(phy, 0x00ffff02, OFS_U2_PHY_DCR0);
	u2_phy_r32(phy, OFS_U2_PHY_DCR0);
	u2_phy_w32(phy, 0x00555502, OFS_U2_PHY_DCR0);
	u2_phy_r32(phy, OFS_U2_PHY_DCR0);
	u2_phy_w32(phy, 0x00aaaa02, OFS_U2_PHY_DCR0);
	u2_phy_r32(phy, OFS_U2_PHY_DCR0);
	u2_phy_w32(phy, 0x00000402, OFS_U2_PHY_DCR0);
	u2_phy_r32(phy, OFS_U2_PHY_DCR0);
	u2_phy_w32(phy, 0x0048086a, OFS_U2_PHY_AC0);
	u2_phy_w32(phy, 0x4400001c, OFS_U2_PHY_AC1);
	u2_phy_w32(phy, 0xc0200000, OFS_U2_PHY_ACR3);
	u2_phy_w32(phy, 0x02000000, OFS_U2_PHY_DTM0);
}

static int mt76x8_usb_phy_power_on(struct phy *_phy)
{
	struct mt76x8_usb_phy *phy = dev_get_priv(_phy->dev);
	u32 t;

	/* enable the phy */
	regmap_update_bits(phy->sysctl, RT_SYSC_REG_CLKCFG1,
			   phy->clk, phy->clk);

	/* setup host mode */
	regmap_update_bits(phy->sysctl, RT_SYSC_REG_SYSCFG1,
			   RT_SYSCFG1_USB0_HOST_MODE,
			   RT_SYSCFG1_USB0_HOST_MODE);

	/*
	 * The SDK kernel had a delay of 100ms. however on device
	 * testing showed that 10ms is enough
	 */
	mdelay(10);

	if (phy->base)
		mt76x8_usb_phy_init(phy);

	/* print some status info */
	regmap_read(phy->sysctl, RT_SYSC_REG_USB_PHY_CFG, &t);
	printf("remote usb device wakeup %s\n",
	       (t & UDEV_WAKEUP) ? "enabled" : "disabled");
	if (t & USB_PHY_UTMI_8B60M)
		printf("UTMI 8bit 60MHz\n");
	else
		printf("UTMI 16bit 30MHz\n");

	return 0;
}

static int mt76x8_usb_phy_power_off(struct phy *_phy)
{
	struct mt76x8_usb_phy *phy = dev_get_priv(_phy->dev);

	/* disable the phy */
	regmap_update_bits(phy->sysctl, RT_SYSC_REG_CLKCFG1,
			   phy->clk, 0);

	return 0;
}

static int mt76x8_usb_phy_probe(struct udevice *dev)
{
	struct mt76x8_usb_phy *phy = dev_get_priv(dev);

	phy->sysctl = syscon_regmap_lookup_by_phandle(dev, "ralink,sysctl");
	if (IS_ERR(phy->sysctl))
		return PTR_ERR(phy->sysctl);

	phy->base = dev_read_addr_ptr(dev);
	if (!phy->base)
		return -EINVAL;

	return 0;
}

static struct phy_ops mt76x8_usb_phy_ops = {
	.power_on = mt76x8_usb_phy_power_on,
	.power_off = mt76x8_usb_phy_power_off,
};

static const struct udevice_id mt76x8_usb_phy_ids[] = {
	{ .compatible = "mediatek,mt7628-usbphy" },
	{ }
};

U_BOOT_DRIVER(mt76x8_usb_phy) = {
	.name		= "mt76x8_usb_phy",
	.id		= UCLASS_PHY,
	.of_match	= mt76x8_usb_phy_ids,
	.ops		= &mt76x8_usb_phy_ops,
	.probe		= mt76x8_usb_phy_probe,
	.priv_auto_alloc_size = sizeof(struct mt76x8_usb_phy),
};
