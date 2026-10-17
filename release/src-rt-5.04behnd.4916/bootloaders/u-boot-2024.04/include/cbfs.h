/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 */

#ifndef __CBFS_H
#define __CBFS_H

#include <compiler.h>
#include <linux/compiler.h>

struct cbfs_priv;

enum cbfs_result {
	CBFS_SUCCESS = 0,
	CBFS_NOT_INITIALIZED,
	CBFS_BAD_HEADER,
	CBFS_BAD_FILE,
	CBFS_FILE_NOT_FOUND
};

enum cbfs_filetype {
	CBFS_TYPE_BOOTBLOCK = 0x01,
	CBFS_TYPE_CBFSHEADER = 0x02,
	CBFS_TYPE_LEGACY_STAGE = 0x10,
	CBFS_TYPE_PAYLOAD = 0x20,
	CBFS_TYPE_SELF = CBFS_TYPE_PAYLOAD,

	CBFS_TYPE_FIT = 0x21,
	CBFS_TYPE_OPTIONROM = 0x30,
	CBFS_TYPE_BOOTSPLASH = 0x40,
	CBFS_TYPE_RAW = 0x50,
	CBFS_TYPE_VSA = 0x51,
	CBFS_TYPE_MBI = 0x52,
	CBFS_TYPE_MICROCODE = 0x53,
	CBFS_TYPE_FSP = 0x60,
	CBFS_TYPE_MRC = 0x61,
	CBFS_TYPE_MMA = 0x62,
	CBFS_TYPE_EFI = 0x63,
	CBFS_TYPE_STRUCT = 0x70,
	CBFS_TYPE_CMOS_DEFAULT = 0xaa,
	CBFS_TYPE_SPD = 0xab,
	CBFS_TYPE_MRC_CACHE = 0xac,
	CBFS_TYPE_CMOS_LAYOUT = 0x01aa
};

enum {
	CBFS_HEADER_MAGIC	= 0x4f524243,
	CBFS_SIZE_UNKNOWN	= 0xffffffff,
	CBFS_ALIGN_SIZE		= 0x40,
};

/**
 * struct cbfs_header - header at the start of a CBFS region
 *
 * All fields use big-endian format.
 *
 * @magic: Magic number (CBFS_HEADER_MAGIC)
 */
struct cbfs_header {
	u32 magic;
	u32 version;
	u32 rom_size;
	u32 boot_block_size;
	u32 align;
	u32 offset;
	u32 pad[2];
} __packed;

struct cbfs_fileheader {
	u8 magic[8];
	u32 len;
	u32 type;
	/* offset to struct cbfs_file_attribute or 0 */
	u32 attributes_offset;
	u32 offset;
	char filename[];
} __packed;

/**
 * These are standard values for the known compression alogrithms that coreboot
 * knows about for stages and payloads.  Of course, other CBFS users can use
 * whatever values they want, as long as they understand them.
 */
#define CBFS_COMPRESS_NONE  0
#define CBFS_COMPRESS_LZMA  1
#define CBFS_COMPRESS_LZ4   2

/*
 * Depending on how the header was initialized, it may be backed with 0x00 or
 * 0xff, so support both
 */
#define CBFS_FILE_ATTR_TAG_UNUSED 0
#define CBFS_FILE_ATTR_TAG_UNUSED2 0xffffffff
#define CBFS_FILE_ATTR_TAG_COMPRESSION 0x42435a4c
#define CBFS_FILE_ATTR_TAG_HASH 0x68736148

/*
 * The common fields of extended cbfs file attributes. Attributes are expected
 * to start with tag/len, then append their specific fields
 */
struct cbfs_file_attribute {
	u32 tag;
	/* len covers the whole structure, incl. tag and len */
	u32 len;
	u8 data[0];
} __packed;

struct cbfs_file_attr_compression {
	u32 tag;
	u32 len;
	/* whole file compression format. 0 if no compression. */
	u32 compression;
	u32 decompressed_size;
} __packed;

struct cbfs_file_attr_hash {
	u32 tag;
	u32 len;
	u32 hash_type;
	/* hash_data is len - sizeof(struct) bytes */
	u8  hash_data[];
} __packed;

/*** Component sub-headers ***/

/* Following are component sub-headers for the "standard" component types */

/**
 * struct cbfs_stage - sub-header for stage components
 *
 * Stages are loaded by coreboot during the normal boot process
 */
struct cbfs_stage {
	u32 compression;  /** Compression type */
	u64 entry;  /** entry point */
	u64 load;   /** Where to load in memory */
	u32 len;          /** length of data to load */
	u32 memlen;	   /** total length of object in memory */
} __packed;

/**
 * struct cbfs_payload_segment - sub-header for payload components
 *
 * Payloads are loaded by coreboot at the end of the boot process
 */
struct cbfs_payload_segment {
	u32 type;
	u32 compression;
	u32 offset;
	u64 load_addr;
	u32 len;
	u32 mem_len;
} __packed;

struct cbfs_payload {
	struct cbfs_payload_segment segments;
};

#define PAYLOAD_SEGMENT_CODE   0x45444F43
#define PAYLOAD_SEGMENT_DATA   0x41544144
#define PAYLOAD_SEGMENT_BSS    0x20535342
#define PAYLOAD_SEGMENT_PARAMS 0x41524150
#define PAYLOAD_SEGMENT_ENTRY  0x52544E45

struct cbfs_cachenode {
	struct cbfs_cachenode *next;
	void *data;
	char *name;
	u32 type;
	u32 data_length;
	u32 name_length;
	u32 attr_offset;
	u32 comp_algo;
	u32 decomp_size;
};

/**
 * file_cbfs_error() - Return a string describing the most recent error
 * condition.
 *
 * Return: A pointer to the constant string.
 */
const char *file_cbfs_error(void);

/**
 * cbfs_get_result() - Get the result of the last CBFS operation
 *
 *Return: last result
 */
enum cbfs_result cbfs_get_result(void);

/**
 * file_cbfs_init() - Initialize the CBFS driver and load metadata into RAM.
 *
 * @end_of_rom: Points to the end of the ROM the CBFS should be read from
 * Return: 0 if OK, -ve on error
 */
int file_cbfs_init(ulong end_of_rom);

/**
 * file_cbfs_get_header() - Get the header structure for the current CBFS.
 *
 * Return: A pointer to the constant structure, or NULL if there is none.
 */
const struct cbfs_header *file_cbfs_get_header(void);

/**
 * cbfs_get_first() - Get the first file in a CBFS
 *
 * Return: pointer to first file, or NULL if it is empty
 */
const struct cbfs_cachenode *cbfs_get_first(const struct cbfs_priv *priv);

/**
 * cbfs_get_next() - Get the next file in a CBFS
 *
 * @filep: Pointer to current file; updated to point to the next file, if any,
 *	else NULL
 */
void cbfs_get_next(const struct cbfs_cachenode **filep);

/**
 * file_cbfs_get_first() - Get a handle for the first file in CBFS.
 *
 * Return: A handle for the first file in CBFS, NULL on error.
 */
const struct cbfs_cachenode *file_cbfs_get_first(void);

/**
 * file_cbfs_get_next() - Get a handle to the file after this one in CBFS.
 *
 * @file:		A pointer to the handle to advance.
 */
void file_cbfs_get_next(const struct cbfs_cachenode **file);

/**
 * file_cbfs_find() - Find a file with a particular name in CBFS.
 *
 * @name:		The name to search for.
 *
 * Return: A handle to the file, or NULL on error.
 */
const struct cbfs_cachenode *file_cbfs_find(const char *name);

/**
 * cbfs_find_file() - Find a file in a given CBFS
 *
 * @cbfs: CBFS to look in (use cbfs_init_mem() to set it up)
 * @name: Filename to look for
 * Return: pointer to CBFS node if found, else NULL
 */
const struct cbfs_cachenode *cbfs_find_file(struct cbfs_priv *cbfs,
					    const char *name);

/**
 * cbfs_init_mem() - Set up a new CBFS
 *
 * @base: Base address of CBFS
 * @size: Size of CBFS if known, else CBFS_SIZE_UNKNOWN
 * @require_header: true to read a header at the start, false to not require one
 * @cbfsp: Returns a pointer to CBFS on success
 * Return: 0 if OK, -ve on error
 */
int cbfs_init_mem(ulong base, ulong size, bool require_hdr,
		  struct cbfs_priv **privp);

/***************************************************************************/
/* All of the functions below can be used without first initializing CBFS. */
/***************************************************************************/

/**
 * file_cbfs_find_uncached() - Find a file in CBFS given the end of the ROM
 *
 * Note that @node should be declared by the caller. This design is to avoid
 * the need for allocation here.
 *
 * @end_of_rom: Points to the end of the ROM the CBFS should be read from
 * @name: The name to search for
 * @node: Returns the contents of the node if found (i.e. copied into *node)
 * Return: 0 on success, -ENOENT if not found, -EFAULT on bad header
 */
int file_cbfs_find_uncached(ulong end_of_rom, const char *name,
			    struct cbfs_cachenode *node);

/**
 * file_cbfs_find_uncached_base() - Find a file in CBFS given the base address
 *
 * Note that @node should be declared by the caller. This design is to avoid
 * the need for allocation here.
 *
 * @base: Points to the base of the CBFS
 * @name: The name to search for
 * @node: Returns the contents of the node if found (i.e. copied into *node)
 * Return: 0 on success, -ENOENT if not found, -EFAULT on bad header
 */
int file_cbfs_find_uncached_base(ulong base, const char *name,
				 struct cbfs_cachenode *node);

/**
 * file_cbfs_name() - Get the name of a file in CBFS.
 *
 * @file:		The handle to the file.
 *
 * Return: The name of the file, NULL on error.
 */
const char *file_cbfs_name(const struct cbfs_cachenode *file);

/**
 * file_cbfs_size() - Get the size of a file in CBFS.
 *
 * @file:		The handle to the file.
 *
 * Return: The size of the file, zero on error.
 */
u32 file_cbfs_size(const struct cbfs_cachenode *file);

/**
 * file_cbfs_type() - Get the type of a file in CBFS.
 *
 * @file:		The handle to the file.
 *
 * Return: The type of the file, zero on error.
 */
u32 file_cbfs_type(const struct cbfs_cachenode *file);

/**
 * file_cbfs_read() - Read a file from CBFS into RAM
 *
 * @file:		A handle to the file to read.
 * @buffer:		Where to read it into memory.
 * @maxsize:		Maximum number of bytes to read
 *
 * Return: If positive or zero, the number of characters read. If negative, an
 *	   error occurred.
 */
long file_cbfs_read(const struct cbfs_cachenode *file, void *buffer,
		    unsigned long maxsize);

#endif /* __CBFS_H */
