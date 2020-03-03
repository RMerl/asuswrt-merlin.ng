/*
 * FB driver for the ILI9163 LCD Controller
 *
 * Copyright (C) 2015 Kozhevnikov Anatoly
 *
 * Based on ili9325.c by Noralf Tronnes and
 * .S.U.M.O.T.O.Y. by Max MC Costa (https://github.com/sumotoy/TFT_ILI9163C).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include "fbtft.h"

#define DRVNAME		"fb_ili9163"
#define WIDTH		128
#define HEIGHT		128
#define BPP		16
#define FPS		30

#ifdef GAMMA_ADJ
#define GAMMA_LEN	15
#define GAMMA_NUM	1
#define DEFAULT_GAMMA	"36 29 12 22 1C 15 42 B7 2F 13 12 0A 11 0B 06\n"
#endif

/* ILI9163C commands */
#define CMD_NOP		0x00 /* Non operation*/
#define CMD_SWRESET	0x01 /* Soft Reset */
#define CMD_SLPIN	0x10 /* Sleep ON */
#define CMD_SLPOUT	0x11 /* Sleep OFF */
#define CMD_PTLON	0x12 /* Partial Mode ON */
#define CMD_NORML	0x13 /* Normal Display ON */
#define CMD_DINVOF	0x20 /* Display Inversion OFF */
#define CMD_DINVON	0x21 /* Display Inversion ON */
#define CMD_GAMMASET	0x26 /* Gamma Set (0x01[1],0x02[2],0x04[3],0x08[4]) */
#define CMD_DISPOFF	0x28 /* Display OFF */
#define CMD_DISPON	0x29 /* Display ON */
#define CMD_IDLEON	0x39 /* Idle Mode ON */
#define CMD_IDLEOF	0x38 /* Idle Mode OFF */
#define CMD_CLMADRS	0x2A /* Column Address Set */
#define CMD_PGEADRS	0x2B /* Page Address Set */

#define CMD_RAMWR	0x2C /* Memory Write */
#define CMD_RAMRD	0x2E /* Memory Read */
#define CMD_CLRSPACE	0x2D /* Color Space : 4K/65K/262K */
#define CMD_PARTAREA	0x30 /* Partial Area */
#define CMD_VSCLLDEF	0x33 /* Vertical Scroll Definition */
#define CMD_TEFXLON	0x34 /* Tearing Effect Line ON */
#define CMD_TEFXLOF	0x35 /* Tearing Effect Line OFF */
#define CMD_MADCTL	0x36 /* Memory Access Control */

#define CMD_PIXFMT	0x3A /* Interface Pixel Format */
#define CMD_FRMCTR1	0xB1 /* Frame Rate Control
				(In normal mode/Full colors) */
#define CMD_FRMCTR2	0xB2 /* Frame Rate Control (In Idle mode/8-colors) */
#define CMD_FRMCTR3	0xB3 /* Frame Rate Control
				(In Partial mode/full colors) */
#define CMD_DINVCTR	0xB4 /* Display Inversion Control */
#define CMD_RGBBLK	0xB5 /* RGB Interface Blanking Porch setting */
#define CMD_DFUNCTR	0xB6 /* Display Function set 5 */
#define CMD_SDRVDIR	0xB7 /* Source Driver Direction Control */
#define CMD_GDRVDIR	0xB8 /* Gate Driver Direction Control  */

#define CMD_PWCTR1	0xC0 /* Power_Control1 */
#define CMD_PWCTR2	0xC1 /* Power_Control2 */
#define CMD_PWCTR3	0xC2 /* Power_Control3 */
#define CMD_PWCTR4	0xC3 /* Power_Control4 */
#define CMD_PWCTR5	0xC4 /* Power_Control5 */
#define CMD_VCOMCTR1	0xC5 /* VCOM_Control 1 */
#define CMD_VCOMCTR2	0xC6 /* VCOM_Control 2 */
#define CMD_VCOMOFFS	0xC7 /* VCOM Offset Control */
#define CMD_PGAMMAC	0xE0 /* Positive Gamma Correction Setting */
#define CMD_NGAMMAC	0xE1 /* Negative Gamma Correction Setting */
#define CMD_GAMRSEL	0xF2 /* GAM_R_SEL */

/*
This display:
http://www.ebay.com/itm/Replace-Nokia-5110-LCD-1-44-Red-Serial-128X128-SPI-Color-TFT-LCD-Display-Module-/271422122271
This particular display has a design error! The controller has 3 pins to
configure to constrain the memory and resolution to a fixed dimension (in
that case 128x128) but they leaved those pins configured for 128x160 so
there was several pixel memory addressing problems.
I solved by setup several parameters that dinamically fix the resolution as
needit so below the parameters for this display. If you have a strain or a
correct display (can happen with chinese) you can copy those parameters and
create setup for different displays.
*/

#ifdef RED
#define __OFFSET		32 /*see note 2 - this is the red version */
#else
#define __OFFSET		0  /*see note 2 - this is the black version */
#endif

static int init_display(struct fbtft_par *par)
{
	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	par->fbtftops.reset(par);

	if (par->gpio.cs != -1)
		gpio_set_value(par->gpio.cs, 0);  /* Activate chip */

	write_reg(par, CMD_SWRESET); /* software reset */
	mdelay(500);
	write_reg(par, CMD_SLPOUT); /* exit sleep */
	mdelay(5);
	write_reg(par, CMD_PIXFMT, 0x05); /* Set Color Format 16bit */
	write_reg(par, CMD_GAMMASET, 0x02); /* default gamma curve 3 */
#ifdef GAMMA_ADJ
	write_reg(par, CMD_GAMRSEL, 0x01); /* Enable Gamma adj */
#endif
	write_reg(par, CMD_NORML);
	write_reg(par, CMD_DFUNCTR, 0xff, 0x06);
	/* Frame Rate Control (In normal mode/Full colors) */
	write_reg(par, CMD_FRMCTR1, 0x08, 0x02);
	write_reg(par, CMD_DINVCTR, 0x07); /* display inversion  */
	/* Set VRH1[4:0] & VC[2:0] for VCI1 & GVDD */
	write_reg(par, CMD_PWCTR1, 0x0A, 0x02);
	/* Set BT[2:0] for AVDD & VCL & VGH & VGL  */
	write_reg(par, CMD_PWCTR2, 0x02);
	/* Set VMH[6:0] & VML[6:0] for VOMH & VCOML */
	write_reg(par, CMD_VCOMCTR1, 0x50, 0x63);
	write_reg(par, CMD_VCOMOFFS, 0);

	write_reg(par, CMD_CLMADRS, 0, 0, 0, WIDTH); /* Set Column Address */
	write_reg(par, CMD_PGEADRS, 0, 0, 0, HEIGHT); /* Set Page Address */

	write_reg(par, CMD_DISPON); /* display ON */
	write_reg(par, CMD_RAMWR); /* Memory Write */

	return 0;
}

static void set_addr_win(struct fbtft_par *par, int xs, int ys,
				int xe, int ye)
{
	fbtft_par_dbg(DEBUG_SET_ADDR_WIN, par,
		"%s(xs=%d, ys=%d, xe=%d, ye=%d)\n", __func__, xs, ys, xe, ye);

	switch (par->info->var.rotate) {
	case 0:
		write_reg(par, CMD_CLMADRS, xs >> 8, xs & 0xff, xe >> 8,
				xe & 0xff);
		write_reg(par, CMD_PGEADRS,
				(ys + __OFFSET) >> 8, (ys + __OFFSET) & 0xff,
				(ye + __OFFSET) >> 8, (ye + __OFFSET) & 0xff);
		break;
	case 90:
		write_reg(par, CMD_CLMADRS,
				(xs + __OFFSET) >> 8, (xs + __OFFSET) & 0xff,
				(xe + __OFFSET) >> 8, (xe + __OFFSET) & 0xff);
		write_reg(par, CMD_PGEADRS, ys >> 8, ys & 0xff, ye >> 8,
				ye & 0xff);
		break;
	case 180:
	case 270:
		write_reg(par, CMD_CLMADRS, xs >> 8, xs & 0xff, xe >> 8,
				xe & 0xff);
		write_reg(par, CMD_PGEADRS, ys >> 8, ys & 0xff, ye >> 8,
				ye & 0xff);
		break;
	default:
		par->info->var.rotate = 0; /* Fix incorrect setting */
	}
	write_reg(par, CMD_RAMWR); /* Write Data to GRAM mode */
}

/*
7) MY:  1(bottom to top),	0(top to bottom)    Row Address Order
6) MX:  1(R to L),		0(L to R)	    Column Address Order
5) MV:  1(Exchanged),		0(normal)	    Row/Column exchange
4) ML:  1(bottom to top),	0(top to bottom)    Vertical Refresh Order
3) RGB: 1(BGR),			0(RGB)		    Color Space
2) MH:  1(R to L),		0(L to R)	    Horizontal Refresh Order
1)
0)

	MY, MX, MV, ML,RGB, MH, D1, D0
	0 | 0 | 0 | 0 | 1 | 0 | 0 | 0	//normal
	1 | 0 | 0 | 0 | 1 | 0 | 0 | 0	//Y-Mirror
	0 | 1 | 0 | 0 | 1 | 0 | 0 | 0	//X-Mirror
	1 | 1 | 0 | 0 | 1 | 0 | 0 | 0	//X-Y-Mirror
	0 | 0 | 1 | 0 | 1 | 0 | 0 | 0	//X-Y Exchange
	1 | 0 | 1 | 0 | 1 | 0 | 0 | 0	//X-Y Exchange, Y-Mirror
	0 | 1 | 1 | 0 | 1 | 0 | 0 | 0	//XY exchange
	1 | 1 | 1 | 0 | 1 | 0 | 0 | 0
*/
static int set_var(struct fbtft_par *par)
{
	u8 mactrl_data = 0; /* Avoid compiler warning */

	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	switch (par->info->var.rotate) {
	case 0:
		mactrl_data = 0x08;
		break;
	case 180:
		mactrl_data = 0xC8;
		break;
	case 270:
		mactrl_data = 0xA8;
		break;
	case 90:
		mactrl_data = 0x68;
		break;
	}

	/* Colorspcae */
	if (par->bgr)
		mactrl_data |= (1 << 2);
	write_reg(par, CMD_MADCTL, mactrl_data);
	write_reg(par, CMD_RAMWR); /* Write Data to GRAM mode */
	return 0;
}

#ifdef GAMMA_ADJ
#define CURVE(num, idx)  curves[num*par->gamma.num_values + idx]
static int gamma_adj(struct fbtft_par *par, unsigned long *curves)
{
	unsigned long mask[] = {
		0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
		0x1f, 0x3f, 0x0f, 0x0f, 0x7f, 0x1f,
		0x3F, 0x3F, 0x3F, 0x3F, 0x3F};
	int i, j;

	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	for (i = 0; i < GAMMA_NUM; i++)
		for (j = 0; j < GAMMA_LEN; j++)
			CURVE(i, j) &= mask[i*par->gamma.num_values + j];

	write_reg(par, CMD_PGAMMAC,
				CURVE(0, 0),
				CURVE(0, 1),
				CURVE(0, 2),
				CURVE(0, 3),
				CURVE(0, 4),
				CURVE(0, 5),
				CURVE(0, 6),
				(CURVE(0, 7) << 4) | CURVE(0, 8),
				CURVE(0, 9),
				CURVE(0, 10),
				CURVE(0, 11),
				CURVE(0, 12),
				CURVE(0, 13),
				CURVE(0, 14),
				CURVE(0, 15)
				);

	write_reg(par, CMD_RAMWR); /* Write Data to GRAM mode */

	return 0;
}
#undef CURVE
#endif

static struct fbtft_display display = {
	.regwidth = 8,
	.width = WIDTH,
	.height = HEIGHT,
	.bpp = BPP,
	.fps = FPS,
#ifdef GAMMA_ADJ
	.gamma_num = GAMMA_NUM,
	.gamma_len = GAMMA_LEN,
	.gamma = DEFAULT_GAMMA,
#endif
	.fbtftops = {
		.init_display = init_display,
		.set_addr_win = set_addr_win,
		.set_var = set_var,
#ifdef GAMMA_ADJ
		.set_gamma = gamma_adj,
#endif
	},
};

FBTFT_REGISTER_DRIVER(DRVNAME, "ilitek,ili9163", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);
MODULE_ALIAS("spi:ili9163");
MODULE_ALIAS("platform:ili9163");

MODULE_DESCRIPTION("FB driver for the ILI9163 LCD Controller");
MODULE_AUTHOR("Kozhevnikov Anatoly");
MODULE_LICENSE("GPL");
