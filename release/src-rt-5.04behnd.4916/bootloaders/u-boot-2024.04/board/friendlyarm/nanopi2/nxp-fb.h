/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (c) 2017 FriendlyARM (www.arm9.net)
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * Header file for NXP Display Driver
 */

#ifndef __MACH_NXP_FB_H__
#define __MACH_NXP_FB_H__

/*
 * struct nxp_lcd_polarity
 * @rise_vclk:	if 1, video data is fetched at rising edge
 * @inv_hsync:	if HSYNC polarity is inversed
 * @inv_vsync:	if VSYNC polarity is inversed
 * @inv_vden:	if VDEN polarity is inversed
 */
struct nxp_lcd_polarity {
	int	rise_vclk;
	int	inv_hsync;
	int	inv_vsync;
	int	inv_vden;
};

/*
 * struct nxp_lcd_timing
 * @h_fp:	horizontal front porch
 * @h_bp:	horizontal back porch
 * @h_sw:	horizontal sync width
 * @v_fp:	vertical front porch
 * @v_fpe:	vertical front porch for even field
 * @v_bp:	vertical back porch
 * @v_bpe:	vertical back porch for even field
 */
struct nxp_lcd_timing {
	int	h_fp;
	int	h_bp;
	int	h_sw;
	int	v_fp;
	int	v_fpe;
	int	v_bp;
	int	v_bpe;
	int	v_sw;
};

/*
 * struct nxp_lcd
 * @width:		horizontal resolution
 * @height:		vertical resolution
 * @p_width:	width of lcd in mm
 * @p_height:	height of lcd in mm
 * @bpp:		bits per pixel
 * @freq:		vframe frequency
 * @timing:		timing values
 * @polarity:	polarity settings
 * @gpio_init:	pointer to GPIO init function
 *
 */
struct nxp_lcd {
	int	width;
	int	height;
	int	p_width;
	int	p_height;
	int	bpp;
	int	freq;
	struct	nxp_lcd_timing timing;
	struct	nxp_lcd_polarity polarity;
	void	(*gpio_init)(void);
};

/**
 * Public interfaces
 */
enum lcd_format {
	LCD_VESA	= 0,
	LCD_JEIDA	= 1,
	LCD_LOC		= 2,

	LCD_RGB		= 4,
	LCD_HDMI	= 5,
};

extern int bd_setup_lcd_by_id(int id);
extern int bd_setup_lcd_by_name(char *name);
extern struct nxp_lcd *bd_get_lcd(void);
extern const char *bd_get_lcd_name(void);
extern int bd_get_lcd_density(void);
extern enum lcd_format bd_get_lcd_format(void);
extern int bd_fixup_lcd_fdt(void *blob, struct nxp_lcd *cfg);

#endif /* __MACH_NXP_FB_H__ */
