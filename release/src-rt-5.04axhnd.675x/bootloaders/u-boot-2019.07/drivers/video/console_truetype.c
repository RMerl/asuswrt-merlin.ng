// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <video.h>
#include <video_console.h>

/* Functions needed by stb_truetype.h */
static int tt_floor(double val)
{
	if (val < 0)
		return (int)(val - 0.999);

	return (int)val;
}

static int tt_ceil(double val)
{
	if (val < 0)
		return (int)val;

	return (int)(val + 0.999);
}

static double frac(double val)
{
	return val - tt_floor(val);
}

static double tt_fabs(double x)
{
	return x < 0 ? -x : x;
}

 /*
  * Simple square root algorithm. This is from:
  * http://stackoverflow.com/questions/1623375/writing-your-own-square-root-function
  * Written by Chihung Yu
  * Creative Commons license
  * http://creativecommons.org/licenses/by-sa/3.0/legalcode
  * It has been modified to compile correctly, and for U-Boot style.
  */
static double tt_sqrt(double value)
{
	double lo = 1.0;
	double hi = value;

	while (hi - lo > 0.00001) {
		double mid = lo + (hi - lo) / 2;

		if (mid * mid - value > 0.00001)
			hi = mid;
		else
			lo = mid;
	}

	return lo;
}

#define STBTT_ifloor		tt_floor
#define STBTT_iceil		tt_ceil
#define STBTT_fabs		tt_fabs
#define STBTT_sqrt		tt_sqrt
#define STBTT_malloc(size, u)	((void)(u), malloc(size))
#define STBTT_free(size, u)	((void)(u), free(size))
#define STBTT_assert(x)
#define STBTT_strlen(x)		strlen(x)
#define STBTT_memcpy		memcpy
#define STBTT_memset		memset

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

/**
 * struct pos_info - Records a cursor position
 *
 * @xpos_frac:	Fractional X position in pixels (multiplied by VID_FRAC_DIV)
 * @ypos:	Y position (pixels from the top)
 */
struct pos_info {
	int xpos_frac;
	int ypos;
};

/*
 * Allow one for each character on the command line plus one for each newline.
 * This is just an estimate, but it should not be exceeded.
 */
#define POS_HISTORY_SIZE	(CONFIG_SYS_CBSIZE * 11 / 10)

/**
 * struct console_tt_priv - Private data for this driver
 *
 * @font_size:	Vertical font size in pixels
 * @font_data:	Pointer to TrueType font file contents
 * @font:	TrueType font information for the current font
 * @pos:	List of cursor positions for each character written. This is
 *		used to handle backspace. We clear the frame buffer between
 *		the last position and the current position, thus erasing the
 *		last character. We record enough characters to go back to the
 *		start of the current command line.
 * @pos_ptr:	Current position in the position history
 * @baseline:	Pixel offset of the font's baseline from the cursor position.
 *		This is the 'ascent' of the font, scaled to pixel coordinates.
 *		It measures the distance from the baseline to the top of the
 *		font.
 * @scale:	Scale of the font. This is calculated from the pixel height
 *		of the font. It is used by the STB library to generate images
 *		of the correct size.
 */
struct console_tt_priv {
	int font_size;
	u8 *font_data;
	stbtt_fontinfo font;
	struct pos_info pos[POS_HISTORY_SIZE];
	int pos_ptr;
	int baseline;
	double scale;
};

static int console_truetype_set_row(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_tt_priv *priv = dev_get_priv(dev);
	void *line;
	int pixels = priv->font_size * vid_priv->line_length;
	int i;

	line = vid_priv->fb + row * priv->font_size * vid_priv->line_length;
	switch (vid_priv->bpix) {
#ifdef CONFIG_VIDEO_BPP8
	case VIDEO_BPP8: {
		uint8_t *dst = line;

		for (i = 0; i < pixels; i++)
			*dst++ = clr;
		break;
	}
#endif
#ifdef CONFIG_VIDEO_BPP16
	case VIDEO_BPP16: {
		uint16_t *dst = line;

		for (i = 0; i < pixels; i++)
			*dst++ = clr;
		break;
	}
#endif
#ifdef CONFIG_VIDEO_BPP32
	case VIDEO_BPP32: {
		uint32_t *dst = line;

		for (i = 0; i < pixels; i++)
			*dst++ = clr;
		break;
	}
#endif
	default:
		return -ENOSYS;
	}

	return 0;
}

static int console_truetype_move_rows(struct udevice *dev, uint rowdst,
				     uint rowsrc, uint count)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_tt_priv *priv = dev_get_priv(dev);
	void *dst;
	void *src;
	int i, diff;

	dst = vid_priv->fb + rowdst * priv->font_size * vid_priv->line_length;
	src = vid_priv->fb + rowsrc * priv->font_size * vid_priv->line_length;
	memmove(dst, src, priv->font_size * vid_priv->line_length * count);

	/* Scroll up our position history */
	diff = (rowsrc - rowdst) * priv->font_size;
	for (i = 0; i < priv->pos_ptr; i++)
		priv->pos[i].ypos -= diff;

	return 0;
}

static int console_truetype_putc_xy(struct udevice *dev, uint x, uint y,
				    char ch)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);
	struct console_tt_priv *priv = dev_get_priv(dev);
	stbtt_fontinfo *font = &priv->font;
	int width, height, xoff, yoff;
	double xpos, x_shift;
	int lsb;
	int width_frac, linenum;
	struct pos_info *pos;
	u8 *bits, *data;
	int advance;
	void *line;
	int row;

	/* First get some basic metrics about this character */
	stbtt_GetCodepointHMetrics(font, ch, &advance, &lsb);

	/*
	 * First out our current X position in fractional pixels. If we wrote
	 * a character previously, using kerning to fine-tune the position of
	 * this character */
	xpos = frac(VID_TO_PIXEL((double)x));
	if (vc_priv->last_ch) {
		xpos += priv->scale * stbtt_GetCodepointKernAdvance(font,
							vc_priv->last_ch, ch);
	}

	/*
	 * Figure out where the cursor will move to after this character, and
	 * abort if we are out of space on this line. Also calculate the
	 * effective width of this character, which will be our return value:
	 * it dictates how much the cursor will move forward on the line.
	 */
	x_shift = xpos - (double)tt_floor(xpos);
	xpos += advance * priv->scale;
	width_frac = (int)VID_TO_POS(xpos);
	if (x + width_frac >= vc_priv->xsize_frac)
		return -EAGAIN;

	/* Write the current cursor position into history */
	if (priv->pos_ptr < POS_HISTORY_SIZE) {
		pos = &priv->pos[priv->pos_ptr];
		pos->xpos_frac = vc_priv->xcur_frac;
		pos->ypos = vc_priv->ycur;
		priv->pos_ptr++;
	}

	/*
	 * Figure out how much past the start of a pixel we are, and pass this
	 * information into the render, which will return a 8-bit-per-pixel
	 * image of the character. For empty characters, like ' ', data will
	 * return NULL;
	 */
	data = stbtt_GetCodepointBitmapSubpixel(font, priv->scale, priv->scale,
						x_shift, 0, ch, &width, &height,
						&xoff, &yoff);
	if (!data)
		return width_frac;

	/* Figure out where to write the character in the frame buffer */
	bits = data;
	line = vid_priv->fb + y * vid_priv->line_length +
		VID_TO_PIXEL(x) * VNBYTES(vid_priv->bpix);
	linenum = priv->baseline + yoff;
	if (linenum > 0)
		line += linenum * vid_priv->line_length;

	/*
	 * Write a row at a time, converting the 8bpp image into the colour
	 * depth of the display. We only expect white-on-black or the reverse
	 * so the code only handles this simple case.
	 */
	for (row = 0; row < height; row++) {
		switch (vid_priv->bpix) {
#ifdef CONFIG_VIDEO_BPP16
		case VIDEO_BPP16: {
			uint16_t *dst = (uint16_t *)line + xoff;
			int i;

			for (i = 0; i < width; i++) {
				int val = *bits;
				int out;

				if (vid_priv->colour_bg)
					val = 255 - val;
				out = val >> 3 |
					(val >> 2) << 5 |
					(val >> 3) << 11;
				if (vid_priv->colour_fg)
					*dst++ |= out;
				else
					*dst++ &= out;
				bits++;
			}
			break;
		}
#endif
		default:
			free(data);
			return -ENOSYS;
		}

		line += vid_priv->line_length;
	}
	free(data);

	return width_frac;
}

/**
 * console_truetype_erase() - Erase a character
 *
 * This is used for backspace. We erase a square of the display within the
 * given bounds.
 *
 * @dev:	Device to update
 * @xstart:	X start position in pixels from the left
 * @ystart:	Y start position in pixels from the top
 * @xend:	X end position in pixels from the left
 * @yend:	Y end position  in pixels from the top
 * @clr:	Value to write
 * @return 0 if OK, -ENOSYS if the display depth is not supported
 */
static int console_truetype_erase(struct udevice *dev, int xstart, int ystart,
				  int xend, int yend, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	void *line;
	int pixels = xend - xstart;
	int row, i;

	line = vid_priv->fb + ystart * vid_priv->line_length;
	line += xstart * VNBYTES(vid_priv->bpix);
	for (row = ystart; row < yend; row++) {
		switch (vid_priv->bpix) {
#ifdef CONFIG_VIDEO_BPP8
		case VIDEO_BPP8: {
			uint8_t *dst = line;

			for (i = 0; i < pixels; i++)
				*dst++ = clr;
			break;
		}
#endif
#ifdef CONFIG_VIDEO_BPP16
		case VIDEO_BPP16: {
			uint16_t *dst = line;

			for (i = 0; i < pixels; i++)
				*dst++ = clr;
			break;
		}
#endif
#ifdef CONFIG_VIDEO_BPP32
		case VIDEO_BPP32: {
			uint32_t *dst = line;

			for (i = 0; i < pixels; i++)
				*dst++ = clr;
			break;
		}
#endif
		default:
			return -ENOSYS;
		}
		line += vid_priv->line_length;
	}

	return 0;
}

/**
 * console_truetype_backspace() - Handle a backspace operation
 *
 * This clears the previous character so that the console looks as if it had
 * not been entered.
 *
 * @dev:	Device to update
 * @return 0 if OK, -ENOSYS if not supported
 */
static int console_truetype_backspace(struct udevice *dev)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct udevice *vid_dev = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);
	struct pos_info *pos;
	int xend;

	/*
	 * This indicates a very strange error higher in the stack. The caller
	 * has sent out n character and n + 1 backspaces.
	 */
	if (!priv->pos_ptr)
		return -ENOSYS;

	/* Pop the last cursor position off the stack */
	pos = &priv->pos[--priv->pos_ptr];

	/*
	 * Figure out the end position for clearing. Normlly it is the current
	 * cursor position, but if we are clearing a character on the previous
	 * line, we clear from the end of the line.
	 */
	if (pos->ypos == vc_priv->ycur)
		xend = VID_TO_PIXEL(vc_priv->xcur_frac);
	else
		xend = vid_priv->xsize;

	console_truetype_erase(dev, VID_TO_PIXEL(pos->xpos_frac), pos->ypos,
			       xend, pos->ypos + vc_priv->y_charsize,
			       vid_priv->colour_bg);

	/* Move the cursor back to where it was when we pushed this record */
	vc_priv->xcur_frac = pos->xpos_frac;
	vc_priv->ycur = pos->ypos;

	return 0;
}

static int console_truetype_entry_start(struct udevice *dev)
{
	struct console_tt_priv *priv = dev_get_priv(dev);

	/* A new input line has start, so clear our history */
	priv->pos_ptr = 0;

	return 0;
}

/*
 * Provides a list of fonts which can be obtained at run-time in U-Boot. These
 * are compiled in by the Makefile.
 *
 * At present there is no mechanism to select a particular font - the first
 * one found is the one that is used. But the build system and the code here
 * supports multiple fonts, which may be useful for certain firmware screens.
 */
struct font_info {
	char *name;
	u8 *begin;
	u8 *end;
};

#define FONT_DECL(_name) \
	extern u8 __ttf_ ## _name ## _begin[]; \
	extern u8 __ttf_ ## _name ## _end[];

#define FONT_ENTRY(_name)		{ \
	.name = #_name, \
	.begin = __ttf_ ## _name ## _begin, \
	.end = __ttf_ ## _name ## _end, \
	}

FONT_DECL(nimbus_sans_l_regular);
FONT_DECL(ankacoder_c75_r);
FONT_DECL(rufscript010);
FONT_DECL(cantoraone_regular);

static struct font_info font_table[] = {
#ifdef CONFIG_CONSOLE_TRUETYPE_NIMBUS
	FONT_ENTRY(nimbus_sans_l_regular),
#endif
#ifdef CONFIG_CONSOLE_TRUETYPE_ANKACODER
	FONT_ENTRY(ankacoder_c75_r),
#endif
#ifdef CONFIG_CONSOLE_TRUETYPE_RUFSCRIPT
	FONT_ENTRY(rufscript010),
#endif
#ifdef CONFIG_CONSOLE_TRUETYPE_CANTORAONE
	FONT_ENTRY(cantoraone_regular),
#endif
	{} /* sentinel */
};

#define FONT_BEGIN(name)	__ttf_ ## name ## _begin
#define FONT_END(name)		__ttf_ ## name ## _end
#define FONT_IS_VALID(name)	(abs(FONT_END(name) - FONT_BEGIN) > 4)

/**
 * console_truetype_find_font() - Find a suitable font
 *
 * This searched for the first available font.
 *
 * @return pointer to the font, or NULL if none is found
 */
static u8 *console_truetype_find_font(void)
{
	struct font_info *tab;

	for (tab = font_table; tab->begin; tab++) {
		if (abs(tab->begin - tab->end) > 4) {
			debug("%s: Font '%s', at %p, size %lx\n", __func__,
			      tab->name, tab->begin,
			      (ulong)(tab->end - tab->begin));
			return tab->begin;
		}
	}

	return NULL;
}

static int console_truetype_probe(struct udevice *dev)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct udevice *vid_dev = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);
	stbtt_fontinfo *font = &priv->font;
	int ascent;

	debug("%s: start\n", __func__);
	if (vid_priv->font_size)
		priv->font_size = vid_priv->font_size;
	else
		priv->font_size = CONFIG_CONSOLE_TRUETYPE_SIZE;
	priv->font_data = console_truetype_find_font();
	if (!priv->font_data) {
		debug("%s: Could not find any fonts\n", __func__);
		return -EBFONT;
	}

	vc_priv->x_charsize = priv->font_size;
	vc_priv->y_charsize = priv->font_size;
	vc_priv->xstart_frac = VID_TO_POS(2);
	vc_priv->cols = vid_priv->xsize / priv->font_size;
	vc_priv->rows = vid_priv->ysize / priv->font_size;
	vc_priv->tab_width_frac = VID_TO_POS(priv->font_size) * 8 / 2;

	if (!stbtt_InitFont(font, priv->font_data, 0)) {
		debug("%s: Font init failed\n", __func__);
		return -EPERM;
	}

	/* Pre-calculate some things we will need regularly */
	priv->scale = stbtt_ScaleForPixelHeight(font, priv->font_size);
	stbtt_GetFontVMetrics(font, &ascent, 0, 0);
	priv->baseline = (int)(ascent * priv->scale);
	debug("%s: ready\n", __func__);

	return 0;
}

struct vidconsole_ops console_truetype_ops = {
	.putc_xy	= console_truetype_putc_xy,
	.move_rows	= console_truetype_move_rows,
	.set_row	= console_truetype_set_row,
	.backspace	= console_truetype_backspace,
	.entry_start	= console_truetype_entry_start,
};

U_BOOT_DRIVER(vidconsole_truetype) = {
	.name	= "vidconsole_tt",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_truetype_ops,
	.probe	= console_truetype_probe,
	.priv_auto_alloc_size	= sizeof(struct console_tt_priv),
};
