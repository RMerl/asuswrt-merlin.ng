/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#ifndef _NX__DISPLAY_H_
#define _NX__DISPLAY_H_

#define	DP_PLANS_NUM	3

/* the display output format. */
#define	DPC_FORMAT_RGB555		0  /* RGB555 Format */
#define	DPC_FORMAT_RGB565		1  /* RGB565 Format */
#define	DPC_FORMAT_RGB666		2  /* RGB666 Format */
#define	DPC_FORMAT_RGB888		3  /* RGB888 Format */
#define	DPC_FORMAT_MRGB555A		4  /* MRGB555A Format */
#define	DPC_FORMAT_MRGB555B		5  /* MRGB555B Format */
#define	DPC_FORMAT_MRGB565		6  /* MRGB565 Format */
#define	DPC_FORMAT_MRGB666		7  /* MRGB666 Format */
#define	DPC_FORMAT_MRGB888A		8  /* MRGB888A Format */
#define	DPC_FORMAT_MRGB888B		9  /* MRGB888B Format */
#define	DPC_FORMAT_CCIR656		10 /* ITU-R BT.656 / 601(8-bit) */
#define	DPC_FORMAT_CCIR601A		12 /* ITU-R BT.601A */
#define	DPC_FORMAT_CCIR601B		13 /* ITU-R BT.601B */
#define	DPC_FORMAT_4096COLOR	1  /* 4096 Color Format */
#define	DPC_FORMAT_16GRAY		3  /* 16 Level Gray Format */

/* layer pixel format. */
#define	MLC_RGBFMT_R5G6B5		0x44320000	/* {R5,G6,B5 }. */
#define	MLC_RGBFMT_B5G6R5		0xC4320000  /* {B5,G6,R5 }. */
#define	MLC_RGBFMT_X1R5G5B5		0x43420000  /* {X1,R5,G5,B5}. */
#define	MLC_RGBFMT_X1B5G5R5		0xC3420000  /* {X1,B5,G5,R5}. */
#define	MLC_RGBFMT_X4R4G4B4		0x42110000  /* {X4,R4,G4,B4}. */
#define	MLC_RGBFMT_X4B4G4R4		0xC2110000	/* {X4,B4,G4,R4}. */
#define	MLC_RGBFMT_X8R3G3B2		0x41200000	/* {X8,R3,G3,B2}. */
#define	MLC_RGBFMT_X8B3G3R2		0xC1200000	/* {X8,B3,G3,R2}. */
#define	MLC_RGBFMT_A1R5G5B5		0x33420000	/* {A1,R5,G5,B5}. */
#define	MLC_RGBFMT_A1B5G5R5		0xB3420000	/* {A1,B5,G5,R5}. */
#define	MLC_RGBFMT_A4R4G4B4		0x22110000	/* {A4,R4,G4,B4}. */
#define	MLC_RGBFMT_A4B4G4R4		0xA2110000	/* {A4,B4,G4,R4}. */
#define	MLC_RGBFMT_A8R3G3B2		0x11200000	/* {A8,R3,G3,B2}. */
#define	MLC_RGBFMT_A8B3G3R2		0x91200000	/* {A8,B3,G3,R2}. */
#define	MLC_RGBFMT_R8G8B8		0x46530000	/* {R8,G8,B8 }. */
#define	MLC_RGBFMT_B8G8R8		0xC6530000	/* {B8,G8,R8 }. */
#define	MLC_RGBFMT_X8R8G8B8		0x46530000	/* {X8,R8,G8,B8}. */
#define	MLC_RGBFMT_X8B8G8R8		0xC6530000	/* {X8,B8,G8,R8}. */
#define	MLC_RGBFMT_A8R8G8B8		0x06530000	/* {A8,R8,G8,B8}. */
#define	MLC_RGBFMT_A8B8G8R8		0x86530000	/* {A8,B8,G8,R8}.  */

/* the data output order in case of ITU-R BT.656 / 601. */
#define	DPC_YCORDER_CBYCRY		0
#define	DPC_YCORDER_CRYCBY		1
#define	DPC_YCORDER_YCBYCR		2
#define	DPC_YCORDER_YCRYCB		3

/* the PAD output clock. */
#define	DPC_PADCLKSEL_VCLK		0	/* VCLK */
#define	DPC_PADCLKSEL_VCLK2		1	/* VCLK2 */

/* display sync info for DPC */
struct dp_sync_info {
	int interlace;
	int h_active_len;
	int h_sync_width;
	int h_back_porch;
	int h_front_porch;
	int h_sync_invert;	/* default active low */
	int v_active_len;
	int v_sync_width;
	int v_back_porch;
	int v_front_porch;
	int v_sync_invert;	/* default active low */
	int pixel_clock_hz;	/* HZ */
};

/* syncgen control (DPC) */
#define	DP_SYNC_DELAY_RGB_PVD		(1 << 0)
#define	DP_SYNC_DELAY_HSYNC_CP1		(1 << 1)
#define	DP_SYNC_DELAY_VSYNC_FRAM	(1 << 2)
#define	DP_SYNC_DELAY_DE_CP			(1 << 3)

struct dp_ctrl_info {
	/* clock gen */
	int clk_src_lv0;
	int clk_div_lv0;
	int clk_src_lv1;
	int clk_div_lv1;
	/* scan format */
	int interlace;
	/* syncgen format */
	unsigned int out_format;
	int invert_field;	/* 0:normal(Low odd), 1:invert (low even) */
	int swap_RB;
	unsigned int yc_order;	/* for CCIR output */
	/* extern sync delay */
	int delay_mask;		/* if 0, set defalut delays */
	int d_rgb_pvd;		/* delay for RGB/PVD, 0~16, default  0 */
	int d_hsync_cp1;	/* delay for HSYNC/CP1, 0~63, default 12 */
	int d_vsync_fram;	/* delay for VSYNC/FRAM, 0~63, default 12 */
	int d_de_cp2;		/* delay for DE/CP2, 0~63, default 12 */
	/* sync offset */
	int vs_start_offset;	/* start vsync offset, defatult 0 */
	int vs_end_offset;	/* end vsync offset, default 0 */
	int ev_start_offset;	/* start even vsync offset, default 0 */
	int ev_end_offset;	/* end even vsync offset , default 0 */
	/* pad clock seletor */
	int vck_select;		/* 0=vclk0, 1=vclk2 */
	int clk_inv_lv0;	/* OUTCLKINVn */
	int clk_delay_lv0;	/* OUTCLKDELAYn */
	int clk_inv_lv1;	/* OUTCLKINVn */
	int clk_delay_lv1;	/* OUTCLKDELAYn */
	int clk_sel_div1;	/* 0=clk1_inv, 1=clk1_div_2_ns */
};

/* multi layer control (MLC) */
struct dp_plane_top {
	int screen_width;
	int screen_height;
	int video_prior;	/* 0: video>RGBn, 1: RGB0>video>RGB1,
				 *                2: RGB0 > RGB1 > video .. */
	int interlace;
	int plane_num;
	unsigned int back_color;
};

struct dp_plane_info {
	int layer;
	unsigned int fb_base;
	int left;
	int top;
	int width;
	int height;
	int pixel_byte;
	unsigned int format;
	int alpha_on;
	int alpha_depth;
	int tp_on;			/* transparency color enable */
	unsigned int tp_color;
	unsigned int mem_lock_size;	/* memory burst access (4,8,16) */
	int video_layer;
	int enable;
};

/*
 * LCD device dependency struct
 * RGB, LVDS, MiPi, HDMI
 */
enum {
	DP_DEVICE_RESCONV = 0,
	DP_DEVICE_RGBLCD = 1,
	DP_DEVICE_HDMI = 2,
	DP_DEVICE_MIPI = 3,
	DP_DEVICE_LVDS = 4,
	DP_DEVICE_CVBS = 5,
	DP_DEVICE_DP0 = 6,
	DP_DEVICE_DP1 = 7,
	DP_DEVICE_END,
};

enum {
	DP_CLOCK_RESCONV = 0,
	DP_CLOCK_LCDIF = 1,
	DP_CLOCK_MIPI = 2,
	DP_CLOCK_LVDS = 3,
	DP_CLOCK_HDMI = 4,
	DP_CLOCK_END,
};

enum dp_lvds_format {
	DP_LVDS_FORMAT_VESA = 0,
	DP_LVDS_FORMAT_JEIDA = 1,
	DP_LVDS_FORMAT_LOC = 2,
};

#define	DEF_VOLTAGE_LEVEL	(0x20)

struct dp_lvds_dev {
	enum dp_lvds_format lvds_format; /* 0:VESA, 1:JEIDA, 2: Location */
	int pol_inv_hs;		/* hsync polarity invert for VESA, JEIDA */
	int pol_inv_vs;		/* bsync polarity invert for VESA, JEIDA */
	int pol_inv_de;		/* de polarity invert for VESA, JEIDA */
	int pol_inv_ck;		/* input clock(pixel clock) polarity invert */
	int voltage_level;
	/* Location setting */
	unsigned int loc_map[9];	/* Location Setting */
	unsigned int loc_mask[2];	/* Location Setting, 0 ~ 34 */
	unsigned int loc_pol[2];	/* Location Setting, 0 ~ 34 */
};

#include "mipi_display.h"

struct dp_mipi_dev {
	int lp_bitrate;	/* to lcd setup, low power bitrate (150, 100, 80 Mhz) */
	int hs_bitrate; /* to lcd data, high speed bitrate (1000, ... Mhz) */
	int lpm_trans;
	int command_mode;
	unsigned int hs_pllpms;
	unsigned int hs_bandctl;
	unsigned int lp_pllpms;
	unsigned int lp_bandctl;
	struct mipi_dsi_device dsi;
};

struct dp_rgb_dev {
	int lcd_mpu_type;
};

struct dp_hdmi_dev {
	int preset;
};

/* platform data for the driver model */
struct nx_display_plat {
	int module;
	struct dp_sync_info sync;
	struct dp_ctrl_info ctrl;
	struct dp_plane_top top;
	struct dp_plane_info plane[DP_PLANS_NUM];
	int dev_type;
	void *device;
};

/* Lcd api */
void nx_lvds_display(int module,
		     struct dp_sync_info *sync, struct dp_ctrl_info *ctrl,
		     struct dp_plane_top *top,
		     struct dp_plane_info *planes,
		     struct dp_lvds_dev *dev);

void nx_rgb_display(int module,
		    struct dp_sync_info *sync, struct dp_ctrl_info *ctrl,
		    struct dp_plane_top *top, struct dp_plane_info *planes,
		    struct dp_rgb_dev *dev);

void nx_hdmi_display(int module,
		     struct dp_sync_info *sync, struct dp_ctrl_info *ctrl,
		     struct dp_plane_top *top,
		     struct dp_plane_info *planes,
		     struct dp_hdmi_dev *dev);

void nx_mipi_display(int module,
		     struct dp_sync_info *sync, struct dp_ctrl_info *ctrl,
		     struct dp_plane_top *top,
		     struct dp_plane_info *planes,
		     struct dp_mipi_dev *dev);

int nx_mipi_dsi_lcd_bind(struct mipi_dsi_device *dsi);

/* disaply api */
void dp_control_init(int module);
int  dp_control_setup(int module, struct dp_sync_info *sync,
		      struct dp_ctrl_info *ctrl);
void dp_control_enable(int module, int on);

void dp_plane_init(int module);
int  dp_plane_screen_setup(int module, struct dp_plane_top *top);
void dp_plane_screen_enable(int module, int on);

int  dp_plane_layer_setup(int module, struct dp_plane_info *plane);
void dp_plane_layer_enable(int module, struct dp_plane_info *plane, int on);

int dp_plane_set_enable(int module, int layer, int on);
int dp_plane_set_address(int module, int layer, unsigned int address);
int dp_plane_wait_vsync(int module, int layer, int fps);

#if defined CONFIG_SPL_BUILD ||	\
	(!defined(CONFIG_DM) && !defined(CONFIG_OF_CONTROL))
int nx_display_probe(struct nx_display_plat *plat);
#endif

#endif
