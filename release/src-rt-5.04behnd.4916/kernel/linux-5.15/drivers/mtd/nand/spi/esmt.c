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

#define SPINAND_MFR_ESMT		0xC8

#define ESMT_STATUS_ECC_MASK		GENMASK(7, 4)
#define ESMT_STATUS_ECC_NO_BITFLIPS	(0 << 4)
#define ESMT_STATUS_ECC_1TO3_BITFLIPS	(1 << 4)
#define ESMT_STATUS_ECC_4TO6_BITFLIPS	(3 << 4)
#define ESMT_STATUS_ECC_7TO8_BITFLIPS	(5 << 4)

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_QUADIO_OP(0, 2, NULL, 0), // 0xeb, quad lane command and data transfer
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0), // 0x6b, quad lane data transfer
		SPINAND_PAGE_READ_FROM_CACHE_DUALIO_OP(0, 1, NULL, 0), // 0xbb, dual lane command and data transfer
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0), // 0x3b, dual lane data transfer
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0), // 0xb
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0)); // 0x3

static SPINAND_OP_VARIANTS(x1_read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int esmt_ooblayout_ecc(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = 16 * section;
	region->length = 8;

	if (section)
	{
		region->offset++;
		region->length--;
	}

	return 0;
};

static int esmt_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = 8 + (16 * section);
	region->length = 9;

	if (section == 3)
		region->length--;

	return 0;
};

static int esmt2_ooblayout_ecc(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 4)
		return -ERANGE;

	if (!section)
	{
		region->offset = 0;
		region->length = 2;
	}
	else
	{
		region->offset = 8 + (16 * (section - 1));
		region->length = 10;

		if (section == 4)
			region->length -= 2;
	}

	return 0;
};

static int esmt2_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = 2 + (16 * section);
	region->length = 6;

	return 0;
};

static int esmt3_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 64;
	region->length = 64;

	return 0;
}

static int esmt3_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	// Reserve 2 bytes for the BBM.
	region->offset = 2;
	region->length = 62;

	return 0;
}

static const struct mtd_ooblayout_ops esmt_ooblayout = {
	.ecc = esmt_ooblayout_ecc,
	.free = esmt_ooblayout_free,
};

static const struct mtd_ooblayout_ops esmt2_ooblayout = {
	.ecc = esmt2_ooblayout_ecc,
	.free = esmt2_ooblayout_free,
};

static const struct mtd_ooblayout_ops esmt3_ooblayout =
{
	.ecc = esmt3_ooblayout_ecc,
	.free = esmt3_ooblayout_free,
};

static int select_target(struct spinand_device *spinand,
				  unsigned int target)
{
	struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(0xc2, 1),
					  SPI_MEM_OP_NO_ADDR,
					  SPI_MEM_OP_NO_DUMMY,
					  SPI_MEM_OP_DATA_OUT(1,
							spinand->scratchbuf,
							1));

	*spinand->scratchbuf = target;
	return spi_mem_exec_op(spinand->spimem, &op);
};

static int esmt_ecc_get_status(struct spinand_device *spinand,
					 u8 status)
{
	switch (status & ESMT_STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	case ESMT_STATUS_ECC_1TO3_BITFLIPS:
		return 3;

	case ESMT_STATUS_ECC_4TO6_BITFLIPS:
		return 6;

	case ESMT_STATUS_ECC_7TO8_BITFLIPS:
		return 8;

	default:
		break;
	}

	return -EINVAL;
};

static const struct spinand_info esmt_spinand_table[] = {
	SPINAND_INFO("F50L1G41A",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0x21),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&esmt_ooblayout, NULL)),
	SPINAND_INFO("F50L1G41LB",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0x01),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&x1_read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&esmt2_ooblayout, NULL)),
	SPINAND_INFO("F50L2G41LB",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0x0a),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 2),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&esmt2_ooblayout, NULL),
		     SPINAND_SELECT_TARGET(select_target)),
	SPINAND_INFO("F50L2G41KA",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0x41),
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&esmt3_ooblayout, esmt_ecc_get_status),
		     SPINAND_SELECT_TARGET(select_target)),
};

static const struct spinand_manufacturer_ops spinand_manuf_ops = {
	.init = NULL,
	.cleanup = NULL,
};

const struct spinand_manufacturer esmt_spinand_manufacturer = {
	.id = SPINAND_MFR_ESMT,
	.name = "ESMT",
	.chips = esmt_spinand_table,
	.nchips = ARRAY_SIZE(esmt_spinand_table),
	.ops = &spinand_manuf_ops,
};
#endif
