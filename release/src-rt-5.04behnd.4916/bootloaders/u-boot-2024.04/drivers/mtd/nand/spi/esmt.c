// SPDX-License-Identifier: GPL-2.0
/*
 * Author:
 *	Chuanhong Guo <gch981213@gmail.com> - the main driver logic
 *	Martin Kurbanov <mmkurbanov@sberdevices.ru> - OOB layout
 */

#ifndef __UBOOT__
#include <linux/device.h>
#include <linux/kernel.h>
#endif
#include <linux/mtd/spinand.h>

/* ESMT uses GigaDevice 0xc8 JECDEC ID on some SPI NANDs */
#define SPINAND_MFR_ESMT_C8		0xc8
#define ESMT_STATUS_ECC_MASK		GENMASK(7, 4)
#define ESMT_STATUS_ECC_NO_BITFLIPS	0x00
#define ESMT_STATUS_ECC_1TO3_BITFLIPS	0x10
#define ESMT_STATUS_ECC_4TO6_BITFLIPS	0x30
#define ESMT_STATUS_ECC_7TO8_BITFLIPS	0x50

static SPINAND_OP_VARIANTS(read_cache_variants,
			   SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
			   SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
			   SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
			   SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(x1_read_cache_variants,
			   SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
			   SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
			   SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
			   SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
			   SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
			   SPINAND_PROG_LOAD(false, 0, NULL, 0));

/*
 * OOB spare area map (64 bytes)
 *
 * Bad Block Markers
 * filled by HW and kernel                 Reserved
 *   |                 +-----------------------+-----------------------+
 *   |                 |                       |                       |
 *   |                 |    OOB free data Area |non ECC protected      |
 *   |   +-------------|-----+-----------------|-----+-----------------|-----+
 *   |   |             |     |                 |     |                 |     |
 * +-|---|----------+--|-----|--------------+--|-----|--------------+--|-----|--------------+
 * | |   | section0 |  |     |    section1  |  |     |    section2  |  |     |    section3  |
 * +-v-+-v-+---+----+--v--+--v--+-----+-----+--v--+--v--+-----+-----+--v--+--v--+-----+-----+
 * |   |   |   |    |     |     |     |     |     |     |     |     |     |     |     |     |
 * |0:1|2:3|4:7|8:15|16:17|18:19|20:23|24:31|32:33|34:35|36:39|40:47|48:49|50:51|52:55|56:63|
 * |   |   |   |    |     |     |     |     |     |     |     |     |     |     |     |     |
 * +---+---+-^-+--^-+-----+-----+--^--+--^--+-----+-----+--^--+--^--+-----+-----+--^--+--^--+
 *           |    |                |     |                 |     |                 |     |
 *           |    +----------------|-----+-----------------|-----+-----------------|-----+
 *           |             ECC Area|(Main + Spare) - filled|by ESMT NAND HW        |
 *           |                     |                       |                       |
 *           +---------------------+-----------------------+-----------------------+
 *                         OOB ECC protected Area - not used due to
 *                         partial programming from some filesystems
 *                             (like JFFS2 with cleanmarkers)
 */

#define ESMT_OOB_SECTION_COUNT			4
#define ESMT_OOB_SECTION_SIZE(nand) \
	(nanddev_per_page_oobsize(nand) / ESMT_OOB_SECTION_COUNT)
#define ESMT_OOB_FREE_SIZE(nand) \
	(ESMT_OOB_SECTION_SIZE(nand) / 2)
#define ESMT_OOB_ECC_SIZE(nand) \
	(ESMT_OOB_SECTION_SIZE(nand) - ESMT_OOB_FREE_SIZE(nand))
#define ESMT_OOB_BBM_SIZE			2

static int f50l1g41lb_ooblayout_ecc(struct mtd_info *mtd, int section,
				    struct mtd_oob_region *region)
{
	struct nand_device *nand = mtd_to_nanddev(mtd);

	if (section >= ESMT_OOB_SECTION_COUNT)
		return -ERANGE;

	region->offset = section * ESMT_OOB_SECTION_SIZE(nand) +
			 ESMT_OOB_FREE_SIZE(nand);
	region->length = ESMT_OOB_ECC_SIZE(nand);

	return 0;
}

static int f50l1g41lb_ooblayout_free(struct mtd_info *mtd, int section,
				     struct mtd_oob_region *region)
{
	struct nand_device *nand = mtd_to_nanddev(mtd);

	if (section >= ESMT_OOB_SECTION_COUNT)
		return -ERANGE;

	/*
	 * Reserve space for bad blocks markers (section0) and
	 * reserved bytes (sections 1-3)
	 */
	region->offset = section * ESMT_OOB_SECTION_SIZE(nand) + 2;

	/* Use only 2 non-protected ECC bytes per each OOB section */
	region->length = 2;

	return 0;
}

static const struct mtd_ooblayout_ops f50l1g41lb_ooblayout = {
	.ecc = f50l1g41lb_ooblayout_ecc,
	.rfree = f50l1g41lb_ooblayout_free,
};

static int esmt_ooblayout_ecc(struct mtd_info *mtd, int section,
			      struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = 16 * section;
	region->length = 8;

	if (section) {
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

	if (!section) {
		region->offset = 0;
		region->length = 2;
	} else {
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
	.rfree = esmt_ooblayout_free,
};

static const struct mtd_ooblayout_ops esmt2_ooblayout = {
	.ecc = esmt2_ooblayout_ecc,
	.rfree = esmt2_ooblayout_free,
};

static const struct mtd_ooblayout_ops esmt3_ooblayout = {
	.ecc = esmt3_ooblayout_ecc,
	.rfree = esmt3_ooblayout_free,
};

static int select_target(struct spinand_device *spinand,
			 unsigned int target)
{
	struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(0xc2, 1),
					  SPI_MEM_OP_NO_ADDR,
					  SPI_MEM_OP_NO_DUMMY,
					  SPI_MEM_OP_DATA_OUT(1, spinand->scratchbuf, 1));

	*spinand->scratchbuf = target;
	return spi_mem_exec_op(spinand->slave, &op);
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

static const struct spinand_info esmt_c8_spinand_table[] = {
	SPINAND_INFO("F50L1G41A",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x21),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&esmt_ooblayout, NULL)),
	SPINAND_INFO("F50L1G41LB",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x01),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&x1_read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&f50l1g41lb_ooblayout, NULL)),
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
	SPINAND_INFO("F50D1G41LB",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x11),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&f50l1g41lb_ooblayout, NULL)),
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

static const struct spinand_manufacturer_ops esmt_spinand_manuf_ops = {
};

const struct spinand_manufacturer esmt_c8_spinand_manufacturer = {
	.id = SPINAND_MFR_ESMT_C8,
	.name = "ESMT",
	.chips = esmt_c8_spinand_table,
	.nchips = ARRAY_SIZE(esmt_c8_spinand_table),
	.ops = &esmt_spinand_manuf_ops,
};
