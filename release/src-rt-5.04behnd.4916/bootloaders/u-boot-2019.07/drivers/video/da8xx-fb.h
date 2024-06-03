/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Porting to u-boot:
 *
 * (C) Copyright 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * Copyright (C) 2008-2009 MontaVista Software Inc.
 * Copyright (C) 2008-2009 Texas Instruments Inc
 *
 * Based on the LCD driver for TI Avalanche processors written by
 * Ajay Singh and Shalom Hai.
 */

#ifndef DA8XX_FB_H
#define DA8XX_FB_H

enum panel_type {
	QVGA = 0,
	WVGA
};

enum panel_shade {
	MONOCHROME = 0,
	COLOR_ACTIVE,
	COLOR_PASSIVE,
};

enum raster_load_mode {
	LOAD_DATA = 1,
	LOAD_PALETTE,
};

struct display_panel {
	enum panel_type panel_type; /* QVGA */
	int max_bpp;
	int min_bpp;
	enum panel_shade panel_shade;
};

struct da8xx_panel {
	const char	name[25];	/* Full name <vendor>_<model> */
	unsigned short	width;
	unsigned short	height;
	int		hfp;		/* Horizontal front porch */
	int		hbp;		/* Horizontal back porch */
	int		hsw;		/* Horizontal Sync Pulse Width */
	int		vfp;		/* Vertical front porch */
	int		vbp;		/* Vertical back porch */
	int		vsw;		/* Vertical Sync Pulse Width */
	unsigned int	pxl_clk;	/* Pixel clock */
	unsigned char	invert_pxl_clk;	/* Invert Pixel clock */
};

struct da8xx_lcdc_platform_data {
	const char manu_name[10];
	void *controller_data;
	const char type[25];
	void (*panel_power_ctrl)(int);
};

struct lcd_ctrl_config {
	const struct display_panel *p_disp_panel;

	/* AC Bias Pin Frequency */
	int ac_bias;

	/* AC Bias Pin Transitions per Interrupt */
	int ac_bias_intrpt;

	/* DMA burst size */
	int dma_burst_sz;

	/* Bits per pixel */
	int bpp;

	/* FIFO DMA Request Delay */
	int fdd;

	/* TFT Alternative Signal Mapping (Only for active) */
	unsigned char tft_alt_mode;

	/* 12 Bit Per Pixel (5-6-5) Mode (Only for passive) */
	unsigned char stn_565_mode;

	/* Mono 8-bit Mode: 1=D0-D7 or 0=D0-D3 */
	unsigned char mono_8bit_mode;

	/* Invert line clock */
	unsigned char invert_line_clock;

	/* Invert frame clock  */
	unsigned char invert_frm_clock;

	/* Horizontal and Vertical Sync Edge: 0=rising 1=falling */
	unsigned char sync_edge;

	/* Horizontal and Vertical Sync: Control: 0=ignore */
	unsigned char sync_ctrl;

	/* Raster Data Order Select: 1=Most-to-least 0=Least-to-most */
	unsigned char raster_order;
};

struct lcd_sync_arg {
	int back_porch;
	int front_porch;
	int pulse_width;
};

void da8xx_video_init(const struct da8xx_panel *panel,
		      const struct lcd_ctrl_config *lcd_cfg,
		      int bits_pixel);

#endif  /* ifndef DA8XX_FB_H */
