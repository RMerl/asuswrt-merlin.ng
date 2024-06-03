// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * (C) Copyright 2001-2015
 * DENX Software Engineering -- wd@denx.de
 * Compulab Ltd - http://compulab.co.il/
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 */

#include <common.h>
#include <linux/ctype.h>
#include <dm.h>
#include <video.h>
#include <video_console.h>
#include <video_font.h>		/* Bitmap font for code page 437 */

/*
 * Structure to describe a console color
 */
struct vid_rgb {
	u32 r;
	u32 g;
	u32 b;
};

/* By default we scroll by a single line */
#ifndef CONFIG_CONSOLE_SCROLL_LINES
#define CONFIG_CONSOLE_SCROLL_LINES 1
#endif

int vidconsole_putc_xy(struct udevice *dev, uint x, uint y, char ch)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);

	if (!ops->putc_xy)
		return -ENOSYS;
	return ops->putc_xy(dev, x, y, ch);
}

int vidconsole_move_rows(struct udevice *dev, uint rowdst, uint rowsrc,
			 uint count)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);

	if (!ops->move_rows)
		return -ENOSYS;
	return ops->move_rows(dev, rowdst, rowsrc, count);
}

int vidconsole_set_row(struct udevice *dev, uint row, int clr)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);

	if (!ops->set_row)
		return -ENOSYS;
	return ops->set_row(dev, row, clr);
}

static int vidconsole_entry_start(struct udevice *dev)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);

	if (!ops->entry_start)
		return -ENOSYS;
	return ops->entry_start(dev);
}

/* Move backwards one space */
static int vidconsole_back(struct udevice *dev)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);
	int ret;

	if (ops->backspace) {
		ret = ops->backspace(dev);
		if (ret != -ENOSYS)
			return ret;
	}

	priv->xcur_frac -= VID_TO_POS(priv->x_charsize);
	if (priv->xcur_frac < priv->xstart_frac) {
		priv->xcur_frac = (priv->cols - 1) *
			VID_TO_POS(priv->x_charsize);
		priv->ycur -= priv->y_charsize;
		if (priv->ycur < 0)
			priv->ycur = 0;
	}
	video_sync(dev->parent, false);

	return 0;
}

/* Move to a newline, scrolling the display if necessary */
static void vidconsole_newline(struct udevice *dev)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct udevice *vid_dev = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);
	const int rows = CONFIG_CONSOLE_SCROLL_LINES;
	int i;

	priv->xcur_frac = priv->xstart_frac;
	priv->ycur += priv->y_charsize;

	/* Check if we need to scroll the terminal */
	if ((priv->ycur + priv->y_charsize) / priv->y_charsize > priv->rows) {
		vidconsole_move_rows(dev, 0, rows, priv->rows - rows);
		for (i = 0; i < rows; i++)
			vidconsole_set_row(dev, priv->rows - i - 1,
					   vid_priv->colour_bg);
		priv->ycur -= rows * priv->y_charsize;
	}
	priv->last_ch = 0;

	video_sync(dev->parent, false);
}

static const struct vid_rgb colors[VID_COLOR_COUNT] = {
	{ 0x00, 0x00, 0x00 },  /* black */
	{ 0xc0, 0x00, 0x00 },  /* red */
	{ 0x00, 0xc0, 0x00 },  /* green */
	{ 0xc0, 0x60, 0x00 },  /* brown */
	{ 0x00, 0x00, 0xc0 },  /* blue */
	{ 0xc0, 0x00, 0xc0 },  /* magenta */
	{ 0x00, 0xc0, 0xc0 },  /* cyan */
	{ 0xc0, 0xc0, 0xc0 },  /* light gray */
	{ 0x80, 0x80, 0x80 },  /* gray */
	{ 0xff, 0x00, 0x00 },  /* bright red */
	{ 0x00, 0xff, 0x00 },  /* bright green */
	{ 0xff, 0xff, 0x00 },  /* yellow */
	{ 0x00, 0x00, 0xff },  /* bright blue */
	{ 0xff, 0x00, 0xff },  /* bright magenta */
	{ 0x00, 0xff, 0xff },  /* bright cyan */
	{ 0xff, 0xff, 0xff },  /* white */
};

u32 vid_console_color(struct video_priv *priv, unsigned int idx)
{
	switch (priv->bpix) {
	case VIDEO_BPP16:
		return ((colors[idx].r >> 3) << 11) |
		       ((colors[idx].g >> 2) <<  5) |
		       ((colors[idx].b >> 3) <<  0);
	case VIDEO_BPP32:
		return (colors[idx].r << 16) |
		       (colors[idx].g <<  8) |
		       (colors[idx].b <<  0);
	default:
		/*
		 * For unknown bit arrangements just support
		 * black and white.
		 */
		if (idx)
			return 0xffffff; /* white */
		else
			return 0x000000; /* black */
	}
}

static char *parsenum(char *s, int *num)
{
	char *end;
	*num = simple_strtol(s, &end, 10);
	return end;
}

/**
 * set_cursor_position() - set cursor position
 *
 * @priv:	private data of the video console
 * @row:	new row
 * @col:	new column
 */
static void set_cursor_position(struct vidconsole_priv *priv, int row, int col)
{
	/*
	 * Ensure we stay in the bounds of the screen.
	 */
	if (row >= priv->rows)
		row = priv->rows - 1;
	if (col >= priv->cols)
		col = priv->cols - 1;

	priv->ycur = row * priv->y_charsize;
	priv->xcur_frac = priv->xstart_frac +
			  VID_TO_POS(col * priv->x_charsize);
}

/**
 * get_cursor_position() - get cursor position
 *
 * @priv:	private data of the video console
 * @row:	row
 * @col:	column
 */
static void get_cursor_position(struct vidconsole_priv *priv,
				int *row, int *col)
{
	*row = priv->ycur / priv->y_charsize;
	*col = VID_TO_PIXEL(priv->xcur_frac - priv->xstart_frac) /
	       priv->x_charsize;
}

/*
 * Process a character while accumulating an escape string.  Chars are
 * accumulated into escape_buf until the end of escape sequence is
 * found, at which point the sequence is parsed and processed.
 */
static void vidconsole_escape_char(struct udevice *dev, char ch)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);

	if (!IS_ENABLED(CONFIG_VIDEO_ANSI))
		goto error;

	/* Sanity checking for bogus ESC sequences: */
	if (priv->escape_len >= sizeof(priv->escape_buf))
		goto error;
	if (priv->escape_len == 0) {
		switch (ch) {
		case '7':
			/* Save cursor position */
			get_cursor_position(priv, &priv->row_saved,
					    &priv->col_saved);
			priv->escape = 0;

			return;
		case '8': {
			/* Restore cursor position */
			int row = priv->row_saved;
			int col = priv->col_saved;

			set_cursor_position(priv, row, col);
			priv->escape = 0;
			return;
		}
		case '[':
			break;
		default:
			goto error;
		}
	}

	priv->escape_buf[priv->escape_len++] = ch;

	/*
	 * Escape sequences are terminated by a letter, so keep
	 * accumulating until we get one:
	 */
	if (!isalpha(ch))
		return;

	/*
	 * clear escape mode first, otherwise things will get highly
	 * surprising if you hit any debug prints that come back to
	 * this console.
	 */
	priv->escape = 0;

	switch (ch) {
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F': {
		int row, col, num;
		char *s = priv->escape_buf;

		/*
		 * Cursor up/down: [%dA, [%dB, [%dE, [%dF
		 * Cursor left/right: [%dD, [%dC
		 */
		s++;    /* [ */
		s = parsenum(s, &num);
		if (num == 0)			/* No digit in sequence ... */
			num = 1;		/* ... means "move by 1". */

		get_cursor_position(priv, &row, &col);
		if (ch == 'A' || ch == 'F')
			row -= num;
		if (ch == 'C')
			col += num;
		if (ch == 'D')
			col -= num;
		if (ch == 'B' || ch == 'E')
			row += num;
		if (ch == 'E' || ch == 'F')
			col = 0;
		if (col < 0)
			col = 0;
		if (row < 0)
			row = 0;
		/* Right and bottom overflows are handled in the callee. */
		set_cursor_position(priv, row, col);
		break;
	}
	case 'H':
	case 'f': {
		int row, col;
		char *s = priv->escape_buf;

		/*
		 * Set cursor position: [%d;%df or [%d;%dH
		 */
		s++;    /* [ */
		s = parsenum(s, &row);
		s++;    /* ; */
		s = parsenum(s, &col);

		/*
		 * Video origin is [0, 0], terminal origin is [1, 1].
		 */
		if (row)
			--row;
		if (col)
			--col;

		set_cursor_position(priv, row, col);

		break;
	}
	case 'J': {
		int mode;

		/*
		 * Clear part/all screen:
		 *   [J or [0J - clear screen from cursor down
		 *   [1J       - clear screen from cursor up
		 *   [2J       - clear entire screen
		 *
		 * TODO we really only handle entire-screen case, others
		 * probably require some additions to video-uclass (and
		 * are not really needed yet by efi_console)
		 */
		parsenum(priv->escape_buf + 1, &mode);

		if (mode == 2) {
			video_clear(dev->parent);
			video_sync(dev->parent, false);
			priv->ycur = 0;
			priv->xcur_frac = priv->xstart_frac;
		} else {
			debug("unsupported clear mode: %d\n", mode);
		}
		break;
	}
	case 'K': {
		struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
		int mode;

		/*
		 * Clear (parts of) current line
		 *   [0K       - clear line to end
		 *   [2K       - clear entire line
		 */
		parsenum(priv->escape_buf + 1, &mode);

		if (mode == 2) {
			int row, col;

			get_cursor_position(priv, &row, &col);
			vidconsole_set_row(dev, row, vid_priv->colour_bg);
		}
		break;
	}
	case 'm': {
		struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
		char *s = priv->escape_buf;
		char *end = &priv->escape_buf[priv->escape_len];

		/*
		 * Set graphics mode: [%d;...;%dm
		 *
		 * Currently only supports the color attributes:
		 *
		 * Foreground Colors:
		 *
		 *   30	Black
		 *   31	Red
		 *   32	Green
		 *   33	Yellow
		 *   34	Blue
		 *   35	Magenta
		 *   36	Cyan
		 *   37	White
		 *
		 * Background Colors:
		 *
		 *   40	Black
		 *   41	Red
		 *   42	Green
		 *   43	Yellow
		 *   44	Blue
		 *   45	Magenta
		 *   46	Cyan
		 *   47	White
		 */

		s++;    /* [ */
		while (s < end) {
			int val;

			s = parsenum(s, &val);
			s++;

			switch (val) {
			case 0:
				/* all attributes off */
				video_set_default_colors(dev->parent, false);
				break;
			case 1:
				/* bold */
				vid_priv->fg_col_idx |= 8;
				vid_priv->colour_fg = vid_console_color(
						vid_priv, vid_priv->fg_col_idx);
				break;
			case 7:
				/* reverse video */
				vid_priv->colour_fg = vid_console_color(
						vid_priv, vid_priv->bg_col_idx);
				vid_priv->colour_bg = vid_console_color(
						vid_priv, vid_priv->fg_col_idx);
				break;
			case 30 ... 37:
				/* foreground color */
				vid_priv->fg_col_idx &= ~7;
				vid_priv->fg_col_idx |= val - 30;
				vid_priv->colour_fg = vid_console_color(
						vid_priv, vid_priv->fg_col_idx);
				break;
			case 40 ... 47:
				/* background color, also mask the bold bit */
				vid_priv->bg_col_idx &= ~0xf;
				vid_priv->bg_col_idx |= val - 40;
				vid_priv->colour_bg = vid_console_color(
						vid_priv, vid_priv->bg_col_idx);
				break;
			default:
				/* ignore unsupported SGR parameter */
				break;
			}
		}

		break;
	}
	default:
		debug("unrecognized escape sequence: %*s\n",
		      priv->escape_len, priv->escape_buf);
	}

	return;

error:
	/* something went wrong, just revert to normal mode: */
	priv->escape = 0;
}

/* Put that actual character on the screen (using the CP437 code page). */
static int vidconsole_output_glyph(struct udevice *dev, char ch)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	int ret;

	/*
	 * Failure of this function normally indicates an unsupported
	 * colour depth. Check this and return an error to help with
	 * diagnosis.
	 */
	ret = vidconsole_putc_xy(dev, priv->xcur_frac, priv->ycur, ch);
	if (ret == -EAGAIN) {
		vidconsole_newline(dev);
		ret = vidconsole_putc_xy(dev, priv->xcur_frac, priv->ycur, ch);
	}
	if (ret < 0)
		return ret;
	priv->xcur_frac += ret;
	priv->last_ch = ch;
	if (priv->xcur_frac >= priv->xsize_frac)
		vidconsole_newline(dev);

	return 0;
}

int vidconsole_put_char(struct udevice *dev, char ch)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	int ret;

	if (priv->escape) {
		vidconsole_escape_char(dev, ch);
		return 0;
	}

	switch (ch) {
	case '\x1b':
		priv->escape_len = 0;
		priv->escape = 1;
		break;
	case '\a':
		/* beep */
		break;
	case '\r':
		priv->xcur_frac = priv->xstart_frac;
		break;
	case '\n':
		vidconsole_newline(dev);
		vidconsole_entry_start(dev);
		break;
	case '\t':	/* Tab (8 chars alignment) */
		priv->xcur_frac = ((priv->xcur_frac / priv->tab_width_frac)
				+ 1) * priv->tab_width_frac;

		if (priv->xcur_frac >= priv->xsize_frac)
			vidconsole_newline(dev);
		break;
	case '\b':
		vidconsole_back(dev);
		priv->last_ch = 0;
		break;
	default:
		ret = vidconsole_output_glyph(dev, ch);
		if (ret < 0)
			return ret;
		break;
	}

	return 0;
}

int vidconsole_put_string(struct udevice *dev, const char *str)
{
	const char *s;
	int ret;

	for (s = str; *s; s++) {
		ret = vidconsole_put_char(dev, *s);
		if (ret)
			return ret;
	}

	return 0;
}

static void vidconsole_putc(struct stdio_dev *sdev, const char ch)
{
	struct udevice *dev = sdev->priv;

	vidconsole_put_char(dev, ch);
	video_sync(dev->parent, false);
}

static void vidconsole_puts(struct stdio_dev *sdev, const char *s)
{
	struct udevice *dev = sdev->priv;

	vidconsole_put_string(dev, s);
	video_sync(dev->parent, false);
}

/* Set up the number of rows and colours (rotated drivers override this) */
static int vidconsole_pre_probe(struct udevice *dev)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);

	priv->xsize_frac = VID_TO_POS(vid_priv->xsize);

	return 0;
}

/* Register the device with stdio */
static int vidconsole_post_probe(struct udevice *dev)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct stdio_dev *sdev = &priv->sdev;

	if (!priv->tab_width_frac)
		priv->tab_width_frac = VID_TO_POS(priv->x_charsize) * 8;

	if (dev->seq) {
		snprintf(sdev->name, sizeof(sdev->name), "vidconsole%d",
			 dev->seq);
	} else {
		strcpy(sdev->name, "vidconsole");
	}

	sdev->flags = DEV_FLAGS_OUTPUT;
	sdev->putc = vidconsole_putc;
	sdev->puts = vidconsole_puts;
	sdev->priv = dev;

	return stdio_register(sdev);
}

UCLASS_DRIVER(vidconsole) = {
	.id		= UCLASS_VIDEO_CONSOLE,
	.name		= "vidconsole0",
	.pre_probe	= vidconsole_pre_probe,
	.post_probe	= vidconsole_post_probe,
	.per_device_auto_alloc_size	= sizeof(struct vidconsole_priv),
};

void vidconsole_position_cursor(struct udevice *dev, unsigned col, unsigned row)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct udevice *vid_dev = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);

	col *= priv->x_charsize;
	row *= priv->y_charsize;
	priv->xcur_frac = VID_TO_POS(min_t(short, col, vid_priv->xsize - 1));
	priv->ycur = min_t(short, row, vid_priv->ysize - 1);
}

static int do_video_setcursor(cmd_tbl_t *cmdtp, int flag, int argc,
			      char *const argv[])
{
	unsigned int col, row;
	struct udevice *dev;

	if (argc != 3)
		return CMD_RET_USAGE;

	if (uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev))
		return CMD_RET_FAILURE;
	col = simple_strtoul(argv[1], NULL, 10);
	row = simple_strtoul(argv[2], NULL, 10);
	vidconsole_position_cursor(dev, col, row);

	return 0;
}

static int do_video_puts(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct udevice *dev;
	const char *s;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev))
		return CMD_RET_FAILURE;
	for (s = argv[1]; *s; s++)
		vidconsole_put_char(dev, *s);

	video_sync(dev->parent, false);

	return 0;
}

U_BOOT_CMD(
	setcurs, 3,	1,	do_video_setcursor,
	"set cursor position within screen",
	"    <col> <row> in character"
);

U_BOOT_CMD(
	lcdputs, 2,	1,	do_video_puts,
	"print string on video framebuffer",
	"    <string>"
);
