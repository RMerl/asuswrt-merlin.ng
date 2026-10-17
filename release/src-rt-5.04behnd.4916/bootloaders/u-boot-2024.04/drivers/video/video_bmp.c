// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <bmp_layout.h>
#include <dm.h>
#include <log.h>
#include <mapmem.h>
#include <splash.h>
#include <video.h>
#include <watchdog.h>
#include <asm/unaligned.h>

#define BMP_RLE8_ESCAPE		0
#define BMP_RLE8_EOL		0
#define BMP_RLE8_EOBMP		1
#define BMP_RLE8_DELTA		2

/**
 * get_bmp_col_16bpp() - Convert a colour-table entry into a 16bpp pixel value
 *
 * Return: value to write to the 16bpp frame buffer for this palette entry
 */
static uint get_bmp_col_16bpp(struct bmp_color_table_entry cte)
{
	return ((cte.red   << 8) & 0xf800) |
		((cte.green << 3) & 0x07e0) |
		((cte.blue  >> 3) & 0x001f);
}

/**
 * get_bmp_col_x2r10g10b10() - Convert a colour-table entry into a x2r10g10b10  pixel value
 *
 * Return: value to write to the x2r10g10b10 frame buffer for this palette entry
 */
static u32 get_bmp_col_x2r10g10b10(struct bmp_color_table_entry *cte)
{
	return ((cte->red << 22U) |
		(cte->green << 12U) |
		(cte->blue << 2U));
}

/**
 * get_bmp_col_rgba8888() - Convert a colour-table entry into a rgba8888 pixel value
 *
 * Return: value to write to the rgba8888 frame buffer for this palette entry
 */
static u32 get_bmp_col_rgba8888(struct bmp_color_table_entry *cte)
{
	return ((cte->red) |
		(cte->green << 8U) |
		(cte->blue << 16U) | 0xff << 24U);
}

/**
 * write_pix8() - Write a pixel from a BMP image into the framebuffer
 *
 * This handles frame buffers with 8, 16, 24 or 32 bits per pixel
 *
 * @fb: Place in frame buffer to update
 * @bpix: Frame buffer bits-per-pixel, which controls how many bytes are written
 * @palette: BMP palette table
 * @bmap: Pointer to BMP bitmap position to write. This contains a single byte
 *	which is either written directly (bpix == 8) or used to look up the
 *	palette to get a colour to write
 */
static void write_pix8(u8 *fb, uint bpix, enum video_format eformat,
		       struct bmp_color_table_entry *palette, u8 *bmap)
{
	if (bpix == 8) {
		*fb++ = *bmap;
	} else if (bpix == 16) {
		*(u16 *)fb = get_bmp_col_16bpp(palette[*bmap]);
	} else {
		/* Only support big endian */
		struct bmp_color_table_entry *cte = &palette[*bmap];

		if (bpix == 24) {
			*fb++ = cte->red;
			*fb++ = cte->green;
			*fb++ = cte->blue;
		} else if (eformat == VIDEO_X2R10G10B10) {
			*(u32 *)fb = get_bmp_col_x2r10g10b10(cte);
		} else if (eformat == VIDEO_RGBA8888) {
			*(u32 *)fb = get_bmp_col_rgba8888(cte);
		} else {
			*fb++ = cte->blue;
			*fb++ = cte->green;
			*fb++ = cte->red;
			*fb++ = 0;
		}
	}
}

static void draw_unencoded_bitmap(u8 **fbp, uint bpix,
				  enum video_format eformat, uchar *bmap,
				  struct bmp_color_table_entry *palette,
				  int cnt)
{
	u8 *fb = *fbp;

	while (cnt > 0) {
		write_pix8(fb, bpix, eformat, palette, bmap++);
		fb += bpix / 8;
		cnt--;
	}
	*fbp = fb;
}

static void draw_encoded_bitmap(u8 **fbp, uint bpix, enum video_format eformat,
				struct bmp_color_table_entry *palette, u8 *bmap,
				int cnt)
{
	u8 *fb = *fbp;

	while (cnt > 0) {
		write_pix8(fb, bpix, eformat, palette, bmap);
		fb += bpix / 8;
		cnt--;
	}
	*fbp = fb;
}

static void video_display_rle8_bitmap(struct udevice *dev,
				      struct bmp_image *bmp, uint bpix,
				      struct bmp_color_table_entry *palette,
				      uchar *fb, int x_off, int y_off,
				      ulong width, ulong height)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);
	uchar *bmap;
	ulong cnt, runlen;
	int x, y;
	int decode = 1;
	uint bytes_per_pixel = bpix / 8;
	enum video_format eformat = priv->format;

	debug("%s\n", __func__);
	bmap = (uchar *)bmp + get_unaligned_le32(&bmp->header.data_offset);

	x = 0;
	y = height - 1;

	while (decode) {
		if (bmap[0] == BMP_RLE8_ESCAPE) {
			switch (bmap[1]) {
			case BMP_RLE8_EOL:
				/* end of line */
				bmap += 2;
				x = 0;
				y--;
				fb -= width * bytes_per_pixel +
					priv->line_length;
				break;
			case BMP_RLE8_EOBMP:
				/* end of bitmap */
				decode = 0;
				break;
			case BMP_RLE8_DELTA:
				/* delta run */
				x += bmap[2];
				y -= bmap[3];
				fb = (uchar *)(priv->fb +
					(y + y_off - 1) * priv->line_length +
					(x + x_off) * bytes_per_pixel);
				bmap += 4;
				break;
			default:
				/* unencoded run */
				runlen = bmap[1];
				bmap += 2;
				if (y < height) {
					if (x < width) {
						if (x + runlen > width)
							cnt = width - x;
						else
							cnt = runlen;
						draw_unencoded_bitmap(
							&fb, bpix, eformat,
							bmap, palette, cnt);
					}
					x += runlen;
				}
				bmap += runlen;
				if (runlen & 1)
					bmap++;
			}
		} else {
			/* encoded run */
			if (y < height) {
				runlen = bmap[0];
				if (x < width) {
					/* aggregate the same code */
					while (bmap[0] == 0xff &&
					       bmap[2] != BMP_RLE8_ESCAPE &&
					       bmap[1] == bmap[3]) {
						runlen += bmap[2];
						bmap += 2;
					}
					if (x + runlen > width)
						cnt = width - x;
					else
						cnt = runlen;
					draw_encoded_bitmap(&fb, bpix, eformat,
							    palette, &bmap[1],
							    cnt);
				}
				x += runlen;
			}
			bmap += 2;
		}
	}
}

/**
 * video_splash_align_axis() - Align a single coordinate
 *
 *- if a coordinate is 0x7fff then the image will be centred in
 *  that direction
 *- if a coordinate is -ve then it will be offset to the
 *  left/top of the centre by that many pixels
 *- if a coordinate is positive it will be used unchnaged.
 *
 * @axis:	Input and output coordinate
 * @panel_size:	Size of panel in pixels for that axis
 * @picture_size:	Size of bitmap in pixels for that axis
 */
static void video_splash_align_axis(int *axis, unsigned long panel_size,
				    unsigned long picture_size)
{
	long panel_picture_delta = panel_size - picture_size;
	long axis_alignment;

	if (*axis == BMP_ALIGN_CENTER)
		axis_alignment = panel_picture_delta / 2;
	else if (*axis < 0)
		axis_alignment = panel_picture_delta + *axis + 1;
	else
		return;

	*axis = max(0, (int)axis_alignment);
}

void video_bmp_get_info(void *bmp_image, ulong *widthp, ulong *heightp,
			uint *bpixp)
{
	struct bmp_image *bmp = bmp_image;

	*widthp = get_unaligned_le32(&bmp->header.width);
	*heightp = get_unaligned_le32(&bmp->header.height);
	*bpixp = get_unaligned_le16(&bmp->header.bit_count);
}

int video_bmp_display(struct udevice *dev, ulong bmp_image, int x, int y,
		      bool align)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);
	int i, j;
	uchar *start, *fb;
	struct bmp_image *bmp = map_sysmem(bmp_image, 0);
	uchar *bmap;
	ushort padded_width;
	unsigned long width, height, byte_width;
	unsigned long pwidth = priv->xsize;
	unsigned colours, bpix, bmp_bpix;
	enum video_format eformat;
	struct bmp_color_table_entry *palette;
	int hdr_size;
	int ret;

	if (!bmp || !(bmp->header.signature[0] == 'B' &&
	    bmp->header.signature[1] == 'M')) {
		printf("Error: no valid bmp image at %lx\n", bmp_image);

		return -EINVAL;
	}

	video_bmp_get_info(bmp, &width, &height, &bmp_bpix);
	hdr_size = get_unaligned_le16(&bmp->header.size);
	debug("hdr_size=%d, bmp_bpix=%d\n", hdr_size, bmp_bpix);
	palette = (void *)bmp + 14 + hdr_size;

	colours = 1 << bmp_bpix;

	bpix = VNBITS(priv->bpix);
	eformat = priv->format;

	if (bpix != 1 && bpix != 8 && bpix != 16 && bpix != 32) {
		printf("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
		       bpix, bmp_bpix);

		return -EINVAL;
	}

	/*
	 * We support displaying 8bpp and 24bpp BMPs on 16bpp LCDs
	 * and displaying 24bpp BMPs on 32bpp LCDs
	 */
	if (bpix != bmp_bpix &&
	    !(bmp_bpix == 8 && bpix == 16) &&
	    !(bmp_bpix == 8 && bpix == 24) &&
	    !(bmp_bpix == 8 && bpix == 32) &&
	    !(bmp_bpix == 24 && bpix == 16) &&
	    !(bmp_bpix == 24 && bpix == 32)) {
		printf("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
		       bpix, colours);
		return -EPERM;
	}

	debug("Display-bmp: %d x %d  with %d colours, display %d\n",
	      (int)width, (int)height, (int)colours, 1 << bpix);

	padded_width = (width & 0x3 ? (width & ~0x3) + 4 : width);

	if (align) {
		video_splash_align_axis(&x, priv->xsize, width);
		video_splash_align_axis(&y, priv->ysize, height);
	}

	if ((x + width) > pwidth)
		width = pwidth - x;
	if ((y + height) > priv->ysize)
		height = priv->ysize - y;

	bmap = (uchar *)bmp + get_unaligned_le32(&bmp->header.data_offset);
	start = (uchar *)(priv->fb +
		(y + height) * priv->line_length + x * bpix / 8);

	/* Move back to the final line to be drawn */
	fb = start - priv->line_length;

	switch (bmp_bpix) {
	case 1:
	case 8:
		if (CONFIG_IS_ENABLED(VIDEO_BMP_RLE8)) {
			u32 compression = get_unaligned_le32(
				&bmp->header.compression);
			debug("compressed %d %d\n", compression, BMP_BI_RLE8);
			if (compression == BMP_BI_RLE8) {
				video_display_rle8_bitmap(dev, bmp, bpix, palette, fb,
							  x, y, width, height);
				break;
			}
		}

		/* Not compressed */
		byte_width = width * (bpix / 8);
		if (!byte_width)
			byte_width = width;

		for (i = 0; i < height; ++i) {
			schedule();
			for (j = 0; j < width; j++) {
				write_pix8(fb, bpix, eformat, palette, bmap);
				bmap++;
				fb += bpix / 8;
			}
			bmap += (padded_width - width);
			fb -= byte_width + priv->line_length;
		}
		break;
	case 16:
		if (CONFIG_IS_ENABLED(BMP_16BPP)) {
			for (i = 0; i < height; ++i) {
				schedule();
				for (j = 0; j < width; j++) {
					*fb++ = *bmap++;
					*fb++ = *bmap++;
				}
				bmap += (padded_width - width);
				fb -= width * 2 + priv->line_length;
			}
		}
		break;
	case 24:
		if (CONFIG_IS_ENABLED(BMP_24BPP)) {
			for (i = 0; i < height; ++i) {
				for (j = 0; j < width; j++) {
					if (bpix == 16) {
						/* 16bit 565RGB format */
						*(u16 *)fb = ((bmap[2] >> 3)
							<< 11) |
							((bmap[1] >> 2) << 5) |
							(bmap[0] >> 3);
						bmap += 3;
						fb += 2;
					} else if (eformat == VIDEO_X2R10G10B10) {
						u32 pix;

						pix = *bmap++ << 2U;
						pix |= *bmap++ << 12U;
						pix |= *bmap++ << 22U;
						*fb++ = pix & 0xff;
						*fb++ = (pix >> 8) & 0xff;
						*fb++ = (pix >> 16) & 0xff;
						*fb++ = pix >> 24;
					} else if (eformat == VIDEO_RGBA8888) {
						u32 pix;

						pix = *bmap++ << 8U; /* blue */
						pix |= *bmap++ << 16U; /* green */
						pix |= *bmap++ << 24U; /* red */

						*fb++ = (pix >> 24) & 0xff;
						*fb++ = (pix >> 16) & 0xff;
						*fb++ = (pix >> 8) & 0xff;
						*fb++ = 0xff;
					} else {
						*fb++ = *bmap++;
						*fb++ = *bmap++;
						*fb++ = *bmap++;
						*fb++ = 0;
					}
				}
				fb -= priv->line_length + width * (bpix / 8);
				bmap += (padded_width - width);
			}
		}
		break;
	case 32:
		if (CONFIG_IS_ENABLED(BMP_32BPP)) {
			for (i = 0; i < height; ++i) {
				for (j = 0; j < width; j++) {
					if (eformat == VIDEO_X2R10G10B10) {
						u32 pix;

						pix = *bmap++ << 2U;
						pix |= *bmap++ << 12U;
						pix |= *bmap++ << 22U;
						pix |= (*bmap++ >> 6) << 30U;
						*fb++ = pix & 0xff;
						*fb++ = (pix >> 8) & 0xff;
						*fb++ = (pix >> 16) & 0xff;
						*fb++ = pix >> 24;
					} else if (eformat == VIDEO_RGBA8888) {
						u32 pix;

						pix = *bmap++ << 8U; /* blue */
						pix |= *bmap++ << 16U; /* green */
						pix |= *bmap++ << 24U; /* red */
						bmap++;
						*fb++ = (pix >> 24) & 0xff;
						*fb++ = (pix >> 16) & 0xff;
						*fb++ = (pix >> 8) & 0xff;
						*fb++ = 0xff; /* opacity */
					} else {
						*fb++ = *bmap++;
						*fb++ = *bmap++;
						*fb++ = *bmap++;
						*fb++ = *bmap++;
					}
				}
				fb -= priv->line_length + width * (bpix / 8);
			}
		}
		break;
	default:
		break;
	};

	/* Find the position of the top left of the image in the framebuffer */
	fb = (uchar *)(priv->fb + y * priv->line_length + x * bpix / 8);
	ret = video_sync_copy(dev, start, fb);
	if (ret)
		return log_ret(ret);

	return video_sync(dev, false);
}
