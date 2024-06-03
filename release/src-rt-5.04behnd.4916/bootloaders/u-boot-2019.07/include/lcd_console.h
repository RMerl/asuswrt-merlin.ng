/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 */

/* By default we scroll by a single line */

struct console_t {
	short curr_col, curr_row;
	short cols, rows;
	void *fbbase;
	u32 lcdsizex, lcdsizey, lcdrot;
	void (*fp_putc_xy)(struct console_t *pcons, ushort x, ushort y, char c);
	void (*fp_console_moverow)(struct console_t *pcons,
				   u32 rowdst, u32 rowsrc);
	void (*fp_console_setrow)(struct console_t *pcons, u32 row, int clr);
};

/**
 * console_calc_rowcol() - calculate available rows / columns wihtin a given
 * screen-size based on used VIDEO_FONT.
 *
 * @pcons: Pointer to struct console_t
 * @sizex: size X of the screen in pixel
 * @sizey: size Y of the screen in pixel
 */
void console_calc_rowcol(struct console_t *pcons, u32 sizex, u32 sizey);
/**
 * lcd_init_console() - Initialize lcd console parameters
 *
 * Setup the address of console base, and the number of rows and columns the
 * console has.
 *
 * @address: Console base address
 * @vl_rows: Number of rows in the console
 * @vl_cols: Number of columns in the console
 * @vl_rot: Rotation of display in degree (0 - 90 - 180 - 270) counterlockwise
 */
void lcd_init_console(void *address, int vl_cols, int vl_rows, int vl_rot);
/**
 * lcd_set_col() - Set the number of the current lcd console column
 *
 * Set the number of the console column where the cursor is.
 *
 * @col: Column number
 */
void lcd_set_col(short col);

/**
 * lcd_set_row() - Set the number of the current lcd console row
 *
 * Set the number of the console row where the cursor is.
 *
 * @row: Row number
 */
void lcd_set_row(short row);

/**
 * lcd_position_cursor() - Position the cursor on the screen
 *
 * Position the cursor at the given coordinates on the screen.
 *
 * @col: Column number
 * @row: Row number
 */
void lcd_position_cursor(unsigned col, unsigned row);

/**
 * lcd_get_screen_rows() - Get the total number of screen rows
 *
 * @return: Number of screen rows
 */
int lcd_get_screen_rows(void);

/**
 * lcd_get_screen_columns() - Get the total number of screen columns
 *
 * @return: Number of screen columns
 */
int lcd_get_screen_columns(void);

/**
 * lcd_putc() - Print to screen a single character at the location of the cursor
 *
 * @c: The character to print
 */
void lcd_putc(const char c);

/**
 * lcd_puts() - Print to screen a string at the location of the cursor
 *
 * @s: The string to print
 */
void lcd_puts(const char *s);

/**
 * lcd_printf() - Print to screen a formatted string at location of the cursor
 *
 * @fmt: The formatted string to print
 * @...: The arguments for the formatted string
 */
void lcd_printf(const char *fmt, ...);
