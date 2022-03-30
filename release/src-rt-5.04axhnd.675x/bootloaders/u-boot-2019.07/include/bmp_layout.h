/* SPDX-License-Identifier: GPL-2.0+ */
/* (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 */

/************************************************************************/
/* ** Layout of a bmp file						*/
/************************************************************************/

#ifndef _BMP_H_
#define _BMP_H_

struct __packed bmp_color_table_entry {
	__u8	blue;
	__u8	green;
	__u8	red;
	__u8	reserved;
};

/* When accessing these fields, remember that they are stored in little
   endian format, so use linux macros, e.g. le32_to_cpu(width)          */

struct __packed bmp_header {
	/* Header */
	char signature[2];
	__u32	file_size;
	__u32	reserved;
	__u32	data_offset;
	/* InfoHeader */
	__u32	size;
	__u32	width;
	__u32	height;
	__u16	planes;
	__u16	bit_count;
	__u32	compression;
	__u32	image_size;
	__u32	x_pixels_per_m;
	__u32	y_pixels_per_m;
	__u32	colors_used;
	__u32	colors_important;
	/* ColorTable */
};

struct bmp_image {
	struct bmp_header header;
	/* We use a zero sized array just as a placeholder for variable
	   sized array */
	struct bmp_color_table_entry color_table[0];
};

/* Data in the bmp_image is aligned to this length */
#define BMP_DATA_ALIGN	4

/* Constants for the compression field */
#define BMP_BI_RGB	0
#define BMP_BI_RLE8	1
#define BMP_BI_RLE4	2

#endif							/* _BMP_H_ */
