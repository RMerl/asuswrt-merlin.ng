/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MediaTek BootROM header definitions
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MTK_IMAGE_H
#define _MTK_IMAGE_H

/* Device header definitions */

/* Header for NOR/SD/eMMC */
union gen_boot_header {
	struct {
		char name[12];
		__le32 version;
		__le32 size;
	};

	uint8_t pad[0x200];
};

#define EMMC_BOOT_NAME		"EMMC_BOOT"
#define SF_BOOT_NAME		"SF_BOOT"
#define SDMMC_BOOT_NAME		"SDMMC_BOOT"

/* Header for NAND */
union nand_boot_header {
	struct {
		char name[12];
		char version[4];
		char id[8];
		__le16 ioif;
		__le16 pagesize;
		__le16 addrcycles;
		__le16 oobsize;
		__le16 pages_of_block;
		__le16 numblocks;
		__le16 writesize_shift;
		__le16 erasesize_shift;
		uint8_t dummy[60];
		uint8_t ecc_parity[28];
	};

	uint8_t data[0x80];
};

#define NAND_BOOT_NAME		"BOOTLOADER!"
#define NAND_BOOT_VERSION	"V006"
#define NAND_BOOT_ID		"NFIINFO"

/* BootROM layout header */
struct brom_layout_header {
	char name[8];
	__le32 version;
	__le32 header_size;
	__le32 total_size;
	__le32 magic;
	__le32 type;
	__le32 header_size_2;
	__le32 total_size_2;
	__le32 unused;
};

#define BRLYT_NAME		"BRLYT"
#define BRLYT_MAGIC		0x42424242

enum brlyt_img_type {
	BRLYT_TYPE_INVALID = 0,
	BRLYT_TYPE_NAND = 0x10002,
	BRLYT_TYPE_EMMC = 0x10005,
	BRLYT_TYPE_NOR = 0x10007,
	BRLYT_TYPE_SDMMC = 0x10008,
	BRLYT_TYPE_SNAND = 0x10009
};

/* Combined device header for NOR/SD/eMMC */
struct gen_device_header {
	union gen_boot_header boot;

	union {
		struct brom_layout_header brlyt;
		uint8_t brlyt_pad[0x400];
	};
};

/* BootROM header definitions */
struct gfh_common_header {
	uint8_t magic[3];
	uint8_t version;
	__le16 size;
	__le16 type;
};

#define GFH_HEADER_MAGIC	"MMM"

#define GFH_TYPE_FILE_INFO	0
#define GFH_TYPE_BL_INFO	1
#define GFH_TYPE_BROM_CFG	7
#define GFH_TYPE_BL_SEC_KEY	3
#define GFH_TYPE_ANTI_CLONE	2
#define GFH_TYPE_BROM_SEC_CFG	8

struct gfh_file_info {
	struct gfh_common_header gfh;
	char name[12];
	__le32 unused;
	__le16 file_type;
	uint8_t flash_type;
	uint8_t sig_type;
	__le32 load_addr;
	__le32 total_size;
	__le32 max_size;
	__le32 hdr_size;
	__le32 sig_size;
	__le32 jump_offset;
	__le32 processed;
};

#define GFH_FILE_INFO_NAME	"FILE_INFO"

#define GFH_FLASH_TYPE_GEN	5
#define GFH_FLASH_TYPE_NAND	2

#define GFH_SIG_TYPE_NONE	0
#define GFH_SIG_TYPE_SHA256	1

struct gfh_bl_info {
	struct gfh_common_header gfh;
	__le32 attr;
};

struct gfh_brom_cfg {
	struct gfh_common_header gfh;
	__le32 cfg_bits;
	__le32 usbdl_by_auto_detect_timeout_ms;
	uint8_t unused[0x48];
	__le32 usbdl_by_kcol0_timeout_ms;
	__le32 usbdl_by_flag_timeout_ms;
	uint32_t pad;
};

#define GFH_BROM_CFG_USBDL_BY_AUTO_DETECT_TIMEOUT_EN	0x02
#define GFH_BROM_CFG_USBDL_AUTO_DETECT_DIS		0x10
#define GFH_BROM_CFG_USBDL_BY_KCOL0_TIMEOUT_EN		0x80
#define GFH_BROM_CFG_USBDL_BY_FLAG_TIMEOUT_EN		0x100

struct gfh_bl_sec_key {
	struct gfh_common_header gfh;
	uint8_t pad[0x20c];
};

struct gfh_anti_clone {
	struct gfh_common_header gfh;
	uint8_t ac_b2k;
	uint8_t ac_b2c;
	uint16_t pad;
	__le32 ac_offset;
	__le32 ac_len;
};

struct gfh_brom_sec_cfg {
	struct gfh_common_header gfh;
	__le32 cfg_bits;
	char customer_name[0x20];
	__le32 pad;
};

#define BROM_SEC_CFG_JTAG_EN	1
#define BROM_SEC_CFG_UART_EN	2

struct gfh_header {
	struct gfh_file_info file_info;
	struct gfh_bl_info bl_info;
	struct gfh_brom_cfg brom_cfg;
	struct gfh_bl_sec_key bl_sec_key;
	struct gfh_anti_clone anti_clone;
	struct gfh_brom_sec_cfg brom_sec_cfg;
};

/* LK image header */

union lk_hdr {
	struct {
		__le32 magic;
		__le32 size;
		char name[32];
		__le32 loadaddr;
		__le32 mode;
	};

	uint8_t data[512];
};

#define LK_PART_MAGIC		0x58881688

#endif /* _MTK_IMAGE_H */
