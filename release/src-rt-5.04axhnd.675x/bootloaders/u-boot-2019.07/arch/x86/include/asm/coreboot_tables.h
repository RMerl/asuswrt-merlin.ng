/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * This file is part of the libpayload project.
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 */

#ifndef _COREBOOT_TABLES_H
#define _COREBOOT_TABLES_H

struct cbuint64 {
	u32 lo;
	u32 hi;
};

struct cb_header {
	u8 signature[4];
	u32 header_bytes;
	u32 header_checksum;
	u32 table_bytes;
	u32 table_checksum;
	u32 table_entries;
};

struct cb_record {
	u32 tag;
	u32 size;
};

#define CB_TAG_UNUSED			0x0000
#define CB_TAG_MEMORY			0x0001

struct cb_memory_range {
	struct cbuint64 start;
	struct cbuint64 size;
	u32 type;
};

#define CB_MEM_RAM			1
#define CB_MEM_RESERVED			2
#define CB_MEM_ACPI			3
#define CB_MEM_NVS			4
#define CB_MEM_UNUSABLE			5
#define CB_MEM_VENDOR_RSVD		6
#define CB_MEM_TABLE			16

struct cb_memory {
	u32 tag;
	u32 size;
	struct cb_memory_range map[0];
};

#define CB_TAG_HWRPB			0x0002

struct cb_hwrpb {
	u32 tag;
	u32 size;
	u64 hwrpb;
};

#define CB_TAG_MAINBOARD		0x0003

struct cb_mainboard {
	u32 tag;
	u32 size;
	u8 vendor_idx;
	u8 part_number_idx;
	u8 strings[0];
};

#define CB_TAG_VERSION			0x0004
#define CB_TAG_EXTRA_VERSION		0x0005
#define CB_TAG_BUILD			0x0006
#define CB_TAG_COMPILE_TIME		0x0007
#define CB_TAG_COMPILE_BY		0x0008
#define CB_TAG_COMPILE_HOST		0x0009
#define CB_TAG_COMPILE_DOMAIN		0x000a
#define CB_TAG_COMPILER			0x000b
#define CB_TAG_LINKER			0x000c
#define CB_TAG_ASSEMBLER		0x000d

struct cb_string {
	u32 tag;
	u32 size;
	u8 string[0];
};

#define CB_TAG_SERIAL			0x000f

struct cb_serial {
	u32 tag;
	u32 size;
#define CB_SERIAL_TYPE_IO_MAPPED	1
#define CB_SERIAL_TYPE_MEMORY_MAPPED	2
	u32 type;
	u32 baseaddr;
	u32 baud;
};

#define CB_TAG_CONSOLE			0x0010

struct cb_console {
	u32 tag;
	u32 size;
	u16 type;
};

#define CB_TAG_CONSOLE_SERIAL8250	0
#define CB_TAG_CONSOLE_VGA		1 /* OBSOLETE */
#define CB_TAG_CONSOLE_BTEXT		2 /* OBSOLETE */
#define CB_TAG_CONSOLE_LOGBUF		3
#define CB_TAG_CONSOLE_SROM		4 /* OBSOLETE */
#define CB_TAG_CONSOLE_EHCI		5

#define CB_TAG_FORWARD			0x0011

struct cb_forward {
	u32 tag;
	u32 size;
	u64 forward;
};

#define CB_TAG_FRAMEBUFFER		0x0012

struct cb_framebuffer {
	u32 tag;
	u32 size;
	u64 physical_address;
	u32 x_resolution;
	u32 y_resolution;
	u32 bytes_per_line;
	u8 bits_per_pixel;
	u8 red_mask_pos;
	u8 red_mask_size;
	u8 green_mask_pos;
	u8 green_mask_size;
	u8 blue_mask_pos;
	u8 blue_mask_size;
	u8 reserved_mask_pos;
	u8 reserved_mask_size;
};

#define CB_TAG_GPIO			0x0013
#define GPIO_MAX_NAME_LENGTH		16

struct cb_gpio {
	u32 port;
	u32 polarity;
	u32 value;
	u8 name[GPIO_MAX_NAME_LENGTH];
};

struct cb_gpios {
	u32 tag;
	u32 size;
	u32 count;
	struct cb_gpio gpios[0];
};

#define CB_TAG_FDT			0x0014

struct cb_fdt {
	uint32_t tag;
	uint32_t size;	/* size of the entire entry */
	/* the actual FDT gets placed here */
};

#define CB_TAG_VDAT			0x0015

struct cb_vdat {
	uint32_t tag;
	uint32_t size;	/* size of the entire entry */
	void *vdat_addr;
	uint32_t vdat_size;
};

#define CB_TAG_TIMESTAMPS		0x0016
#define CB_TAG_CBMEM_CONSOLE		0x0017
#define CB_TAG_MRC_CACHE		0x0018

struct cb_cbmem_tab {
	uint32_t tag;
	uint32_t size;
	void *cbmem_tab;
};

#define CB_TAG_VBNV			0x0019

struct cb_vbnv {
	uint32_t tag;
	uint32_t size;
	uint32_t vbnv_start;
	uint32_t vbnv_size;
};

#define CB_TAG_CMOS_OPTION_TABLE	0x00c8

struct cb_cmos_option_table {
	u32 tag;
	u32 size;
	u32 header_length;
};

#define CB_TAG_OPTION			0x00c9

#define CMOS_MAX_NAME_LENGTH		32

struct cb_cmos_entries {
	u32 tag;
	u32 size;
	u32 bit;
	u32 length;
	u32 config;
	u32 config_id;
	u8 name[CMOS_MAX_NAME_LENGTH];
};

#define CB_TAG_OPTION_ENUM		0x00ca
#define CMOS_MAX_TEXT_LENGTH		32

struct cb_cmos_enums {
	u32 tag;
	u32 size;
	u32 config_id;
	u32 value;
	u8 text[CMOS_MAX_TEXT_LENGTH];
};

#define CB_TAG_OPTION_DEFAULTS		0x00cb
#define CMOS_IMAGE_BUFFER_SIZE		128

struct cb_cmos_defaults {
	u32 tag;
	u32 size;
	u32 name_length;
	u8 name[CMOS_MAX_NAME_LENGTH];
	u8 default_set[CMOS_IMAGE_BUFFER_SIZE];
};

#define CB_TAG_OPTION_CHECKSUM		0x00cc
#define CHECKSUM_NONE			0
#define CHECKSUM_PCBIOS			1

struct	cb_cmos_checksum {
	u32 tag;
	u32 size;
	u32 range_start;
	u32 range_end;
	u32 location;
	u32 type;
};

/* Helpful macros */

#define MEM_RANGE_COUNT(_rec) \
	(((_rec)->size - sizeof(*(_rec))) / sizeof((_rec)->map[0]))

#define MEM_RANGE_PTR(_rec, _idx) \
	(((u8 *) (_rec)) + sizeof(*(_rec)) \
	+ (sizeof((_rec)->map[0]) * (_idx)))

#define MB_VENDOR_STRING(_mb) \
	(((unsigned char *) ((_mb)->strings)) + (_mb)->vendor_idx)

#define MB_PART_STRING(_mb) \
	(((unsigned char *) ((_mb)->strings)) + (_mb)->part_number_idx)

#define UNPACK_CB64(_in) \
	((((u64) _in.hi) << 32) | _in.lo)

#define CBMEM_TOC_RESERVED		512
#define MAX_CBMEM_ENTRIES		16
#define CBMEM_MAGIC			0x434f5245

struct cbmem_entry {
	u32 magic;
	u32 id;
	u64 base;
	u64 size;
} __packed;

#define CBMEM_ID_FREESPACE		0x46524545
#define CBMEM_ID_GDT			0x4c474454
#define CBMEM_ID_ACPI			0x41435049
#define CBMEM_ID_CBTABLE		0x43425442
#define CBMEM_ID_PIRQ			0x49525154
#define CBMEM_ID_MPTABLE		0x534d5054
#define CBMEM_ID_RESUME			0x5245534d
#define CBMEM_ID_RESUME_SCRATCH		0x52455343
#define CBMEM_ID_SMBIOS			0x534d4254
#define CBMEM_ID_TIMESTAMP		0x54494d45
#define CBMEM_ID_MRCDATA		0x4d524344
#define CBMEM_ID_CONSOLE		0x434f4e53
#define CBMEM_ID_NONE			0x00000000

/**
 * high_table_reserve() - reserve configuration table in high memory
 *
 * This reserves configuration table in high memory.
 *
 * @return:	always 0
 */
int high_table_reserve(void);

/**
 * high_table_malloc() - allocate configuration table in high memory
 *
 * This allocates configuration table in high memory.
 *
 * @bytes:	size of configuration table to be allocated
 * @return:	pointer to configuration table in high memory
 */
void *high_table_malloc(size_t bytes);

/**
 * write_coreboot_table() - write coreboot table
 *
 * This writes coreboot table at a given address.
 *
 * @addr:	start address to write coreboot table
 * @cfg_tables:	pointer to configuration table memory area
 */
void write_coreboot_table(u32 addr, struct memory_area *cfg_tables);

#endif
