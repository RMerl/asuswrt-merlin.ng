// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2017 exceet electronics GmbH
 *
 * Authors:
 *	Frieder Schrempf <frieder.schrempf@exceet.de>
 *	Boris Brezillon <boris.brezillon@bootlin.com>
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_WINBOND		0xEF

#define WINBOND_CFG_BUF_READ		BIT(3)

#ifdef CONFIG_BCM_KF_MTD_BCMSPINAND
#define WINBOND_FEATURE_STAT_ENH	0x30
#define WINBOND_ENH_STAT_MASK		0xF0
#define WINBOND_ENH_STAT_SHIFT		4

#define WINBOND_BFD			0x10
#define WINBOND_BFD_MASK		GENMASK(6, 4)
#define WINBOND_BFD_SET			BIT(5) /* set to 2 bits, so > 2 bits (3 of 4) triggers correctible */

#define STATUS_ECC_LIMIT_BITFLIPS (3 << 4)
#endif

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

static const struct mtd_ooblayout_ops w25m02gv_ooblayout = {
	.ecc = w25m02gv_ooblayout_ecc,
	.free = w25m02gv_ooblayout_free,
};

static int w25m02gv_select_target(struct spinand_device *spinand,
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
}

#ifdef CONFIG_BCM_KF_MTD_BCMSPINAND
static int winbond_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 128;
	region->length = 128;

	return 0;
};

static int winbond_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	// Reserve 2 bytes for the BBM.
	region->offset = 2;
	region->length = 126;

	return 0;
};

static const struct mtd_ooblayout_ops winbond_ooblayout =
{
	.ecc = winbond_ooblayout_ecc,
	.free = winbond_ooblayout_free,
};

static int w25n01kv_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 64;
	region->length = 32;

	return 0;
};

static int w25n01kv_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	// Reserve 2 bytes for the BBM.
	region->offset = 2;
	region->length = 62;

	return 0;
};

static const struct mtd_ooblayout_ops w25n01kv_ooblayout =
{
	.ecc = w25n01kv_ooblayout_ecc,
	.free = w25n01kv_ooblayout_free,
};

static int winbond_ecc_get_status(struct spinand_device *spinand, u8 status)
{
	switch (status & STATUS_ECC_MASK)
	{
		case STATUS_ECC_NO_BITFLIPS:
			return 0;

		case STATUS_ECC_UNCOR_ERROR:
			return -EBADMSG;

		case STATUS_ECC_HAS_BITFLIPS:
			return 2;

		case STATUS_ECC_LIMIT_BITFLIPS:
			return 4;
	}

	return -EINVAL;
};

static int winbond_ecc_get_status2(struct spinand_device *spinand, u8 status)
{
	u8 status2;
	struct spi_mem_op op = SPINAND_GET_FEATURE_OP(WINBOND_FEATURE_STAT_ENH, &status2);
	int ret;

	switch (status & STATUS_ECC_MASK)
	{
		case STATUS_ECC_NO_BITFLIPS:
			return 0;

		case STATUS_ECC_UNCOR_ERROR:
			return -EBADMSG;

		default:
		{ // default to correctible errors since Winbond uses the remaining two states as correctible
			ret = spi_mem_exec_op(spinand->spimem, &op);
			if (!ret)
				ret = ((status2 & WINBOND_ENH_STAT_MASK) >> WINBOND_ENH_STAT_SHIFT);

			return(ret);
		}
	}

	return -EINVAL;
};

static const struct spinand_info winbond_spinand_table[] = {
	SPINAND_INFO("W25N512GV",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xaa, 0x20),
		     NAND_MEMORG(1, 2048, 64, 64, 512, 10, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&w25m02gv_ooblayout, NULL)),
	SPINAND_INFO("W25N01GV",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xaa, 0x21),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&w25m02gv_ooblayout, NULL)),
	SPINAND_INFO("W25N02GV",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xaa, 0x22),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&w25m02gv_ooblayout, NULL),
		     SPINAND_SELECT_TARGET(w25m02gv_select_target)),
	SPINAND_INFO("W25N04KV",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xaa, 0x23),
		     NAND_MEMORG(1, 2048, 128, 64, 4096, 80, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&winbond_ooblayout, winbond_ecc_get_status2)),
	SPINAND_INFO("W25M02GV",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xab, 0x21),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 2),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&w25m02gv_ooblayout, NULL),
		     SPINAND_SELECT_TARGET(w25m02gv_select_target)),
	SPINAND_INFO("W25N01KV",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xae, 0x21),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&w25n01kv_ooblayout, winbond_ecc_get_status)),
};
#else
static const struct spinand_info winbond_spinand_table[] = {
	SPINAND_INFO("W25M02GV",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xab),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 2),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&w25m02gv_ooblayout, NULL),
		     SPINAND_SELECT_TARGET(w25m02gv_select_target)),
	SPINAND_INFO("W25N01GV",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xaa),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&w25m02gv_ooblayout, NULL)),
};
#endif

static int winbond_spinand_init(struct spinand_device *spinand)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int i;

	/*
	 * Make sure all dies are in buffer read mode and not continuous read
	 * mode.
	 */
	for (i = 0; i < nand->memorg.ntargets; i++) {
		spinand_select_target(spinand, i);
		spinand_upd_cfg(spinand, WINBOND_CFG_BUF_READ,
				WINBOND_CFG_BUF_READ);
	}

#ifdef CONFIG_BCM_KF_MTD_BCMSPINAND
	if ((spinand->id.data[1] == 0xae) && (spinand->id.data[2] == 0x21))
	{ // set bit flip detect to two bits, this should enable it such that <= 2 bits and > 2 bits are detected seperately when reading the status
		u8 status;
		struct spi_mem_op op = SPINAND_GET_FEATURE_OP(WINBOND_BFD, &status);
		struct spi_mem_op op2;
		int ret;

		if ((ret = spi_mem_exec_op(spinand->spimem, &op)))
		{
			printk("ERROR!!! Could not set Winbond SPI NAND feature\n");
			return(ret);
		}

		status &= ~WINBOND_BFD_MASK;
		status |= WINBOND_BFD_SET;

		{ // need to do this because even though the address of status is passed, it's converted to a static value on invocation
			struct spi_mem_op op_temp = SPINAND_SET_FEATURE_OP(WINBOND_BFD, &status);
			op2 = op_temp;
		}

		if ((ret = spi_mem_exec_op(spinand->spimem, &op)))
		{
			printk("ERROR!!! Could not set Winbond SPI NAND feature\n");
			return(ret);
		}
	}
#endif
	return 0;
}

static const struct spinand_manufacturer_ops winbond_spinand_manuf_ops = {
	.init = winbond_spinand_init,
};

const struct spinand_manufacturer winbond_spinand_manufacturer = {
	.id = SPINAND_MFR_WINBOND,
	.name = "Winbond",
	.chips = winbond_spinand_table,
	.nchips = ARRAY_SIZE(winbond_spinand_table),
	.ops = &winbond_spinand_manuf_ops,
};
