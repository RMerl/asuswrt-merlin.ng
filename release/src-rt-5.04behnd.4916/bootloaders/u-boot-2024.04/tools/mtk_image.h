/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MediaTek BootROM header definitions
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MTK_IMAGE_H
#define _MTK_IMAGE_H

/* Device header definitions, all fields are little-endian */

/* Header for NOR/SD/eMMC */
union gen_boot_header {
	struct {
		char name[12];
		uint32_t version;
		uint32_t size;
	};

	uint8_t pad[0x200];
};

#define EMMC_BOOT_NAME		"EMMC_BOOT"
#define SF_BOOT_NAME		"SF_BOOT"
#define SDMMC_BOOT_NAME		"SDMMC_BOOT"

/* BootROM layout header */
struct brom_layout_header {
	char name[8];
	uint32_t version;
	uint32_t header_size;
	uint32_t total_size;
	uint32_t magic;
	uint32_t type;
	uint32_t header_size_2;
	uint32_t total_size_2;
	uint32_t unused;
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
	uint32_t magic_version;
	uint16_t size;
	uint16_t type;
};

#define GFH_HEADER_MAGIC		0x4D4D4D
#define GFH_HEADER_VERSION_SHIFT	24

#define GFH_TYPE_FILE_INFO	0
#define GFH_TYPE_BL_INFO	1
#define GFH_TYPE_BROM_CFG	7
#define GFH_TYPE_BL_SEC_KEY	3
#define GFH_TYPE_ANTI_CLONE	2
#define GFH_TYPE_BROM_SEC_CFG	8

struct gfh_file_info {
	struct gfh_common_header gfh;
	char name[12];
	uint32_t unused;
	uint16_t file_type;
	uint8_t flash_type;
	uint8_t sig_type;
	uint32_t load_addr;
	uint32_t total_size;
	uint32_t max_size;
	uint32_t hdr_size;
	uint32_t sig_size;
	uint32_t jump_offset;
	uint32_t processed;
};

#define GFH_FILE_INFO_NAME	"FILE_INFO"

#define GFH_FLASH_TYPE_GEN	5
#define GFH_FLASH_TYPE_NAND	2

#define GFH_SIG_TYPE_NONE	0
#define GFH_SIG_TYPE_SHA256	1

struct gfh_bl_info {
	struct gfh_common_header gfh;
	uint32_t attr;
};

struct gfh_brom_cfg {
	struct gfh_common_header gfh;
	uint32_t cfg_bits;
	uint32_t usbdl_by_auto_detect_timeout_ms;
	uint8_t unused[0x45];
	uint8_t jump_bl_arm64;
	uint8_t unused2[2];
	uint32_t usbdl_by_kcol0_timeout_ms;
	uint32_t usbdl_by_flag_timeout_ms;
	uint32_t pad;
};

#define GFH_BROM_CFG_USBDL_BY_AUTO_DETECT_TIMEOUT_EN	0x02
#define GFH_BROM_CFG_USBDL_AUTO_DETECT_DIS		0x10
#define GFH_BROM_CFG_USBDL_BY_KCOL0_TIMEOUT_EN		0x80
#define GFH_BROM_CFG_USBDL_BY_FLAG_TIMEOUT_EN		0x100
#define GFH_BROM_CFG_JUMP_BL_ARM64_EN			0x1000
#define GFH_BROM_CFG_JUMP_BL_ARM64			0x64

struct gfh_bl_sec_key {
	struct gfh_common_header gfh;
	uint8_t pad[0x20c];
};

struct gfh_anti_clone {
	struct gfh_common_header gfh;
	uint8_t ac_b2k;
	uint8_t ac_b2c;
	uint16_t pad;
	uint32_t ac_offset;
	uint32_t ac_len;
};

struct gfh_brom_sec_cfg {
	struct gfh_common_header gfh;
	uint32_t cfg_bits;
	char customer_name[0x20];
	uint32_t pad;
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
		uint32_t magic;
		uint32_t size;
		char name[32];
		uint32_t loadaddr;
		uint32_t mode;
	};

	uint8_t data[512];
};

#define LK_PART_MAGIC		0x58881688

/* MT7621 NAND SPL image header */

#define MT7621_IH_NMLEN			12
#define MT7621_IH_CRC_POLYNOMIAL	0x04c11db7

struct mt7621_nand_header {
	char ih_name[MT7621_IH_NMLEN];
	uint32_t nand_ac_timing;
	uint32_t ih_stage_offset;
	uint32_t ih_bootloader_offset;
	uint32_t nand_info_1_data;
	uint32_t crc;
};

struct mt7621_stage1_header {
	uint32_t jump_insn[2];
	uint32_t ep;
	uint32_t stage_size;
	uint32_t has_stage2;
	uint32_t next_ep;
	uint32_t next_size;
	uint32_t next_offset;
};

#endif /* _MTK_IMAGE_H */
