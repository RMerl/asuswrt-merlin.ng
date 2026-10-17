/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2023 Texas Instruments Incorporated - https://www.ti.com/
 * Nikhil M Jain, n-jain1@ti.com
 *
 * based on the linux tidss driver, which is
 *
 * (C) Copyright 2018 Texas Instruments Incorporated - https://www.ti.com/
 * Author: Tomi Valkeinen <tomi.valkeinen@ti.com>
 */

#ifndef __TIDSS_DRV_H__
#define __TIDSS_DRV_H__

#include <media_bus_format.h>

#define TIDSS_MAX_PORTS 4
#define TIDSS_MAX_PLANES 4

enum dss_vp_bus_type {
	DSS_VP_DPI,		/* DPI output */
	DSS_VP_OLDI,		/* OLDI (LVDS) output */
	DSS_VP_INTERNAL,	/* SoC internal routing */
	DSS_VP_MAX_BUS_TYPE,
};

enum dss_oldi_modes {
	OLDI_MODE_OFF,				/* OLDI turned off / tied off in IP. */
	OLDI_SINGLE_LINK_SINGLE_MODE,		/* Single Output over OLDI 0. */
	OLDI_SINGLE_LINK_DUPLICATE_MODE,	/* Duplicate Output over OLDI 0 and 1. */
	OLDI_DUAL_LINK,				/* Combined Output over OLDI 0 and 1. */
};

struct dss_features_scaling {
	u32 in_width_max_5tap_rgb;
	u32 in_width_max_3tap_rgb;
	u32 in_width_max_5tap_yuv;
	u32 in_width_max_3tap_yuv;
	u32 upscale_limit;
	u32 downscale_limit_5tap;
	u32 downscale_limit_3tap;
	u32 xinc_max;
};

enum tidss_gamma_type { TIDSS_GAMMA_8BIT, TIDSS_GAMMA_10BIT };

/* choose specific DSS based on the board */
enum dss_subrevision {
	DSS_K2G,
	DSS_AM65X,
	DSS_J721E,
	DSS_AM625,
};

struct tidss_vp_feat {
	struct tidss_vp_color_feat {
		u32 gamma_size;
		enum tidss_gamma_type gamma_type;
		bool has_ctm;
	} color;
};

struct dss_color_lut {
	/*
	 * Data is U0.16 fixed point format.
	 */
	__u16 red;
	__u16 green;
	__u16 blue;
	__u16 reserved;
};

struct dss_vp_data {
	u32 *gamma_table;
};

struct dss_features {
	int min_pclk_khz;
	int max_pclk_khz[DSS_VP_MAX_BUS_TYPE];

	struct dss_features_scaling scaling;

	enum dss_subrevision subrev;

	const char *common;
	const u16 *common_regs;
	u32 num_vps;
	const char *vp_name[TIDSS_MAX_PORTS]; /* Should match dt reg names */
	const char *ovr_name[TIDSS_MAX_PORTS]; /* Should match dt reg names */
	const char *vpclk_name[TIDSS_MAX_PORTS]; /* Should match dt clk names */
	const enum dss_vp_bus_type vp_bus_type[TIDSS_MAX_PORTS];
	struct tidss_vp_feat vp_feat;
	u32 num_planes;
	const char *vid_name[TIDSS_MAX_PLANES]; /* Should match dt reg names */
	bool vid_lite[TIDSS_MAX_PLANES];
	u32 vid_order[TIDSS_MAX_PLANES];
};

enum dss_oldi_mode_reg_val { SPWG_18 = 0, JEIDA_24 = 1, SPWG_24 = 2 };

struct dss_bus_format {
	u32 bus_fmt;
	u32 data_width;
	bool is_oldi_fmt;
	enum dss_oldi_mode_reg_val oldi_mode_reg_val;
};

static struct dss_bus_format dss_bus_formats[] = {
	{ MEDIA_BUS_FMT_RGB444_1X12,		12, false, 0 },
	{ MEDIA_BUS_FMT_RGB565_1X16,		16, false, 0 },
	{ MEDIA_BUS_FMT_RGB666_1X18,		18, false, 0 },
	{ MEDIA_BUS_FMT_RGB888_1X24,		24, false, 0 },
	{ MEDIA_BUS_FMT_RGB101010_1X30,		30, false, 0 },
	{ MEDIA_BUS_FMT_RGB121212_1X36,		36, false, 0 },
	{ MEDIA_BUS_FMT_RGB666_1X7X3_SPWG,	18, true, SPWG_18 },
	{ MEDIA_BUS_FMT_RGB888_1X7X4_SPWG,	24, true, SPWG_24 },
	{ MEDIA_BUS_FMT_RGB888_1X7X4_JEIDA,	24, true, JEIDA_24 },
};

struct tidss_drv_priv {
	struct udevice *dev;
	void __iomem *base_common; /* common register region of dss*/
	void __iomem *base_vid[TIDSS_MAX_PLANES]; /* plane register region of dss*/
	void __iomem *base_ovr[TIDSS_MAX_PORTS]; /* overlay register region of dss*/
	void __iomem *base_vp[TIDSS_MAX_PORTS]; /* video port register region of dss*/
	struct regmap *oldi_io_ctrl;
	struct clk vp_clk[TIDSS_MAX_PORTS];
	const struct dss_features *feat;
	struct clk fclk;
	struct dss_vp_data vp_data[TIDSS_MAX_PORTS];
	enum dss_oldi_modes oldi_mode;
	struct dss_bus_format *bus_format;
	u32 pixel_format;
	u32 memory_bandwidth_limit;
};

#endif
