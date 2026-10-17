// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <common.h>
#include <abuf.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
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

static double tt_fmod(double x, double y)
{
	double rem;

	if (y == 0.0)
		return 0.0;
	rem = x - (x / y) * y;

	return rem;
}

/* dummy implementation */
static double tt_pow(double x, double y)
{
	return 0;
}

/* dummy implementation */
static double tt_cos(double val)
{
	return 0;
}

/* dummy implementation */
static double tt_acos(double val)
{
	return 0;
}

#define STBTT_ifloor		tt_floor
#define STBTT_iceil		tt_ceil
#define STBTT_fabs		tt_fabs
#define STBTT_sqrt		tt_sqrt
#define STBTT_pow		tt_pow
#define STBTT_fmod		tt_fmod
#define STBTT_cos		tt_cos
#define STBTT_acos		tt_acos
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
 * struct console_tt_metrics - Information about a font / size combination
 *
 * This caches various font metrics which are expensive to regenerate each time
 * the font size changes. There is one of these for each font / size combination
 * that is being used
 *
 * @font_name:	Name of the font
 * @font_size:	Vertical font size in pixels
 * @font_data:	Pointer to TrueType font file contents
 * @font:	TrueType font information for the current font
 * @baseline:	Pixel offset of the font's baseline from the cursor position.
 *		This is the 'ascent' of the font, scaled to pixel coordinates.
 *		It measures the distance from the baseline to the top of the
 *		font.
 * @scale:	Scale of the font. This is calculated from the pixel height
 *		of the font. It is used by the STB library to generate images
 *		of the correct size.
 */
struct console_tt_metrics {
	const char *font_name;
	int font_size;
	const u8 *font_data;
	stbtt_fontinfo font;
	int baseline;
	double scale;
};

/**
 * struct console_tt_priv - Private data for this driver
 *
 * @cur_met:	Current metrics being used
 * @metrics:	List metrics that can be used
 * @num_metrics:	Number of available metrics
 * @pos:	List of cursor positions for each character written. This is
 *		used to handle backspace. We clear the frame buffer between
 *		the last position and the current position, thus erasing the
 *		last character. We record enough characters to go back to the
 *		start of the current command line.
 * @pos_ptr:	Current position in the position history
 */
struct console_tt_priv {
	struct console_tt_metrics *cur_met;
	struct console_tt_metrics metrics[CONFIG_CONSOLE_TRUETYPE_MAX_METRICS];
	int num_metrics;
	struct pos_info pos[POS_HISTORY_SIZE];
	int pos_ptr;
};

/**
 * struct console_tt_store - Format used for save/restore of entry information
 *
 * @priv: Private data
 * @cur: Current cursor position
 */
struct console_tt_store {
	struct console_tt_priv priv;
	struct pos_info cur;
};

static int console_truetype_set_row(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct console_tt_metrics *met = priv->cur_met;
	void *end, *line;
	int ret;

	line = vid_priv->fb + row * met->font_size * vid_priv->line_length;
	end = line + met->font_size * vid_priv->line_length;

	switch (vid_priv->bpix) {
	case VIDEO_BPP8: {
		u8 *dst;

		if (IS_ENABLED(CONFIG_VIDEO_BPP8)) {
			for (dst = line; dst < (u8 *)end; ++dst)
				*dst = clr;
		}
		break;
	}
	case VIDEO_BPP16: {
		u16 *dst = line;

		if (IS_ENABLED(CONFIG_VIDEO_BPP16)) {
			for (dst = line; dst < (u16 *)end; ++dst)
				*dst = clr;
		}
		break;
	}
	case VIDEO_BPP32: {
		u32 *dst = line;

		if (IS_ENABLED(CONFIG_VIDEO_BPP32)) {
			for (dst = line; dst < (u32 *)end; ++dst)
				*dst = clr;
		}
		break;
	}
	default:
		return -ENOSYS;
	}
	ret = vidconsole_sync_copy(dev, line, end);
	if (ret)
		return ret;

	return 0;
}

static int console_truetype_move_rows(struct udevice *dev, uint rowdst,
				     uint rowsrc, uint count)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct console_tt_metrics *met = priv->cur_met;
	void *dst;
	void *src;
	int i, diff, ret;

	dst = vid_priv->fb + rowdst * met->font_size * vid_priv->line_length;
	src = vid_priv->fb + rowsrc * met->font_size * vid_priv->line_length;
	ret = vidconsole_memmove(dev, dst, src, met->font_size *
				 vid_priv->line_length * count);
	if (ret)
		return ret;

	/* Scroll up our position history */
	diff = (rowsrc - rowdst) * met->font_size;
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
	struct console_tt_metrics *met = priv->cur_met;
	stbtt_fontinfo *font = &met->font;
	int width, height, xoff, yoff;
	double xpos, x_shift;
	int lsb;
	int width_frac, linenum;
	struct pos_info *pos;
	u8 *bits, *data;
	int advance;
	void *start, *end, *line;
	int row, ret;

	/* First get some basic metrics about this character */
	stbtt_GetCodepointHMetrics(font, ch, &advance, &lsb);

	/*
	 * First out our current X position in fractional pixels. If we wrote
	 * a character previously, using kerning to fine-tune the position of
	 * this character */
	xpos = frac(VID_TO_PIXEL((double)x));
	if (vc_priv->last_ch) {
		xpos += met->scale * stbtt_GetCodepointKernAdvance(font,
							vc_priv->last_ch, ch);
	}

	/*
	 * Figure out where the cursor will move to after this character, and
	 * abort if we are out of space on this line. Also calculate the
	 * effective width of this character, which will be our return value:
	 * it dictates how much the cursor will move forward on the line.
	 */
	x_shift = xpos - (double)tt_floor(xpos);
	xpos += advance * met->scale;
	width_frac = (int)VID_TO_POS(advance * met->scale);
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
	data = stbtt_GetCodepointBitmapSubpixel(font, met->scale, met->scale,
						x_shift, 0, ch, &width, &height,
						&xoff, &yoff);
	if (!data)
		return width_frac;

	/* Figure out where to write the character in the frame buffer */
	bits = data;
	start = vid_priv->fb + y * vid_priv->line_length +
		VID_TO_PIXEL(x) * VNBYTES(vid_priv->bpix);
	linenum = met->baseline + yoff;
	if (linenum > 0)
		start += linenum * vid_priv->line_length;
	line = start;

	/*
	 * Write a row at a time, converting the 8bpp image into the colour
	 * depth of the display. We only expect white-on-black or the reverse
	 * so the code only handles this simple case.
	 */
	for (row = 0; row < height; row++) {
		switch (vid_priv->bpix) {
		case VIDEO_BPP8:
			if (IS_ENABLED(CONFIG_VIDEO_BPP8)) {
				u8 *dst = line + xoff;
				int i;

				for (i = 0; i < width; i++) {
					int val = *bits;
					int out;

					if (vid_priv->colour_bg)
						val = 255 - val;
					out = val;
					if (vid_priv->colour_fg)
						*dst++ |= out;
					else
						*dst++ &= out;
					bits++;
				}
				end = dst;
			}
			break;
		case VIDEO_BPP16: {
			uint16_t *dst = (uint16_t *)line + xoff;
			int i;

			if (IS_ENABLED(CONFIG_VIDEO_BPP16)) {
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
				end = dst;
			}
			break;
		}
		case VIDEO_BPP32: {
			u32 *dst = (u32 *)line + xoff;
			int i;

			if (IS_ENABLED(CONFIG_VIDEO_BPP32)) {
				for (i = 0; i < width; i++) {
					int val = *bits;
					int out;

					if (vid_priv->colour_bg)
						val = 255 - val;
					if (vid_priv->format == VIDEO_X2R10G10B10)
						out = val << 2 | val << 12 | val << 22;
					else
						out = val | val << 8 | val << 16;
					if (vid_priv->colour_fg)
						*dst++ |= out;
					else
						*dst++ &= out;
					bits++;
				}
				end = dst;
			}
			break;
		}
		default:
			free(data);
			return -ENOSYS;
		}

		line += vid_priv->line_length;
	}
	ret = vidconsole_sync_copy(dev, start, line);
	if (ret)
		return ret;
	free(data);

	return width_frac;
}

/**
 * console_truetype_backspace() - Handle a backspace operation
 *
 * This clears the previous character so that the console looks as if it had
 * not been entered.
 *
 * @dev:	Device to update
 * Return: 0 if OK, -ENOSYS if not supported
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
	 * Figure out the end position for clearing. Normally it is the current
	 * cursor position, but if we are clearing a character on the previous
	 * line, we clear from the end of the line.
	 */
	if (pos->ypos == vc_priv->ycur)
		xend = VID_TO_PIXEL(vc_priv->xcur_frac);
	else
		xend = vid_priv->xsize;

	video_fill_part(vid_dev, VID_TO_PIXEL(pos->xpos_frac), pos->ypos,
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

/**
 * font_valid() - Check if a font-table entry is valid
 *
 * Depending on available files in the build system, fonts may end up being
 * empty.
 *
 * @return true if the entry is valid
 */
static inline bool font_valid(struct font_info *tab)
{
	return abs(tab->begin - tab->end) > 4;
}

/**
 * console_truetype_find_font() - Find a suitable font
 *
 * This searches for the first available font.
 *
 * Return: pointer to the font-table entry, or NULL if none is found
 */
static struct font_info *console_truetype_find_font(void)
{
	struct font_info *tab;

	for (tab = font_table; tab->begin; tab++) {
		if (font_valid(tab)) {
			debug("%s: Font '%s', at %p, size %lx\n", __func__,
			      tab->name, tab->begin,
			      (ulong)(tab->end - tab->begin));
			return tab;
		}
	}

	return NULL;
}

int console_truetype_get_font(struct udevice *dev, int seq,
			      struct vidfont_info *info)
{
	struct font_info *tab;
	int i;

	for (i = 0, tab = font_table; tab->begin; tab++, i++) {
		if (i == seq && font_valid(tab)) {
			info->name = tab->name;
			return 0;
		}
	}

	return -ENOENT;
}

/**
 * truetype_add_metrics() - Add a new font/size combination
 *
 * @dev:	Video console device to update
 * @font_name:	Name of font
 * @font_size:	Size of the font (norminal pixel height)
 * @font_data:	Pointer to the font data
 * @return 0 if OK, -EPERM if stbtt failed, -E2BIG if the the metrics table is
 *	full
 */
static int truetype_add_metrics(struct udevice *dev, const char *font_name,
				uint font_size, const void *font_data)
{
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct console_tt_metrics *met;
	stbtt_fontinfo *font;
	int ascent;

	if (priv->num_metrics == CONFIG_CONSOLE_TRUETYPE_MAX_METRICS)
		return log_msg_ret("num", -E2BIG);

	met = &priv->metrics[priv->num_metrics];
	met->font_name = font_name;
	met->font_size = font_size;
	met->font_data = font_data;

	font = &met->font;
	if (!stbtt_InitFont(font, font_data, 0)) {
		debug("%s: Font init failed\n", __func__);
		return -EPERM;
	}

	/* Pre-calculate some things we will need regularly */
	met->scale = stbtt_ScaleForPixelHeight(font, font_size);
	stbtt_GetFontVMetrics(font, &ascent, 0, 0);
	met->baseline = (int)(ascent * met->scale);

	return priv->num_metrics++;
}

/**
 * find_metrics() - Find the metrics for a given font and size
 *
 * @dev:	Video console device to update
 * @name:	Name of font
 * @size:	Size of the font (norminal pixel height)
 * @return metrics, if found, else NULL
 */
static struct console_tt_metrics *find_metrics(struct udevice *dev,
					       const char *name, uint size)
{
	struct console_tt_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < priv->num_metrics; i++) {
		struct console_tt_metrics *met = &priv->metrics[i];

		if (!strcmp(name, met->font_name) && met->font_size == size)
			return met;
	}

	return NULL;
}

static void select_metrics(struct udevice *dev, struct console_tt_metrics *met)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct udevice *vid_dev = dev_get_parent(dev);
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);

	priv->cur_met = met;
	vc_priv->x_charsize = met->font_size;
	vc_priv->y_charsize = met->font_size;
	vc_priv->xstart_frac = VID_TO_POS(2);
	vc_priv->cols = vid_priv->xsize / met->font_size;
	vc_priv->rows = vid_priv->ysize / met->font_size;
	vc_priv->tab_width_frac = VID_TO_POS(met->font_size) * 8 / 2;
}

static int get_metrics(struct udevice *dev, const char *name, uint size,
		       struct console_tt_metrics **metp)
{
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct console_tt_metrics *met;
	struct font_info *tab;

	if (name || size) {
		if (!size)
			size = CONFIG_CONSOLE_TRUETYPE_SIZE;
		if (!name)
			name = font_table->name;

		met = find_metrics(dev, name, size);
		if (!met) {
			for (tab = font_table; tab->begin; tab++) {
				if (font_valid(tab) &&
				    !strcmp(name, tab->name)) {
					int ret;

					ret = truetype_add_metrics(dev,
								   tab->name,
								   size,
								   tab->begin);
					if (ret < 0)
						return log_msg_ret("add", ret);

					met = &priv->metrics[ret];
					break;
				}
			}
		}
		if (!met)
			return log_msg_ret("find", -ENOENT);
	} else {
		/* Use the default font */
		met = priv->metrics;
	}

	*metp = met;

	return 0;
}

static int truetype_select_font(struct udevice *dev, const char *name,
				uint size)
{
	struct console_tt_metrics *met;
	int ret;

	ret = get_metrics(dev, name, size, &met);
	if (ret)
		return log_msg_ret("sel", ret);

	select_metrics(dev, met);

	return 0;
}

static int truetype_measure(struct udevice *dev, const char *name, uint size,
			    const char *text, struct vidconsole_bbox *bbox)
{
	struct console_tt_metrics *met;
	stbtt_fontinfo *font;
	int lsb, advance;
	const char *s;
	int width;
	int last;
	int ret;

	ret = get_metrics(dev, name, size, &met);
	if (ret)
		return log_msg_ret("sel", ret);

	bbox->valid = false;
	if (!*text)
		return 0;

	font = &met->font;
	width = 0;
	for (last = 0, s = text; *s; s++) {
		int ch = *s;

		/* Used kerning to fine-tune the position of this character */
		if (last)
			width += stbtt_GetCodepointKernAdvance(font, last, ch);

		/* First get some basic metrics about this character */
		stbtt_GetCodepointHMetrics(font, ch, &advance, &lsb);

		width += advance;
		last = ch;
	}

	bbox->valid = true;
	bbox->x0 = 0;
	bbox->y0 = 0;
	bbox->x1 = tt_ceil((double)width * met->scale);
	bbox->y1 = met->font_size;

	return 0;
}

static int truetype_nominal(struct udevice *dev, const char *name, uint size,
			    uint num_chars, struct vidconsole_bbox *bbox)
{
	struct console_tt_metrics *met;
	stbtt_fontinfo *font;
	int lsb, advance;
	int width;
	int ret;

	ret = get_metrics(dev, name, size, &met);
	if (ret)
		return log_msg_ret("sel", ret);

	font = &met->font;
	width = 0;

	/* First get some basic metrics about this character */
	stbtt_GetCodepointHMetrics(font, 'W', &advance, &lsb);

	width = advance;

	bbox->valid = true;
	bbox->x0 = 0;
	bbox->y0 = 0;
	bbox->x1 = tt_ceil((double)width * num_chars * met->scale);
	bbox->y1 = met->font_size;

	return 0;
}

static int truetype_entry_save(struct udevice *dev, struct abuf *buf)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct console_tt_store store;
	const uint size = sizeof(store);

	/*
	 * store the whole priv structure as it is simpler that picking out
	 * what we need
	 */
	if (!abuf_realloc(buf, size))
		return log_msg_ret("sav", -ENOMEM);

	store.priv = *priv;
	store.cur.xpos_frac = vc_priv->xcur_frac;
	store.cur.ypos  = vc_priv->ycur;
	memcpy(abuf_data(buf), &store, size);

	return 0;
}

static int truetype_entry_restore(struct udevice *dev, struct abuf *buf)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct console_tt_store store;

	memcpy(&store, abuf_data(buf), sizeof(store));

	vc_priv->xcur_frac = store.cur.xpos_frac;
	vc_priv->ycur = store.cur.ypos;
	priv->pos_ptr = store.priv.pos_ptr;
	memcpy(priv->pos, store.priv.pos,
	       store.priv.pos_ptr * sizeof(struct pos_info));

	return 0;
}

static int truetype_set_cursor_visible(struct udevice *dev, bool visible,
				       uint x, uint y, uint index)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct console_tt_metrics *met = priv->cur_met;
	uint row, width, height, xoff;
	void *start, *line;
	uint out, val;
	int ret;

	if (!visible)
		return 0;

	/*
	 * figure out where to place the cursor. This driver ignores the
	 * passed-in values, since an entry_restore() must have been done before
	 * calling this function.
	 */
	if (index < priv->pos_ptr)
		x = VID_TO_PIXEL(priv->pos[index].xpos_frac);
	else
		x = VID_TO_PIXEL(vc_priv->xcur_frac);

	y = vc_priv->ycur;
	height = met->font_size;
	xoff = 0;

	val = vid_priv->colour_bg ? 0 : 255;
	width = VIDCONSOLE_CURSOR_WIDTH;

	/* Figure out where to write the cursor in the frame buffer */
	start = vid_priv->fb + y * vid_priv->line_length +
		x * VNBYTES(vid_priv->bpix);
	line = start;

	/* draw a vertical bar in the correct position */
	for (row = 0; row < height; row++) {
		switch (vid_priv->bpix) {
		case VIDEO_BPP8:
			if (IS_ENABLED(CONFIG_VIDEO_BPP8)) {
				u8 *dst = line + xoff;
				int i;

				out = val;
				for (i = 0; i < width; i++) {
					if (vid_priv->colour_fg)
						*dst++ |= out;
					else
						*dst++ &= out;
				}
			}
			break;
		case VIDEO_BPP16: {
			u16 *dst = (u16 *)line + xoff;
			int i;

			if (IS_ENABLED(CONFIG_VIDEO_BPP16)) {
				for (i = 0; i < width; i++) {
					out = val >> 3 |
						(val >> 2) << 5 |
						(val >> 3) << 11;
					if (vid_priv->colour_fg)
						*dst++ |= out;
					else
						*dst++ &= out;
				}
			}
			break;
		}
		case VIDEO_BPP32: {
			u32 *dst = (u32 *)line + xoff;
			int i;

			if (IS_ENABLED(CONFIG_VIDEO_BPP32)) {
				for (i = 0; i < width; i++) {
					int out;

					if (vid_priv->format == VIDEO_X2R10G10B10)
						out = val << 2 | val << 12 | val << 22;
					else
						out = val | val << 8 | val << 16;
					if (vid_priv->colour_fg)
						*dst++ |= out;
					else
						*dst++ &= out;
				}
			}
			break;
		}
		default:
			return -ENOSYS;
		}

		line += vid_priv->line_length;
	}
	ret = vidconsole_sync_copy(dev, start, line);
	if (ret)
		return ret;

	return video_sync(vid, true);
}

const char *console_truetype_get_font_size(struct udevice *dev, uint *sizep)
{
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct console_tt_metrics *met = priv->cur_met;

	*sizep = met->font_size;

	return met->font_name;
}

static int console_truetype_probe(struct udevice *dev)
{
	struct console_tt_priv *priv = dev_get_priv(dev);
	struct udevice *vid_dev = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);
	struct font_info *tab;
	uint font_size;
	int ret;

	debug("%s: start\n", __func__);
	if (vid_priv->font_size)
		font_size = vid_priv->font_size;
	else
		font_size = CONFIG_CONSOLE_TRUETYPE_SIZE;
	tab = console_truetype_find_font();
	if (!tab) {
		debug("%s: Could not find any fonts\n", __func__);
		return -EBFONT;
	}

	ret = truetype_add_metrics(dev, tab->name, font_size, tab->begin);
	if (ret < 0)
		return log_msg_ret("add", ret);
	priv->cur_met = &priv->metrics[ret];

	select_metrics(dev, &priv->metrics[ret]);

	debug("%s: ready\n", __func__);

	return 0;
}

struct vidconsole_ops console_truetype_ops = {
	.putc_xy	= console_truetype_putc_xy,
	.move_rows	= console_truetype_move_rows,
	.set_row	= console_truetype_set_row,
	.backspace	= console_truetype_backspace,
	.entry_start	= console_truetype_entry_start,
	.get_font	= console_truetype_get_font,
	.get_font_size	= console_truetype_get_font_size,
	.select_font	= truetype_select_font,
	.measure	= truetype_measure,
	.nominal	= truetype_nominal,
	.entry_save	= truetype_entry_save,
	.entry_restore	= truetype_entry_restore,
	.set_cursor_visible	= truetype_set_cursor_visible
};

U_BOOT_DRIVER(vidconsole_truetype) = {
	.name	= "vidconsole_tt",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_truetype_ops,
	.probe	= console_truetype_probe,
	.priv_auto	= sizeof(struct console_tt_priv),
};
