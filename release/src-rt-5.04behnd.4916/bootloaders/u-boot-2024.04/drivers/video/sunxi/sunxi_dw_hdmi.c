// SPDX-License-Identifier: GPL-2.0+
/*
 * Allwinner DW HDMI bridge
 *
 * (C) Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 */

#include <clk.h>
#include <common.h>
#include <display.h>
#include <dm.h>
#include <dw_hdmi.h>
#include <edid.h>
#include <log.h>
#include <reset.h>
#include <time.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/lcdc.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <power/regulator.h>

struct sunxi_dw_hdmi_priv {
	struct dw_hdmi hdmi;
	struct reset_ctl_bulk resets;
	struct clk_bulk clocks;
	struct udevice *hvcc;
};

struct sunxi_hdmi_phy {
	u32 pol;
	u32 res1[3];
	u32 read_en;
	u32 unscramble;
	u32 res2[2];
	u32 ctrl;
	u32 unk1;
	u32 unk2;
	u32 pll;
	u32 clk;
	u32 unk3;
	u32 status;
};

#define HDMI_PHY_OFFS 0x10000

static int sunxi_dw_hdmi_get_divider(uint clock)
{
	/*
	 * Due to missing documentaion of HDMI PHY, we know correct
	 * settings only for following four PHY dividers. Select one
	 * based on clock speed.
	 */
	if (clock <= 27000000)
		return 11;
	else if (clock <= 74250000)
		return 4;
	else if (clock <= 148500000)
		return 2;
	else
		return 1;
}

static void sunxi_dw_hdmi_phy_init(struct dw_hdmi *hdmi)
{
	struct sunxi_hdmi_phy * const phy =
		(struct sunxi_hdmi_phy *)(hdmi->ioaddr + HDMI_PHY_OFFS);
	unsigned long tmo;
	u32 tmp;

	/*
	 * HDMI PHY settings are taken as-is from Allwinner BSP code.
	 * There is no documentation.
	 */
	writel(0, &phy->ctrl);
	setbits_le32(&phy->ctrl, BIT(0));
	udelay(5);
	setbits_le32(&phy->ctrl, BIT(16));
	setbits_le32(&phy->ctrl, BIT(1));
	udelay(10);
	setbits_le32(&phy->ctrl, BIT(2));
	udelay(5);
	setbits_le32(&phy->ctrl, BIT(3));
	udelay(40);
	setbits_le32(&phy->ctrl, BIT(19));
	udelay(100);
	setbits_le32(&phy->ctrl, BIT(18));
	setbits_le32(&phy->ctrl, 7 << 4);

	/* Note that Allwinner code doesn't fail in case of timeout */
	tmo = timer_get_us() + 2000;
	while ((readl(&phy->status) & 0x80) == 0) {
		if (timer_get_us() > tmo) {
			printf("Warning: HDMI PHY init timeout!\n");
			break;
		}
	}

	setbits_le32(&phy->ctrl, 0xf << 8);
	setbits_le32(&phy->ctrl, BIT(7));

	writel(0x39dc5040, &phy->pll);
	writel(0x80084343, &phy->clk);
	udelay(10000);
	writel(1, &phy->unk3);
	setbits_le32(&phy->pll, BIT(25));
	udelay(100000);
	tmp = (readl(&phy->status) & 0x1f800) >> 11;
	setbits_le32(&phy->pll, BIT(31) | BIT(30));
	setbits_le32(&phy->pll, tmp);
	writel(0x01FF0F7F, &phy->ctrl);
	writel(0x80639000, &phy->unk1);
	writel(0x0F81C405, &phy->unk2);

	/* enable read access to HDMI controller */
	writel(0x54524545, &phy->read_en);
	/* descramble register offsets */
	writel(0x42494E47, &phy->unscramble);
}

static void sunxi_dw_hdmi_phy_set(struct dw_hdmi *hdmi, uint clock, int phy_div)
{
	struct sunxi_hdmi_phy * const phy =
		(struct sunxi_hdmi_phy *)(hdmi->ioaddr + HDMI_PHY_OFFS);
	int div = sunxi_dw_hdmi_get_divider(clock);
	u32 tmp;

	/*
	 * Unfortunately, we don't know much about those magic
	 * numbers. They are taken from Allwinner BSP driver.
	 */
	switch (div) {
	case 1:
		writel(0x30dc5fc0, &phy->pll);
		writel(0x800863C0 | (phy_div - 1), &phy->clk);
		mdelay(10);
		writel(0x00000001, &phy->unk3);
		setbits_le32(&phy->pll, BIT(25));
		mdelay(200);
		tmp = (readl(&phy->status) & 0x1f800) >> 11;
		setbits_le32(&phy->pll, BIT(31) | BIT(30));
		if (tmp < 0x3d)
			setbits_le32(&phy->pll, tmp + 2);
		else
			setbits_le32(&phy->pll, 0x3f);
		mdelay(100);
		writel(0x01FFFF7F, &phy->ctrl);
		writel(0x8063b000, &phy->unk1);
		writel(0x0F8246B5, &phy->unk2);
		break;
	case 2:
		writel(0x39dc5040, &phy->pll);
		writel(0x80084380 | (phy_div - 1), &phy->clk);
		mdelay(10);
		writel(0x00000001, &phy->unk3);
		setbits_le32(&phy->pll, BIT(25));
		mdelay(100);
		tmp = (readl(&phy->status) & 0x1f800) >> 11;
		setbits_le32(&phy->pll, BIT(31) | BIT(30));
		setbits_le32(&phy->pll, tmp);
		writel(0x01FFFF7F, &phy->ctrl);
		writel(0x8063a800, &phy->unk1);
		writel(0x0F81C485, &phy->unk2);
		break;
	case 4:
		writel(0x39dc5040, &phy->pll);
		writel(0x80084340 | (phy_div - 1), &phy->clk);
		mdelay(10);
		writel(0x00000001, &phy->unk3);
		setbits_le32(&phy->pll, BIT(25));
		mdelay(100);
		tmp = (readl(&phy->status) & 0x1f800) >> 11;
		setbits_le32(&phy->pll, BIT(31) | BIT(30));
		setbits_le32(&phy->pll, tmp);
		writel(0x01FFFF7F, &phy->ctrl);
		writel(0x8063b000, &phy->unk1);
		writel(0x0F81C405, &phy->unk2);
		break;
	case 11:
		writel(0x39dc5040, &phy->pll);
		writel(0x80084300 | (phy_div - 1), &phy->clk);
		mdelay(10);
		writel(0x00000001, &phy->unk3);
		setbits_le32(&phy->pll, BIT(25));
		mdelay(100);
		tmp = (readl(&phy->status) & 0x1f800) >> 11;
		setbits_le32(&phy->pll, BIT(31) | BIT(30));
		setbits_le32(&phy->pll, tmp);
		writel(0x01FFFF7F, &phy->ctrl);
		writel(0x8063b000, &phy->unk1);
		writel(0x0F81C405, &phy->unk2);
		break;
	}
}

static void sunxi_dw_hdmi_pll_set(uint clk_khz, int *phy_div)
{
	int value, n, m, div, diff;
	int best_n = 0, best_m = 0, best_div = 0, best_diff = 0x0FFFFFFF;

	/*
	 * Find the lowest divider resulting in a matching clock. If there
	 * is no match, pick the closest lower clock, as monitors tend to
	 * not sync to higher frequencies.
	 */
	for (div = 1; div <= 16; div++) {
		int target = clk_khz * div;

		if (target < 192000)
			continue;
		if (target > 912000)
			continue;

		for (m = 1; m <= 16; m++) {
			n = (m * target) / 24000;

			if (n >= 1 && n <= 128) {
				value = (24000 * n) / m / div;
				diff = clk_khz - value;
				if (diff < best_diff) {
					best_diff = diff;
					best_m = m;
					best_n = n;
					best_div = div;
				}
			}
		}
	}

	*phy_div = best_div;

	clock_set_pll3_factors(best_m, best_n);
	debug("dotclock: %dkHz = %dkHz: (24MHz * %d) / %d / %d\n",
	      clk_khz, (clock_get_pll3() / 1000) / best_div,
	      best_n, best_m, best_div);
}

static void sunxi_dw_hdmi_lcdc_init(int mux, const struct display_timing *edid,
				    int bpp)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	int div = DIV_ROUND_UP(clock_get_pll3(), edid->pixelclock.typ);
	struct sunxi_lcdc_reg *lcdc;

	if (mux == 0) {
		lcdc = (struct sunxi_lcdc_reg *)SUNXI_LCD0_BASE;

		/* Reset off */
		setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_LCD0);

		/* Clock on */
		setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_LCD0);
		writel(CCM_LCD0_CTRL_GATE | CCM_LCD0_CTRL_M(div),
		       &ccm->lcd0_clk_cfg);
	} else {
		lcdc = (struct sunxi_lcdc_reg *)SUNXI_LCD1_BASE;

		/* Reset off */
		setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_LCD1);

		/* Clock on */
		setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_LCD1);
		writel(CCM_LCD1_CTRL_GATE | CCM_LCD1_CTRL_M(div),
		       &ccm->lcd1_clk_cfg);
	}

	lcdc_init(lcdc);
	lcdc_tcon1_mode_set(lcdc, edid, false, false);
	lcdc_enable(lcdc, bpp);
}

static int sunxi_dw_hdmi_phy_cfg(struct dw_hdmi *hdmi, uint mpixelclock)
{
	int phy_div;

	sunxi_dw_hdmi_pll_set(mpixelclock / 1000, &phy_div);
	sunxi_dw_hdmi_phy_set(hdmi, mpixelclock, phy_div);

	return 0;
}

static int sunxi_dw_hdmi_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	struct sunxi_dw_hdmi_priv *priv = dev_get_priv(dev);

	return dw_hdmi_read_edid(&priv->hdmi, buf, buf_size);
}

static bool sunxi_dw_hdmi_mode_valid(struct udevice *dev,
				     const struct display_timing *timing)
{
	return timing->pixelclock.typ <= 297000000;
}

static int sunxi_dw_hdmi_enable(struct udevice *dev, int panel_bpp,
				const struct display_timing *edid)
{
	struct sunxi_dw_hdmi_priv *priv = dev_get_priv(dev);
	struct sunxi_hdmi_phy * const phy =
		(struct sunxi_hdmi_phy *)(priv->hdmi.ioaddr + HDMI_PHY_OFFS);
	struct display_plat *uc_plat = dev_get_uclass_plat(dev);
	int ret;

	ret = dw_hdmi_enable(&priv->hdmi, edid);
	if (ret)
		return ret;

	sunxi_dw_hdmi_lcdc_init(uc_plat->source_id, edid, panel_bpp);

	if (edid->flags & DISPLAY_FLAGS_VSYNC_LOW)
		setbits_le32(&phy->pol, 0x200);

	if (edid->flags & DISPLAY_FLAGS_HSYNC_LOW)
		setbits_le32(&phy->pol, 0x100);

	setbits_le32(&phy->ctrl, 0xf << 12);

	/*
	 * This is last hdmi access before boot, so scramble addresses
	 * again or othwerwise BSP driver won't work. Dummy read is
	 * needed or otherwise last write doesn't get written correctly.
	 */
	(void)readb(priv->hdmi.ioaddr);
	writel(0, &phy->unscramble);

	return 0;
}

static int sunxi_dw_hdmi_probe(struct udevice *dev)
{
	struct sunxi_dw_hdmi_priv *priv = dev_get_priv(dev);
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	int ret;

	if (priv->hvcc)
		regulator_set_enable(priv->hvcc, true);

	/* Set pll3 to 297 MHz */
	clock_set_pll3(297000000);

	/* Set hdmi parent to pll3 */
	clrsetbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_PLL_MASK,
			CCM_HDMI_CTRL_PLL3);

	/* This reset is referenced from the PHY devicetree node. */
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_HDMI2);

	ret = reset_deassert_bulk(&priv->resets);
	if (ret)
		return ret;

	ret = clk_enable_bulk(&priv->clocks);
	if (ret)
		return ret;

	sunxi_dw_hdmi_phy_init(&priv->hdmi);

	ret = dw_hdmi_phy_wait_for_hpd(&priv->hdmi);
	if (ret < 0) {
		debug("hdmi can not get hpd signal\n");
		return -1;
	}

	dw_hdmi_init(&priv->hdmi);

	return 0;
}

static int sunxi_dw_hdmi_of_to_plat(struct udevice *dev)
{
	struct sunxi_dw_hdmi_priv *priv = dev_get_priv(dev);
	struct dw_hdmi *hdmi = &priv->hdmi;
	int ret;

	hdmi->ioaddr = (ulong)dev_read_addr(dev);
	hdmi->i2c_clk_high = 0xd8;
	hdmi->i2c_clk_low = 0xfe;
	hdmi->reg_io_width = 1;
	hdmi->phy_set = sunxi_dw_hdmi_phy_cfg;

	ret = reset_get_bulk(dev, &priv->resets);
	if (ret)
		return ret;

	ret = clk_get_bulk(dev, &priv->clocks);
	if (ret)
		return ret;

	ret = device_get_supply_regulator(dev, "hvcc-supply", &priv->hvcc);
	if (ret)
		priv->hvcc = NULL;

	return 0;
}

static const struct dm_display_ops sunxi_dw_hdmi_ops = {
	.read_edid = sunxi_dw_hdmi_read_edid,
	.enable = sunxi_dw_hdmi_enable,
	.mode_valid = sunxi_dw_hdmi_mode_valid,
};

static const struct udevice_id sunxi_dw_hdmi_ids[] = {
	{ .compatible = "allwinner,sun8i-a83t-dw-hdmi" },
	{ }
};

U_BOOT_DRIVER(sunxi_dw_hdmi) = {
	.name		= "sunxi_dw_hdmi",
	.id		= UCLASS_DISPLAY,
	.of_match	= sunxi_dw_hdmi_ids,
	.probe		= sunxi_dw_hdmi_probe,
	.of_to_plat	= sunxi_dw_hdmi_of_to_plat,
	.priv_auto	= sizeof(struct sunxi_dw_hdmi_priv),
	.ops		= &sunxi_dw_hdmi_ops,
};
