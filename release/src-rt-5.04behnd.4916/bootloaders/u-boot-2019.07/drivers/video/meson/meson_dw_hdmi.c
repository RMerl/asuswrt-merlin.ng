// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 BayLibre, SAS
 * Author: Jorge Ramirez-Ortiz <jramirez@baylibre.com>
 */

#include <common.h>
#include <display.h>
#include <dm.h>
#include <edid.h>
#include <asm/io.h>
#include <dw_hdmi.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <power/regulator.h>
#include <clk.h>
#include <linux/delay.h>
#include <reset.h>
#include <media_bus_format.h>
#include "meson_dw_hdmi.h"
#include "meson_vpu.h"

/* TOP Block Communication Channel */
#define HDMITX_TOP_ADDR_REG	0x0
#define HDMITX_TOP_DATA_REG	0x4
#define HDMITX_TOP_CTRL_REG	0x8

/* Controller Communication Channel */
#define HDMITX_DWC_ADDR_REG	0x10
#define HDMITX_DWC_DATA_REG	0x14
#define HDMITX_DWC_CTRL_REG	0x18

/* HHI Registers */
#define HHI_MEM_PD_REG0		0x100 /* 0x40 */
#define HHI_HDMI_CLK_CNTL	0x1cc /* 0x73 */
#define HHI_HDMI_PHY_CNTL0	0x3a0 /* 0xe8 */
#define HHI_HDMI_PHY_CNTL1	0x3a4 /* 0xe9 */
#define HHI_HDMI_PHY_CNTL2	0x3a8 /* 0xea */
#define HHI_HDMI_PHY_CNTL3	0x3ac /* 0xeb */

struct meson_dw_hdmi {
	struct udevice *dev;
	struct dw_hdmi hdmi;
	void __iomem *hhi_base;
};

enum hdmi_compatible {
	HDMI_COMPATIBLE_GXBB = 0,
	HDMI_COMPATIBLE_GXL = 1,
	HDMI_COMPATIBLE_GXM = 2,
};

static inline bool meson_hdmi_is_compatible(struct meson_dw_hdmi *priv,
					    enum hdmi_compatible family)
{
	enum hdmi_compatible compat = dev_get_driver_data(priv->dev);

	return compat == family;
}

static unsigned int dw_hdmi_top_read(struct dw_hdmi *hdmi, unsigned int addr)
{
	unsigned int data;

	/* ADDR must be written twice */
	writel(addr & 0xffff, hdmi->ioaddr + HDMITX_TOP_ADDR_REG);
	writel(addr & 0xffff, hdmi->ioaddr + HDMITX_TOP_ADDR_REG);

	/* Read needs a second DATA read */
	data = readl(hdmi->ioaddr + HDMITX_TOP_DATA_REG);
	data = readl(hdmi->ioaddr + HDMITX_TOP_DATA_REG);

	return data;
}

static inline void dw_hdmi_top_write(struct dw_hdmi *hdmi,
				     unsigned int addr, unsigned int data)
{
	/* ADDR must be written twice */
	writel(addr & 0xffff, hdmi->ioaddr + HDMITX_TOP_ADDR_REG);
	writel(addr & 0xffff, hdmi->ioaddr + HDMITX_TOP_ADDR_REG);

	/* Write needs single DATA write */
	writel(data, hdmi->ioaddr + HDMITX_TOP_DATA_REG);
}

static inline void dw_hdmi_top_write_bits(struct dw_hdmi *hdmi,
					  unsigned int addr,
					  unsigned int mask,
					  unsigned int val)
{
	unsigned int data = dw_hdmi_top_read(hdmi, addr);

	data &= ~mask;
	data |= val;
	dw_hdmi_top_write(hdmi, addr, data);
}

static u8 dw_hdmi_dwc_read(struct dw_hdmi *hdmi, int addr)
{
	unsigned int data;

	/* ADDR must be written twice */
	writel(addr & 0xffff, hdmi->ioaddr + HDMITX_DWC_ADDR_REG);
	writel(addr & 0xffff, hdmi->ioaddr + HDMITX_DWC_ADDR_REG);

	/* Read needs a second DATA read */
	data = readl(hdmi->ioaddr + HDMITX_DWC_DATA_REG);
	data = readl(hdmi->ioaddr + HDMITX_DWC_DATA_REG);

	return data;
}

static inline void dw_hdmi_dwc_write(struct dw_hdmi *hdmi, u8 data, int addr)
{
	/* ADDR must be written twice */
	writel(addr & 0xffff, hdmi->ioaddr + HDMITX_DWC_ADDR_REG);
	writel(addr & 0xffff, hdmi->ioaddr + HDMITX_DWC_ADDR_REG);

	/* Write needs single DATA write */
	writel(data, hdmi->ioaddr + HDMITX_DWC_DATA_REG);
}

static inline void dw_hdmi_dwc_write_bits(struct dw_hdmi *hdmi,
					  unsigned int addr,
					  unsigned int mask,
					  unsigned int val)
{
	u8 data = dw_hdmi_dwc_read(hdmi, addr);

	data &= ~mask;
	data |= val;

	dw_hdmi_dwc_write(hdmi, data, addr);
}

static inline void dw_hdmi_hhi_write(struct meson_dw_hdmi *priv,
				     unsigned int addr, unsigned int data)
{
	hhi_write(addr, data);
}

__attribute__((unused))
static unsigned int dw_hdmi_hhi_read(struct meson_dw_hdmi *priv,
				     unsigned int addr)
{
	return hhi_read(addr);
}

static inline void dw_hdmi_hhi_update_bits(struct meson_dw_hdmi *priv,
					   unsigned int addr,
					   unsigned int mask,
					   unsigned int val)
{
	hhi_update_bits(addr, mask, val);
}

static int meson_dw_hdmi_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
#if defined DEBUG
	struct display_timing timing;
	int panel_bits_per_colour;
#endif
	struct meson_dw_hdmi *priv = dev_get_priv(dev);
	int ret;

	ret = dw_hdmi_read_edid(&priv->hdmi, buf, buf_size);

#if defined DEBUG
	if (!ret)
		return ret;

	edid_print_info((struct edid1_info *)buf);
	edid_get_timing(buf, ret, &timing, &panel_bits_per_colour);
	debug("Display timing:\n");
	debug(" hactive %04d, hfrontp %04d, hbackp %04d hsync %04d\n"
	      " vactive %04d, vfrontp %04d, vbackp %04d vsync %04d\n",
	       timing.hactive.typ, timing.hfront_porch.typ,
	       timing.hback_porch.typ, timing.hsync_len.typ,
	       timing.vactive.typ, timing.vfront_porch.typ,
	       timing.vback_porch.typ, timing.vsync_len.typ);
	debug(" flags: ");
	if (timing.flags & DISPLAY_FLAGS_INTERLACED)
		debug("interlaced ");
	if (timing.flags & DISPLAY_FLAGS_DOUBLESCAN)
		debug("doublescan ");
	if (timing.flags & DISPLAY_FLAGS_DOUBLECLK)
		debug("doubleclk ");
	if (timing.flags & DISPLAY_FLAGS_HSYNC_LOW)
		debug("hsync_low ");
	if (timing.flags & DISPLAY_FLAGS_HSYNC_HIGH)
		debug("hsync_high ");
	if (timing.flags & DISPLAY_FLAGS_VSYNC_LOW)
		debug("vsync_low ");
	if (timing.flags & DISPLAY_FLAGS_VSYNC_HIGH)
		debug("vsync_high ");
	debug("\n");
#endif

	return ret;
}

static inline void meson_dw_hdmi_phy_reset(struct meson_dw_hdmi *priv)
{
	/* Enable and software reset */
	dw_hdmi_hhi_update_bits(priv, HHI_HDMI_PHY_CNTL1, 0xf, 0xf);

	mdelay(2);

	/* Enable and unreset */
	dw_hdmi_hhi_update_bits(priv, HHI_HDMI_PHY_CNTL1, 0xf, 0xe);

	mdelay(2);
}

static void meson_dw_hdmi_phy_setup_mode(struct meson_dw_hdmi *priv,
					 uint pixel_clock)
{
	pixel_clock = pixel_clock / 1000;

	if (meson_hdmi_is_compatible(priv, HDMI_COMPATIBLE_GXL) ||
	    meson_hdmi_is_compatible(priv, HDMI_COMPATIBLE_GXM)) {
		if (pixel_clock >= 371250) {
			/* 5.94Gbps, 3.7125Gbps */
			hhi_write(HHI_HDMI_PHY_CNTL0, 0x333d3282);
			hhi_write(HHI_HDMI_PHY_CNTL3, 0x2136315b);
		} else if (pixel_clock >= 297000) {
			/* 2.97Gbps */
			hhi_write(HHI_HDMI_PHY_CNTL0, 0x33303382);
			hhi_write(HHI_HDMI_PHY_CNTL3, 0x2036315b);
		} else if (pixel_clock >= 148500) {
			/* 1.485Gbps */
			hhi_write(HHI_HDMI_PHY_CNTL0, 0x33303362);
			hhi_write(HHI_HDMI_PHY_CNTL3, 0x2016315b);
		} else {
			/* 742.5Mbps, and below */
			hhi_write(HHI_HDMI_PHY_CNTL0, 0x33604142);
			hhi_write(HHI_HDMI_PHY_CNTL3, 0x0016315b);
		}
	} else {
		if (pixel_clock >= 371250) {
			/* 5.94Gbps, 3.7125Gbps */
			hhi_write(HHI_HDMI_PHY_CNTL0, 0x33353245);
			hhi_write(HHI_HDMI_PHY_CNTL3, 0x2100115b);
		} else if (pixel_clock >= 297000) {
			/* 2.97Gbps */
			hhi_write(HHI_HDMI_PHY_CNTL0, 0x33634283);
			hhi_write(HHI_HDMI_PHY_CNTL3, 0xb000115b);
		} else {
			/* 1.485Gbps, and below */
			hhi_write(HHI_HDMI_PHY_CNTL0, 0x33632122);
			hhi_write(HHI_HDMI_PHY_CNTL3, 0x2000115b);
		}
	}
}

static int meson_dw_hdmi_phy_init(struct dw_hdmi *hdmi, uint pixel_clock)
{
	struct meson_dw_hdmi *priv = container_of(hdmi, struct meson_dw_hdmi,
						  hdmi);
	/* Enable clocks */
	dw_hdmi_hhi_update_bits(priv, HHI_HDMI_CLK_CNTL, 0xffff, 0x100);

	/* Bring HDMITX MEM output of power down */
	dw_hdmi_hhi_update_bits(priv, HHI_MEM_PD_REG0, 0xff << 8, 0);

	/* Bring out of reset */
	dw_hdmi_top_write(hdmi, HDMITX_TOP_SW_RESET,  0);

	/* Enable internal pixclk, tmds_clk, spdif_clk, i2s_clk, cecclk */
	dw_hdmi_top_write_bits(hdmi, HDMITX_TOP_CLK_CNTL, 0x3, 0x3);
	dw_hdmi_top_write_bits(hdmi, HDMITX_TOP_CLK_CNTL, 0x3 << 4, 0x3 << 4);

	/* Enable normal output to PHY */
	dw_hdmi_top_write(hdmi, HDMITX_TOP_BIST_CNTL, BIT(12));

	/* TMDS pattern setup (TOFIX pattern for 4k2k scrambling) */
	dw_hdmi_top_write(hdmi, HDMITX_TOP_TMDS_CLK_PTTN_01, 0x001f001f);
	dw_hdmi_top_write(hdmi, HDMITX_TOP_TMDS_CLK_PTTN_23, 0x001f001f);

	/* Load TMDS pattern */
	dw_hdmi_top_write(hdmi, HDMITX_TOP_TMDS_CLK_PTTN_CNTL, 0x1);
	mdelay(20);
	dw_hdmi_top_write(hdmi, HDMITX_TOP_TMDS_CLK_PTTN_CNTL, 0x2);

	/* Setup PHY parameters */
	meson_dw_hdmi_phy_setup_mode(priv, pixel_clock);

	/* Setup PHY */
	dw_hdmi_hhi_update_bits(priv, HHI_HDMI_PHY_CNTL1,
				0xffff << 16, 0x0390 << 16);

	/* BIT_INVERT */
	if (meson_hdmi_is_compatible(priv, HDMI_COMPATIBLE_GXL) ||
	    meson_hdmi_is_compatible(priv, HDMI_COMPATIBLE_GXM))
		dw_hdmi_hhi_update_bits(priv, HHI_HDMI_PHY_CNTL1, BIT(17), 0);
	else
		dw_hdmi_hhi_update_bits(priv, HHI_HDMI_PHY_CNTL1,
					BIT(17), BIT(17));

	/* Disable clock, fifo, fifo_wr */
	dw_hdmi_hhi_update_bits(priv, HHI_HDMI_PHY_CNTL1, 0xf, 0);

	mdelay(100);

	/* Reset PHY 3 times in a row */
	meson_dw_hdmi_phy_reset(priv);
	meson_dw_hdmi_phy_reset(priv);
	meson_dw_hdmi_phy_reset(priv);

	return 0;
}

static int meson_dw_hdmi_enable(struct udevice *dev, int panel_bpp,
				const struct display_timing *edid)
{
	struct meson_dw_hdmi *priv = dev_get_priv(dev);

	/* will back into meson_dw_hdmi_phy_init */
	return dw_hdmi_enable(&priv->hdmi, edid);
}

static int meson_dw_hdmi_wait_hpd(struct dw_hdmi *hdmi)
{
	int i;

	/* Poll 1 second for HPD signal */
	for (i = 0; i < 10; ++i) {
		if (dw_hdmi_top_read(hdmi, HDMITX_TOP_STAT0))
			return 0;

		mdelay(100);
	}

	return -ETIMEDOUT;
}

static int meson_dw_hdmi_probe(struct udevice *dev)
{
	struct meson_dw_hdmi *priv = dev_get_priv(dev);
	struct reset_ctl_bulk resets;
	struct clk_bulk clocks;
	struct udevice *supply;
	int ret;

	priv->dev = dev;

	priv->hdmi.ioaddr = (ulong)dev_remap_addr_index(dev, 0);
	if (!priv->hdmi.ioaddr)
		return -EINVAL;

	priv->hhi_base = dev_remap_addr_index(dev, 1);
	if (!priv->hhi_base)
		return -EINVAL;

	priv->hdmi.hdmi_data.enc_out_bus_format = MEDIA_BUS_FMT_RGB888_1X24;
	priv->hdmi.hdmi_data.enc_in_bus_format = MEDIA_BUS_FMT_YUV8_1X24;
	priv->hdmi.phy_set = meson_dw_hdmi_phy_init;
	priv->hdmi.write_reg = dw_hdmi_dwc_write;
	priv->hdmi.read_reg = dw_hdmi_dwc_read;
	priv->hdmi.i2c_clk_high = 0x67;
	priv->hdmi.i2c_clk_low = 0x78;

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	ret = device_get_supply_regulator(dev, "hdmi-supply", &supply);
	if (ret && ret != -ENOENT) {
		pr_err("Failed to get HDMI regulator\n");
		return ret;
	}

	if (!ret) {
		ret = regulator_set_enable(supply, true);
		if (ret)
			return ret;
	}
#endif

	ret = reset_get_bulk(dev, &resets);
	if (ret)
		return ret;

	ret = clk_get_bulk(dev, &clocks);
	if (ret)
		return ret;

	ret = clk_enable_bulk(&clocks);
	if (ret)
		return ret;

	/* Enable clocks */
	dw_hdmi_hhi_update_bits(priv, HHI_HDMI_CLK_CNTL, 0xffff, 0x100);

	/* Bring HDMITX MEM output of power down */
	dw_hdmi_hhi_update_bits(priv, HHI_MEM_PD_REG0, 0xff << 8, 0);

	/* Reset HDMITX APB & TX & PHY: cycle needed for EDID */
	ret = reset_deassert_bulk(&resets);
	if (ret)
		return ret;

	ret = reset_assert_bulk(&resets);
	if (ret)
		return ret;

	ret = reset_deassert_bulk(&resets);
	if (ret)
		return ret;

	/* Enable APB3 fail on error */
	writel_bits(BIT(15), BIT(15), priv->hdmi.ioaddr + HDMITX_TOP_CTRL_REG);
	writel_bits(BIT(15), BIT(15), priv->hdmi.ioaddr + HDMITX_DWC_CTRL_REG);

	/* Bring out of reset */
	dw_hdmi_top_write(&priv->hdmi, HDMITX_TOP_SW_RESET,  0);
	mdelay(20);
	dw_hdmi_top_write(&priv->hdmi, HDMITX_TOP_CLK_CNTL, 0xff);

	dw_hdmi_init(&priv->hdmi);
	dw_hdmi_phy_init(&priv->hdmi);

	/* wait for connector */
	ret = meson_dw_hdmi_wait_hpd(&priv->hdmi);
	if (ret)
		debug("hdmi can not get hpd signal\n");

	return ret;
}

static const struct dm_display_ops meson_dw_hdmi_ops = {
	.read_edid = meson_dw_hdmi_read_edid,
	.enable = meson_dw_hdmi_enable,
};

static const struct udevice_id meson_dw_hdmi_ids[] = {
	{ .compatible = "amlogic,meson-gxbb-dw-hdmi",
		.data = HDMI_COMPATIBLE_GXBB },
	{ .compatible = "amlogic,meson-gxl-dw-hdmi",
		.data = HDMI_COMPATIBLE_GXL },
	{ .compatible = "amlogic,meson-gxm-dw-hdmi",
		.data = HDMI_COMPATIBLE_GXM },
	{ }
};

U_BOOT_DRIVER(meson_dw_hdmi) = {
	.name = "meson_dw_hdmi",
	.id = UCLASS_DISPLAY,
	.of_match = meson_dw_hdmi_ids,
	.ops = &meson_dw_hdmi_ops,
	.probe = meson_dw_hdmi_probe,
	.priv_auto_alloc_size = sizeof(struct meson_dw_hdmi),
};
