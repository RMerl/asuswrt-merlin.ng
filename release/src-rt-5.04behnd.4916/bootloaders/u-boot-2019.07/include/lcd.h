/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MPC823 and PXA LCD Controller
 *
 * Modeled after video interface by Paolo Scaffardi
 *
 *
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _LCD_H_
#define _LCD_H_
#include <lcd_console.h>
#if defined(CONFIG_CMD_BMP) || defined(CONFIG_SPLASH_SCREEN)
#include <bmp_layout.h>
#include <asm/byteorder.h>
#endif

int bmp_display(ulong addr, int x, int y);
struct bmp_image *gunzip_bmp(unsigned long addr, unsigned long *lenp,
			     void **alloc_addr);

#ifndef CONFIG_DM_VIDEO

extern char lcd_is_enabled;
extern int lcd_line_length;
extern struct vidinfo panel_info;

void lcd_ctrl_init(void *lcdbase);
void lcd_enable(void);
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue);

/**
 * Set whether we need to flush the dcache when changing the LCD image. This
 * defaults to off.
 *
 * @param flush		non-zero to flush cache after update, 0 to skip
 */
void lcd_set_flush_dcache(int flush);

#if defined(CONFIG_CPU_PXA25X) || defined(CONFIG_CPU_PXA27X) || \
	defined CONFIG_CPU_MONAHANS
#include <pxa_lcd.h>
#elif defined(CONFIG_ATMEL_LCD) || defined(CONFIG_ATMEL_HLCD)
#include <atmel_lcd.h>
#elif defined(CONFIG_EXYNOS_FB)
#include <exynos_lcd.h>
#else
typedef struct vidinfo {
	ushort	vl_col;		/* Number of columns (i.e. 160) */
	ushort	vl_row;		/* Number of rows (i.e. 100) */
	ushort	vl_rot;		/* Rotation of Display (0, 1, 2, 3) */
	u_char	vl_bpix;	/* Bits per pixel, 0 = 1 */
	ushort	*cmap;		/* Pointer to the colormap */
	void	*priv;		/* Pointer to driver-specific data */
} vidinfo_t;

static __maybe_unused ushort *configuration_get_cmap(void)
{
	return panel_info.cmap;
}
#endif

ushort *configuration_get_cmap(void);

extern vidinfo_t panel_info;

void lcd_putc(const char c);
void lcd_puts(const char *s);
void lcd_printf(const char *fmt, ...);
void lcd_clear(void);
int lcd_display_bitmap(ulong bmp_image, int x, int y);

/**
 * Get the width of the LCD in pixels
 *
 * @return width of LCD in pixels
 */
int lcd_get_pixel_width(void);

/**
 * Get the height of the LCD in pixels
 *
 * @return height of LCD in pixels
 */
int lcd_get_pixel_height(void);

/**
 * Get the number of text lines/rows on the LCD
 *
 * @return number of rows
 */
int lcd_get_screen_rows(void);

/**
 * Get the number of text columns on the LCD
 *
 * @return number of columns
 */
int lcd_get_screen_columns(void);

/**
 * Get the background color of the LCD
 *
 * @return background color value
 */
int lcd_getbgcolor(void);

/**
 * Get the foreground color of the LCD
 *
 * @return foreground color value
 */
int lcd_getfgcolor(void);

/**
 * Set the position of the text cursor
 *
 * @param col	Column to place cursor (0 = left side)
 * @param row	Row to place cursor (0 = top line)
 */
void lcd_position_cursor(unsigned col, unsigned row);

/* Allow boards to customize the information displayed */
void lcd_show_board_info(void);

/* Return the size of the LCD frame buffer, and the line length */
int lcd_get_size(int *line_length);

/* Update the LCD / flush the cache */
void lcd_sync(void);

/*
 *  Information about displays we are using. This is for configuring
 *  the LCD controller and memory allocation. Someone has to know what
 *  is connected, as we can't autodetect anything.
 */
#define CONFIG_SYS_HIGH	0	/* Pins are active high			*/
#define CONFIG_SYS_LOW	1	/* Pins are active low			*/

#define LCD_MONOCHROME	0
#define LCD_COLOR2	1
#define LCD_COLOR4	2
#define LCD_COLOR8	3
#define LCD_COLOR16	4
#define LCD_COLOR32	5

#if defined(CONFIG_LCD_INFO_BELOW_LOGO)
#define LCD_INFO_X		0
#define LCD_INFO_Y		(BMP_LOGO_HEIGHT + VIDEO_FONT_HEIGHT)
#elif defined(CONFIG_LCD_LOGO)
#define LCD_INFO_X		(BMP_LOGO_WIDTH + 4 * VIDEO_FONT_WIDTH)
#define LCD_INFO_Y		VIDEO_FONT_HEIGHT
#else
#define LCD_INFO_X		VIDEO_FONT_WIDTH
#define LCD_INFO_Y		VIDEO_FONT_HEIGHT
#endif

/* Default to 8bpp if bit depth not specified */
#ifndef LCD_BPP
#define LCD_BPP			LCD_COLOR8
#endif

#ifndef LCD_DF
#define LCD_DF			1
#endif

/* Calculate nr. of bits per pixel  and nr. of colors */
#define NBITS(bit_code)		(1 << (bit_code))
#define NCOLORS(bit_code)	(1 << NBITS(bit_code))

#if LCD_BPP == LCD_COLOR8
# define CONSOLE_COLOR_BLACK	0
# define CONSOLE_COLOR_RED	1
# define CONSOLE_COLOR_GREEN	2
# define CONSOLE_COLOR_YELLOW	3
# define CONSOLE_COLOR_BLUE	4
# define CONSOLE_COLOR_MAGENTA	5
# define CONSOLE_COLOR_CYAN	6
# define CONSOLE_COLOR_GREY	14
# define CONSOLE_COLOR_WHITE	15		/* Must remain last / highest */
#elif LCD_BPP == LCD_COLOR32
#define CONSOLE_COLOR_RED	0x00ff0000
#define CONSOLE_COLOR_GREEN	0x0000ff00
#define CONSOLE_COLOR_YELLOW	0x00ffff00
#define CONSOLE_COLOR_BLUE	0x000000ff
#define CONSOLE_COLOR_MAGENTA	0x00ff00ff
#define CONSOLE_COLOR_CYAN	0x0000ffff
#define CONSOLE_COLOR_GREY	0x00aaaaaa
#define CONSOLE_COLOR_BLACK	0x00000000
#define CONSOLE_COLOR_WHITE	0x00ffffff	/* Must remain last / highest */
#define NBYTES(bit_code)	(NBITS(bit_code) >> 3)
#else /* 16bpp color definitions */
# define CONSOLE_COLOR_BLACK	0x0000
# define CONSOLE_COLOR_RED	0xF800
# define CONSOLE_COLOR_GREEN	0x07E0
# define CONSOLE_COLOR_YELLOW	0xFFE0
# define CONSOLE_COLOR_BLUE	0x001F
# define CONSOLE_COLOR_MAGENTA	0xF81F
# define CONSOLE_COLOR_CYAN	0x07FF
# define CONSOLE_COLOR_GREY	0xC618
# define CONSOLE_COLOR_WHITE	0xffff		/* Must remain last / highest */
#endif /* color definitions */

#if LCD_BPP == LCD_COLOR16
#define fbptr_t ushort
#elif LCD_BPP == LCD_COLOR32
#define fbptr_t u32
#else
#define fbptr_t uchar
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE	4096
#endif

#endif /* !CONFIG_DM_VIDEO */

#endif	/* _LCD_H_ */
