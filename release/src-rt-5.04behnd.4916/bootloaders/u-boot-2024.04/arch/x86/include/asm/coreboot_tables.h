/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * This file is part of the libpayload project.
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 */

#ifndef _COREBOOT_TABLES_H
#define _COREBOOT_TABLES_H

struct timestamp_entry {
	u32	entry_id;
	u64	entry_stamp;
} __packed;

struct timestamp_table {
	u64	base_time;
	u16	max_entries;
	u16	tick_freq_mhz;
	u32	num_entries;
	struct timestamp_entry entries[0]; /* Variable number of entries */
} __packed;

enum timestamp_id {
	/* coreboot-specific timestamp IDs */
	TS_START_ROMSTAGE = 1,
	TS_BEFORE_INITRAM = 2,
	TS_AFTER_INITRAM = 3,
	TS_END_ROMSTAGE = 4,
	TS_START_VBOOT = 5,
	TS_END_VBOOT = 6,
	TS_START_COPYRAM = 8,
	TS_END_COPYRAM = 9,
	TS_START_RAMSTAGE = 10,
	TS_START_BOOTBLOCK = 11,
	TS_END_BOOTBLOCK = 12,
	TS_START_COPYROM = 13,
	TS_END_COPYROM = 14,
	TS_START_ULZMA = 15,
	TS_END_ULZMA = 16,
	TS_START_ULZ4F = 17,
	TS_END_ULZ4F = 18,
	TS_DEVICE_ENUMERATE = 30,
	TS_DEVICE_CONFIGURE = 40,
	TS_DEVICE_ENABLE = 50,
	TS_DEVICE_INITIALIZE = 60,
	TS_DEVICE_DONE = 70,
	TS_CBMEM_POST = 75,
	TS_WRITE_TABLES = 80,
	TS_FINALIZE_CHIPS = 85,
	TS_LOAD_PAYLOAD = 90,
	TS_ACPI_WAKE_JUMP = 98,
	TS_SELFBOOT_JUMP = 99,

	/* 500+ reserved for vendorcode extensions (500-600: google/chromeos) */
	TS_START_COPYVER = 501,
	TS_END_COPYVER = 502,
	TS_START_TPMINIT = 503,
	TS_END_TPMINIT = 504,
	TS_START_VERIFY_SLOT = 505,
	TS_END_VERIFY_SLOT = 506,
	TS_START_HASH_BODY = 507,
	TS_DONE_LOADING = 508,
	TS_DONE_HASHING = 509,
	TS_END_HASH_BODY = 510,
	TS_START_COPYVPD = 550,
	TS_END_COPYVPD_RO = 551,
	TS_END_COPYVPD_RW = 552,

	/* 940-950 reserved for vendorcode extensions (940-950: Intel ME) */
	TS_ME_INFORM_DRAM_WAIT = 940,
	TS_ME_INFORM_DRAM_DONE = 941,

	/* 950+ reserved for vendorcode extensions (950-999: intel/fsp) */
	TS_FSP_MEMORY_INIT_START = 950,
	TS_FSP_MEMORY_INIT_END = 951,
	TS_FSP_TEMP_RAM_EXIT_START = 952,
	TS_FSP_TEMP_RAM_EXIT_END = 953,
	TS_FSP_SILICON_INIT_START = 954,
	TS_FSP_SILICON_INIT_END = 955,
	TS_FSP_BEFORE_ENUMERATE = 956,
	TS_FSP_AFTER_ENUMERATE = 957,
	TS_FSP_BEFORE_FINALIZE = 958,
	TS_FSP_AFTER_FINALIZE = 959,
	TS_FSP_BEFORE_END_OF_FIRMWARE = 960,
	TS_FSP_AFTER_END_OF_FIRMWARE = 961,

	/* 1000+ reserved for payloads (1000-1200: ChromeOS depthcharge) */

	/* U-Boot entry IDs start at 1000 */
	TS_U_BOOT_INITTED = 1000, /* This is where U-Boot starts */

	TS_RO_PARAMS_INIT = 1001,
	TS_RO_VB_INIT = 1002,
	TS_RO_VB_SELECT_FIRMWARE = 1003,
	TS_RO_VB_SELECT_AND_LOAD_KERNEL = 1004,

	TS_RW_VB_SELECT_AND_LOAD_KERNEL = 1010,

	TS_VB_SELECT_AND_LOAD_KERNEL = 1020,
	TS_VB_EC_VBOOT_DONE = 1030,
	TS_VB_STORAGE_INIT_DONE = 1040,
	TS_VB_READ_KERNEL_DONE = 1050,
	TS_VB_VBOOT_DONE = 1100,

	TS_START_KERNEL = 1101,
	TS_KERNEL_DECOMPRESSION = 1102,
	TS_U_BOOT_START_KERNEL = 1100, /* Right before jumping to kernel */
};

struct memory_area;

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
	u32 regwidth;

	/*
	 * Crystal or input frequency to the chip containing the UART.
	 * Provide the board specific details to allow the payload to
	 * initialize the chip containing the UART and make independent
	 * decisions as to which dividers to select and their values
	 * to eventually arrive at the desired console baud-rate.
	 */
	u32 input_hertz;

	/*
	 * UART PCI address: bus, device, function
	 * 1 << 31 - Valid bit, PCI UART in use
	 * Bus << 20
	 * Device << 15
	 * Function << 12
	 */
	u32 uart_pci_addr;
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
#define CB_GPIO_ACTIVE_LOW		0
#define CB_GPIO_ACTIVE_HIGH		1
#define CB_GPIO_MAX_NAME_LENGTH		16
struct cb_gpio {
	u32 port;
	u32 polarity;
	u32 value;
	u8 name[CB_GPIO_MAX_NAME_LENGTH];
};

struct cb_gpios {
	u32 tag;
	u32 size;
	u32 count;
	struct cb_gpio gpios[0];
};

#define CB_TAG_FDT			0x0014

struct cb_fdt {
	u32 tag;
	u32 size;	/* size of the entire entry */
	/* the actual FDT gets placed here */
};

#define CB_TAG_VDAT			0x0015

struct cb_vdat {
	u32 tag;
	u32 size;	/* size of the entire entry */
	void *vdat_addr;
	u32 vdat_size;
};

#define CB_TAG_TIMESTAMPS		0x0016
#define CB_TAG_CBMEM_CONSOLE		0x0017

#define CBMC_CURSOR_MASK	((1 << 28) - 1)
#define CBMC_OVERFLOW		BIT(31)

/*
 * struct cbmem_console - In-memory console buffer for coreboot
 *
 * Structure describing console buffer. It is overlaid on a flat memory area,
 * with body covering the extent of the memory. Once the buffer is full,
 * output will wrap back around to the start of the buffer. The high bit of the
 * cursor field gets set to indicate that this happened. If the underlying
 * storage allows this, the buffer will persist across multiple boots and append
 * to the previous log.
 */
struct cbmem_console {
	u32 size;
	u32 cursor;
	u8  body[0];
};

#define CB_TAG_MRC_CACHE		0x0018

struct cb_cbmem_tab {
	u32 tag;
	u32 size;
	u64 cbmem_tab;
};

#define CB_TAG_VBNV			0x0019

struct cb_vbnv {
	u32 tag;
	u32 size;
	u32 vbnv_start;
	u32 vbnv_size;
};

#define CB_TAG_VBOOT_HANDOFF		0x0020

#define CB_TAG_X86_ROM_MTRR		0x0021
struct cb_x86_rom_mtrr {
	u32 tag;
	u32 size;
	/*
	 * The variable range MTRR index covering the ROM. If one wants to
	 * enable caching the ROM, the variable MTRR needs to be set to
	 * write-protect. To disable the caching after enabling set the
	 * type to uncacheable
	 */
	u32 index;
};

#define CB_TAG_DMA			0x0022
#define CB_TAG_RAM_OOPS			0x0023
#define CB_TAG_ACPI_GNVS		0x0024

#define CB_TAG_BOARD_ID			0x0025
struct cb_board_id {
	u32 tag;
	u32 size;
	/* Board ID as retrieved from the board revision GPIOs. */
	u32 board_id;
};

#define CB_TAG_MAC_ADDRS		0x0026
struct mac_address {
	u8 mac_addr[6];
	u8 pad[2];         /* Pad it to 8 bytes to keep it simple. */
};

struct cb_macs {
	u32 tag;
	u32 size;
	u32 count;
	struct mac_address mac_addrs[0];
};

#define CB_TAG_WIFI_CALIBRATION		0x0027

#define CB_TAG_RAM_CODE			0x0028
struct cb_ram_code {
	u32 tag;
	u32 size;
	u32 ram_code;
};

#define CB_TAG_SPI_FLASH		0x0029
struct cb_spi_flash {
	u32 tag;
	u32 size;
	u32 flash_size;
	u32 sector_size;
	u32 erase_cmd;
};

#define CB_TAG_MTC			0x002b
#define CB_TAG_VPD			0x002c
struct lb_range {
	u32 tag;
	u32 size;
	u64 range_start;
	u32 range_size;
};

#define CB_TAG_BOOT_MEDIA_PARAMS	0x0030
struct cb_boot_media_params {
	u32 tag;
	u32 size;
	/* offsets are relative to start of boot media */
	u64 fmap_offset;
	u64 cbfs_offset;
	u64 cbfs_size;
	u64 boot_media_size;
};

#define CB_TAG_CBMEM_ENTRY		0x0031
#define CBMEM_ID_SMBIOS			0x534d4254

struct cb_cbmem_entry {
	u32 tag;
	u32 size;
	u64 address;
	u32 entry_size;
	u32 id;
};

#define CB_TAG_TSC_INFO			0x0032
struct cb_tsc_info {
	u32 tag;
	u32 size;

	u32 freq_khz;
};

#define CB_TAG_SERIALNO			0x002a
#define CB_MAX_SERIALNO_LENGTH		32

#define CB_TAG_ACPI_RSDP		0x0043

#define CB_TAG_CMOS_OPTION_TABLE	0x00c8

struct cb_cmos_option_table {
	u32 tag;
	u32 size;
	u32 header_length;
	/* entries follow after this header */
};

#define CB_TAG_OPTION			0x00c9

#define CB_CMOS_MAX_NAME_LENGTH		32

struct cb_cmos_entries {
	u32 tag;
	u32 size;
	u32 bit;
	u32 length;
	u32 config;
	u32 config_id;
	u8 name[CB_CMOS_MAX_NAME_LENGTH];
};

#define CB_TAG_OPTION_ENUM		0x00ca
#define CB_CMOS_MAX_TEXT_LENGTH		32
struct cb_cmos_enums {
	u32 tag;
	u32 size;
	u32 config_id;
	u32 value;
	u8 text[CB_CMOS_MAX_TEXT_LENGTH];
};

#define CB_TAG_OPTION_DEFAULTS		0x00cb
#define CB_CMOS_IMAGE_BUFFER_SIZE	128

struct cb_cmos_defaults {
	u32 tag;
	u32 size;
	u32 name_length;
	u8 name[CB_CMOS_MAX_NAME_LENGTH];
	u8 default_set[CB_CMOS_IMAGE_BUFFER_SIZE];
};

#define CB_TAG_OPTION_CHECKSUM		0x00cc
#define CB_CHECKSUM_NONE		0
#define CB_CHECKSUM_PCBIOS		1

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

/**
 * locate_coreboot_table() - Try to find coreboot tables at standard locations
 *
 * Return: address of table that was found, or -ve error number
 */
long locate_coreboot_table(void);

#endif
