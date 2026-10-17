/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP_FV___
#define __FSP_FV___

#include <efi.h>

/* Value of EFI_FV_FILE_ATTRIBUTES */
#define EFI_FV_FILE_ATTR_ALIGNMENT	0x0000001F
#define EFI_FV_FILE_ATTR_FIXED		0x00000100
#define EFI_FV_FILE_ATTR_MEMORY_MAPPED	0x00000200

/* Attributes bit definitions */
#define EFI_FVB2_READ_DISABLED_CAP	0x00000001
#define EFI_FVB2_READ_ENABLED_CAP	0x00000002
#define EFI_FVB2_READ_STATUS		0x00000004
#define EFI_FVB2_WRITE_DISABLED_CAP	0x00000008
#define EFI_FVB2_WRITE_ENABLED_CAP	0x00000010
#define EFI_FVB2_WRITE_STATUS		0x00000020
#define EFI_FVB2_LOCK_CAP		0x00000040
#define EFI_FVB2_LOCK_STATUS		0x00000080
#define EFI_FVB2_STICKY_WRITE		0x00000200
#define EFI_FVB2_MEMORY_MAPPED		0x00000400
#define EFI_FVB2_ERASE_POLARITY		0x00000800
#define EFI_FVB2_READ_LOCK_CAP		0x00001000
#define EFI_FVB2_READ_LOCK_STATUS	0x00002000
#define EFI_FVB2_WRITE_LOCK_CAP		0x00004000
#define EFI_FVB2_WRITE_LOCK_STATUS	0x00008000
#define EFI_FVB2_ALIGNMENT		0x001F0000
#define EFI_FVB2_ALIGNMENT_1		0x00000000
#define EFI_FVB2_ALIGNMENT_2		0x00010000
#define EFI_FVB2_ALIGNMENT_4		0x00020000
#define EFI_FVB2_ALIGNMENT_8		0x00030000
#define EFI_FVB2_ALIGNMENT_16		0x00040000
#define EFI_FVB2_ALIGNMENT_32		0x00050000
#define EFI_FVB2_ALIGNMENT_64		0x00060000
#define EFI_FVB2_ALIGNMENT_128		0x00070000
#define EFI_FVB2_ALIGNMENT_256		0x00080000
#define EFI_FVB2_ALIGNMENT_512		0x00090000
#define EFI_FVB2_ALIGNMENT_1K		0x000A0000
#define EFI_FVB2_ALIGNMENT_2K		0x000B0000
#define EFI_FVB2_ALIGNMENT_4K		0x000C0000
#define EFI_FVB2_ALIGNMENT_8K		0x000D0000
#define EFI_FVB2_ALIGNMENT_16K		0x000E0000
#define EFI_FVB2_ALIGNMENT_32K		0x000F0000
#define EFI_FVB2_ALIGNMENT_64K		0x00100000
#define EFI_FVB2_ALIGNMENT_128K		0x00110000
#define EFI_FVB2_ALIGNMENT_256K		0x00120000
#define EFI_FVB2_ALIGNMENT_512K		0x00130000
#define EFI_FVB2_ALIGNMENT_1M		0x00140000
#define EFI_FVB2_ALIGNMENT_2M		0x00150000
#define EFI_FVB2_ALIGNMENT_4M		0x00160000
#define EFI_FVB2_ALIGNMENT_8M		0x00170000
#define EFI_FVB2_ALIGNMENT_16M		0x00180000
#define EFI_FVB2_ALIGNMENT_32M		0x00190000
#define EFI_FVB2_ALIGNMENT_64M		0x001A0000
#define EFI_FVB2_ALIGNMENT_128M		0x001B0000
#define EFI_FVB2_ALIGNMENT_256M		0x001C0000
#define EFI_FVB2_ALIGNMENT_512M		0x001D0000
#define EFI_FVB2_ALIGNMENT_1G		0x001E0000
#define EFI_FVB2_ALIGNMENT_2G		0x001F0000

struct fv_blkmap_entry {
	/* The number of sequential blocks which are of the same size */
	u32	num_blocks;
	/* The size of the blocks */
	u32	length;
};

/* Describes the features and layout of the firmware volume */
struct fv_header {
	/*
	 * The first 16 bytes are reserved to allow for the reset vector of
	 * processors whose reset vector is at address 0.
	 */
	u8			zero_vec[16];
	/*
	 * Declares the file system with which the firmware volume
	 * is formatted.
	 */
	efi_guid_t		fs_guid;
	/*
	 * Length in bytes of the complete firmware volume, including
	 * the header.
	 */
	u64			fv_len;
	/* Set to EFI_FVH_SIGNATURE */
	u32			sign;
	/*
	 * Declares capabilities and power-on defaults for the firmware
	 * volume.
	 */
	u32			attr;
	/* Length in bytes of the complete firmware volume header */
	u16			hdr_len;
	/*
	 * A 16-bit checksum of the firmware volume header.
	 * A valid header sums to zero.
	 */
	u16			checksum;
	/*
	 * Offset, relative to the start of the header, of the extended
	 * header (EFI_FIRMWARE_VOLUME_EXT_HEADER) or zero if there is
	 * no extended header.
	 */
	u16			ext_hdr_off;
	/* This field must always be set to zero */
	u8			reserved[1];
	/*
	 * Set to 2. Future versions of this specification may define new
	 * header fields and will increment the Revision field accordingly.
	 */
	u8			rev;
	/*
	 * An array of run-length encoded FvBlockMapEntry structures.
	 * The array is terminated with an entry of {0,0}.
	 */
	struct fv_blkmap_entry	block_map[1];
};

#define EFI_FVH_SIGNATURE	SIGNATURE_32('_', 'F', 'V', 'H')

/* Firmware Volume Header Revision definition */
#define EFI_FVH_REVISION	0x02

/* Extension header pointed by ExtHeaderOffset of volume header */
struct fv_ext_header {
	/* firmware volume name */
	efi_guid_t		fv_name;
	/* Size of the rest of the extension header including this structure */
	u32			ext_hdr_size;
};

#endif
