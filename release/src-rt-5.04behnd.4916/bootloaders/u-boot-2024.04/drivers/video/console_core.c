// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * (C) Copyright 2015
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 * (C) Copyright 2023 Dzmitry Sankouski <dsankouski@gmail.com>
 */

#include <video.h>
#include <video_console.h>
#include <dm.h>
#include <video_font.h>
#include "vidconsole_internal.h"

/**
 * console_set_font() - prepare vidconsole for chosen font.
 *
 * @dev		vidconsole device
 * @fontdata	pointer to font data struct
 */
static int console_set_font(struct udevice *dev, struct video_fontdata *fontdata)
{
	struct console_simple_priv *priv = dev_get_priv(dev);
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);

	debug("console_simple: setting %s font\n", fontdata->name);
	debug("width: %d\n", fontdata->width);
	debug("byte width: %d\n", fontdata->byte_width);
	debug("height: %d\n", fontdata->height);

	priv->fontdata = fontdata;
	vc_priv->x_charsize = fontdata->width;
	vc_priv->y_charsize = fontdata->height;
	if (vid_priv->rot % 2) {
		vc_priv->cols = vid_priv->ysize / fontdata->width;
		vc_priv->rows = vid_priv->xsize / fontdata->height;
		vc_priv->xsize_frac = VID_TO_POS(vid_priv->ysize);
	} else {
		vc_priv->cols = vid_priv->xsize / fontdata->width;
		vc_priv->rows = vid_priv->ysize / fontdata->height;
	}

	return 0;
}

int check_bpix_support(int bpix)
{
	if (bpix == VIDEO_BPP8 && CONFIG_IS_ENABLED(VIDEO_BPP8))
		return 0;
	else if (bpix == VIDEO_BPP16 && CONFIG_IS_ENABLED(VIDEO_BPP16))
		return 0;
	else if (bpix == VIDEO_BPP32 && CONFIG_IS_ENABLED(VIDEO_BPP32))
		return 0;
	else
		return -ENOSYS;
}

inline void fill_pixel_and_goto_next(void **dstp, u32 value, int pbytes, int step)
{
	u8 *dst_byte = *dstp;

	if (pbytes == 4) {
		u32 *dst = *dstp;
		*dst = value;
	}
	if (pbytes == 2) {
		u16 *dst = *dstp;
		*dst = value;
	}
	if (pbytes == 1) {
		u8 *dst = *dstp;
		*dst = value;
	}
	*dstp = dst_byte + step;
}

int fill_char_vertically(uchar *pfont, void **line, struct video_priv *vid_priv,
			 struct video_fontdata *fontdata, bool direction)
{
	int step, line_step, pbytes, bitcount, width_remainder, ret;
	void *dst;

	ret = check_bpix_support(vid_priv->bpix);
	if (ret)
		return ret;

	pbytes = VNBYTES(vid_priv->bpix);
	if (direction) {
		step = -pbytes;
		line_step = -vid_priv->line_length;
	} else {
		step = pbytes;
		line_step = vid_priv->line_length;
	}

	width_remainder = fontdata->width % 8;
	for (int row = 0; row < fontdata->height; row++) {
		uchar bits;

		bitcount = 8;
		dst = *line;
		for (int col = 0; col < fontdata->byte_width; col++) {
			if (width_remainder) {
				bool is_last_col = (fontdata->byte_width - col == 1);

				if (is_last_col)
					bitcount = width_remainder;
			}
			bits = pfont[col];

			for (int bit = 0; bit < bitcount; bit++) {
				u32 value = (bits & 0x80) ?
					vid_priv->colour_fg :
					vid_priv->colour_bg;

				fill_pixel_and_goto_next(&dst,
							 value,
							 pbytes,
							 step
				);
				bits <<= 1;
			}
		}
		*line += line_step;
		pfont += fontdata->byte_width;
	}
	return ret;
}

int fill_char_horizontally(uchar *pfont, void **line, struct video_priv *vid_priv,
			   struct video_fontdata *fontdata, bool direction)
{
	int step, line_step, pbytes, bitcount = 8, width_remainder, ret;
	void *dst;
	u8 mask;

	ret = check_bpix_support(vid_priv->bpix);
	if (ret)
		return ret;

	pbytes = VNBYTES(vid_priv->bpix);
	if (direction) {
		step = -pbytes;
		line_step = vid_priv->line_length;
	} else {
		step = pbytes;
		line_step = -vid_priv->line_length;
	}

	width_remainder = fontdata->width % 8;
	for (int col = 0; col < fontdata->byte_width; col++) {
		mask = 0x80;
		if (width_remainder) {
			bool is_last_col = (fontdata->byte_width - col == 1);

			if (is_last_col)
				bitcount = width_remainder;
		}
		for (int bit = 0; bit < bitcount; bit++) {
			dst = *line;
			for (int row = 0; row < fontdata->height; row++) {
				u32 value = (pfont[row * fontdata->byte_width + col]
					     & mask) ? vid_priv->colour_fg : vid_priv->colour_bg;

				fill_pixel_and_goto_next(&dst,
							 value,
							 pbytes,
							 step
				);
			}
			*line += line_step;
			mask >>= 1;
		}
	}
	return ret;
}

int draw_cursor_vertically(void **line, struct video_priv *vid_priv,
			   uint height, bool direction)
{
	int step, line_step, pbytes, ret;
	uint value;
	void *dst;

	ret = check_bpix_support(vid_priv->bpix);
	if (ret)
		return ret;

	pbytes = VNBYTES(vid_priv->bpix);
	if (direction) {
		step = -pbytes;
		line_step = -vid_priv->line_length;
	} else {
		step = pbytes;
		line_step = vid_priv->line_length;
	}

	value = vid_priv->colour_fg;

	for (int row = 0; row < height; row++) {
		dst = *line;
		for (int col = 0; col < VIDCONSOLE_CURSOR_WIDTH; col++)
			fill_pixel_and_goto_next(&dst, value, pbytes, step);
		*line += line_step;
	}
	return ret;
}

int console_probe(struct udevice *dev)
{
	return console_set_font(dev, fonts);
}

const char *console_simple_get_font_size(struct udevice *dev, uint *sizep)
{
	struct console_simple_priv *priv = dev_get_priv(dev);

	*sizep = priv->fontdata->width;

	return priv->fontdata->name;
}

int console_simple_get_font(struct udevice *dev, int seq, struct vidfont_info *info)
{
	info->name = fonts[seq].name;

	return info->name ? 0 : -ENOENT;
}

int console_simple_select_font(struct udevice *dev, const char *name, uint size)
{
	struct video_fontdata *font;

	if (!name) {
		if (fonts->name)
			console_set_font(dev, fonts);
		return 0;
	}

	for (font = fonts; font->name; font++) {
		if (!strcmp(name, font->name)) {
			console_set_font(dev, font);
			return 0;
		}
	};
	printf("no such font: %s, make sure it's name has <width>x<height> format\n", name);
	return -ENOENT;
}
