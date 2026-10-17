/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MediaTek BootROM NAND header definitions
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MTK_NAND_HEADERS_H
#define _MTK_NAND_HEADERS_H

#include <stdint.h>
#include <stdbool.h>

struct nand_header_info {
	uint32_t page_size;
	uint32_t spare_size;
	uint32_t gfh_offset;
	bool snfi;
};

/* AP BROM Header for NAND */
union nand_boot_header {
	struct {
		char name[12];
		char version[4];
		char id[8];
		uint16_t ioif;			/* I/O interface */
		uint16_t pagesize;		/* NAND page size */
		uint16_t addrcycles;		/* Address cycles */
		uint16_t oobsize;		/* NAND page spare size */
		uint16_t pages_of_block;	/* Pages of one block */
		uint16_t numblocks;		/* Total blocks of NAND chip */
		uint16_t writesize_shift;
		uint16_t erasesize_shift;
		uint8_t dummy[60];
		uint8_t ecc_parity[28];		/* ECC parity of this header */
	};

	uint8_t data[0x80];
};

/* HSM BROM Header for NAND */
union hsm_nand_boot_header {
	struct {
		char id[8];
		uint32_t version;		/* Header version */
		uint32_t config;		/* Header config */
		uint32_t sector_size;		/* ECC step size */
		uint32_t fdm_size;		/* User OOB size of a step */
		uint32_t fdm_ecc_size;		/* ECC parity size of a step */
		uint32_t lbs;
		uint32_t page_size;		/* NAND page size */
		uint32_t spare_size;		/* NAND page spare size */
		uint32_t page_per_block;	/* Pages of one block */
		uint32_t blocks;		/* Total blocks of NAND chip */
		uint32_t plane_sel_position;	/* Plane bit position */
		uint32_t pll;			/* Value of pll reg */
		uint32_t acccon;		/* Value of access timing reg */
		uint32_t strobe_sel;		/* Value of DQS selection reg*/
		uint32_t acccon1;		/* Value of access timing reg */
		uint32_t dqs_mux;		/* Value of DQS mux reg */
		uint32_t dqs_ctrl;		/* Value of DQS control reg */
		uint32_t delay_ctrl;		/* Value of delay ctrl reg */
		uint32_t latch_lat;		/* Value of latch latency reg */
		uint32_t sample_delay;		/* Value of sample delay reg */
		uint32_t driving;		/* Value of driving reg */
		uint32_t bl_start;		/* Bootloader start addr */
		uint32_t bl_end;		/* Bootloader end addr */
		uint8_t ecc_parity[42];		/* ECC parity of this header */
	};

	uint8_t data[0x8E];
};

/* HSM2.0 BROM Header for NAND */
union hsm20_nand_boot_header {
	struct {
		char id[8];
		uint32_t version;		/* Header version */
		uint32_t config;		/* Header config */
		uint32_t sector_size;		/* ECC step size */
		uint32_t fdm_size;		/* User OOB size of a step */
		uint32_t fdm_ecc_size;		/* ECC parity size of a step */
		uint32_t lbs;
		uint32_t page_size;		/* NAND page size */
		uint32_t spare_size;		/* NAND page spare size */
		uint32_t page_per_block;	/* Pages of one block */
		uint32_t blocks;		/* Total blocks of NAND chip */
		uint32_t plane_sel_position;	/* Plane bit position */
		uint32_t pll;			/* Value of pll reg */
		uint32_t acccon;		/* Value of access timing reg */
		uint32_t strobe_sel;		/* Value of DQS selection reg*/
		uint32_t acccon1;		/* Value of access timing reg */
		uint32_t dqs_mux;		/* Value of DQS mux reg */
		uint32_t dqs_ctrl;		/* Value of DQS control reg */
		uint32_t delay_ctrl;		/* Value of delay ctrl reg */
		uint32_t latch_lat;		/* Value of latch latency reg */
		uint32_t sample_delay;		/* Value of sample delay reg */
		uint32_t driving;		/* Value of driving reg */
		uint32_t reserved;
		uint32_t bl0_start;		/* Bootloader start addr */
		uint32_t bl0_end;		/* Bootloader end addr */
		uint32_t bl0_type;		/* Bootloader type */
		uint8_t bl_reserve[84];
		uint8_t ecc_parity[42];		/* ECC parity of this header */
	};

	uint8_t data[0xEA];
};

/* SPIM BROM Header for SPI-NAND */
union spim_nand_boot_header {
	struct {
		char id[8];
		uint32_t version;		/* Header version */
		uint32_t config;		/* Header config */
		uint32_t page_size;		/* NAND page size */
		uint32_t spare_size;		/* NAND page spare size */
		uint16_t page_per_block;	/* Pages of one block */
		uint16_t plane_sel_position;	/* Plane bit position */
		uint16_t reserve_reg;
		uint16_t reserve_val;
		uint16_t ecc_error;		/* ECC error reg addr */
		uint16_t ecc_mask;		/* ECC error bit mask */
		uint32_t bl_start;		/* Bootloader start addr */
		uint32_t bl_end;		/* Bootloader end addr */
		uint8_t ecc_parity[32];		/* ECC parity of this header */
		uint32_t integrity_crc;		/* CRC of this header */
	};

	uint8_t data[0x50];
};

enum nand_boot_header_type {
	NAND_BOOT_AP_HEADER,
	NAND_BOOT_HSM_HEADER,
	NAND_BOOT_HSM20_HEADER,
	NAND_BOOT_SPIM_HEADER
};

#define NAND_BOOT_NAME		"BOOTLOADER!"
#define NAND_BOOT_VERSION	"V006"
#define NAND_BOOT_ID		"NFIINFO"

#define HSM_NAND_BOOT_NAME	"NANDCFG!"
#define SPIM_NAND_BOOT_NAME	"SPINAND!"

/* Find nand header data by name */
const struct nand_header_type *mtk_nand_header_find(const char *name);

/* Device header size using this nand header */
uint32_t mtk_nand_header_size(const struct nand_header_type *hdr_nand);

/* Get nand info from nand header (page size, spare size, ...) */
int mtk_nand_header_info(const void *ptr, struct nand_header_info *info);

/* Whether given header data is valid */
bool is_mtk_nand_header(const void *ptr);

/* Generate Device header using give nand header */
uint32_t mtk_nand_header_put(const struct nand_header_type *hdr_nand,
			     void *ptr);

#endif /* _MTK_NAND_HEADERS_H */
