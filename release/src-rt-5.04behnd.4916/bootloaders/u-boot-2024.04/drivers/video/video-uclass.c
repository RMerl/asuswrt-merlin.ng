// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#define LOG_CATEGORY UCLASS_VIDEO

#include <common.h>
#include <bloblist.h>
#include <console.h>
#include <cpu_func.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <spl.h>
#include <stdio_dev.h>
#include <video.h>
#include <video_console.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <dm/lists.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#ifdef CONFIG_SANDBOX
#include <asm/sdl.h>
#endif

/*
 * Theory of operation:
 *
 * Before relocation each device is bound. The driver for each device must
 * set the @align and @size values in struct video_uc_plat. This
 * information represents the requires size and alignment of the frame buffer
 * for the device. The values can be an over-estimate but cannot be too
 * small. The actual values will be suppled (in the same manner) by the bind()
 * method after relocation. Additionally driver can allocate frame buffer
 * itself by setting plat->base.
 *
 * This information is then picked up by video_reserve() which works out how
 * much memory is needed for all devices. This is allocated between
 * gd->video_bottom and gd->video_top.
 *
 * After relocation the same process occurs. The driver supplies the same
 * @size and @align information and this time video_post_bind() checks that
 * the drivers does not overflow the allocated memory.
 *
 * The frame buffer address is actually set (to plat->base) in
 * video_post_probe(). This function also clears the frame buffer and
 * allocates a suitable text console device. This can then be used to write
 * text to the video device.
 */
DECLARE_GLOBAL_DATA_PTR;

/**
 * struct video_uc_priv - Information for the video uclass
 *
 * @video_ptr: Current allocation position of the video framebuffer pointer.
 *	While binding devices after relocation, this points to the next
 *	available address to use for a device's framebuffer. It starts at
 *	gd->video_top and works downwards, running out of space when it hits
 *	gd->video_bottom.
 */
struct video_uc_priv {
	ulong video_ptr;
};

/** struct vid_rgb - Describes a video colour */
struct vid_rgb {
	u32 r;
	u32 g;
	u32 b;
};

void video_set_flush_dcache(struct udevice *dev, bool flush)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);

	priv->flush_dcache = flush;
}

static ulong alloc_fb_(ulong align, ulong size, ulong *addrp)
{
	ulong base;

	align = align ? align : 1 << 20;
	base = *addrp - size;
	base &= ~(align - 1);
	size = *addrp - base;
	*addrp = base;

	return size;
}

static ulong alloc_fb(struct udevice *dev, ulong *addrp)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	ulong size;

	if (!plat->size) {
		if (IS_ENABLED(CONFIG_VIDEO_COPY) && plat->copy_size) {
			size = alloc_fb_(plat->align, plat->copy_size, addrp);
			plat->copy_base = *addrp;
			return size;
		}

		return 0;
	}

	/* Allow drivers to allocate the frame buffer themselves */
	if (plat->base)
		return 0;

	size = alloc_fb_(plat->align, plat->size, addrp);
	plat->base = *addrp;

	return size;
}

int video_reserve(ulong *addrp)
{
	struct udevice *dev;
	ulong size;

	if (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && spl_phase() == PHASE_BOARD_F)
		return 0;

	gd->video_top = *addrp;
	for (uclass_find_first_device(UCLASS_VIDEO, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		size = alloc_fb(dev, addrp);
		debug("%s: Reserving %lx bytes at %lx for video device '%s'\n",
		      __func__, size, *addrp, dev->name);
	}

	/* Allocate space for PCI video devices in case there were not bound */
	if (*addrp == gd->video_top)
		*addrp -= CONFIG_VAL(VIDEO_PCI_DEFAULT_FB_SIZE);

	gd->video_bottom = *addrp;
	gd->fb_base = *addrp;
	debug("Video frame buffers from %lx to %lx\n", gd->video_bottom,
	      gd->video_top);

	return 0;
}

int video_fill_part(struct udevice *dev, int xstart, int ystart, int xend,
		    int yend, u32 colour)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);
	void *start, *line;
	int pixels = xend - xstart;
	int row, i, ret;

	start = priv->fb + ystart * priv->line_length;
	start += xstart * VNBYTES(priv->bpix);
	line = start;
	for (row = ystart; row < yend; row++) {
		switch (priv->bpix) {
		case VIDEO_BPP8: {
			u8 *dst = line;

			if (IS_ENABLED(CONFIG_VIDEO_BPP8)) {
				for (i = 0; i < pixels; i++)
					*dst++ = colour;
			}
			break;
		}
		case VIDEO_BPP16: {
			u16 *dst = line;

			if (IS_ENABLED(CONFIG_VIDEO_BPP16)) {
				for (i = 0; i < pixels; i++)
					*dst++ = colour;
			}
			break;
		}
		case VIDEO_BPP32: {
			u32 *dst = line;

			if (IS_ENABLED(CONFIG_VIDEO_BPP32)) {
				for (i = 0; i < pixels; i++)
					*dst++ = colour;
			}
			break;
		}
		default:
			return -ENOSYS;
		}
		line += priv->line_length;
	}
	ret = video_sync_copy(dev, start, line);
	if (ret)
		return ret;

	return 0;
}

int video_reserve_from_bloblist(struct video_handoff *ho)
{
	if (!ho->fb || ho->size == 0)
		return -ENOENT;

	gd->video_bottom = ho->fb;
	gd->fb_base = ho->fb;
	gd->video_top = ho->fb + ho->size;
	debug("%s: Reserving %lx bytes at %08x as per bloblist received\n",
	      __func__, (unsigned long)ho->size, (u32)ho->fb);

	return 0;
}

int video_fill(struct udevice *dev, u32 colour)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);
	int ret;

	switch (priv->bpix) {
	case VIDEO_BPP16:
		if (CONFIG_IS_ENABLED(VIDEO_BPP16)) {
			u16 *ppix = priv->fb;
			u16 *end = priv->fb + priv->fb_size;

			while (ppix < end)
				*ppix++ = colour;
			break;
		}
	case VIDEO_BPP32:
		if (CONFIG_IS_ENABLED(VIDEO_BPP32)) {
			u32 *ppix = priv->fb;
			u32 *end = priv->fb + priv->fb_size;

			while (ppix < end)
				*ppix++ = colour;
			break;
		}
	default:
		memset(priv->fb, colour, priv->fb_size);
		break;
	}
	ret = video_sync_copy(dev, priv->fb, priv->fb + priv->fb_size);
	if (ret)
		return ret;

	return video_sync(dev, false);
}

int video_clear(struct udevice *dev)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);
	int ret;

	ret = video_fill(dev, priv->colour_bg);
	if (ret)
		return ret;

	return 0;
}

static const struct vid_rgb colours[VID_COLOUR_COUNT] = {
	{ 0x00, 0x00, 0x00 },  /* black */
	{ 0xc0, 0x00, 0x00 },  /* red */
	{ 0x00, 0xc0, 0x00 },  /* green */
	{ 0xc0, 0x60, 0x00 },  /* brown */
	{ 0x00, 0x00, 0xc0 },  /* blue */
	{ 0xc0, 0x00, 0xc0 },  /* magenta */
	{ 0x00, 0xc0, 0xc0 },  /* cyan */
	{ 0xc0, 0xc0, 0xc0 },  /* light gray */
	{ 0x80, 0x80, 0x80 },  /* gray */
	{ 0xff, 0x00, 0x00 },  /* bright red */
	{ 0x00, 0xff, 0x00 },  /* bright green */
	{ 0xff, 0xff, 0x00 },  /* yellow */
	{ 0x00, 0x00, 0xff },  /* bright blue */
	{ 0xff, 0x00, 0xff },  /* bright magenta */
	{ 0x00, 0xff, 0xff },  /* bright cyan */
	{ 0xff, 0xff, 0xff },  /* white */
};

u32 video_index_to_colour(struct video_priv *priv, enum colour_idx idx)
{
	switch (priv->bpix) {
	case VIDEO_BPP16:
		if (CONFIG_IS_ENABLED(VIDEO_BPP16)) {
			return ((colours[idx].r >> 3) << 11) |
			       ((colours[idx].g >> 2) <<  5) |
			       ((colours[idx].b >> 3) <<  0);
		}
		break;
	case VIDEO_BPP32:
		if (CONFIG_IS_ENABLED(VIDEO_BPP32)) {
			switch (priv->format) {
			case VIDEO_X2R10G10B10:
				return (colours[idx].r << 22) |
				       (colours[idx].g << 12) |
				       (colours[idx].b <<  2);
			case VIDEO_RGBA8888:
				return (colours[idx].r << 24) |
				       (colours[idx].g << 16) |
				       (colours[idx].b << 8) | 0xff;
			default:
				return (colours[idx].r << 16) |
				       (colours[idx].g <<  8) |
				       (colours[idx].b <<  0);
			}
		}
		break;
	default:
		break;
	}

	/*
	 * For unknown bit arrangements just support
	 * black and white.
	 */
	if (idx)
		return 0xffffff; /* white */

	return 0x000000; /* black */
}

void video_set_default_colors(struct udevice *dev, bool invert)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);
	int fore, back;

	if (CONFIG_IS_ENABLED(SYS_WHITE_ON_BLACK)) {
		/* White is used when switching to bold, use light gray here */
		fore = VID_LIGHT_GRAY;
		back = VID_BLACK;
	} else {
		fore = VID_BLACK;
		back = VID_WHITE;
	}
	if (invert) {
		int temp;

		temp = fore;
		fore = back;
		back = temp;
	}
	priv->fg_col_idx = fore;
	priv->bg_col_idx = back;
	priv->colour_fg = video_index_to_colour(priv, fore);
	priv->colour_bg = video_index_to_colour(priv, back);
}

/* Flush video activity to the caches */
int video_sync(struct udevice *vid, bool force)
{
	struct video_ops *ops = video_get_ops(vid);
	int ret;

	if (ops && ops->video_sync) {
		ret = ops->video_sync(vid);
		if (ret)
			return ret;
	}

	/*
	 * flush_dcache_range() is declared in common.h but it seems that some
	 * architectures do not actually implement it. Is there a way to find
	 * out whether it exists? For now, ARM is safe.
	 */
#if defined(CONFIG_ARM) && !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	struct video_priv *priv = dev_get_uclass_priv(vid);

	if (priv->flush_dcache) {
		flush_dcache_range((ulong)priv->fb,
				   ALIGN((ulong)priv->fb + priv->fb_size,
					 CONFIG_SYS_CACHELINE_SIZE));
	}
#elif defined(CONFIG_VIDEO_SANDBOX_SDL)
	struct video_priv *priv = dev_get_uclass_priv(vid);
	static ulong last_sync;

	if (force || get_timer(last_sync) > 100) {
		sandbox_sdl_sync(priv->fb);
		last_sync = get_timer(0);
	}
#endif
	return 0;
}

void video_sync_all(void)
{
	struct udevice *dev;
	int ret;

	for (uclass_find_first_device(UCLASS_VIDEO, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		if (device_active(dev)) {
			ret = video_sync(dev, true);
			if (ret)
				dev_dbg(dev, "Video sync failed\n");
		}
	}
}

bool video_is_active(void)
{
	struct udevice *dev;

	for (uclass_find_first_device(UCLASS_VIDEO, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		if (device_active(dev))
			return true;
	}

	return false;
}

int video_get_xsize(struct udevice *dev)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);

	return priv->xsize;
}

int video_get_ysize(struct udevice *dev)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);

	return priv->ysize;
}

#ifdef CONFIG_VIDEO_COPY
int video_sync_copy(struct udevice *dev, void *from, void *to)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);

	if (priv->copy_fb) {
		long offset, size;

		/* Find the offset of the first byte to copy */
		if ((ulong)to > (ulong)from) {
			size = to - from;
			offset = from - priv->fb;
		} else {
			size = from - to;
			offset = to - priv->fb;
		}

		/*
		 * Allow a bit of leeway for valid requests somewhere near the
		 * frame buffer
		 */
		if (offset < -priv->fb_size || offset > 2 * priv->fb_size) {
#ifdef DEBUG
			char str[120];

			snprintf(str, sizeof(str),
				 "[** FAULT sync_copy fb=%p, from=%p, to=%p, offset=%lx]",
				 priv->fb, from, to, offset);
			console_puts_select_stderr(true, str);
#endif
			return -EFAULT;
		}

		/*
		 * Silently crop the memcpy. This allows callers to avoid doing
		 * this themselves. It is common for the end pointer to go a
		 * few lines after the end of the frame buffer, since most of
		 * the update algorithms terminate a line after their last write
		 */
		if (offset + size > priv->fb_size) {
			size = priv->fb_size - offset;
		} else if (offset < 0) {
			size += offset;
			offset = 0;
		}

		memcpy(priv->copy_fb + offset, priv->fb + offset, size);
	}

	return 0;
}

int video_sync_copy_all(struct udevice *dev)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);

	video_sync_copy(dev, priv->fb, priv->fb + priv->fb_size);

	return 0;
}

#endif

#define SPLASH_DECL(_name) \
	extern u8 __splash_ ## _name ## _begin[]; \
	extern u8 __splash_ ## _name ## _end[]

#define SPLASH_START(_name)	__splash_ ## _name ## _begin

SPLASH_DECL(u_boot_logo);

void *video_get_u_boot_logo(void)
{
	return SPLASH_START(u_boot_logo);
}

static int show_splash(struct udevice *dev)
{
	u8 *data = SPLASH_START(u_boot_logo);
	int ret;

	ret = video_bmp_display(dev, map_to_sysmem(data), -4, 4, true);

	return 0;
}

int video_default_font_height(struct udevice *dev)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);

	if (IS_ENABLED(CONFIG_CONSOLE_TRUETYPE))
		return IF_ENABLED_INT(CONFIG_CONSOLE_TRUETYPE,
				      CONFIG_CONSOLE_TRUETYPE_SIZE);

	return vc_priv->y_charsize;
}

/* Set up the display ready for use */
static int video_post_probe(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *priv = dev_get_uclass_priv(dev);
	char name[30], drv[15], *str;
	const char *drv_name = drv;
	struct udevice *cons;
	int ret;

	/* Set up the line and display size */
	priv->fb = map_sysmem(plat->base, plat->size);
	if (!priv->line_length)
		priv->line_length = priv->xsize * VNBYTES(priv->bpix);

	priv->fb_size = priv->line_length * priv->ysize;

	/*
	 * Set up video handoff fields for passing video blob to next stage
	 * NOTE:
	 * This assumes that reserved video memory only uses a single framebuffer
	 */
	if (spl_phase() == PHASE_SPL && CONFIG_IS_ENABLED(BLOBLIST)) {
		struct video_handoff *ho;

		ho = bloblist_add(BLOBLISTT_U_BOOT_VIDEO, sizeof(*ho), 0);
		if (!ho)
			return log_msg_ret("blf", -ENOENT);
		ho->fb = gd->video_bottom;
		/* Fill aligned size here as calculated in video_reserve() */
		ho->size = gd->video_top - gd->video_bottom;
		ho->xsize = priv->xsize;
		ho->ysize = priv->ysize;
		ho->line_length = priv->line_length;
		ho->bpix = priv->bpix;
	}

	if (IS_ENABLED(CONFIG_VIDEO_COPY) && plat->copy_base)
		priv->copy_fb = map_sysmem(plat->copy_base, plat->size);

	/* Set up colors  */
	video_set_default_colors(dev, false);

	if (!CONFIG_IS_ENABLED(NO_FB_CLEAR))
		video_clear(dev);

	/*
	 * Create a text console device. For now we always do this, although
	 * it might be useful to support only bitmap drawing on the device
	 * for boards that don't need to display text. We create a TrueType
	 * console if enabled, a rotated console if the video driver requests
	 * it, otherwise a normal console.
	 *
	 * The console can be override by setting vidconsole_drv_name before
	 * probing this video driver, or in the probe() method.
	 *
	 * TrueType does not support rotation at present so fall back to the
	 * rotated console in that case.
	 */
	if (!priv->rot && IS_ENABLED(CONFIG_CONSOLE_TRUETYPE)) {
		snprintf(name, sizeof(name), "%s.vidconsole_tt", dev->name);
		strcpy(drv, "vidconsole_tt");
	} else {
		snprintf(name, sizeof(name), "%s.vidconsole%d", dev->name,
			 priv->rot);
		snprintf(drv, sizeof(drv), "vidconsole%d", priv->rot);
	}

	str = strdup(name);
	if (!str)
		return -ENOMEM;
	if (priv->vidconsole_drv_name)
		drv_name = priv->vidconsole_drv_name;
	ret = device_bind_driver(dev, drv_name, str, &cons);
	if (ret) {
		debug("%s: Cannot bind console driver\n", __func__);
		return ret;
	}

	ret = device_probe(cons);
	if (ret) {
		debug("%s: Cannot probe console driver\n", __func__);
		return ret;
	}

	if (CONFIG_IS_ENABLED(VIDEO_LOGO) &&
	    !CONFIG_IS_ENABLED(SPLASH_SCREEN) && !plat->hide_logo) {
		ret = show_splash(dev);
		if (ret) {
			log_debug("Cannot show splash screen\n");
			return ret;
		}
	}

	return 0;
};

/* Post-relocation, allocate memory for the frame buffer */
static int video_post_bind(struct udevice *dev)
{
	struct video_uc_priv *uc_priv;
	ulong addr;
	ulong size;

	/* Before relocation there is nothing to do here */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	/* Set up the video pointer, if this is the first device */
	uc_priv = uclass_get_priv(dev->uclass);
	if (!uc_priv->video_ptr)
		uc_priv->video_ptr = gd->video_top;

	/* Allocate framebuffer space for this device */
	addr = uc_priv->video_ptr;
	size = alloc_fb(dev, &addr);
	if (addr < gd->video_bottom) {
		/*
		 * Device tree node may need the 'bootph-all' or
		 * 'bootph-some-ram' tag
		 */
		printf("Video device '%s' cannot allocate frame buffer memory "
		       "- ensure the device is set up before relocation\n",
		       dev->name);
		return -ENOSPC;
	}
	debug("%s: Claiming %lx bytes at %lx for video device '%s'\n",
	      __func__, size, addr, dev->name);
	uc_priv->video_ptr = addr;

	return 0;
}

UCLASS_DRIVER(video) = {
	.id		= UCLASS_VIDEO,
	.name		= "video",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.post_bind	= video_post_bind,
	.post_probe	= video_post_probe,
	.priv_auto	= sizeof(struct video_uc_priv),
	.per_device_auto	= sizeof(struct video_priv),
	.per_device_plat_auto	= sizeof(struct video_uc_plat),
};
