/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _FSP_HEADER_H_
#define _FSP_HEADER_H_

#define FSP_HEADER_OFF	0x94	/* Fixed FSP header offset in the FSP image */

struct __packed fsp_header {
	u32	sign;			/* 'FSPH' */
	u32	hdr_len;		/* header length */
	u8	reserved1[3];
	u8	hdr_rev;		/* header rev */
	u32	img_rev;		/* image rev */
	char	img_id[8];		/* signature string */
	u32	img_size;		/* image size */
	u32	img_base;		/* image base */
	u32	img_attr;		/* image attribute */
	u32	cfg_region_off;		/* configuration region offset */
	u32	cfg_region_size;	/* configuration region size */
	u32	api_num;		/* number of API entries */
	u32	fsp_tempram_init;	/* tempram_init offset */
	u32	fsp_init;		/* fsp_init offset */
	u32	fsp_notify;		/* fsp_notify offset */
	u32	fsp_mem_init;		/* fsp_mem_init offset */
	u32	fsp_tempram_exit;	/* fsp_tempram_exit offset */
	u32	fsp_silicon_init;	/* fsp_silicon_init offset */
};

#define FSP_HEADER_REVISION_1		1
#define FSP_HEADER_REVISION_2		2

enum fsp_type {
	FSP_ATTR_COMP_TYPE_FSP_T	= 1,
	FSP_ATTR_COMP_TYPE_FSP_M	= 2,
	FSP_ATTR_COMP_TYPE_FSP_S	= 3,
};

enum {
	FSP_ATTR_GRAPHICS_SUPPORT	= 1 << 0,
	FSP_ATTR_COMP_TYPE_SHIFT	= 28,
	FSP_ATTR_COMP_TYPE_MASK		= 0xfU << FSP_ATTR_COMP_TYPE_SHIFT,

};

#define EFI_FSPH_SIGNATURE		SIGNATURE_32('F', 'S', 'P', 'H')

#endif
