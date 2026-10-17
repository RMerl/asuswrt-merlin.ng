// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#include <config.h>
#include <common.h>
#include <errno.h>
#include <log.h>

#include <asm/arch/nexell.h>
#include <asm/arch/tieoff.h>
#include <asm/arch/reset.h>
#include <asm/arch/display.h>

#include <linux/delay.h>

#include "soc/s5pxx18_soc_dpc.h"
#include "soc/s5pxx18_soc_hdmi.h"
#include "soc/s5pxx18_soc_disptop.h"
#include "soc/s5pxx18_soc_disptop_clk.h"

#define	__io_address(a)	(void *)(uintptr_t)(a)

static const u8 hdmiphy_preset74_25[32] = {
	0xd1, 0x1f, 0x10, 0x40, 0x40, 0xf8, 0xc8, 0x81,
	0xe8, 0xba, 0xd8, 0x45, 0xa0, 0xac, 0x80, 0x0a,
	0x80, 0x09, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
	0xa5, 0x24, 0x01, 0x00, 0x00, 0x01, 0x10, 0x80,
};

static const u8 hdmiphy_preset148_5[32] = {
	0xd1, 0x1f, 0x00, 0x40, 0x40, 0xf8, 0xc8, 0x81,
	0xe8, 0xba, 0xd8, 0x45, 0xa0, 0xac, 0x80, 0x0a,
	0x80, 0x09, 0x84, 0x05, 0x22, 0x24, 0x86, 0x54,
	0x4b, 0x25, 0x03, 0x00, 0x00, 0x01, 0x80,
};

#define HDMIPHY_PRESET_TABLE_SIZE   (32)

enum NXP_HDMI_PRESET {
	NXP_HDMI_PRESET_720P = 0,	/* 1280 x 720 */
	NXP_HDMI_PRESET_1080P,	/* 1920 x 1080 */
	NXP_HDMI_PRESET_MAX
};

static void hdmi_reset(void)
{
	nx_rstcon_setrst(RESET_ID_HDMI_VIDEO, RSTCON_ASSERT);
	nx_rstcon_setrst(RESET_ID_HDMI_SPDIF, RSTCON_ASSERT);
	nx_rstcon_setrst(RESET_ID_HDMI_TMDS, RSTCON_ASSERT);
	nx_rstcon_setrst(RESET_ID_HDMI_VIDEO, RSTCON_NEGATE);
	nx_rstcon_setrst(RESET_ID_HDMI_SPDIF, RSTCON_NEGATE);
	nx_rstcon_setrst(RESET_ID_HDMI_TMDS, RSTCON_NEGATE);
}

static int hdmi_phy_enable(int preset, int enable)
{
	const u8 *table = NULL;
	int size = 0;
	u32 addr, i = 0;

	if (!enable)
		return 0;

	switch (preset) {
	case NXP_HDMI_PRESET_720P:
		table = hdmiphy_preset74_25;
		size = 32;
		break;
	case NXP_HDMI_PRESET_1080P:
		table = hdmiphy_preset148_5;
		size = 31;
		break;
	default:
		printf("hdmi: phy not support preset %d\n", preset);
		return -EINVAL;
	}

	nx_hdmi_set_reg(0, HDMI_PHY_REG7C, (0 << 7));
	nx_hdmi_set_reg(0, HDMI_PHY_REG7C, (0 << 7));
	nx_hdmi_set_reg(0, HDMI_PHY_REG04, (0 << 4));
	nx_hdmi_set_reg(0, HDMI_PHY_REG04, (0 << 4));
	nx_hdmi_set_reg(0, HDMI_PHY_REG24, (1 << 7));
	nx_hdmi_set_reg(0, HDMI_PHY_REG24, (1 << 7));

	for (i = 0, addr = HDMI_PHY_REG04; size > i; i++, addr += 4) {
		nx_hdmi_set_reg(0, addr, table[i]);
		nx_hdmi_set_reg(0, addr, table[i]);
	}

	nx_hdmi_set_reg(0, HDMI_PHY_REG7C, 0x80);
	nx_hdmi_set_reg(0, HDMI_PHY_REG7C, 0x80);
	nx_hdmi_set_reg(0, HDMI_PHY_REG7C, (1 << 7));
	nx_hdmi_set_reg(0, HDMI_PHY_REG7C, (1 << 7));
	debug("%s: preset = %d\n", __func__, preset);

	return 0;
}

static inline int hdmi_wait_phy_ready(void)
{
	int count = 500;

	do {
		u32 val = nx_hdmi_get_reg(0, HDMI_LINK_PHY_STATUS_0);

		if (val & 0x01) {
			printf("HDMI:  phy ready...\n");
			return 1;
		}
		mdelay(10);
	} while (count--);

	return 0;
}

static inline int hdmi_get_vsync(int preset,
				 struct dp_sync_info *sync,
				 struct dp_ctrl_info *ctrl)
{
	switch (preset) {
	case NXP_HDMI_PRESET_720P:	/* 720p: 1280x720 */
		sync->h_active_len = 1280;
		sync->h_sync_width = 40;
		sync->h_back_porch = 220;
		sync->h_front_porch = 110;
		sync->h_sync_invert = 0;
		sync->v_active_len = 720;
		sync->v_sync_width = 5;
		sync->v_back_porch = 20;
		sync->v_front_porch = 5;
		sync->v_sync_invert = 0;
		break;

	case NXP_HDMI_PRESET_1080P:	/* 1080p: 1920x1080 */
		sync->h_active_len = 1920;
		sync->h_sync_width = 44;
		sync->h_back_porch = 148;
		sync->h_front_porch = 88;
		sync->h_sync_invert = 0;
		sync->v_active_len = 1080;
		sync->v_sync_width = 5;
		sync->v_back_porch = 36;
		sync->v_front_porch = 4;
		sync->v_sync_invert = 0;
		break;
	default:
		printf("HDMI: not support preset sync %d\n", preset);
		return -EINVAL;
	}

	ctrl->clk_src_lv0 = 4;
	ctrl->clk_div_lv0 = 1;
	ctrl->clk_src_lv1 = 7;
	ctrl->clk_div_lv1 = 1;

	ctrl->out_format = outputformat_rgb888;
	ctrl->delay_mask = (DP_SYNC_DELAY_RGB_PVD | DP_SYNC_DELAY_HSYNC_CP1 |
			    DP_SYNC_DELAY_VSYNC_FRAM | DP_SYNC_DELAY_DE_CP);
	ctrl->d_rgb_pvd = 0;
	ctrl->d_hsync_cp1 = 0;
	ctrl->d_vsync_fram = 0;
	ctrl->d_de_cp2 = 7;

	/* HFP + HSW + HBP + AVWidth-VSCLRPIXEL- 1; */
	ctrl->vs_start_offset = (sync->h_front_porch + sync->h_sync_width +
				 sync->h_back_porch + sync->h_active_len - 1);
	ctrl->vs_end_offset = 0;

	/* HFP + HSW + HBP + AVWidth-EVENVSCLRPIXEL- 1 */
	ctrl->ev_start_offset = (sync->h_front_porch + sync->h_sync_width +
				 sync->h_back_porch + sync->h_active_len - 1);
	ctrl->ev_end_offset = 0;
	debug("%s: preset: %d\n", __func__, preset);

	return 0;
}

static void hdmi_clock(void)
{
	void *base =
	    __io_address(nx_disp_top_clkgen_get_physical_address
			 (to_mipi_clkgen));

	nx_disp_top_clkgen_set_base_address(to_mipi_clkgen, base);
	nx_disp_top_clkgen_set_clock_divisor_enable(to_mipi_clkgen, 0);
	nx_disp_top_clkgen_set_clock_pclk_mode(to_mipi_clkgen,
					       nx_pclkmode_always);
	nx_disp_top_clkgen_set_clock_source(to_mipi_clkgen, HDMI_SPDIF_CLKOUT,
					    2);
	nx_disp_top_clkgen_set_clock_divisor(to_mipi_clkgen, HDMI_SPDIF_CLKOUT,
					     2);
	nx_disp_top_clkgen_set_clock_source(to_mipi_clkgen, 1, 7);
	nx_disp_top_clkgen_set_clock_divisor_enable(to_mipi_clkgen, 1);

	/* must initialize this !!! */
	nx_disp_top_hdmi_set_vsync_hsstart_end(0, 0);
	nx_disp_top_hdmi_set_vsync_start(0);
	nx_disp_top_hdmi_set_hactive_start(0);
	nx_disp_top_hdmi_set_hactive_end(0);
}

static void hdmi_vsync(struct dp_sync_info *sync)
{
	int width = sync->h_active_len;
	int hsw = sync->h_sync_width;
	int hbp = sync->h_back_porch;
	int height = sync->v_active_len;
	int vsw = sync->v_sync_width;
	int vbp = sync->v_back_porch;

	int v_sync_s = vsw + vbp + height - 1;
	int h_active_s = hsw + hbp;
	int h_active_e = width + hsw + hbp;
	int v_sync_hs_se0 = hsw + hbp + 1;
	int v_sync_hs_se1 = hsw + hbp + 2;

	nx_disp_top_hdmi_set_vsync_start(v_sync_s);
	nx_disp_top_hdmi_set_hactive_start(h_active_s);
	nx_disp_top_hdmi_set_hactive_end(h_active_e);
	nx_disp_top_hdmi_set_vsync_hsstart_end(v_sync_hs_se0, v_sync_hs_se1);
}

static int hdmi_prepare(struct dp_sync_info *sync)
{
	int width = sync->h_active_len;
	int hsw = sync->h_sync_width;
	int hfp = sync->h_front_porch;
	int hbp = sync->h_back_porch;
	int height = sync->v_active_len;
	int vsw = sync->v_sync_width;
	int vfp = sync->v_front_porch;
	int vbp = sync->v_back_porch;

	u32 h_blank, h_line, h_sync_start, h_sync_end;
	u32 v_blank, v2_blank, v_line;
	u32 v_sync_line_bef_1, v_sync_line_bef_2;

	u32 fixed_ffff = 0xffff;

	/* calculate sync variables */
	h_blank = hfp + hsw + hbp;
	v_blank = vfp + vsw + vbp;
	v2_blank = height + vfp + vsw + vbp;
	v_line = height + vfp + vsw + vbp;	/* total v */
	h_line = width + hfp + hsw + hbp;	/* total h */
	h_sync_start = hfp;
	h_sync_end = hfp + hsw;
	v_sync_line_bef_1 = vfp;
	v_sync_line_bef_2 = vfp + vsw;

	/* no blue screen mode, encoding order as it is */
	nx_hdmi_set_reg(0, HDMI_LINK_HDMI_CON_0, (0 << 5) | (1 << 4));

	/* set HDMI_LINK_BLUE_SCREEN_* to 0x0 */
	nx_hdmi_set_reg(0, HDMI_LINK_BLUE_SCREEN_R_0, 0x5555);
	nx_hdmi_set_reg(0, HDMI_LINK_BLUE_SCREEN_R_1, 0x5555);
	nx_hdmi_set_reg(0, HDMI_LINK_BLUE_SCREEN_G_0, 0x5555);
	nx_hdmi_set_reg(0, HDMI_LINK_BLUE_SCREEN_G_1, 0x5555);
	nx_hdmi_set_reg(0, HDMI_LINK_BLUE_SCREEN_B_0, 0x5555);
	nx_hdmi_set_reg(0, HDMI_LINK_BLUE_SCREEN_B_1, 0x5555);

	/* set HDMI_CON_1 to 0x0 */
	nx_hdmi_set_reg(0, HDMI_LINK_HDMI_CON_1, 0x0);
	nx_hdmi_set_reg(0, HDMI_LINK_HDMI_CON_2, 0x0);

	/* set interrupt : enable hpd_plug, hpd_unplug */
	nx_hdmi_set_reg(0, HDMI_LINK_INTC_CON_0,
			(1 << 6) | (1 << 3) | (1 << 2));

	/* set STATUS_EN to 0x17 */
	nx_hdmi_set_reg(0, HDMI_LINK_STATUS_EN, 0x17);

	/* TODO set HDP to 0x0 : later check hpd */
	nx_hdmi_set_reg(0, HDMI_LINK_HPD, 0x0);

	/* set MODE_SEL to 0x02 */
	nx_hdmi_set_reg(0, HDMI_LINK_MODE_SEL, 0x2);

	/* set H_BLANK_*, V1_BLANK_*, V2_BLANK_*, V_LINE_*,
	 * H_LINE_*, H_SYNC_START_*, H_SYNC_END_ *
	 * V_SYNC_LINE_BEF_1_*, V_SYNC_LINE_BEF_2_*
	 */
	nx_hdmi_set_reg(0, HDMI_LINK_H_BLANK_0, h_blank % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_H_BLANK_1, h_blank >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V1_BLANK_0, v_blank % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V1_BLANK_1, v_blank >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V2_BLANK_0, v2_blank % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V2_BLANK_1, v2_blank >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_LINE_0, v_line % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_LINE_1, v_line >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_H_LINE_0, h_line % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_H_LINE_1, h_line >> 8);

	if (width == 1280) {
		nx_hdmi_set_reg(0, HDMI_LINK_HSYNC_POL, 0x1);
		nx_hdmi_set_reg(0, HDMI_LINK_VSYNC_POL, 0x1);
	} else {
		nx_hdmi_set_reg(0, HDMI_LINK_HSYNC_POL, 0x0);
		nx_hdmi_set_reg(0, HDMI_LINK_VSYNC_POL, 0x0);
	}

	nx_hdmi_set_reg(0, HDMI_LINK_INT_PRO_MODE, 0x0);

	nx_hdmi_set_reg(0, HDMI_LINK_H_SYNC_START_0, (h_sync_start % 256) - 2);
	nx_hdmi_set_reg(0, HDMI_LINK_H_SYNC_START_1, h_sync_start >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_H_SYNC_END_0, (h_sync_end % 256) - 2);
	nx_hdmi_set_reg(0, HDMI_LINK_H_SYNC_END_1, h_sync_end >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_BEF_1_0,
			v_sync_line_bef_1 % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_BEF_1_1,
			v_sync_line_bef_1 >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_BEF_2_0,
			v_sync_line_bef_2 % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_BEF_2_1,
			v_sync_line_bef_2 >> 8);

	/* Set V_SYNC_LINE_AFT*, V_SYNC_LINE_AFT_PXL*, VACT_SPACE* */
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_1_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_1_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_2_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_2_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_3_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_3_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_4_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_4_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_5_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_5_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_6_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_6_1, fixed_ffff >> 8);

	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_1_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_2_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_3_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_4_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_5_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_V_SYNC_LINE_AFT_PXL_6_1, fixed_ffff >> 8);

	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE1_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE1_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE2_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE2_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE3_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE3_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE4_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE4_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE5_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE5_1, fixed_ffff >> 8);
	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE6_0, fixed_ffff % 256);
	nx_hdmi_set_reg(0, HDMI_LINK_VACT_SPACE6_1, fixed_ffff >> 8);

	nx_hdmi_set_reg(0, HDMI_LINK_CSC_MUX, 0x0);
	nx_hdmi_set_reg(0, HDMI_LINK_SYNC_GEN_MUX, 0x0);

	nx_hdmi_set_reg(0, HDMI_LINK_SEND_START_0, 0xfd);
	nx_hdmi_set_reg(0, HDMI_LINK_SEND_START_1, 0x01);
	nx_hdmi_set_reg(0, HDMI_LINK_SEND_END_0, 0x0d);
	nx_hdmi_set_reg(0, HDMI_LINK_SEND_END_1, 0x3a);
	nx_hdmi_set_reg(0, HDMI_LINK_SEND_END_2, 0x08);

	/* Set DC_CONTROL to 0x00 */
	nx_hdmi_set_reg(0, HDMI_LINK_DC_CONTROL, 0x0);

	if (IS_ENABLED(CONFIG_HDMI_PATTERN))
		nx_hdmi_set_reg(0, HDMI_LINK_VIDEO_PATTERN_GEN, 0x1);
	else
		nx_hdmi_set_reg(0, HDMI_LINK_VIDEO_PATTERN_GEN, 0x0);

	nx_hdmi_set_reg(0, HDMI_LINK_GCP_CON, 0x0a);
	return 0;
}

static void hdmi_init(void)
{
	void *base;
   /**
    * [SEQ 2] set the HDMI CLKGEN's PCLKMODE to always enabled
    */
	base =
	    __io_address(nx_disp_top_clkgen_get_physical_address(hdmi_clkgen));
	nx_disp_top_clkgen_set_base_address(hdmi_clkgen, base);
	nx_disp_top_clkgen_set_clock_pclk_mode(hdmi_clkgen, nx_pclkmode_always);

	base = __io_address(nx_hdmi_get_physical_address(0));
	nx_hdmi_set_base_address(0, base);

    /**
     * [SEQ 3] set the 0xC001100C[0] to 1
     */
	nx_tieoff_set(NX_TIEOFF_DISPLAYTOP0_i_HDMI_PHY_REFCLK_SEL, 1);

    /**
     * [SEQ 4] release the resets of HDMI.i_PHY_nRST and HDMI.i_nRST
     */
	nx_rstcon_setrst(RESET_ID_HDMI_PHY, RSTCON_ASSERT);
	nx_rstcon_setrst(RESET_ID_HDMI, RSTCON_ASSERT);
	nx_rstcon_setrst(RESET_ID_HDMI_PHY, RSTCON_NEGATE);
	nx_rstcon_setrst(RESET_ID_HDMI, RSTCON_NEGATE);
}

void hdmi_enable(int input, int preset, struct dp_sync_info *sync, int enable)
{
	if (enable) {
		nx_hdmi_set_reg(0, HDMI_LINK_HDMI_CON_0,
				(nx_hdmi_get_reg(0, HDMI_LINK_HDMI_CON_0) |
				 0x1));
		hdmi_vsync(sync);
	} else {
		hdmi_phy_enable(preset, 0);
	}
}

static int hdmi_setup(int input, int preset,
		      struct dp_sync_info *sync, struct dp_ctrl_info *ctrl)
{
	u32 HDMI_SEL = 0;
	int ret;

	switch (input) {
	case DP_DEVICE_DP0:
		HDMI_SEL = primary_mlc;
		break;
	case DP_DEVICE_DP1:
		HDMI_SEL = secondary_mlc;
		break;
	case DP_DEVICE_RESCONV:
		HDMI_SEL = resolution_conv;
		break;
	default:
		printf("HDMI: not support source device %d\n", input);
		return -EINVAL;
	}

	/**
	 * [SEQ 5] set up the HDMI PHY to specific video clock.
	 */
	ret = hdmi_phy_enable(preset, 1);
	if (ret < 0)
		return ret;

	/**
	 * [SEQ 6] I2S (or SPDIFTX) configuration for the source audio data
	 * this is done in another user app  - ex> Android Audio HAL
	 */

	/**
	 * [SEQ 7] Wait for ECID ready
	 */

	/**
	 * [SEQ 8] release the resets of HDMI.i_VIDEO_nRST and HDMI.i_SPDIF_nRST
	 * and HDMI.i_TMDS_nRST
	 */
	hdmi_reset();

	/**
	 * [SEQ 9] Wait for HDMI PHY ready (wait until 0xC0200020.[0], 1)
	 */
	if (hdmi_wait_phy_ready() == 0) {
		printf("%s: failed to wait for hdmiphy ready\n", __func__);
		hdmi_phy_enable(preset, 0);
		return -EIO;
	}
	/* set mux */
	nx_disp_top_set_hdmimux(1, HDMI_SEL);

	/**
	 * [SEC 10] Set the DPC CLKGEN's Source Clock to HDMI_CLK &
	 * Set Sync Parameter
	 */
	hdmi_clock();
	/* set hdmi link clk to clkgen  vs default is hdmi phy clk */

	/**
	 * [SEQ 11] Set up the HDMI Converter parameters
	 */
	hdmi_get_vsync(preset, sync, ctrl);
	hdmi_prepare(sync);

	return 0;
}

void nx_hdmi_display(int module,
		     struct dp_sync_info *sync, struct dp_ctrl_info *ctrl,
		     struct dp_plane_top *top, struct dp_plane_info *planes,
		     struct dp_hdmi_dev *dev)
{
	struct dp_plane_info *plane = planes;
	int input = module == 0 ? DP_DEVICE_DP0 : DP_DEVICE_DP1;
	int count = top->plane_num;
	int preset = dev->preset;
	int i = 0;

	debug("HDMI:  display.%d\n", module);

	switch (preset) {
	case 0:
		top->screen_width = 1280;
		top->screen_height = 720;
		sync->h_active_len = 1280;
		sync->v_active_len = 720;
		break;
	case 1:
		top->screen_width = 1920;
		top->screen_height = 1080;
		sync->h_active_len = 1920;
		sync->v_active_len = 1080;
		break;
	default:
		printf("hdmi not support preset %d\n", preset);
		return;
	}

	printf("HDMI:  display.%d, preset %d (%4d * %4d)\n",
	       module, preset, top->screen_width, top->screen_height);

	dp_control_init(module);
	dp_plane_init(module);

	hdmi_init();
	hdmi_setup(input, preset, sync, ctrl);

	dp_plane_screen_setup(module, top);
	for (i = 0; count > i; i++, plane++) {
		if (!plane->enable)
			continue;
		dp_plane_layer_setup(module, plane);
		dp_plane_layer_enable(module, plane, 1);
	}
	dp_plane_screen_enable(module, 1);

	dp_control_setup(module, sync, ctrl);
	dp_control_enable(module, 1);

	hdmi_enable(input, preset, sync, 1);
}
