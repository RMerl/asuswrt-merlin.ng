/*
 * Video uclass and legacy implementation
 *
 * Copyright (c) 2015 Google, Inc
 *
 * MPC823 Video Controller
 * =======================
 * (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
 * AIRVENT SAM s.p.a - RIMINI(ITALY)
 *
 */

#ifndef _VIDEO_H_
#define _VIDEO_H_

#ifdef CONFIG_DM_VIDEO

#include <stdio_dev.h>

struct video_uc_platdata {
	uint align;
	uint size;
	ulong base;
};

enum video_polarity {
	VIDEO_ACTIVE_HIGH,	/* Pins are active high */
	VIDEO_ACTIVE_LOW,	/* Pins are active low */
};

/*
 * Bits per pixel selector. Each value n is such that the bits-per-pixel is
 * 2 ^ n
 */
enum video_log2_bpp {
	VIDEO_BPP1	= 0,
	VIDEO_BPP2,
	VIDEO_BPP4,
	VIDEO_BPP8,
	VIDEO_BPP16,
	VIDEO_BPP32,
};

/*
 * Convert enum video_log2_bpp to bytes and bits. Note we omit the outer
 * brackets to allow multiplication by fractional pixels.
 */
#define VNBYTES(bpix)	(1 << (bpix)) / 8

#define VNBITS(bpix)	(1 << (bpix))

/**
 * struct video_priv - Device information used by the video uclass
 *
 * @xsize:	Number of pixel columns (e.g. 1366)
 * @ysize:	Number of pixels rows (e.g.. 768)
 * @rot:	Display rotation (0=none, 1=90 degrees clockwise, etc.)
 * @bpix:	Encoded bits per pixel (enum video_log2_bpp)
 * @vidconsole_drv_name:	Driver to use for the text console, NULL to
 *		select automatically
 * @font_size:	Font size in pixels (0 to use a default value)
 * @fb:		Frame buffer
 * @fb_size:	Frame buffer size
 * @line_length:	Length of each frame buffer line, in bytes. This can be
 *		set by the driver, but if not, the uclass will set it after
 *		probing
 * @colour_fg:	Foreground colour (pixel value)
 * @colour_bg:	Background colour (pixel value)
 * @flush_dcache:	true to enable flushing of the data cache after
 *		the LCD is updated
 * @cmap:	Colour map for 8-bit-per-pixel displays
 * @fg_col_idx:	Foreground color code (bit 3 = bold, bit 0-2 = color)
 * @bg_col_idx:	Background color code (bit 3 = bold, bit 0-2 = color)
 */
struct video_priv {
	/* Things set up by the driver: */
	ushort xsize;
	ushort ysize;
	ushort rot;
	enum video_log2_bpp bpix;
	const char *vidconsole_drv_name;
	int font_size;

	/*
	 * Things that are private to the uclass: don't use these in the
	 * driver
	 */
	void *fb;
	int fb_size;
	int line_length;
	u32 colour_fg;
	u32 colour_bg;
	bool flush_dcache;
	ushort *cmap;
	u8 fg_col_idx;
	u8 bg_col_idx;
};

/* Placeholder - there are no video operations at present */
struct video_ops {
};

#define video_get_ops(dev)        ((struct video_ops *)(dev)->driver->ops)

/**
 * video_reserve() - Reserve frame-buffer memory for video devices
 *
 * Note: This function is for internal use.
 *
 * This uses the uclass platdata's @size and @align members to figure out
 * a size and position for each frame buffer as part of the pre-relocation
 * process of determining the post-relocation memory layout.
 *
 * gd->video_top is set to the initial value of *@addrp and gd->video_bottom
 * is set to the final value.
 *
 * @addrp:	On entry, the top of available memory. On exit, the new top,
 *		after allocating the required memory.
 * @return 0
 */
int video_reserve(ulong *addrp);

/**
 * video_clear() - Clear a device's frame buffer to background color.
 *
 * @dev:	Device to clear
 * @return 0
 */
int video_clear(struct udevice *dev);

/**
 * video_sync() - Sync a device's frame buffer with its hardware
 *
 * Some frame buffers are cached or have a secondary frame buffer. This
 * function syncs these up so that the current contents of the U-Boot frame
 * buffer are displayed to the user.
 *
 * @dev:	Device to sync
 * @force:	True to force a sync even if there was one recently (this is
 *		very expensive on sandbox)
 */
void video_sync(struct udevice *vid, bool force);

/**
 * video_sync_all() - Sync all devices' frame buffers with there hardware
 *
 * This calls video_sync() on all active video devices.
 */
void video_sync_all(void);

/**
 * video_bmp_display() - Display a BMP file
 *
 * @dev:	Device to display the bitmap on
 * @bmp_image:	Address of bitmap image to display
 * @x:		X position in pixels from the left
 * @y:		Y position in pixels from the top
 * @align:	true to adjust the coordinates to centre the image. If false
 *		the coordinates are used as is. If true:
 *
 *		- if a coordinate is 0x7fff then the image will be centred in
 *		  that direction
 *		- if a coordinate is -ve then it will be offset to the
 *		  left/top of the centre by that many pixels
 *		- if a coordinate is positive it will be used unchnaged.
 * @return 0 if OK, -ve on error
 */
int video_bmp_display(struct udevice *dev, ulong bmp_image, int x, int y,
		      bool align);

/**
 * video_get_xsize() - Get the width of the display in pixels
 *
 * @dev:	Device to check
 * @return device frame buffer width in pixels
 */
int video_get_xsize(struct udevice *dev);

/**
 * video_get_ysize() - Get the height of the display in pixels
 *
 * @dev:	Device to check
 * @return device frame buffer height in pixels
 */
int video_get_ysize(struct udevice *dev);

/**
 * Set whether we need to flush the dcache when changing the image. This
 * defaults to off.
 *
 * @param flush		non-zero to flush cache after update, 0 to skip
 */
void video_set_flush_dcache(struct udevice *dev, bool flush);

/**
 * Set default colors and attributes
 *
 * @dev:	video device
 * @invert	true to invert colours
 */
void video_set_default_colors(struct udevice *dev, bool invert);

#endif /* CONFIG_DM_VIDEO */

#ifndef CONFIG_DM_VIDEO

/* Video functions */

/**
 * Display a BMP format bitmap on the screen
 *
 * @param bmp_image	Address of BMP image
 * @param x		X position to draw image
 * @param y		Y position to draw image
 */
int video_display_bitmap(ulong bmp_image, int x, int y);

/**
 * Get the width of the screen in pixels
 *
 * @return width of screen in pixels
 */
int video_get_pixel_width(void);

/**
 * Get the height of the screen in pixels
 *
 * @return height of screen in pixels
 */
int video_get_pixel_height(void);

/**
 * Get the number of text lines/rows on the screen
 *
 * @return number of rows
 */
int video_get_screen_rows(void);

/**
 * Get the number of text columns on the screen
 *
 * @return number of columns
 */
int video_get_screen_columns(void);

/**
 * Set the position of the text cursor
 *
 * @param col	Column to place cursor (0 = left side)
 * @param row	Row to place cursor (0 = top line)
 */
void video_position_cursor(unsigned col, unsigned row);

/* Clear the display */
void video_clear(void);

#if defined(CONFIG_FORMIKE)
int kwh043st20_f01_spi_startup(unsigned int bus, unsigned int cs,
	unsigned int max_hz, unsigned int spi_mode);
#endif
#if defined(CONFIG_LG4573)
int lg4573_spi_startup(unsigned int bus, unsigned int cs,
	unsigned int max_hz, unsigned int spi_mode);
#endif

/*
 * video_get_info_str() - obtain a board string: type, speed, etc.
 *
 * This is called if CONFIG_CONSOLE_EXTRA_INFO is enabled.
 *
 * line_number:	location to place info string beside logo
 * info:	buffer for info string (empty if nothing to display on this
 * line)
 */
void video_get_info_str(int line_number, char *info);

#endif /* !CONFIG_DM_VIDEO */

#endif
