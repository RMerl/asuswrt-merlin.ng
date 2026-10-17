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

static int console_set_row_1(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	int pbytes = VNBYTES(vid_priv->bpix);
	void *start, *dst, *line;
	int i, j;
	int ret;

	start = vid_priv->fb + vid_priv->line_length -
		(row + 1) * fontdata->height * pbytes;
	line = start;
	for (j = 0; j < vid_priv->ysize; j++) {
		dst = line;
		for (i = 0; i < fontdata->height; i++)
			fill_pixel_and_goto_next(&dst, clr, pbytes, pbytes);
		line += vid_priv->line_length;
	}
	ret = vidconsole_sync_copy(dev, start, line);
	if (ret)
		return ret;

	return 0;
}

static int console_move_rows_1(struct udevice *dev, uint rowdst, uint rowsrc,
				   uint count)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	int pbytes = VNBYTES(vid_priv->bpix);
	void *dst;
	void *src;
	int j, ret;

	dst = vid_priv->fb + vid_priv->line_length -
		(rowdst + count) * fontdata->height * pbytes;
	src = vid_priv->fb + vid_priv->line_length -
		(rowsrc + count) * fontdata->height * pbytes;

	for (j = 0; j < vid_priv->ysize; j++) {
		ret = vidconsole_memmove(dev, dst, src,
					fontdata->height * pbytes * count);
		if (ret)
			return ret;
		src += vid_priv->line_length;
		dst += vid_priv->line_length;
	}

	return 0;
}

static int console_putc_xy_1(struct udevice *dev, uint x_frac, uint y, char ch)
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
	linenum = VID_TO_PIXEL(x_frac) + 1;
	x = y + 1;
	start = vid_priv->fb + linenum * vid_priv->line_length - x * pbytes;
	line = start;

	ret = fill_char_horizontally(pfont, &line, vid_priv, fontdata, FLIPPED_DIRECTION);
	if (ret)
		return ret;

	/* We draw backwards from 'start, so account for the first line */
	ret = vidconsole_sync_copy(dev, start - vid_priv->line_length, line);
	if (ret)
		return ret;

	return VID_TO_POS(fontdata->width);
}


static int console_set_row_2(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	void *start, *line, *dst, *end;
	int pixels = fontdata->height * vid_priv->xsize;
	int i, ret;
	int pbytes = VNBYTES(vid_priv->bpix);

	start = vid_priv->fb + vid_priv->ysize * vid_priv->line_length -
		(row + 1) * fontdata->height * vid_priv->line_length;
	line = start;
	dst = line;
	for (i = 0; i < pixels; i++)
		fill_pixel_and_goto_next(&dst, clr, pbytes, pbytes);
	end = dst;
	ret = vidconsole_sync_copy(dev, start, end);
	if (ret)
		return ret;

	return 0;
}

static int console_move_rows_2(struct udevice *dev, uint rowdst, uint rowsrc,
			       uint count)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	void *dst;
	void *src;
	void *end;

	end = vid_priv->fb + vid_priv->ysize * vid_priv->line_length;
	dst = end - (rowdst + count) * fontdata->height *
		vid_priv->line_length;
	src = end - (rowsrc + count) * fontdata->height *
		vid_priv->line_length;
	vidconsole_memmove(dev, dst, src,
			   fontdata->height * vid_priv->line_length * count);

	return 0;
}

static int console_putc_xy_2(struct udevice *dev, uint x_frac, uint y, char ch)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	int pbytes = VNBYTES(vid_priv->bpix);
	int linenum, x, ret;
	void *start, *line;
	uchar *pfont = fontdata->video_fontdata +
			(u8)ch * fontdata->char_pixel_bytes;

	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;
	linenum = vid_priv->ysize - y - 1;
	x = vid_priv->xsize - VID_TO_PIXEL(x_frac) - 1;
	start = vid_priv->fb + linenum * vid_priv->line_length + x * pbytes;
	line = start;

	ret = fill_char_vertically(pfont, &line, vid_priv, fontdata, FLIPPED_DIRECTION);
	if (ret)
		return ret;

	/* Add 4 bytes to allow for the first pixel writen */
	ret = vidconsole_sync_copy(dev, start + 4, line);
	if (ret)
		return ret;

	return VID_TO_POS(fontdata->width);
}

static int console_set_row_3(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	int pbytes = VNBYTES(vid_priv->bpix);
	void *start, *dst, *line;
	int i, j, ret;

	start = vid_priv->fb + row * fontdata->height * pbytes;
	line = start;
	for (j = 0; j < vid_priv->ysize; j++) {
		dst = line;
		for (i = 0; i < fontdata->height; i++)
			fill_pixel_and_goto_next(&dst, clr, pbytes, pbytes);
		line += vid_priv->line_length;
	}
	ret = vidconsole_sync_copy(dev, start, line);
	if (ret)
		return ret;

	return 0;
}

static int console_move_rows_3(struct udevice *dev, uint rowdst, uint rowsrc,
			       uint count)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	int pbytes = VNBYTES(vid_priv->bpix);
	void *dst;
	void *src;
	int j, ret;

	dst = vid_priv->fb + rowdst * fontdata->height * pbytes;
	src = vid_priv->fb + rowsrc * fontdata->height * pbytes;

	for (j = 0; j < vid_priv->ysize; j++) {
		ret = vidconsole_memmove(dev, dst, src,
					fontdata->height * pbytes * count);
		if (ret)
			return ret;
		src += vid_priv->line_length;
		dst += vid_priv->line_length;
	}

	return 0;
}

static int console_putc_xy_3(struct udevice *dev, uint x_frac, uint y, char ch)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct video_fontdata *fontdata = priv->fontdata;
	int pbytes = VNBYTES(vid_priv->bpix);
	int linenum, x, ret;
	void *start, *line;
	uchar *pfont = fontdata->video_fontdata +
			(u8)ch * fontdata->char_pixel_bytes;

	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;
	x = y;
	linenum = vid_priv->ysize - VID_TO_PIXEL(x_frac) - 1;
	start = vid_priv->fb + linenum * vid_priv->line_length + y * pbytes;
	line = start;

	ret = fill_char_horizontally(pfont, &line, vid_priv, fontdata, NORMAL_DIRECTION);
	if (ret)
		return ret;
	/* Add a line to allow for the first pixels writen */
	ret = vidconsole_sync_copy(dev, start + vid_priv->line_length, line);
	if (ret)
		return ret;

	return VID_TO_POS(fontdata->width);
}

struct vidconsole_ops console_ops_1 = {
	.putc_xy	= console_putc_xy_1,
	.move_rows	= console_move_rows_1,
	.set_row	= console_set_row_1,
	.get_font_size	= console_simple_get_font_size,
	.get_font	= console_simple_get_font,
	.select_font	= console_simple_select_font,
};

struct vidconsole_ops console_ops_2 = {
	.putc_xy	= console_putc_xy_2,
	.move_rows	= console_move_rows_2,
	.set_row	= console_set_row_2,
	.get_font_size	= console_simple_get_font_size,
	.get_font	= console_simple_get_font,
	.select_font	= console_simple_select_font,
};

struct vidconsole_ops console_ops_3 = {
	.putc_xy	= console_putc_xy_3,
	.move_rows	= console_move_rows_3,
	.set_row	= console_set_row_3,
	.get_font_size	= console_simple_get_font_size,
	.get_font	= console_simple_get_font,
	.select_font	= console_simple_select_font,
};

U_BOOT_DRIVER(vidconsole_1) = {
	.name	= "vidconsole1",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_ops_1,
	.probe	= console_probe,
	.priv_auto	= sizeof(struct console_simple_priv),
};

U_BOOT_DRIVER(vidconsole_2) = {
	.name	= "vidconsole2",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_ops_2,
	.probe	= console_probe,
	.priv_auto	= sizeof(struct console_simple_priv),
};

U_BOOT_DRIVER(vidconsole_3) = {
	.name	= "vidconsole3",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_ops_3,
	.probe	= console_probe,
	.priv_auto	= sizeof(struct console_simple_priv),
};
