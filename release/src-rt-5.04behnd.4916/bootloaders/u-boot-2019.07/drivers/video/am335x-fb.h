/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2018 Hannes Schmelzer <oe5hpm@oevsv.at> -
 * B&R Industrial Automation GmbH - http://www.br-automation.com
 */

#ifndef AM335X_FB_H
#define AM335X_FB_H

#define HSVS_CONTROL	(0x01 << 25)	/*
					 * 0 = lcd_lp and lcd_fp are driven on
					 * opposite edges of pixel clock than
					 * the lcd_pixel_o
					 * 1 = lcd_lp and lcd_fp are driven
					 * according to bit 24 Note that this
					 * bit MUST be set to '0' for Passive
					 * Matrix displays the edge timing is
					 * fixed
					 */
#define HSVS_RISEFALL	(0x01 << 24)	/*
					 * 0 = lcd_lp and lcd_fp are driven on
					 * the rising edge of pixel clock (bit
					 * 25 must be set to 1)
					 * 1 = lcd_lp and lcd_fp are driven on
					 * the falling edge of pixel clock (bit
					 * 25 must be set to 1)
					 */
#define DE_INVERT	(0x01 << 23)	/*
					 * 0 = DE is low-active
					 * 1 = DE is high-active
					 */
#define PXCLK_INVERT	(0x01 << 22)	/*
					 * 0 = pix-clk is high-active
					 * 1 = pic-clk is low-active
					 */
#define HSYNC_INVERT	(0x01 << 21)	/*
					 * 0 = HSYNC is active high
					 * 1 = HSYNC is avtive low
					 */
#define VSYNC_INVERT	(0x01 << 20)	/*
					 * 0 = VSYNC is active high
					 * 1 = VSYNC is active low
					 */

struct am335x_lcdpanel {
	unsigned int	hactive;	/* Horizontal active area */
	unsigned int	vactive;	/* Vertical active area */
	unsigned int	bpp;		/* bits per pixel */
	unsigned int	hfp;		/* Horizontal front porch */
	unsigned int	hbp;		/* Horizontal back porch */
	unsigned int	hsw;		/* Horizontal Sync Pulse Width */
	unsigned int	vfp;		/* Vertical front porch */
	unsigned int	vbp;		/* Vertical back porch */
	unsigned int	vsw;		/* Vertical Sync Pulse Width */
	unsigned int	pxl_clk;	/* Pixel clock */
	unsigned int	pol;		/* polarity of sync, clock signals */
	unsigned int	pup_delay;	/*
					 * time in ms after power on to
					 * initialization of lcd-controller
					 * (VCC ramp up time)
					 */
	unsigned int	pon_delay;	/*
					 * time in ms after initialization of
					 * lcd-controller (pic stabilization)
					 */
	void (*panel_power_ctrl)(int);	/* fp for power on/off display */
};

int am335xfb_init(struct am335x_lcdpanel *panel);

#endif  /* AM335X_FB_H */
