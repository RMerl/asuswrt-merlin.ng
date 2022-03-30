// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 */

/*
 * cfb_console.c
 *
 * Color Framebuffer Console driver for 8/15/16/24/32 bits per pixel.
 *
 * At the moment only the 8x16 font is tested and the font fore- and
 * background color is limited to black/white/gray colors. The Linux
 * logo can be placed in the upper left corner and additional board
 * information strings (that normally goes to serial port) can be drawn.
 *
 * The console driver can use a keyboard interface for character input
 * but this is deprecated. Only rk51 uses it.
 *
 * Character output goes to a memory-mapped video
 * framebuffer with little or big-endian organisation.
 * With environment setting 'console=serial' the console i/o can be
 * forced to serial port.
 *
 * The driver uses graphic specific defines/parameters/functions:
 *
 * (for SMI LynxE graphic chip)
 *
 * VIDEO_FB_LITTLE_ENDIAN     - framebuffer organisation default: big endian
 * VIDEO_HW_RECTFILL	      - graphic driver supports hardware rectangle fill
 * VIDEO_HW_BITBLT	      - graphic driver supports hardware bit blt
 *
 * Console Parameters are set by graphic drivers global struct:
 *
 * VIDEO_VISIBLE_COLS	      - x resolution
 * VIDEO_VISIBLE_ROWS	      - y resolution
 * VIDEO_PIXEL_SIZE	      - storage size in byte per pixel
 * VIDEO_DATA_FORMAT	      - graphical data format GDF
 * VIDEO_FB_ADRS	      - start of video memory
 *
 * VIDEO_KBD_INIT_FCT	      - init function for keyboard
 * VIDEO_TSTC_FCT	      - keyboard_tstc function
 * VIDEO_GETC_FCT	      - keyboard_getc function
 *
 * CONFIG_VIDEO_LOGO	      - display Linux Logo in upper left corner.
 *				Use CONFIG_SPLASH_SCREEN_ALIGN with
 *				environment variable "splashpos" to place
 *				the logo on other position. In this case
 *				no CONSOLE_EXTRA_INFO is possible.
 * CONFIG_VIDEO_BMP_LOGO      - use bmp_logo instead of linux_logo
 * CONFIG_CONSOLE_EXTRA_INFO  - display additional board information
 *				strings that normaly goes to serial
 *				port.  This define requires a board
 *				specific function:
 *				video_drawstring (VIDEO_INFO_X,
 *					VIDEO_INFO_Y + i*VIDEO_FONT_HEIGHT,
 *					info);
 *				that fills a info buffer at i=row.
 *				s.a: board/eltec/bab7xx.
 *
 * CONFIG_VIDEO_SW_CURSOR:    - Draws a cursor after the last
 *				character. No blinking is provided.
 *				Uses the macros CURSOR_SET and
 *				CURSOR_OFF.
 */

#include <common.h>
#include <fdtdec.h>
#include <version.h>
#include <malloc.h>
#include <video.h>
#include <linux/compiler.h>

#if defined(CONFIG_VIDEO_MXS)
#define VIDEO_FB_16BPP_WORD_SWAP
#endif

/*
 * Defines for the MB862xx driver
 */
#ifdef CONFIG_VIDEO_MB862xx

#ifdef CONFIG_VIDEO_CORALP
#define VIDEO_FB_LITTLE_ENDIAN
#endif
#ifdef CONFIG_VIDEO_MB862xx_ACCEL
#define VIDEO_HW_RECTFILL
#define VIDEO_HW_BITBLT
#endif
#endif

/*
 * Defines for the i.MX31 driver (mx3fb.c)
 */
#if defined(CONFIG_VIDEO_MX3) || defined(CONFIG_VIDEO_IPUV3)
#define VIDEO_FB_16BPP_WORD_SWAP
#endif

/*
 * Include video_fb.h after definitions of VIDEO_HW_RECTFILL etc.
 */
#include <video_fb.h>

#include <splash.h>

/*
 * some Macros
 */
#define VIDEO_VISIBLE_COLS	(pGD->winSizeX)
#define VIDEO_VISIBLE_ROWS	(pGD->winSizeY)
#define VIDEO_PIXEL_SIZE	(pGD->gdfBytesPP)
#define VIDEO_DATA_FORMAT	(pGD->gdfIndex)
#define VIDEO_FB_ADRS		(pGD->frameAdrs)

/*
 * Console device
 */

#include <version.h>
#include <linux/types.h>
#include <stdio_dev.h>
#include <video_font.h>

#if defined(CONFIG_CMD_DATE)
#include <rtc.h>
#endif

#if defined(CONFIG_CMD_BMP) || defined(CONFIG_SPLASH_SCREEN)
#include <watchdog.h>
#include <bmp_layout.h>
#include <splash.h>
#endif

#if !defined(CONFIG_VIDEO_SW_CURSOR)
/* no Cursor defined */
#define CURSOR_ON
#define CURSOR_OFF
#define CURSOR_SET
#endif

#if defined(CONFIG_VIDEO_SW_CURSOR)
void console_cursor(int state);

#define CURSOR_ON  console_cursor(1)
#define CURSOR_OFF console_cursor(0)
#define CURSOR_SET video_set_cursor()
#endif /* CONFIG_VIDEO_SW_CURSOR */

#ifdef	CONFIG_VIDEO_LOGO
#ifdef	CONFIG_VIDEO_BMP_LOGO
#include <bmp_logo.h>
#include <bmp_logo_data.h>
#define VIDEO_LOGO_WIDTH	BMP_LOGO_WIDTH
#define VIDEO_LOGO_HEIGHT	BMP_LOGO_HEIGHT
#define VIDEO_LOGO_LUT_OFFSET	BMP_LOGO_OFFSET
#define VIDEO_LOGO_COLORS	BMP_LOGO_COLORS

#else  /* CONFIG_VIDEO_BMP_LOGO */
#define LINUX_LOGO_WIDTH	80
#define LINUX_LOGO_HEIGHT	80
#define LINUX_LOGO_COLORS	214
#define LINUX_LOGO_LUT_OFFSET	0x20
#define __initdata
#include <linux_logo.h>
#define VIDEO_LOGO_WIDTH	LINUX_LOGO_WIDTH
#define VIDEO_LOGO_HEIGHT	LINUX_LOGO_HEIGHT
#define VIDEO_LOGO_LUT_OFFSET	LINUX_LOGO_LUT_OFFSET
#define VIDEO_LOGO_COLORS	LINUX_LOGO_COLORS
#endif /* CONFIG_VIDEO_BMP_LOGO */
#define VIDEO_INFO_X		(VIDEO_LOGO_WIDTH)
#define VIDEO_INFO_Y		(VIDEO_FONT_HEIGHT/2)
#else  /* CONFIG_VIDEO_LOGO */
#define VIDEO_LOGO_WIDTH	0
#define VIDEO_LOGO_HEIGHT	0
#endif /* CONFIG_VIDEO_LOGO */

#define VIDEO_COLS		VIDEO_VISIBLE_COLS
#define VIDEO_ROWS		VIDEO_VISIBLE_ROWS
#ifndef VIDEO_LINE_LEN
#define VIDEO_LINE_LEN		(VIDEO_COLS * VIDEO_PIXEL_SIZE)
#endif
#define VIDEO_SIZE		(VIDEO_ROWS * VIDEO_LINE_LEN)
#define VIDEO_BURST_LEN		(VIDEO_COLS/8)

#ifdef	CONFIG_VIDEO_LOGO
#define CONSOLE_ROWS		((VIDEO_ROWS - video_logo_height) / VIDEO_FONT_HEIGHT)
#else
#define CONSOLE_ROWS		(VIDEO_ROWS / VIDEO_FONT_HEIGHT)
#endif

#define CONSOLE_COLS		(VIDEO_COLS / VIDEO_FONT_WIDTH)
#define CONSOLE_ROW_SIZE	(VIDEO_FONT_HEIGHT * VIDEO_LINE_LEN)
#define CONSOLE_ROW_FIRST	(video_console_address)
#define CONSOLE_ROW_SECOND	(video_console_address + CONSOLE_ROW_SIZE)
#define CONSOLE_ROW_LAST	(video_console_address + CONSOLE_SIZE - CONSOLE_ROW_SIZE)
#define CONSOLE_SIZE		(CONSOLE_ROW_SIZE * CONSOLE_ROWS)

/* By default we scroll by a single line */
#ifndef CONFIG_CONSOLE_SCROLL_LINES
#define CONFIG_CONSOLE_SCROLL_LINES 1
#endif

/* Macros */
#ifdef	VIDEO_FB_LITTLE_ENDIAN
#define SWAP16(x)		((((x) & 0x00ff) << 8) | \
				  ((x) >> 8) \
				)
#define SWAP32(x)		((((x) & 0x000000ff) << 24) | \
				 (((x) & 0x0000ff00) <<  8) | \
				 (((x) & 0x00ff0000) >>  8) | \
				 (((x) & 0xff000000) >> 24)   \
				)
#define SHORTSWAP32(x)		((((x) & 0x000000ff) <<  8) | \
				 (((x) & 0x0000ff00) >>  8) | \
				 (((x) & 0x00ff0000) <<  8) | \
				 (((x) & 0xff000000) >>  8)   \
				)
#else
#define SWAP16(x)		(x)
#define SWAP32(x)		(x)
#if defined(VIDEO_FB_16BPP_WORD_SWAP)
#define SHORTSWAP32(x)		(((x) >> 16) | ((x) << 16))
#else
#define SHORTSWAP32(x)		(x)
#endif
#endif

DECLARE_GLOBAL_DATA_PTR;

/* Locals */
static GraphicDevice *pGD;	/* Pointer to Graphic array */

static void *video_fb_address;	/* frame buffer address */
static void *video_console_address;	/* console buffer start address */

static int video_logo_height = VIDEO_LOGO_HEIGHT;

static int __maybe_unused cursor_state;
static int __maybe_unused old_col;
static int __maybe_unused old_row;

static int console_col;		/* cursor col */
static int console_row;		/* cursor row */

static u32 eorx, fgx, bgx;	/* color pats */

static int cfb_do_flush_cache;

#ifdef CONFIG_CFB_CONSOLE_ANSI
static char ansi_buf[10];
static int ansi_buf_size;
static int ansi_colors_need_revert;
static int ansi_cursor_hidden;
#endif

static const int video_font_draw_table8[] = {
	0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff,
	0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff,
	0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
	0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff
};

static const int video_font_draw_table15[] = {
	0x00000000, 0x00007fff, 0x7fff0000, 0x7fff7fff
};

static const int video_font_draw_table16[] = {
	0x00000000, 0x0000ffff, 0xffff0000, 0xffffffff
};

static const int video_font_draw_table24[16][3] = {
	{0x00000000, 0x00000000, 0x00000000},
	{0x00000000, 0x00000000, 0x00ffffff},
	{0x00000000, 0x0000ffff, 0xff000000},
	{0x00000000, 0x0000ffff, 0xffffffff},
	{0x000000ff, 0xffff0000, 0x00000000},
	{0x000000ff, 0xffff0000, 0x00ffffff},
	{0x000000ff, 0xffffffff, 0xff000000},
	{0x000000ff, 0xffffffff, 0xffffffff},
	{0xffffff00, 0x00000000, 0x00000000},
	{0xffffff00, 0x00000000, 0x00ffffff},
	{0xffffff00, 0x0000ffff, 0xff000000},
	{0xffffff00, 0x0000ffff, 0xffffffff},
	{0xffffffff, 0xffff0000, 0x00000000},
	{0xffffffff, 0xffff0000, 0x00ffffff},
	{0xffffffff, 0xffffffff, 0xff000000},
	{0xffffffff, 0xffffffff, 0xffffffff}
};

static const int video_font_draw_table32[16][4] = {
	{0x00000000, 0x00000000, 0x00000000, 0x00000000},
	{0x00000000, 0x00000000, 0x00000000, 0x00ffffff},
	{0x00000000, 0x00000000, 0x00ffffff, 0x00000000},
	{0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff},
	{0x00000000, 0x00ffffff, 0x00000000, 0x00000000},
	{0x00000000, 0x00ffffff, 0x00000000, 0x00ffffff},
	{0x00000000, 0x00ffffff, 0x00ffffff, 0x00000000},
	{0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff},
	{0x00ffffff, 0x00000000, 0x00000000, 0x00000000},
	{0x00ffffff, 0x00000000, 0x00000000, 0x00ffffff},
	{0x00ffffff, 0x00000000, 0x00ffffff, 0x00000000},
	{0x00ffffff, 0x00000000, 0x00ffffff, 0x00ffffff},
	{0x00ffffff, 0x00ffffff, 0x00000000, 0x00000000},
	{0x00ffffff, 0x00ffffff, 0x00000000, 0x00ffffff},
	{0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00000000},
	{0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff}
};

/*
 * Implement a weak default function for boards that optionally
 * need to skip the cfb initialization.
 */
__weak int board_cfb_skip(void)
{
	/* As default, don't skip cfb init */
	return 0;
}

static void video_drawchars(int xx, int yy, unsigned char *s, int count)
{
	u8 *cdat, *dest, *dest0;
	int rows, offset, c;

	offset = yy * VIDEO_LINE_LEN + xx * VIDEO_PIXEL_SIZE;
	dest0 = video_fb_address + offset;

	switch (VIDEO_DATA_FORMAT) {
	case GDF__8BIT_INDEX:
	case GDF__8BIT_332RGB:
		while (count--) {
			c = *s;
			cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
			     rows--; dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] =
					(video_font_draw_table8[bits >> 4] &
					 eorx) ^ bgx;

				if (VIDEO_FONT_WIDTH == 4)
					continue;

				((u32 *) dest)[1] =
					(video_font_draw_table8[bits & 15] &
					 eorx) ^ bgx;
			}
			dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
			s++;
		}
		break;

	case GDF_15BIT_555RGB:
		while (count--) {
			c = *s;
			cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
			     rows--; dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] =
					SHORTSWAP32((video_font_draw_table15
						     [bits >> 6] & eorx) ^
						    bgx);
				((u32 *) dest)[1] =
					SHORTSWAP32((video_font_draw_table15
						     [bits >> 4 & 3] & eorx) ^
						    bgx);

				if (VIDEO_FONT_WIDTH == 4)
					continue;

				((u32 *) dest)[2] =
					SHORTSWAP32((video_font_draw_table15
						     [bits >> 2 & 3] & eorx) ^
						    bgx);
				((u32 *) dest)[3] =
					SHORTSWAP32((video_font_draw_table15
						     [bits & 3] & eorx) ^
						    bgx);
			}
			dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
			s++;
		}
		break;

	case GDF_16BIT_565RGB:
		while (count--) {
			c = *s;
			cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
			     rows--; dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] =
					SHORTSWAP32((video_font_draw_table16
						     [bits >> 6] & eorx) ^
						    bgx);
				((u32 *) dest)[1] =
					SHORTSWAP32((video_font_draw_table16
						     [bits >> 4 & 3] & eorx) ^
						    bgx);

				if (VIDEO_FONT_WIDTH == 4)
					continue;

				((u32 *) dest)[2] =
					SHORTSWAP32((video_font_draw_table16
						     [bits >> 2 & 3] & eorx) ^
						    bgx);
				((u32 *) dest)[3] =
					SHORTSWAP32((video_font_draw_table16
						     [bits & 3] & eorx) ^
						    bgx);
			}
			dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
			s++;
		}
		break;

	case GDF_32BIT_X888RGB:
		while (count--) {
			c = *s;
			cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
			     rows--; dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] =
					SWAP32((video_font_draw_table32
						[bits >> 4][0] & eorx) ^ bgx);
				((u32 *) dest)[1] =
					SWAP32((video_font_draw_table32
						[bits >> 4][1] & eorx) ^ bgx);
				((u32 *) dest)[2] =
					SWAP32((video_font_draw_table32
						[bits >> 4][2] & eorx) ^ bgx);
				((u32 *) dest)[3] =
					SWAP32((video_font_draw_table32
						[bits >> 4][3] & eorx) ^ bgx);


				if (VIDEO_FONT_WIDTH == 4)
					continue;

				((u32 *) dest)[4] =
					SWAP32((video_font_draw_table32
						[bits & 15][0] & eorx) ^ bgx);
				((u32 *) dest)[5] =
					SWAP32((video_font_draw_table32
						[bits & 15][1] & eorx) ^ bgx);
				((u32 *) dest)[6] =
					SWAP32((video_font_draw_table32
						[bits & 15][2] & eorx) ^ bgx);
				((u32 *) dest)[7] =
					SWAP32((video_font_draw_table32
						[bits & 15][3] & eorx) ^ bgx);
			}
			dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
			s++;
		}
		break;

	case GDF_24BIT_888RGB:
		while (count--) {
			c = *s;
			cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
			     rows--; dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] =
					(video_font_draw_table24[bits >> 4][0]
					 & eorx) ^ bgx;
				((u32 *) dest)[1] =
					(video_font_draw_table24[bits >> 4][1]
					 & eorx) ^ bgx;
				((u32 *) dest)[2] =
					(video_font_draw_table24[bits >> 4][2]
					 & eorx) ^ bgx;

				if (VIDEO_FONT_WIDTH == 4)
					continue;

				((u32 *) dest)[3] =
					(video_font_draw_table24[bits & 15][0]
					 & eorx) ^ bgx;
				((u32 *) dest)[4] =
					(video_font_draw_table24[bits & 15][1]
					 & eorx) ^ bgx;
				((u32 *) dest)[5] =
					(video_font_draw_table24[bits & 15][2]
					 & eorx) ^ bgx;
			}
			dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
			s++;
		}
		break;
	}
}

static inline void video_drawstring(int xx, int yy, unsigned char *s)
{
	video_drawchars(xx, yy, s, strlen((char *) s));
}

static void video_putchar(int xx, int yy, unsigned char c)
{
	video_drawchars(xx, yy + video_logo_height, &c, 1);
}

#if defined(CONFIG_VIDEO_SW_CURSOR)
static void video_set_cursor(void)
{
	if (cursor_state)
		console_cursor(0);
	console_cursor(1);
}

static void video_invertchar(int xx, int yy)
{
	int firstx = xx * VIDEO_PIXEL_SIZE;
	int lastx = (xx + VIDEO_FONT_WIDTH) * VIDEO_PIXEL_SIZE;
	int firsty = yy * VIDEO_LINE_LEN;
	int lasty = (yy + VIDEO_FONT_HEIGHT) * VIDEO_LINE_LEN;
	int x, y;
	for (y = firsty; y < lasty; y += VIDEO_LINE_LEN) {
		for (x = firstx; x < lastx; x++) {
			u8 *dest = (u8 *)(video_fb_address) + x + y;
			*dest = ~*dest;
		}
	}
}

void console_cursor(int state)
{
	if (cursor_state != state) {
		if (cursor_state) {
			/* turn off the cursor */
			video_invertchar(old_col * VIDEO_FONT_WIDTH,
					 old_row * VIDEO_FONT_HEIGHT +
					 video_logo_height);
		} else {
			/* turn off the cursor and record where it is */
			video_invertchar(console_col * VIDEO_FONT_WIDTH,
					 console_row * VIDEO_FONT_HEIGHT +
					 video_logo_height);
			old_col = console_col;
			old_row = console_row;
		}
		cursor_state = state;
	}
	if (cfb_do_flush_cache)
		flush_cache(VIDEO_FB_ADRS, VIDEO_SIZE);
}
#endif

#ifndef VIDEO_HW_RECTFILL
static void memsetl(int *p, int c, int v)
{
	while (c--)
		*(p++) = v;
}
#endif

#ifndef VIDEO_HW_BITBLT
static void memcpyl(int *d, int *s, int c)
{
	while (c--)
		*(d++) = *(s++);
}
#endif

static void console_clear_line(int line, int begin, int end)
{
#ifdef VIDEO_HW_RECTFILL
	video_hw_rectfill(VIDEO_PIXEL_SIZE,		/* bytes per pixel */
			  VIDEO_FONT_WIDTH * begin,	/* dest pos x */
			  video_logo_height +
			  VIDEO_FONT_HEIGHT * line,	/* dest pos y */
			  VIDEO_FONT_WIDTH * (end - begin + 1), /* fr. width */
			  VIDEO_FONT_HEIGHT,		/* frame height */
			  bgx				/* fill color */
		);
#else
	if (begin == 0 && (end + 1) == CONSOLE_COLS) {
		memsetl(CONSOLE_ROW_FIRST +
			CONSOLE_ROW_SIZE * line,	/* offset of row */
			CONSOLE_ROW_SIZE >> 2,		/* length of row */
			bgx				/* fill color */
		);
	} else {
		void *offset;
		int i, size;

		offset = CONSOLE_ROW_FIRST +
			 CONSOLE_ROW_SIZE * line +	/* offset of row */
			 VIDEO_FONT_WIDTH *
			 VIDEO_PIXEL_SIZE * begin;	/* offset of col */
		size = VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE * (end - begin + 1);
		size >>= 2; /* length to end for memsetl() */
		/* fill at col offset of i'th line using bgx as fill color */
		for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
			memsetl(offset + i * VIDEO_LINE_LEN, size, bgx);
	}
#endif
}

static void console_scrollup(void)
{
	const int rows = CONFIG_CONSOLE_SCROLL_LINES;
	int i;

	/* copy up rows ignoring the first one */

#ifdef VIDEO_HW_BITBLT
	video_hw_bitblt(VIDEO_PIXEL_SIZE,	/* bytes per pixel */
			0,			/* source pos x */
			video_logo_height +
				VIDEO_FONT_HEIGHT * rows, /* source pos y */
			0,			/* dest pos x */
			video_logo_height,	/* dest pos y */
			VIDEO_VISIBLE_COLS,	/* frame width */
			VIDEO_VISIBLE_ROWS
			- video_logo_height
			- VIDEO_FONT_HEIGHT * rows	/* frame height */
		);
#else
	memcpyl(CONSOLE_ROW_FIRST, CONSOLE_ROW_FIRST + rows * CONSOLE_ROW_SIZE,
		(CONSOLE_SIZE - CONSOLE_ROW_SIZE * rows) >> 2);
#endif
	/* clear the last one */
	for (i = 1; i <= rows; i++)
		console_clear_line(CONSOLE_ROWS - i, 0, CONSOLE_COLS - 1);

	/* Decrement row number */
	console_row -= rows;
}

static void console_back(void)
{
	console_col--;

	if (console_col < 0) {
		console_col = CONSOLE_COLS - 1;
		console_row--;
		if (console_row < 0)
			console_row = 0;
	}
}

#ifdef CONFIG_CFB_CONSOLE_ANSI

static void console_clear(void)
{
#ifdef VIDEO_HW_RECTFILL
	video_hw_rectfill(VIDEO_PIXEL_SIZE,	/* bytes per pixel */
			  0,			/* dest pos x */
			  video_logo_height,	/* dest pos y */
			  VIDEO_VISIBLE_COLS,	/* frame width */
			  VIDEO_VISIBLE_ROWS,	/* frame height */
			  bgx			/* fill color */
	);
#else
	memsetl(CONSOLE_ROW_FIRST, CONSOLE_SIZE, bgx);
#endif
}

static void console_cursor_fix(void)
{
	if (console_row < 0)
		console_row = 0;
	if (console_row >= CONSOLE_ROWS)
		console_row = CONSOLE_ROWS - 1;
	if (console_col < 0)
		console_col = 0;
	if (console_col >= CONSOLE_COLS)
		console_col = CONSOLE_COLS - 1;
}

static void console_cursor_up(int n)
{
	console_row -= n;
	console_cursor_fix();
}

static void console_cursor_down(int n)
{
	console_row += n;
	console_cursor_fix();
}

static void console_cursor_left(int n)
{
	console_col -= n;
	console_cursor_fix();
}

static void console_cursor_right(int n)
{
	console_col += n;
	console_cursor_fix();
}

static void console_cursor_set_position(int row, int col)
{
	if (console_row != -1)
		console_row = row;
	if (console_col != -1)
		console_col = col;
	console_cursor_fix();
}

static void console_previousline(int n)
{
	/* FIXME: also scroll terminal ? */
	console_row -= n;
	console_cursor_fix();
}

static void console_swap_colors(void)
{
	eorx = fgx;
	fgx = bgx;
	bgx = eorx;
	eorx = fgx ^ bgx;
}

static inline int console_cursor_is_visible(void)
{
	return !ansi_cursor_hidden;
}
#else
static inline int console_cursor_is_visible(void)
{
	return 1;
}
#endif

static void console_newline(int n)
{
	console_row += n;
	console_col = 0;

	/* Check if we need to scroll the terminal */
	if (console_row >= CONSOLE_ROWS) {
		/* Scroll everything up */
		console_scrollup();
	}
}

static void console_cr(void)
{
	console_col = 0;
}

static void parse_putc(const char c)
{
	static int nl = 1;

	if (console_cursor_is_visible())
		CURSOR_OFF;

	switch (c) {
	case 13:		/* back to first column */
		console_cr();
		break;

	case '\n':		/* next line */
		if (console_col || nl)
			console_newline(1);
		nl = 1;
		break;

	case 9:		/* tab 8 */
		console_col |= 0x0008;
		console_col &= ~0x0007;

		if (console_col >= CONSOLE_COLS)
			console_newline(1);
		break;

	case 8:		/* backspace */
		console_back();
		break;

	case 7:		/* bell */
		break;	/* ignored */

	default:		/* draw the char */
		video_putchar(console_col * VIDEO_FONT_WIDTH,
			      console_row * VIDEO_FONT_HEIGHT, c);
		console_col++;

		/* check for newline */
		if (console_col >= CONSOLE_COLS) {
			console_newline(1);
			nl = 0;
		}
	}

	if (console_cursor_is_visible())
		CURSOR_SET;
}

static void cfb_video_putc(struct stdio_dev *dev, const char c)
{
#ifdef CONFIG_CFB_CONSOLE_ANSI
	int i;

	if (c == 27) {
		for (i = 0; i < ansi_buf_size; ++i)
			parse_putc(ansi_buf[i]);
		ansi_buf[0] = 27;
		ansi_buf_size = 1;
		return;
	}

	if (ansi_buf_size > 0) {
		/*
		 * 0 - ESC
		 * 1 - [
		 * 2 - num1
		 * 3 - ..
		 * 4 - ;
		 * 5 - num2
		 * 6 - ..
		 * - cchar
		 */
		int next = 0;

		int flush = 0;
		int fail = 0;

		int num1 = 0;
		int num2 = 0;
		int cchar = 0;

		ansi_buf[ansi_buf_size++] = c;

		if (ansi_buf_size >= sizeof(ansi_buf))
			fail = 1;

		for (i = 0; i < ansi_buf_size; ++i) {
			if (fail)
				break;

			switch (next) {
			case 0:
				if (ansi_buf[i] == 27)
					next = 1;
				else
					fail = 1;
				break;

			case 1:
				if (ansi_buf[i] == '[')
					next = 2;
				else
					fail = 1;
				break;

			case 2:
				if (ansi_buf[i] >= '0' && ansi_buf[i] <= '9') {
					num1 = ansi_buf[i]-'0';
					next = 3;
				} else if (ansi_buf[i] != '?') {
					--i;
					num1 = 1;
					next = 4;
				}
				break;

			case 3:
				if (ansi_buf[i] >= '0' && ansi_buf[i] <= '9') {
					num1 *= 10;
					num1 += ansi_buf[i]-'0';
				} else {
					--i;
					next = 4;
				}
				break;

			case 4:
				if (ansi_buf[i] != ';') {
					--i;
					next = 7;
				} else
					next = 5;
				break;

			case 5:
				if (ansi_buf[i] >= '0' && ansi_buf[i] <= '9') {
					num2 = ansi_buf[i]-'0';
					next = 6;
				} else
					fail = 1;
				break;

			case 6:
				if (ansi_buf[i] >= '0' && ansi_buf[i] <= '9') {
					num2 *= 10;
					num2 += ansi_buf[i]-'0';
				} else {
					--i;
					next = 7;
				}
				break;

			case 7:
				if ((ansi_buf[i] >= 'A' && ansi_buf[i] <= 'H')
					|| ansi_buf[i] == 'J'
					|| ansi_buf[i] == 'K'
					|| ansi_buf[i] == 'h'
					|| ansi_buf[i] == 'l'
					|| ansi_buf[i] == 'm') {
					cchar = ansi_buf[i];
					flush = 1;
				} else
					fail = 1;
				break;
			}
		}

		if (fail) {
			for (i = 0; i < ansi_buf_size; ++i)
				parse_putc(ansi_buf[i]);
			ansi_buf_size = 0;
			return;
		}

		if (flush) {
			if (!ansi_cursor_hidden)
				CURSOR_OFF;
			ansi_buf_size = 0;
			switch (cchar) {
			case 'A':
				/* move cursor num1 rows up */
				console_cursor_up(num1);
				break;
			case 'B':
				/* move cursor num1 rows down */
				console_cursor_down(num1);
				break;
			case 'C':
				/* move cursor num1 columns forward */
				console_cursor_right(num1);
				break;
			case 'D':
				/* move cursor num1 columns back */
				console_cursor_left(num1);
				break;
			case 'E':
				/* move cursor num1 rows up at begin of row */
				console_previousline(num1);
				break;
			case 'F':
				/* move cursor num1 rows down at begin of row */
				console_newline(num1);
				break;
			case 'G':
				/* move cursor to column num1 */
				console_cursor_set_position(-1, num1-1);
				break;
			case 'H':
				/* move cursor to row num1, column num2 */
				console_cursor_set_position(num1-1, num2-1);
				break;
			case 'J':
				/* clear console and move cursor to 0, 0 */
				console_clear();
				console_cursor_set_position(0, 0);
				break;
			case 'K':
				/* clear line */
				if (num1 == 0)
					console_clear_line(console_row,
							console_col,
							CONSOLE_COLS-1);
				else if (num1 == 1)
					console_clear_line(console_row,
							0, console_col);
				else
					console_clear_line(console_row,
							0, CONSOLE_COLS-1);
				break;
			case 'h':
				ansi_cursor_hidden = 0;
				break;
			case 'l':
				ansi_cursor_hidden = 1;
				break;
			case 'm':
				if (num1 == 0) { /* reset swapped colors */
					if (ansi_colors_need_revert) {
						console_swap_colors();
						ansi_colors_need_revert = 0;
					}
				} else if (num1 == 7) { /* once swap colors */
					if (!ansi_colors_need_revert) {
						console_swap_colors();
						ansi_colors_need_revert = 1;
					}
				}
				break;
			}
			if (!ansi_cursor_hidden)
				CURSOR_SET;
		}
	} else {
		parse_putc(c);
	}
#else
	parse_putc(c);
#endif
	if (cfb_do_flush_cache)
		flush_cache(VIDEO_FB_ADRS, VIDEO_SIZE);
}

static void cfb_video_puts(struct stdio_dev *dev, const char *s)
{
	int flush = cfb_do_flush_cache;
	int count = strlen(s);

	/* temporarily disable cache flush */
	cfb_do_flush_cache = 0;

	while (count--)
		cfb_video_putc(dev, *s++);

	if (flush) {
		cfb_do_flush_cache = flush;
		flush_cache(VIDEO_FB_ADRS, VIDEO_SIZE);
	}
}

/*
 * Do not enforce drivers (or board code) to provide empty
 * video_set_lut() if they do not support 8 bpp format.
 * Implement weak default function instead.
 */
__weak void video_set_lut(unsigned int index, unsigned char r,
		     unsigned char g, unsigned char b)
{
}

#if defined(CONFIG_CMD_BMP) || defined(CONFIG_SPLASH_SCREEN)

#define FILL_8BIT_332RGB(r,g,b)	{			\
	*fb = ((r>>5)<<5) | ((g>>5)<<2) | (b>>6);	\
	fb ++;						\
}

#define FILL_15BIT_555RGB(r,g,b) {			\
	*(unsigned short *)fb =				\
		SWAP16((unsigned short)(((r>>3)<<10) |	\
					((g>>3)<<5)  |	\
					 (b>>3)));	\
	fb += 2;					\
}

#define FILL_16BIT_565RGB(r,g,b) {			\
	*(unsigned short *)fb =				\
		SWAP16((unsigned short)((((r)>>3)<<11)| \
					(((g)>>2)<<5) | \
					 ((b)>>3)));	\
	fb += 2;					\
}

#define FILL_32BIT_X888RGB(r,g,b) {			\
	*(u32 *)fb =				\
		SWAP32((unsigned int)(((r<<16) |	\
					(g<<8)  |	\
					 b)));		\
	fb += 4;					\
}

#ifdef VIDEO_FB_LITTLE_ENDIAN
#define FILL_24BIT_888RGB(r,g,b) {			\
	fb[0] = b;					\
	fb[1] = g;					\
	fb[2] = r;					\
	fb += 3;					\
}
#else
#define FILL_24BIT_888RGB(r,g,b) {			\
	fb[0] = r;					\
	fb[1] = g;					\
	fb[2] = b;					\
	fb += 3;					\
}
#endif

#if defined(VIDEO_FB_16BPP_PIXEL_SWAP)
static inline void fill_555rgb_pswap(uchar *fb, int x, u8 r, u8 g, u8 b)
{
	ushort *dst = (ushort *) fb;
	ushort color = (ushort) (((r >> 3) << 10) |
				 ((g >> 3) <<  5) |
				  (b >> 3));
	if (x & 1)
		*(--dst) = color;
	else
		*(++dst) = color;
}
#endif

/*
 * RLE8 bitmap support
 */

#ifdef CONFIG_VIDEO_BMP_RLE8
/* Pre-calculated color table entry */
struct palette {
	union {
		unsigned short w;	/* word */
		unsigned int dw;	/* double word */
	} ce;				/* color entry */
};

/*
 * Helper to draw encoded/unencoded run.
 */
static void draw_bitmap(uchar **fb, uchar *bm, struct palette *p,
			int cnt, int enc)
{
	ulong addr = (ulong) *fb;
	int *off;
	int enc_off = 1;
	int i;

	/*
	 * Setup offset of the color index in the bitmap.
	 * Color index of encoded run is at offset 1.
	 */
	off = enc ? &enc_off : &i;

	switch (VIDEO_DATA_FORMAT) {
	case GDF__8BIT_INDEX:
		for (i = 0; i < cnt; i++)
			*(unsigned char *) addr++ = bm[*off];
		break;
	case GDF_15BIT_555RGB:
	case GDF_16BIT_565RGB:
		/* differences handled while pre-calculating palette */
		for (i = 0; i < cnt; i++) {
			*(unsigned short *) addr = p[bm[*off]].ce.w;
			addr += 2;
		}
		break;
	case GDF_32BIT_X888RGB:
		for (i = 0; i < cnt; i++) {
			*(u32 *) addr = p[bm[*off]].ce.dw;
			addr += 4;
		}
		break;
	}
	*fb = (uchar *) addr;	/* return modified address */
}

static int display_rle8_bitmap(struct bmp_image *img, int xoff, int yoff,
			       int width, int height)
{
	unsigned char *bm;
	unsigned char *fbp;
	unsigned int cnt, runlen;
	int decode = 1;
	int x, y, bpp, i, ncolors;
	struct palette p[256];
	struct bmp_color_table_entry cte;
	int green_shift, red_off;
	int limit = (VIDEO_LINE_LEN / VIDEO_PIXEL_SIZE) * VIDEO_ROWS;
	int pixels = 0;

	x = 0;
	y = __le32_to_cpu(img->header.height) - 1;
	ncolors = __le32_to_cpu(img->header.colors_used);
	bpp = VIDEO_PIXEL_SIZE;
	fbp = (unsigned char *) ((unsigned int) video_fb_address +
				 (y + yoff) * VIDEO_LINE_LEN +
				 xoff * bpp);

	bm = (uchar *) img + __le32_to_cpu(img->header.data_offset);

	/* pre-calculate and setup palette */
	switch (VIDEO_DATA_FORMAT) {
	case GDF__8BIT_INDEX:
		for (i = 0; i < ncolors; i++) {
			cte = img->color_table[i];
			video_set_lut(i, cte.red, cte.green, cte.blue);
		}
		break;
	case GDF_15BIT_555RGB:
	case GDF_16BIT_565RGB:
		if (VIDEO_DATA_FORMAT == GDF_15BIT_555RGB) {
			green_shift = 3;
			red_off = 10;
		} else {
			green_shift = 2;
			red_off = 11;
		}
		for (i = 0; i < ncolors; i++) {
			cte = img->color_table[i];
			p[i].ce.w = SWAP16((unsigned short)
					   (((cte.red >> 3) << red_off) |
					    ((cte.green >> green_shift) << 5) |
					    cte.blue >> 3));
		}
		break;
	case GDF_32BIT_X888RGB:
		for (i = 0; i < ncolors; i++) {
			cte = img->color_table[i];
			p[i].ce.dw = SWAP32((cte.red << 16) |
					    (cte.green << 8) |
					     cte.blue);
		}
		break;
	default:
		printf("RLE Bitmap unsupported in video mode 0x%x\n",
		       VIDEO_DATA_FORMAT);
		return -1;
	}

	while (decode) {
		switch (bm[0]) {
		case 0:
			switch (bm[1]) {
			case 0:
				/* scan line end marker */
				bm += 2;
				x = 0;
				y--;
				fbp = (unsigned char *)
					((unsigned int) video_fb_address +
					 (y + yoff) * VIDEO_LINE_LEN +
					 xoff * bpp);
				continue;
			case 1:
				/* end of bitmap data marker */
				decode = 0;
				break;
			case 2:
				/* run offset marker */
				x += bm[2];
				y -= bm[3];
				fbp = (unsigned char *)
					((unsigned int) video_fb_address +
					 (y + yoff) * VIDEO_LINE_LEN +
					 xoff * bpp);
				bm += 4;
				break;
			default:
				/* unencoded run */
				cnt = bm[1];
				runlen = cnt;
				pixels += cnt;
				if (pixels > limit)
					goto error;

				bm += 2;
				if (y < height) {
					if (x >= width) {
						x += runlen;
						goto next_run;
					}
					if (x + runlen > width)
						cnt = width - x;
					draw_bitmap(&fbp, bm, p, cnt, 0);
					x += runlen;
				}
next_run:
				bm += runlen;
				if (runlen & 1)
					bm++;	/* 0 padding if length is odd */
			}
			break;
		default:
			/* encoded run */
			cnt = bm[0];
			runlen = cnt;
			pixels += cnt;
			if (pixels > limit)
				goto error;

			if (y < height) {     /* only draw into visible area */
				if (x >= width) {
					x += runlen;
					bm += 2;
					continue;
				}
				if (x + runlen > width)
					cnt = width - x;
				draw_bitmap(&fbp, bm, p, cnt, 1);
				x += runlen;
			}
			bm += 2;
			break;
		}
	}

	if (cfb_do_flush_cache)
		flush_cache(VIDEO_FB_ADRS, VIDEO_SIZE);

	return 0;
error:
	printf("Error: Too much encoded pixel data, validate your bitmap\n");
	return -1;
}
#endif

/*
 * Display the BMP file located at address bmp_image.
 */
int video_display_bitmap(ulong bmp_image, int x, int y)
{
	ushort xcount, ycount;
	uchar *fb;
	struct bmp_image *bmp = (struct bmp_image *)bmp_image;
	uchar *bmap;
	ushort padded_line;
	unsigned long width, height, bpp;
	unsigned colors;
	unsigned long compression;
	struct bmp_color_table_entry cte;

#ifdef CONFIG_VIDEO_BMP_GZIP
	unsigned char *dst = NULL;
	ulong len;
#endif

	WATCHDOG_RESET();

	if (!((bmp->header.signature[0] == 'B') &&
	      (bmp->header.signature[1] == 'M'))) {

#ifdef CONFIG_VIDEO_BMP_GZIP
		/*
		 * Could be a gzipped bmp image, try to decrompress...
		 */
		len = CONFIG_SYS_VIDEO_LOGO_MAX_SIZE;
		dst = malloc(CONFIG_SYS_VIDEO_LOGO_MAX_SIZE);
		if (dst == NULL) {
			printf("Error: malloc in gunzip failed!\n");
			return 1;
		}
		/*
		 * NB: we need to force offset of +2
		 * See doc/README.displaying-bmps
		 */
		if (gunzip(dst+2, CONFIG_SYS_VIDEO_LOGO_MAX_SIZE-2,
			   (uchar *) bmp_image,
			   &len) != 0) {
			printf("Error: no valid bmp or bmp.gz image at %lx\n",
			       bmp_image);
			free(dst);
			return 1;
		}
		if (len == CONFIG_SYS_VIDEO_LOGO_MAX_SIZE) {
			printf("Image could be truncated "
				"(increase CONFIG_SYS_VIDEO_LOGO_MAX_SIZE)!\n");
		}

		/*
		 * Set addr to decompressed image
		 */
		bmp = (struct bmp_image *)(dst+2);

		if (!((bmp->header.signature[0] == 'B') &&
		      (bmp->header.signature[1] == 'M'))) {
			printf("Error: no valid bmp.gz image at %lx\n",
			       bmp_image);
			free(dst);
			return 1;
		}
#else
		printf("Error: no valid bmp image at %lx\n", bmp_image);
		return 1;
#endif /* CONFIG_VIDEO_BMP_GZIP */
	}

	width = le32_to_cpu(bmp->header.width);
	height = le32_to_cpu(bmp->header.height);
	bpp = le16_to_cpu(bmp->header.bit_count);
	colors = le32_to_cpu(bmp->header.colors_used);
	compression = le32_to_cpu(bmp->header.compression);

	debug("Display-bmp: %ld x %ld  with %d colors\n",
	      width, height, colors);

	if (compression != BMP_BI_RGB
#ifdef CONFIG_VIDEO_BMP_RLE8
	    && compression != BMP_BI_RLE8
#endif
		) {
		printf("Error: compression type %ld not supported\n",
		       compression);
#ifdef CONFIG_VIDEO_BMP_GZIP
		if (dst)
			free(dst);
#endif
		return 1;
	}

	padded_line = (((width * bpp + 7) / 8) + 3) & ~0x3;

#ifdef CONFIG_SPLASH_SCREEN_ALIGN
	if (x == BMP_ALIGN_CENTER)
		x = max(0, (int)(VIDEO_VISIBLE_COLS - width) / 2);
	else if (x < 0)
		x = max(0, (int)(VIDEO_VISIBLE_COLS - width + x + 1));

	if (y == BMP_ALIGN_CENTER)
		y = max(0, (int)(VIDEO_VISIBLE_ROWS - height) / 2);
	else if (y < 0)
		y = max(0, (int)(VIDEO_VISIBLE_ROWS - height + y + 1));
#endif /* CONFIG_SPLASH_SCREEN_ALIGN */

	/*
	 * Just ignore elements which are completely beyond screen
	 * dimensions.
	 */
	if ((x >= VIDEO_VISIBLE_COLS) || (y >= VIDEO_VISIBLE_ROWS))
		return 0;

	if ((x + width) > VIDEO_VISIBLE_COLS)
		width = VIDEO_VISIBLE_COLS - x;
	if ((y + height) > VIDEO_VISIBLE_ROWS)
		height = VIDEO_VISIBLE_ROWS - y;

	bmap = (uchar *) bmp + le32_to_cpu(bmp->header.data_offset);
	fb = (uchar *) (video_fb_address +
			((y + height - 1) * VIDEO_LINE_LEN) +
			x * VIDEO_PIXEL_SIZE);

#ifdef CONFIG_VIDEO_BMP_RLE8
	if (compression == BMP_BI_RLE8) {
		return display_rle8_bitmap(bmp, x, y, width, height);
	}
#endif

	/* We handle only 4, 8, or 24 bpp bitmaps */
	switch (le16_to_cpu(bmp->header.bit_count)) {
	case 4:
		padded_line -= width / 2;
		ycount = height;

		switch (VIDEO_DATA_FORMAT) {
		case GDF_32BIT_X888RGB:
			while (ycount--) {
				WATCHDOG_RESET();
				/*
				 * Don't assume that 'width' is an
				 * even number
				 */
				for (xcount = 0; xcount < width; xcount++) {
					uchar idx;

					if (xcount & 1) {
						idx = *bmap & 0xF;
						bmap++;
					} else
						idx = *bmap >> 4;
					cte = bmp->color_table[idx];
					FILL_32BIT_X888RGB(cte.red, cte.green,
							   cte.blue);
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		default:
			puts("4bpp bitmap unsupported with current "
			     "video mode\n");
			break;
		}
		break;

	case 8:
		padded_line -= width;
		if (VIDEO_DATA_FORMAT == GDF__8BIT_INDEX) {
			/* Copy colormap */
			for (xcount = 0; xcount < colors; ++xcount) {
				cte = bmp->color_table[xcount];
				video_set_lut(xcount, cte.red, cte.green,
					      cte.blue);
			}
		}
		ycount = height;
		switch (VIDEO_DATA_FORMAT) {
		case GDF__8BIT_INDEX:
			while (ycount--) {
				WATCHDOG_RESET();
				xcount = width;
				while (xcount--) {
					*fb++ = *bmap++;
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		case GDF__8BIT_332RGB:
			while (ycount--) {
				WATCHDOG_RESET();
				xcount = width;
				while (xcount--) {
					cte = bmp->color_table[*bmap++];
					FILL_8BIT_332RGB(cte.red, cte.green,
							 cte.blue);
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		case GDF_15BIT_555RGB:
			while (ycount--) {
#if defined(VIDEO_FB_16BPP_PIXEL_SWAP)
				int xpos = x;
#endif
				WATCHDOG_RESET();
				xcount = width;
				while (xcount--) {
					cte = bmp->color_table[*bmap++];
#if defined(VIDEO_FB_16BPP_PIXEL_SWAP)
					fill_555rgb_pswap(fb, xpos++, cte.red,
							  cte.green,
							  cte.blue);
					fb += 2;
#else
					FILL_15BIT_555RGB(cte.red, cte.green,
							  cte.blue);
#endif
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		case GDF_16BIT_565RGB:
			while (ycount--) {
				WATCHDOG_RESET();
				xcount = width;
				while (xcount--) {
					cte = bmp->color_table[*bmap++];
					FILL_16BIT_565RGB(cte.red, cte.green,
							  cte.blue);
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		case GDF_32BIT_X888RGB:
			while (ycount--) {
				WATCHDOG_RESET();
				xcount = width;
				while (xcount--) {
					cte = bmp->color_table[*bmap++];
					FILL_32BIT_X888RGB(cte.red, cte.green,
							   cte.blue);
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		case GDF_24BIT_888RGB:
			while (ycount--) {
				WATCHDOG_RESET();
				xcount = width;
				while (xcount--) {
					cte = bmp->color_table[*bmap++];
					FILL_24BIT_888RGB(cte.red, cte.green,
							  cte.blue);
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		}
		break;
	case 24:
		padded_line -= 3 * width;
		ycount = height;
		switch (VIDEO_DATA_FORMAT) {
		case GDF__8BIT_332RGB:
			while (ycount--) {
				WATCHDOG_RESET();
				xcount = width;
				while (xcount--) {
					FILL_8BIT_332RGB(bmap[2], bmap[1],
							 bmap[0]);
					bmap += 3;
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		case GDF_15BIT_555RGB:
			while (ycount--) {
#if defined(VIDEO_FB_16BPP_PIXEL_SWAP)
				int xpos = x;
#endif
				WATCHDOG_RESET();
				xcount = width;
				while (xcount--) {
#if defined(VIDEO_FB_16BPP_PIXEL_SWAP)
					fill_555rgb_pswap(fb, xpos++, bmap[2],
							  bmap[1], bmap[0]);
					fb += 2;
#else
					FILL_15BIT_555RGB(bmap[2], bmap[1],
							  bmap[0]);
#endif
					bmap += 3;
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		case GDF_16BIT_565RGB:
			while (ycount--) {
				WATCHDOG_RESET();
				xcount = width;
				while (xcount--) {
					FILL_16BIT_565RGB(bmap[2], bmap[1],
							  bmap[0]);
					bmap += 3;
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		case GDF_32BIT_X888RGB:
			while (ycount--) {
				WATCHDOG_RESET();
				xcount = width;
				while (xcount--) {
					FILL_32BIT_X888RGB(bmap[2], bmap[1],
							   bmap[0]);
					bmap += 3;
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		case GDF_24BIT_888RGB:
			while (ycount--) {
				WATCHDOG_RESET();
				xcount = width;
				while (xcount--) {
					FILL_24BIT_888RGB(bmap[2], bmap[1],
							  bmap[0]);
					bmap += 3;
				}
				bmap += padded_line;
				fb -= VIDEO_LINE_LEN + width *
					VIDEO_PIXEL_SIZE;
			}
			break;
		default:
			printf("Error: 24 bits/pixel bitmap incompatible "
				"with current video mode\n");
			break;
		}
		break;
	default:
		printf("Error: %d bit/pixel bitmaps not supported by U-Boot\n",
			le16_to_cpu(bmp->header.bit_count));
		break;
	}

#ifdef CONFIG_VIDEO_BMP_GZIP
	if (dst) {
		free(dst);
	}
#endif

	if (cfb_do_flush_cache)
		flush_cache(VIDEO_FB_ADRS, VIDEO_SIZE);
	return (0);
}
#endif


#ifdef CONFIG_VIDEO_LOGO
static int video_logo_xpos;
static int video_logo_ypos;

static void plot_logo_or_black(void *screen, int x, int y, int black);

static void logo_plot(void *screen, int x, int y)
{
	plot_logo_or_black(screen, x, y, 0);
}

static void logo_black(void)
{
	plot_logo_or_black(video_fb_address, video_logo_xpos, video_logo_ypos,
			1);
}

static int do_clrlogo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 1)
		return cmd_usage(cmdtp);

	logo_black();
	return 0;
}

U_BOOT_CMD(
	   clrlogo, 1, 0, do_clrlogo,
	   "fill the boot logo area with black",
	   " "
	   );

static void plot_logo_or_black(void *screen, int x, int y, int black)
{

	int xcount, i;
	int skip = VIDEO_LINE_LEN - VIDEO_LOGO_WIDTH * VIDEO_PIXEL_SIZE;
	int ycount = video_logo_height;
	unsigned char r, g, b, *logo_red, *logo_blue, *logo_green;
	unsigned char *source;
	unsigned char *dest;

#ifdef CONFIG_SPLASH_SCREEN_ALIGN
	if (x == BMP_ALIGN_CENTER)
		x = max(0, (int)(VIDEO_VISIBLE_COLS - VIDEO_LOGO_WIDTH) / 2);
	else if (x < 0)
		x = max(0, (int)(VIDEO_VISIBLE_COLS - VIDEO_LOGO_WIDTH + x + 1));

	if (y == BMP_ALIGN_CENTER)
		y = max(0, (int)(VIDEO_VISIBLE_ROWS - VIDEO_LOGO_HEIGHT) / 2);
	else if (y < 0)
		y = max(0, (int)(VIDEO_VISIBLE_ROWS - VIDEO_LOGO_HEIGHT + y + 1));
#endif /* CONFIG_SPLASH_SCREEN_ALIGN */

	dest = (unsigned char *)screen + y * VIDEO_LINE_LEN + x * VIDEO_PIXEL_SIZE;

#ifdef CONFIG_VIDEO_BMP_LOGO
	source = bmp_logo_bitmap;

	/* Allocate temporary space for computing colormap */
	logo_red = malloc(BMP_LOGO_COLORS);
	logo_green = malloc(BMP_LOGO_COLORS);
	logo_blue = malloc(BMP_LOGO_COLORS);
	/* Compute color map */
	for (i = 0; i < VIDEO_LOGO_COLORS; i++) {
		logo_red[i] = (bmp_logo_palette[i] & 0x0f00) >> 4;
		logo_green[i] = (bmp_logo_palette[i] & 0x00f0);
		logo_blue[i] = (bmp_logo_palette[i] & 0x000f) << 4;
	}
#else
	source = linux_logo;
	logo_red = linux_logo_red;
	logo_green = linux_logo_green;
	logo_blue = linux_logo_blue;
#endif

	if (VIDEO_DATA_FORMAT == GDF__8BIT_INDEX) {
		for (i = 0; i < VIDEO_LOGO_COLORS; i++) {
			video_set_lut(i + VIDEO_LOGO_LUT_OFFSET,
				      logo_red[i], logo_green[i],
				      logo_blue[i]);
		}
	}

	while (ycount--) {
#if defined(VIDEO_FB_16BPP_PIXEL_SWAP)
		int xpos = x;
#endif
		xcount = VIDEO_LOGO_WIDTH;
		while (xcount--) {
			if (black) {
				r = 0x00;
				g = 0x00;
				b = 0x00;
			} else {
				r = logo_red[*source - VIDEO_LOGO_LUT_OFFSET];
				g = logo_green[*source - VIDEO_LOGO_LUT_OFFSET];
				b = logo_blue[*source - VIDEO_LOGO_LUT_OFFSET];
			}

			switch (VIDEO_DATA_FORMAT) {
			case GDF__8BIT_INDEX:
				*dest = *source;
				break;
			case GDF__8BIT_332RGB:
				*dest = ((r >> 5) << 5) |
					((g >> 5) << 2) |
					 (b >> 6);
				break;
			case GDF_15BIT_555RGB:
#if defined(VIDEO_FB_16BPP_PIXEL_SWAP)
				fill_555rgb_pswap(dest, xpos++, r, g, b);
#else
				*(unsigned short *) dest =
					SWAP16((unsigned short) (
							((r >> 3) << 10) |
							((g >> 3) <<  5) |
							 (b >> 3)));
#endif
				break;
			case GDF_16BIT_565RGB:
				*(unsigned short *) dest =
					SWAP16((unsigned short) (
							((r >> 3) << 11) |
							((g >> 2) <<  5) |
							 (b >> 3)));
				break;
			case GDF_32BIT_X888RGB:
				*(u32 *) dest =
					SWAP32((u32) (
							(r << 16) |
							(g <<  8) |
							 b));
				break;
			case GDF_24BIT_888RGB:
#ifdef VIDEO_FB_LITTLE_ENDIAN
				dest[0] = b;
				dest[1] = g;
				dest[2] = r;
#else
				dest[0] = r;
				dest[1] = g;
				dest[2] = b;
#endif
				break;
			}
			source++;
			dest += VIDEO_PIXEL_SIZE;
		}
		dest += skip;
	}
#ifdef CONFIG_VIDEO_BMP_LOGO
	free(logo_red);
	free(logo_green);
	free(logo_blue);
#endif
}

static void *video_logo(void)
{
	char info[128];
	__maybe_unused int y_off = 0;
	__maybe_unused ulong addr;
	__maybe_unused char *s;
	__maybe_unused int len, ret, space;

	splash_get_pos(&video_logo_xpos, &video_logo_ypos);

#ifdef CONFIG_SPLASH_SCREEN
	s = env_get("splashimage");
	if (s != NULL) {
		ret = splash_screen_prepare();
		if (ret < 0)
			return video_fb_address;
		addr = simple_strtoul(s, NULL, 16);

		if (video_display_bitmap(addr,
					video_logo_xpos,
					video_logo_ypos) == 0) {
			video_logo_height = 0;
			return ((void *) (video_fb_address));
		}
	}
#endif /* CONFIG_SPLASH_SCREEN */

	logo_plot(video_fb_address, video_logo_xpos, video_logo_ypos);

#ifdef CONFIG_SPLASH_SCREEN_ALIGN
	/*
	 * when using splashpos for video_logo, skip any info
	 * output on video console if the logo is not at 0,0
	 */
	if (video_logo_xpos || video_logo_ypos) {
		/*
		 * video_logo_height is used in text and cursor offset
		 * calculations. Since the console is below the logo,
		 * we need to adjust the logo height
		 */
		if (video_logo_ypos == BMP_ALIGN_CENTER)
			video_logo_height += max(0, (int)(VIDEO_VISIBLE_ROWS -
						     VIDEO_LOGO_HEIGHT) / 2);
		else if (video_logo_ypos > 0)
			video_logo_height += video_logo_ypos;

		return video_fb_address + video_logo_height * VIDEO_LINE_LEN;
	}
#endif
	if (board_cfb_skip())
		return 0;

	sprintf(info, " %s", version_string);

#ifndef CONFIG_HIDE_LOGO_VERSION
	space = (VIDEO_COLS - VIDEO_INFO_X) / VIDEO_FONT_WIDTH;
	len = strlen(info);

	if (len > space) {
		int xx = VIDEO_INFO_X, yy = VIDEO_INFO_Y;
		uchar *p = (uchar *) info;

		while (len) {
			if (len > space) {
				video_drawchars(xx, yy, p, space);
				len -= space;

				p = (uchar *)p + space;

				if (!y_off) {
					xx += VIDEO_FONT_WIDTH;
					space--;
				}
				yy += VIDEO_FONT_HEIGHT;

				y_off++;
			} else {
				video_drawchars(xx, yy, p, len);
				len = 0;
			}
		}
	} else
		video_drawstring(VIDEO_INFO_X, VIDEO_INFO_Y, (uchar *) info);

#ifdef CONFIG_CONSOLE_EXTRA_INFO
	{
		int i, n =
			((video_logo_height -
			  VIDEO_FONT_HEIGHT) / VIDEO_FONT_HEIGHT);

		for (i = 1; i < n; i++) {
			video_get_info_str(i, info);
			if (!*info)
				continue;

			len = strlen(info);
			if (len > space) {
				video_drawchars(VIDEO_INFO_X,
						VIDEO_INFO_Y +
						(i + y_off) *
							VIDEO_FONT_HEIGHT,
						(uchar *) info, space);
				y_off++;
				video_drawchars(VIDEO_INFO_X +
						VIDEO_FONT_WIDTH,
						VIDEO_INFO_Y +
							(i + y_off) *
							VIDEO_FONT_HEIGHT,
						(uchar *) info + space,
						len - space);
			} else {
				video_drawstring(VIDEO_INFO_X,
						 VIDEO_INFO_Y +
						 (i + y_off) *
							VIDEO_FONT_HEIGHT,
						 (uchar *) info);
			}
		}
	}
#endif
#endif

	return (video_fb_address + video_logo_height * VIDEO_LINE_LEN);
}
#endif

static int cfb_fb_is_in_dram(void)
{
	bd_t *bd = gd->bd;
#if defined(CONFIG_ARM) || defined(CONFIG_NDS32) || \
defined(CONFIG_SANDBOX) || defined(CONFIG_X86)
	ulong start, end;
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		start = bd->bi_dram[i].start;
		end = bd->bi_dram[i].start + bd->bi_dram[i].size - 1;
		if ((ulong)video_fb_address >= start &&
		    (ulong)video_fb_address < end)
			return 1;
	}
#else
	if ((ulong)video_fb_address >= bd->bi_memstart &&
	    (ulong)video_fb_address < bd->bi_memstart + bd->bi_memsize)
		return 1;
#endif
	return 0;
}

void video_clear(void)
{
	if (!video_fb_address)
		return;
#ifdef VIDEO_HW_RECTFILL
	video_hw_rectfill(VIDEO_PIXEL_SIZE,	/* bytes per pixel */
			  0,			/* dest pos x */
			  0,			/* dest pos y */
			  VIDEO_VISIBLE_COLS,	/* frame width */
			  VIDEO_VISIBLE_ROWS,	/* frame height */
			  bgx			/* fill color */
	);
#else
	memsetl(video_fb_address,
		(VIDEO_VISIBLE_ROWS * VIDEO_LINE_LEN) / sizeof(int), bgx);
#endif
}

static int cfg_video_init(void)
{
	unsigned char color8;

	pGD = video_hw_init();
	if (pGD == NULL)
		return -1;

	video_fb_address = (void *) VIDEO_FB_ADRS;

	cfb_do_flush_cache = cfb_fb_is_in_dram() && dcache_status();

	/* Init drawing pats */
	switch (VIDEO_DATA_FORMAT) {
	case GDF__8BIT_INDEX:
		video_set_lut(0x01, CONFIG_SYS_CONSOLE_FG_COL,
			      CONFIG_SYS_CONSOLE_FG_COL,
			      CONFIG_SYS_CONSOLE_FG_COL);
		video_set_lut(0x00, CONFIG_SYS_CONSOLE_BG_COL,
			      CONFIG_SYS_CONSOLE_BG_COL,
			      CONFIG_SYS_CONSOLE_BG_COL);
		fgx = 0x01010101;
		bgx = 0x00000000;
		break;
	case GDF__8BIT_332RGB:
		color8 = ((CONFIG_SYS_CONSOLE_FG_COL & 0xe0) |
			  ((CONFIG_SYS_CONSOLE_FG_COL >> 3) & 0x1c) |
			  CONFIG_SYS_CONSOLE_FG_COL >> 6);
		fgx = (color8 << 24) | (color8 << 16) | (color8 << 8) |
			color8;
		color8 = ((CONFIG_SYS_CONSOLE_BG_COL & 0xe0) |
			  ((CONFIG_SYS_CONSOLE_BG_COL >> 3) & 0x1c) |
			  CONFIG_SYS_CONSOLE_BG_COL >> 6);
		bgx = (color8 << 24) | (color8 << 16) | (color8 << 8) |
			color8;
		break;
	case GDF_15BIT_555RGB:
		fgx = (((CONFIG_SYS_CONSOLE_FG_COL >> 3) << 26) |
		       ((CONFIG_SYS_CONSOLE_FG_COL >> 3) << 21) |
		       ((CONFIG_SYS_CONSOLE_FG_COL >> 3) << 16) |
		       ((CONFIG_SYS_CONSOLE_FG_COL >> 3) << 10) |
		       ((CONFIG_SYS_CONSOLE_FG_COL >> 3) <<  5) |
			(CONFIG_SYS_CONSOLE_FG_COL >> 3));
		bgx = (((CONFIG_SYS_CONSOLE_BG_COL >> 3) << 26) |
		       ((CONFIG_SYS_CONSOLE_BG_COL >> 3) << 21) |
		       ((CONFIG_SYS_CONSOLE_BG_COL >> 3) << 16) |
		       ((CONFIG_SYS_CONSOLE_BG_COL >> 3) << 10) |
		       ((CONFIG_SYS_CONSOLE_BG_COL >> 3) <<  5) |
			(CONFIG_SYS_CONSOLE_BG_COL >> 3));
		break;
	case GDF_16BIT_565RGB:
		fgx = (((CONFIG_SYS_CONSOLE_FG_COL >> 3) << 27) |
		       ((CONFIG_SYS_CONSOLE_FG_COL >> 2) << 21) |
		       ((CONFIG_SYS_CONSOLE_FG_COL >> 3) << 16) |
		       ((CONFIG_SYS_CONSOLE_FG_COL >> 3) << 11) |
		       ((CONFIG_SYS_CONSOLE_FG_COL >> 2) <<  5) |
			(CONFIG_SYS_CONSOLE_FG_COL >> 3));
		bgx = (((CONFIG_SYS_CONSOLE_BG_COL >> 3) << 27) |
		       ((CONFIG_SYS_CONSOLE_BG_COL >> 2) << 21) |
		       ((CONFIG_SYS_CONSOLE_BG_COL >> 3) << 16) |
		       ((CONFIG_SYS_CONSOLE_BG_COL >> 3) << 11) |
		       ((CONFIG_SYS_CONSOLE_BG_COL >> 2) <<  5) |
			(CONFIG_SYS_CONSOLE_BG_COL >> 3));
		break;
	case GDF_32BIT_X888RGB:
		fgx =	(CONFIG_SYS_CONSOLE_FG_COL << 16) |
			(CONFIG_SYS_CONSOLE_FG_COL <<  8) |
			 CONFIG_SYS_CONSOLE_FG_COL;
		bgx =	(CONFIG_SYS_CONSOLE_BG_COL << 16) |
			(CONFIG_SYS_CONSOLE_BG_COL <<  8) |
			 CONFIG_SYS_CONSOLE_BG_COL;
		break;
	case GDF_24BIT_888RGB:
		fgx =	(CONFIG_SYS_CONSOLE_FG_COL << 24) |
			(CONFIG_SYS_CONSOLE_FG_COL << 16) |
			(CONFIG_SYS_CONSOLE_FG_COL <<  8) |
			 CONFIG_SYS_CONSOLE_FG_COL;
		bgx =	(CONFIG_SYS_CONSOLE_BG_COL << 24) |
			(CONFIG_SYS_CONSOLE_BG_COL << 16) |
			(CONFIG_SYS_CONSOLE_BG_COL <<  8) |
			 CONFIG_SYS_CONSOLE_BG_COL;
		break;
	}
	eorx = fgx ^ bgx;

	if (!CONFIG_IS_ENABLED(NO_FB_CLEAR))
		video_clear();

#ifdef CONFIG_VIDEO_LOGO
	/* Plot the logo and get start point of console */
	debug("Video: Drawing the logo ...\n");
	video_console_address = video_logo();
#else
	video_console_address = video_fb_address;
#endif

	/* Initialize the console */
	console_col = 0;
	console_row = 0;

	if (cfb_do_flush_cache)
		flush_cache(VIDEO_FB_ADRS, VIDEO_SIZE);

	return 0;
}

/*
 * Implement a weak default function for boards that optionally
 * need to skip the video initialization.
 */
__weak int board_video_skip(void)
{
	/* As default, don't skip test */
	return 0;
}

int drv_video_init(void)
{
	struct stdio_dev console_dev;
	bool have_keyboard;
	bool __maybe_unused keyboard_ok = false;

	/* Check if video initialization should be skipped */
	if (board_video_skip())
		return 0;

	/* Init video chip - returns with framebuffer cleared */
	if (cfg_video_init() == -1)
		return 0;

	if (board_cfb_skip())
		return 0;

#if defined(CONFIG_VGA_AS_SINGLE_DEVICE)
	have_keyboard = false;
#elif defined(CONFIG_OF_CONTROL)
	have_keyboard = !fdtdec_get_config_bool(gd->fdt_blob,
						"u-boot,no-keyboard");
#else
	have_keyboard = true;
#endif
	if (have_keyboard) {
		debug("KBD: Keyboard init ...\n");
#if !defined(CONFIG_VGA_AS_SINGLE_DEVICE)
		keyboard_ok = !(VIDEO_KBD_INIT_FCT == -1);
#endif
	}

	/* Init vga device */
	memset(&console_dev, 0, sizeof(console_dev));
	strcpy(console_dev.name, "vga");
	console_dev.flags = DEV_FLAGS_OUTPUT;
	console_dev.putc = cfb_video_putc;	/* 'putc' function */
	console_dev.puts = cfb_video_puts;	/* 'puts' function */

#if !defined(CONFIG_VGA_AS_SINGLE_DEVICE)
	if (have_keyboard && keyboard_ok) {
		/* Also init console device */
		console_dev.flags |= DEV_FLAGS_INPUT;
		console_dev.tstc = VIDEO_TSTC_FCT;	/* 'tstc' function */
		console_dev.getc = VIDEO_GETC_FCT;	/* 'getc' function */
	}
#endif

	if (stdio_register(&console_dev) != 0)
		return 0;

	/* Return success */
	return 1;
}

void video_position_cursor(unsigned col, unsigned row)
{
	console_col = min(col, CONSOLE_COLS - 1);
	console_row = min(row, CONSOLE_ROWS - 1);
}

int video_get_pixel_width(void)
{
	return VIDEO_VISIBLE_COLS;
}

int video_get_pixel_height(void)
{
	return VIDEO_VISIBLE_ROWS;
}

int video_get_screen_rows(void)
{
	return CONSOLE_ROWS;
}

int video_get_screen_columns(void)
{
	return CONSOLE_COLS;
}
