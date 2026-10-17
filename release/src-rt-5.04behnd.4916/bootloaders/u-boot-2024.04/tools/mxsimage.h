/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX28 SB image generator
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#ifndef __MXSSB_H__
#define __MXSSB_H__

#include <stdint.h>
#include <arpa/inet.h>

#define SB_BLOCK_SIZE		16

#define roundup(x, y)		((((x) + ((y) - 1)) / (y)) * (y))
#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))

struct sb_boot_image_version {
	uint16_t	major;
	uint16_t	pad0;
	uint16_t	minor;
	uint16_t	pad1;
	uint16_t	revision;
	uint16_t	pad2;
};

struct sb_boot_image_header {
	union {
		/* SHA1 of the header. */
		uint8_t	digest[20];
		struct {
			/* CBC-MAC initialization vector. */
			uint8_t iv[16];
			uint8_t extra[4];
		};
	};
	/* 'STMP' */
	uint8_t		signature1[4];
	/* Major version of the image format. */
	uint8_t		major_version;
	/* Minor version of the image format. */
	uint8_t		minor_version;
	/* Flags associated with the image. */
	uint16_t	flags;
	/* Size of the image in 16b blocks. */
	uint32_t	image_blocks;
	/* Offset of the first tag in 16b blocks. */
	uint32_t	first_boot_tag_block;
	/* ID of the section to boot from. */
	uint32_t	first_boot_section_id;
	/* Amount of crypto keys. */
	uint16_t	key_count;
	/* Offset to the key dictionary in 16b blocks. */
	uint16_t	key_dictionary_block;
	/* Size of this header in 16b blocks. */
	uint16_t	header_blocks;
	/* Amount of section headers. */
	uint16_t	section_count;
	/* Section header size in 16b blocks. */
	uint16_t	section_header_size;
	/* Padding to align timestamp to uint64_t. */
	uint8_t		padding0[2];
	/* 'sgtl' (since v1.1) */
	uint8_t		signature2[4];
	/* Image generation date, in microseconds since 1.1.2000 . */
	uint64_t	timestamp_us;
	/* Product version. */
	struct sb_boot_image_version
			product_version;
	/* Component version. */
	struct sb_boot_image_version
			component_version;
	/* Drive tag for the system drive. (since v1.1) */
	uint16_t	drive_tag;
	/* Padding. */
	uint8_t		padding1[6];
};

#define	SB_VERSION_MAJOR	1
#define	SB_VERSION_MINOR	1

/* Enable to HTLLC boot report. */
#define SB_IMAGE_FLAG_DISPLAY_PROGRESS	(1 << 0)
#define SB_IMAGE_FLAGS_MASK SB_IMAGE_FLAG_DISPLAY_PROGRESS

struct sb_key_dictionary_key {
	/* The CBC-MAC of image and sections header. */
	uint8_t		cbc_mac[SB_BLOCK_SIZE];
	/* The AES key encrypted by image key (zero). */
	uint8_t		key[SB_BLOCK_SIZE];
};

struct sb_ivt_header {
	uint32_t	header;
	uint32_t	entry;
	uint32_t	reserved1;
	uint32_t	dcd;
	uint32_t	boot_data;
	uint32_t	self;
	uint32_t	csf;
	uint32_t	reserved2;
};

#define	SB_HAB_IVT_TAG			0xd1UL
#define	SB_HAB_DCD_TAG			0xd2UL

#define	SB_HAB_VERSION			0x40UL

/*
 * The "size" field in the IVT header is not naturally aligned,
 * use this macro to fill first 4 bytes of the IVT header without
 * causing issues on some systems (esp. M68k, PPC, MIPS-BE, ARM-BE).
 */
static inline uint32_t sb_hab_ivt_header(void)
{
	uint32_t ret = 0;
	ret |= SB_HAB_IVT_TAG << 24;
	ret |= sizeof(struct sb_ivt_header) << 16;
	ret |= SB_HAB_VERSION;
	return htonl(ret);
}

struct sb_sections_header {
	/* Section number. */
	uint32_t	section_number;
	/* Offset of this sections first instruction after "TAG". */
	uint32_t	section_offset;
	/* Size of the section in 16b blocks. */
	uint32_t	section_size;
	/* Section flags. */
	uint32_t	section_flags;
};

#define	SB_SECTION_FLAG_BOOTABLE	(1 << 0)

struct sb_command {
	struct {
		uint8_t		checksum;
		uint8_t		tag;
		uint16_t	flags;
#define ROM_TAG_CMD_FLAG_ROM_LAST_TAG	0x1
#define ROM_LOAD_CMD_FLAG_DCD_LOAD	0x1	/* MX28 only */
#define ROM_JUMP_CMD_FLAG_HAB		0x1	/* MX28 only */
#define ROM_CALL_CMD_FLAG_HAB		0x1	/* MX28 only */
	} header;

	union {
	struct {
		uint32_t	reserved[3];
	} nop;
	struct {
		uint32_t	section_number;
		uint32_t	section_length;
		uint32_t	section_flags;
	} tag;
	struct {
		uint32_t	address;
		uint32_t	count;
		uint32_t	crc32;
	} load;
	struct {
		uint32_t	address;
		uint32_t	count;
		uint32_t	pattern;
	} fill;
	struct {
		uint32_t	address;
		uint32_t	reserved;
		/* Passed in register r0 before JUMP */
		uint32_t	argument;
	} jump;
	struct {
		uint32_t	address;
		uint32_t	reserved;
		/* Passed in register r0 before CALL */
		uint32_t	argument;
	} call;
	struct {
		uint32_t	reserved1;
		uint32_t	reserved2;
		uint32_t	mode;
	} mode;

	};
};

/*
 * Most of the mode names are same or at least similar
 * on i.MX23 and i.MX28, but some of the mode names
 * differ. The "name" field represents the mode name
 * on i.MX28 as seen in Table 12-2 of the datasheet.
 * The "altname" field represents the differently named
 * fields on i.MX23 as seen in Table 35-3 of the
 * datasheet.
 */
static const struct {
	const char	*name;
	const char	*altname;
	const uint8_t	mode;
} modetable[] = {
	{ "USB",		NULL,		0x00 },
	{ "I2C",		NULL,		0x01 },
	{ "SPI2_FLASH",		"SPI1_FLASH",	0x02 },
	{ "SPI3_FLASH",		"SPI2_FLASH",	0x03 },
	{ "NAND_BCH",		NULL,		0x04 },
	{ "JTAG",		NULL,		0x06 },
	{ "SPI3_EEPROM",	"SPI2_EEPROM",	0x08 },
	{ "SD_SSP0",		NULL,		0x09 },
	{ "SD_SSP1",		NULL,		0x0A }
};

enum sb_tag {
	ROM_NOP_CMD	= 0x00,
	ROM_TAG_CMD	= 0x01,
	ROM_LOAD_CMD	= 0x02,
	ROM_FILL_CMD	= 0x03,
	ROM_JUMP_CMD	= 0x04,
	ROM_CALL_CMD	= 0x05,
	ROM_MODE_CMD	= 0x06
};

struct sb_source_entry {
	uint8_t		tag;
	uint32_t	address;
	uint32_t	flags;
	char		*filename;
};

#endif	/* __MXSSB_H__ */
