/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sunxi platform display controller register and constant defines
 *
 * (C) Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 *
 * Based on out of tree Linux DRM driver defines:
 * Copyright (C) 2016 Jean-Francois Moine <moinejf@free.fr>
 * Copyright (c) 2016 Allwinnertech Co., Ltd.
 */

#ifndef _SUNXI_DISPLAY2_H
#define _SUNXI_DISPLAY2_H

/* internal clock settings */
struct de_clk {
	u32 gate_cfg;
	u32 bus_cfg;
	u32 rst_cfg;
	u32 div_cfg;
	u32 sel_cfg;
};

/* global control */
struct de_glb {
	u32 ctl;
	u32 status;
	u32 dbuff;
	u32 size;
};

/* alpha blending */
struct de_bld {
	u32 fcolor_ctl;
	struct {
		u32 fcolor;
		u32 insize;
		u32 offset;
		u32 dum;
	} attr[4];
	u32 dum0[15];
	u32 route;
	u32 premultiply;
	u32 bkcolor;
	u32 output_size;
	u32 bld_mode[4];
	u32 dum1[4];
	u32 ck_ctl;
	u32 ck_cfg;
	u32 dum2[2];
	u32 ck_max[4];
	u32 dum3[4];
	u32 ck_min[4];
	u32 dum4[3];
	u32 out_ctl;
};

/* VI channel */
struct de_vi {
	struct {
		u32 attr;
		u32 size;
		u32 coord;
		u32 pitch[3];
		u32 top_laddr[3];
		u32 bot_laddr[3];
	} cfg[4];
	u32 fcolor[4];
	u32 top_haddr[3];
	u32 bot_haddr[3];
	u32 ovl_size[2];
	u32 hori[2];
	u32 vert[2];
};

struct de_ui {
	struct {
		u32 attr;
		u32 size;
		u32 coord;
		u32 pitch;
		u32 top_laddr;
		u32 bot_laddr;
		u32 fcolor;
		u32 dum;
	} cfg[4];
	u32 top_haddr;
	u32 bot_haddr;
	u32 ovl_size;
};

struct de_csc {
	u32 csc_ctl;
	u8 res[0xc];
	u32 coef11;
	u32 coef12;
	u32 coef13;
	u32 coef14;
	u32 coef21;
	u32 coef22;
	u32 coef23;
	u32 coef24;
	u32 coef31;
	u32 coef32;
	u32 coef33;
	u32 coef34;
};

/*
 * DE register constants.
 */
#define SUNXI_DE2_MUX0_BASE			(SUNXI_DE2_BASE + 0x100000)
#define SUNXI_DE2_MUX1_BASE			(SUNXI_DE2_BASE + 0x200000)

#define SUNXI_DE2_MUX_GLB_REGS			0x00000
#define SUNXI_DE2_MUX_BLD_REGS			0x01000
#define SUNXI_DE2_MUX_CHAN_REGS			0x02000
#define SUNXI_DE2_MUX_CHAN_SZ			0x1000
#define SUNXI_DE2_MUX_VSU_REGS			0x20000
#define SUNXI_DE2_MUX_GSU1_REGS			0x30000
#define SUNXI_DE2_MUX_GSU2_REGS			0x40000
#define SUNXI_DE2_MUX_GSU3_REGS			0x50000
#define SUNXI_DE2_MUX_FCE_REGS			0xa0000
#define SUNXI_DE2_MUX_BWS_REGS			0xa2000
#define SUNXI_DE2_MUX_LTI_REGS			0xa4000
#define SUNXI_DE2_MUX_PEAK_REGS			0xa6000
#define SUNXI_DE2_MUX_ASE_REGS			0xa8000
#define SUNXI_DE2_MUX_FCC_REGS			0xaa000
#define SUNXI_DE2_MUX_DCSC_REGS			0xb0000

#define SUNXI_DE2_FORMAT_XRGB_8888		4
#define SUNXI_DE2_FORMAT_RGB_565		10

#define SUNXI_DE2_MUX_GLB_CTL_EN		(1 << 0)
#define SUNXI_DE2_UI_CFG_ATTR_EN		(1 << 0)
#define SUNXI_DE2_UI_CFG_ATTR_FMT(f)		((f & 0xf) << 8)

#define SUNXI_DE2_WH(w, h)			(((h - 1) << 16) | (w - 1))

#endif /* _SUNXI_DISPLAY2_H */
