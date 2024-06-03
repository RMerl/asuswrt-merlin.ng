/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Michal Simek <michals@xilinx.com>
 * Copyright (C) 2015 Nathan Rossi <nathan@nathanrossi.com>
 *
 * The following Boot Header format/structures and values are defined in the
 * following documents:
 *   * ug1085 ZynqMP TRM doc v1.4 (Chapter 11, Table 11-4)
 *   * ug1137 ZynqMP Software Developer Guide v6.0 (Chapter 16)
 */

#ifndef _ZYNQMPIMAGE_H_
#define _ZYNQMPIMAGE_H_

#include <stdint.h>

#define HEADER_INTERRUPT_DEFAULT (cpu_to_le32(0xeafffffe))
#define HEADER_REGINIT_NULL (cpu_to_le32(0xffffffff))
#define HEADER_WIDTHDETECTION (cpu_to_le32(0xaa995566))
#define HEADER_IMAGEIDENTIFIER (cpu_to_le32(0x584c4e58))
#define HEADER_CPU_SELECT_MASK		(0x3 << 10)
#define HEADER_CPU_SELECT_R5_SINGLE	(0x0 << 10)
#define HEADER_CPU_SELECT_A53_32BIT	(0x1 << 10)
#define HEADER_CPU_SELECT_A53_64BIT	(0x2 << 10)
#define HEADER_CPU_SELECT_R5_DUAL	(0x3 << 10)

enum {
	ENCRYPTION_EFUSE = 0xa5c3c5a3,
	ENCRYPTION_OEFUSE = 0xa5c3c5a7,
	ENCRYPTION_BBRAM = 0x3a5c3c5a,
	ENCRYPTION_OBBRAM = 0xa35c7ca5,
	ENCRYPTION_NONE = 0x0,
};

struct zynqmp_reginit {
	uint32_t address;
	uint32_t data;
};

#define HEADER_INTERRUPT_VECTORS	8
#define HEADER_REGINITS			256

struct image_header_table {
	uint32_t version;		  /* 0x00 */
	uint32_t nr_parts;		  /* 0x04 */
	uint32_t partition_header_offset; /* 0x08, divided by 4 */
	uint32_t image_header_offset;	  /* 0x0c, divided by 4 */
	uint32_t auth_certificate_offset; /* 0x10 */
	uint32_t boot_device;		  /* 0x14 */
	uint32_t __reserved1[9];	  /* 0x18 - 0x38 */
	uint32_t checksum;		  /* 0x3c */
};

#define PART_ATTR_VEC_LOCATION		0x800000
#define PART_ATTR_BS_BLOCK_SIZE_MASK	0x700000
#define     PART_ATTR_BS_BLOCK_SIZE_DEFAULT	0x000000
#define     PART_ATTR_BS_BLOCK_SIZE_8MB		0x400000
#define PART_ATTR_BIG_ENDIAN		0x040000
#define PART_ATTR_PART_OWNER_MASK	0x030000
#define     PART_ATTR_PART_OWNER_FSBL		0x000000
#define     PART_ATTR_PART_OWNER_UBOOT		0x010000
#define PART_ATTR_RSA_SIG		0x008000
#define PART_ATTR_CHECKSUM_MASK		0x007000
#define    PART_ATTR_CHECKSUM_NONE		0x000000
#define    PART_ATTR_CHECKSUM_MD5		0x001000
#define    PART_ATTR_CHECKSUM_SHA2		0x002000
#define    PART_ATTR_CHECKSUM_SHA3		0x003000
#define PART_ATTR_DEST_CPU_SHIFT	8
#define PART_ATTR_DEST_CPU_MASK		0x000f00
#define    PART_ATTR_DEST_CPU_NONE		0x000000
#define    PART_ATTR_DEST_CPU_A53_0		0x000100
#define    PART_ATTR_DEST_CPU_A53_1		0x000200
#define    PART_ATTR_DEST_CPU_A53_2		0x000300
#define    PART_ATTR_DEST_CPU_A53_3		0x000400
#define    PART_ATTR_DEST_CPU_R5_0		0x000500
#define    PART_ATTR_DEST_CPU_R5_1		0x000600
#define    PART_ATTR_DEST_CPU_R5_L		0x000700
#define    PART_ATTR_DEST_CPU_PMU		0x000800
#define PART_ATTR_ENCRYPTED		0x000080
#define PART_ATTR_DEST_DEVICE_SHIFT	4
#define PART_ATTR_DEST_DEVICE_MASK	0x000070
#define    PART_ATTR_DEST_DEVICE_NONE		0x000000
#define    PART_ATTR_DEST_DEVICE_PS		0x000010
#define    PART_ATTR_DEST_DEVICE_PL		0x000020
#define    PART_ATTR_DEST_DEVICE_PMU		0x000030
#define    PART_ATTR_DEST_DEVICE_XIP		0x000040
#define PART_ATTR_A53_EXEC_AARCH32	0x000008
#define PART_ATTR_TARGET_EL_SHIFT	1
#define PART_ATTR_TARGET_EL_MASK	0x000006
#define PART_ATTR_TZ_SECURE		0x000001

static const char *dest_cpus[0x10] = {
	"none", "a5x-0", "a5x-1", "a5x-2", "a5x-3", "r5-0", "r5-1",
	"r5-lockstep", "pmu", "unknown", "unknown", "unknown", "unknown",
	"unknown", "unknown", "unknown"
};

struct partition_header {
	uint32_t len_enc;		  /* 0x00, divided by 4 */
	uint32_t len_unenc;		  /* 0x04, divided by 4 */
	uint32_t len;			  /* 0x08, divided by 4 */
	uint32_t next_partition_offset;   /* 0x0c */
	uint64_t entry_point;		  /* 0x10 */
	uint64_t load_address;		  /* 0x18 */
	uint32_t offset;		  /* 0x20, divided by 4 */
	uint32_t attributes;		  /* 0x24 */
	uint32_t __reserved1;		  /* 0x28 */
	uint32_t checksum_offset;	  /* 0x2c, divided by 4 */
	uint32_t __reserved2;		  /* 0x30 */
	uint32_t auth_certificate_offset; /* 0x34 */
	uint32_t __reserved3;		  /* 0x38 */
	uint32_t checksum;		  /* 0x3c */
};

struct zynqmp_header {
	uint32_t interrupt_vectors[HEADER_INTERRUPT_VECTORS]; /* 0x0 */
	uint32_t width_detection; /* 0x20 */
	uint32_t image_identifier; /* 0x24 */
	uint32_t encryption; /* 0x28 */
	uint32_t image_load; /* 0x2c */
	uint32_t image_offset; /* 0x30 */
	uint32_t pfw_image_length; /* 0x34 */
	uint32_t total_pfw_image_length; /* 0x38 */
	uint32_t image_size; /* 0x3c */
	uint32_t image_stored_size; /* 0x40 */
	uint32_t image_attributes; /* 0x44 */
	uint32_t checksum; /* 0x48 */
	uint32_t __reserved1[19]; /* 0x4c */
	uint32_t image_header_table_offset; /* 0x98 */
	uint32_t __reserved2[7]; /* 0x9c */
	struct zynqmp_reginit register_init[HEADER_REGINITS]; /* 0xb8 */
	uint32_t __reserved4[66]; /* 0x9c0 */
};

void zynqmpimage_default_header(struct zynqmp_header *ptr);
void zynqmpimage_print_header(const void *ptr);

#endif /* _ZYNQMPIMAGE_H_ */
