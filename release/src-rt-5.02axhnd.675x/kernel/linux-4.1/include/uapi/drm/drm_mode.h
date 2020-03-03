/*
 * Copyright (c) 2007 Dave Airlie <airlied@linux.ie>
 * Copyright (c) 2007 Jakob Bornecrantz <wallbraker@gmail.com>
 * Copyright (c) 2008 Red Hat Inc.
 * Copyright (c) 2007-2008 Tungsten Graphics, Inc., Cedar Park, TX., USA
 * Copyright (c) 2007-2008 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef _DRM_MODE_H
#define _DRM_MODE_H

#include <linux/types.h>

#define DRM_DISPLAY_INFO_LEN	32
#define DRM_CONNECTOR_NAME_LEN	32
#define DRM_DISPLAY_MODE_LEN	32
#define DRM_PROP_NAME_LEN	32

#define DRM_MODE_TYPE_BUILTIN	(1<<0)
#define DRM_MODE_TYPE_CLOCK_C	((1<<1) | DRM_MODE_TYPE_BUILTIN)
#define DRM_MODE_TYPE_CRTC_C	((1<<2) | DRM_MODE_TYPE_BUILTIN)
#define DRM_MODE_TYPE_PREFERRED	(1<<3)
#define DRM_MODE_TYPE_DEFAULT	(1<<4)
#define DRM_MODE_TYPE_USERDEF	(1<<5)
#define DRM_MODE_TYPE_DRIVER	(1<<6)

/* Video mode flags */
/* bit compatible with the xorg definitions. */
#define DRM_MODE_FLAG_PHSYNC			(1<<0)
#define DRM_MODE_FLAG_NHSYNC			(1<<1)
#define DRM_MODE_FLAG_PVSYNC			(1<<2)
#define DRM_MODE_FLAG_NVSYNC			(1<<3)
#define DRM_MODE_FLAG_INTERLACE			(1<<4)
#define DRM_MODE_FLAG_DBLSCAN			(1<<5)
#define DRM_MODE_FLAG_CSYNC			(1<<6)
#define DRM_MODE_FLAG_PCSYNC			(1<<7)
#define DRM_MODE_FLAG_NCSYNC			(1<<8)
#define DRM_MODE_FLAG_HSKEW			(1<<9) /* hskew provided */
#define DRM_MODE_FLAG_BCAST			(1<<10)
#define DRM_MODE_FLAG_PIXMUX			(1<<11)
#define DRM_MODE_FLAG_DBLCLK			(1<<12)
#define DRM_MODE_FLAG_CLKDIV2			(1<<13)
 /*
  * When adding a new stereo mode don't forget to adjust DRM_MODE_FLAGS_3D_MAX
  * (define not exposed to user space).
  */
#define DRM_MODE_FLAG_3D_MASK			(0x1f<<14)
#define  DRM_MODE_FLAG_3D_NONE			(0<<14)
#define  DRM_MODE_FLAG_3D_FRAME_PACKING		(1<<14)
#define  DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE	(2<<14)
#define  DRM_MODE_FLAG_3D_LINE_ALTERNATIVE	(3<<14)
#define  DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL	(4<<14)
#define  DRM_MODE_FLAG_3D_L_DEPTH		(5<<14)
#define  DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH	(6<<14)
#define  DRM_MODE_FLAG_3D_TOP_AND_BOTTOM	(7<<14)
#define  DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF	(8<<14)


/* DPMS flags */
/* bit compatible with the xorg definitions. */
#define DRM_MODE_DPMS_ON	0
#define DRM_MODE_DPMS_STANDBY	1
#define DRM_MODE_DPMS_SUSPEND	2
#define DRM_MODE_DPMS_OFF	3

/* Scaling mode options */
#define DRM_MODE_SCALE_NONE		0 /* Unmodified timing (display or
					     software can still scale) */
#define DRM_MODE_SCALE_FULLSCREEN	1 /* Full screen, ignore aspect */
#define DRM_MODE_SCALE_CENTER		2 /* Centered, no scaling */
#define DRM_MODE_SCALE_ASPECT		3 /* Full screen, preserve aspect */

/* Picture aspect ratio options */
#define DRM_MODE_PICTURE_ASPECT_NONE	0
#define DRM_MODE_PICTURE_ASPECT_4_3	1
#define DRM_MODE_PICTURE_ASPECT_16_9	2

/* Dithering mode options */
#define DRM_MODE_DITHERING_OFF	0
#define DRM_MODE_DITHERING_ON	1
#define DRM_MODE_DITHERING_AUTO 2

/* Dirty info options */
#define DRM_MODE_DIRTY_OFF      0
#define DRM_MODE_DIRTY_ON       1
#define DRM_MODE_DIRTY_ANNOTATE 2

struct drm_mode_modeinfo {
	__u32 clock;
	__u16 hdisplay, hsync_start, hsync_end, htotal, hskew;
	__u16 vdisplay, vsync_start, vsync_end, vtotal, vscan;

	__u32 vrefresh;

	__u32 flags;
	__u32 type;
	char name[DRM_DISPLAY_MODE_LEN];
};

struct drm_mode_card_res {
	__u64 fb_id_ptr;
	__u64 crtc_id_ptr;
	__u64 connector_id_ptr;
	__u64 encoder_id_ptr;
	__u32 count_fbs;
	__u32 count_crtcs;
	__u32 count_connectors;
	__u32 count_encoders;
	__u32 min_width, max_width;
	__u32 min_height, max_height;
};

struct drm_mode_crtc {
	__u64 set_connectors_ptr;
	__u32 count_connectors;

	__u32 crtc_id; /**< Id */
	__u32 fb_id; /**< Id of framebuffer */

	__u32 x, y; /**< Position on the frameuffer */

	__u32 gamma_size;
	__u32 mode_valid;
	struct drm_mode_modeinfo mode;
};

#define DRM_MODE_PRESENT_TOP_FIELD	(1<<0)
#define DRM_MODE_PRESENT_BOTTOM_FIELD	(1<<1)

/* Planes blend with or override other bits on the CRTC */
struct drm_mode_set_plane {
	__u32 plane_id;
	__u32 crtc_id;
	__u32 fb_id; /* fb object contains surface format type */
	__u32 flags; /* see above flags */

	/* Signed dest location allows it to be partially off screen */
	__s32 crtc_x, crtc_y;
	__u32 crtc_w, crtc_h;

	/* Source values are 16.16 fixed point */
	__u32 src_x, src_y;
	__u32 src_h, src_w;
};

struct drm_mode_get_plane {
	__u32 plane_id;

	__u32 crtc_id;
	__u32 fb_id;

	__u32 possible_crtcs;
	__u32 gamma_size;

	__u32 count_format_types;
	__u64 format_type_ptr;
};

struct drm_mode_get_plane_res {
	__u64 plane_id_ptr;
	__u32 count_planes;
};

#define DRM_MODE_ENCODER_NONE	0
#define DRM_MODE_ENCODER_DAC	1
#define DRM_MODE_ENCODER_TMDS	2
#define DRM_MODE_ENCODER_LVDS	3
#define DRM_MODE_ENCODER_TVDAC	4
#define DRM_MODE_ENCODER_VIRTUAL 5
#define DRM_MODE_ENCODER_DSI	6
#define DRM_MODE_ENCODER_DPMST	7

struct drm_mode_get_encoder {
	__u32 encoder_id;
	__u32 encoder_type;

	__u32 crtc_id; /**< Id of crtc */

	__u32 possible_crtcs;
	__u32 possible_clones;
};

/* This is for connectors with multiple signal types. */
/* Try to match DRM_MODE_CONNECTOR_X as closely as possible. */
#define DRM_MODE_SUBCONNECTOR_Automatic	0
#define DRM_MODE_SUBCONNECTOR_Unknown	0
#define DRM_MODE_SUBCONNECTOR_DVID	3
#define DRM_MODE_SUBCONNECTOR_DVIA	4
#define DRM_MODE_SUBCONNECTOR_Composite	5
#define DRM_MODE_SUBCONNECTOR_SVIDEO	6
#define DRM_MODE_SUBCONNECTOR_Component	8
#define DRM_MODE_SUBCONNECTOR_SCART	9

#define DRM_MODE_CONNECTOR_Unknown	0
#define DRM_MODE_CONNECTOR_VGA		1
#define DRM_MODE_CONNECTOR_DVII		2
#define DRM_MODE_CONNECTOR_DVID		3
#define DRM_MODE_CONNECTOR_DVIA		4
#define DRM_MODE_CONNECTOR_Composite	5
#define DRM_MODE_CONNECTOR_SVIDEO	6
#define DRM_MODE_CONNECTOR_LVDS		7
#define DRM_MODE_CONNECTOR_Component	8
#define DRM_MODE_CONNECTOR_9PinDIN	9
#define DRM_MODE_CONNECTOR_DisplayPort	10
#define DRM_MODE_CONNECTOR_HDMIA	11
#define DRM_MODE_CONNECTOR_HDMIB	12
#define DRM_MODE_CONNECTOR_TV		13
#define DRM_MODE_CONNECTOR_eDP		14
#define DRM_MODE_CONNECTOR_VIRTUAL      15
#define DRM_MODE_CONNECTOR_DSI		16

struct drm_mode_get_connector {

	__u64 encoders_ptr;
	__u64 modes_ptr;
	__u64 props_ptr;
	__u64 prop_values_ptr;

	__u32 count_modes;
	__u32 count_props;
	__u32 count_encoders;

	__u32 encoder_id; /**< Current Encoder */
	__u32 connector_id; /**< Id */
	__u32 connector_type;
	__u32 connector_type_id;

	__u32 connection;
	__u32 mm_width, mm_height; /**< HxW in millimeters */
	__u32 subpixel;

	__u32 pad;
};

#define DRM_MODE_PROP_PENDING	(1<<0)
#define DRM_MODE_PROP_RANGE	(1<<1)
#define DRM_MODE_PROP_IMMUTABLE	(1<<2)
#define DRM_MODE_PROP_ENUM	(1<<3) /* enumerated type with text strings */
#define DRM_MODE_PROP_BLOB	(1<<4)
#define DRM_MODE_PROP_BITMASK	(1<<5) /* bitmask of enumerated types */

/* non-extended types: legacy bitmask, one bit per type: */
#define DRM_MODE_PROP_LEGACY_TYPE  ( \
		DRM_MODE_PROP_RANGE | \
		DRM_MODE_PROP_ENUM | \
		DRM_MODE_PROP_BLOB | \
		DRM_MODE_PROP_BITMASK)

/* extended-types: rather than continue to consume a bit per type,
 * grab a chunk of the bits to use as integer type id.
 */
#define DRM_MODE_PROP_EXTENDED_TYPE	0x0000ffc0
#define DRM_MODE_PROP_TYPE(n)		((n) << 6)
#define DRM_MODE_PROP_OBJECT		DRM_MODE_PROP_TYPE(1)
#define DRM_MODE_PROP_SIGNED_RANGE	DRM_MODE_PROP_TYPE(2)

/* the PROP_ATOMIC flag is used to hide properties from userspace that
 * is not aware of atomic properties.  This is mostly to work around
 * older userspace (DDX drivers) that read/write each prop they find,
 * witout being aware that this could be triggering a lengthy modeset.
 */
#define DRM_MODE_PROP_ATOMIC        0x80000000

struct drm_mode_property_enum {
	__u64 value;
	char name[DRM_PROP_NAME_LEN];
};

struct drm_mode_get_property {
	__u64 values_ptr; /* values and blob lengths */
	__u64 enum_blob_ptr; /* enum and blob id ptrs */

	__u32 prop_id;
	__u32 flags;
	char name[DRM_PROP_NAME_LEN];

	__u32 count_values;
	/* This is only used to count enum values, not blobs. The _blobs is
	 * simply because of a historical reason, i.e. backwards compat. */
	__u32 count_enum_blobs;
};

struct drm_mode_connector_set_property {
	__u64 value;
	__u32 prop_id;
	__u32 connector_id;
};

struct drm_mode_obj_get_properties {
	__u64 props_ptr;
	__u64 prop_values_ptr;
	__u32 count_props;
	__u32 obj_id;
	__u32 obj_type;
};

struct drm_mode_obj_set_property {
	__u64 value;
	__u32 prop_id;
	__u32 obj_id;
	__u32 obj_type;
};

struct drm_mode_get_blob {
	__u32 blob_id;
	__u32 length;
	__u64 data;
};

struct drm_mode_fb_cmd {
	__u32 fb_id;
	__u32 width, height;
	__u32 pitch;
	__u32 bpp;
	__u32 depth;
	/* driver specific handle */
	__u32 handle;
};

#define DRM_MODE_FB_INTERLACED	(1<<0) /* for interlaced framebuffers */
#define DRM_MODE_FB_MODIFIERS	(1<<1) /* enables ->modifer[] */

struct drm_mode_fb_cmd2 {
	__u32 fb_id;
	__u32 width, height;
	__u32 pixel_format; /* fourcc code from drm_fourcc.h */
	__u32 flags; /* see above flags */

	/*
	 * In case of planar formats, this ioctl allows up to 4
	 * buffer objects with offsets and pitches per plane.
	 * The pitch and offset order is dictated by the fourcc,
	 * e.g. NV12 (http://fourcc.org/yuv.php#NV12) is described as:
	 *
	 *   YUV 4:2:0 image with a plane of 8 bit Y samples
	 *   followed by an interleaved U/V plane containing
	 *   8 bit 2x2 subsampled colour difference samples.
	 *
	 * So it would consist of Y as offsets[0] and UV as
	 * offsets[1].  Note that offsets[0] will generally
	 * be 0 (but this is not required).
	 *
	 * To accommodate tiled, compressed, etc formats, a per-plane
	 * modifier can be specified.  The default value of zero
	 * indicates "native" format as specified by the fourcc.
	 * Vendor specific modifier token.  This allows, for example,
	 * different tiling/swizzling pattern on different planes.
	 * See discussion above of DRM_FORMAT_MOD_xxx.
	 */
	__u32 handles[4];
	__u32 pitches[4]; /* pitch for each plane */
	__u32 offsets[4]; /* offset of each plane */
	__u64 modifier[4]; /* ie, tiling, compressed (per plane) */
};

#define DRM_MODE_FB_DIRTY_ANNOTATE_COPY 0x01
#define DRM_MODE_FB_DIRTY_ANNOTATE_FILL 0x02
#define DRM_MODE_FB_DIRTY_FLAGS         0x03

#define DRM_MODE_FB_DIRTY_MAX_CLIPS     256

/*
 * Mark a region of a framebuffer as dirty.
 *
 * Some hardware does not automatically update display contents
 * as a hardware or software draw to a framebuffer. This ioctl
 * allows userspace to tell the kernel and the hardware what
 * regions of the framebuffer have changed.
 *
 * The kernel or hardware is free to update more then just the
 * region specified by the clip rects. The kernel or hardware
 * may also delay and/or coalesce several calls to dirty into a
 * single update.
 *
 * Userspace may annotate the updates, the annotates are a
 * promise made by the caller that the change is either a copy
 * of pixels or a fill of a single color in the region specified.
 *
 * If the DRM_MODE_FB_DIRTY_ANNOTATE_COPY flag is given then
 * the number of updated regions are half of num_clips given,
 * where the clip rects are paired in src and dst. The width and
 * height of each one of the pairs must match.
 *
 * If the DRM_MODE_FB_DIRTY_ANNOTATE_FILL flag is given the caller
 * promises that the region specified of the clip rects is filled
 * completely with a single color as given in the color argument.
 */

struct drm_mode_fb_dirty_cmd {
	__u32 fb_id;
	__u32 flags;
	__u32 color;
	__u32 num_clips;
	__u64 clips_ptr;
};

struct drm_mode_mode_cmd {
	__u32 connector_id;
	struct drm_mode_modeinfo mode;
};

#define DRM_MODE_CURSOR_BO	0x01
#define DRM_MODE_CURSOR_MOVE	0x02
#define DRM_MODE_CURSOR_FLAGS	0x03

/*
 * depending on the value in flags different members are used.
 *
 * CURSOR_BO uses
 *    crtc_id
 *    width
 *    height
 *    handle - if 0 turns the cursor off
 *
 * CURSOR_MOVE uses
 *    crtc_id
 *    x
 *    y
 */
struct drm_mode_cursor {
	__u32 flags;
	__u32 crtc_id;
	__s32 x;
	__s32 y;
	__u32 width;
	__u32 height;
	/* driver specific handle */
	__u32 handle;
};

struct drm_mode_cursor2 {
	__u32 flags;
	__u32 crtc_id;
	__s32 x;
	__s32 y;
	__u32 width;
	__u32 height;
	/* driver specific handle */
	__u32 handle;
	__s32 hot_x;
	__s32 hot_y;
};

struct drm_mode_crtc_lut {
	__u32 crtc_id;
	__u32 gamma_size;

	/* pointers to arrays */
	__u64 red;
	__u64 green;
	__u64 blue;
};

#define DRM_MODE_PAGE_FLIP_EVENT 0x01
#define DRM_MODE_PAGE_FLIP_ASYNC 0x02
#define DRM_MODE_PAGE_FLIP_FLAGS (DRM_MODE_PAGE_FLIP_EVENT|DRM_MODE_PAGE_FLIP_ASYNC)

/*
 * Request a page flip on the specified crtc.
 *
 * This ioctl will ask KMS to schedule a page flip for the specified
 * crtc.  Once any pending rendering targeting the specified fb (as of
 * ioctl time) has completed, the crtc will be reprogrammed to display
 * that fb after the next vertical refresh.  The ioctl returns
 * immediately, but subsequent rendering to the current fb will block
 * in the execbuffer ioctl until the page flip happens.  If a page
 * flip is already pending as the ioctl is called, EBUSY will be
 * returned.
 *
 * Flag DRM_MODE_PAGE_FLIP_EVENT requests that drm sends back a vblank
 * event (see drm.h: struct drm_event_vblank) when the page flip is
 * done.  The user_data field passed in with this ioctl will be
 * returned as the user_data field in the vblank event struct.
 *
 * Flag DRM_MODE_PAGE_FLIP_ASYNC requests that the flip happen
 * 'as soon as possible', meaning that it not delay waiting for vblank.
 * This may cause tearing on the screen.
 *
 * The reserved field must be zero until we figure out something
 * clever to use it for.
 */

struct drm_mode_crtc_page_flip {
	__u32 crtc_id;
	__u32 fb_id;
	__u32 flags;
	__u32 reserved;
	__u64 user_data;
};

/* create a dumb scanout buffer */
struct drm_mode_create_dumb {
	uint32_t height;
	uint32_t width;
	uint32_t bpp;
	uint32_t flags;
	/* handle, pitch, size will be returned */
	uint32_t handle;
	uint32_t pitch;
	uint64_t size;
};

/* set up for mmap of a dumb scanout buffer */
struct drm_mode_map_dumb {
	/** Handle for the object being mapped. */
	__u32 handle;
	__u32 pad;
	/**
	 * Fake offset to use for subsequent mmap call
	 *
	 * This is a fixed-size type for 32/64 compatibility.
	 */
	__u64 offset;
};

struct drm_mode_destroy_dumb {
	uint32_t handle;
};

/* page-flip flags are valid, plus: */
#define DRM_MODE_ATOMIC_TEST_ONLY 0x0100
#define DRM_MODE_ATOMIC_NONBLOCK  0x0200
#define DRM_MODE_ATOMIC_ALLOW_MODESET 0x0400

#define DRM_MODE_ATOMIC_FLAGS (\
		DRM_MODE_PAGE_FLIP_EVENT |\
		DRM_MODE_PAGE_FLIP_ASYNC |\
		DRM_MODE_ATOMIC_TEST_ONLY |\
		DRM_MODE_ATOMIC_NONBLOCK |\
		DRM_MODE_ATOMIC_ALLOW_MODESET)

struct drm_mode_atomic {
	__u32 flags;
	__u32 count_objs;
	__u64 objs_ptr;
	__u64 count_props_ptr;
	__u64 props_ptr;
	__u64 prop_values_ptr;
	__u64 reserved;
	__u64 user_data;
};

#endif
