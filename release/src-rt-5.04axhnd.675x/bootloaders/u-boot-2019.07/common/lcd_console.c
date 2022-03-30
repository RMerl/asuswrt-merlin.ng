// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001-2015
 * DENX Software Engineering -- wd@denx.de
 * Compulab Ltd - http://compulab.co.il/
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 */

#include <common.h>
#include <lcd.h>
#include <video_font.h>		/* Get font data, width and height */
#if defined(CONFIG_LCD_LOGO)
#include <bmp_logo.h>
#endif

static struct console_t cons;

void lcd_set_col(short col)
{
	cons.curr_col = col;
}

void lcd_set_row(short row)
{
	cons.curr_row = row;
}

void lcd_position_cursor(unsigned col, unsigned row)
{
	cons.curr_col = min_t(short, col, cons.cols - 1);
	cons.curr_row = min_t(short, row, cons.rows - 1);
}

int lcd_get_screen_rows(void)
{
	return cons.rows;
}

int lcd_get_screen_columns(void)
{
	return cons.cols;
}

static void lcd_putc_xy0(struct console_t *pcons, ushort x, ushort y, char c)
{
	int fg_color = lcd_getfgcolor();
	int bg_color = lcd_getbgcolor();
	int i, row;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  y * pcons->lcdsizex +
				  x;

	for (row = 0; row < VIDEO_FONT_HEIGHT; row++) {
		uchar bits = video_fontdata[c * VIDEO_FONT_HEIGHT + row];
		for (i = 0; i < VIDEO_FONT_WIDTH; ++i) {
			*dst++ = (bits & 0x80) ? fg_color : bg_color;
			bits <<= 1;
		}
		dst += (pcons->lcdsizex - VIDEO_FONT_WIDTH);
	}
}

static inline void console_setrow0(struct console_t *pcons, u32 row, int clr)
{
	int i;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  row * VIDEO_FONT_HEIGHT *
				  pcons->lcdsizex;

	for (i = 0; i < (VIDEO_FONT_HEIGHT * pcons->lcdsizex); i++)
		*dst++ = clr;
}

static inline void console_moverow0(struct console_t *pcons,
				    u32 rowdst, u32 rowsrc)
{
	int i;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  rowdst * VIDEO_FONT_HEIGHT *
				  pcons->lcdsizex;

	fbptr_t *src = (fbptr_t *)pcons->fbbase +
				  rowsrc * VIDEO_FONT_HEIGHT *
				  pcons->lcdsizex;

	for (i = 0; i < (VIDEO_FONT_HEIGHT * pcons->lcdsizex); i++)
		*dst++ = *src++;
}

static inline void console_back(void)
{
	if (--cons.curr_col < 0) {
		cons.curr_col = cons.cols - 1;
		if (--cons.curr_row < 0)
			cons.curr_row = 0;
	}

	cons.fp_putc_xy(&cons,
			cons.curr_col * VIDEO_FONT_WIDTH,
			cons.curr_row * VIDEO_FONT_HEIGHT, ' ');
}

static inline void console_newline(void)
{
	const int rows = CONFIG_CONSOLE_SCROLL_LINES;
	int bg_color = lcd_getbgcolor();
	int i;

	cons.curr_col = 0;

	/* Check if we need to scroll the terminal */
	if (++cons.curr_row >= cons.rows) {
		for (i = 0; i < cons.rows-rows; i++)
			cons.fp_console_moverow(&cons, i, i+rows);
		for (i = 0; i < rows; i++)
			cons.fp_console_setrow(&cons, cons.rows-i-1, bg_color);
		cons.curr_row -= rows;
	}
	lcd_sync();
}

void console_calc_rowcol(struct console_t *pcons, u32 sizex, u32 sizey)
{
	pcons->cols = sizex / VIDEO_FONT_WIDTH;
#if defined(CONFIG_LCD_LOGO) && !defined(CONFIG_LCD_INFO_BELOW_LOGO)
	pcons->rows = (pcons->lcdsizey - BMP_LOGO_HEIGHT);
	pcons->rows /= VIDEO_FONT_HEIGHT;
#else
	pcons->rows = sizey / VIDEO_FONT_HEIGHT;
#endif
}

void __weak lcd_init_console_rot(struct console_t *pcons)
{
	return;
}

void lcd_init_console(void *address, int vl_cols, int vl_rows, int vl_rot)
{
	memset(&cons, 0, sizeof(cons));
	cons.fbbase = address;

	cons.lcdsizex = vl_cols;
	cons.lcdsizey = vl_rows;
	cons.lcdrot = vl_rot;

	cons.fp_putc_xy = &lcd_putc_xy0;
	cons.fp_console_moverow = &console_moverow0;
	cons.fp_console_setrow = &console_setrow0;
	console_calc_rowcol(&cons, cons.lcdsizex, cons.lcdsizey);

	lcd_init_console_rot(&cons);

	debug("lcd_console: have %d/%d col/rws on scr %dx%d (%d deg rotated)\n",
	      cons.cols, cons.rows, cons.lcdsizex, cons.lcdsizey, vl_rot);
}

void lcd_putc(const char c)
{
	if (!lcd_is_enabled) {
		serial_putc(c);

		return;
	}

	switch (c) {
	case '\r':
		cons.curr_col = 0;
		return;
	case '\n':
		console_newline();

		return;
	case '\t':	/* Tab (8 chars alignment) */
		cons.curr_col +=  8;
		cons.curr_col &= ~7;

		if (cons.curr_col >= cons.cols)
			console_newline();

		return;
	case '\b':
		console_back();

		return;
	default:
		cons.fp_putc_xy(&cons,
				cons.curr_col * VIDEO_FONT_WIDTH,
				cons.curr_row * VIDEO_FONT_HEIGHT, c);
		if (++cons.curr_col >= cons.cols)
			console_newline();
	}
}

void lcd_puts(const char *s)
{
	if (!lcd_is_enabled) {
		serial_puts(s);

		return;
	}

	while (*s)
		lcd_putc(*s++);

	lcd_sync();
}

void lcd_printf(const char *fmt, ...)
{
	va_list args;
	char buf[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	lcd_puts(buf);
}

static int do_lcd_setcursor(cmd_tbl_t *cmdtp, int flag, int argc,
			    char *const argv[])
{
	unsigned int col, row;

	if (argc != 3)
		return CMD_RET_USAGE;

	col = simple_strtoul(argv[1], NULL, 10);
	row = simple_strtoul(argv[2], NULL, 10);
	lcd_position_cursor(col, row);

	return 0;
}

static int do_lcd_puts(cmd_tbl_t *cmdtp, int flag, int argc,
		       char *const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	lcd_puts(argv[1]);

	return 0;
}

U_BOOT_CMD(
	setcurs, 3,	1,	do_lcd_setcursor,
	"set cursor position within screen",
	"    <col> <row> in character"
);

U_BOOT_CMD(
	lcdputs, 2,	1,	do_lcd_puts,
	"print string on lcd-framebuffer",
	"    <string>"
);

