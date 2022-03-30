/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * exynos_lcd.h - Exynos LCD Controller structures
 *
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _EXYNOS_LCD_H_
#define _EXYNOS_LCD_H_

enum {
	FIMD_RGB_INTERFACE = 1,
	FIMD_CPU_INTERFACE = 2,
};

enum exynos_fb_rgb_mode_t {
	MODE_RGB_P = 0,
	MODE_BGR_P = 1,
	MODE_RGB_S = 2,
	MODE_BGR_S = 3,
};

typedef struct vidinfo {
	ushort vl_col;		/* Number of columns (i.e. 640) */
	ushort vl_row;		/* Number of rows (i.e. 480) */
	ushort vl_rot;		/* Rotation of Display (0, 1, 2, 3) */
	ushort vl_width;	/* Width of display area in millimeters */
	ushort vl_height;	/* Height of display area in millimeters */

	/* LCD configuration register */
	u_char vl_freq;		/* Frequency */
	u_char vl_clkp;		/* Clock polarity */
	u_char vl_oep;		/* Output Enable polarity */
	u_char vl_hsp;		/* Horizontal Sync polarity */
	u_char vl_vsp;		/* Vertical Sync polarity */
	u_char vl_dp;		/* Data polarity */
	u_char vl_bpix;		/* Bits per pixel */

	/* Horizontal control register. Timing from data sheet */
	u_char vl_hspw;		/* Horz sync pulse width */
	u_char vl_hfpd;		/* Wait before of line */
	u_char vl_hbpd;		/* Wait end of line */

	/* Vertical control register. */
	u_char	vl_vspw;	/* Vertical sync pulse width */
	u_char	vl_vfpd;	/* Wait before of frame */
	u_char	vl_vbpd;	/* Wait end of frame */
	u_char  vl_cmd_allow_len; /* Wait end of frame */

	unsigned int win_id;
	unsigned int init_delay;
	unsigned int power_on_delay;
	unsigned int reset_delay;
	unsigned int interface_mode;
	unsigned int mipi_enabled;
	unsigned int dp_enabled;
	unsigned int cs_setup;
	unsigned int wr_setup;
	unsigned int wr_act;
	unsigned int wr_hold;
	unsigned int logo_on;
	unsigned int logo_width;
	unsigned int logo_height;
	int logo_x_offset;
	int logo_y_offset;
	unsigned long logo_addr;
	unsigned int rgb_mode;
	unsigned int resolution;

	/* parent clock name(MPLL, EPLL or VPLL) */
	unsigned int pclk_name;
	/* ratio value for source clock from parent clock. */
	unsigned int sclk_div;

	unsigned int dual_lcd_enabled;
	struct exynos_fb *reg;
	struct exynos_platform_mipi_dsim *dsim_platform_data_dt;
} vidinfo_t;

#endif
