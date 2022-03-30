/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom Ltd.
 */

#include <common.h>
#include <nand.h>
#include <spl.h>

#include <asm/arch/misc.h>

#include "bca_common.h"
#include "bcmbca_nand_spl.h"
#include "brcmnand_spl.h"
#include <linux/mtd/spinand_mini.h>


u32 (*get_block_size)(void);
u32 (*get_page_size)(void);
u64 (*get_total_size)(void);
int (*read_block)(int block, int offset, u8 *dst, u32 len);
int (*is_bad_block)(int block);


int type = BOOT_DEVICE_NAND;


void nand_init(void)
{
	static int nand_init = 0;

	/* return if already initalized */
	if (nand_init)
		return;

	type = bcmbca_get_boot_device();

	switch (type)
	{
#if defined(CONFIG_MTD_SPI_NAND)
		case BOOT_DEVICE_SPI:
			spinandmini_init();
			get_page_size = spinandmini_get_page_size;
			get_block_size = spinandmini_get_block_size;
			get_total_size = spinandmini_get_total_size;
			read_block = spinandmini_read_buf;
			is_bad_block = spinandmini_is_bad_block;
			break;
#endif

#if defined(CONFIG_NAND_BRCMNAND)
		case BOOT_DEVICE_NAND:			
		default:
			brcmnand_init();
			get_page_size = brcmnand_get_page_size;
			get_block_size = brcmnand_get_block_size;
			get_total_size = brcmnand_get_total_size;
			read_block = brcmnand_read_buf;
			is_bad_block = brcmnand_is_bad_block;
			break;
#endif
	}

	nand_init = 1;

	return;
}

void nand_deselect(void)
{
}

u32 nand_spl_get_page_size(void)
{
	return (*get_page_size)();
}

u32 nand_spl_get_blk_size(void)
{
	return (*get_block_size)();
}

u64 nand_spl_get_total_size(void)
{
	return (*get_total_size)();
}

__weak int nand_is_bad_block(unsigned int blk)
{
	return (*is_bad_block)(blk);
}

int nand_spl_read_block(int block, int offset, int len, void *dst)
{
	int ret;
  
	ret = (*read_block)(block, offset, dst, len);
	return (ret > 0) ? 0 : ret;
}

int nand_spl_load_image(u32 offs, u32 size, void *dst)
{
	u32 block, lastblock, block_size;
	u32 block_offset, read_size;
	int ret = 0;

	block_size = nand_spl_get_blk_size();
	/* offs has to be aligned to a page address! */
	block = offs / block_size;
	lastblock = (offs + size - 1) / block_size;
	block_offset = offs % block_size;
	read_size = block_size - block_offset;
	if (read_size > size)
		read_size = size;

	if (nand_is_bad_block(block))
		return -EIO;

	while (block <= lastblock) {
		/* Skip bad blocks */
		if (!nand_is_bad_block(block)) {
			ret = nand_spl_read_block(block, block_offset, read_size, dst);
			if (ret < 0) {
				printf("read block %d offset 0x%x size 0x%x dst %p failed ret %d\n",
				       block, block_offset, read_size, dst, ret);
				return -EIO;
			}
		} else {
			printf("nand_spl_load_image skip bad block %d\n", block);
			lastblock++;
			block++;
			continue;
		}

		size -= read_size;
		if (size == 0)
			break;

		dst += read_size;
		read_size = block_size;
		if (read_size > size)
			read_size = size;
		block_offset = 0;
		block++;
	}

	return 0;
}
