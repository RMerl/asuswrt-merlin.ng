/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * pxa_lcd.h - PXA LCD Controller structures
 *
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _PXA_LCD_H_
#define _PXA_LCD_H_

/*
 * PXA LCD DMA descriptor
 */
struct pxafb_dma_descriptor {
	u_long	fdadr;		/* Frame descriptor address register */
	u_long	fsadr;		/* Frame source address register */
	u_long	fidr;		/* Frame ID register */
	u_long	ldcmd;		/* Command register */
};

/*
 * PXA LCD info
 */
struct pxafb_info {
	/* Misc registers */
	u_long	reg_lccr3;
	u_long	reg_lccr2;
	u_long	reg_lccr1;
	u_long	reg_lccr0;
	u_long	fdadr0;
	u_long	fdadr1;

	/* DMA descriptors */
	struct	pxafb_dma_descriptor *dmadesc_fblow;
	struct	pxafb_dma_descriptor *dmadesc_fbhigh;
	struct	pxafb_dma_descriptor *dmadesc_palette;

	u_long	screen;		/* physical address of frame buffer */
	u_long	palette;	/* physical address of palette memory */
	u_int	palette_size;
};

/*
 * LCD controller stucture for PXA CPU
 */
typedef struct vidinfo {
	ushort	vl_col;		/* Number of columns (i.e. 640) */
	ushort	vl_row;		/* Number of rows (i.e. 480) */
	ushort  vl_rot;		/* Rotation of Display (0, 1, 2, 3) */
	ushort	vl_width;	/* Width of display area in millimeters */
	ushort	vl_height;	/* Height of display area in millimeters */

	/* LCD configuration register */
	u_char	vl_clkp;	/* Clock polarity */
	u_char	vl_oep;		/* Output Enable polarity */
	u_char	vl_hsp;		/* Horizontal Sync polarity */
	u_char	vl_vsp;		/* Vertical Sync polarity */
	u_char	vl_dp;		/* Data polarity */
	u_char	vl_bpix;/* Bits per pixel, 0 = 1, 1 = 2, 2 = 4, 3 = 8, 4 = 16 */
	u_char	vl_lbw;		/* LCD Bus width, 0 = 4, 1 = 8 */
	u_char	vl_splt;/* Split display, 0 = single-scan, 1 = dual-scan */
	u_char	vl_clor;	/* Color, 0 = mono, 1 = color */
	u_char	vl_tft;		/* 0 = passive, 1 = TFT */

	/* Horizontal control register. Timing from data sheet */
	ushort	vl_hpw;		/* Horz sync pulse width */
	u_char	vl_blw;		/* Wait before of line */
	u_char	vl_elw;		/* Wait end of line */

	/* Vertical control register. */
	u_char	vl_vpw;		/* Vertical sync pulse width */
	u_char	vl_bfw;		/* Wait before of frame */
	u_char	vl_efw;		/* Wait end of frame */

	/* PXA LCD controller params */
	struct	pxafb_info pxa;
} vidinfo_t;

#endif
