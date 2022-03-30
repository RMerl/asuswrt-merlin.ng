// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 * Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <media_bus_format.h>
#include "dw_hdmi.h"

struct tmds_n_cts {
	u32 tmds;
	u32 cts;
	u32 n;
};

static const struct tmds_n_cts n_cts_table[] = {
	{
		.tmds = 25175000, .n = 6144, .cts = 25175,
	}, {
		.tmds = 25200000, .n = 6144, .cts = 25200,
	}, {
		.tmds = 27000000, .n = 6144, .cts = 27000,
	}, {
		.tmds = 27027000, .n = 6144, .cts = 27027,
	}, {
		.tmds = 40000000, .n = 6144, .cts = 40000,
	}, {
		.tmds = 54000000, .n = 6144, .cts = 54000,
	}, {
		.tmds = 54054000, .n = 6144, .cts = 54054,
	}, {
		.tmds = 65000000, .n = 6144, .cts = 65000,
	}, {
		.tmds = 74176000, .n = 11648, .cts = 140625,
	}, {
		.tmds = 74250000, .n = 6144, .cts = 74250,
	}, {
		.tmds = 83500000, .n = 6144, .cts = 83500,
	}, {
		.tmds = 106500000, .n = 6144, .cts = 106500,
	}, {
		.tmds = 108000000, .n = 6144, .cts = 108000,
	}, {
		.tmds = 148352000, .n = 5824, .cts = 140625,
	}, {
		.tmds = 148500000, .n = 6144, .cts = 148500,
	}, {
		.tmds = 297000000, .n = 5120, .cts = 247500,
	}
};

static const u16 csc_coeff_default[3][4] = {
	{ 0x2000, 0x0000, 0x0000, 0x0000 },
	{ 0x0000, 0x2000, 0x0000, 0x0000 },
	{ 0x0000, 0x0000, 0x2000, 0x0000 }
};

static const u16 csc_coeff_rgb_in_eitu601[3][4] = {
	{ 0x2591, 0x1322, 0x074b, 0x0000 },
	{ 0x6535, 0x2000, 0x7acc, 0x0200 },
	{ 0x6acd, 0x7534, 0x2000, 0x0200 }
};

static const u16 csc_coeff_rgb_out_eitu601[3][4] = {
	{ 0x2000, 0x6926, 0x74fd, 0x010e },
	{ 0x2000, 0x2cdd, 0x0000, 0x7e9a },
	{ 0x2000, 0x0000, 0x38b4, 0x7e3b }
};

static void dw_hdmi_write(struct dw_hdmi *hdmi, u8 val, int offset)
{
	switch (hdmi->reg_io_width) {
	case 1:
		writeb(val, hdmi->ioaddr + offset);
		break;
	case 4:
		writel(val, hdmi->ioaddr + (offset << 2));
		break;
	default:
		debug("reg_io_width has unsupported width!\n");
		break;
	}
}

static u8 dw_hdmi_read(struct dw_hdmi *hdmi, int offset)
{
	switch (hdmi->reg_io_width) {
	case 1:
		return readb(hdmi->ioaddr + offset);
	case 4:
		return readl(hdmi->ioaddr + (offset << 2));
	default:
		debug("reg_io_width has unsupported width!\n");
		break;
	}

	return 0;
}

static u8 (*hdmi_read)(struct dw_hdmi *hdmi, int offset) = dw_hdmi_read;
static void (*hdmi_write)(struct dw_hdmi *hdmi, u8 val, int offset) =
								 dw_hdmi_write;

static void hdmi_mod(struct dw_hdmi *hdmi, unsigned reg, u8 mask, u8 data)
{
	u8 val = hdmi_read(hdmi, reg) & ~mask;

	val |= data & mask;
	hdmi_write(hdmi, val, reg);
}

static void hdmi_set_clock_regenerator(struct dw_hdmi *hdmi, u32 n, u32 cts)
{
	uint cts3;
	uint n3;

	/* first set ncts_atomic_write (if present) */
	n3 = HDMI_AUD_N3_NCTS_ATOMIC_WRITE;
	hdmi_write(hdmi, n3, HDMI_AUD_N3);

	/* set cts_manual (if present) */
	cts3 = HDMI_AUD_CTS3_CTS_MANUAL;

	cts3 |= HDMI_AUD_CTS3_N_SHIFT_1 << HDMI_AUD_CTS3_N_SHIFT_OFFSET;
	cts3 |= (cts >> 16) & HDMI_AUD_CTS3_AUDCTS19_16_MASK;

	/* write cts values; cts3 must be written first */
	hdmi_write(hdmi, cts3, HDMI_AUD_CTS3);
	hdmi_write(hdmi, (cts >> 8) & 0xff, HDMI_AUD_CTS2);
	hdmi_write(hdmi, cts & 0xff, HDMI_AUD_CTS1);

	/* write n values; n1 must be written last */
	n3 |= (n >> 16) & HDMI_AUD_N3_AUDN19_16_MASK;
	hdmi_write(hdmi, n3, HDMI_AUD_N3);
	hdmi_write(hdmi, (n >> 8) & 0xff, HDMI_AUD_N2);
	hdmi_write(hdmi, n & 0xff, HDMI_AUD_N3);

	hdmi_write(hdmi, HDMI_AUD_INPUTCLKFS_128, HDMI_AUD_INPUTCLKFS);
}

static int hdmi_lookup_n_cts(u32 pixel_clk)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(n_cts_table); i++)
		if (pixel_clk <= n_cts_table[i].tmds)
			break;

	if (i >= ARRAY_SIZE(n_cts_table))
		return -1;

	return i;
}

static void hdmi_audio_set_samplerate(struct dw_hdmi *hdmi, u32 pixel_clk)
{
	u32 clk_n, clk_cts;
	int index;

	index = hdmi_lookup_n_cts(pixel_clk);
	if (index == -1) {
		debug("audio not supported for pixel clk %d\n", pixel_clk);
		return;
	}

	clk_n = n_cts_table[index].n;
	clk_cts = n_cts_table[index].cts;
	hdmi_set_clock_regenerator(hdmi, clk_n, clk_cts);
}

/*
 * this submodule is responsible for the video data synchronization.
 * for example, for rgb 4:4:4 input, the data map is defined as
 *			pin{47~40} <==> r[7:0]
 *			pin{31~24} <==> g[7:0]
 *			pin{15~8}  <==> b[7:0]
 */
static void hdmi_video_sample(struct dw_hdmi *hdmi)
{
	u32 color_format;
	uint val;

	switch (hdmi->hdmi_data.enc_in_bus_format) {
	case MEDIA_BUS_FMT_RGB888_1X24:
		color_format = 0x01;
		break;
	case MEDIA_BUS_FMT_RGB101010_1X30:
		color_format = 0x03;
		break;
	case MEDIA_BUS_FMT_RGB121212_1X36:
		color_format = 0x05;
		break;
	case MEDIA_BUS_FMT_RGB161616_1X48:
		color_format = 0x07;
		break;
	case MEDIA_BUS_FMT_YUV8_1X24:
	case MEDIA_BUS_FMT_UYYVYY8_0_5X24:
		color_format = 0x09;
		break;
	case MEDIA_BUS_FMT_YUV10_1X30:
	case MEDIA_BUS_FMT_UYYVYY10_0_5X30:
		color_format = 0x0B;
		break;
	case MEDIA_BUS_FMT_YUV12_1X36:
	case MEDIA_BUS_FMT_UYYVYY12_0_5X36:
		color_format = 0x0D;
		break;
	case MEDIA_BUS_FMT_YUV16_1X48:
	case MEDIA_BUS_FMT_UYYVYY16_0_5X48:
		color_format = 0x0F;
		break;
	case MEDIA_BUS_FMT_UYVY8_1X16:
		color_format = 0x16;
		break;
	case MEDIA_BUS_FMT_UYVY10_1X20:
		color_format = 0x14;
		break;
	case MEDIA_BUS_FMT_UYVY12_1X24:
		color_format = 0x12;
		break;
	default:
		color_format = 0x01;
		break;
	}

	val = HDMI_TX_INVID0_INTERNAL_DE_GENERATOR_DISABLE |
	      ((color_format << HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET) &
	      HDMI_TX_INVID0_VIDEO_MAPPING_MASK);

	hdmi_write(hdmi, val, HDMI_TX_INVID0);

	/* enable tx stuffing: when de is inactive, fix the output data to 0 */
	val = HDMI_TX_INSTUFFING_BDBDATA_STUFFING_ENABLE |
	      HDMI_TX_INSTUFFING_RCRDATA_STUFFING_ENABLE |
	      HDMI_TX_INSTUFFING_GYDATA_STUFFING_ENABLE;
	hdmi_write(hdmi, val, HDMI_TX_INSTUFFING);
	hdmi_write(hdmi, 0x0, HDMI_TX_GYDATA0);
	hdmi_write(hdmi, 0x0, HDMI_TX_GYDATA1);
	hdmi_write(hdmi, 0x0, HDMI_TX_RCRDATA0);
	hdmi_write(hdmi, 0x0, HDMI_TX_RCRDATA1);
	hdmi_write(hdmi, 0x0, HDMI_TX_BCBDATA0);
	hdmi_write(hdmi, 0x0, HDMI_TX_BCBDATA1);
}

static void hdmi_video_packetize(struct dw_hdmi *hdmi)
{
	u32 output_select = HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS;
	u32 remap_size = HDMI_VP_REMAP_YCC422_16BIT;
	u32 color_depth = 0;
	uint val, vp_conf;

	/* set the packetizer registers */
	val = ((color_depth << HDMI_VP_PR_CD_COLOR_DEPTH_OFFSET) &
		HDMI_VP_PR_CD_COLOR_DEPTH_MASK) |
		((0 << HDMI_VP_PR_CD_DESIRED_PR_FACTOR_OFFSET) &
		HDMI_VP_PR_CD_DESIRED_PR_FACTOR_MASK);
	hdmi_write(hdmi, val, HDMI_VP_PR_CD);

	hdmi_mod(hdmi, HDMI_VP_STUFF, HDMI_VP_STUFF_PR_STUFFING_MASK,
		 HDMI_VP_STUFF_PR_STUFFING_STUFFING_MODE);

	/* data from pixel repeater block */
	vp_conf = HDMI_VP_CONF_PR_EN_DISABLE |
		  HDMI_VP_CONF_BYPASS_SELECT_VID_PACKETIZER;

	hdmi_mod(hdmi, HDMI_VP_CONF, HDMI_VP_CONF_PR_EN_MASK |
		 HDMI_VP_CONF_BYPASS_SELECT_MASK, vp_conf);

	hdmi_mod(hdmi, HDMI_VP_STUFF, HDMI_VP_STUFF_IDEFAULT_PHASE_MASK,
		 1 << HDMI_VP_STUFF_IDEFAULT_PHASE_OFFSET);

	hdmi_write(hdmi, remap_size, HDMI_VP_REMAP);

	vp_conf = HDMI_VP_CONF_BYPASS_EN_ENABLE |
		  HDMI_VP_CONF_PP_EN_DISABLE |
		  HDMI_VP_CONF_YCC422_EN_DISABLE;

	hdmi_mod(hdmi, HDMI_VP_CONF, HDMI_VP_CONF_BYPASS_EN_MASK |
		 HDMI_VP_CONF_PP_EN_ENMASK | HDMI_VP_CONF_YCC422_EN_MASK,
		 vp_conf);

	hdmi_mod(hdmi, HDMI_VP_STUFF, HDMI_VP_STUFF_PP_STUFFING_MASK |
		 HDMI_VP_STUFF_YCC422_STUFFING_MASK,
		 HDMI_VP_STUFF_PP_STUFFING_STUFFING_MODE |
		 HDMI_VP_STUFF_YCC422_STUFFING_STUFFING_MODE);

	hdmi_mod(hdmi, HDMI_VP_CONF, HDMI_VP_CONF_OUTPUT_SELECTOR_MASK,
		 output_select);
}

static inline void hdmi_phy_test_clear(struct dw_hdmi *hdmi, uint bit)
{
	hdmi_mod(hdmi, HDMI_PHY_TST0, HDMI_PHY_TST0_TSTCLR_MASK,
		 bit << HDMI_PHY_TST0_TSTCLR_OFFSET);
}

static int hdmi_phy_wait_i2c_done(struct dw_hdmi *hdmi, u32 msec)
{
	ulong start;
	u32 val;

	start = get_timer(0);
	do {
		val = hdmi_read(hdmi, HDMI_IH_I2CMPHY_STAT0);
		if (val & 0x3) {
			hdmi_write(hdmi, val, HDMI_IH_I2CMPHY_STAT0);
			return 0;
		}

		udelay(100);
	} while (get_timer(start) < msec);

	return 1;
}

static void hdmi_phy_i2c_write(struct dw_hdmi *hdmi, uint data, uint addr)
{
	hdmi_write(hdmi, 0xff, HDMI_IH_I2CMPHY_STAT0);
	hdmi_write(hdmi, addr, HDMI_PHY_I2CM_ADDRESS_ADDR);
	hdmi_write(hdmi, (u8)(data >> 8), HDMI_PHY_I2CM_DATAO_1_ADDR);
	hdmi_write(hdmi, (u8)(data >> 0), HDMI_PHY_I2CM_DATAO_0_ADDR);
	hdmi_write(hdmi, HDMI_PHY_I2CM_OPERATION_ADDR_WRITE,
		   HDMI_PHY_I2CM_OPERATION_ADDR);

	hdmi_phy_wait_i2c_done(hdmi, 1000);
}

static void hdmi_phy_enable_power(struct dw_hdmi *hdmi, uint enable)
{
	hdmi_mod(hdmi, HDMI_PHY_CONF0, HDMI_PHY_CONF0_PDZ_MASK,
		 enable << HDMI_PHY_CONF0_PDZ_OFFSET);
}

static void hdmi_phy_enable_tmds(struct dw_hdmi *hdmi, uint enable)
{
	hdmi_mod(hdmi, HDMI_PHY_CONF0, HDMI_PHY_CONF0_ENTMDS_MASK,
		 enable << HDMI_PHY_CONF0_ENTMDS_OFFSET);
}

static void hdmi_phy_enable_spare(struct dw_hdmi *hdmi, uint enable)
{
	hdmi_mod(hdmi, HDMI_PHY_CONF0, HDMI_PHY_CONF0_SPARECTRL_MASK,
		 enable << HDMI_PHY_CONF0_SPARECTRL_OFFSET);
}

static void hdmi_phy_gen2_pddq(struct dw_hdmi *hdmi, uint enable)
{
	hdmi_mod(hdmi, HDMI_PHY_CONF0, HDMI_PHY_CONF0_GEN2_PDDQ_MASK,
		 enable << HDMI_PHY_CONF0_GEN2_PDDQ_OFFSET);
}

static void hdmi_phy_gen2_txpwron(struct dw_hdmi *hdmi, uint enable)
{
	hdmi_mod(hdmi, HDMI_PHY_CONF0,
		 HDMI_PHY_CONF0_GEN2_TXPWRON_MASK,
		 enable << HDMI_PHY_CONF0_GEN2_TXPWRON_OFFSET);
}

static void hdmi_phy_sel_data_en_pol(struct dw_hdmi *hdmi, uint enable)
{
	hdmi_mod(hdmi, HDMI_PHY_CONF0,
		 HDMI_PHY_CONF0_SELDATAENPOL_MASK,
		 enable << HDMI_PHY_CONF0_SELDATAENPOL_OFFSET);
}

static void hdmi_phy_sel_interface_control(struct dw_hdmi *hdmi,
					   uint enable)
{
	hdmi_mod(hdmi, HDMI_PHY_CONF0, HDMI_PHY_CONF0_SELDIPIF_MASK,
		 enable << HDMI_PHY_CONF0_SELDIPIF_OFFSET);
}

static int hdmi_phy_configure(struct dw_hdmi *hdmi, u32 mpixelclock)
{
	ulong start;
	uint i, val;

	if (!hdmi->mpll_cfg || !hdmi->phy_cfg)
		return -1;

	/* gen2 tx power off */
	hdmi_phy_gen2_txpwron(hdmi, 0);

	/* gen2 pddq */
	hdmi_phy_gen2_pddq(hdmi, 1);

	/* phy reset */
	hdmi_write(hdmi, HDMI_MC_PHYRSTZ_DEASSERT, HDMI_MC_PHYRSTZ);
	hdmi_write(hdmi, HDMI_MC_PHYRSTZ_ASSERT, HDMI_MC_PHYRSTZ);
	hdmi_write(hdmi, HDMI_MC_HEACPHY_RST_ASSERT, HDMI_MC_HEACPHY_RST);

	hdmi_phy_test_clear(hdmi, 1);
	hdmi_write(hdmi, HDMI_PHY_I2CM_SLAVE_ADDR_PHY_GEN2,
		   HDMI_PHY_I2CM_SLAVE_ADDR);
	hdmi_phy_test_clear(hdmi, 0);

	/* pll/mpll cfg - always match on final entry */
	for (i = 0; hdmi->mpll_cfg[i].mpixelclock != (~0ul); i++)
		if (mpixelclock <= hdmi->mpll_cfg[i].mpixelclock)
			break;

	hdmi_phy_i2c_write(hdmi, hdmi->mpll_cfg[i].cpce, PHY_OPMODE_PLLCFG);
	hdmi_phy_i2c_write(hdmi, hdmi->mpll_cfg[i].gmp, PHY_PLLGMPCTRL);
	hdmi_phy_i2c_write(hdmi, hdmi->mpll_cfg[i].curr, PHY_PLLCURRCTRL);

	hdmi_phy_i2c_write(hdmi, 0x0000, PHY_PLLPHBYCTRL);
	hdmi_phy_i2c_write(hdmi, 0x0006, PHY_PLLCLKBISTPHASE);

	for (i = 0; hdmi->phy_cfg[i].mpixelclock != (~0ul); i++)
		if (mpixelclock <= hdmi->phy_cfg[i].mpixelclock)
			break;

	/*
	 * resistance term 133ohm cfg
	 * preemp cgf 0.00
	 * tx/ck lvl 10
	 */
	hdmi_phy_i2c_write(hdmi, hdmi->phy_cfg[i].term, PHY_TXTERM);
	hdmi_phy_i2c_write(hdmi, hdmi->phy_cfg[i].sym_ctr, PHY_CKSYMTXCTRL);
	hdmi_phy_i2c_write(hdmi, hdmi->phy_cfg[i].vlev_ctr, PHY_VLEVCTRL);

	/* remove clk term */
	hdmi_phy_i2c_write(hdmi, 0x8000, PHY_CKCALCTRL);

	hdmi_phy_enable_power(hdmi, 1);

	/* toggle tmds enable */
	hdmi_phy_enable_tmds(hdmi, 0);
	hdmi_phy_enable_tmds(hdmi, 1);

	/* gen2 tx power on */
	hdmi_phy_gen2_txpwron(hdmi, 1);
	hdmi_phy_gen2_pddq(hdmi, 0);

	hdmi_phy_enable_spare(hdmi, 1);

	/* wait for phy pll lock */
	start = get_timer(0);
	do {
		val = hdmi_read(hdmi, HDMI_PHY_STAT0);
		if (!(val & HDMI_PHY_TX_PHY_LOCK))
			return 0;

		udelay(100);
	} while (get_timer(start) < 5);

	return -1;
}

static void hdmi_av_composer(struct dw_hdmi *hdmi,
			     const struct display_timing *edid)
{
	bool mdataenablepolarity = true;
	uint inv_val;
	uint hbl;
	uint vbl;

	hbl = edid->hback_porch.typ + edid->hfront_porch.typ +
			edid->hsync_len.typ;
	vbl = edid->vback_porch.typ + edid->vfront_porch.typ +
			edid->vsync_len.typ;

	/* set up hdmi_fc_invidconf */
	inv_val = HDMI_FC_INVIDCONF_HDCP_KEEPOUT_INACTIVE;

	inv_val |= (edid->flags & DISPLAY_FLAGS_VSYNC_HIGH ?
		   HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_HIGH :
		   HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_LOW);

	inv_val |= (edid->flags & DISPLAY_FLAGS_HSYNC_HIGH ?
		   HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_HIGH :
		   HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_LOW);

	inv_val |= (mdataenablepolarity ?
		   HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_HIGH :
		   HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_LOW);

	inv_val |= (edid->hdmi_monitor ?
		   HDMI_FC_INVIDCONF_DVI_MODEZ_HDMI_MODE :
		   HDMI_FC_INVIDCONF_DVI_MODEZ_DVI_MODE);

	inv_val |= HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_LOW;

	inv_val |= HDMI_FC_INVIDCONF_IN_I_P_PROGRESSIVE;

	hdmi_write(hdmi, inv_val, HDMI_FC_INVIDCONF);

	/* set up horizontal active pixel width */
	hdmi_write(hdmi, edid->hactive.typ >> 8, HDMI_FC_INHACTV1);
	hdmi_write(hdmi, edid->hactive.typ, HDMI_FC_INHACTV0);

	/* set up vertical active lines */
	hdmi_write(hdmi, edid->vactive.typ >> 8, HDMI_FC_INVACTV1);
	hdmi_write(hdmi, edid->vactive.typ, HDMI_FC_INVACTV0);

	/* set up horizontal blanking pixel region width */
	hdmi_write(hdmi, hbl >> 8, HDMI_FC_INHBLANK1);
	hdmi_write(hdmi, hbl, HDMI_FC_INHBLANK0);

	/* set up vertical blanking pixel region width */
	hdmi_write(hdmi, vbl, HDMI_FC_INVBLANK);

	/* set up hsync active edge delay width (in pixel clks) */
	hdmi_write(hdmi, edid->hfront_porch.typ >> 8, HDMI_FC_HSYNCINDELAY1);
	hdmi_write(hdmi, edid->hfront_porch.typ, HDMI_FC_HSYNCINDELAY0);

	/* set up vsync active edge delay (in lines) */
	hdmi_write(hdmi, edid->vfront_porch.typ, HDMI_FC_VSYNCINDELAY);

	/* set up hsync active pulse width (in pixel clks) */
	hdmi_write(hdmi, edid->hsync_len.typ >> 8, HDMI_FC_HSYNCINWIDTH1);
	hdmi_write(hdmi, edid->hsync_len.typ, HDMI_FC_HSYNCINWIDTH0);

	/* set up vsync active edge delay (in lines) */
	hdmi_write(hdmi, edid->vsync_len.typ, HDMI_FC_VSYNCINWIDTH);
}

static bool hdmi_bus_fmt_is_rgb(unsigned int bus_format)
{
	switch (bus_format) {
	case MEDIA_BUS_FMT_RGB888_1X24:
	case MEDIA_BUS_FMT_RGB101010_1X30:
	case MEDIA_BUS_FMT_RGB121212_1X36:
	case MEDIA_BUS_FMT_RGB161616_1X48:
		return true;

	default:
		return false;
	}
}

static bool hdmi_bus_fmt_is_yuv444(unsigned int bus_format)
{
	switch (bus_format) {
	case MEDIA_BUS_FMT_YUV8_1X24:
	case MEDIA_BUS_FMT_YUV10_1X30:
	case MEDIA_BUS_FMT_YUV12_1X36:
	case MEDIA_BUS_FMT_YUV16_1X48:
		return true;

	default:
		return false;
	}
}

static bool hdmi_bus_fmt_is_yuv422(unsigned int bus_format)
{
	switch (bus_format) {
	case MEDIA_BUS_FMT_UYVY8_1X16:
	case MEDIA_BUS_FMT_UYVY10_1X20:
	case MEDIA_BUS_FMT_UYVY12_1X24:
		return true;

	default:
		return false;
	}
}

static int is_color_space_interpolation(struct dw_hdmi *hdmi)
{
	if (!hdmi_bus_fmt_is_yuv422(hdmi->hdmi_data.enc_in_bus_format))
		return 0;

	if (hdmi_bus_fmt_is_rgb(hdmi->hdmi_data.enc_out_bus_format) ||
	    hdmi_bus_fmt_is_yuv444(hdmi->hdmi_data.enc_out_bus_format))
		return 1;

	return 0;
}

static int is_color_space_decimation(struct dw_hdmi *hdmi)
{
	if (!hdmi_bus_fmt_is_yuv422(hdmi->hdmi_data.enc_out_bus_format))
		return 0;

	if (hdmi_bus_fmt_is_rgb(hdmi->hdmi_data.enc_in_bus_format) ||
	    hdmi_bus_fmt_is_yuv444(hdmi->hdmi_data.enc_in_bus_format))
		return 1;

	return 0;
}

static int hdmi_bus_fmt_color_depth(unsigned int bus_format)
{
	switch (bus_format) {
	case MEDIA_BUS_FMT_RGB888_1X24:
	case MEDIA_BUS_FMT_YUV8_1X24:
	case MEDIA_BUS_FMT_UYVY8_1X16:
	case MEDIA_BUS_FMT_UYYVYY8_0_5X24:
		return 8;

	case MEDIA_BUS_FMT_RGB101010_1X30:
	case MEDIA_BUS_FMT_YUV10_1X30:
	case MEDIA_BUS_FMT_UYVY10_1X20:
	case MEDIA_BUS_FMT_UYYVYY10_0_5X30:
		return 10;

	case MEDIA_BUS_FMT_RGB121212_1X36:
	case MEDIA_BUS_FMT_YUV12_1X36:
	case MEDIA_BUS_FMT_UYVY12_1X24:
	case MEDIA_BUS_FMT_UYYVYY12_0_5X36:
		return 12;

	case MEDIA_BUS_FMT_RGB161616_1X48:
	case MEDIA_BUS_FMT_YUV16_1X48:
	case MEDIA_BUS_FMT_UYYVYY16_0_5X48:
		return 16;

	default:
		return 0;
	}
}

static int is_color_space_conversion(struct dw_hdmi *hdmi)
{
	return hdmi->hdmi_data.enc_in_bus_format !=
	       hdmi->hdmi_data.enc_out_bus_format;
}

static void dw_hdmi_update_csc_coeffs(struct dw_hdmi *hdmi)
{
	const u16 (*csc_coeff)[3][4] = &csc_coeff_default;
	unsigned int i;
	u32 csc_scale = 1;

	if (is_color_space_conversion(hdmi)) {
		if (hdmi_bus_fmt_is_rgb(hdmi->hdmi_data.enc_out_bus_format)) {
			csc_coeff = &csc_coeff_rgb_out_eitu601;
		} else if (hdmi_bus_fmt_is_rgb(
					hdmi->hdmi_data.enc_in_bus_format)) {
			csc_coeff = &csc_coeff_rgb_in_eitu601;
			csc_scale = 0;
		}
	}

	/* The CSC registers are sequential, alternating MSB then LSB */
	for (i = 0; i < ARRAY_SIZE(csc_coeff_default[0]); i++) {
		u16 coeff_a = (*csc_coeff)[0][i];
		u16 coeff_b = (*csc_coeff)[1][i];
		u16 coeff_c = (*csc_coeff)[2][i];

		hdmi_write(hdmi, coeff_a & 0xff, HDMI_CSC_COEF_A1_LSB + i * 2);
		hdmi_write(hdmi, coeff_a >> 8, HDMI_CSC_COEF_A1_MSB + i * 2);
		hdmi_write(hdmi, coeff_b & 0xff, HDMI_CSC_COEF_B1_LSB + i * 2);
		hdmi_write(hdmi, coeff_b >> 8, HDMI_CSC_COEF_B1_MSB + i * 2);
		hdmi_write(hdmi, coeff_c & 0xff, HDMI_CSC_COEF_C1_LSB + i * 2);
		hdmi_write(hdmi, coeff_c >> 8, HDMI_CSC_COEF_C1_MSB + i * 2);
	}

	hdmi_mod(hdmi, HDMI_CSC_SCALE, HDMI_CSC_SCALE_CSCSCALE_MASK, csc_scale);
}

static void hdmi_video_csc(struct dw_hdmi *hdmi)
{
	int color_depth = 0;
	int interpolation = HDMI_CSC_CFG_INTMODE_DISABLE;
	int decimation = 0;

	/* YCC422 interpolation to 444 mode */
	if (is_color_space_interpolation(hdmi))
		interpolation = HDMI_CSC_CFG_INTMODE_CHROMA_INT_FORMULA1;
	else if (is_color_space_decimation(hdmi))
		decimation = HDMI_CSC_CFG_DECMODE_CHROMA_INT_FORMULA3;

	switch (hdmi_bus_fmt_color_depth(hdmi->hdmi_data.enc_out_bus_format)) {
	case 8:
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_24BPP;
		break;
	case 10:
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_30BPP;
		break;
	case 12:
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_36BPP;
		break;
	case 16:
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_48BPP;
		break;

	default:
		return;
	}

	/* Configure the CSC registers */
	hdmi_write(hdmi, interpolation | decimation, HDMI_CSC_CFG);

	hdmi_mod(hdmi, HDMI_CSC_SCALE, HDMI_CSC_SCALE_CSC_COLORDE_PTH_MASK,
		 color_depth);

	dw_hdmi_update_csc_coeffs(hdmi);
}

/* hdmi initialization step b.4 */
static void hdmi_enable_video_path(struct dw_hdmi *hdmi, bool audio)
{
	uint clkdis;

	/* control period minimum duration */
	hdmi_write(hdmi, 12, HDMI_FC_CTRLDUR);
	hdmi_write(hdmi, 32, HDMI_FC_EXCTRLDUR);
	hdmi_write(hdmi, 1, HDMI_FC_EXCTRLSPAC);

	/* set to fill tmds data channels */
	hdmi_write(hdmi, 0x0b, HDMI_FC_CH0PREAM);
	hdmi_write(hdmi, 0x16, HDMI_FC_CH1PREAM);
	hdmi_write(hdmi, 0x21, HDMI_FC_CH2PREAM);

	hdmi_write(hdmi, HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_BYPASS,
		   HDMI_MC_FLOWCTRL);

	/* enable pixel clock and tmds data path */
	clkdis = 0x7f;
	clkdis &= ~HDMI_MC_CLKDIS_PIXELCLK_DISABLE;
	hdmi_write(hdmi, clkdis, HDMI_MC_CLKDIS);

	clkdis &= ~HDMI_MC_CLKDIS_TMDSCLK_DISABLE;
	hdmi_write(hdmi, clkdis, HDMI_MC_CLKDIS);

	/* Enable csc path */
	if (is_color_space_conversion(hdmi)) {
		clkdis &= ~HDMI_MC_CLKDIS_CSCCLK_DISABLE;
		hdmi_write(hdmi, clkdis, HDMI_MC_CLKDIS);
	}

	/* Enable color space conversion if needed */
	if (is_color_space_conversion(hdmi))
		hdmi_write(hdmi, HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_IN_PATH,
			   HDMI_MC_FLOWCTRL);
	else
		hdmi_write(hdmi, HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_BYPASS,
			   HDMI_MC_FLOWCTRL);

	if (audio) {
		clkdis &= ~HDMI_MC_CLKDIS_AUDCLK_DISABLE;
		hdmi_write(hdmi, clkdis, HDMI_MC_CLKDIS);
	}
}

/* workaround to clear the overflow condition */
static void hdmi_clear_overflow(struct dw_hdmi *hdmi)
{
	uint val, count;

	/* tmds software reset */
	hdmi_write(hdmi, (u8)~HDMI_MC_SWRSTZ_TMDSSWRST_REQ, HDMI_MC_SWRSTZ);

	val = hdmi_read(hdmi, HDMI_FC_INVIDCONF);

	for (count = 0; count < 4; count++)
		hdmi_write(hdmi, val, HDMI_FC_INVIDCONF);
}

static void hdmi_audio_set_format(struct dw_hdmi *hdmi)
{
	hdmi_write(hdmi, HDMI_AUD_CONF0_I2S_SELECT | HDMI_AUD_CONF0_I2S_IN_EN_0,
		   HDMI_AUD_CONF0);


	hdmi_write(hdmi, HDMI_AUD_CONF1_I2S_MODE_STANDARD_MODE |
		   HDMI_AUD_CONF1_I2S_WIDTH_16BIT, HDMI_AUD_CONF1);

	hdmi_write(hdmi, 0x00, HDMI_AUD_CONF2);
}

static void hdmi_audio_fifo_reset(struct dw_hdmi *hdmi)
{
	hdmi_write(hdmi, (u8)~HDMI_MC_SWRSTZ_II2SSWRST_REQ, HDMI_MC_SWRSTZ);
	hdmi_write(hdmi, HDMI_AUD_CONF0_SW_AUDIO_FIFO_RST, HDMI_AUD_CONF0);

	hdmi_write(hdmi, 0x00, HDMI_AUD_INT);
	hdmi_write(hdmi, 0x00, HDMI_AUD_INT1);
}

static int hdmi_get_plug_in_status(struct dw_hdmi *hdmi)
{
	uint val = hdmi_read(hdmi, HDMI_PHY_STAT0) & HDMI_PHY_HPD;

	return !!val;
}

static int hdmi_ddc_wait_i2c_done(struct dw_hdmi *hdmi, int msec)
{
	u32 val;
	ulong start;

	start = get_timer(0);
	do {
		val = hdmi_read(hdmi, HDMI_IH_I2CM_STAT0);
		if (val & 0x2) {
			hdmi_write(hdmi, val, HDMI_IH_I2CM_STAT0);
			return 0;
		}

		udelay(100);
	} while (get_timer(start) < msec);

	return 1;
}

static void hdmi_ddc_reset(struct dw_hdmi *hdmi)
{
	hdmi_mod(hdmi, HDMI_I2CM_SOFTRSTZ, HDMI_I2CM_SOFTRSTZ_MASK, 0);
}

static int hdmi_read_edid(struct dw_hdmi *hdmi, int block, u8 *buff)
{
	int shift = (block % 2) * 0x80;
	int edid_read_err = 0;
	u32 trytime = 5;
	u32 n;

	/* set ddc i2c clk which devided from ddc_clk to 100khz */
	hdmi_write(hdmi, hdmi->i2c_clk_high, HDMI_I2CM_SS_SCL_HCNT_0_ADDR);
	hdmi_write(hdmi, hdmi->i2c_clk_low, HDMI_I2CM_SS_SCL_LCNT_0_ADDR);
	hdmi_mod(hdmi, HDMI_I2CM_DIV, HDMI_I2CM_DIV_FAST_STD_MODE,
		 HDMI_I2CM_DIV_STD_MODE);

	hdmi_write(hdmi, HDMI_I2CM_SLAVE_DDC_ADDR, HDMI_I2CM_SLAVE);
	hdmi_write(hdmi, HDMI_I2CM_SEGADDR_DDC, HDMI_I2CM_SEGADDR);
	hdmi_write(hdmi, block >> 1, HDMI_I2CM_SEGPTR);

	while (trytime--) {
		edid_read_err = 0;

		for (n = 0; n < HDMI_EDID_BLOCK_SIZE; n++) {
			hdmi_write(hdmi, shift + n, HDMI_I2CM_ADDRESS);

			if (block == 0)
				hdmi_write(hdmi, HDMI_I2CM_OP_RD8,
					   HDMI_I2CM_OPERATION);
			else
				hdmi_write(hdmi, HDMI_I2CM_OP_RD8_EXT,
					   HDMI_I2CM_OPERATION);

			if (hdmi_ddc_wait_i2c_done(hdmi, 10)) {
				hdmi_ddc_reset(hdmi);
				edid_read_err = 1;
				break;
			}

			buff[n] = hdmi_read(hdmi, HDMI_I2CM_DATAI);
		}

		if (!edid_read_err)
			break;
	}

	return edid_read_err;
}

static const u8 pre_buf[] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
	0x04, 0x69, 0xfa, 0x23, 0xc8, 0x28, 0x01, 0x00,
	0x10, 0x17, 0x01, 0x03, 0x80, 0x33, 0x1d, 0x78,
	0x2a, 0xd9, 0x45, 0xa2, 0x55, 0x4d, 0xa0, 0x27,
	0x12, 0x50, 0x54, 0xb7, 0xef, 0x00, 0x71, 0x4f,
	0x81, 0x40, 0x81, 0x80, 0x95, 0x00, 0xb3, 0x00,
	0xd1, 0xc0, 0x81, 0xc0, 0x81, 0x00, 0x02, 0x3a,
	0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
	0x45, 0x00, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x1e,
	0x00, 0x00, 0x00, 0xff, 0x00, 0x44, 0x34, 0x4c,
	0x4d, 0x54, 0x46, 0x30, 0x37, 0x35, 0x39, 0x37,
	0x36, 0x0a, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x32,
	0x4b, 0x18, 0x53, 0x11, 0x00, 0x0a, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc,
	0x00, 0x41, 0x53, 0x55, 0x53, 0x20, 0x56, 0x53,
	0x32, 0x33, 0x38, 0x0a, 0x20, 0x20, 0x01, 0xb0,
	0x02, 0x03, 0x22, 0x71, 0x4f, 0x01, 0x02, 0x03,
	0x11, 0x12, 0x13, 0x04, 0x14, 0x05, 0x0e, 0x0f,
	0x1d, 0x1e, 0x1f, 0x10, 0x23, 0x09, 0x17, 0x07,
	0x83, 0x01, 0x00, 0x00, 0x65, 0x03, 0x0c, 0x00,
	0x10, 0x00, 0x8c, 0x0a, 0xd0, 0x8a, 0x20, 0xe0,
	0x2d, 0x10, 0x10, 0x3e, 0x96, 0x00, 0xfd, 0x1e,
	0x11, 0x00, 0x00, 0x18, 0x01, 0x1d, 0x00, 0x72,
	0x51, 0xd0, 0x1e, 0x20, 0x6e, 0x28, 0x55, 0x00,
	0xfd, 0x1e, 0x11, 0x00, 0x00, 0x1e, 0x01, 0x1d,
	0x00, 0xbc, 0x52, 0xd0, 0x1e, 0x20, 0xb8, 0x28,
	0x55, 0x40, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x1e,
	0x8c, 0x0a, 0xd0, 0x90, 0x20, 0x40, 0x31, 0x20,
	0x0c, 0x40, 0x55, 0x00, 0xfd, 0x1e, 0x11, 0x00,
	0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe9,
};

int dw_hdmi_phy_cfg(struct dw_hdmi *hdmi, uint mpixelclock)
{
	int i, ret;

	/* hdmi phy spec says to do the phy initialization sequence twice */
	for (i = 0; i < 2; i++) {
		hdmi_phy_sel_data_en_pol(hdmi, 1);
		hdmi_phy_sel_interface_control(hdmi, 0);
		hdmi_phy_enable_tmds(hdmi, 0);
		hdmi_phy_enable_power(hdmi, 0);

		ret = hdmi_phy_configure(hdmi, mpixelclock);
		if (ret) {
			debug("hdmi phy config failure %d\n", ret);
			return ret;
		}
	}

	return 0;
}

int dw_hdmi_phy_wait_for_hpd(struct dw_hdmi *hdmi)
{
	ulong start;

	start = get_timer(0);
	do {
		if (hdmi_get_plug_in_status(hdmi))
			return 0;
		udelay(100);
	} while (get_timer(start) < 300);

	return -1;
}

void dw_hdmi_phy_init(struct dw_hdmi *hdmi)
{
	/* enable phy i2cm done irq */
	hdmi_write(hdmi, HDMI_PHY_I2CM_INT_ADDR_DONE_POL,
		   HDMI_PHY_I2CM_INT_ADDR);

	/* enable phy i2cm nack & arbitration error irq */
	hdmi_write(hdmi, HDMI_PHY_I2CM_CTLINT_ADDR_NAC_POL |
		   HDMI_PHY_I2CM_CTLINT_ADDR_ARBITRATION_POL,
		   HDMI_PHY_I2CM_CTLINT_ADDR);

	/* enable cable hot plug irq */
	hdmi_write(hdmi, (u8)~HDMI_PHY_HPD, HDMI_PHY_MASK0);

	/* clear hotplug interrupts */
	hdmi_write(hdmi, HDMI_IH_PHY_STAT0_HPD, HDMI_IH_PHY_STAT0);
}

int dw_hdmi_read_edid(struct dw_hdmi *hdmi, u8 *buf, int buf_size)
{
	u32 edid_size = HDMI_EDID_BLOCK_SIZE;
	int ret;

	if (0) {
		edid_size = sizeof(pre_buf);
		memcpy(buf, pre_buf, edid_size);
	} else {
		ret = hdmi_read_edid(hdmi, 0, buf);
		if (ret) {
			debug("failed to read edid.\n");
			return -1;
		}

		if (buf[0x7e] != 0) {
			hdmi_read_edid(hdmi, 1, buf + HDMI_EDID_BLOCK_SIZE);
			edid_size += HDMI_EDID_BLOCK_SIZE;
		}
	}

	return edid_size;
}

int dw_hdmi_enable(struct dw_hdmi *hdmi, const struct display_timing *edid)
{
	int ret;

	debug("%s, mode info : clock %d hdis %d vdis %d\n",
	      edid->hdmi_monitor ? "hdmi" : "dvi",
	      edid->pixelclock.typ, edid->hactive.typ, edid->vactive.typ);

	hdmi_av_composer(hdmi, edid);

	ret = hdmi->phy_set(hdmi, edid->pixelclock.typ);
	if (ret)
		return ret;

	hdmi_enable_video_path(hdmi, edid->hdmi_monitor);

	if (edid->hdmi_monitor) {
		hdmi_audio_fifo_reset(hdmi);
		hdmi_audio_set_format(hdmi);
		hdmi_audio_set_samplerate(hdmi, edid->pixelclock.typ);
	}

	hdmi_video_packetize(hdmi);
	hdmi_video_csc(hdmi);
	hdmi_video_sample(hdmi);

	hdmi_clear_overflow(hdmi);

	return 0;
}

void dw_hdmi_init(struct dw_hdmi *hdmi)
{
	uint ih_mute;

	/*
	 * boot up defaults are:
	 * hdmi_ih_mute   = 0x03 (disabled)
	 * hdmi_ih_mute_* = 0x00 (enabled)
	 *
	 * disable top level interrupt bits in hdmi block
	 */
	ih_mute = /*hdmi_read(hdmi, HDMI_IH_MUTE) |*/
		  HDMI_IH_MUTE_MUTE_WAKEUP_INTERRUPT |
		  HDMI_IH_MUTE_MUTE_ALL_INTERRUPT;

	if (hdmi->write_reg)
		hdmi_write = hdmi->write_reg;

	if (hdmi->read_reg)
		hdmi_read = hdmi->read_reg;

	hdmi_write(hdmi, ih_mute, HDMI_IH_MUTE);

	/* enable i2c master done irq */
	hdmi_write(hdmi, ~0x04, HDMI_I2CM_INT);

	/* enable i2c client nack % arbitration error irq */
	hdmi_write(hdmi, ~0x44, HDMI_I2CM_CTLINT);
}
