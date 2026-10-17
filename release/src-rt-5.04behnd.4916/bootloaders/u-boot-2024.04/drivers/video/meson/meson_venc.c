// SPDX-License-Identifier: GPL-2.0
/*
 * Amlogic Meson Video Processing Unit driver
 *
 * Copyright (c) 2018 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <dm.h>
#include <edid.h>
#include <fdtdec.h>
#include <log.h>
#include <asm/io.h>
#include "meson_vpu.h"

enum {
	MESON_VENC_MODE_NONE = 0,
	MESON_VENC_MODE_CVBS_PAL,
	MESON_VENC_MODE_CVBS_NTSC,
	MESON_VENC_MODE_HDMI,
};

enum meson_venc_source {
	MESON_VENC_SOURCE_NONE = 0,
	MESON_VENC_SOURCE_ENCI = 1,
	MESON_VENC_SOURCE_ENCP = 2,
};

#define HHI_VDAC_CNTL0		0x2F4 /* 0xbd offset in data sheet */
#define HHI_VDAC_CNTL0_G12A	0x2EC /* 0xbb offset in data sheet */
#define HHI_VDAC_CNTL1		0x2F8 /* 0xbe offset in data sheet */
#define HHI_VDAC_CNTL1_G12A	0x2F0 /* 0xbc offset in data sheet */

struct meson_cvbs_enci_mode {
	unsigned int mode_tag;
	unsigned int hso_begin; /* HSO begin position */
	unsigned int hso_end; /* HSO end position */
	unsigned int vso_even; /* VSO even line */
	unsigned int vso_odd; /* VSO odd line */
	unsigned int macv_max_amp; /* Macrovision max amplitude */
	unsigned int video_prog_mode;
	unsigned int video_mode;
	unsigned int sch_adjust;
	unsigned int yc_delay;
	unsigned int pixel_start;
	unsigned int pixel_end;
	unsigned int top_field_line_start;
	unsigned int top_field_line_end;
	unsigned int bottom_field_line_start;
	unsigned int bottom_field_line_end;
	unsigned int video_saturation;
	unsigned int video_contrast;
	unsigned int video_brightness;
	unsigned int video_hue;
	unsigned int analog_sync_adj;
};

struct meson_cvbs_enci_mode meson_cvbs_enci_pal = {
	.mode_tag = MESON_VENC_MODE_CVBS_PAL,
	.hso_begin = 3,
	.hso_end = 129,
	.vso_even = 3,
	.vso_odd = 260,
	.macv_max_amp = 7,
	.video_prog_mode = 0xff,
	.video_mode = 0x13,
	.sch_adjust = 0x28,
	.yc_delay = 0x343,
	.pixel_start = 251,
	.pixel_end = 1691,
	.top_field_line_start = 22,
	.top_field_line_end = 310,
	.bottom_field_line_start = 23,
	.bottom_field_line_end = 311,
	.video_saturation = 9,
	.video_contrast = 0,
	.video_brightness = 0,
	.video_hue = 0,
	.analog_sync_adj = 0x8080,
};

struct meson_cvbs_enci_mode meson_cvbs_enci_ntsc = {
	.mode_tag = MESON_VENC_MODE_CVBS_NTSC,
	.hso_begin = 5,
	.hso_end = 129,
	.vso_even = 3,
	.vso_odd = 260,
	.macv_max_amp = 0xb,
	.video_prog_mode = 0xf0,
	.video_mode = 0x8,
	.sch_adjust = 0x20,
	.yc_delay = 0x333,
	.pixel_start = 227,
	.pixel_end = 1667,
	.top_field_line_start = 18,
	.top_field_line_end = 258,
	.bottom_field_line_start = 19,
	.bottom_field_line_end = 259,
	.video_saturation = 18,
	.video_contrast = 3,
	.video_brightness = 0,
	.video_hue = 0,
	.analog_sync_adj = 0x9c00,
};

union meson_hdmi_venc_mode {
	struct {
		unsigned int mode_tag;
		unsigned int hso_begin;
		unsigned int hso_end;
		unsigned int vso_even;
		unsigned int vso_odd;
		unsigned int macv_max_amp;
		unsigned int video_prog_mode;
		unsigned int video_mode;
		unsigned int sch_adjust;
		unsigned int yc_delay;
		unsigned int pixel_start;
		unsigned int pixel_end;
		unsigned int top_field_line_start;
		unsigned int top_field_line_end;
		unsigned int bottom_field_line_start;
		unsigned int bottom_field_line_end;
	} enci;
	struct {
		unsigned int dvi_settings;
		unsigned int video_mode;
		unsigned int video_mode_adv;
		unsigned int video_prog_mode;
		bool video_prog_mode_present;
		unsigned int video_sync_mode;
		bool video_sync_mode_present;
		unsigned int video_yc_dly;
		bool video_yc_dly_present;
		unsigned int video_rgb_ctrl;
		bool video_rgb_ctrl_present;
		unsigned int video_filt_ctrl;
		bool video_filt_ctrl_present;
		unsigned int video_ofld_voav_ofst;
		bool video_ofld_voav_ofst_present;
		unsigned int yfp1_htime;
		unsigned int yfp2_htime;
		unsigned int max_pxcnt;
		unsigned int hspuls_begin;
		unsigned int hspuls_end;
		unsigned int hspuls_switch;
		unsigned int vspuls_begin;
		unsigned int vspuls_end;
		unsigned int vspuls_bline;
		unsigned int vspuls_eline;
		unsigned int eqpuls_begin;
		bool eqpuls_begin_present;
		unsigned int eqpuls_end;
		bool eqpuls_end_present;
		unsigned int eqpuls_bline;
		bool eqpuls_bline_present;
		unsigned int eqpuls_eline;
		bool eqpuls_eline_present;
		unsigned int havon_begin;
		unsigned int havon_end;
		unsigned int vavon_bline;
		unsigned int vavon_eline;
		unsigned int hso_begin;
		unsigned int hso_end;
		unsigned int vso_begin;
		unsigned int vso_end;
		unsigned int vso_bline;
		unsigned int vso_eline;
		bool vso_eline_present;
		unsigned int sy_val;
		bool sy_val_present;
		unsigned int sy2_val;
		bool sy2_val_present;
		unsigned int max_lncnt;
	} encp;
};

union meson_hdmi_venc_mode meson_hdmi_enci_mode_480i = {
	.enci = {
		.hso_begin = 5,
		.hso_end = 129,
		.vso_even = 3,
		.vso_odd = 260,
		.macv_max_amp = 0xb,
		.video_prog_mode = 0xf0,
		.video_mode = 0x8,
		.sch_adjust = 0x20,
		.yc_delay = 0,
		.pixel_start = 227,
		.pixel_end = 1667,
		.top_field_line_start = 18,
		.top_field_line_end = 258,
		.bottom_field_line_start = 19,
		.bottom_field_line_end = 259,
	},
};

union meson_hdmi_venc_mode meson_hdmi_enci_mode_576i = {
	.enci = {
		.hso_begin = 3,
		.hso_end = 129,
		.vso_even = 3,
		.vso_odd = 260,
		.macv_max_amp = 0x7,
		.video_prog_mode = 0xff,
		.video_mode = 0x13,
		.sch_adjust = 0x28,
		.yc_delay = 0x333,
		.pixel_start = 251,
		.pixel_end = 1691,
		.top_field_line_start = 22,
		.top_field_line_end = 310,
		.bottom_field_line_start = 23,
		.bottom_field_line_end = 311,
	},
};

union meson_hdmi_venc_mode meson_hdmi_encp_mode_480p = {
	.encp = {
		.dvi_settings = 0x21,
		.video_mode = 0x4000,
		.video_mode_adv = 0x9,
		.video_prog_mode = 0,
		.video_prog_mode_present = true,
		.video_sync_mode = 7,
		.video_sync_mode_present = true,
		/* video_yc_dly */
		/* video_rgb_ctrl */
		.video_filt_ctrl = 0x2052,
		.video_filt_ctrl_present = true,
		/* video_ofld_voav_ofst */
		.yfp1_htime = 244,
		.yfp2_htime = 1630,
		.max_pxcnt = 1715,
		.hspuls_begin = 0x22,
		.hspuls_end = 0xa0,
		.hspuls_switch = 88,
		.vspuls_begin = 0,
		.vspuls_end = 1589,
		.vspuls_bline = 0,
		.vspuls_eline = 5,
		.havon_begin = 249,
		.havon_end = 1689,
		.vavon_bline = 42,
		.vavon_eline = 521,
		/* eqpuls_begin */
		/* eqpuls_end */
		/* eqpuls_bline */
		/* eqpuls_eline */
		.hso_begin = 3,
		.hso_end = 5,
		.vso_begin = 3,
		.vso_end = 5,
		.vso_bline = 0,
		/* vso_eline */
		.sy_val	= 8,
		.sy_val_present = true,
		.sy2_val = 0x1d8,
		.sy2_val_present = true,
		.max_lncnt = 524,
	},
};

union meson_hdmi_venc_mode meson_hdmi_encp_mode_576p = {
	.encp = {
		.dvi_settings = 0x21,
		.video_mode = 0x4000,
		.video_mode_adv = 0x9,
		.video_prog_mode = 0,
		.video_prog_mode_present = true,
		.video_sync_mode = 7,
		.video_sync_mode_present = true,
		/* video_yc_dly */
		/* video_rgb_ctrl */
		.video_filt_ctrl = 0x52,
		.video_filt_ctrl_present = true,
		/* video_ofld_voav_ofst */
		.yfp1_htime = 235,
		.yfp2_htime = 1674,
		.max_pxcnt = 1727,
		.hspuls_begin = 0,
		.hspuls_end = 0x80,
		.hspuls_switch = 88,
		.vspuls_begin = 0,
		.vspuls_end = 1599,
		.vspuls_bline = 0,
		.vspuls_eline = 4,
		.havon_begin = 235,
		.havon_end = 1674,
		.vavon_bline = 44,
		.vavon_eline = 619,
		/* eqpuls_begin */
		/* eqpuls_end */
		/* eqpuls_bline */
		/* eqpuls_eline */
		.hso_begin = 0x80,
		.hso_end = 0,
		.vso_begin = 0,
		.vso_end = 5,
		.vso_bline = 0,
		/* vso_eline */
		.sy_val	= 8,
		.sy_val_present = true,
		.sy2_val = 0x1d8,
		.sy2_val_present = true,
		.max_lncnt = 624,
	},
};

union meson_hdmi_venc_mode meson_hdmi_encp_mode_720p60 = {
	.encp = {
		.dvi_settings = 0x2029,
		.video_mode = 0x4040,
		.video_mode_adv = 0x19,
		/* video_prog_mode */
		/* video_sync_mode */
		/* video_yc_dly */
		/* video_rgb_ctrl */
		/* video_filt_ctrl */
		/* video_ofld_voav_ofst */
		.yfp1_htime = 648,
		.yfp2_htime = 3207,
		.max_pxcnt = 3299,
		.hspuls_begin = 80,
		.hspuls_end = 240,
		.hspuls_switch = 80,
		.vspuls_begin = 688,
		.vspuls_end = 3248,
		.vspuls_bline = 4,
		.vspuls_eline = 8,
		.havon_begin = 648,
		.havon_end = 3207,
		.vavon_bline = 29,
		.vavon_eline = 748,
		/* eqpuls_begin */
		/* eqpuls_end */
		/* eqpuls_bline */
		/* eqpuls_eline */
		.hso_begin = 256,
		.hso_end = 168,
		.vso_begin = 168,
		.vso_end = 256,
		.vso_bline = 0,
		.vso_eline = 5,
		.vso_eline_present = true,
		/* sy_val */
		/* sy2_val */
		.max_lncnt = 749,
	},
};

union meson_hdmi_venc_mode meson_hdmi_encp_mode_720p50 = {
	.encp = {
		.dvi_settings = 0x202d,
		.video_mode = 0x4040,
		.video_mode_adv = 0x19,
		.video_prog_mode = 0x100,
		.video_prog_mode_present = true,
		.video_sync_mode = 0x407,
		.video_sync_mode_present = true,
		.video_yc_dly = 0,
		.video_yc_dly_present = true,
		/* video_rgb_ctrl */
		/* video_filt_ctrl */
		/* video_ofld_voav_ofst */
		.yfp1_htime = 648,
		.yfp2_htime = 3207,
		.max_pxcnt = 3959,
		.hspuls_begin = 80,
		.hspuls_end = 240,
		.hspuls_switch = 80,
		.vspuls_begin = 688,
		.vspuls_end = 3248,
		.vspuls_bline = 4,
		.vspuls_eline = 8,
		.havon_begin = 648,
		.havon_end = 3207,
		.vavon_bline = 29,
		.vavon_eline = 748,
		/* eqpuls_begin */
		/* eqpuls_end */
		/* eqpuls_bline */
		/* eqpuls_eline */
		.hso_begin = 128,
		.hso_end = 208,
		.vso_begin = 128,
		.vso_end = 128,
		.vso_bline = 0,
		.vso_eline = 5,
		.vso_eline_present = true,
		/* sy_val */
		/* sy2_val */
		.max_lncnt = 749,
	},
};

union meson_hdmi_venc_mode meson_hdmi_encp_mode_1080i60 = {
	.encp = {
		.dvi_settings = 0x2029,
		.video_mode = 0x5ffc,
		.video_mode_adv = 0x19,
		.video_prog_mode = 0x100,
		.video_prog_mode_present = true,
		.video_sync_mode = 0x207,
		.video_sync_mode_present = true,
		/* video_yc_dly */
		/* video_rgb_ctrl */
		/* video_filt_ctrl */
		.video_ofld_voav_ofst = 0x11,
		.video_ofld_voav_ofst_present = true,
		.yfp1_htime = 516,
		.yfp2_htime = 4355,
		.max_pxcnt = 4399,
		.hspuls_begin = 88,
		.hspuls_end = 264,
		.hspuls_switch = 88,
		.vspuls_begin = 440,
		.vspuls_end = 2200,
		.vspuls_bline = 0,
		.vspuls_eline = 4,
		.havon_begin = 516,
		.havon_end = 4355,
		.vavon_bline = 20,
		.vavon_eline = 559,
		.eqpuls_begin = 2288,
		.eqpuls_begin_present = true,
		.eqpuls_end = 2464,
		.eqpuls_end_present = true,
		.eqpuls_bline = 0,
		.eqpuls_bline_present = true,
		.eqpuls_eline = 4,
		.eqpuls_eline_present = true,
		.hso_begin = 264,
		.hso_end = 176,
		.vso_begin = 88,
		.vso_end = 88,
		.vso_bline = 0,
		.vso_eline = 5,
		.vso_eline_present = true,
		/* sy_val */
		/* sy2_val */
		.max_lncnt = 1124,
	},
};

union meson_hdmi_venc_mode meson_hdmi_encp_mode_1080i50 = {
	.encp = {
		.dvi_settings = 0x202d,
		.video_mode = 0x5ffc,
		.video_mode_adv = 0x19,
		.video_prog_mode = 0x100,
		.video_prog_mode_present = true,
		.video_sync_mode = 0x7,
		.video_sync_mode_present = true,
		/* video_yc_dly */
		/* video_rgb_ctrl */
		/* video_filt_ctrl */
		.video_ofld_voav_ofst = 0x11,
		.video_ofld_voav_ofst_present = true,
		.yfp1_htime = 526,
		.yfp2_htime = 4365,
		.max_pxcnt = 5279,
		.hspuls_begin = 88,
		.hspuls_end = 264,
		.hspuls_switch = 88,
		.vspuls_begin = 440,
		.vspuls_end = 2200,
		.vspuls_bline = 0,
		.vspuls_eline = 4,
		.havon_begin = 526,
		.havon_end = 4365,
		.vavon_bline = 20,
		.vavon_eline = 559,
		.eqpuls_begin = 2288,
		.eqpuls_begin_present = true,
		.eqpuls_end = 2464,
		.eqpuls_end_present = true,
		.eqpuls_bline = 0,
		.eqpuls_bline_present = true,
		.eqpuls_eline = 4,
		.eqpuls_eline_present = true,
		.hso_begin = 142,
		.hso_end = 230,
		.vso_begin = 142,
		.vso_end = 142,
		.vso_bline = 0,
		.vso_eline = 5,
		.vso_eline_present = true,
		/* sy_val */
		/* sy2_val */
		.max_lncnt = 1124,
	},
};

union meson_hdmi_venc_mode meson_hdmi_encp_mode_1080p24 = {
	.encp = {
		.dvi_settings = 0xd,
		.video_mode = 0x4040,
		.video_mode_adv = 0x18,
		.video_prog_mode = 0x100,
		.video_prog_mode_present = true,
		.video_sync_mode = 0x7,
		.video_sync_mode_present = true,
		.video_yc_dly = 0,
		.video_yc_dly_present = true,
		.video_rgb_ctrl = 2,
		.video_rgb_ctrl_present = true,
		.video_filt_ctrl = 0x1052,
		.video_filt_ctrl_present = true,
		/* video_ofld_voav_ofst */
		.yfp1_htime = 271,
		.yfp2_htime = 2190,
		.max_pxcnt = 2749,
		.hspuls_begin = 44,
		.hspuls_end = 132,
		.hspuls_switch = 44,
		.vspuls_begin = 220,
		.vspuls_end = 2140,
		.vspuls_bline = 0,
		.vspuls_eline = 4,
		.havon_begin = 271,
		.havon_end = 2190,
		.vavon_bline = 41,
		.vavon_eline = 1120,
		/* eqpuls_begin */
		/* eqpuls_end */
		.eqpuls_bline = 0,
		.eqpuls_bline_present = true,
		.eqpuls_eline = 4,
		.eqpuls_eline_present = true,
		.hso_begin = 79,
		.hso_end = 123,
		.vso_begin = 79,
		.vso_end = 79,
		.vso_bline = 0,
		.vso_eline = 5,
		.vso_eline_present = true,
		/* sy_val */
		/* sy2_val */
		.max_lncnt = 1124,
	},
};

union meson_hdmi_venc_mode meson_hdmi_encp_mode_1080p30 = {
	.encp = {
		.dvi_settings = 0x1,
		.video_mode = 0x4040,
		.video_mode_adv = 0x18,
		.video_prog_mode = 0x100,
		.video_prog_mode_present = true,
		/* video_sync_mode */
		/* video_yc_dly */
		/* video_rgb_ctrl */
		.video_filt_ctrl = 0x1052,
		.video_filt_ctrl_present = true,
		/* video_ofld_voav_ofst */
		.yfp1_htime = 140,
		.yfp2_htime = 2060,
		.max_pxcnt = 2199,
		.hspuls_begin = 2156,
		.hspuls_end = 44,
		.hspuls_switch = 44,
		.vspuls_begin = 140,
		.vspuls_end = 2059,
		.vspuls_bline = 0,
		.vspuls_eline = 4,
		.havon_begin = 148,
		.havon_end = 2067,
		.vavon_bline = 41,
		.vavon_eline = 1120,
		/* eqpuls_begin */
		/* eqpuls_end */
		/* eqpuls_bline */
		/* eqpuls_eline */
		.hso_begin = 44,
		.hso_end = 2156,
		.vso_begin = 2100,
		.vso_end = 2164,
		.vso_bline = 0,
		.vso_eline = 5,
		.vso_eline_present = true,
		/* sy_val */
		/* sy2_val */
		.max_lncnt = 1124,
	},
};

union meson_hdmi_venc_mode meson_hdmi_encp_mode_1080p50 = {
	.encp = {
		.dvi_settings = 0xd,
		.video_mode = 0x4040,
		.video_mode_adv = 0x18,
		.video_prog_mode = 0x100,
		.video_prog_mode_present = true,
		.video_sync_mode = 0x7,
		.video_sync_mode_present = true,
		.video_yc_dly = 0,
		.video_yc_dly_present = true,
		.video_rgb_ctrl = 2,
		.video_rgb_ctrl_present = true,
		/* video_filt_ctrl */
		/* video_ofld_voav_ofst */
		.yfp1_htime = 271,
		.yfp2_htime = 2190,
		.max_pxcnt = 2639,
		.hspuls_begin = 44,
		.hspuls_end = 132,
		.hspuls_switch = 44,
		.vspuls_begin = 220,
		.vspuls_end = 2140,
		.vspuls_bline = 0,
		.vspuls_eline = 4,
		.havon_begin = 271,
		.havon_end = 2190,
		.vavon_bline = 41,
		.vavon_eline = 1120,
		/* eqpuls_begin */
		/* eqpuls_end */
		.eqpuls_bline = 0,
		.eqpuls_bline_present = true,
		.eqpuls_eline = 4,
		.eqpuls_eline_present = true,
		.hso_begin = 79,
		.hso_end = 123,
		.vso_begin = 79,
		.vso_end = 79,
		.vso_bline = 0,
		.vso_eline = 5,
		.vso_eline_present = true,
		/* sy_val */
		/* sy2_val */
		.max_lncnt = 1124,
	},
};

union meson_hdmi_venc_mode meson_hdmi_encp_mode_1080p60 = {
	.encp = {
		.dvi_settings = 0x1,
		.video_mode = 0x4040,
		.video_mode_adv = 0x18,
		.video_prog_mode = 0x100,
		.video_prog_mode_present = true,
		/* video_sync_mode */
		/* video_yc_dly */
		/* video_rgb_ctrl */
		.video_filt_ctrl = 0x1052,
		.video_filt_ctrl_present = true,
		/* video_ofld_voav_ofst */
		.yfp1_htime = 140,
		.yfp2_htime = 2060,
		.max_pxcnt = 2199,
		.hspuls_begin = 2156,
		.hspuls_end = 44,
		.hspuls_switch = 44,
		.vspuls_begin = 140,
		.vspuls_end = 2059,
		.vspuls_bline = 0,
		.vspuls_eline = 4,
		.havon_begin = 148,
		.havon_end = 2067,
		.vavon_bline = 41,
		.vavon_eline = 1120,
		/* eqpuls_begin */
		/* eqpuls_end */
		/* eqpuls_bline */
		/* eqpuls_eline */
		.hso_begin = 44,
		.hso_end = 2156,
		.vso_begin = 2100,
		.vso_end = 2164,
		.vso_bline = 0,
		.vso_eline = 5,
		.vso_eline_present = true,
		/* sy_val */
		/* sy2_val */
		.max_lncnt = 1124,
	},
};

static signed int to_signed(unsigned int a)
{
	if (a <= 7)
		return a;
	else
		return a - 16;
}

static unsigned long modulo(unsigned long a, unsigned long b)
{
	if (a >= b)
		return a - b;
	else
		return a;
}

bool meson_venc_hdmi_supported_mode(const struct display_timing *mode)
{
	if (mode->flags & ~(DISPLAY_FLAGS_HSYNC_LOW | DISPLAY_FLAGS_HSYNC_HIGH |
			    DISPLAY_FLAGS_VSYNC_LOW | DISPLAY_FLAGS_VSYNC_HIGH))
		return false;

	if (mode->hactive.typ < 640 || mode->hactive.typ > 1920)
		return false;

	if (mode->vactive.typ < 480 || mode->vactive.typ > 1200)
		return false;

	return true;
}

static void meson_venc_hdmi_get_dmt_vmode(const struct display_timing *mode,
					  union meson_hdmi_venc_mode *dmt_mode)
{
	memset(dmt_mode, 0, sizeof(*dmt_mode));

	dmt_mode->encp.dvi_settings = 0x21;
	dmt_mode->encp.video_mode = 0x4040;
	dmt_mode->encp.video_mode_adv = 0x18;

	dmt_mode->encp.max_pxcnt = mode->hactive.typ +
				   mode->hfront_porch.typ +
				   mode->hback_porch.typ +
				   mode->hsync_len.typ - 1;

	dmt_mode->encp.havon_begin = mode->hback_porch.typ +
				     mode->hsync_len.typ;

	dmt_mode->encp.havon_end = dmt_mode->encp.havon_begin +
				   mode->hactive.typ - 1;

	dmt_mode->encp.vavon_bline = mode->vback_porch.typ +
				     mode->vsync_len.typ;
	dmt_mode->encp.vavon_eline = dmt_mode->encp.vavon_bline +
				     mode->vactive.typ - 1;

	/* to investigate */
	dmt_mode->encp.hso_begin = 0;
	dmt_mode->encp.hso_end = mode->hsync_len.typ;
	dmt_mode->encp.vso_begin = 30;
	dmt_mode->encp.vso_end = 50;

	dmt_mode->encp.vso_bline = 0;
	dmt_mode->encp.vso_eline = mode->vsync_len.typ;
	dmt_mode->encp.vso_eline_present = true;

	dmt_mode->encp.max_lncnt = mode->vactive.typ +
			    mode->vfront_porch.typ +
			    mode->vback_porch.typ +
			    mode->vsync_len.typ - 1;
}

static void meson_venc_hdmi_mode_set(struct meson_vpu_priv *priv,
				     const struct display_timing *mode)
{
	union meson_hdmi_venc_mode *vmode = NULL;
	union meson_hdmi_venc_mode vmode_dmt;
	bool use_enci = false;
	bool venc_repeat = false;
	bool hdmi_repeat = false;
	unsigned int venc_hdmi_latency = 2;
	unsigned long total_pixels_venc = 0;
	unsigned long active_pixels_venc = 0;
	unsigned long front_porch_venc = 0;
	unsigned long hsync_pixels_venc = 0;
	unsigned long de_h_begin = 0;
	unsigned long de_h_end = 0;
	unsigned long de_v_begin_even = 0;
	unsigned long de_v_end_even = 0;
	unsigned long de_v_begin_odd = 0;
	unsigned long de_v_end_odd = 0;
	unsigned long hs_begin = 0;
	unsigned long hs_end = 0;
	unsigned long vs_adjust = 0;
	unsigned long vs_bline_evn = 0;
	unsigned long vs_eline_evn = 0;
	unsigned long vs_bline_odd = 0;
	unsigned long vs_eline_odd = 0;
	unsigned long vso_begin_evn = 0;
	unsigned long vso_begin_odd = 0;
	unsigned int eof_lines;
	unsigned int sof_lines;
	unsigned int vsync_lines;
	u32 reg;

	/* Use VENCI for 480i and 576i and double HDMI pixels */
	if (mode->flags & DISPLAY_FLAGS_DOUBLECLK) {
		hdmi_repeat = true;
		use_enci = true;
		venc_hdmi_latency = 1;
	}

	meson_venc_hdmi_get_dmt_vmode(mode, &vmode_dmt);
	vmode = &vmode_dmt;
	use_enci = false;

	debug(" max_pxcnt   %04d, max_lncnt   %04d\n"
	      " havon_begin %04d, havon_end   %04d\n"
	      " vavon_bline %04d, vavon_eline %04d\n"
	      " hso_begin   %04d, hso_end     %04d\n"
	      " vso_begin   %04d, vso_end     %04d\n"
	      " vso_bline   %04d, vso_eline   %04d\n",
		vmode->encp.max_pxcnt,   vmode->encp.max_lncnt,
		vmode->encp.havon_begin, vmode->encp.havon_end,
		vmode->encp.vavon_bline, vmode->encp.vavon_eline,
		vmode->encp.hso_begin,   vmode->encp.hso_end,
		vmode->encp.vso_begin,   vmode->encp.vso_end,
		vmode->encp.vso_bline,   vmode->encp.vso_eline);

	eof_lines = mode->vfront_porch.typ;
	if (mode->flags & DISPLAY_FLAGS_INTERLACED)
		eof_lines /= 2;

	sof_lines = mode->vback_porch.typ;
	if (mode->flags & DISPLAY_FLAGS_INTERLACED)
		sof_lines /= 2;

	vsync_lines = mode->vsync_len.typ;
	if (mode->flags & DISPLAY_FLAGS_INTERLACED)
		vsync_lines /= 2;

	total_pixels_venc = mode->hback_porch.typ + mode->hactive.typ +
			mode->hfront_porch.typ + mode->hsync_len.typ;
	if (hdmi_repeat)
		total_pixels_venc /= 2;
	if (venc_repeat)
		total_pixels_venc *= 2;

	active_pixels_venc = mode->hactive.typ;
	if (hdmi_repeat)
		active_pixels_venc /= 2;
	if (venc_repeat)
		active_pixels_venc *= 2;

	front_porch_venc = mode->hfront_porch.typ;
	if (hdmi_repeat)
		front_porch_venc /= 2;
	if (venc_repeat)
		front_porch_venc *= 2;

	hsync_pixels_venc = mode->hsync_len.typ;
	if (hdmi_repeat)
		hsync_pixels_venc /= 2;
	if (venc_repeat)
		hsync_pixels_venc *= 2;

	/* Disable VDACs */
	writel_bits(0xff, 0xff,
		    priv->io_base + _REG(VENC_VDAC_SETTING));

	writel(0, priv->io_base + _REG(ENCI_VIDEO_EN));
	writel(0, priv->io_base + _REG(ENCP_VIDEO_EN));

	debug("use_enci: %d, hdmi_repeat: %d\n", use_enci, hdmi_repeat);

	if (use_enci) {
		unsigned int lines_f0;
		unsigned int lines_f1;

		/* CVBS Filter settings */
		writel(ENCI_CFILT_CMPT_SEL_HIGH | 0x10,
		       priv->io_base + _REG(ENCI_CFILT_CTRL));
		writel(ENCI_CFILT_CMPT_CR_DLY(2) |
		       ENCI_CFILT_CMPT_CB_DLY(1),
		       priv->io_base + _REG(ENCI_CFILT_CTRL2));

		/* Digital Video Select : Interlace, clk27 clk, external */
		writel(0, priv->io_base + _REG(VENC_DVI_SETTING));

		/* Reset Video Mode */
		writel(0, priv->io_base + _REG(ENCI_VIDEO_MODE));
		writel(0, priv->io_base + _REG(ENCI_VIDEO_MODE_ADV));

		/* Horizontal sync signal output */
		writel(vmode->enci.hso_begin,
		       priv->io_base + _REG(ENCI_SYNC_HSO_BEGIN));
		writel(vmode->enci.hso_end,
		       priv->io_base + _REG(ENCI_SYNC_HSO_END));

		/* Vertical Sync lines */
		writel(vmode->enci.vso_even,
		       priv->io_base + _REG(ENCI_SYNC_VSO_EVNLN));
		writel(vmode->enci.vso_odd,
		       priv->io_base + _REG(ENCI_SYNC_VSO_ODDLN));

		/* Macrovision max amplitude change */
		writel(ENCI_MACV_MAX_AMP_ENABLE_CHANGE |
		       ENCI_MACV_MAX_AMP_VAL(vmode->enci.macv_max_amp),
		       priv->io_base + _REG(ENCI_MACV_MAX_AMP));

		/* Video mode */
		writel(vmode->enci.video_prog_mode,
		       priv->io_base + _REG(VENC_VIDEO_PROG_MODE));
		writel(vmode->enci.video_mode,
		       priv->io_base + _REG(ENCI_VIDEO_MODE));

		/*
		 * Advanced Video Mode :
		 * Demux shifting 0x2
		 * Blank line end at line17/22
		 * High bandwidth Luma Filter
		 * Low bandwidth Chroma Filter
		 * Bypass luma low pass filter
		 * No macrovision on CSYNC
		 */
		writel(ENCI_VIDEO_MODE_ADV_DMXMD(2) |
		       ENCI_VIDEO_MODE_ADV_VBICTL_LINE_17_22 |
		       ENCI_VIDEO_MODE_ADV_YBW_HIGH,
		       priv->io_base + _REG(ENCI_VIDEO_MODE_ADV));

		writel(vmode->enci.sch_adjust,
		       priv->io_base + _REG(ENCI_VIDEO_SCH));

		/* Sync mode : MASTER Master mode, free run, send HSO/VSO out */
		writel(0x07, priv->io_base + _REG(ENCI_SYNC_MODE));

		if (vmode->enci.yc_delay)
			writel(vmode->enci.yc_delay,
			       priv->io_base + _REG(ENCI_YC_DELAY));

		/* UNreset Interlaced TV Encoder */
		writel(0, priv->io_base + _REG(ENCI_DBG_PX_RST));

		/*
		 * Enable Vfifo2vd and set Y_Cb_Y_Cr:
		 * Corresponding value:
		 * Y  => 00 or 10
		 * Cb => 01
		 * Cr => 11
		 * Ex: 0x4e => 01001110 would mean Cb/Y/Cr/Y
		 */
		writel(ENCI_VFIFO2VD_CTL_ENABLE |
		       ENCI_VFIFO2VD_CTL_VD_SEL(0x4e),
		       priv->io_base + _REG(ENCI_VFIFO2VD_CTL));

		/* Timings */
		writel(vmode->enci.pixel_start,
		       priv->io_base + _REG(ENCI_VFIFO2VD_PIXEL_START));
		writel(vmode->enci.pixel_end,
		       priv->io_base + _REG(ENCI_VFIFO2VD_PIXEL_END));

		writel(vmode->enci.top_field_line_start,
		       priv->io_base + _REG(ENCI_VFIFO2VD_LINE_TOP_START));
		writel(vmode->enci.top_field_line_end,
		       priv->io_base + _REG(ENCI_VFIFO2VD_LINE_TOP_END));

		writel(vmode->enci.bottom_field_line_start,
		       priv->io_base + _REG(ENCI_VFIFO2VD_LINE_BOT_START));
		writel(vmode->enci.bottom_field_line_end,
		       priv->io_base + _REG(ENCI_VFIFO2VD_LINE_BOT_END));

		/* Select ENCI for VIU */
		meson_vpp_setup_mux(priv, MESON_VIU_VPP_MUX_ENCI);

		/* Interlace video enable */
		writel(ENCI_VIDEO_EN_ENABLE,
		       priv->io_base + _REG(ENCI_VIDEO_EN));

		lines_f0 = mode->vback_porch.typ + mode->vactive.typ +
			   mode->vback_porch.typ + mode->vsync_len.typ;
		lines_f0 = lines_f0 >> 1;
		lines_f1 = lines_f0 + 1;

		de_h_begin = modulo(readl(priv->io_base +
					_REG(ENCI_VFIFO2VD_PIXEL_START))
					+ venc_hdmi_latency,
				    total_pixels_venc);
		de_h_end  = modulo(de_h_begin + active_pixels_venc,
				   total_pixels_venc);

		writel(de_h_begin,
		       priv->io_base + _REG(ENCI_DE_H_BEGIN));
		writel(de_h_end,
		       priv->io_base + _REG(ENCI_DE_H_END));

		de_v_begin_even = readl(priv->io_base +
					_REG(ENCI_VFIFO2VD_LINE_TOP_START));
		de_v_end_even  = de_v_begin_even + mode->vactive.typ;
		de_v_begin_odd = readl(priv->io_base +
					_REG(ENCI_VFIFO2VD_LINE_BOT_START));
		de_v_end_odd = de_v_begin_odd + mode->vactive.typ;

		writel(de_v_begin_even,
		       priv->io_base + _REG(ENCI_DE_V_BEGIN_EVEN));
		writel(de_v_end_even,
		       priv->io_base + _REG(ENCI_DE_V_END_EVEN));
		writel(de_v_begin_odd,
		       priv->io_base + _REG(ENCI_DE_V_BEGIN_ODD));
		writel(de_v_end_odd,
		       priv->io_base + _REG(ENCI_DE_V_END_ODD));

		/* Program Hsync timing */
		hs_begin = de_h_end + front_porch_venc;
		if (de_h_end + front_porch_venc >= total_pixels_venc) {
			hs_begin -= total_pixels_venc;
			vs_adjust  = 1;
		} else {
			hs_begin = de_h_end + front_porch_venc;
			vs_adjust  = 0;
		}

		hs_end = modulo(hs_begin + hsync_pixels_venc,
				total_pixels_venc);
		writel(hs_begin,
		       priv->io_base + _REG(ENCI_DVI_HSO_BEGIN));
		writel(hs_end,
		       priv->io_base + _REG(ENCI_DVI_HSO_END));

		/* Program Vsync timing for even field */
		if (((de_v_end_odd - 1) + eof_lines + vs_adjust) >= lines_f1) {
			vs_bline_evn = (de_v_end_odd - 1)
					+ eof_lines
					+ vs_adjust
					- lines_f1;
			vs_eline_evn = vs_bline_evn + vsync_lines;

			writel(vs_bline_evn,
			       priv->io_base + _REG(ENCI_DVI_VSO_BLINE_EVN));

			writel(vs_eline_evn,
			       priv->io_base + _REG(ENCI_DVI_VSO_ELINE_EVN));

			writel(hs_begin,
			       priv->io_base + _REG(ENCI_DVI_VSO_BEGIN_EVN));
			writel(hs_begin,
			       priv->io_base + _REG(ENCI_DVI_VSO_END_EVN));
		} else {
			vs_bline_odd = (de_v_end_odd - 1)
					+ eof_lines
					+ vs_adjust;

			writel(vs_bline_odd,
			       priv->io_base + _REG(ENCI_DVI_VSO_BLINE_ODD));

			writel(hs_begin,
			       priv->io_base + _REG(ENCI_DVI_VSO_BEGIN_ODD));

			if ((vs_bline_odd + vsync_lines) >= lines_f1) {
				vs_eline_evn = vs_bline_odd
						+ vsync_lines
						- lines_f1;

				writel(vs_eline_evn, priv->io_base
				       + _REG(ENCI_DVI_VSO_ELINE_EVN));

				writel(hs_begin, priv->io_base
				       + _REG(ENCI_DVI_VSO_END_EVN));
			} else {
				vs_eline_odd = vs_bline_odd
						+ vsync_lines;

				writel(vs_eline_odd, priv->io_base
				       + _REG(ENCI_DVI_VSO_ELINE_ODD));

				writel(hs_begin, priv->io_base
				       + _REG(ENCI_DVI_VSO_END_ODD));
			}
		}

		/* Program Vsync timing for odd field */
		if (((de_v_end_even - 1) + (eof_lines + 1)) >= lines_f0) {
			vs_bline_odd = (de_v_end_even - 1)
					+ (eof_lines + 1)
					- lines_f0;
			vs_eline_odd = vs_bline_odd + vsync_lines;

			writel(vs_bline_odd,
			       priv->io_base + _REG(ENCI_DVI_VSO_BLINE_ODD));

			writel(vs_eline_odd,
			       priv->io_base + _REG(ENCI_DVI_VSO_ELINE_ODD));

			vso_begin_odd  = modulo(hs_begin
						+ (total_pixels_venc >> 1),
						total_pixels_venc);

			writel(vso_begin_odd,
			       priv->io_base + _REG(ENCI_DVI_VSO_BEGIN_ODD));
			writel(vso_begin_odd,
			       priv->io_base + _REG(ENCI_DVI_VSO_END_ODD));
		} else {
			vs_bline_evn = (de_v_end_even - 1)
					+ (eof_lines + 1);

			writel(vs_bline_evn,
			       priv->io_base + _REG(ENCI_DVI_VSO_BLINE_EVN));

			vso_begin_evn  = modulo(hs_begin
						+ (total_pixels_venc >> 1),
						total_pixels_venc);

			writel(vso_begin_evn, priv->io_base
					+ _REG(ENCI_DVI_VSO_BEGIN_EVN));

			if (vs_bline_evn + vsync_lines >= lines_f0) {
				vs_eline_odd = vs_bline_evn
						+ vsync_lines
						- lines_f0;

				writel(vs_eline_odd, priv->io_base
						+ _REG(ENCI_DVI_VSO_ELINE_ODD));

				writel(vso_begin_evn, priv->io_base
						+ _REG(ENCI_DVI_VSO_END_ODD));
			} else {
				vs_eline_evn = vs_bline_evn + vsync_lines;

				writel(vs_eline_evn, priv->io_base
						+ _REG(ENCI_DVI_VSO_ELINE_EVN));

				writel(vso_begin_evn, priv->io_base
						+ _REG(ENCI_DVI_VSO_END_EVN));
			}
		}
	} else {
		writel(vmode->encp.dvi_settings,
		       priv->io_base + _REG(VENC_DVI_SETTING));
		writel(vmode->encp.video_mode,
		       priv->io_base + _REG(ENCP_VIDEO_MODE));
		writel(vmode->encp.video_mode_adv,
		       priv->io_base + _REG(ENCP_VIDEO_MODE_ADV));
		if (vmode->encp.video_prog_mode_present)
			writel(vmode->encp.video_prog_mode,
			       priv->io_base + _REG(VENC_VIDEO_PROG_MODE));
		if (vmode->encp.video_sync_mode_present)
			writel(vmode->encp.video_sync_mode,
			       priv->io_base + _REG(ENCP_VIDEO_SYNC_MODE));
		if (vmode->encp.video_yc_dly_present)
			writel(vmode->encp.video_yc_dly,
			       priv->io_base + _REG(ENCP_VIDEO_YC_DLY));
		if (vmode->encp.video_rgb_ctrl_present)
			writel(vmode->encp.video_rgb_ctrl,
			       priv->io_base + _REG(ENCP_VIDEO_RGB_CTRL));
		if (vmode->encp.video_filt_ctrl_present)
			writel(vmode->encp.video_filt_ctrl,
			       priv->io_base + _REG(ENCP_VIDEO_FILT_CTRL));
		if (vmode->encp.video_ofld_voav_ofst_present)
			writel(vmode->encp.video_ofld_voav_ofst,
			       priv->io_base
			       + _REG(ENCP_VIDEO_OFLD_VOAV_OFST));
		writel(vmode->encp.yfp1_htime,
		       priv->io_base + _REG(ENCP_VIDEO_YFP1_HTIME));
		writel(vmode->encp.yfp2_htime,
		       priv->io_base + _REG(ENCP_VIDEO_YFP2_HTIME));
		writel(vmode->encp.max_pxcnt,
		       priv->io_base + _REG(ENCP_VIDEO_MAX_PXCNT));
		writel(vmode->encp.hspuls_begin,
		       priv->io_base + _REG(ENCP_VIDEO_HSPULS_BEGIN));
		writel(vmode->encp.hspuls_end,
		       priv->io_base + _REG(ENCP_VIDEO_HSPULS_END));
		writel(vmode->encp.hspuls_switch,
		       priv->io_base + _REG(ENCP_VIDEO_HSPULS_SWITCH));
		writel(vmode->encp.vspuls_begin,
		       priv->io_base + _REG(ENCP_VIDEO_VSPULS_BEGIN));
		writel(vmode->encp.vspuls_end,
		       priv->io_base + _REG(ENCP_VIDEO_VSPULS_END));
		writel(vmode->encp.vspuls_bline,
		       priv->io_base + _REG(ENCP_VIDEO_VSPULS_BLINE));
		writel(vmode->encp.vspuls_eline,
		       priv->io_base + _REG(ENCP_VIDEO_VSPULS_ELINE));
		if (vmode->encp.eqpuls_begin_present)
			writel(vmode->encp.eqpuls_begin,
			       priv->io_base + _REG(ENCP_VIDEO_EQPULS_BEGIN));
		if (vmode->encp.eqpuls_end_present)
			writel(vmode->encp.eqpuls_end,
			       priv->io_base + _REG(ENCP_VIDEO_EQPULS_END));
		if (vmode->encp.eqpuls_bline_present)
			writel(vmode->encp.eqpuls_bline,
			       priv->io_base + _REG(ENCP_VIDEO_EQPULS_BLINE));
		if (vmode->encp.eqpuls_eline_present)
			writel(vmode->encp.eqpuls_eline,
			       priv->io_base + _REG(ENCP_VIDEO_EQPULS_ELINE));
		writel(vmode->encp.havon_begin,
		       priv->io_base + _REG(ENCP_VIDEO_HAVON_BEGIN));
		writel(vmode->encp.havon_end,
		       priv->io_base + _REG(ENCP_VIDEO_HAVON_END));
		writel(vmode->encp.vavon_bline,
		       priv->io_base + _REG(ENCP_VIDEO_VAVON_BLINE));
		writel(vmode->encp.vavon_eline,
		       priv->io_base + _REG(ENCP_VIDEO_VAVON_ELINE));
		writel(vmode->encp.hso_begin,
		       priv->io_base + _REG(ENCP_VIDEO_HSO_BEGIN));
		writel(vmode->encp.hso_end,
		       priv->io_base + _REG(ENCP_VIDEO_HSO_END));
		writel(vmode->encp.vso_begin,
		       priv->io_base + _REG(ENCP_VIDEO_VSO_BEGIN));
		writel(vmode->encp.vso_end,
		       priv->io_base + _REG(ENCP_VIDEO_VSO_END));
		writel(vmode->encp.vso_bline,
		       priv->io_base + _REG(ENCP_VIDEO_VSO_BLINE));
		if (vmode->encp.vso_eline_present)
			writel(vmode->encp.vso_eline,
			       priv->io_base + _REG(ENCP_VIDEO_VSO_ELINE));
		if (vmode->encp.sy_val_present)
			writel(vmode->encp.sy_val,
			       priv->io_base + _REG(ENCP_VIDEO_SY_VAL));
		if (vmode->encp.sy2_val_present)
			writel(vmode->encp.sy2_val,
			       priv->io_base + _REG(ENCP_VIDEO_SY2_VAL));
		writel(vmode->encp.max_lncnt,
		       priv->io_base + _REG(ENCP_VIDEO_MAX_LNCNT));

		writel(1, priv->io_base + _REG(ENCP_VIDEO_EN));

		/* Set DE signal's polarity is active high */
		writel_bits(ENCP_VIDEO_MODE_DE_V_HIGH,
			    ENCP_VIDEO_MODE_DE_V_HIGH,
			    priv->io_base + _REG(ENCP_VIDEO_MODE));

		/* Program DE timing */
		de_h_begin = modulo(readl(priv->io_base +
					_REG(ENCP_VIDEO_HAVON_BEGIN))
					+ venc_hdmi_latency,
				    total_pixels_venc);
		de_h_end = modulo(de_h_begin + active_pixels_venc,
				  total_pixels_venc);

		writel(de_h_begin,
		       priv->io_base + _REG(ENCP_DE_H_BEGIN));
		writel(de_h_end,
		       priv->io_base + _REG(ENCP_DE_H_END));

		/* Program DE timing for even field */
		de_v_begin_even = readl(priv->io_base
						+ _REG(ENCP_VIDEO_VAVON_BLINE));
		if (mode->flags & DISPLAY_FLAGS_INTERLACED)
			de_v_end_even = de_v_begin_even +
					(mode->vactive.typ / 2);
		else
			de_v_end_even = de_v_begin_even + mode->vactive.typ;

		writel(de_v_begin_even,
		       priv->io_base + _REG(ENCP_DE_V_BEGIN_EVEN));
		writel(de_v_end_even,
		       priv->io_base + _REG(ENCP_DE_V_END_EVEN));

		/* Program DE timing for odd field if needed */
		if (mode->flags & DISPLAY_FLAGS_INTERLACED) {
			unsigned int ofld_voav_ofst =
				readl(priv->io_base +
					_REG(ENCP_VIDEO_OFLD_VOAV_OFST));
			de_v_begin_odd = to_signed((ofld_voav_ofst & 0xf0) >> 4)
					+ de_v_begin_even
					+ ((mode->vfront_porch.typ +
					    mode->vactive.typ +
					    mode->vsync_len.typ - 1) / 2);
			de_v_end_odd = de_v_begin_odd + (mode->vactive.typ / 2);

			writel(de_v_begin_odd,
			       priv->io_base + _REG(ENCP_DE_V_BEGIN_ODD));
			writel(de_v_end_odd,
			       priv->io_base + _REG(ENCP_DE_V_END_ODD));
		}

		/* Program Hsync timing */
		if ((de_h_end + front_porch_venc) >= total_pixels_venc) {
			hs_begin = de_h_end
				   + front_porch_venc
				   - total_pixels_venc;
			vs_adjust  = 1;
		} else {
			hs_begin = de_h_end
				   + front_porch_venc;
			vs_adjust  = 0;
		}

		hs_end = modulo(hs_begin + hsync_pixels_venc,
				total_pixels_venc);

		writel(hs_begin,
		       priv->io_base + _REG(ENCP_DVI_HSO_BEGIN));
		writel(hs_end,
		       priv->io_base + _REG(ENCP_DVI_HSO_END));

		/* Program Vsync timing for even field */
		if (de_v_begin_even >=
				(sof_lines + vsync_lines + (1 - vs_adjust)))
			vs_bline_evn = de_v_begin_even
					- sof_lines
					- vsync_lines
					- (1 - vs_adjust);
		else
			vs_bline_evn = (mode->vfront_porch.typ +
					mode->vactive.typ +
					mode->vsync_len.typ) +
					+ de_v_begin_even
					- sof_lines
					- vsync_lines
					- (1 - vs_adjust);

		vs_eline_evn = modulo(vs_bline_evn + vsync_lines,
				      mode->hfront_porch.typ +
				      mode->hactive.typ +
				      mode->hsync_len.typ);

		writel(vs_bline_evn,
		       priv->io_base + _REG(ENCP_DVI_VSO_BLINE_EVN));
		writel(vs_eline_evn,
		       priv->io_base + _REG(ENCP_DVI_VSO_ELINE_EVN));

		vso_begin_evn = hs_begin;
		writel(vso_begin_evn,
		       priv->io_base + _REG(ENCP_DVI_VSO_BEGIN_EVN));
		writel(vso_begin_evn,
		       priv->io_base + _REG(ENCP_DVI_VSO_END_EVN));

		/* Program Vsync timing for odd field if needed */
		if (mode->flags & DISPLAY_FLAGS_INTERLACED) {
			vs_bline_odd = (de_v_begin_odd - 1)
					- sof_lines
					- vsync_lines;
			vs_eline_odd = (de_v_begin_odd - 1)
					- vsync_lines;
			vso_begin_odd  = modulo(hs_begin
						+ (total_pixels_venc >> 1),
						total_pixels_venc);

			writel(vs_bline_odd,
			       priv->io_base + _REG(ENCP_DVI_VSO_BLINE_ODD));
			writel(vs_eline_odd,
			       priv->io_base + _REG(ENCP_DVI_VSO_ELINE_ODD));
			writel(vso_begin_odd,
			       priv->io_base + _REG(ENCP_DVI_VSO_BEGIN_ODD));
			writel(vso_begin_odd,
			       priv->io_base + _REG(ENCP_DVI_VSO_END_ODD));
		}

		/* Select ENCP for VIU */
		meson_vpp_setup_mux(priv, MESON_VIU_VPP_MUX_ENCP);
	}

	/* Set VPU HDMI setting */
	/* Select ENCP or ENCI data to HDMI */
	if (use_enci)
		reg = VPU_HDMI_ENCI_DATA_TO_HDMI;
	else
		reg = VPU_HDMI_ENCP_DATA_TO_HDMI;

	/* Invert polarity of HSYNC from VENC */
	if (mode->flags & DISPLAY_FLAGS_HSYNC_HIGH)
		reg |= VPU_HDMI_INV_HSYNC;

	/* Invert polarity of VSYNC from VENC */
	if (mode->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		reg |= VPU_HDMI_INV_VSYNC;

	/* Output data format: CbYCr */
	reg |= VPU_HDMI_OUTPUT_CBYCR;

	/*
	 * Write rate to the async FIFO between VENC and HDMI.
	 * One write every 2 wr_clk.
	 */
	if (venc_repeat)
		reg |= VPU_HDMI_WR_RATE(2);

	/*
	 * Read rate to the async FIFO between VENC and HDMI.
	 * One read every 2 wr_clk.
	 */
	if (hdmi_repeat)
		reg |= VPU_HDMI_RD_RATE(2);

	writel(reg, priv->io_base + _REG(VPU_HDMI_SETTING));
}

static void meson_venci_cvbs_mode_set(struct meson_vpu_priv *priv,
				      struct meson_cvbs_enci_mode *mode)
{
	u32 reg;

	/* CVBS Filter settings */
	writel(ENCI_CFILT_CMPT_SEL_HIGH | 0x10,
	       priv->io_base + _REG(ENCI_CFILT_CTRL));
	writel(ENCI_CFILT_CMPT_CR_DLY(2) |
	       ENCI_CFILT_CMPT_CB_DLY(1),
	       priv->io_base + _REG(ENCI_CFILT_CTRL2));

	/* Digital Video Select : Interlace, clk27 clk, external */
	writel(0, priv->io_base + _REG(VENC_DVI_SETTING));

	/* Reset Video Mode */
	writel(0, priv->io_base + _REG(ENCI_VIDEO_MODE));
	writel(0, priv->io_base + _REG(ENCI_VIDEO_MODE_ADV));

	/* Horizontal sync signal output */
	writel(mode->hso_begin,
	       priv->io_base + _REG(ENCI_SYNC_HSO_BEGIN));
	writel(mode->hso_end,
	       priv->io_base + _REG(ENCI_SYNC_HSO_END));

	/* Vertical Sync lines */
	writel(mode->vso_even,
	       priv->io_base + _REG(ENCI_SYNC_VSO_EVNLN));
	writel(mode->vso_odd,
	       priv->io_base + _REG(ENCI_SYNC_VSO_ODDLN));

	/* Macrovision max amplitude change */
	writel(ENCI_MACV_MAX_AMP_ENABLE_CHANGE |
	       ENCI_MACV_MAX_AMP_VAL(mode->macv_max_amp),
	       priv->io_base + _REG(ENCI_MACV_MAX_AMP));

	/* Video mode */
	writel(mode->video_prog_mode,
	       priv->io_base + _REG(VENC_VIDEO_PROG_MODE));
	writel(mode->video_mode,
	       priv->io_base + _REG(ENCI_VIDEO_MODE));

	/*
	 * Advanced Video Mode :
	 * Demux shifting 0x2
	 * Blank line end at line17/22
	 * High bandwidth Luma Filter
	 * Low bandwidth Chroma Filter
	 * Bypass luma low pass filter
	 * No macrovision on CSYNC
	 */
	writel(ENCI_VIDEO_MODE_ADV_DMXMD(2) |
	       ENCI_VIDEO_MODE_ADV_VBICTL_LINE_17_22 |
	       ENCI_VIDEO_MODE_ADV_YBW_HIGH,
	       priv->io_base + _REG(ENCI_VIDEO_MODE_ADV));

	writel(mode->sch_adjust, priv->io_base + _REG(ENCI_VIDEO_SCH));

	/* Sync mode : MASTER Master mode, free run, send HSO/VSO out */
	writel(0x07, priv->io_base + _REG(ENCI_SYNC_MODE));

	/* 0x3 Y, C, and Component Y delay */
	writel(mode->yc_delay, priv->io_base + _REG(ENCI_YC_DELAY));

	/* Timings */
	writel(mode->pixel_start,
	       priv->io_base + _REG(ENCI_VFIFO2VD_PIXEL_START));
	writel(mode->pixel_end,
	       priv->io_base + _REG(ENCI_VFIFO2VD_PIXEL_END));

	writel(mode->top_field_line_start,
	       priv->io_base + _REG(ENCI_VFIFO2VD_LINE_TOP_START));
	writel(mode->top_field_line_end,
	       priv->io_base + _REG(ENCI_VFIFO2VD_LINE_TOP_END));

	writel(mode->bottom_field_line_start,
	       priv->io_base + _REG(ENCI_VFIFO2VD_LINE_BOT_START));
	writel(mode->bottom_field_line_end,
	       priv->io_base + _REG(ENCI_VFIFO2VD_LINE_BOT_END));

	/* Internal Venc, Internal VIU Sync, Internal Vencoder */
	writel(0, priv->io_base + _REG(VENC_SYNC_ROUTE));

	/* UNreset Interlaced TV Encoder */
	writel(0, priv->io_base + _REG(ENCI_DBG_PX_RST));

	/*
	 * Enable Vfifo2vd and set Y_Cb_Y_Cr:
	 * Corresponding value:
	 * Y  => 00 or 10
	 * Cb => 01
	 * Cr => 11
	 * Ex: 0x4e => 01001110 would mean Cb/Y/Cr/Y
	 */
	writel(ENCI_VFIFO2VD_CTL_ENABLE |
	       ENCI_VFIFO2VD_CTL_VD_SEL(0x4e),
	       priv->io_base + _REG(ENCI_VFIFO2VD_CTL));

	/* Power UP Dacs */
	writel(0, priv->io_base + _REG(VENC_VDAC_SETTING));

	/* Video Upsampling */
	/*
	 * CTRL0, CTRL1 and CTRL2:
	 * Filter0: input data sample every 2 cloks
	 * Filter1: filtering and upsample enable
	 */
	reg = VENC_UPSAMPLE_CTRL_F0_2_CLK_RATIO | VENC_UPSAMPLE_CTRL_F1_EN |
		VENC_UPSAMPLE_CTRL_F1_UPSAMPLE_EN;

	/*
	 * Upsample CTRL0:
	 * Interlace High Bandwidth Luma
	 */
	writel(VENC_UPSAMPLE_CTRL_INTERLACE_HIGH_LUMA | reg,
	       priv->io_base + _REG(VENC_UPSAMPLE_CTRL0));

	/*
	 * Upsample CTRL1:
	 * Interlace Pb
	 */
	writel(VENC_UPSAMPLE_CTRL_INTERLACE_PB | reg,
	       priv->io_base + _REG(VENC_UPSAMPLE_CTRL1));

	/*
	 * Upsample CTRL2:
	 * Interlace R
	 */
	writel(VENC_UPSAMPLE_CTRL_INTERLACE_PR | reg,
	       priv->io_base + _REG(VENC_UPSAMPLE_CTRL2));

	/* Select Interlace Y DACs */
	writel(0, priv->io_base + _REG(VENC_VDAC_DACSEL0));
	writel(0, priv->io_base + _REG(VENC_VDAC_DACSEL1));
	writel(0, priv->io_base + _REG(VENC_VDAC_DACSEL2));
	writel(0, priv->io_base + _REG(VENC_VDAC_DACSEL3));
	writel(0, priv->io_base + _REG(VENC_VDAC_DACSEL4));
	writel(0, priv->io_base + _REG(VENC_VDAC_DACSEL5));

	/* Select ENCI for VIU */
	meson_vpp_setup_mux(priv, MESON_VIU_VPP_MUX_ENCI);

	/* Enable ENCI FIFO */
	writel(VENC_VDAC_FIFO_EN_ENCI_ENABLE,
	       priv->io_base + _REG(VENC_VDAC_FIFO_CTRL));

	/* Select ENCI DACs 0, 1, 4, and 5 */
	writel(0x11, priv->io_base + _REG(ENCI_DACSEL_0));
	writel(0x11, priv->io_base + _REG(ENCI_DACSEL_1));

	/* Interlace video enable */
	writel(ENCI_VIDEO_EN_ENABLE,
	       priv->io_base + _REG(ENCI_VIDEO_EN));

	/* Configure Video Saturation / Contrast / Brightness / Hue */
	writel(mode->video_saturation,
	       priv->io_base + _REG(ENCI_VIDEO_SAT));
	writel(mode->video_contrast,
	       priv->io_base + _REG(ENCI_VIDEO_CONT));
	writel(mode->video_brightness,
	       priv->io_base + _REG(ENCI_VIDEO_BRIGHT));
	writel(mode->video_hue,
	       priv->io_base + _REG(ENCI_VIDEO_HUE));

	/* Enable DAC0 Filter */
	writel(VENC_VDAC_DAC0_FILT_CTRL0_EN,
	       priv->io_base + _REG(VENC_VDAC_DAC0_FILT_CTRL0));
	writel(0xfc48, priv->io_base + _REG(VENC_VDAC_DAC0_FILT_CTRL1));

	/* 0 in Macrovision register 0 */
	writel(0, priv->io_base + _REG(ENCI_MACV_N0));

	/* Analog Synchronization and color burst value adjust */
	writel(mode->analog_sync_adj,
	       priv->io_base + _REG(ENCI_SYNC_ADJ));

	/* enable VDAC */
	writel_bits(VENC_VDAC_SEL_ATV_DMD, 0,
		    priv->io_base + _REG(VENC_VDAC_DACSEL0));

	if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_GXBB))
		hhi_write(HHI_VDAC_CNTL0, 1);
	else if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_GXL) ||
		 meson_vpu_is_compatible(priv, VPU_COMPATIBLE_GXM))
		hhi_write(HHI_VDAC_CNTL0, 0xf0001);
	else if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_G12A))
		hhi_write(HHI_VDAC_CNTL0_G12A, 0x906001);

	if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_G12A))
		hhi_write(HHI_VDAC_CNTL1_G12A, 0);
	else
		hhi_write(HHI_VDAC_CNTL1, 0);
}

void meson_vpu_setup_venc(struct udevice *dev,
			  const struct display_timing *mode, bool is_cvbs)
{
	struct meson_vpu_priv *priv = dev_get_priv(dev);

	if (is_cvbs)
		return meson_venci_cvbs_mode_set(priv, &meson_cvbs_enci_pal);

	meson_venc_hdmi_mode_set(priv, mode);
}
