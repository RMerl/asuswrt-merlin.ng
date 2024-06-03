// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application disk support
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#include <common.h>
#include <dm.h>
#include <efi_loader.h>
#include <lcd.h>
#include <malloc.h>
#include <video.h>

DECLARE_GLOBAL_DATA_PTR;

static const efi_guid_t efi_gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

/**
 * struct efi_gop_obj - graphical output protocol object
 *
 * @header:	EFI object header
 * @ops:	graphical output protocol interface
 * @info:	graphical output mode information
 * @mode:	graphical output mode
 * @bpix:	bits per pixel
 * @fb:		frame buffer
 */
struct efi_gop_obj {
	struct efi_object header;
	struct efi_gop ops;
	struct efi_gop_mode_info info;
	struct efi_gop_mode mode;
	/* Fields we only have access to during init */
	u32 bpix;
	void *fb;
};

static efi_status_t EFIAPI gop_query_mode(struct efi_gop *this, u32 mode_number,
					  efi_uintn_t *size_of_info,
					  struct efi_gop_mode_info **info)
{
	struct efi_gop_obj *gopobj;
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %x, %p, %p", this, mode_number, size_of_info, info);

	if (!this || !size_of_info || !info || mode_number) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	gopobj = container_of(this, struct efi_gop_obj, ops);
	ret = efi_allocate_pool(EFI_BOOT_SERVICES_DATA, sizeof(gopobj->info),
				(void **)info);
	if (ret != EFI_SUCCESS)
		goto out;
	*size_of_info = sizeof(gopobj->info);
	memcpy(*info, &gopobj->info, sizeof(gopobj->info));

out:
	return EFI_EXIT(ret);
}

static __always_inline struct efi_gop_pixel efi_vid16_to_blt_col(u16 vid)
{
	struct efi_gop_pixel blt = {
		.reserved = 0,
	};

	blt.blue  = (vid & 0x1f) << 3;
	vid >>= 5;
	blt.green = (vid & 0x3f) << 2;
	vid >>= 6;
	blt.red   = (vid & 0x1f) << 3;
	return blt;
}

static __always_inline u16 efi_blt_col_to_vid16(struct efi_gop_pixel *blt)
{
	return (u16)(blt->red   >> 3) << 11 |
	       (u16)(blt->green >> 2) <<  5 |
	       (u16)(blt->blue  >> 3);
}

static __always_inline efi_status_t gop_blt_int(struct efi_gop *this,
						struct efi_gop_pixel *bufferp,
						u32 operation, efi_uintn_t sx,
						efi_uintn_t sy, efi_uintn_t dx,
						efi_uintn_t dy,
						efi_uintn_t width,
						efi_uintn_t height,
						efi_uintn_t delta,
						efi_uintn_t vid_bpp)
{
	struct efi_gop_obj *gopobj = container_of(this, struct efi_gop_obj, ops);
	efi_uintn_t i, j, linelen, slineoff = 0, dlineoff, swidth, dwidth;
	u32 *fb32 = gopobj->fb;
	u16 *fb16 = gopobj->fb;
	struct efi_gop_pixel *buffer = __builtin_assume_aligned(bufferp, 4);

	if (delta) {
		/* Check for 4 byte alignment */
		if (delta & 3)
			return EFI_INVALID_PARAMETER;
		linelen = delta >> 2;
	} else {
		linelen = width;
	}

	/* Check source rectangle */
	switch (operation) {
	case EFI_BLT_VIDEO_FILL:
		break;
	case EFI_BLT_BUFFER_TO_VIDEO:
		if (sx + width > linelen)
			return EFI_INVALID_PARAMETER;
		break;
	case EFI_BLT_VIDEO_TO_BLT_BUFFER:
	case EFI_BLT_VIDEO_TO_VIDEO:
		if (sx + width > gopobj->info.width ||
		    sy + height > gopobj->info.height)
			return EFI_INVALID_PARAMETER;
		break;
	default:
		return EFI_INVALID_PARAMETER;
	}

	/* Check destination rectangle */
	switch (operation) {
	case EFI_BLT_VIDEO_FILL:
	case EFI_BLT_BUFFER_TO_VIDEO:
	case EFI_BLT_VIDEO_TO_VIDEO:
		if (dx + width > gopobj->info.width ||
		    dy + height > gopobj->info.height)
			return EFI_INVALID_PARAMETER;
		break;
	case EFI_BLT_VIDEO_TO_BLT_BUFFER:
		if (dx + width > linelen)
			return EFI_INVALID_PARAMETER;
		break;
	}

	/* Calculate line width */
	switch (operation) {
	case EFI_BLT_BUFFER_TO_VIDEO:
		swidth = linelen;
		break;
	case EFI_BLT_VIDEO_TO_BLT_BUFFER:
	case EFI_BLT_VIDEO_TO_VIDEO:
		swidth = gopobj->info.width;
		if (!vid_bpp)
			return EFI_UNSUPPORTED;
		break;
	case EFI_BLT_VIDEO_FILL:
		swidth = 0;
		break;
	}

	switch (operation) {
	case EFI_BLT_BUFFER_TO_VIDEO:
	case EFI_BLT_VIDEO_FILL:
	case EFI_BLT_VIDEO_TO_VIDEO:
		dwidth = gopobj->info.width;
		if (!vid_bpp)
			return EFI_UNSUPPORTED;
		break;
	case EFI_BLT_VIDEO_TO_BLT_BUFFER:
		dwidth = linelen;
		break;
	}

	slineoff = swidth * sy;
	dlineoff = dwidth * dy;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			struct efi_gop_pixel pix;

			/* Read source pixel */
			switch (operation) {
			case EFI_BLT_VIDEO_FILL:
				pix = *buffer;
				break;
			case EFI_BLT_BUFFER_TO_VIDEO:
				pix = buffer[slineoff + j + sx];
				break;
			case EFI_BLT_VIDEO_TO_BLT_BUFFER:
			case EFI_BLT_VIDEO_TO_VIDEO:
				if (vid_bpp == 32)
					pix = *(struct efi_gop_pixel *)&fb32[
						slineoff + j + sx];
				else
					pix = efi_vid16_to_blt_col(fb16[
						slineoff + j + sx]);
				break;
			}

			/* Write destination pixel */
			switch (operation) {
			case EFI_BLT_VIDEO_TO_BLT_BUFFER:
				buffer[dlineoff + j + dx] = pix;
				break;
			case EFI_BLT_BUFFER_TO_VIDEO:
			case EFI_BLT_VIDEO_FILL:
			case EFI_BLT_VIDEO_TO_VIDEO:
				if (vid_bpp == 32)
					fb32[dlineoff + j + dx] = *(u32 *)&pix;
				else
					fb16[dlineoff + j + dx] =
						efi_blt_col_to_vid16(&pix);
				break;
			}
		}
		slineoff += swidth;
		dlineoff += dwidth;
	}

	return EFI_SUCCESS;
}

static efi_uintn_t gop_get_bpp(struct efi_gop *this)
{
	struct efi_gop_obj *gopobj = container_of(this, struct efi_gop_obj, ops);
	efi_uintn_t vid_bpp = 0;

	switch (gopobj->bpix) {
#ifdef CONFIG_DM_VIDEO
	case VIDEO_BPP32:
#else
	case LCD_COLOR32:
#endif
		vid_bpp = 32;
		break;
#ifdef CONFIG_DM_VIDEO
	case VIDEO_BPP16:
#else
	case LCD_COLOR16:
#endif
		vid_bpp = 16;
		break;
	}

	return vid_bpp;
}

/*
 * GCC can't optimize our BLT function well, but we need to make sure that
 * our 2-dimensional loop gets executed very quickly, otherwise the system
 * will feel slow.
 *
 * By manually putting all obvious branch targets into functions which call
 * our generic BLT function with constants, the compiler can successfully
 * optimize for speed.
 */
static efi_status_t gop_blt_video_fill(struct efi_gop *this,
				       struct efi_gop_pixel *buffer,
				       u32 foo, efi_uintn_t sx,
				       efi_uintn_t sy, efi_uintn_t dx,
				       efi_uintn_t dy, efi_uintn_t width,
				       efi_uintn_t height, efi_uintn_t delta,
				       efi_uintn_t vid_bpp)
{
	return gop_blt_int(this, buffer, EFI_BLT_VIDEO_FILL, sx, sy, dx,
			   dy, width, height, delta, vid_bpp);
}

static efi_status_t gop_blt_buf_to_vid16(struct efi_gop *this,
					 struct efi_gop_pixel *buffer,
					 u32 foo, efi_uintn_t sx,
					 efi_uintn_t sy, efi_uintn_t dx,
					 efi_uintn_t dy, efi_uintn_t width,
					 efi_uintn_t height, efi_uintn_t delta)
{
	return gop_blt_int(this, buffer, EFI_BLT_BUFFER_TO_VIDEO, sx, sy, dx,
			   dy, width, height, delta, 16);
}

static efi_status_t gop_blt_buf_to_vid32(struct efi_gop *this,
					 struct efi_gop_pixel *buffer,
					 u32 foo, efi_uintn_t sx,
					 efi_uintn_t sy, efi_uintn_t dx,
					 efi_uintn_t dy, efi_uintn_t width,
					 efi_uintn_t height, efi_uintn_t delta)
{
	return gop_blt_int(this, buffer, EFI_BLT_BUFFER_TO_VIDEO, sx, sy, dx,
			   dy, width, height, delta, 32);
}

static efi_status_t gop_blt_vid_to_vid(struct efi_gop *this,
				       struct efi_gop_pixel *buffer,
				       u32 foo, efi_uintn_t sx,
				       efi_uintn_t sy, efi_uintn_t dx,
				       efi_uintn_t dy, efi_uintn_t width,
				       efi_uintn_t height, efi_uintn_t delta,
				       efi_uintn_t vid_bpp)
{
	return gop_blt_int(this, buffer, EFI_BLT_VIDEO_TO_VIDEO, sx, sy, dx,
			   dy, width, height, delta, vid_bpp);
}

static efi_status_t gop_blt_vid_to_buf(struct efi_gop *this,
				       struct efi_gop_pixel *buffer,
				       u32 foo, efi_uintn_t sx,
				       efi_uintn_t sy, efi_uintn_t dx,
				       efi_uintn_t dy, efi_uintn_t width,
				       efi_uintn_t height, efi_uintn_t delta,
				       efi_uintn_t vid_bpp)
{
	return gop_blt_int(this, buffer, EFI_BLT_VIDEO_TO_BLT_BUFFER, sx, sy,
			   dx, dy, width, height, delta, vid_bpp);
}

/**
 * gop_set_mode() - set graphical output mode
 *
 * This function implements the SetMode() service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:		the graphical output protocol
 * @model_number:	the mode to be set
 * Return:		status code
 */
static efi_status_t EFIAPI gop_set_mode(struct efi_gop *this, u32 mode_number)
{
	struct efi_gop_obj *gopobj;
	struct efi_gop_pixel buffer = {0, 0, 0, 0};
	efi_uintn_t vid_bpp;
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %x", this, mode_number);

	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	if (mode_number) {
		ret = EFI_UNSUPPORTED;
		goto out;
	}
	gopobj = container_of(this, struct efi_gop_obj, ops);
	vid_bpp = gop_get_bpp(this);
	ret = gop_blt_video_fill(this, &buffer, EFI_BLT_VIDEO_FILL, 0, 0, 0, 0,
				 gopobj->info.width, gopobj->info.height, 0,
				 vid_bpp);
out:
	return EFI_EXIT(ret);
}

/*
 * Copy rectangle.
 *
 * This function implements the Blt service of the EFI_GRAPHICS_OUTPUT_PROTOCOL.
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:	EFI_GRAPHICS_OUTPUT_PROTOCOL
 * @buffer:	pixel buffer
 * @sx:		source x-coordinate
 * @sy:		source y-coordinate
 * @dx:		destination x-coordinate
 * @dy:		destination y-coordinate
 * @width:	width of rectangle
 * @height:	height of rectangle
 * @delta:	length in bytes of a line in the pixel buffer (optional)
 * @return:	status code
 */
efi_status_t EFIAPI gop_blt(struct efi_gop *this, struct efi_gop_pixel *buffer,
			    u32 operation, efi_uintn_t sx,
			    efi_uintn_t sy, efi_uintn_t dx,
			    efi_uintn_t dy, efi_uintn_t width,
			    efi_uintn_t height, efi_uintn_t delta)
{
	efi_status_t ret = EFI_INVALID_PARAMETER;
	efi_uintn_t vid_bpp;

	EFI_ENTRY("%p, %p, %u, %zu, %zu, %zu, %zu, %zu, %zu, %zu", this,
		  buffer, operation, sx, sy, dx, dy, width, height, delta);

	vid_bpp = gop_get_bpp(this);

	/* Allow for compiler optimization */
	switch (operation) {
	case EFI_BLT_VIDEO_FILL:
		ret = gop_blt_video_fill(this, buffer, operation, sx, sy, dx,
					 dy, width, height, delta, vid_bpp);
		break;
	case EFI_BLT_BUFFER_TO_VIDEO:
		/* This needs to be super-fast, so duplicate for 16/32bpp */
		if (vid_bpp == 32)
			ret = gop_blt_buf_to_vid32(this, buffer, operation, sx,
						   sy, dx, dy, width, height,
						   delta);
		else
			ret = gop_blt_buf_to_vid16(this, buffer, operation, sx,
						   sy, dx, dy, width, height,
						   delta);
		break;
	case EFI_BLT_VIDEO_TO_VIDEO:
		ret = gop_blt_vid_to_vid(this, buffer, operation, sx, sy, dx,
					 dy, width, height, delta, vid_bpp);
		break;
	case EFI_BLT_VIDEO_TO_BLT_BUFFER:
		ret = gop_blt_vid_to_buf(this, buffer, operation, sx, sy, dx,
					 dy, width, height, delta, vid_bpp);
		break;
	default:
		ret = EFI_INVALID_PARAMETER;
	}

	if (ret != EFI_SUCCESS)
		return EFI_EXIT(ret);

#ifdef CONFIG_DM_VIDEO
	video_sync_all();
#else
	lcd_sync();
#endif

	return EFI_EXIT(EFI_SUCCESS);
}

/*
 * Install graphical output protocol.
 *
 * If no supported video device exists this is not considered as an
 * error.
 */
efi_status_t efi_gop_register(void)
{
	struct efi_gop_obj *gopobj;
	u32 bpix, col, row;
	u64 fb_base, fb_size;
	void *fb;
	efi_status_t ret;

#ifdef CONFIG_DM_VIDEO
	struct udevice *vdev;
	struct video_priv *priv;

	/* We only support a single video output device for now */
	if (uclass_first_device(UCLASS_VIDEO, &vdev) || !vdev) {
		debug("WARNING: No video device\n");
		return EFI_SUCCESS;
	}

	priv = dev_get_uclass_priv(vdev);
	bpix = priv->bpix;
	col = video_get_xsize(vdev);
	row = video_get_ysize(vdev);
	fb_base = (uintptr_t)priv->fb;
	fb_size = priv->fb_size;
	fb = priv->fb;
#else
	int line_len;

	bpix = panel_info.vl_bpix;
	col = panel_info.vl_col;
	row = panel_info.vl_row;
	fb_base = gd->fb_base;
	fb_size = lcd_get_size(&line_len);
	fb = (void*)gd->fb_base;
#endif

	switch (bpix) {
#ifdef CONFIG_DM_VIDEO
	case VIDEO_BPP16:
	case VIDEO_BPP32:
#else
	case LCD_COLOR32:
	case LCD_COLOR16:
#endif
		break;
	default:
		/* So far, we only work in 16 or 32 bit mode */
		debug("WARNING: Unsupported video mode\n");
		return EFI_SUCCESS;
	}

	gopobj = calloc(1, sizeof(*gopobj));
	if (!gopobj) {
		printf("ERROR: Out of memory\n");
		return EFI_OUT_OF_RESOURCES;
	}

	/* Hook up to the device list */
	efi_add_handle(&gopobj->header);

	/* Fill in object data */
	ret = efi_add_protocol(&gopobj->header, &efi_gop_guid,
			       &gopobj->ops);
	if (ret != EFI_SUCCESS) {
		printf("ERROR: Failure adding GOP protocol\n");
		return ret;
	}
	gopobj->ops.query_mode = gop_query_mode;
	gopobj->ops.set_mode = gop_set_mode;
	gopobj->ops.blt = gop_blt;
	gopobj->ops.mode = &gopobj->mode;

	gopobj->mode.max_mode = 1;
	gopobj->mode.info = &gopobj->info;
	gopobj->mode.info_size = sizeof(gopobj->info);

	gopobj->mode.fb_base = fb_base;
	gopobj->mode.fb_size = fb_size;

	gopobj->info.version = 0;
	gopobj->info.width = col;
	gopobj->info.height = row;
#ifdef CONFIG_DM_VIDEO
	if (bpix == VIDEO_BPP32)
#else
	if (bpix == LCD_COLOR32)
#endif
	{
		gopobj->info.pixel_format = EFI_GOT_BGRA8;
	} else {
		gopobj->info.pixel_format = EFI_GOT_BITMASK;
		gopobj->info.pixel_bitmask[0] = 0xf800; /* red */
		gopobj->info.pixel_bitmask[1] = 0x07e0; /* green */
		gopobj->info.pixel_bitmask[2] = 0x001f; /* blue */
	}
	gopobj->info.pixels_per_scanline = col;
	gopobj->bpix = bpix;
	gopobj->fb = fb;

	return EFI_SUCCESS;
}
