// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 */

#include <common.h>
#include <lcd.h>
#include <video_font.h>		/* Get font data, width and height */

static void lcd_putc_xy90(struct console_t *pcons, ushort x, ushort y, char c)
{
	int fg_color = lcd_getfgcolor();
	int bg_color = lcd_getbgcolor();
	int col, i;

	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  (x+1) * pcons->lcdsizex -
				  y;

	uchar msk = 0x80;
	uchar *pfont = video_fontdata + c * VIDEO_FONT_HEIGHT;
	for (col = 0; col < VIDEO_FONT_WIDTH; ++col) {
		for (i = 0; i < VIDEO_FONT_HEIGHT; ++i)
			*dst-- = (*(pfont + i) & msk) ? fg_color : bg_color;
		msk >>= 1;
		dst += (pcons->lcdsizex + VIDEO_FONT_HEIGHT);
	}
}

static inline void console_setrow90(struct console_t *pcons, u32 row, int clr)
{
	int i, j;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  pcons->lcdsizex -
				  row*VIDEO_FONT_HEIGHT+1;

	for (j = 0; j < pcons->lcdsizey; j++) {
		for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
			*dst-- = clr;
		dst += (pcons->lcdsizex + VIDEO_FONT_HEIGHT);
	}
}

static inline void console_moverow90(struct console_t *pcons,
				      u32 rowdst, u32 rowsrc)
{
	int i, j;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  pcons->lcdsizex -
				  (rowdst*VIDEO_FONT_HEIGHT+1);

	fbptr_t *src = (fbptr_t *)pcons->fbbase +
				  pcons->lcdsizex -
				  (rowsrc*VIDEO_FONT_HEIGHT+1);

	for (j = 0; j < pcons->lcdsizey; j++) {
		for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
			*dst-- = *src--;
		src += (pcons->lcdsizex + VIDEO_FONT_HEIGHT);
		dst += (pcons->lcdsizex + VIDEO_FONT_HEIGHT);
	}
}
static void lcd_putc_xy180(struct console_t *pcons, ushort x, ushort y, char c)
{
	int fg_color = lcd_getfgcolor();
	int bg_color = lcd_getbgcolor();
	int i, row;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  pcons->lcdsizex +
				  pcons->lcdsizey * pcons->lcdsizex -
				  y * pcons->lcdsizex -
				  (x+1);

	for (row = 0; row < VIDEO_FONT_HEIGHT; row++) {
		uchar bits = video_fontdata[c * VIDEO_FONT_HEIGHT + row];

		for (i = 0; i < VIDEO_FONT_WIDTH; ++i) {
			*dst-- = (bits & 0x80) ? fg_color : bg_color;
			bits <<= 1;
		}
		dst -= (pcons->lcdsizex - VIDEO_FONT_WIDTH);
	}
}

static inline void console_setrow180(struct console_t *pcons, u32 row, int clr)
{
	int i;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  (pcons->rows-row-1) * VIDEO_FONT_HEIGHT *
				  pcons->lcdsizex;

	for (i = 0; i < (VIDEO_FONT_HEIGHT * pcons->lcdsizex); i++)
		*dst++ = clr;
}

static inline void console_moverow180(struct console_t *pcons,
				      u32 rowdst, u32 rowsrc)
{
	int i;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  (pcons->rows-rowdst-1) * VIDEO_FONT_HEIGHT *
				  pcons->lcdsizex;

	fbptr_t *src = (fbptr_t *)pcons->fbbase +
				  (pcons->rows-rowsrc-1) * VIDEO_FONT_HEIGHT *
				  pcons->lcdsizex;

	for (i = 0; i < (VIDEO_FONT_HEIGHT * pcons->lcdsizex); i++)
		*dst++ = *src++;
}

static void lcd_putc_xy270(struct console_t *pcons, ushort x, ushort y, char c)
{
	int fg_color = lcd_getfgcolor();
	int bg_color = lcd_getbgcolor();
	int i, col;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  pcons->lcdsizey * pcons->lcdsizex -
				  (x+1) * pcons->lcdsizex +
				  y;

	uchar msk = 0x80;
	uchar *pfont = video_fontdata + c * VIDEO_FONT_HEIGHT;
	for (col = 0; col < VIDEO_FONT_WIDTH; ++col) {
		for (i = 0; i < VIDEO_FONT_HEIGHT; ++i)
			*dst++ = (*(pfont + i) & msk) ? fg_color : bg_color;
		msk >>= 1;
		dst -= (pcons->lcdsizex + VIDEO_FONT_HEIGHT);
	}
}

static inline void console_setrow270(struct console_t *pcons, u32 row, int clr)
{
	int i, j;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  row*VIDEO_FONT_HEIGHT;

	for (j = 0; j < pcons->lcdsizey; j++) {
		for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
			*dst++ = clr;
		dst += (pcons->lcdsizex - VIDEO_FONT_HEIGHT);
	}
}

static inline void console_moverow270(struct console_t *pcons,
				     u32 rowdst, u32 rowsrc)
{
	int i, j;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  rowdst*VIDEO_FONT_HEIGHT;

	fbptr_t *src = (fbptr_t *)pcons->fbbase +
				  rowsrc*VIDEO_FONT_HEIGHT;

	for (j = 0; j < pcons->lcdsizey; j++) {
		for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
			*dst++ = *src++;
		src += (pcons->lcdsizex - VIDEO_FONT_HEIGHT);
		dst += (pcons->lcdsizex - VIDEO_FONT_HEIGHT);
	}
}

static void console_calc_rowcol_rot(struct console_t *pcons)
{
	if (pcons->lcdrot == 1 || pcons->lcdrot == 3)
		console_calc_rowcol(pcons, pcons->lcdsizey, pcons->lcdsizex);
	else
		console_calc_rowcol(pcons, pcons->lcdsizex, pcons->lcdsizey);
}

void lcd_init_console_rot(struct console_t *pcons)
{
	if (pcons->lcdrot == 0) {
		return;
	} else if (pcons->lcdrot == 1) {
		pcons->fp_putc_xy = &lcd_putc_xy90;
		pcons->fp_console_moverow = &console_moverow90;
		pcons->fp_console_setrow = &console_setrow90;
	} else if (pcons->lcdrot == 2) {
		pcons->fp_putc_xy = &lcd_putc_xy180;
		pcons->fp_console_moverow = &console_moverow180;
		pcons->fp_console_setrow = &console_setrow180;
	} else if (pcons->lcdrot == 3) {
		pcons->fp_putc_xy = &lcd_putc_xy270;
		pcons->fp_console_moverow = &console_moverow270;
		pcons->fp_console_setrow = &console_setrow270;
	} else {
		printf("%s: invalid framebuffer rotation (%d)!\n",
		       __func__, pcons->lcdrot);
		return;
	}
	console_calc_rowcol_rot(pcons);
}
