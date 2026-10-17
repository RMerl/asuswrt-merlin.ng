/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2023 Dzmitry Sankouski <dsankouski@gmail.com>
 */

#ifndef _VIDEO_FONT_DATA_
#define _VIDEO_FONT_DATA_
#define VIDEO_FONT_BYTE_WIDTH(width)	((width / 8) + (width % 8 > 0))
#define VIDEO_FONT_CHAR_PIXEL_BYTES(width, height)	(height * VIDEO_FONT_BYTE_WIDTH(width))
#define VIDEO_FONT_SIZE(chars, width, height)	(chars * VIDEO_FONT_CHAR_PIXEL_BYTES(width, height))

struct video_fontdata {
	const char *name;
	int width;
	int height;
	int byte_width;
	int char_pixel_bytes;
	unsigned char *video_fontdata;
};

#define FONT_ENTRY(_font_width, _font_height, _width_x_height) \
{	\
	.name = #_width_x_height,	\
	.width = _font_width,		\
	.height = _font_height,		\
	.byte_width = VIDEO_FONT_BYTE_WIDTH(_font_width),	\
	.char_pixel_bytes = VIDEO_FONT_CHAR_PIXEL_BYTES(_font_width, _font_height),	\
	.video_fontdata = video_fontdata_##_width_x_height,	\
}

#endif /* _VIDEO_FONT_DATA_ */
