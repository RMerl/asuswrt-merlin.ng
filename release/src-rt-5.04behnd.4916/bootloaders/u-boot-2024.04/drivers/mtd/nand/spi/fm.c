// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022 Broadcom
 *
 * Author:
 *	David Regan
 */

#ifndef __UBOOT__
#include <linux/device.h>
#include <linux/kernel.h>
#endif
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_FM		0xA1

static SPINAND_OP_VARIANTS(read_cache_variants,
		// 0xeb, quad lane command and data transfer:
		SPINAND_PAGE_READ_FROM_CACHE_QUADIO_OP(0, 2, NULL, 0),
		// 0x6b, quad lane data transfer:
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		// 0xbb, dual lane command and data transfer:
		SPINAND_PAGE_READ_FROM_CACHE_DUALIO_OP(0, 1, NULL, 0),
		// 0x3b, dual lane data transfer:
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		// 0xb:
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		// 0x3:
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int fm25s01_ooblayout_ecc(struct mtd_info *mtd, int section,
				 struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 64;
	region->length = 64;

	return 0;
};

static int fm25s01_ooblayout_free(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 2 bytes for the BBM. */
	region->offset = 2;
	region->length = 62;

	return 0;
};

static const struct mtd_ooblayout_ops fm25s01_ooblayout = {
	.ecc = fm25s01_ooblayout_ecc,
	.rfree	 = fm25s01_ooblayout_free,
};

static const struct spinand_info fm_spinand_table[] = {
	SPINAND_INFO("FM25S01",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xa1),
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fm25s01_ooblayout, NULL)),
	SPINAND_INFO("FM25S01A",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xe4),
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fm25s01_ooblayout, NULL)),
};

static const struct spinand_manufacturer_ops spinand_manuf_ops = {
	.init = NULL,
	.cleanup = NULL,
};

const struct spinand_manufacturer fm_spinand_manufacturer = {
	.id = SPINAND_MFR_FM,
	.name = "FM",
	.chips = fm_spinand_table,
	.nchips = ARRAY_SIZE(fm_spinand_table),
	.ops = &spinand_manuf_ops,
};
