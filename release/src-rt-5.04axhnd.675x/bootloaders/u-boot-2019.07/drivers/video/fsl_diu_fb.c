// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007, 2010-2011 Freescale Semiconductor, Inc.
 * Authors: York Sun <yorksun@freescale.com>
 *          Timur Tabi <timur@freescale.com>
 *
 * FSL DIU Framebuffer driver
 */

#include <common.h>
#include <malloc.h>
#include <asm/io.h>

#include "videomodes.h"
#include <video_fb.h>
#include <fsl_diu_fb.h>
#include <linux/list.h>
#include <linux/fb.h>

/* This setting is used for the ifm pdm360ng with PRIMEVIEW PM070WL3 */
static struct fb_videomode fsl_diu_mode_800_480 = {
	.name		= "800x480-60",
	.refresh	= 60,
	.xres		= 800,
	.yres		= 480,
	.pixclock	= 31250,
	.left_margin	= 86,
	.right_margin	= 42,
	.upper_margin	= 33,
	.lower_margin	= 10,
	.hsync_len	= 128,
	.vsync_len	= 2,
	.sync		= 0,
	.vmode		= FB_VMODE_NONINTERLACED
};

/* For the SHARP LQ084S3LG01, used on the P1022DS board */
static struct fb_videomode fsl_diu_mode_800_600 = {
	.name		= "800x600-60",
	.refresh	= 60,
	.xres		= 800,
	.yres		= 600,
	.pixclock	= 25000,
	.left_margin	= 88,
	.right_margin	= 40,
	.upper_margin	= 23,
	.lower_margin	= 1,
	.hsync_len	= 128,
	.vsync_len	= 4,
	.sync		= FB_SYNC_COMP_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode		= FB_VMODE_NONINTERLACED
};

/*
 * These parameters give default parameters
 * for video output 1024x768,
 * FIXME - change timing to proper amounts
 * hsync 31.5kHz, vsync 60Hz
 */
static struct fb_videomode fsl_diu_mode_1024_768 = {
	.name		= "1024x768-60",
	.refresh	= 60,
	.xres		= 1024,
	.yres		= 768,
	.pixclock	= 15385,
	.left_margin	= 160,
	.right_margin	= 24,
	.upper_margin	= 29,
	.lower_margin	= 3,
	.hsync_len	= 136,
	.vsync_len	= 6,
	.sync		= FB_SYNC_COMP_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode		= FB_VMODE_NONINTERLACED
};

static struct fb_videomode fsl_diu_mode_1280_1024 = {
	.name		= "1280x1024-60",
	.refresh	= 60,
	.xres		= 1280,
	.yres		= 1024,
	.pixclock	= 9375,
	.left_margin	= 38,
	.right_margin	= 128,
	.upper_margin	= 2,
	.lower_margin	= 7,
	.hsync_len	= 216,
	.vsync_len	= 37,
	.sync		= FB_SYNC_COMP_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode		= FB_VMODE_NONINTERLACED
};

static struct fb_videomode fsl_diu_mode_1280_720 = {
	.name		= "1280x720-60",
	.refresh	= 60,
	.xres		= 1280,
	.yres		= 720,
	.pixclock	= 13426,
	.left_margin	= 192,
	.right_margin	= 64,
	.upper_margin	= 22,
	.lower_margin	= 1,
	.hsync_len	= 136,
	.vsync_len	= 3,
	.sync		= FB_SYNC_COMP_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode		= FB_VMODE_NONINTERLACED
};

static struct fb_videomode fsl_diu_mode_1920_1080 = {
	.name		= "1920x1080-60",
	.refresh	= 60,
	.xres		= 1920,
	.yres		= 1080,
	.pixclock	= 5787,
	.left_margin	= 328,
	.right_margin	= 120,
	.upper_margin	= 34,
	.lower_margin	= 1,
	.hsync_len	= 208,
	.vsync_len	= 3,
	.sync		= FB_SYNC_COMP_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode		= FB_VMODE_NONINTERLACED
};

/*
 * These are the fields of area descriptor(in DDR memory) for every plane
 */
struct diu_ad {
	/* Word 0(32-bit) in DDR memory */
	__le32 pix_fmt; /* hard coding pixel format */
	/* Word 1(32-bit) in DDR memory */
	__le32 addr;
	/* Word 2(32-bit) in DDR memory */
	__le32 src_size_g_alpha;
	/* Word 3(32-bit) in DDR memory */
	__le32 aoi_size;
	/* Word 4(32-bit) in DDR memory */
	__le32 offset_xyi;
	/* Word 5(32-bit) in DDR memory */
	__le32 offset_xyd;
	/* Word 6(32-bit) in DDR memory */
	__le32 ckmax_r:8;
	__le32 ckmax_g:8;
	__le32 ckmax_b:8;
	__le32 res9:8;
	/* Word 7(32-bit) in DDR memory */
	__le32 ckmin_r:8;
	__le32 ckmin_g:8;
	__le32 ckmin_b:8;
	__le32 res10:8;
	/* Word 8(32-bit) in DDR memory */
	__le32 next_ad;
	/* Word 9(32-bit) in DDR memory, just for 64-bit aligned */
	__le32 res[3];
} __attribute__ ((packed));

/*
 * DIU register map
 */
struct diu {
	__be32 desc[3];
	__be32 gamma;
	__be32 pallete;
	__be32 cursor;
	__be32 curs_pos;
	__be32 diu_mode;
	__be32 bgnd;
	__be32 bgnd_wb;
	__be32 disp_size;
	__be32 wb_size;
	__be32 wb_mem_addr;
	__be32 hsyn_para;
	__be32 vsyn_para;
	__be32 syn_pol;
	__be32 thresholds;
	__be32 int_status;
	__be32 int_mask;
	__be32 colorbar[8];
	__be32 filling;
	__be32 plut;
} __attribute__ ((packed));

struct diu_addr {
	void *vaddr;		/* Virtual address */
	u32 paddr;		/* 32-bit physical address */
	unsigned int offset;	/* Alignment offset */
};

static struct fb_info info;

/*
 * Align to 64-bit(8-byte), 32-byte, etc.
 */
static int allocate_buf(struct diu_addr *buf, u32 size, u32 bytes_align)
{
	u32 offset, ssize;
	u32 mask;

	ssize = size + bytes_align;
	buf->vaddr = malloc(ssize);
	if (!buf->vaddr)
		return -1;

	memset(buf->vaddr, 0, ssize);
	mask = bytes_align - 1;
	offset = (u32)buf->vaddr & mask;
	if (offset) {
		buf->offset = bytes_align - offset;
		buf->vaddr += offset;
	} else
		buf->offset = 0;

	buf->paddr = virt_to_phys(buf->vaddr);
	return 0;
}

/*
 * Allocate a framebuffer and an Area Descriptor that points to it.  Both
 * are created in the same memory block.  The Area Descriptor is updated to
 * point to the framebuffer memory. Memory is aligned as needed.
 */
static struct diu_ad *allocate_fb(unsigned int xres, unsigned int yres,
				  unsigned int depth, char **fb)
{
	unsigned long size = xres * yres * depth;
	struct diu_addr addr;
	struct diu_ad *ad;
	size_t ad_size = roundup(sizeof(struct diu_ad), 32);

	/*
	 * Allocate a memory block that holds the Area Descriptor and the
	 * frame buffer right behind it.  To keep the code simple, everything
	 * is aligned on a 32-byte address.
	 */
	if (allocate_buf(&addr, ad_size + size, 32) < 0)
		return NULL;

	ad = addr.vaddr;
	ad->addr = cpu_to_le32(addr.paddr + ad_size);
	ad->aoi_size = cpu_to_le32((yres << 16) | xres);
	ad->src_size_g_alpha = cpu_to_le32((yres << 12) | xres);
	ad->offset_xyi = 0;
	ad->offset_xyd = 0;

	if (fb)
		*fb = addr.vaddr + ad_size;

	return ad;
}

int fsl_diu_init(u16 xres, u16 yres, u32 pixel_format, int gamma_fix)
{
	struct fb_videomode *fsl_diu_mode_db;
	struct diu_ad *ad;
	struct diu *hw = (struct diu *)CONFIG_SYS_DIU_ADDR;
	u8 *gamma_table_base;
	unsigned int i, j;
	struct diu_addr gamma;
	struct diu_addr cursor;

/* Convert the X,Y resolution pair into a single number */
#define RESOLUTION(x, y) (((u32)(x) << 16) | (y))

	switch (RESOLUTION(xres, yres)) {
	case RESOLUTION(800, 480):
		fsl_diu_mode_db = &fsl_diu_mode_800_480;
		break;
	case RESOLUTION(800, 600):
		fsl_diu_mode_db = &fsl_diu_mode_800_600;
		break;
	case RESOLUTION(1024, 768):
		fsl_diu_mode_db = &fsl_diu_mode_1024_768;
		break;
	case RESOLUTION(1280, 1024):
		fsl_diu_mode_db = &fsl_diu_mode_1280_1024;
		break;
	case RESOLUTION(1280, 720):
		fsl_diu_mode_db = &fsl_diu_mode_1280_720;
		break;
	case RESOLUTION(1920, 1080):
		fsl_diu_mode_db = &fsl_diu_mode_1920_1080;
		break;
	default:
		printf("DIU:   Unsupported resolution %ux%u\n", xres, yres);
		return -1;
	}

	/* read mode info */
	info.var.xres = fsl_diu_mode_db->xres;
	info.var.yres = fsl_diu_mode_db->yres;
	info.var.bits_per_pixel = 32;
	info.var.pixclock = fsl_diu_mode_db->pixclock;
	info.var.left_margin = fsl_diu_mode_db->left_margin;
	info.var.right_margin = fsl_diu_mode_db->right_margin;
	info.var.upper_margin = fsl_diu_mode_db->upper_margin;
	info.var.lower_margin = fsl_diu_mode_db->lower_margin;
	info.var.hsync_len = fsl_diu_mode_db->hsync_len;
	info.var.vsync_len = fsl_diu_mode_db->vsync_len;
	info.var.sync = fsl_diu_mode_db->sync;
	info.var.vmode = fsl_diu_mode_db->vmode;
	info.fix.line_length = info.var.xres * info.var.bits_per_pixel / 8;

	/* Memory allocation for framebuffer */
	info.screen_size =
		info.var.xres * info.var.yres * (info.var.bits_per_pixel / 8);
	ad = allocate_fb(info.var.xres, info.var.yres,
			 info.var.bits_per_pixel / 8, &info.screen_base);
	if (!ad) {
		printf("DIU:   Out of memory\n");
		return -1;
	}

	ad->pix_fmt = pixel_format;

	/* Disable chroma keying function */
	ad->ckmax_r = 0;
	ad->ckmax_g = 0;
	ad->ckmax_b = 0;

	ad->ckmin_r = 255;
	ad->ckmin_g = 255;
	ad->ckmin_b = 255;

	/* Initialize the gamma table */
	if (allocate_buf(&gamma, 256 * 3, 32) < 0) {
		printf("DIU:   Out of memory\n");
		return -1;
	}
	gamma_table_base = gamma.vaddr;
	for (i = 0; i <= 2; i++)
		for (j = 0; j < 256; j++)
			*gamma_table_base++ = j;

	if (gamma_fix == 1) {	/* fix the gamma */
		gamma_table_base = gamma.vaddr;
		for (i = 0; i < 256 * 3; i++) {
			gamma_table_base[i] = (gamma_table_base[i] << 2)
				| ((gamma_table_base[i] >> 6) & 0x03);
		}
	}

	/* Initialize the cursor */
	if (allocate_buf(&cursor, 32 * 32 * 2, 32) < 0) {
		printf("DIU:   Can't alloc cursor data\n");
		return -1;
	}

	/* Program DIU registers */
	out_be32(&hw->diu_mode, 0);	/* Temporarily disable the DIU */

	out_be32(&hw->gamma, gamma.paddr);
	out_be32(&hw->cursor, cursor.paddr);
	out_be32(&hw->bgnd, 0x007F7F7F);
	out_be32(&hw->disp_size, info.var.yres << 16 | info.var.xres);
	out_be32(&hw->hsyn_para, info.var.left_margin << 22 |
			info.var.hsync_len << 11 |
			info.var.right_margin);

	out_be32(&hw->vsyn_para, info.var.upper_margin << 22 |
			info.var.vsync_len << 11 |
			info.var.lower_margin);

	/* Pixel Clock configuration */
	diu_set_pixel_clock(info.var.pixclock);

	/* Set the frame buffers */
	out_be32(&hw->desc[0], virt_to_phys(ad));
	out_be32(&hw->desc[1], 0);
	out_be32(&hw->desc[2], 0);

	/* Enable the DIU, set display to all three planes */
	out_be32(&hw->diu_mode, 1);

	return 0;
}

void *video_hw_init(void)
{
	static GraphicDevice ctfb;
	const char *options;
	unsigned int depth = 0, freq = 0;

	if (!video_get_video_mode(&ctfb.winSizeX, &ctfb.winSizeY, &depth, &freq,
				  &options))
		return NULL;

	/* Find the monitor port, which is a required option */
	if (!options)
		return NULL;
	if (strncmp(options, "monitor=", 8) != 0)
		return NULL;

	if (platform_diu_init(ctfb.winSizeX, ctfb.winSizeY, options + 8) < 0)
		return NULL;

	/* fill in Graphic device struct */
	sprintf(ctfb.modeIdent, "%ix%ix%i %ikHz %iHz",
		ctfb.winSizeX, ctfb.winSizeY, depth, 64, freq);

	ctfb.frameAdrs = (unsigned int)info.screen_base;
	ctfb.plnSizeX = ctfb.winSizeX;
	ctfb.plnSizeY = ctfb.winSizeY;

	ctfb.gdfBytesPP = 4;
	ctfb.gdfIndex = GDF_32BIT_X888RGB;

	ctfb.isaBase = 0;
	ctfb.pciBase = 0;
	ctfb.memSize = info.screen_size;

	/* Cursor Start Address */
	ctfb.dprBase = 0;
	ctfb.vprBase = 0;
	ctfb.cprBase = 0;

	return &ctfb;
}
