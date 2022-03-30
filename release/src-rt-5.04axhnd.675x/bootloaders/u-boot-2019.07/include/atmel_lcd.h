/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * atmel_lcd.h - Atmel LCD Controller structures
 *
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _ATMEL_LCD_H_
#define _ATMEL_LCD_H_

/**
 * struct atmel_lcd_platdata - platform data for Atmel LCDs with driver model
 *
 * @timing_index:	Index of LCD timing to use in device tree node
 */
struct atmel_lcd_platdata {
	int timing_index;
};

typedef struct vidinfo {
	ushort vl_col;		/* Number of columns (i.e. 640) */
	ushort vl_row;		/* Number of rows (i.e. 480) */
	ushort vl_rot;		/* Rotation of Display (0, 1, 2, 3) */
	u_long vl_clk;		/* pixel clock in ps    */

	/* LCD configuration register */
	u_long vl_sync;		/* Horizontal / vertical sync */
	u_long vl_bpix;	/* Bits per pixel, 0 = 1, 1 = 2, 2 = 4, 3 = 8, 4 = 16 */
	u_long vl_tft;		/* 0 = passive, 1 = TFT */
	u_long vl_cont_pol_low;	/* contrast polarity is low */
	u_long vl_clk_pol;	/* clock polarity */

	/* Horizontal control register. */
	u_long vl_hsync_len;	/* Length of horizontal sync */
	u_long vl_left_margin;	/* Time from sync to picture */
	u_long vl_right_margin;	/* Time from picture to sync */

	/* Vertical control register. */
	u_long vl_vsync_len;	/* Length of vertical sync */
	u_long vl_upper_margin;	/* Time from sync to picture */
	u_long vl_lower_margin;	/* Time from picture to sync */

	u_long	mmio;		/* Memory mapped registers */

	u_int logo_width;
	u_int logo_height;
	int logo_x_offset;
	int logo_y_offset;
	u_long logo_addr;
} vidinfo_t;

void atmel_logo_info(vidinfo_t *info);
void microchip_logo_info(vidinfo_t *info);

#endif
