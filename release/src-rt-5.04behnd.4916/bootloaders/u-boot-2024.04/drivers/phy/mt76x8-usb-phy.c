// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Stefan Roese <sr@denx.de>
 *
 * Derived from linux/drivers/phy/ralink/phy-ralink-usb.c
 *     Copyright (C) 2017 John Crispin <john@phrozen.org>
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <log.h>
#include <reset.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#define OFS_U2_PHY_AC0			0x800
#define USBPLL_FBDIV_S			16
#define USBPLL_FBDIV_M			GENMASK(22, 16)
#define BG_TRIM_S			8
#define BG_TRIM_M			GENMASK(11, 8)
#define BG_RBSEL_S			6
#define BG_RBSEL_M			GENMASK(7, 6)
#define BG_RASEL_S			4
#define BG_RASEL_M			GENMASK(5, 4)
#define BGR_DIV_S			2
#define BGR_DIV_M			GENMASK(3, 2)
#define CHP_EN				BIT(1)

#define OFS_U2_PHY_AC1			0x804
#define VRT_VREF_SEL_S			28
#define VRT_VREF_SEL_M			GENMASK(30, 28)
#define TERM_VREF_SEL_S			24
#define TERM_VREF_SEL_M			GENMASK(26, 24)
#define USBPLL_RSVD			BIT(4)
#define USBPLL_ACCEN			BIT(3)
#define USBPLL_LF			BIT(2)

#define OFS_U2_PHY_AC2			0x808

#define OFS_U2_PHY_ACR0			0x810
#define HSTX_SRCAL_EN			BIT(23)
#define HSTX_SRCTRL_S			16
#define HSTX_SRCTRL_M			GENMASK(18, 16)

#define OFS_U2_PHY_ACR3			0x81C
#define HSTX_DBIST_S			28
#define HSTX_DBIST_M			GENMASK(31, 28)
#define HSRX_BIAS_EN_SEL_S		20
#define HSRX_BIAS_EN_SEL_M		GENMASK(21, 20)

#define OFS_U2_PHY_DCR0			0x860
#define PHYD_RESERVE_S			8
#define PHYD_RESERVE_M			GENMASK(23, 8)
#define CDR_FILT_S			0
#define CDR_FILT_M			GENMASK(3, 0)

#define OFS_U2_PHY_DTM0			0x868
#define FORCE_USB_CLKEN			BIT(25)

#define OFS_FM_CR0			0xf00
#define FREQDET_EN			BIT(24)
#define CYCLECNT_S			0
#define CYCLECNT_M			GENMASK(23, 0)

#define OFS_FM_MONR0			0xf0c

#define OFS_FM_MONR1			0xf10
#define FRCK_EN				BIT(8)

#define U2_SR_COEF_7628			32

struct mt76x8_usb_phy {
	void __iomem		*base;
	struct clk		cg;	/* for clock gating */
	struct reset_ctl	rst_phy;
};

static void phy_w32(struct mt76x8_usb_phy *phy, u32 reg, u32 val)
{
	writel(val, phy->base + reg);
}

static u32 phy_r32(struct mt76x8_usb_phy *phy, u32 reg)
{
	return readl(phy->base + reg);
}

static void phy_rmw32(struct mt76x8_usb_phy *phy, u32 reg, u32 clr, u32 set)
{
	clrsetbits_32(phy->base + reg, clr, set);
}

static void mt76x8_usb_phy_init(struct mt76x8_usb_phy *phy)
{
	phy_r32(phy, OFS_U2_PHY_AC2);
	phy_r32(phy, OFS_U2_PHY_ACR0);
	phy_r32(phy, OFS_U2_PHY_DCR0);

	phy_w32(phy, OFS_U2_PHY_DCR0,
		(0xffff << PHYD_RESERVE_S) | (2 << CDR_FILT_S));
	phy_r32(phy, OFS_U2_PHY_DCR0);

	phy_w32(phy, OFS_U2_PHY_DCR0,
		(0x5555 << PHYD_RESERVE_S) | (2 << CDR_FILT_S));
	phy_r32(phy, OFS_U2_PHY_DCR0);

	phy_w32(phy, OFS_U2_PHY_DCR0,
		(0xaaaa << PHYD_RESERVE_S) | (2 << CDR_FILT_S));
	phy_r32(phy, OFS_U2_PHY_DCR0);

	phy_w32(phy, OFS_U2_PHY_DCR0,
		(4 << PHYD_RESERVE_S) | (2 << CDR_FILT_S));
	phy_r32(phy, OFS_U2_PHY_DCR0);

	phy_w32(phy, OFS_U2_PHY_AC0,
		(0x48 << USBPLL_FBDIV_S) | (8 << BG_TRIM_S) |
		(1 << BG_RBSEL_S) | (2 << BG_RASEL_S) | (2 << BGR_DIV_S) |
		CHP_EN);

	phy_w32(phy, OFS_U2_PHY_AC1,
		(4 << VRT_VREF_SEL_S) | (4 << TERM_VREF_SEL_S) | USBPLL_RSVD |
		USBPLL_ACCEN | USBPLL_LF);

	phy_w32(phy, OFS_U2_PHY_ACR3,
		(12 << HSTX_DBIST_S) | (2 << HSRX_BIAS_EN_SEL_S));

	phy_w32(phy, OFS_U2_PHY_DTM0, FORCE_USB_CLKEN);
}

static void mt76x8_usb_phy_sr_calibrate(struct mt76x8_usb_phy *phy)
{
	u32 fmout, tmp = 4;
	int i;

	/* Enable HS TX SR calibration */
	phy_rmw32(phy, OFS_U2_PHY_ACR0, 0, HSTX_SRCAL_EN);
	mdelay(1);

	/* Enable free run clock */
	phy_rmw32(phy, OFS_FM_MONR1, 0, FRCK_EN);

	/* Set cycle count = 0x400 */
	phy_rmw32(phy, OFS_FM_CR0, CYCLECNT_M, 0x400 << CYCLECNT_S);

	/* Enable frequency meter */
	phy_rmw32(phy, OFS_FM_CR0, 0, FREQDET_EN);

	/* Wait for FM detection done, set timeout to 10ms */
	for (i = 0; i < 10; i++) {
		fmout = phy_r32(phy, OFS_FM_MONR0);

		if (fmout)
			break;

		mdelay(1);
	}

	/* Disable frequency meter */
	phy_rmw32(phy, OFS_FM_CR0, FREQDET_EN, 0);

	/* Disable free run clock */
	phy_rmw32(phy, OFS_FM_MONR1, FRCK_EN, 0);

	/* Disable HS TX SR calibration */
	phy_rmw32(phy, OFS_U2_PHY_ACR0, HSTX_SRCAL_EN, 0);
	mdelay(1);

	if (fmout) {
		/*
		 * set reg = (1024 / FM_OUT) * 25 * 0.028
		 * (round to the nearest digits)
		 */
		tmp = (((1024 * 25 * U2_SR_COEF_7628) / fmout) + 500) / 1000;
	}

	phy_rmw32(phy, OFS_U2_PHY_ACR0, HSTX_SRCTRL_M,
		  (tmp << HSTX_SRCTRL_S) & HSTX_SRCTRL_M);
}

static int mt76x8_usb_phy_power_on(struct phy *_phy)
{
	struct mt76x8_usb_phy *phy = dev_get_priv(_phy->dev);

	clk_enable(&phy->cg);

	reset_deassert(&phy->rst_phy);

	/*
	 * The SDK kernel had a delay of 100ms. however on device
	 * testing showed that 10ms is enough
	 */
	mdelay(10);

	mt76x8_usb_phy_init(phy);
	mt76x8_usb_phy_sr_calibrate(phy);

	return 0;
}

static int mt76x8_usb_phy_power_off(struct phy *_phy)
{
	struct mt76x8_usb_phy *phy = dev_get_priv(_phy->dev);

	clk_disable(&phy->cg);

	reset_assert(&phy->rst_phy);

	return 0;
}

static int mt76x8_usb_phy_probe(struct udevice *dev)
{
	struct mt76x8_usb_phy *phy = dev_get_priv(dev);
	int ret;

	phy->base = dev_read_addr_ptr(dev);
	if (!phy->base)
		return -EINVAL;

	/* clock gate */
	ret = clk_get_by_name(dev, "cg", &phy->cg);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "phy", &phy->rst_phy);
	if (ret)
		return ret;

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
	.priv_auto	= sizeof(struct mt76x8_usb_phy),
};
