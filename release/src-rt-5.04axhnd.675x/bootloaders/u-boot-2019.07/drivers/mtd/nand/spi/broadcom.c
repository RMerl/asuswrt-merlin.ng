// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Broadcom
 *
 * Author:
 *	David Regan
 */

#ifndef __UBOOT__
#include <linux/device.h>
#include <linux/kernel.h>
#endif
#include <linux/mtd/spinand.h>


#define FEATURE_STAT_ADDR   0xC0


static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_QUADIO_OP(0, 2, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_DUALIO_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));


///////////////////////////////////// toshiba device data /////////////////////////////////////

#define SPINAND_MFR_TOSHIBA		0x98
#define TOSHIBA_CFG_BUF_READ		BIT(3)
#define TOSHIBA_FEATURE_STAT_ENH	0x30
#define TOSHIBA_ENH_STAT_MASK		0xF0
#define TOSHIBA_ENH_STAT_SHIFT		4

static int toshibamicron_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 64;
	region->length = 64;

	return 0;
}

static int toshibamicron_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	// Reserve 2 bytes for the BBM.
	region->offset = 2;
	region->length = 62;

	return 0;
}

static int toshiba_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 128;
	region->length = 128;

	return 0;
}

static int toshiba_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	// Reserve 2 bytes for the BBM.
	region->offset = 2;
	region->length = 126;

	return 0;
}



static const struct mtd_ooblayout_ops toshibamicron_ooblayout =
{
	.ecc = toshibamicron_ooblayout_ecc,
	.free = toshibamicron_ooblayout_free,
};


static const struct mtd_ooblayout_ops toshiba_ooblayout =
{
	.ecc = toshiba_ooblayout_ecc,
	.free = toshiba_ooblayout_free,
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
	return spi_mem_exec_op(spinand->slave, &op);
}

static int toshiba_ecc_get_status(struct spinand_device *spinand, u8 status)
{
	u8 status2;
	struct spi_mem_op op = SPINAND_GET_FEATURE_OP(TOSHIBA_FEATURE_STAT_ENH, &status2);
	int ret;

	switch (status & STATUS_ECC_MASK)
	{
		case STATUS_ECC_NO_BITFLIPS:
			return 0;

		case STATUS_ECC_UNCOR_ERROR:
			return -EBADMSG; // 74

		default:
		{ // default to correctible errors since Toshiba uses the remaining two states as correctible
			ret = spi_mem_exec_op(spinand->slave, &op);
			if (!ret)
				ret = ((status2 & TOSHIBA_ENH_STAT_MASK) >> TOSHIBA_ENH_STAT_SHIFT);

			return(ret);
		}
	}

	return -EINVAL; // 22
}

static const struct spinand_info toshiba_spinand_table[] =
{
	SPINAND_INFO("TC58CVG0S", 0xC2,
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshibamicron_ooblayout, toshiba_ecc_get_status),
		     SPINAND_SELECT_TARGET(select_target)),

	SPINAND_INFO("TC58CVG1S", 0xCB,
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshibamicron_ooblayout, toshiba_ecc_get_status),
		     SPINAND_SELECT_TARGET(select_target)),

	SPINAND_INFO("TC58CVG1S0HRAIJ", 0xEB,
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshibamicron_ooblayout, toshiba_ecc_get_status),
		     SPINAND_SELECT_TARGET(select_target)),

	SPINAND_INFO("TC58CVG2S", 0xCD,
		     NAND_MEMORG(1, 4096, 256, 64, 2048, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshiba_ooblayout, toshiba_ecc_get_status),
		     SPINAND_SELECT_TARGET(select_target)),

	SPINAND_INFO("TC58CVG2S0HRAIJ", 0xED,
		     NAND_MEMORG(1, 4096, 256, 64, 2048, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshiba_ooblayout, toshiba_ecc_get_status),
		     SPINAND_SELECT_TARGET(select_target)),

	SPINAND_INFO("TC58CVG0S3", 0xE2,
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshibamicron_ooblayout, toshiba_ecc_get_status),
		     SPINAND_SELECT_TARGET(select_target))
};


///////////////////////////////////// micron device data /////////////////////////////////////

#define SPINAND_MFR_MICRON		0x2C

#define MICRON_STATUS_ECC_MASK		GENMASK(7, 4)
#define MICRON_STATUS_ECC_NO_BITFLIPS	(0 << 4)
#define MICRON_STATUS_ECC_1TO3_BITFLIPS	(1 << 4)
#define MICRON_STATUS_ECC_4TO6_BITFLIPS	(3 << 4)
#define MICRON_STATUS_ECC_7TO8_BITFLIPS	(5 << 4)

static int micron_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = (16 * section) + 8;
	region->length = 8;

	return 0;
}

static int micron_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	if (!section)
	{
		region->offset = 2;
		region->length = 6;
	}
	else
	{
		region->offset = 16 * section;
		region->length = 8;
	}

	return 0;
}

static int micron_ecc_get_status(struct spinand_device *spinand,
					 u8 status)
{
	switch (status & MICRON_STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	case MICRON_STATUS_ECC_1TO3_BITFLIPS:
		return 3;

	case MICRON_STATUS_ECC_4TO6_BITFLIPS:
		return 6;

	case MICRON_STATUS_ECC_7TO8_BITFLIPS:
		return 8;

	default:
		break;
	}

	return -EINVAL;
}

static const struct mtd_ooblayout_ops micron_ooblayout =
{
	.ecc = micron_ooblayout_ecc,
	.free = micron_ooblayout_free,
};

static const struct spinand_info micron_spinand_table[] =
{
	SPINAND_INFO("MT29F1G01AA", 0x12,
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&micron_ooblayout,
				     micron_ecc_get_status)),

	SPINAND_INFO("MT29F2G01AA", 0x22,
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1), // bits_per_cell, pagesize, oobsize, pages_per_eraseblock, eraseblocks_per_lun, planes_per_lun, luns_per_target, ntargets	
		     NAND_ECCREQ(8, 512), // strength, step_size (ecc per how many page bytes)
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&micron_ooblayout,
				     micron_ecc_get_status)),

	SPINAND_INFO("MT29F4G01AA", 0x32,
		     NAND_MEMORG(1, 2048, 128, 64, 4096, 1, 1, 1), // bits_per_cell, pagesize, oobsize, pages_per_eraseblock, eraseblocks_per_lun, planes_per_lun, luns_per_target, ntargets	
		     NAND_ECCREQ(8, 512), // strength, step_size (ecc per how many page bytes)
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&micron_ooblayout,
				     micron_ecc_get_status)),

	SPINAND_INFO("MT29F1G01A", 0x14,
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshibamicron_ooblayout,
				     micron_ecc_get_status)),

	SPINAND_INFO("MT29F2G01AB", 0x24,
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 2, 1, 1), // bits_per_cell, pagesize, oobsize, pages_per_eraseblock, eraseblocks_per_lun, planes_per_lun, luns_per_target, ntargets	
		     NAND_ECCREQ(8, 512), // strength, step_size (ecc per how many page bytes)
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshibamicron_ooblayout,
				     micron_ecc_get_status)),

	SPINAND_INFO("MT29F4G01AB", 0x34,
		     NAND_MEMORG(1, 4096, 128, 64, 2048, 1, 1, 1), // bits_per_cell, pagesize, oobsize, pages_per_eraseblock, eraseblocks_per_lun, planes_per_lun, luns_per_target, ntargets	
		     NAND_ECCREQ(8, 512), // strength, step_size (ecc per how many page bytes)
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshibamicron_ooblayout,
				     micron_ecc_get_status)),

	SPINAND_INFO("MT29F4G01AD", 0x36,
		     NAND_MEMORG(1, 2048, 128, 64, 4096, 1, 1, 1), // bits_per_cell, pagesize, oobsize, pages_per_eraseblock, eraseblocks_per_lun, planes_per_lun, luns_per_target, ntargets	
		     NAND_ECCREQ(8, 512), // strength, step_size (ecc per how many page bytes)
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshibamicron_ooblayout,
				     micron_ecc_get_status)),
};

///////////////////////////////////// winbond device data /////////////////////////////////////

#define SPINAND_MFR_WINBOND		0xEF

#define WINBOND_CFG_BUF_READ		BIT(3)

static int w25m02gv_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = (16 * section) + 8;
	region->length = 8;

	return 0;
}

static int w25m02gv_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = (16 * section) + 2;
	region->length = 6;

	return 0;
}

static const struct mtd_ooblayout_ops w25m02gv_ooblayout =
{
	.ecc = w25m02gv_ooblayout_ecc,
	.free = w25m02gv_ooblayout_free,
};

static const struct spinand_info winbond_spinand_AA_table[] =
{ // Winbond uses two bytes to identify the device ID, first byte after manufacturer ID is 0xAA or 0xAB
	SPINAND_INFO("W25N512GV", 0x20,
		     NAND_MEMORG(1, 2048, 64, 64, 512, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&w25m02gv_ooblayout, NULL)),
	SPINAND_INFO("W25N01GV", 0x21,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&w25m02gv_ooblayout, NULL)),
	SPINAND_INFO("W25N02GV", 0x22,
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&w25m02gv_ooblayout, NULL),
		     SPINAND_SELECT_TARGET(select_target)),
	SPINAND_INFO("W25N04KV", 0x23,
		     NAND_MEMORG(1, 2048, 128, 64, 4096, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshiba_ooblayout, toshiba_ecc_get_status)),
};

static const struct spinand_info winbond_spinand_AB_table[] =
{ // Winbond uses two bytes to identify the device ID, first byte after manufacturer ID is 0xAA or 0xAB
	SPINAND_INFO("W25M02GV", 0x21,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 2),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&w25m02gv_ooblayout, NULL),
		     SPINAND_SELECT_TARGET(select_target)),
};

///////////////////////////////////// gigadevice device data /////////////////////////////////////

#define SPINAND_MFR_GIGADEVICE_ESMT		0xC8
#define GD5FXGQ4XA_STATUS_ECC_1_7_BITFLIPS	(1 << 4)
#define GD5FXGQ4XA_STATUS_ECC_8_BITFLIPS	(3 << 4)

#define GD5FXGQ4XEXXG_REG_STATUS2		0xf0


static int gd5fxgq4xa_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = (16 * section) + 8;
	region->length = 8;

	return 0;
}

static int gd5fxgq4xa_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	if (section) {
		region->offset = 16 * section;
		region->length = 8;
	} else {
		/* section 0 has one byte reserved for bad block mark */
		region->offset = 1;
		region->length = 7;
	}
	return 0;
}

static int gd5fxgq4xa_ecc_get_status(struct spinand_device *spinand,
					 u8 status)
{
	switch (status & STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case GD5FXGQ4XA_STATUS_ECC_1_7_BITFLIPS:
		/* 1-7 bits are flipped. return the maximum. */
		return 7;

	case GD5FXGQ4XA_STATUS_ECC_8_BITFLIPS:
		return 8;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	default:
		break;
	}

	return -EINVAL;
}

static int gd5fxgq4xexxg_ooblayout_ecc(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 64;
	region->length = 64;

	return 0;
}

static int gd5fxgq4xexxg_ooblayout_free(struct mtd_info *mtd, int section,
					struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 2 bytes for the BBM. */
	region->offset = 2;
	region->length = 62;

	return 0;
}

static int gd5fxgq4xexxg2_ooblayout_ecc(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 128;
	region->length = 128;

	return 0;
}

static int gd5fxgq4xexxg2_ooblayout_free(struct mtd_info *mtd, int section,
					struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 2 bytes for the BBM. */
	region->offset = 2;
	region->length = 126;

	return 0;
}

static int gd5fxgq4xexxg_ecc_get_status(struct spinand_device *spinand,
					u8 status)
{
	u8 status2;
	struct spi_mem_op op = SPINAND_GET_FEATURE_OP(GD5FXGQ4XEXXG_REG_STATUS2,
						      &status2);
	int ret;

	switch (status & STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case GD5FXGQ4XA_STATUS_ECC_1_7_BITFLIPS:
		/*
		 * Read status2 register to determine a more fine grained
		 * bit error status
		 */
		ret = spi_mem_exec_op(spinand->slave, &op);
		if (ret)
			return ret;

		/*
		 * 4 ... 7 bits are flipped (1..4 can't be detected, so
		 * report the maximum of 4 in this case
		 */
		/* bits sorted this way (3...0): ECCS1,ECCS0,ECCSE1,ECCSE0 */
		return ((status & STATUS_ECC_MASK) >> 2) |
			((status2 & STATUS_ECC_MASK) >> 4);

	case GD5FXGQ4XA_STATUS_ECC_8_BITFLIPS:
		return 8;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	default:
		break;
	}

	return -EINVAL;
}

static int gd5fxgq5_ecc_get_status(struct spinand_device *spinand,
					u8 status)
{
	u8 status2;
	struct spi_mem_op op = SPINAND_GET_FEATURE_OP(GD5FXGQ4XEXXG_REG_STATUS2,
						      &status2);
	int ret;

	switch (status & STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case GD5FXGQ4XA_STATUS_ECC_1_7_BITFLIPS:
		/*
		 * Read status2 register to determine a more fine grained
		 * bit error status
		 */
		ret = spi_mem_exec_op(spinand->slave, &op);
		if (ret)
			return ret;

		/*
		 * 1 ... 4 bits are flipped
		 */
		/* bits sorted this way (4..1): ECCSE1,ECCSE0 */
		return ( ((status2 & STATUS_ECC_MASK) >> 4) + 1);

	case GD5FXGQ4XA_STATUS_ECC_8_BITFLIPS:
	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	default:
		break;
	}

	return -EINVAL;
}

static const struct mtd_ooblayout_ops gd5fxgq4xa_ooblayout =
{
	.ecc = gd5fxgq4xa_ooblayout_ecc,
	.free = gd5fxgq4xa_ooblayout_free,
};

static const struct mtd_ooblayout_ops gd5fxgq4xexxg_ooblayout =
{
	.ecc = gd5fxgq4xexxg_ooblayout_ecc,
	.free = gd5fxgq4xexxg_ooblayout_free,
};

static const struct mtd_ooblayout_ops gd5fxgq4xexxg2_ooblayout =
{
	.ecc = gd5fxgq4xexxg2_ooblayout_ecc,
	.free = gd5fxgq4xexxg2_ooblayout_free,
};

static const struct spinand_info gigadevice_spinand_table[] =
{
	SPINAND_INFO("GD5F1GQ4xA", 0xF1,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&gd5fxgq4xa_ooblayout,
				     gd5fxgq4xa_ecc_get_status)),
	SPINAND_INFO("GD5F2GQ4xA", 0xF2,
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&gd5fxgq4xa_ooblayout,
				     gd5fxgq4xa_ecc_get_status)),
	SPINAND_INFO("GD5F4GQ4xA", 0xF4,
		     NAND_MEMORG(1, 2048, 64, 64, 4096, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&gd5fxgq4xa_ooblayout,
				     gd5fxgq4xa_ecc_get_status)),

	SPINAND_INFO("GD5F1GQ4UExxG", 0xd1,
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1), // bits_per_cell, pagesize, oobsize, pages_per_eraseblock, eraseblocks_per_lun, planes_per_lun, luns_per_target, ntargets	
		     NAND_ECCREQ(8, 512), // strength, step_size (ecc per how many page bytes)
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&gd5fxgq4xexxg_ooblayout,
				     gd5fxgq4xexxg_ecc_get_status)),

	SPINAND_INFO("GD5F2GQ4UExxG", 0xd2,
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&gd5fxgq4xexxg_ooblayout,
				     gd5fxgq4xexxg_ecc_get_status)),

	SPINAND_INFO("GD5F4GQ4UExxG", 0xd4,
		     NAND_MEMORG(1, 4096, 256, 64, 2048, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&gd5fxgq4xexxg2_ooblayout,
				     gd5fxgq4xexxg_ecc_get_status)),

	SPINAND_INFO("GD5F4GQ6UExxG", 0x55,
		     NAND_MEMORG(1, 2048, 128, 64, 4096, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&gd5fxgq4xexxg_ooblayout,
				     gd5fxgq5_ecc_get_status)),

	SPINAND_INFO("GD5F1GQ5UExxG", 0x51,
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&gd5fxgq4xexxg_ooblayout,
				     gd5fxgq5_ecc_get_status)),
};

///////////////////////////////////// macronix device data /////////////////////////////////////

#define SPINAND_MFR_MACRONIX		0xC2
#define MACRONIX_ECCSR_MASK		0x0F

static int mx35lfxge4ab_ooblayout_ecc(struct mtd_info *mtd, int section,
				      struct mtd_oob_region *region)
{
	return -ERANGE;
}

static int mx35lfxge4ab_ooblayout_free(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 2;
	region->length = mtd->oobsize - 2;

	return 0;
}

static const struct mtd_ooblayout_ops mx35lfxge4ab_ooblayout = {
	.ecc = mx35lfxge4ab_ooblayout_ecc,
	.free = mx35lfxge4ab_ooblayout_free,
};

static int mx35lf1ge4ab_get_eccsr(struct spinand_device *spinand, u8 *eccsr)
{
	struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(0x7c, 1),
					  SPI_MEM_OP_NO_ADDR,
					  SPI_MEM_OP_DUMMY(1, 1),
					  SPI_MEM_OP_DATA_IN(1, eccsr, 1));

	int ret = spi_mem_exec_op(spinand->slave, &op);
	if (ret)
		return ret;

	*eccsr &= MACRONIX_ECCSR_MASK;
	return 0;
}

static int mx35lf1ge4ab_ecc_get_status(struct spinand_device *spinand,
				       u8 status)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	u8 eccsr;

	switch (status & STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	case STATUS_ECC_HAS_BITFLIPS:
		/*
		 * Let's try to retrieve the real maximum number of bitflips
		 * in order to avoid forcing the wear-leveling layer to move
		 * data around if it's not necessary.
		 */
		if (mx35lf1ge4ab_get_eccsr(spinand, &eccsr))
			return nand->eccreq.strength;

		if (WARN_ON(eccsr > nand->eccreq.strength || !eccsr))
			return nand->eccreq.strength;

		return eccsr;

	default:
		break;
	}

	return -EINVAL;
}

static const struct spinand_info macronix_spinand_table[] = {
	SPINAND_INFO("MX35LF1GE4AB", 0x12,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&mx35lfxge4ab_ooblayout,
				     mx35lf1ge4ab_ecc_get_status)),
	SPINAND_INFO("MX35LF2GE4AB", 0x22,
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 2, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&mx35lfxge4ab_ooblayout,
				     mx35lf1ge4ab_ecc_get_status)),
	SPINAND_INFO("MX35LF2GE4AD", 0x26,
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1), // bits_per_cell, pagesize, oobsize, pages_per_eraseblock, eraseblocks_per_lun, planes_per_lun, luns_per_target, ntargets
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&toshibamicron_ooblayout,
				     mx35lf1ge4ab_ecc_get_status)),
	SPINAND_INFO("MX35LF4GE4AD", 0x37,
		     NAND_MEMORG(1, 4096, 256, 64, 2048, 1, 1, 1), // bits_per_cell, pagesize, oobsize, pages_per_eraseblock, eraseblocks_per_lun, planes_per_lun, luns_per_target, ntargets
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&toshiba_ooblayout,
				     mx35lf1ge4ab_ecc_get_status)),
};

///////////////////////////////////// paragon device data /////////////////////////////////////

#define SPINAND_MFR_PARAGON_FM	0xa1

#define PN26G0XA_STATUS_ECC_BITMASK		(3 << 4)

#define PN26G0XA_STATUS_ECC_NONE_DETECTED	(0 << 4)
#define PN26G0XA_STATUS_ECC_1_7_CORRECTED	(1 << 4)
#define PN26G0XA_STATUS_ECC_ERRORED		(2 << 4)
#define PN26G0XA_STATUS_ECC_8_CORRECTED		(3 << 4)

static int pn26g0xa_ooblayout_ecc(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = 6 + (15 * section); /* 4 BBM + 2 user bytes */
	region->length = 13;

	return 0;
}

static int pn26g0xa_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 4)
		return -ERANGE;

	if (section == 4) {
		region->offset = 64;
		region->length = 64;
	} else {
		region->offset = 4 + (15 * section);
		region->length = 2;
	}

	return 0;
}

static int pn26g0xa_ecc_get_status(struct spinand_device *spinand,
				   u8 status)
{
	switch (status & PN26G0XA_STATUS_ECC_BITMASK) {
	case PN26G0XA_STATUS_ECC_NONE_DETECTED:
		return 0;

	case PN26G0XA_STATUS_ECC_1_7_CORRECTED:
		return 7;	/* Return upper limit by convention */

	case PN26G0XA_STATUS_ECC_8_CORRECTED:
		return 8;

	case PN26G0XA_STATUS_ECC_ERRORED:
		return -EBADMSG;

	default:
		break;
	}

	return -EINVAL;
}

static const struct mtd_ooblayout_ops pn26g0xa_ooblayout = {
	.ecc = pn26g0xa_ooblayout_ecc,
	.free = pn26g0xa_ooblayout_free,
};


static const struct spinand_info paragon_spinand_table[] = {
	SPINAND_INFO("PN26G01A", 0xe1,
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&pn26g0xa_ooblayout,
				     pn26g0xa_ecc_get_status)),
	SPINAND_INFO("PN26G02A", 0xe2,
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 2, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&pn26g0xa_ooblayout,
				     pn26g0xa_ecc_get_status)),
};

///////////////////////////////////// fm device data /////////////////////////////////////

static int fm25s01_ooblayout_ecc(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 64;
	region->length = 64;

	return 0;
}

static int fm25s01_ooblayout_free(struct mtd_info *mtd, int section,
					struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 2 bytes for the BBM. */
	region->offset = 2;
	region->length = 62;

	return 0;
}

static const struct mtd_ooblayout_ops fm25s01_ooblayout = {
	.ecc = fm25s01_ooblayout_ecc,
	.free = fm25s01_ooblayout_free,
};

static const struct spinand_info fm_spinand_table[] = {
	SPINAND_INFO("FM25S01", 0xa1,
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fm25s01_ooblayout, NULL)),
	SPINAND_INFO("FM25S01A", 0xe4,
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fm25s01_ooblayout, NULL)),
};

///////////////////////////////////// esmt device data /////////////////////////////////////

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
}

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
}

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
}

static int esmt2_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = 2 + (16 * section);
	region->length = 6;

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

static const struct spinand_info esmt_spinand_table[] = {
	SPINAND_INFO("F50L1G41A", 0x21,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&esmt_ooblayout, NULL)),
	SPINAND_INFO("F50L1G41LB", 0x01,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&esmt2_ooblayout, NULL)),
	SPINAND_INFO("F50L2G41LB", 0x0a,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 2),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&esmt2_ooblayout, NULL),
		     SPINAND_SELECT_TARGET(select_target)),
	SPINAND_INFO("F50L2G41KA", 0x41,
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&toshibamicron_ooblayout, micron_ecc_get_status),
		     SPINAND_SELECT_TARGET(select_target)),
};

///////////////////////////////////// etron device data /////////////////////////////////////

// for uboot we can't support Etron 8 bit ECC devices, the problem is they only have 4 status states when reading a page,
// no bits flipped, some bits flipped, max bits flipped and too mand bits flipped (ECC could not correct)
// so for example when we have a single bit error we would have to report the maximum bit errors for that status
// which is 7 (because we don't know the exact bit error amount and we report the worst.) Then if we do that the
// filesystem will see bit flips of 7 (out of 8 maximum bit flips) and it will clean the block and move the data. With too
// many single bit flips we will prematurely wear out the SPI NAND with all the filesystem block data moves.
// the other option is to read the page with and without the ECC turned on and count the bit differences, while not only
// making the interface slower it looks like this may not be possible because the API to the bad bit reporting code is:
// spinand->eccinfo.get_status(struct spinand_device *spinand, u8 status)

#define SPINAND_MFR_ETRON		0xD5

#define STATUS_ECC_LIMIT_BITFLIPS (3 << 4)

static int etron_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = mtd->oobsize / 2;
	region->length = mtd->oobsize / 2;

	return 0;
}

static int etron_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 1 bytes for the BBM. */
	region->offset = 1;
	region->length = (mtd->oobsize / 2) - 1;

	return 0;
}

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
}

static const struct spinand_info etron_spinand_table[] = {
	/* EM73C 1Gb 3.3V */
	SPINAND_INFO("EM73C044VCF", 0x25,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&etron_ooblayout, etron_ecc_get_status)),
};

///////////////////////////////////// APIs /////////////////////////////////////


/**
 * spinand_detect - initialize device related part in spinand_device struct
 * @spinand: SPI NAND device structure
 */

#define SPINAND_GENERIC_OP(out, in, buf)				\
	SPI_MEM_OP(SPI_MEM_OP_CMD(buf[0], 1),				\
		   SPI_MEM_OP_ADDR(out - 1, buf[1], 1),			\
		   SPI_MEM_OP_NO_DUMMY,							\
		   SPI_MEM_OP_DATA_IN(in, buf, 1))

static int spinand_read_id_op(struct spinand_device *spinand, u8 *buf)
{ // this is not defined correctly in core.c, should be one dummy byte
	//struct spi_mem_op op = SPINAND_READID_OP(1, spinand->scratchbuf, SPINAND_MAX_ID_LEN);
	int ret, i;
	buf[0] = 0x9F;
	buf[1] = 0;
	struct spi_mem_op op = SPINAND_GENERIC_OP(2, 4, buf);

	ret = spi_mem_exec_op(spinand->slave, &op);

	printf("SPINAND ID ");
	for (i = 0; i < SPINAND_MAX_ID_LEN; i++)
		printf("0x%x ", buf[i]);

	return ret;
}

static void spinand_report(const struct spinand_info *table, unsigned int table_size, u8 devid)
{
	unsigned int i;

	for (i = 0; i < table_size; i++)
	{
		const struct spinand_info *info = &table[i];

		if (devid != info->devid)
			continue;

		printf(" %s", info->model);
	}
	printf("\n");
}

static int spinand_detect(struct spinand_device *spinand)
{
	u8 *id = spinand->id.data;
	int ret;

	ret = spinand_read_id_op(spinand, spinand->id.data);
	if (ret)
		return ret;

	ret = -1;

	/*
	 * SPI NAND is now probed correctly issuing dummy byte of value 0
	 */

	if (id[0] == SPINAND_MFR_TOSHIBA)
	{
		if (!(ret = spinand_match_and_init(spinand, toshiba_spinand_table, ARRAY_SIZE(toshiba_spinand_table), id[1])))
		{
			printf("TOSHIBA");
			spinand_report(toshiba_spinand_table, ARRAY_SIZE(toshiba_spinand_table), id[1]);
		}
	}
	else if (id[0] == SPINAND_MFR_MICRON)
	{
		if (!(ret = spinand_match_and_init(spinand, micron_spinand_table, ARRAY_SIZE(micron_spinand_table), id[1])))
		{
			printf("MICRON");
			spinand_report(micron_spinand_table, ARRAY_SIZE(micron_spinand_table), id[1]);
		}
	}
	else if (id[0] == SPINAND_MFR_WINBOND)
	{
		if (id[1] == 0xAA)
		{
			if (!(ret = spinand_match_and_init(spinand, winbond_spinand_AA_table, ARRAY_SIZE(winbond_spinand_AA_table), id[2])))
			{
				printf("WINBOND");
				spinand_report(winbond_spinand_AA_table, ARRAY_SIZE(winbond_spinand_AA_table), id[2]);
			}
		}
		else if (id[1] == 0xAB)
		{
			if (!(ret = spinand_match_and_init(spinand, winbond_spinand_AB_table, ARRAY_SIZE(winbond_spinand_AB_table), id[2])))
			{
				printf("WINBOND");
				spinand_report(winbond_spinand_AB_table, ARRAY_SIZE(winbond_spinand_AB_table), id[2]);
			}
		}
	}
	else if (id[0] == SPINAND_MFR_MACRONIX)
	{
		if (!(ret = spinand_match_and_init(spinand, macronix_spinand_table, ARRAY_SIZE(macronix_spinand_table), id[1])))
		{
			printf("MACRONIX");
			spinand_report(macronix_spinand_table, ARRAY_SIZE(macronix_spinand_table), id[1]);
		}
	}
	else if (id[0] == SPINAND_MFR_PARAGON_FM)
	{
		if (!(ret = spinand_match_and_init(spinand, fm_spinand_table, ARRAY_SIZE(fm_spinand_table), id[1])))
		{
			printf("PARAGON");
			spinand_report(paragon_spinand_table, ARRAY_SIZE(paragon_spinand_table), id[1]);
		}
		else if (!(ret = spinand_match_and_init(spinand, fm_spinand_table, ARRAY_SIZE(fm_spinand_table), id[1])))
		{
			printf("FM");
			spinand_report(fm_spinand_table, ARRAY_SIZE(fm_spinand_table), id[1]);
		}
	}
	else if (id[0] == SPINAND_MFR_ETRON)
	{
		if (!(ret = spinand_match_and_init(spinand, etron_spinand_table, ARRAY_SIZE(etron_spinand_table), id[1])))
		{
			printf("ETRON");
			spinand_report(etron_spinand_table, ARRAY_SIZE(etron_spinand_table), id[1]);
		}
	}
	else if (id[0] == SPINAND_MFR_GIGADEVICE_ESMT)
	{
		if (!(ret = spinand_match_and_init(spinand, gigadevice_spinand_table, ARRAY_SIZE(gigadevice_spinand_table), id[1])))
		{
			{
				printf("GIGADEVICE");
				spinand_report(gigadevice_spinand_table, ARRAY_SIZE(gigadevice_spinand_table), id[1]);
			}
		}
		else if (!(ret = spinand_match_and_init(spinand, esmt_spinand_table, ARRAY_SIZE(esmt_spinand_table), id[1])))
		{
			printf("ESMT");
			spinand_report(esmt_spinand_table, ARRAY_SIZE(esmt_spinand_table), id[1]);
		}
	}
	else if (id[1] == SPINAND_MFR_GIGADEVICE_ESMT)
	{ // this is a hack for Gigadevice until they can get their act together, check this one last
		if (!(ret = spinand_match_and_init(spinand, gigadevice_spinand_table, ARRAY_SIZE(gigadevice_spinand_table), id[0])))
		{
			printf("GIGADEVICE");
			spinand_report(gigadevice_spinand_table, ARRAY_SIZE(gigadevice_spinand_table), id[1]);
		}
	}

	if (ret)
	{
		printf("SPI NAND NOT FOUND!!!!!\n");
		return ret;
	}

	return 1;
}

static const struct spinand_manufacturer_ops spinand_manuf_ops = {
	.detect = spinand_detect,
	.init = NULL,
};

const struct spinand_manufacturer broadcom_spinand_manufacturer = {
	.id = 0,
	.name = "Broadcom",
	.ops = &spinand_manuf_ops,
};

