/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP_FFS_H__
#define __FSP_FFS_H__

/* Used to verify the integrity of the file */
union __packed ffs_integrity {
	struct {
		/*
		 * The IntegrityCheck.checksum.header field is an 8-bit
		 * checksum of the file header. The State and
		 * IntegrityCheck.checksum.file fields are assumed to be zero
		 * and the checksum is calculated such that the entire header
		 * sums to zero.
		 */
		u8	header;
		/*
		 * If the FFS_ATTRIB_CHECKSUM (see definition below) bit of
		 * the Attributes field is set to one, the
		 * IntegrityCheck.checksum.file field is an 8-bit checksum of
		 * the file data. If the FFS_ATTRIB_CHECKSUM bit of the
		 * Attributes field is cleared to zero, the
		 * IntegrityCheck.checksum.file field must be initialized with
		 * a value of 0xAA. The IntegrityCheck.checksum.file field is
		 * valid any time the EFI_FILE_DATA_VALID bit is set in the
		 * State field.
		 */
		u8	file;
	} checksum;

	/* This is the full 16 bits of the IntegrityCheck field */
	u16	checksum16;
};

/*
 * Each file begins with the header that describe the
 * contents and state of the files.
 */
struct __packed ffs_file_header {
	/*
	 * This GUID is the file name.
	 * It is used to uniquely identify the file.
	 */
	struct efi_guid		name;
	/* Used to verify the integrity of the file */
	union ffs_integrity	integrity;
	/* Identifies the type of file */
	u8			type;
	/* Declares various file attribute bits */
	u8			attr;
	/* The length of the file in bytes, including the FFS header */
	u8			size[3];
	/*
	 * Used to track the state of the file throughout the life of
	 * the file from creation to deletion.
	 */
	u8			state;
};

struct __packed ffs_file_header2 {
	/*
	 * This GUID is the file name. It is used to uniquely identify the file.
	 * There may be only one instance of a file with the file name GUID of
	 * Name in any given firmware volume, except if the file type is
	 * EFI_FV_FILE_TYPE_FFS_PAD.
	 */
	struct efi_guid		name;
	/* Used to verify the integrity of the file */
	union ffs_integrity	integrity;
	/* Identifies the type of file */
	u8			type;
	/* Declares various file attribute bits */
	u8			attr;
	/*
	 * The length of the file in bytes, including the FFS header.
	 * The length of the file data is either
	 * (size - sizeof(struct ffs_file_header)). This calculation means a
	 * zero-length file has a size of 24 bytes, which is
	 * sizeof(struct ffs_file_header). Size is not required to be a
	 * multiple of 8 bytes. Given a file F, the next file header is located
	 * at the next 8-byte aligned firmware volume offset following the last
	 * byte of the file F.
	 */
	u8			size[3];
	/*
	 * Used to track the state of the file throughout the life of
	 * the file from creation to deletion.
	 */
	u8			state;
	/*
	 * If FFS_ATTRIB_LARGE_FILE is set in attr, then ext_size exists
	 * and size must be set to zero.
	 * If FFS_ATTRIB_LARGE_FILE is not set then
	 * struct ffs_file_header is used.
	 */
	u32			ext_size;
};

/*
 * Pseudo type. It is used as a wild card when retrieving sections.
 * The section type EFI_SECTION_ALL matches all section types.
 */
#define EFI_SECTION_ALL				0x00

/* Encapsulation section Type values */
#define EFI_SECTION_COMPRESSION			0x01
#define EFI_SECTION_GUID_DEFINED		0x02
#define EFI_SECTION_DISPOSABLE			0x03

/* Leaf section Type values */
#define EFI_SECTION_PE32			0x10
#define EFI_SECTION_PIC				0x11
#define EFI_SECTION_TE				0x12
#define EFI_SECTION_DXE_DEPEX			0x13
#define EFI_SECTION_VERSION			0x14
#define EFI_SECTION_USER_INTERFACE		0x15
#define EFI_SECTION_COMPATIBILITY16		0x16
#define EFI_SECTION_FIRMWARE_VOLUME_IMAGE	0x17
#define EFI_SECTION_FREEFORM_SUBTYPE_GUID	0x18
#define EFI_SECTION_RAW				0x19
#define EFI_SECTION_PEI_DEPEX			0x1B
#define EFI_SECTION_SMM_DEPEX			0x1C

/* Common section header */
struct __packed raw_section {
	/*
	 * A 24-bit unsigned integer that contains the total size of
	 * the section in bytes, including the EFI_COMMON_SECTION_HEADER.
	 */
	u8	size[3];
	u8	type;
};

struct __packed raw_section2 {
	/*
	 * A 24-bit unsigned integer that contains the total size of
	 * the section in bytes, including the EFI_COMMON_SECTION_HEADER.
	 */
	u8	size[3];
	u8	type;
	/*
	 * If size is 0xFFFFFF, then ext_size contains the size of
	 * the section. If size is not equal to 0xFFFFFF, then this
	 * field does not exist.
	 */
	u32	ext_size;
};

#endif
