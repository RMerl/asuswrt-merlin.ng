/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 * Copyright (C) 2016-2017 Micron Technology, Inc.
 *
 * Authors:
 *	Peter Pan <peterpandong@micron.com>
 *	Boris Brezillon <boris.brezillon@bootlin.com>
 */
#include <common.h>
#include <errno.h>

#include <linux/mtd/spinand_mini.h>

static struct spinandmini_device *spinand = NULL;

static const struct spinandmini_info spinand_chip_table[] = {
	{
		/* TC58CVG0S */
		{SPINAND_MFR_TOSHIBA, 0xC2}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1)
	},
	{
		/* TC58CVG1S */
		{SPINAND_MFR_TOSHIBA, 0xCB}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1)
	},
	{
		/* TC58CVG1S0HRAIJ */
		{SPINAND_MFR_TOSHIBA, 0xEB}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1)
	},
	{
		/* TC58CVG2S */
		{SPINAND_MFR_TOSHIBA, 0xCD}, 2,
		SPINANDMINI_MEMORG(1, 4096, 256, 64, 2048, 1, 1, 1)
	},
	{
		/* TC58CVG2S0HRAIJ */
		{SPINAND_MFR_TOSHIBA, 0xED}, 2,
		SPINANDMINI_MEMORG(1, 4096, 256, 64, 2048, 1, 1, 1)
	},
	{
		/* TC58CVG03S */
		{SPINAND_MFR_TOSHIBA, 0xE2, 0x40}, 3,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1)
	},
	{
		/* MT29F1G01AA */
		{SPINAND_MFR_MICRON, 0x12}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1)
	},
	{
		/* MT29F2G01AA */
		{SPINAND_MFR_MICRON, 0x22}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1)
	},
	{
		/* MT29F4G01AA */
		{SPINAND_MFR_MICRON, 0x32}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 4096, 1, 1, 1)
	},
	{
		/* MT29F1G01A */
		{SPINAND_MFR_MICRON, 0x14}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1)
	},
	{
		/* MT29F2G01B */
		{SPINAND_MFR_MICRON, 0x24}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 2048, 2, 1, 1)
	},
	{
		/* MT29F4G01AB */
		{SPINAND_MFR_MICRON, 0x34}, 2,
		SPINANDMINI_MEMORG(1, 4096, 256, 64, 2048, 1, 1, 1)
	},
	{
		/* MT29F4G01AD */
		{SPINAND_MFR_MICRON, 0x36}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 2048, 2, 1, 2)
	},
	{
		/* W25N04KV */
		{SPINAND_MFR_WINBOND, 0xAA, 0x23}, 3,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 2)
	},
	{
		/* W25M02GV */
		{SPINAND_MFR_WINBOND, 0xAB, 0x21}, 3,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 2)
	},
	{
		/* W25N02GV */
		{SPINAND_MFR_WINBOND, 0xAA, 0x22}, 3,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 2048, 1, 1, 1)
	},
	{
		/* W25N01GV */
		{SPINAND_MFR_WINBOND, 0xAA, 0x21}, 3,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1)
	},
	{
		/* W25N512GV */
		{SPINAND_MFR_WINBOND, 0xAA, 0x20}, 3,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 512, 1, 1, 1)
	},
	{
		/* MX35LF1GE4AB */
		{SPINAND_MFR_MACRONIX, 0x12}, 2,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1)
	},
	{
		/* MX35LF2GE4AB */
		{SPINAND_MFR_MACRONIX, 0x22}, 2,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 2048, 2, 1, 1)
	},
	{
		/* MX35LF2GE4AD */
		{SPINAND_MFR_MACRONIX, 0x26}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1)
	},
	{
		/* MX35LF4GE4AD */
		{SPINAND_MFR_MACRONIX, 0x37}, 2,
		SPINANDMINI_MEMORG(1, 4096, 128, 64, 2048, 1, 1, 1)
	},
	{
		/* PN26G01A */
		{SPINAND_MFR_PARAGON, 0xE1}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1)
	},
	{
		/* PN26G02A */
		{SPINAND_MFR_PARAGON, 0xE2}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1)
	},
	{
		/* F50L1G41A */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0x21}, 2,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1)
	},
	{
		/* F50L1G41LB */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0x01}, 2,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1)
	},
	{
		/* F50L2G41LB */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0x0A}, 2,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 2)
	},
	{
		/* F50L2G41KA */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0x41}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1)
	},
	{
		/* GD5F1GQ4xA */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0xF1}, 2,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1)
	},
	{
		/* GD5F2GQ4xA */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0xF2}, 2,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 2048, 1, 1, 1)
	},
	{
		/* GD5F4GQ4xA */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0xF4}, 2,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 4096, 1, 1, 1)
	},
	{
		/* GD5F4GQ4UExxG */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0xD4}, 2,
		SPINANDMINI_MEMORG(1, 4096, 256, 64, 2048, 1, 1, 1)
	},
	{
		/* GD5F2GQ4UExxG */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0xD2}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 2048, 1, 1, 1)
	},
	{
		/* GD5F1GQ4UExxG */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0xD1}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1)
	},
	{
		/* GD5F4GQ6UExxG */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0x55}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 4096, 1, 1, 1)
	},
	{
		/* GD5F1GQ5UExxG */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0x51}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1)
	},
	{
		/* GD5F1GQ4UFxxG */
		{SPINAND_MFR_GIGADEVICE_ESMT, 0xB1, 0x48}, 3,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1)
	},
	{
		/* FM25S01 */
		{SPINAND_MFR_PARAGON, 0xA1}, 2,
		SPINANDMINI_MEMORG(1, 2048, 128, 64, 1024, 1, 1, 1)
	},
	{
		/* FM25S01A */
		{SPINAND_MFR_PARAGON, 0xE4}, 2,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1)
	},
	{
		/* EM73C044VCF */
		{SPINAND_MFR_ETRON, 0x25}, 2,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1)
	},
	{
		/* Unknown, must be last entry */
		{0x0, 0x0}, 2,
		SPINANDMINI_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1)
	},

};

static int spinandchip_select_target(struct spinandmini_device *spinand,
				  unsigned int target)
{
	u32 buf = target;  
	struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(0xc2, 1),
					  SPI_MEM_OP_NO_ADDR,
					  SPI_MEM_OP_NO_DUMMY,
					  SPI_MEM_OP_DATA_OUT(1,
							&buf,
							1));

	return spi_mem_exec_op(spinand->slave, &op);
}

static int spinandmini_read_reg_op(struct spinandmini_device *spinand, u8 reg, u8 *val)
{
	u8 feature = 0;
	struct spi_mem_op op = SPINAND_GET_FEATURE_OP(reg, &feature);
	int ret;

	ret = spi_mem_exec_op(spinand->slave, &op);
	if (ret)
		return ret;

	*val = feature;
	return 0;
}

static int spinandmini_write_reg_op(struct spinandmini_device *spinand, u8 reg, u8 val)
{
	struct spi_mem_op op = SPINAND_SET_FEATURE_OP(reg, &val);

	return spi_mem_exec_op(spinand->slave, &op);
}

static int spinandmini_read_status(struct spinandmini_device *spinand, u8 *status)
{
	return spinandmini_read_reg_op(spinand, REG_STATUS, status);
}

static int spinandmini_get_cfg(struct spinandmini_device *spinand, u8 *cfg)
{
	if (WARN_ON(spinand->cur_target < 0 ||
		    spinand->cur_target >= spinand->memorg->ntargets))
		return -EINVAL;

	*cfg = spinand->cfg_cache[spinand->cur_target];
	return 0;
}

static int spinandmini_set_cfg(struct spinandmini_device *spinand, u8 cfg)
{
	int ret;

	if (WARN_ON(spinand->cur_target < 0 ||
		    spinand->cur_target >= spinand->memorg->ntargets))
		return -EINVAL;

	if (spinand->cfg_cache[spinand->cur_target] == cfg)
		return 0;

	ret = spinandmini_write_reg_op(spinand, REG_CFG, cfg);
	if (ret)
		return ret;

	spinand->cfg_cache[spinand->cur_target] = cfg;
	return 0;
}

static int spinandmini_upd_cfg(struct spinandmini_device *spinand, u8 mask, u8 val)
{
	int ret;
	u8 cfg;

	ret = spinandmini_get_cfg(spinand, &cfg);
	if (ret)
		return ret;

	cfg &= ~mask;
	cfg |= val;

	return spinandmini_set_cfg(spinand, cfg);
}

static int spinandmini_ecc_enable(struct spinandmini_device *spinand,
			      bool enable)
{
	return spinandmini_upd_cfg(spinand, CFG_ECC_ENABLE,
			       enable ? CFG_ECC_ENABLE : 0);
}

static int spinandmini_load_page_op(struct spinandmini_device *spinand,
				u64 page_addr)
{
	unsigned int row = page_addr >> spinand->page_shift;
	struct spi_mem_op op = SPINAND_PAGE_READ_OP(row);

	return spi_mem_exec_op(spinand->slave, &op);
}

static int spinandmini_read_from_cache_op(struct spinandmini_device *spinand,
				      u64 page_addr, u32 page_offset,
				      u8* buf, u32 len)
{
	struct spi_mem_op op =
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0);
	u16 column = page_offset;
	int ret;

	//TODO plane bit at bit 12 or right above column MSB bit?? 
	column |= ((page_addr >> spinand->block_shift) % spinand->memorg->planes_per_lun)<<12;
	op.addr.val = column;

	/*
	 * Some controllers are limited in term of max RX data size. In this
	 * case, just repeat the READ_CACHE operation after updating the
	 * column.
	 */
	while (len) {
		op.data.buf.in = buf;
		op.data.nbytes = len;
		ret = spi_mem_adjust_op_size(spinand->slave, &op);
		if (ret)
			return ret;

		ret = spi_mem_exec_op(spinand->slave, &op);
		if (ret)
			return ret;

		buf += op.data.nbytes;
		len -= op.data.nbytes;
		op.addr.val += op.data.nbytes;
	}

	return 0;
}

static int spinandmini_wait(struct spinandmini_device *spinand, u8 *s)
{
	unsigned long start, stop;
	u8 status;
	int ret;

	start = get_timer(0);
	stop = 400;
	do {
		ret = spinandmini_read_status(spinand, &status);
		if (ret)
			return ret;

		if (!(status & STATUS_BUSY))
			goto out;
	} while (get_timer(start) < stop);

	/*
	 * Extra read, just in case the STATUS_READY bit has changed
	 * since our last check
	 */
	ret = spinandmini_read_status(spinand, &status);
	if (ret)
		return ret;

out:
	if (s)
		*s = status;

	return status & STATUS_BUSY ? -ETIMEDOUT : 0;
}

static int spinandmini_read_id_op(struct spinandmini_device *spinand)
{
	/* looks like most of the chips use the op + addr method to detect id. 
	 * TODO: Add parameter to this function to support op+dummy or op only 
 	*/
	struct spi_mem_op op = SPINAND_READID_OP_ADDR(spinand->id.data,
						 SPINAND_MAX_ID_LEN);

	return spi_mem_exec_op(spinand->slave, &op);
}

static int spinandmini_reset_op(struct spinandmini_device *spinand)
{
	struct spi_mem_op op = SPINAND_RESET_OP;
	int ret;

	ret = spi_mem_exec_op(spinand->slave, &op);
	if (ret)
		return ret;

	return spinandmini_wait(spinand, NULL);
}

static int spinandmini_lock_block(struct spinandmini_device *spinand, u8 lock)
{
	return spinandmini_write_reg_op(spinand, REG_BLOCK_LOCK, lock);
}

static int spinandmini_select_target(struct spinandmini_device *spinand, unsigned int target)
{
	int ret;

	if (WARN_ON(target >= spinand->memorg->ntargets))
		return -EINVAL;

	if (spinand->cur_target == target)
		return 0;

	if (spinand->memorg->ntargets == 1) {
		spinand->cur_target = target;
		return 0;
	}

	ret = spinand->select_target(spinand, target);
	if (ret)
		return ret;

	spinand->cur_target = target;
	return 0;
}


static int spinandmini_read_page(struct spinandmini_device *spinand,
				      u64 page_addr, u32 page_offset,
			 	      u8* buf, u32 len)
{
	u8 status;
	int ret = 0;

	ret = spinandmini_load_page_op(spinand, page_addr);
	if (ret)
		return ret;

	ret = spinandmini_wait(spinand, &status);
	if (ret < 0)
		return ret;

	ret = spinandmini_read_from_cache_op(spinand, page_addr, page_offset,
					 buf, len);
	return ret;
}

int spinandmini_read_buf(int blk, int offset, u8 *buffer, u32 len)
{
	int ret = 0, target = 0, readlen = len;
	u64 addr, page_addr, page_boundary;
	u32 page_offset;
	u32 size;	

	addr = (blk * spinand->block_size) + offset;
	page_addr = addr & ~(spinand->page_size - 1);
	page_offset = addr - page_addr;
	page_boundary = page_addr + spinand->page_size;

	size = page_boundary - addr;
	if(size > len)
		size = len;

	while (len) {
		target = page_addr >> spinand->target_shift;
		ret = spinandmini_select_target(spinand, target);
		if (ret)
			break;

		ret = spinandmini_ecc_enable(spinand, true);
		if (ret)
			break;

		ret = spinandmini_read_page(spinand, page_addr, page_offset,
			buffer, size);
		if (ret)
			break;

		len -= size;
		if (len) {
			page_addr += spinand->page_size;
			page_offset = 0;
			buffer += size;
			if(len > spinand->page_size)
				size = spinand->page_size;
			else
				size = len;
		}
	}

	return ret ? ret : readlen;
}

int spinandmini_is_bad_block(int blk)
{
	u8 oobbuf = 0;
	u64 addr = (blk * spinand->block_size);
	int target = addr >> spinand->target_shift;
	int ret = 0;
	
	ret = spinandmini_select_target(spinand, target);
	if (ret)
		return 1;

	ret = spinandmini_ecc_enable(spinand, false);
	if (ret)
		return 1;

	ret = spinandmini_read_page(spinand, addr, spinand->page_size,
		&oobbuf, 1);
	if (ret)
		return 1;

	if (hweight8(oobbuf) < 4)
		return 1;

	return 0;
}

static void spinandmini_print_id(struct spinandmini_device *spinand)
{
	int i;
	for (i = 0; i < spinand->id.len; i++)
		printf("0x%02x ", spinand->id.data[i]);
	printf("\n");
}

static int spinandmini_detect(struct spinandmini_device *spinand)
{
	int ret, i, tbl_size;
	const struct spinandmini_info* spinand_info;
	const struct spinandmini_mem_org* memorg;
	u64 target_size;
	
	ret = spinandmini_reset_op(spinand);
	if (ret)
		return ret;

	ret = spinandmini_read_id_op(spinand);
	if (ret)
		return ret;

	tbl_size = sizeof(spinand_chip_table)/sizeof(struct spinandmini_info);
	
	for (i = 0; i < tbl_size; i++) {
		spinand_info = &spinand_chip_table[i];
		if (!memcmp(spinand_info->id, spinand->id.data, spinand_info->id_len))
			break;
	}
	
	if ( i == tbl_size) {
		printf("unknown ID, use default setting\n");
	}
	memorg = spinand->memorg = &spinand_info->memorg;
	spinand->id.len = spinand_info->id_len;
	
	spinand->page_size = memorg->pagesize;
	spinand->page_shift = fls(spinand->page_size-1);
	
	spinand->block_size =
		spinand->page_size * memorg->pages_per_eraseblock;
	spinand->block_shift = fls(spinand->block_size-1);

	target_size = spinand->block_size *
		memorg->eraseblocks_per_lun *
		memorg->luns_per_target;
	spinand->target_shift = fls(target_size-1);
	
	spinand->total_size = target_size * memorg->ntargets;

	spinand->select_target = spinandchip_select_target;
	
	if (spinand->memorg->ntargets > 1 && !spinand->select_target) {
		return -EINVAL;
	}

	printf("SPI NAND ID: ");
	spinandmini_print_id(spinand);
	printf("%llu MiB, block size: %u KiB, page size: %u\n",
		spinand->total_size>>20, spinand->block_size>>10, spinand->page_size);

	return 0;
}

static int spinandmini_init_cfg_cache(struct spinandmini_device *spinand)
{
	unsigned int target;
	int ret;

	spinand->cfg_cache = malloc(sizeof(*spinand->cfg_cache) *
				    spinand->memorg->ntargets);

	if (!spinand->cfg_cache)
		return -ENOMEM;

	for (target = 0; target < spinand->memorg->ntargets; target++) {
		ret = spinandmini_select_target(spinand, target);
		if (ret)
			return ret;

		/*
		 * We use spinand_read_reg_op() instead of spinand_get_cfg()
		 * here to bypass the config cache.
		 */
		ret = spinandmini_read_reg_op(spinand, REG_CFG,
					  &spinand->cfg_cache[target]);
		if (ret)
			return ret;
	}

	return 0;
}

static int spinandmini_initdev(struct spinandmini_device *spinand)
{
	int ret = 0, i;

	ret = spinandmini_detect(spinand);
	if (ret)
		return ret;

	ret = spinandmini_init_cfg_cache(spinand);
	if (ret)
		return ret;

	/* After power up, all blocks are locked, so unlock them here. */
	for (i = 0; i < spinand->memorg->ntargets; i++) {
		ret = spinandmini_select_target(spinand, i);
		if (ret)
			break;

		ret = spinandmini_lock_block(spinand, BL_ALL_UNLOCKED);
		if (ret)
			break;
	}

	return ret;
}

static int spinandmini_probe(struct udevice *dev)
{
	struct spinandmini_device *spinand_dev = dev_get_priv(dev);
	struct spi_slave *slave = dev_get_parent_priv(dev);

	spinand = spinand_dev;
	spinand->slave = slave;
	
	return spinandmini_initdev(spinand);
}

void spinandmini_init(void)
{
	struct udevice *dev;
	int ret = -1;

	ret = uclass_get_device_by_driver(UCLASS_MTD, DM_GET_DRIVER(spinand),
		  &dev);
	if (ret){
		printf("SPI NAND failed to initialize. (error %d)\n", ret);
		hang();
	}

	return;
}

u32 spinandmini_get_block_size(void)
{
	return spinand->block_size;
}

u32 spinandmini_get_page_size(void)
{
	return spinand->page_size;
}

u64 spinandmini_get_total_size(void)
{
	return spinand->total_size;  
}

static const struct udevice_id spinand_ids[] = {
	{ .compatible = "spi-nand" },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(spinand) = {
	.name = "spi_nand",
	.id = UCLASS_MTD,
	.of_match = spinand_ids,
	.priv_auto_alloc_size = sizeof(struct spinandmini_device),
	.probe = spinandmini_probe,
};
