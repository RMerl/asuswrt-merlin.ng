// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * (C) Copyright 2015
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 * (C) Copyright 2023 Dzmitry Sankouski <dsankouski@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <video.h>
#include <video_console.h>
#include <video_font.h>		/* Get font data, width and height */
#include "vidconsole_internal.h"

static int console_set_row(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	void *line, *dst, *end;
	int pixels = fontdata->height * vid_priv->xsize;
	int ret;
	int i;
	int pbytes;

	ret = check_bpix_support(vid_priv->bpix);
	if (ret)
		return ret;

	line = vid_priv->fb + row * fontdata->height * vid_priv->line_length;
	dst = line;
	pbytes = VNBYTES(vid_priv->bpix);
	for (i = 0; i < pixels; i++)
		fill_pixel_and_goto_next(&dst, clr, pbytes, pbytes);
	end = dst;

	ret = vidconsole_sync_copy(dev, line, end);
	if (ret)
		return ret;

	return 0;
}

static int console_move_rows(struct udevice *dev, uint rowdst,
			     uint rowsrc, uint count)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	void *dst;
	void *src;
	int size;
	int ret;

	dst = vid_priv->fb + rowdst * fontdata->height * vid_priv->line_length;
	src = vid_priv->fb + rowsrc * fontdata->height * vid_priv->line_length;
	size = fontdata->height * vid_priv->line_length * count;
	ret = vidconsole_memmove(dev, dst, src, size);
	if (ret)
		return ret;

	return 0;
}

static int console_putc_xy(struct udevice *dev, uint x_frac, uint y, char ch)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	int pbytes = VNBYTES(vid_priv->bpix);
	int x, linenum, ret;
	void *start, *line;
	uchar *pfont = fontdata->video_fontdata +
			(u8)ch * fontdata->char_pixel_bytes;

	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;
	linenum = y;
	x = VID_TO_PIXEL(x_frac);
	start = vid_priv->fb + linenum * vid_priv->line_length + x * pbytes;
	line = start;

	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;

	ret = fill_char_vertically(pfont, &line, vid_priv, fontdata, NORMAL_DIRECTION);
	if (ret)
		return ret;

	ret = vidconsole_sync_copy(dev, start, line);
	if (ret)
		return ret;

	return VID_TO_POS(fontdata->width);
}

static int console_set_cursor_visible(struct udevice *dev, bool visible,
				      uint x, uint y, uint index)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	int pbytes = VNBYTES(vid_priv->bpix);
	void *start, *line;

	/* for now, this is not used outside expo */
	if (!IS_ENABLED(CONFIG_EXPO))
		return -ENOSYS;

	x += index * fontdata->width;
	start = vid_priv->fb + y * vid_priv->line_length + x * pbytes;

	/* place the cursor 1 pixel before the start of the next char */
	x -= 1;

	line = start;
	draw_cursor_vertically(&line, vid_priv, vc_priv->y_charsize,
			       NORMAL_DIRECTION);

	return 0;
}

struct vidconsole_ops console_ops = {
	.putc_xy	= console_putc_xy,
	.move_rows	= console_move_rows,
	.set_row	= console_set_row,
	.get_font_size	= console_simple_get_font_size,
	.get_font	= console_simple_get_font,
	.select_font	= console_simple_select_font,
	.set_cursor_visible	= console_set_cursor_visible,
};

U_BOOT_DRIVER(vidconsole_normal) = {
	.name		= "vidconsole0",
	.id		= UCLASS_VIDEO_CONSOLE,
	.ops		= &console_ops,
	.probe		= console_probe,
	.priv_auto	= sizeof(struct console_simple_priv),
};
