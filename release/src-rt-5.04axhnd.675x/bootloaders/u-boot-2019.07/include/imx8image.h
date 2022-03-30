/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#ifndef _IMX8IMAGE_H_
#define _IMX8IMAGE_H_

#include <image.h>
#include <inttypes.h>
#include "imagetool.h"
#include "linux/kernel.h"

#define __packed   __attribute__((packed))

#define IV_MAX_LEN			32
#define HASH_MAX_LEN			64
#define MAX_NUM_IMGS			6
#define MAX_NUM_SRK_RECORDS		4

#define IVT_HEADER_TAG_B0		0x87
#define IVT_VERSION_B0			0x00

#define IMG_FLAG_HASH_SHA256		0x000
#define IMG_FLAG_HASH_SHA384		0x100
#define IMG_FLAG_HASH_SHA512		0x200

#define IMG_FLAG_ENCRYPTED_MASK		0x400
#define IMG_FLAG_ENCRYPTED_SHIFT	0x0A

#define IMG_FLAG_BOOTFLAGS_MASK		0xFFFF0000
#define IMG_FLAG_BOOTFLAGS_SHIFT	0x10

#define IMG_ARRAY_ENTRY_SIZE		128
#define HEADER_IMG_ARRAY_OFFSET		0x10

#define HASH_TYPE_SHA_256		256
#define HASH_TYPE_SHA_384		384
#define HASH_TYPE_SHA_512		512

#define IMAGE_HASH_ALGO_DEFAULT		384
#define IMAGE_PADDING_DEFAULT		0x1000

#define DCD_ENTRY_ADDR_IN_SCFW		0x240

#define CONTAINER_ALIGNMENT		0x400
#define CONTAINER_FLAGS_DEFAULT		0x10
#define CONTAINER_FUSE_DEFAULT		0x0

#define SIGNATURE_BLOCK_HEADER_LENGTH	0x10

#define MAX_NUM_OF_CONTAINER		2

#define FIRST_CONTAINER_HEADER_LENGTH	0x400

#define BOOT_IMG_META_MU_RID_SHIFT	10
#define BOOT_IMG_META_PART_ID_SHIFT	20

#define IMAGE_A35_DEFAULT_META(PART)	(((PART == 0) ? \
					 PARTITION_ID_AP : PART) << \
					 BOOT_IMG_META_PART_ID_SHIFT | \
					 SC_R_MU_0A << \
					 BOOT_IMG_META_MU_RID_SHIFT | \
					 SC_R_A35_0)

#define IMAGE_A53_DEFAULT_META(PART)	(((PART == 0) ? \
					 PARTITION_ID_AP : PART) << \
					 BOOT_IMG_META_PART_ID_SHIFT | \
					 SC_R_MU_0A << \
					 BOOT_IMG_META_MU_RID_SHIFT | \
					 SC_R_A53_0)

#define IMAGE_A72_DEFAULT_META(PART)	(((PART == 0) ? \
					 PARTITION_ID_AP : PART) << \
					 BOOT_IMG_META_PART_ID_SHIFT | \
					 SC_R_MU_0A << \
					 BOOT_IMG_META_MU_RID_SHIFT | \
					 SC_R_A72_0)

#define IMAGE_M4_0_DEFAULT_META(PART)	(((PART == 0) ? \
					 PARTITION_ID_M4 : PART) << \
					 BOOT_IMG_META_PART_ID_SHIFT | \
					 SC_R_M4_0_MU_1A << \
					 BOOT_IMG_META_MU_RID_SHIFT | \
					 SC_R_M4_0_PID0)

#define IMAGE_M4_1_DEFAULT_META(PART)	(((PART == 0) ? \
					 PARTITION_ID_M4 : PART) << \
					 BOOT_IMG_META_PART_ID_SHIFT | \
					 SC_R_M4_1_MU_1A << \
					 BOOT_IMG_META_MU_RID_SHIFT | \
					 SC_R_M4_1_PID0)

#define CONTAINER_IMAGE_ARRAY_START_OFFSET	0x2000

typedef struct {
	uint8_t version;
	uint16_t length;
	uint8_t tag;
	uint16_t srk_table_offset;
	uint16_t cert_offset;
	uint16_t blob_offset;
	uint16_t signature_offset;
	uint32_t reserved;
} __packed sig_blk_hdr_t;

typedef struct {
	uint32_t offset;
	uint32_t size;
	uint64_t dst;
	uint64_t entry;
	uint32_t hab_flags;
	uint32_t meta;
	uint8_t hash[HASH_MAX_LEN];
	uint8_t iv[IV_MAX_LEN];
} __packed boot_img_t;

typedef struct {
	uint8_t version;
	uint16_t length;
	uint8_t tag;
	uint32_t flags;
	uint16_t sw_version;
	uint8_t fuse_version;
	uint8_t num_images;
	uint16_t sig_blk_offset;
	uint16_t reserved;
	boot_img_t img[MAX_NUM_IMGS];
	sig_blk_hdr_t sig_blk_hdr;
	uint32_t sigblk_size;
	uint32_t padding;
} __packed flash_header_v3_t;

typedef struct {
	flash_header_v3_t fhdr[MAX_NUM_OF_CONTAINER];
}  __packed imx_header_v3_t;

struct image_array {
	char *name;
	unsigned int core_type;
	unsigned int core_id;
	unsigned int load_addr;
};

enum imx8image_cmd {
	CMD_INVALID,
	CMD_BOOT_FROM,
	CMD_FUSE_VERSION,
	CMD_SW_VERSION,
	CMD_MSG_BLOCK,
	CMD_FILEOFF,
	CMD_FLAG,
	CMD_APPEND,
	CMD_PARTITION,
	CMD_SOC_TYPE,
	CMD_CONTAINER,
	CMD_IMAGE,
	CMD_DATA
};

enum imx8image_core_type {
	CFG_CORE_INVALID,
	CFG_SCU,
	CFG_M40,
	CFG_M41,
	CFG_A35,
	CFG_A53,
	CFG_A72
};

enum imx8image_fld_types {
	CFG_INVALID = -1,
	CFG_COMMAND,
	CFG_CORE_TYPE,
	CFG_IMAGE_NAME,
	CFG_LOAD_ADDR
};

typedef enum SOC_TYPE {
	NONE = 0,
	QX,
	QM
} soc_type_t;

typedef enum option_type {
	NO_IMG = 0,
	DCD,
	SCFW,
	SECO,
	M40,
	M41,
	AP,
	OUTPUT,
	SCD,
	CSF,
	FLAG,
	DEVICE,
	NEW_CONTAINER,
	APPEND,
	DATA,
	PARTITION,
	FILEOFF,
	MSG_BLOCK
} option_type_t;

typedef struct {
	option_type_t option;
	char *filename;
	uint64_t src;
	uint64_t dst;
	uint64_t entry;
	uint64_t ext;
} image_t;

#define CORE_SC         1
#define CORE_CM4_0      2
#define CORE_CM4_1      3
#define CORE_CA53       4
#define CORE_CA35       4
#define CORE_CA72       5
#define CORE_SECO       6

#define SC_R_OTP	357U
#define SC_R_DEBUG	354U
#define SC_R_ROM_0	236U

#define MSG_DEBUG_EN	SC_R_DEBUG
#define MSG_FUSE	SC_R_OTP
#define MSG_FIELD	SC_R_ROM_0

#define IMG_TYPE_CSF     0x01   /* CSF image type */
#define IMG_TYPE_SCD     0x02   /* SCD image type */
#define IMG_TYPE_EXEC    0x03   /* Executable image type */
#define IMG_TYPE_DATA    0x04   /* Data image type */
#define IMG_TYPE_DCD_DDR 0x05   /* DCD/DDR image type */
#define IMG_TYPE_SECO    0x06   /* SECO image type */
#define IMG_TYPE_PROV    0x07   /* Provisioning image type */
#define IMG_TYPE_DEK     0x08   /* DEK validation type */

#define IMG_TYPE_SHIFT   0
#define IMG_TYPE_MASK    0x1f
#define IMG_TYPE(x)      (((x) & IMG_TYPE_MASK) >> IMG_TYPE_SHIFT)

#define BOOT_IMG_FLAGS_CORE_MASK		0xF
#define BOOT_IMG_FLAGS_CORE_SHIFT		0x04
#define BOOT_IMG_FLAGS_CPU_RID_MASK		0x3FF0
#define BOOT_IMG_FLAGS_CPU_RID_SHIFT		4
#define BOOT_IMG_FLAGS_MU_RID_MASK		0xFFC000
#define BOOT_IMG_FLAGS_MU_RID_SHIFT		14
#define BOOT_IMG_FLAGS_PARTITION_ID_MASK	0x1F000000
#define BOOT_IMG_FLAGS_PARTITION_ID_SHIFT	24

/* Resource id used in scfw */
#define SC_R_A35_0		508
#define SC_R_A53_0		1
#define SC_R_A72_0		6
#define SC_R_MU_0A		213
#define SC_R_M4_0_PID0		278
#define SC_R_M4_0_MU_1A		297
#define SC_R_M4_1_PID0		298
#define SC_R_M4_1_MU_1A		317
#define PARTITION_ID_M4		0
#define PARTITION_ID_AP		1

#define IMG_STACK_SIZE	32

#define append(p, s, l) do { \
	memcpy((p), (uint8_t *)(s), (l)); (p) += (l); \
	} while (0)

#endif
