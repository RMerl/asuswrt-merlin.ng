/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _BOOT_BLOB_H
#define _BOOT_BLOB_H

#define BOOT_BLOB_SUCCESS		0
#define BOOT_BLOB_NOT_IN_HASTTBL	-1
#define BOOT_BLOB_INVALID_PARAM		-2
#define BOOT_BLOB_MAGIC_NOT_FOUND	-3
#define BOOT_BLOB_VERIFICATION_FAIL	-4

#define DPFE_MAGIC_MASK		0xffffff00
#define MCB_TABLE_MAGIC		0x00CB00CB
#define DDR3_TABLE_MAGIC	0x64447233
#define DDR4_TABLE_MAGIC	0x64447234
#define DPFE_DDR3_TABLE_MAGIC	0x64503300
#define DPFE_DDR4_TABLE_MAGIC	0x64503400
#define UBOOT_ENV_MAGIC		0x75456e76
#define TPL_TABLE_MAGIC		0x74506c21

#define IS_DPFE_DDR3_MAGIC(magic)	\
	(((magic)&DPFE_MAGIC_MASK) == DPFE_DDR3_TABLE_MAGIC)
#define IS_DPFE_DDR4_MAGIC(magic)	\
	(((magic)&DPFE_MAGIC_MASK) == DPFE_DDR4_TABLE_MAGIC)
#define IS_DPFE_MAGIC(magic)		\
	(IS_DPFE_DDR3_MAGIC(magic) || IS_DPFE_DDR4_MAGIC(magic))

#define BOOT_BLOB_MAX_MAGIC_SEARCH	6
#define BOOT_BLOB_MAX_MAGIC_NUMS	6
#define BOOT_BLOB_SEARCH_START_ADDR	0x0
#define BOOT_BLOB_SEARCH_END_ADDR	0x200000
#define BOOT_BLOB_SEARCH_BOUNDARY	0x1000
#define BOOT_BLOB_MAX_ENV_SIZE		0x10000

#if defined(CONFIG_BCM6846)
#define BOOT_BLOB_MAX_DDR_SIZE		0xb000
#elif defined(CONFIG_BCM6878)
#define BOOT_BLOB_MAX_DDR_SIZE		0x9000
#else
#define BOOT_BLOB_MAX_DDR_SIZE		0x10000
#endif

struct overlays {
	uint32_t ovltype;
	uint32_t selector;
	uint32_t offset;
	uint32_t size;
	uint8_t sha[32];
};

typedef struct _boot_blob_hdr {
	uint32_t magic;
	uint32_t length;
	uint32_t crc;
}boot_blob_hdr;

int load_boot_blob(uint32_t magic, uint32_t sel, void *data, int* len);
void *load_spl_env(void *buffer);
struct overlays* get_boot_blob_hash_entry(int i);

#endif
