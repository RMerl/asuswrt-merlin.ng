#ifdef CONFIG_BCM_KF_MTD_BCMSPINAND
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

#define SPINAND_MFR_ETRON		0xD5

#define STATUS_ECC_LIMIT_BITFLIPS (3 << 4)

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_QUADIO_OP(0, 2, NULL, 0), // 0xeb, quad lane command and data transfer
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0), // 0x6b, quad lane data transfer
		SPINAND_PAGE_READ_FROM_CACHE_DUALIO_OP(0, 1, NULL, 0), // 0xbb, dual lane command and data transfer
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0), // 0x3b, dual lane data transfer
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0), // 0xb
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0)); // 0x3

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

// Probably shouldn't support Etron 8 bit ECC devices, the problem is they only have 4 status states when reading a page,
// no bits flipped, some bits flipped, max bits flipped and too mand bits flipped (ECC could not correct)
// so for example if a single bit error then the maximum bit errors for that status
// which is 7 (because it's not known the exact bit error amount and the worst must be reported.) This will report
// 7 bit flips (out of 8 maximum bit flips) and most likely trigger the FS to clean the block and move the data. With too
// many single bit flips the SPI NAND will prematurely wear out with all the filesystem block data moves.

static int etron_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = mtd->oobsize / 2;
	region->length = mtd->oobsize / 2;

	return 0;
};

static int etron_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 1 bytes for the BBM. */
	region->offset = 1;
	region->length = (mtd->oobsize / 2) - 1;

	return 0;
};

static const struct mtd_ooblayout_ops etron_ooblayout = {
	.ecc = etron_ooblayout_ecc,
	.free = etron_ooblayout_free,
};

static int etron_ecc_get_status(struct spinand_device *spinand,
				   u8 status)
{
	switch (status & STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	case STATUS_ECC_HAS_BITFLIPS:
		return 3;

	case STATUS_ECC_LIMIT_BITFLIPS:
		return 4;


	default:
		break;
	}

	return -EINVAL;
};

static const struct spinand_info etron_spinand_table[] = {
	/* EM73C 1Gb 3.3V */
	SPINAND_INFO("EM73C044VCF",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0x25),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&etron_ooblayout, etron_ecc_get_status)),
};

static const struct spinand_manufacturer_ops spinand_manuf_ops = {
	.init = NULL,
	.cleanup = NULL,
};

const struct spinand_manufacturer etron_spinand_manufacturer = {
	.id = SPINAND_MFR_ETRON,
	.name = "Etron",
	.chips = etron_spinand_table,
	.nchips = ARRAY_SIZE(etron_spinand_table),
	.ops = &spinand_manuf_ops,
};
#endif
