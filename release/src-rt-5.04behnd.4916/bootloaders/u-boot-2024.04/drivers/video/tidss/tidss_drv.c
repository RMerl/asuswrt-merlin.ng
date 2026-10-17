// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023 Texas Instruments Incorporated - https://www.ti.com/
 * Nikhil M Jain, n-jain1@ti.com
 *
 * based on the linux tidss driver, which is
 *
 * (C) Copyright 2018 Texas Instruments Incorporated - https://www.ti.com/
 * Author: Tomi Valkeinen <tomi.valkeinen@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <log.h>
#include <video.h>
#include <errno.h>
#include <panel.h>
#include <reset.h>
#include <malloc.h>
#include <fdtdec.h>
#include <syscon.h>
#include <regmap.h>
#include <cpu_func.h>
#include <media_bus_format.h>

#include <asm/io.h>
#include <asm/cache.h>
#include <asm/utils.h>
#include <asm/bitops.h>

#include <dm/devres.h>
#include <dm/of_access.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>

#include <linux/bug.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#include "tidss_drv.h"
#include "tidss_regs.h"

DECLARE_GLOBAL_DATA_PTR;

/* Panel parameters */
enum {
	LCD_MAX_WIDTH		= 1920,
	LCD_MAX_HEIGHT		= 1200,
	LCD_MAX_LOG2_BPP	= VIDEO_BPP32,
};

static const u16 *dss_common_regmap;

static const u16 tidss_am62x_common_regs[DSS_COMMON_REG_TABLE_LEN] = {
	[DSS_REVISION_OFF] =			0x4,
	[DSS_SYSCONFIG_OFF] =			0x8,
	[DSS_SYSSTATUS_OFF] =			0x20,
	[DSS_IRQ_EOI_OFF] =			0x24,
	[DSS_IRQSTATUS_RAW_OFF] =		0x28,
	[DSS_IRQSTATUS_OFF] =			0x2c,
	[DSS_IRQENABLE_SET_OFF] =		0x30,
	[DSS_IRQENABLE_CLR_OFF] =		0x40,
	[DSS_VID_IRQENABLE_OFF] =		0x44,
	[DSS_VID_IRQSTATUS_OFF] =		0x58,
	[DSS_VP_IRQENABLE_OFF] =		0x70,
	[DSS_VP_IRQSTATUS_OFF] =		0x7c,

	[WB_IRQENABLE_OFF] =			0x88,
	[WB_IRQSTATUS_OFF] =			0x8c,

	[DSS_GLOBAL_MFLAG_ATTRIBUTE_OFF] =	0x90,
	[DSS_GLOBAL_OUTPUT_ENABLE_OFF] =	0x94,
	[DSS_GLOBAL_BUFFER_OFF] =		0x98,
	[DSS_CBA_CFG_OFF] =			0x9c,
	[DSS_DBG_CONTROL_OFF] =		0xa0,
	[DSS_DBG_STATUS_OFF] =		0xa4,
	[DSS_CLKGATING_DISABLE_OFF] =		0xa8,
	[DSS_SECURE_DISABLE_OFF] =		0xac,
};

/* TIDSS AM62x Features */
const struct dss_features dss_am625_feats = {
	.max_pclk_khz = {
		[DSS_VP_DPI] = 165000,
		[DSS_VP_OLDI] = 165000,
	},

	.subrev = DSS_AM625,

	.common = "common",
	.common_regs = tidss_am62x_common_regs,

	.num_vps = 2,
	.vp_name = { "vp1", "vp2" },
	.ovr_name = { "ovr1", "ovr2" },
	.vpclk_name =  { "vp1", "vp2" },
	.vp_bus_type = { DSS_VP_OLDI, DSS_VP_DPI },

	.vp_feat = { .color = {
			.has_ctm = true,
			.gamma_size = 256,
			.gamma_type = TIDSS_GAMMA_8BIT,
		},
	},

	.num_planes = 2,
	/* note: vid is plane_id 0 and vidl1 is plane_id 1 */
	.vid_name = { "vidl1", "vid" },
	.vid_lite = { true, false },
	.vid_order = { 1, 0 },
};

/* Wrapper functions to write and read TI_DSS registers */
static void dss_write(struct tidss_drv_priv *priv, u16 reg, u32 val)
{
	writel(val, priv->base_common + reg);
}

static u32 dss_read(struct tidss_drv_priv *priv, u16 reg)
{
	return readl(priv->base_common + reg);
}

static void dss_vid_write(struct tidss_drv_priv *priv, u32 hw_plane, u16 reg, u32 val)
{
	void __iomem *base = priv->base_vid[hw_plane];

	writel(val, base + reg);
}

static u32 dss_vid_read(struct tidss_drv_priv *priv, u32 hw_plane, u16 reg)
{
	void __iomem *base = priv->base_vid[hw_plane];

	return readl(base + reg);
}

static void dss_ovr_write(struct tidss_drv_priv *priv, u32 hw_videoport,
			  u16 reg, u32 val)
{
	void __iomem *base = priv->base_ovr[hw_videoport];

	writel(val, base + reg);
}

static u32 dss_ovr_read(struct tidss_drv_priv *priv, u32 hw_videoport, u16 reg)
{
	void __iomem *base = priv->base_ovr[hw_videoport];

	return readl(base + reg);
}

static void dss_vp_write(struct tidss_drv_priv *priv, u32 hw_videoport,
			 u16 reg, u32 val)
{
	void __iomem *base = priv->base_vp[hw_videoport];

	writel(val, base + reg);
}

static u32 dss_vp_read(struct tidss_drv_priv *priv, u32 hw_videoport, u16 reg)
{
	void __iomem *base = priv->base_vp[hw_videoport];

	return readl(base + reg);
}

/* generate mask on a register */
static u32 FLD_MASK(u32 start, u32 end)
{
	return ((1 << (start - end + 1)) - 1) << end;
}

/* set the given val in specified range */
static u32 FLD_VAL(u32 val, u32 start, u32 end)
{
	return (val << end) & FLD_MASK(start, end);
}

/* return the value in the specified range */
static u32 FLD_GET(u32 val, u32 start, u32 end)
{
	return (val & FLD_MASK(start, end)) >> end;
}

/* modify the value of the specified range */
static u32 FLD_MOD(u32 orig, u32 val, u32 start, u32 end)
{
	return (orig & ~FLD_MASK(start, end)) | FLD_VAL(val, start, end);
}

/* read and modify common register region of DSS*/
__maybe_unused
static u32 REG_GET(struct tidss_drv_priv *priv, u32 idx, u32 start, u32 end)
{
	return FLD_GET(dss_read(priv, idx), start, end);
}

static void REG_FLD_MOD(struct tidss_drv_priv *priv, u32 idx, u32 val,
			u32 start, u32 end)
{
	dss_write(priv, idx, FLD_MOD(dss_read(priv, idx), val,
				     start, end));
}

/* read and modify planes vid1 and vid2 register of DSS*/
static u32 VID_REG_GET(struct tidss_drv_priv *priv, u32 hw_plane, u32 idx,
		       u32 start, u32 end)
{
	return FLD_GET(dss_vid_read(priv, hw_plane, idx), start, end);
}

static void VID_REG_FLD_MOD(struct tidss_drv_priv *priv, u32 hw_plane, u32 idx,
			    u32 val, u32 start, u32 end)
{
	dss_vid_write(priv, hw_plane, idx,
		      FLD_MOD(dss_vid_read(priv, hw_plane, idx),
			      val, start, end));
}

/* read and modify port vid1 and vid2 registers of DSS*/
__maybe_unused
static u32 VP_REG_GET(struct tidss_drv_priv *priv, u32 vp, u32 idx,
		      u32 start, u32 end)
{
	return FLD_GET(dss_vp_read(priv, vp, idx), start, end);
}

static void VP_REG_FLD_MOD(struct tidss_drv_priv *priv, u32 vp, u32 idx, u32 val,
			   u32 start, u32 end)
{
	dss_vp_write(priv, vp, idx, FLD_MOD(dss_vp_read(priv, vp, idx),
					    val, start, end));
}

/* read and modify overlay ovr1 and ovr2 registers of DSS*/
__maybe_unused
static u32 OVR_REG_GET(struct tidss_drv_priv *priv, u32 ovr, u32 idx,
		       u32 start, u32 end)
{
	return FLD_GET(dss_ovr_read(priv, ovr, idx), start, end);
}

static void OVR_REG_FLD_MOD(struct tidss_drv_priv *priv, u32 ovr, u32 idx,
			    u32 val, u32 start, u32 end)
{
	dss_ovr_write(priv, ovr, idx, FLD_MOD(dss_ovr_read(priv, ovr, idx),
					      val, start, end));
}

static void dss_oldi_tx_power(struct tidss_drv_priv *priv, bool power)
{
	u32 val;

	if (WARN_ON(!priv->oldi_io_ctrl))
		return;

	if (priv->feat->subrev == DSS_AM625) {
		if (power) {
			switch (priv->oldi_mode) {
			case OLDI_SINGLE_LINK_SINGLE_MODE:
				/* Power down OLDI TX 1 */
				val = OLDI1_PWRDN_TX;
				break;
			case OLDI_DUAL_LINK:
				/* No Power down */
				val = 0;
			break;
			default:
				/* Power down both the OLDI TXes */
				val = OLDI_BANDGAP_PWR | OLDI0_PWRDN_TX | OLDI1_PWRDN_TX;
				break;
			}
		} else {
			val = OLDI_BANDGAP_PWR | OLDI0_PWRDN_TX | OLDI1_PWRDN_TX;
		}
		regmap_update_bits(priv->oldi_io_ctrl, OLDI_PD_CTRL,
				   OLDI_BANDGAP_PWR | OLDI0_PWRDN_TX | OLDI1_PWRDN_TX, val);
	}
}

static void dss_set_num_datalines(struct tidss_drv_priv *priv,
				  u32 hw_videoport)
{
	int v;
	u32 num_lines = priv->bus_format->data_width;

	switch (num_lines) {
	case 12:
		v = 0; break;
	case 16:
		v = 1; break;
	case 18:
		v = 2; break;
	case 24:
		v = 3; break;
	case 30:
		v = 4; break;
	case 36:
		v = 5; break;
	default:
		WARN_ON(1);
		v = 3;
	}

	VP_REG_FLD_MOD(priv, hw_videoport, DSS_VP_CONTROL, v, 10, 8);
}

static void dss_enable_oldi(struct tidss_drv_priv *priv, u32 hw_videoport)
{
	u32 oldi_cfg = 0;
	u32 oldi_reset_bit = BIT(5 + hw_videoport);
	int count = 0;

	/*
	 * For the moment MASTERSLAVE, and SRC bits of DSS_VP_DSS_OLDI_CFG are
	 * set statically to 0.
	 */

	if (priv->bus_format->data_width == 24)
		oldi_cfg |= BIT(8); /* MSB */
	else if (priv->bus_format->data_width != 18)
		dev_warn(priv->dev, "%s: %d port width not supported\n",
			 __func__, priv->bus_format->data_width);

	oldi_cfg |= BIT(7); /* DEPOL */

	oldi_cfg = FLD_MOD(oldi_cfg, priv->bus_format->oldi_mode_reg_val, 3, 1);

	oldi_cfg |= BIT(12); /* SOFTRST */

	oldi_cfg |= BIT(0); /* ENABLE */

	switch (priv->oldi_mode) {
	case OLDI_MODE_OFF:
		oldi_cfg &= ~BIT(0); /* DISABLE */
		break;

	case OLDI_SINGLE_LINK_SINGLE_MODE:
		/* All configuration is done for this mode.  */
		break;

	case OLDI_SINGLE_LINK_DUPLICATE_MODE:
		oldi_cfg |= BIT(5); /* DUPLICATE MODE */
		break;

	case OLDI_DUAL_LINK:
		oldi_cfg |= BIT(11); /* DUALMODESYNC */
		oldi_cfg |= BIT(3); /* data-mapping field also indicates dual-link mode */
		break;

	default:
		dev_warn(priv->dev, "%s: Incorrect oldi mode. Returning.\n",
			 __func__);
		return;
	}

	dss_vp_write(priv, hw_videoport, DSS_VP_DSS_OLDI_CFG, oldi_cfg);

	while (!(oldi_reset_bit & dss_read(priv, DSS_SYSSTATUS)) &&
	       count < 10000)
		count++;

	if (!(oldi_reset_bit & dss_read(priv, DSS_SYSSTATUS)))
		dev_warn(priv->dev, "%s: timeout waiting OLDI reset done\n",
			 __func__);
}

static const struct dss_color_lut dss_vp_gamma_default_lut[] = {
	{ .red = 0, .green = 0, .blue = 0, },
	{ .red = U16_MAX, .green = U16_MAX, .blue = U16_MAX, },
};

static void dss_vp_write_gamma_table(struct tidss_drv_priv *priv,
				     u32 hw_videoport)
{
	u32 *table = priv->vp_data[hw_videoport].gamma_table;
	u32 hwlen = priv->feat->vp_feat.color.gamma_size;
	unsigned int i;

	dev_dbg(priv->dev, "%s: hw_videoport %d\n", __func__, hw_videoport);

	for (i = 0; i < hwlen; ++i) {
		u32 v = table[i];

		v |= i << 24;

		dss_vp_write(priv, hw_videoport, DSS_VP_GAMMA_TABLE, v);
	}
}

static void dss_vp_set_gamma(struct tidss_drv_priv *priv,
			     u32 hw_videoport, const struct dss_color_lut *lut,
			     unsigned int length)
{
	u32 *table = priv->vp_data[hw_videoport].gamma_table;
	u32 hwlen = priv->feat->vp_feat.color.gamma_size;
	u32 hwbits;
	unsigned int i;

	dev_dbg(priv->dev, "%s: hw_videoport %d, lut len %u, hw len %u\n",
		__func__, hw_videoport, length, hwlen);

	if (priv->feat->vp_feat.color.gamma_type == TIDSS_GAMMA_10BIT)
		hwbits = 10;
	else
		hwbits = 8;

	lut = dss_vp_gamma_default_lut;
	length = ARRAY_SIZE(dss_vp_gamma_default_lut);

	for (i = 0; i < length - 1; ++i) {
		unsigned int first = i * (hwlen - 1) / (length - 1);
		unsigned int last = (i + 1) * (hwlen - 1) / (length - 1);
		unsigned int w = last - first;
		u16 r, g, b;
		unsigned int j;

		if (w == 0)
			continue;

		for (j = 0; j <= w; j++) {
			r = (lut[i].red * (w - j) + lut[i + 1].red * j) / w;
			g = (lut[i].green * (w - j) + lut[i + 1].green * j) / w;
			b = (lut[i].blue * (w - j) + lut[i + 1].blue * j) / w;

			r >>= 16 - hwbits;
			g >>= 16 - hwbits;
			b >>= 16 - hwbits;

			table[first + j] = (r << (hwbits * 2)) |
				(g << hwbits) | b;
		}
	}

	dss_vp_write_gamma_table(priv, hw_videoport);
}

void dss_vp_enable(struct tidss_drv_priv *priv, u32 hw_videoport, struct display_timing *timing)
{
	bool align, onoff, rf, ieo, ipc, ihs, ivs;
	u32 hsw, hfp, hbp, vsw, vfp, vbp;

	dss_set_num_datalines(priv, hw_videoport);

	/* panel parameters to set clocks for video port*/
	hfp = timing->hfront_porch.typ;
	hsw = timing->hsync_len.typ;
	hbp = timing->hback_porch.typ;
	vfp = timing->vfront_porch.typ;
	vsw = timing->vsync_len.typ;
	vbp = timing->vback_porch.typ;

	dss_vp_write(priv, hw_videoport, DSS_VP_TIMING_H,
		     FLD_VAL(hsw - 1, 7, 0) |
		     FLD_VAL(hfp - 1, 19, 8) | FLD_VAL(hbp - 1, 31, 20));

	dss_vp_write(priv, hw_videoport, DSS_VP_TIMING_V,
		     FLD_VAL(vsw - 1, 7, 0) |
		     FLD_VAL(vfp, 19, 8) | FLD_VAL(vbp, 31, 20));

	ivs = !!(timing->flags & (1 << 3));

	ihs = !!(timing->flags & (1 << 1));

	ieo = 0;

	ipc = 0;

	/* always use the 'rf' setting */
	onoff = true;

	rf = true;

	/* always use aligned syncs */
	align = true;

	/* always use DE_HIGH for OLDI */
	if (priv->feat->vp_bus_type[hw_videoport] == DSS_VP_OLDI)
		ieo = false;

	dss_vp_write(priv, hw_videoport, DSS_VP_POL_FREQ,
		     FLD_VAL(align, 18, 18) |
		     FLD_VAL(onoff, 17, 17) |
		     FLD_VAL(rf, 16, 16) |
		     FLD_VAL(ieo, 15, 15) |
		     FLD_VAL(ipc, 14, 14) |
		     FLD_VAL(ihs, 13, 13) |
		     FLD_VAL(ivs, 12, 12));

	dss_vp_write(priv, hw_videoport, DSS_VP_SIZE_SCREEN,
		     FLD_VAL(timing->hactive.typ - 1, 11, 0) |
		     FLD_VAL(timing->vactive.typ - 1, 27, 16));

	VP_REG_FLD_MOD(priv, hw_videoport, DSS_VP_CONTROL, 1, 0, 0);
}

enum c8_to_c12_mode { C8_TO_C12_REPLICATE, C8_TO_C12_MAX, C8_TO_C12_MIN };

static u16 c8_to_c12(u8 c8, enum c8_to_c12_mode mode)
{
	u16 c12;

	c12 = c8 << 4;

	switch (mode) {
	case C8_TO_C12_REPLICATE:
		/* Copy c8 4 MSB to 4 LSB for full scale c12 */
		c12 |= c8 >> 4;
		break;
	case C8_TO_C12_MAX:
		c12 |= 0xF;
		break;
	default:
	case C8_TO_C12_MIN:
		break;
	}

	return c12;
}

static u64 argb8888_to_argb12121212(u32 argb8888, enum c8_to_c12_mode m)
{
	u8 a, r, g, b;
	u64 v;

	a = (argb8888 >> 24) & 0xff;
	r = (argb8888 >> 16) & 0xff;
	g = (argb8888 >> 8) & 0xff;
	b = (argb8888 >> 0) & 0xff;

	v = ((u64)c8_to_c12(a, m) << 36) | ((u64)c8_to_c12(r, m) << 24) |
		((u64)c8_to_c12(g, m) << 12) | (u64)c8_to_c12(b, m);

	return v;
}

static void dss_vp_set_default_color(struct tidss_drv_priv *priv,
				     u32 hw_videoport, u32 default_color)
{
	u64 v;

	v = argb8888_to_argb12121212(default_color, C8_TO_C12_REPLICATE);
	dss_ovr_write(priv, hw_videoport,
		      DSS_OVR_DEFAULT_COLOR, v & 0xffffffff);
	dss_ovr_write(priv, hw_videoport,
		      DSS_OVR_DEFAULT_COLOR2, (v >> 32) & 0xffff);
}

int dss_vp_enable_clk(struct tidss_drv_priv *priv, u32 hw_videoport)
{
	int ret = clk_prepare_enable(&priv->vp_clk[hw_videoport]);

	if (ret)
		dev_dbg(priv->dev, "%s: enabling clk failed: %d\n", __func__,
			ret);

	return ret;
}

void dss_vp_prepare(struct tidss_drv_priv *priv, u32 hw_videoport)
{
	dss_vp_set_gamma(priv, hw_videoport, NULL, 0);
	dss_vp_set_default_color(priv, 0, 0);

	if (priv->feat->vp_bus_type[hw_videoport] == DSS_VP_OLDI) {
		dss_oldi_tx_power(priv, true);
		dss_enable_oldi(priv, hw_videoport);
	}
}

static
unsigned int dss_pclk_diff(unsigned long rate, unsigned long real_rate)
{
	int r = rate / 100, rr = real_rate / 100;

	return (unsigned int)(abs(((rr - r) * 100) / r));
}

int dss_vp_set_clk_rate(struct tidss_drv_priv *priv, u32 hw_videoport,
			unsigned long rate)
{
	int r;
	unsigned long new_rate;

	/*
	 * For AM625 OLDI video ports, the requested pixel clock needs to take into account the
	 * serial clock required for the serialization of DPI signals into LVDS signals. The
	 * incoming pixel clock on the OLDI video port gets divided by 7 whenever OLDI enable bit
	 * gets set.
	 */
	if (priv->feat->vp_bus_type[hw_videoport] == DSS_VP_OLDI &&
	    priv->feat->subrev == DSS_AM625)
		rate *= 7;

	r = clk_set_rate(&priv->vp_clk[hw_videoport], rate);

	new_rate = clk_get_rate(&priv->vp_clk[hw_videoport]);

	if (dss_pclk_diff(rate, new_rate) > 5)
		dev_warn(priv->dev,
			 "vp%d: Clock rate %lu differs over 5%% from requested %lu\n",
			 hw_videoport, new_rate, rate);

	dev_dbg(priv->dev, "vp%d: new rate %lu Hz (requested %lu Hz)\n",
		hw_videoport, clk_get_rate(&priv->vp_clk[hw_videoport]), rate);
	return 0;
}

static void dss_ovr_set_plane(struct tidss_drv_priv *priv,
			      u32 hw_plane, u32 hw_ovr,
			      u32 x, u32 y, u32 layer)
{
	OVR_REG_FLD_MOD(priv, hw_ovr, DSS_OVR_ATTRIBUTES(layer),
			0x1, 4, 1);
	OVR_REG_FLD_MOD(priv, hw_ovr, DSS_OVR_ATTRIBUTES(layer),
			x, 17, 6);
	OVR_REG_FLD_MOD(priv, hw_ovr, DSS_OVR_ATTRIBUTES(layer),
			y, 30, 19);
}

void dss_ovr_enable_layer(struct tidss_drv_priv *priv,
			  u32 hw_ovr, u32 layer, bool enable)
{
	OVR_REG_FLD_MOD(priv, hw_ovr, DSS_OVR_ATTRIBUTES(layer),
			!!enable, 0, 0);
}

static void dss_vid_csc_enable(struct tidss_drv_priv *priv, u32 hw_plane,
			       bool enable)
{
	VID_REG_FLD_MOD(priv, hw_plane, DSS_VID_ATTRIBUTES, !!enable, 9, 9);
}

int dss_plane_setup(struct tidss_drv_priv *priv, u32 hw_plane, u32 hw_videoport)
{
	VID_REG_FLD_MOD(priv, hw_plane, DSS_VID_ATTRIBUTES,
			priv->pixel_format, 6, 1);

	dss_vid_write(priv, hw_plane, DSS_VID_PICTURE_SIZE,
		      ((LCD_MAX_WIDTH - 1) | ((LCD_MAX_HEIGHT - 1) << 16)));

	dss_vid_csc_enable(priv, hw_plane, false);

	dss_vid_write(priv, hw_plane, DSS_VID_GLOBAL_ALPHA, 0xFF);

	VID_REG_FLD_MOD(priv, hw_plane, DSS_VID_ATTRIBUTES, 1, 28, 28);
	return 0;
}

int dss_plane_enable(struct tidss_drv_priv *priv, u32 hw_plane, bool enable)
{
	VID_REG_FLD_MOD(priv, hw_plane, DSS_VID_ATTRIBUTES, !!enable, 0, 0);

	return 0;
}

static u32 dss_vid_get_fifo_size(struct tidss_drv_priv *priv, u32 hw_plane)
{
	return VID_REG_GET(priv, hw_plane, DSS_VID_BUF_SIZE_STATUS, 15, 0);
}

static void dss_vid_set_mflag_threshold(struct tidss_drv_priv *priv,
					u32 hw_plane, u32 low, u32 high)
{
	dss_vid_write(priv, hw_plane, DSS_VID_MFLAG_THRESHOLD,
		      FLD_VAL(high, 31, 16) | FLD_VAL(low, 15, 0));
}

static
void dss_vid_set_buf_threshold(struct tidss_drv_priv *priv,
			       u32 hw_plane, u32 low, u32 high)
{
	dss_vid_write(priv, hw_plane, DSS_VID_BUF_THRESHOLD,
		      FLD_VAL(high, 31, 16) | FLD_VAL(low, 15, 0));
}

static void dss_plane_init(struct tidss_drv_priv *priv)
{
	unsigned int hw_plane;
	u32 cba_lo_pri = 1;
	u32 cba_hi_pri = 0;

	REG_FLD_MOD(priv, DSS_CBA_CFG, cba_lo_pri, 2, 0);
	REG_FLD_MOD(priv, DSS_CBA_CFG, cba_hi_pri, 5, 3);

	/* MFLAG_CTRL = ENABLED */
	REG_FLD_MOD(priv, DSS_GLOBAL_MFLAG_ATTRIBUTE, 2, 1, 0);
	/* MFLAG_START = MFLAGNORMALSTARTMODE */
	REG_FLD_MOD(priv, DSS_GLOBAL_MFLAG_ATTRIBUTE, 0, 6, 6);

	for (hw_plane = 0; hw_plane < priv->feat->num_planes; hw_plane++) {
		u32 size = dss_vid_get_fifo_size(priv, hw_plane);
		u32 thr_low, thr_high;
		u32 mflag_low, mflag_high;
		u32 preload;

		thr_high = size - 1;
		thr_low = size / 2;

		mflag_high = size * 2 / 3;
		mflag_low = size / 3;

		preload = thr_low;

		dev_dbg(priv->dev,
			"%s: bufsize %u, buf_threshold %u/%u, mflag threshold %u/%u preload %u\n",
			priv->feat->vid_name[hw_plane],
			size,
			thr_high, thr_low,
			mflag_high, mflag_low,
			preload);

		dss_vid_set_buf_threshold(priv, hw_plane,
					  thr_low, thr_high);
		dss_vid_set_mflag_threshold(priv, hw_plane,
					    mflag_low, mflag_high);

		dss_vid_write(priv, hw_plane, DSS_VID_PRELOAD, preload);

		/* Prefech up to PRELOAD value */
		VID_REG_FLD_MOD(priv, hw_plane, DSS_VID_ATTRIBUTES, 0,
				19, 19);
	}
}

static void dss_vp_init(struct tidss_drv_priv *priv)
{
	unsigned int i;

	/* Enable the gamma Shadow bit-field for all VPs*/
	for (i = 0; i < priv->feat->num_vps; i++)
		VP_REG_FLD_MOD(priv, i, DSS_VP_CONFIG, 1, 2, 2);
}

static int dss_init_am65x_oldi_io_ctrl(struct udevice *dev,
				       struct tidss_drv_priv *priv)
{
	struct udevice *syscon;
	struct regmap *regmap;
	int ret = 0;

	ret = uclass_get_device_by_phandle(UCLASS_SYSCON, dev, "ti,am65x-oldi-io-ctrl",
					   &syscon);
	if (ret) {
		debug("unable to find ti,am65x-oldi-io-ctrl syscon device (%d)\n", ret);
		return ret;
	}

	/* get grf-reg base address */
	regmap = syscon_get_regmap(syscon);
	if (!regmap) {
		debug("unable to find rockchip grf regmap\n");
		return -ENODEV;
	}
	priv->oldi_io_ctrl = regmap;
	return 0;
}

static int tidss_drv_probe(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct tidss_drv_priv *priv = dev_get_priv(dev);
	struct udevice *panel = NULL;
	struct display_timing timings;
	unsigned int i;
	int ret = 0;
	const char *mode;

	priv->dev = dev;

	priv->feat = &dss_am625_feats;

    /*
     * set your plane format based on your bmp image
     * Supported 24bpp and 32bpp bmpimages
     */

	priv->pixel_format = DSS_FORMAT_XRGB8888;

	dss_common_regmap = priv->feat->common_regs;

	ret = uclass_first_device_err(UCLASS_PANEL, &panel);
	if (ret) {
		if (ret != -ENODEV)
			dev_err(dev, "panel device error %d\n", ret);
		return ret;
	}

	ret = panel_get_display_timing(panel, &timings);
	if (ret) {
		ret = ofnode_decode_panel_timing(dev_ofnode(panel),
						 &timings);
		if (ret) {
			dev_err(dev, "decode display timing error %d\n", ret);
			return ret;
		}
	}

	mode = ofnode_read_string(dev_ofnode(panel), "data-mapping");
	if (!mode) {
		debug("%s: Could not read mode property\n", dev->name);
		return -EINVAL;
	}

	uc_priv->bpix = VIDEO_BPP32;

	if (!strcmp(mode, "vesa-24"))
		priv->bus_format = &dss_bus_formats[7];
	else
		priv->bus_format = &dss_bus_formats[8];

	/* Common address */
	priv->base_common = dev_remap_addr_name(dev, priv->feat->common);
	if (!priv->base_common)
		return -EINVAL;

	/* plane address setup and enable */
	for (i = 0; i < priv->feat->num_planes; i++) {
		priv->base_vid[i] = dev_remap_addr_name(dev, priv->feat->vid_name[i]);
		if (!priv->base_vid[i])
			return -EINVAL;
	}

	dss_vid_write(priv, 0, DSS_VID_BA_0, uc_plat->base & 0xffffffff);
	dss_vid_write(priv, 0, DSS_VID_BA_EXT_0, (u64)uc_plat->base >> 32);
	dss_vid_write(priv, 0, DSS_VID_BA_1, uc_plat->base & 0xffffffff);
	dss_vid_write(priv, 0, DSS_VID_BA_EXT_1, (u64)uc_plat->base >> 32);

	ret = dss_plane_setup(priv, 0, 0);
	if (ret) {
		dss_plane_enable(priv, 0, false);
			return ret;
	}

	dss_plane_enable(priv, 0, true);
	dss_plane_init(priv);

	/* video port address clocks and enable */
	for (i = 0; i < priv->feat->num_vps; i++) {
		priv->base_ovr[i] = dev_remap_addr_name(dev, priv->feat->ovr_name[i]);
		priv->base_vp[i] = dev_remap_addr_name(dev, priv->feat->vp_name[i]);
	}

	ret = clk_get_by_name(dev, "vp1", &priv->vp_clk[0]);
	if (ret) {
		dev_err(dev, "video port %d clock enable error %d\n", i, ret);
		return ret;
	}

	dss_ovr_set_plane(priv, 1, 0, 0, 0, 0);
	dss_ovr_enable_layer(priv, 0, 0, true);

	/* Video Port cloks */
	dss_vp_enable_clk(priv, 0);

	dss_vp_set_clk_rate(priv, 0, timings.pixelclock.typ * 1000);

	priv->oldi_mode = OLDI_MODE_OFF;
	uc_priv->xsize = timings.hactive.typ;
	uc_priv->ysize = timings.vactive.typ;
	if (priv->feat->subrev == DSS_AM65X || priv->feat->subrev == DSS_AM625) {
		priv->oldi_mode = OLDI_DUAL_LINK;
		if (priv->oldi_mode) {
			ret = dss_init_am65x_oldi_io_ctrl(dev, priv);
			if (ret)
				return ret;
		}
	}

	dss_vp_prepare(priv, 0);
	dss_vp_enable(priv, 0, &timings);
	dss_vp_init(priv);

	ret = clk_get_by_name(dev, "fck", &priv->fclk);
	if (ret) {
		dev_err(dev, "peripheral clock get error %d\n", ret);
		return ret;
	}

	ret = clk_enable(&priv->fclk);
	if (ret) {
		dev_err(dev, "peripheral clock enable error %d\n", ret);
		return ret;
	}

	if (IS_ERR(&priv->fclk)) {
		dev_err(dev, "%s: Failed to get fclk: %ld\n",
			__func__, PTR_ERR(&priv->fclk));
		return PTR_ERR(&priv->fclk);
	}

	dev_dbg(dev, "DSS fclk %lu Hz\n", clk_get_rate(&priv->fclk));

	video_set_flush_dcache(dev, true);
	return 0;
}

static int tidss_drv_remove(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(VIDEO_REMOVE)) {
		struct tidss_drv_priv *priv = dev_get_priv(dev);

		VP_REG_FLD_MOD(priv, 0, DSS_VP_CONTROL, 0, 0, 0);
	}
	return 0;
}

static int tidss_drv_bind(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	uc_plat->size = ((LCD_MAX_WIDTH * LCD_MAX_HEIGHT *
			  (1 << LCD_MAX_LOG2_BPP)) >> 3) + 0x20;
	return 0;
}

static const struct udevice_id tidss_drv_ids[] = {
	{ .compatible = "ti,am625-dss" },
	{ }
};

U_BOOT_DRIVER(tidss_drv) = {
	.name = "tidss_drv",
	.id = UCLASS_VIDEO,
	.of_match = tidss_drv_ids,
	.bind = tidss_drv_bind,
	.probe = tidss_drv_probe,
	.remove = tidss_drv_remove,
	.priv_auto = sizeof(struct tidss_drv_priv),
#if CONFIG_IS_ENABLED(VIDEO_REMOVE)
	.flags = DM_FLAG_OS_PREPARE,
#else
	.flags = DM_FLAG_OS_PREPARE | DM_FLAG_LEAVE_PD_ON,
#endif
};
