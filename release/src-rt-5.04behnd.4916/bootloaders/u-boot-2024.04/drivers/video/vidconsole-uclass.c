// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * (C) Copyright 2001-2015
 * DENX Software Engineering -- wd@denx.de
 * Compulab Ltd - http://compulab.co.il/
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 */

#define LOG_CATEGORY UCLASS_VIDEO_CONSOLE

#include <common.h>
#include <abuf.h>
#include <command.h>
#include <console.h>
#include <log.h>
#include <dm.h>
#include <video.h>
#include <video_console.h>
#include <video_font.h>		/* Bitmap font for code page 437 */
#include <linux/ctype.h>

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

int vidconsole_entry_start(struct udevice *dev)
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
	return video_sync(dev->parent, false);
}

/* Move to a newline, scrolling the display if necessary */
static void vidconsole_newline(struct udevice *dev)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct udevice *vid_dev = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);
	const int rows = CONFIG_VAL(CONSOLE_SCROLL_LINES);
	int i, ret;

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

	ret = video_sync(dev->parent, false);
	if (ret) {
#ifdef DEBUG
		console_puts_select_stderr(true, "[vc err: video_sync]");
#endif
	}
}

static char *parsenum(char *s, int *num)
{
	char *end;
	*num = simple_strtol(s, &end, 10);
	return end;
}

void vidconsole_set_cursor_pos(struct udevice *dev, int x, int y)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);

	priv->xcur_frac = VID_TO_POS(x);
	priv->xstart_frac = priv->xcur_frac;
	priv->ycur = y;
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
			int ret;

			video_clear(dev->parent);
			ret = video_sync(dev->parent, false);
			if (ret) {
#ifdef DEBUG
				console_puts_select_stderr(true, "[vc err: video_sync]");
#endif
			}
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
				vid_priv->colour_fg = video_index_to_colour(
						vid_priv, vid_priv->fg_col_idx);
				break;
			case 7:
				/* reverse video */
				vid_priv->colour_fg = video_index_to_colour(
						vid_priv, vid_priv->bg_col_idx);
				vid_priv->colour_bg = video_index_to_colour(
						vid_priv, vid_priv->fg_col_idx);
				break;
			case 30 ... 37:
				/* foreground color */
				vid_priv->fg_col_idx &= ~7;
				vid_priv->fg_col_idx |= val - 30;
				vid_priv->colour_fg = video_index_to_colour(
						vid_priv, vid_priv->fg_col_idx);
				break;
			case 40 ... 47:
				/* background color, also mask the bold bit */
				vid_priv->bg_col_idx &= ~0xf;
				vid_priv->bg_col_idx |= val - 40;
				vid_priv->colour_bg = video_index_to_colour(
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
	int ret;

	ret = vidconsole_put_char(dev, ch);
	if (ret) {
#ifdef DEBUG
		console_puts_select_stderr(true, "[vc err: putc]");
#endif
	}
	ret = video_sync(dev->parent, false);
	if (ret) {
#ifdef DEBUG
		console_puts_select_stderr(true, "[vc err: video_sync]");
#endif
	}
}

static void vidconsole_puts(struct stdio_dev *sdev, const char *s)
{
	struct udevice *dev = sdev->priv;
	int ret;

	ret = vidconsole_put_string(dev, s);
	if (ret) {
#ifdef DEBUG
		char str[30];

		snprintf(str, sizeof(str), "[vc err: puts %d]", ret);
		console_puts_select_stderr(true, str);
#endif
	}
	ret = video_sync(dev->parent, false);
	if (ret) {
#ifdef DEBUG
		console_puts_select_stderr(true, "[vc err: video_sync]");
#endif
	}
}

void vidconsole_list_fonts(struct udevice *dev)
{
	struct vidfont_info info;
	int ret, i;

	for (i = 0, ret = 0; !ret; i++) {
		ret = vidconsole_get_font(dev, i, &info);
		if (!ret)
			printf("%s\n", info.name);
	}
}

int vidconsole_get_font(struct udevice *dev, int seq,
			struct vidfont_info *info)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);

	if (!ops->get_font)
		return -ENOSYS;

	return ops->get_font(dev, seq, info);
}

int vidconsole_get_font_size(struct udevice *dev, const char **name, uint *sizep)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);

	if (!ops->get_font_size)
		return -ENOSYS;

	*name = ops->get_font_size(dev, sizep);
	return 0;
}

int vidconsole_select_font(struct udevice *dev, const char *name, uint size)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);

	if (!ops->select_font)
		return -ENOSYS;

	return ops->select_font(dev, name, size);
}

int vidconsole_measure(struct udevice *dev, const char *name, uint size,
		       const char *text, struct vidconsole_bbox *bbox)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);
	int ret;

	if (ops->measure) {
		ret = ops->measure(dev, name, size, text, bbox);
		if (ret != -ENOSYS)
			return ret;
	}

	bbox->valid = true;
	bbox->x0 = 0;
	bbox->y0 = 0;
	bbox->x1 = priv->x_charsize * strlen(text);
	bbox->y1 = priv->y_charsize;

	return 0;
}

int vidconsole_nominal(struct udevice *dev, const char *name, uint size,
		       uint num_chars, struct vidconsole_bbox *bbox)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);
	int ret;

	if (ops->measure) {
		ret = ops->nominal(dev, name, size, num_chars, bbox);
		if (ret != -ENOSYS)
			return ret;
	}

	bbox->valid = true;
	bbox->x0 = 0;
	bbox->y0 = 0;
	bbox->x1 = priv->x_charsize * num_chars;
	bbox->y1 = priv->y_charsize;

	return 0;
}

int vidconsole_entry_save(struct udevice *dev, struct abuf *buf)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);
	int ret;

	if (ops->measure) {
		ret = ops->entry_save(dev, buf);
		if (ret != -ENOSYS)
			return ret;
	}

	/* no data so make sure the buffer is empty */
	abuf_realloc(buf, 0);

	return 0;
}

int vidconsole_entry_restore(struct udevice *dev, struct abuf *buf)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);
	int ret;

	if (ops->measure) {
		ret = ops->entry_restore(dev, buf);
		if (ret != -ENOSYS)
			return ret;
	}

	return 0;
}

int vidconsole_set_cursor_visible(struct udevice *dev, bool visible,
				  uint x, uint y, uint index)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);
	int ret;

	if (ops->set_cursor_visible) {
		ret = ops->set_cursor_visible(dev, visible, x, y, index);
		if (ret != -ENOSYS)
			return ret;
	}

	return 0;
}

void vidconsole_push_colour(struct udevice *dev, enum colour_idx fg,
			    enum colour_idx bg, struct vidconsole_colour *old)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);

	old->colour_fg = vid_priv->colour_fg;
	old->colour_bg = vid_priv->colour_bg;

	vid_priv->colour_fg = video_index_to_colour(vid_priv, fg);
	vid_priv->colour_bg = video_index_to_colour(vid_priv, bg);
}

void vidconsole_pop_colour(struct udevice *dev, struct vidconsole_colour *old)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);

	vid_priv->colour_fg = old->colour_fg;
	vid_priv->colour_bg = old->colour_bg;
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

	if (dev_seq(dev)) {
		snprintf(sdev->name, sizeof(sdev->name), "vidconsole%d",
			 dev_seq(dev));
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
	.per_device_auto	= sizeof(struct vidconsole_priv),
};

#ifdef CONFIG_VIDEO_COPY
int vidconsole_sync_copy(struct udevice *dev, void *from, void *to)
{
	struct udevice *vid = dev_get_parent(dev);

	return video_sync_copy(vid, from, to);
}

int vidconsole_memmove(struct udevice *dev, void *dst, const void *src,
		       int size)
{
	memmove(dst, src, size);
	return vidconsole_sync_copy(dev, dst, dst + size);
}
#endif

int vidconsole_clear_and_reset(struct udevice *dev)
{
	int ret;

	ret = video_clear(dev_get_parent(dev));
	if (ret)
		return ret;
	vidconsole_position_cursor(dev, 0, 0);

	return 0;
}

void vidconsole_position_cursor(struct udevice *dev, unsigned col, unsigned row)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct udevice *vid_dev = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);
	short x, y;

	x = min_t(short, col * priv->x_charsize, vid_priv->xsize - 1);
	y = min_t(short, row * priv->y_charsize, vid_priv->ysize - 1);
	vidconsole_set_cursor_pos(dev, x, y);
}
