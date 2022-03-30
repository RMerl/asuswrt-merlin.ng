/* SPDX-License-Identifier: BSD-2-Clause */
/******************************************************************************
 * Copyright (c) 2004, 2008 IBM Corporation
 * Copyright (c) 2009 Pattrick Hueper <phueper@hueper.net>
 * All rights reserved.
 *
 * Contributors:
 *     IBM Corporation - initial implementation
 *****************************************************************************/
#ifndef _VBE_H
#define _VBE_H

/* these structs are for input from and output to OF */
struct __packed vbe_screen_info {
	u8 display_type;	/* 0=NONE, 1= analog, 2=digital */
	u16 screen_width;
	u16 screen_height;
	/* bytes per line in framebuffer, may be more than screen_width */
	u16 screen_linebytes;
	u8 color_depth;	/* color depth in bits per pixel */
	u32 framebuffer_address;
	u8 edid_block_zero[128];
};

struct __packed vbe_screen_info_input {
	u8 signature[4];
	u16 size_reserved;
	u8 monitor_number;
	u16 max_screen_width;
	u8 color_depth;
};

/* these structs only store the required a subset of the VBE-defined fields */
struct __packed vbe_info {
	char signature[4];
	u16 version;
	u32 oem_string_ptr;
	u32 capabilities;
	u32 modes_ptr;
	u16 total_memory;
	u16 oem_version;
	u32 vendor_name_ptr;
	u32 product_name_ptr;
	u32 product_rev_ptr;
};

struct __packed vesa_mode_info {
	u16 mode_attributes;	/* 00 */
	u8 win_a_attributes;	/* 02 */
	u8 win_b_attributes;	/* 03 */
	u16 win_granularity;	/* 04 */
	u16 win_size;		/* 06 */
	u16 win_a_segment;	/* 08 */
	u16 win_b_segment;	/* 0a */
	u32 win_func_ptr;	/* 0c */
	u16 bytes_per_scanline;	/* 10 */
	u16 x_resolution;	/* 12 */
	u16 y_resolution;	/* 14 */
	u8 x_charsize;		/* 16 */
	u8 y_charsize;		/* 17 */
	u8 number_of_planes;	/* 18 */
	u8 bits_per_pixel;	/* 19 */
	u8 number_of_banks;	/* 20 */
	u8 memory_model;	/* 21 */
	u8 bank_size;		/* 22 */
	u8 number_of_image_pages; /* 23 */
	u8 reserved_page;
	u8 red_mask_size;
	u8 red_mask_pos;
	u8 green_mask_size;
	u8 green_mask_pos;
	u8 blue_mask_size;
	u8 blue_mask_pos;
	u8 reserved_mask_size;
	u8 reserved_mask_pos;
	u8 direct_color_mode_info;
	u32 phys_base_ptr;
	u32 offscreen_mem_offset;
	u16 offscreen_mem_size;
	u8 reserved[206];
};

struct vbe_mode_info {
	u16 video_mode;
	bool valid;
	union {
		struct vesa_mode_info vesa;
		u8 mode_info_block[256];
	};
};

struct vbe_ddc_info {
	u8 port_number;	/* i.e. monitor number */
	u8 edid_transfer_time;
	u8 ddc_level;
	u8 edid_block_zero[128];
};

#define VESA_GET_INFO		0x4f00
#define VESA_GET_MODE_INFO	0x4f01
#define VESA_SET_MODE		0x4f02
#define VESA_GET_CUR_MODE	0x4f03

extern struct vbe_mode_info mode_info;

struct video_priv;
struct video_uc_platdata;
int vbe_setup_video_priv(struct vesa_mode_info *vesa,
			 struct video_priv *uc_priv,
			 struct video_uc_platdata *plat);
int vbe_setup_video(struct udevice *dev, int (*int15_handler)(void));

#endif
