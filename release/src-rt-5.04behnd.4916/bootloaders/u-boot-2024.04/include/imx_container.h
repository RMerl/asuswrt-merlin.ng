/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018-2019 NXP
 */

#ifndef __CONTAINER_HEADER_H_
#define __CONTAINER_HEADER_H_

#include <linux/sizes.h>
#include <linux/types.h>

#define IV_MAX_LEN			32
#define HASH_MAX_LEN			64

#define CONTAINER_HDR_ALIGNMENT 0x400
#define CONTAINER_HDR_EMMC_OFFSET 0
#define CONTAINER_HDR_MMCSD_OFFSET SZ_32K
#define CONTAINER_HDR_QSPI_OFFSET SZ_4K
#define CONTAINER_HDR_NAND_OFFSET SZ_128M

#define CONTAINER_HDR_TAG 0x87
#define CONTAINER_HDR_VERSION 0

struct container_hdr {
	u8 version;
	u8 length_lsb;
	u8 length_msb;
	u8 tag;
	u32 flags;
	u16 sw_version;
	u8 fuse_version;
	u8 num_images;
	u16 sig_blk_offset;
	u16 reserved;
} __packed;

struct boot_img_t {
	u32 offset;
	u32 size;
	u64 dst;
	u64 entry;
	u32 hab_flags;
	u32 meta;
	u8 hash[HASH_MAX_LEN];
	u8 iv[IV_MAX_LEN];
} __packed;

struct signature_block_hdr {
	u8 version;
	u8 length_lsb;
	u8 length_msb;
	u8 tag;
	u16 srk_table_offset;
	u16 cert_offset;
	u16 blob_offset;
	u16 signature_offset;
	u32 reserved;
} __packed;

struct generate_key_blob_hdr {
	u8 version;
	u8 length_lsb;
	u8 length_msb;
	u8 tag;
	u8 flags;
	u8 size;
	u8 algorithm;
	u8 mode;
} __packed;

int get_container_size(ulong addr, u16 *header_length);

static inline bool valid_container_hdr(struct container_hdr *container)
{
	return container->tag == CONTAINER_HDR_TAG &&
	       container->version == CONTAINER_HDR_VERSION;
}
#endif
