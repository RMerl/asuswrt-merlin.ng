/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP_HOB_H__
#define __FSP_HOB_H__

#include <asm/hob.h>

enum pixel_format {
	pixel_rgbx_8bpc,	/* RGB 8 bit per color */
	pixel_bgrx_8bpc,	/* BGR 8 bit per color */
	pixel_bitmask,
};

struct __packed hob_graphics_info {
	phys_addr_t fb_base;	/* framebuffer base address */
	u32 fb_size;		/* framebuffer size */
	u32 version;
	u32 width;
	u32 height;
	enum pixel_format pixel_format;
	u32 red_mask;
	u32 green_mask;
	u32 blue_mask;
	u32 reserved_mask;
	u32 pixels_per_scanline;
};

/* FSP specific GUID HOB definitions */
#define FSP_GUID_DATA1		0x912740be
#define FSP_GUID_DATA2		0x2284
#define FSP_GUID_DATA3		0x4734
#define FSP_GUID_DATA4_0	0xb9
#define FSP_GUID_DATA4_1	0x71
#define FSP_GUID_DATA4_2	0x84
#define FSP_GUID_DATA4_3	0xb0
#define FSP_GUID_DATA4_4	0x27
#define FSP_GUID_DATA4_5	0x35
#define FSP_GUID_DATA4_6	0x3f
#define FSP_GUID_DATA4_7	0x0c

#define FSP_GUID_BYTE0		0xbe
#define FSP_GUID_BYTE1		0x40
#define FSP_GUID_BYTE2		0x27
#define FSP_GUID_BYTE3		0x91
#define FSP_GUID_BYTE4		0x84
#define FSP_GUID_BYTE5		0x22
#define FSP_GUID_BYTE6		0x34
#define FSP_GUID_BYTE7		0x47
#define FSP_GUID_BYTE8		FSP_GUID_DATA4_0
#define FSP_GUID_BYTE9		FSP_GUID_DATA4_1
#define FSP_GUID_BYTE10		FSP_GUID_DATA4_2
#define FSP_GUID_BYTE11		FSP_GUID_DATA4_3
#define FSP_GUID_BYTE12		FSP_GUID_DATA4_4
#define FSP_GUID_BYTE13		FSP_GUID_DATA4_5
#define FSP_GUID_BYTE14		FSP_GUID_DATA4_6
#define FSP_GUID_BYTE15		FSP_GUID_DATA4_7

#define FSP_HEADER_GUID \
	EFI_GUID(FSP_GUID_DATA1, FSP_GUID_DATA2, FSP_GUID_DATA3, \
		FSP_GUID_DATA4_0, FSP_GUID_DATA4_1, FSP_GUID_DATA4_2, \
		FSP_GUID_DATA4_3, FSP_GUID_DATA4_4, FSP_GUID_DATA4_5, \
		FSP_GUID_DATA4_6, FSP_GUID_DATA4_7)

#define FSP_NON_VOLATILE_STORAGE_HOB_GUID \
	EFI_GUID(0x721acf02, 0x4d77, 0x4c2a, \
		0xb3, 0xdc, 0x27, 0x0b, 0x7b, 0xa9, 0xe4, 0xb0)

#define FSP_VARIABLE_NV_DATA_HOB_GUID \
	EFI_GUID(0xa034147d, 0x690c, 0x4154, \
		0x8d, 0xe6, 0xc0, 0x44, 0x64, 0x1d, 0xe9, 0x42)

#define FSP_BOOTLOADER_TEMP_MEM_HOB_GUID \
	EFI_GUID(0xbbcff46c, 0xc8d3, 0x4113, \
		0x89, 0x85, 0xb9, 0xd4, 0xf3, 0xb3, 0xf6, 0x4e)

#define FSP_HOB_RESOURCE_OWNER_FSP_GUID \
	EFI_GUID(0x69a79759, 0x1373, 0x4367, \
		0xa6, 0xc4, 0xc7, 0xf5, 0x9e, 0xfd, 0x98, 0x6e)

#define FSP_HOB_RESOURCE_OWNER_TSEG_GUID \
	EFI_GUID(0xd038747c, 0xd00c, 0x4980, \
		0xb3, 0x19, 0x49, 0x01, 0x99, 0xa4, 0x7d, 0x55)

#define FSP_HOB_RESOURCE_OWNER_GRAPHICS_GUID \
	EFI_GUID(0x9c7c3aa7, 0x5332, 0x4917, \
		0x82, 0xb9, 0x56, 0xa5, 0xf3, 0xe6, 0x2a, 0x07)

/* The following GUIDs are newly introduced in FSP spec 1.1 */

#define FSP_HOB_RESOURCE_OWNER_BOOTLOADER_TOLUM_GUID \
	EFI_GUID(0x73ff4f56, 0xaa8e, 0x4451, \
		0xb3, 0x16, 0x36, 0x35, 0x36, 0x67, 0xad, 0x44)

#define FSP_GRAPHICS_INFO_HOB_GUID \
	EFI_GUID(0x39f62cce, 0x6825, 0x4669, \
		0xbb, 0x56, 0x54, 0x1a, 0xba, 0x75, 0x3a, 0x07)

/* The following GUIDs are observed with FSP 2.1 / Apollo Lake */
#define FSP_HOB_RESOURCE_OWNER_SMM_PEI_SMRAM_GUID \
	EFI_GUID(0x6dadf1d1, 0xd4cc, 0x4910, \
		0xbb, 0x6e, 0x82, 0xb1, 0xfd, 0x80, 0xff, 0x3d)

#define FSP_HOB_RESOURCE_OWNER_PCD_DATABASE_GUID1 \
	EFI_GUID(0xea296d92, 0x0b69, 0x423c, \
		0x8c, 0x28, 0x33, 0xb4, 0xe0, 0xa9, 0x12, 0x68)

#define FSP_HOB_RESOURCE_OWNER_PCD_DATABASE_GUID2 \
	EFI_GUID(0x9b3ada4f, 0xae56, 0x4c24, \
		0x8d, 0xea, 0xf0, 0x3b, 0x75, 0x58, 0xae, 0x50)

#define FSP_HOB_RESOURCE_OWNER_PEIM_DXE_GUID \
	EFI_GUID(0x86d70125, 0xbaa3, 0x4296, \
		0xa6, 0x2f, 0x60, 0x2b, 0xeb, 0xbb, 0x90, 0x81)

#define FSP_HOB_RESOURCE_OWNER_ALLOC_STACK_GUID \
	EFI_GUID(0x4ed4bf27, 0x4092, 0x42e9, \
		0x80, 0x7d, 0x52, 0x7b, 0x1d, 0x00, 0xc9, 0xbd)

#define FSP_HOB_RESOURCE_OWNER_SMBIOS_MEMORY_GUID \
	EFI_GUID(0x01a1108c, 0x9dee, 0x4984, \
		0x88, 0xc3, 0xee, 0xe8, 0xc4, 0x9e, 0xfb, 0x89)

#endif
