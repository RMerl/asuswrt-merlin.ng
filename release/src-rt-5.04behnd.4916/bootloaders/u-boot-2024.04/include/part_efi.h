/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2008 RuggedCom, Inc.
 * Richard Retanubun <RichardRetanubun@RuggedCom.com>
 */

/*
 * See also linux/fs/partitions/efi.h
 *
 * EFI GUID Partition Table
 * Per Intel EFI Specification v1.02
 * http://developer.intel.com/technology/efi/efi.htm
*/

#include <linux/compiler.h>

#ifndef _DISK_PART_EFI_H
#define _DISK_PART_EFI_H

#include <efi.h>

#define MSDOS_MBR_SIGNATURE 0xAA55
#define MSDOS_MBR_BOOT_CODE_SIZE 440
#define EFI_PMBR_OSTYPE_EFI 0xEF
#define EFI_PMBR_OSTYPE_EFI_GPT 0xEE

#define GPT_HEADER_SIGNATURE_UBOOT 0x5452415020494645ULL // 'EFI PART'
#define GPT_HEADER_CHROMEOS_IGNORE 0x454d45524f4e4749ULL // 'IGNOREME'

#define GPT_HEADER_REVISION_V1 0x00010000
#define GPT_PRIMARY_PARTITION_TABLE_LBA 1ULL
#define GPT_ENTRY_NUMBERS		CONFIG_EFI_PARTITION_ENTRIES_NUMBERS
#define GPT_ENTRY_SIZE			128

#define PARTITION_SYSTEM_GUID \
	EFI_GUID( 0xC12A7328, 0xF81F, 0x11d2, \
		0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B)
#define LEGACY_MBR_PARTITION_GUID \
	EFI_GUID( 0x024DEE41, 0x33E7, 0x11d3, \
		0x9D, 0x69, 0x00, 0x08, 0xC7, 0x81, 0xF3, 0x9F)
#define PARTITION_MSFT_RESERVED_GUID \
	EFI_GUID( 0xE3C9E316, 0x0B5C, 0x4DB8, \
		0x81, 0x7D, 0xF9, 0x2D, 0xF0, 0x02, 0x15, 0xAE)
#define PARTITION_BASIC_DATA_GUID \
	EFI_GUID( 0xEBD0A0A2, 0xB9E5, 0x4433, \
		0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7)
#define PARTITION_LINUX_FILE_SYSTEM_DATA_GUID \
	EFI_GUID(0x0FC63DAF, 0x8483, 0x4772, \
		0x8E, 0x79, 0x3D, 0x69, 0xD8, 0x47, 0x7D, 0xE4)
#define PARTITION_LINUX_RAID_GUID \
	EFI_GUID( 0xa19d880f, 0x05fc, 0x4d3b, \
		0xa0, 0x06, 0x74, 0x3f, 0x0f, 0x84, 0x91, 0x1e)
#define PARTITION_LINUX_SWAP_GUID \
	EFI_GUID( 0x0657fd6d, 0xa4ab, 0x43c4, \
		0x84, 0xe5, 0x09, 0x33, 0xc8, 0x4b, 0x4f, 0x4f)
#define PARTITION_LINUX_LVM_GUID \
	EFI_GUID( 0xe6d6d379, 0xf507, 0x44c2, \
		0xa2, 0x3c, 0x23, 0x8f, 0x2a, 0x3d, 0xf9, 0x28)
#define PARTITION_U_BOOT_ENVIRONMENT \
	EFI_GUID( 0x3de21764, 0x95bd, 0x54bd, \
		0xa5, 0xc3, 0x4a, 0xbe, 0x78, 0x6f, 0x38, 0xa8)

/* Special ChromiumOS things */
#define PARTITION_CROS_KERNEL \
	EFI_GUID(0xfe3a2a5d, 0x4f32, 0x41a7, \
		 0xb7, 0x25, 0xac, 0xcc, 0x32, 0x85, 0xa3, 0x09)
#define PARTITION_CROS_ROOT \
	EFI_GUID(0x3cb8e202, 0x3b7e, 0x47dd, \
		 0x8a, 0x3c, 0x7f, 0xf2, 0xa1, 0x3c, 0xfc, 0xec)
#define PARTITION_CROS_FIRMWARE \
	EFI_GUID(0xcab6e88e, 0xabf3, 0x4102, \
		 0xa0, 0x7a, 0xd4, 0xbb, 0x9b, 0xe3, 0xc1, 0xd3)
#define PARTITION_CROS_RESERVED \
	EFI_GUID(0x2e0a753d, 0x9e48, 0x43b0, \
		 0x83, 0x37, 0xb1, 0x51, 0x92, 0xcb, 0x1b, 0x5e)

/* linux/include/efi.h */
typedef u16 efi_char16_t;

/* based on linux/include/genhd.h */
struct partition {
	u8 boot_ind;		/* 0x80 - active */
	u8 head;		/* starting head */
	u8 sector;		/* starting sector */
	u8 cyl;			/* starting cylinder */
	u8 sys_ind;		/* What partition type */
	u8 end_head;		/* end head */
	u8 end_sector;		/* end sector */
	u8 end_cyl;		/* end cylinder */
	__le32 start_sect;	/* starting sector counting from 0 */
	__le32 nr_sects;	/* nr of sectors in partition */
} __packed;

/* based on linux/fs/partitions/efi.h */
typedef struct _gpt_header {
	__le64 signature;
	__le32 revision;
	__le32 header_size;
	__le32 header_crc32;
	__le32 reserved1;
	__le64 my_lba;
	__le64 alternate_lba;
	__le64 first_usable_lba;
	__le64 last_usable_lba;
	efi_guid_t disk_guid;
	__le64 partition_entry_lba;
	__le32 num_partition_entries;
	__le32 sizeof_partition_entry;
	__le32 partition_entry_array_crc32;
} __packed gpt_header;

typedef union _gpt_entry_attributes {
	struct {
		u64 required_to_function:1;
		u64 no_block_io_protocol:1;
		u64 legacy_bios_bootable:1;
		u64 reserved:45;
		u64 type_guid_specific:16;
	} fields;
	unsigned long long raw;
} __packed gpt_entry_attributes;

#define PARTNAME_SZ	(72 / sizeof(efi_char16_t))
typedef struct _gpt_entry {
	efi_guid_t partition_type_guid;
	efi_guid_t unique_partition_guid;
	__le64 starting_lba;
	__le64 ending_lba;
	gpt_entry_attributes attributes;
	efi_char16_t partition_name[PARTNAME_SZ];
} __packed gpt_entry;

typedef struct _legacy_mbr {
	u8 boot_code[MSDOS_MBR_BOOT_CODE_SIZE];
	__le32 unique_mbr_signature;
	__le16 unknown;
	struct partition partition_record[4];
	__le16 signature;
} __packed legacy_mbr;

#endif	/* _DISK_PART_EFI_H */
