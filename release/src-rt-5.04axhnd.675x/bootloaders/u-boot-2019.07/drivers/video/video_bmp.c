// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <bmp_layout.h>
#include <dm.h>
#include <mapmem.h>
#include <splash.h>
#include <video.h>
#include <watchdog.h>
#include <asm/unaligned.h>

#ifdef CONFIG_VIDEO_BMP_RLE8
#define BMP_RLE8_ESCAPE		0
#define BMP_RLE8_EOL		0
#define BMP_RLE8_EOBMP		1
#define BMP_RLE8_DELTA		2

static void draw_unencoded_bitmap(ushort **fbp, uchar *bmap, ushort *cmap,
				  int cnt)
{
	while (cnt > 0) {
		*(*fbp)++ = cmap[*bmap++];
		cnt--;
	}
}

static void draw_encoded_bitmap(ushort **fbp, ushort col, int cnt)
{
	ushort *fb = *fbp;

	while (cnt > 0) {
		*fb++ = col;
		cnt--;
	}
	*fbp = fb;
}

static void video_display_rle8_bitmap(struct udevice *dev,
				      struct bmp_image *bmp, ushort *cmap,
				      uchar *fb, int x_off, int y_off)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);
	uchar *bmap;
	ulong width, height;
	ulong cnt, runlen;
	int x, y;
	int decode = 1;

	debug("%s\n", __func__);
	width = get_unaligned_le32(&bmp->header.width);
	height = get_unaligned_le32(&bmp->header.height);
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
				/* 16bpix, 2-byte per pixel, width should *2 */
				fb -= (width * 2 + priv->line_length);
				break;
			case BMP_RLE8_EOBMP:
				/* end of bitmap */
				decode = 0;
				break;
			case BMP_RLE8_DELTA:
				/* delta run */
				x += bmap[2];
				y -= bmap[3];
				/* 16bpix, 2-byte per pixel, x should *2 */
				fb = (uchar *)(priv->fb + (y + y_off - 1)
					* priv->line_length + (x + x_off) * 2);
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
							(ushort **)&fb,
							bmap, cmap, cnt);
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
					draw_encoded_bitmap((ushort **)&fb,
						cmap[bmap[1]], cnt);
				}
				x += runlen;
			}
			bmap += 2;
		}
	}
}
#endif

__weak void fb_put_byte(uchar **fb, uchar **from)
{
	*(*fb)++ = *(*from)++;
}

#if defined(CONFIG_BMP_16BPP)
__weak void fb_put_word(uchar **fb, uchar **from)
{
	*(*fb)++ = *(*from)++;
	*(*fb)++ = *(*from)++;
}
#endif /* CONFIG_BMP_16BPP */

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
	unsigned long panel_picture_delta = panel_size - picture_size;
	unsigned long axis_alignment;

	if (*axis == BMP_ALIGN_CENTER)
		axis_alignment = panel_picture_delta / 2;
	else if (*axis < 0)
		axis_alignment = panel_picture_delta + *axis + 1;
	else
		return;

	*axis = max(0, (int)axis_alignment);
}

static void video_set_cmap(struct udevice *dev,
			   struct bmp_color_table_entry *cte, unsigned colours)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);
	int i;
	ushort *cmap = priv->cmap;

	debug("%s: colours=%d\n", __func__, colours);
	for (i = 0; i < colours; ++i) {
		*cmap = ((cte->red   << 8) & 0xf800) |
			((cte->green << 3) & 0x07e0) |
			((cte->blue  >> 3) & 0x001f);
		cmap++;
		cte++;
	}
}

int video_bmp_display(struct udevice *dev, ulong bmp_image, int x, int y,
		      bool align)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);
	ushort *cmap_base = NULL;
	int i, j;
	uchar *fb;
	struct bmp_image *bmp = map_sysmem(bmp_image, 0);
	uchar *bmap;
	ushort padded_width;
	unsigned long width, height, byte_width;
	unsigned long pwidth = priv->xsize;
	unsigned colours, bpix, bmp_bpix;
	struct bmp_color_table_entry *palette;
	int hdr_size;

	if (!bmp || !(bmp->header.signature[0] == 'B' &&
	    bmp->header.signature[1] == 'M')) {
		printf("Error: no valid bmp image at %lx\n", bmp_image);

		return -EINVAL;
	}

	width = get_unaligned_le32(&bmp->header.width);
	height = get_unaligned_le32(&bmp->header.height);
	bmp_bpix = get_unaligned_le16(&bmp->header.bit_count);
	hdr_size = get_unaligned_le16(&bmp->header.size);
	debug("hdr_size=%d, bmp_bpix=%d\n", hdr_size, bmp_bpix);
	palette = (void *)bmp + 14 + hdr_size;

	colours = 1 << bmp_bpix;

	bpix = VNBITS(priv->bpix);

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
	    !(bmp_bpix == 24 && bpix == 16) &&
	    !(bmp_bpix == 24 && bpix == 32)) {
		printf("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
		       bpix, get_unaligned_le16(&bmp->header.bit_count));
		return -EPERM;
	}

	debug("Display-bmp: %d x %d  with %d colours, display %d\n",
	      (int)width, (int)height, (int)colours, 1 << bpix);

	if (bmp_bpix == 8)
		video_set_cmap(dev, palette, colours);

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
	fb = (uchar *)(priv->fb +
		(y + height - 1) * priv->line_length + x * bpix / 8);

	switch (bmp_bpix) {
	case 1:
	case 8: {
		cmap_base = priv->cmap;
#ifdef CONFIG_VIDEO_BMP_RLE8
		u32 compression = get_unaligned_le32(&bmp->header.compression);
		debug("compressed %d %d\n", compression, BMP_BI_RLE8);
		if (compression == BMP_BI_RLE8) {
			if (bpix != 16) {
				/* TODO implement render code for bpix != 16 */
				printf("Error: only support 16 bpix");
				return -EPROTONOSUPPORT;
			}
			video_display_rle8_bitmap(dev, bmp, cmap_base, fb, x,
						  y);
			break;
		}
#endif

		if (bpix != 16)
			byte_width = width;
		else
			byte_width = width * 2;

		for (i = 0; i < height; ++i) {
			WATCHDOG_RESET();
			for (j = 0; j < width; j++) {
				if (bpix != 16) {
					fb_put_byte(&fb, &bmap);
				} else {
					*(uint16_t *)fb = cmap_base[*bmap];
					bmap++;
					fb += sizeof(uint16_t) / sizeof(*fb);
				}
			}
			bmap += (padded_width - width);
			fb -= byte_width + priv->line_length;
		}
		break;
	}
#if defined(CONFIG_BMP_16BPP)
	case 16:
		for (i = 0; i < height; ++i) {
			WATCHDOG_RESET();
			for (j = 0; j < width; j++)
				fb_put_word(&fb, &bmap);

			bmap += (padded_width - width) * 2;
			fb -= width * 2 + priv->line_length;
		}
		break;
#endif /* CONFIG_BMP_16BPP */
#if defined(CONFIG_BMP_24BPP)
	case 24:
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; j++) {
				if (bpix == 16) {
					/* 16bit 555RGB format */
					*(u16 *)fb = ((bmap[2] >> 3) << 10) |
						((bmap[1] >> 3) << 5) |
						(bmap[0] >> 3);
					bmap += 3;
					fb += 2;
				} else {
					*(fb++) = *(bmap++);
					*(fb++) = *(bmap++);
					*(fb++) = *(bmap++);
					*(fb++) = 0;
				}
			}
			fb -= priv->line_length + width * (bpix / 8);
			bmap += (padded_width - width) * 3;
		}
		break;
#endif /* CONFIG_BMP_24BPP */
#if defined(CONFIG_BMP_32BPP)
	case 32:
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; j++) {
				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
			}
			fb -= priv->line_length + width * (bpix / 8);
		}
		break;
#endif /* CONFIG_BMP_32BPP */
	default:
		break;
	};

	video_sync(dev, false);

	return 0;
}

