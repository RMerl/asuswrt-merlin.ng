// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Gateworks Corporation
 * Copyright 2019 NXP
 * Author: Tim Harvey <tharvey@gateworks.com>
 */
#include <common.h>
#include <log.h>
#include <nand.h>
#include <malloc.h>
#include <mxs_nand.h>
#include <asm/cache.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/mtd/rawnand.h>

static struct mtd_info *mtd;
static struct nand_chip nand_chip;

static void mxs_nand_command(struct mtd_info *mtd, unsigned int command,
			     int column, int page_addr)
{
	register struct nand_chip *chip = mtd_to_nand(mtd);
	u32 timeo, time_start;

	/* write out the command to the device */
	chip->cmd_ctrl(mtd, command, NAND_CLE);

	/* Serially input address */
	if (column != -1) {
		/* Adjust columns for 16 bit buswidth */
		if (chip->options & NAND_BUSWIDTH_16 &&
				!nand_opcode_8bits(command))
			column >>= 1;
		chip->cmd_ctrl(mtd, column, NAND_ALE);

		/*
		 * Assume LP NAND here, so use two bytes column address
		 * but not for CMD_READID and CMD_PARAM, which require
		 * only one byte column address
		 */
		if (command != NAND_CMD_READID &&
			command != NAND_CMD_PARAM)
			chip->cmd_ctrl(mtd, column >> 8, NAND_ALE);
	}
	if (page_addr != -1) {
		chip->cmd_ctrl(mtd, page_addr, NAND_ALE);
		chip->cmd_ctrl(mtd, page_addr >> 8, NAND_ALE);
		/* One more address cycle for devices > 128MiB */
		if (chip->chipsize > (128 << 20))
			chip->cmd_ctrl(mtd, page_addr >> 16, NAND_ALE);
	}
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 0);

	if (command == NAND_CMD_READ0) {
		chip->cmd_ctrl(mtd, NAND_CMD_READSTART, NAND_CLE);
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, 0);
	} else if (command == NAND_CMD_RNDOUT) {
		/* No ready / busy check necessary */
		chip->cmd_ctrl(mtd, NAND_CMD_RNDOUTSTART,
			       NAND_NCE | NAND_CLE);
		chip->cmd_ctrl(mtd, NAND_CMD_NONE,
			       NAND_NCE);
	}

	/* wait for nand ready */
	ndelay(100);
	timeo = (CONFIG_SYS_HZ * 20) / 1000;
	time_start = get_timer(0);
	while (get_timer(time_start) < timeo) {
		if (chip->dev_ready(mtd))
			break;
	}
}

#if defined (CONFIG_SPL_NAND_IDENT)

/* Trying to detect the NAND flash using ONFi, JEDEC, and (extended) IDs */
static int mxs_flash_full_ident(struct mtd_info *mtd)
{
	int nand_maf_id, nand_dev_id;
	struct nand_chip *chip = mtd_to_nand(mtd);
	int ret;

	ret = nand_detect(chip, &nand_maf_id, &nand_dev_id, NULL);

	if (ret) {
		chip->select_chip(mtd, -1);
		return ret;
	}

	return 0;
}

#else

/* Trying to detect the NAND flash using ONFi only */
static int mxs_flash_onfi_ident(struct mtd_info *mtd)
{
	register struct nand_chip *chip = mtd_to_nand(mtd);
	int i;
	u8 mfg_id, dev_id;
	u8 id_data[8];
	struct nand_onfi_params *p = &chip->onfi_params;

	/* Reset the chip */
	chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

	/* Send the command for reading device ID */
	chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

	/* Read manufacturer and device IDs */
	mfg_id = chip->read_byte(mtd);
	dev_id = chip->read_byte(mtd);

	/* Try again to make sure */
	chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);
	for (i = 0; i < 8; i++)
		id_data[i] = chip->read_byte(mtd);
	if (id_data[0] != mfg_id || id_data[1] != dev_id) {
		printf("second ID read did not match");
		return -1;
	}
	debug("0x%02x:0x%02x ", mfg_id, dev_id);

	/* read ONFI */
	chip->onfi_version = 0;
	chip->cmdfunc(mtd, NAND_CMD_READID, 0x20, -1);
	if (chip->read_byte(mtd) != 'O' || chip->read_byte(mtd) != 'N' ||
	    chip->read_byte(mtd) != 'F' || chip->read_byte(mtd) != 'I') {
		return -2;
	}

	/* we have ONFI, probe it */
	chip->cmdfunc(mtd, NAND_CMD_PARAM, 0, -1);
	chip->read_buf(mtd, (uint8_t *)p, sizeof(*p));
	mtd->name = p->model;
	mtd->writesize = le32_to_cpu(p->byte_per_page);
	mtd->erasesize = le32_to_cpu(p->pages_per_block) * mtd->writesize;
	mtd->oobsize = le16_to_cpu(p->spare_bytes_per_page);
	chip->chipsize = le32_to_cpu(p->blocks_per_lun);
	chip->chipsize *= (uint64_t)mtd->erasesize * p->lun_count;
	/* Calculate the address shift from the page size */
	chip->page_shift = ffs(mtd->writesize) - 1;
	chip->phys_erase_shift = ffs(mtd->erasesize) - 1;
	/* Convert chipsize to number of pages per chip -1 */
	chip->pagemask = (chip->chipsize >> chip->page_shift) - 1;
	chip->badblockbits = 8;

	debug("erasesize=%d (>>%d)\n", mtd->erasesize, chip->phys_erase_shift);
	debug("writesize=%d (>>%d)\n", mtd->writesize, chip->page_shift);
	debug("oobsize=%d\n", mtd->oobsize);
	debug("chipsize=%lld\n", chip->chipsize);

	return 0;
}

#endif /* CONFIG_SPL_NAND_IDENT */

static int mxs_flash_ident(struct mtd_info *mtd)
{
	int ret;
#if defined (CONFIG_SPL_NAND_IDENT)
	ret = mxs_flash_full_ident(mtd);
#else
	ret = mxs_flash_onfi_ident(mtd);
#endif
	return ret;
}

static int mxs_read_page_ecc(struct mtd_info *mtd, void *buf, unsigned int page)
{
	register struct nand_chip *chip = mtd_to_nand(mtd);
	int ret;

	chip->cmdfunc(mtd, NAND_CMD_READ0, 0x0, page);
	ret = nand_chip.ecc.read_page(mtd, chip, buf, 1, page);
	if (ret < 0) {
		printf("read_page failed %d\n", ret);
		return -1;
	}
	return 0;
}

static int is_badblock(struct mtd_info *mtd, loff_t offs, int allowbbt)
{
	register struct nand_chip *chip = mtd_to_nand(mtd);
	unsigned int block = offs >> chip->phys_erase_shift;
	unsigned int page = offs >> chip->page_shift;

	debug("%s offs=0x%08x block:%d page:%d\n", __func__, (int)offs, block,
	      page);
	chip->cmdfunc(mtd, NAND_CMD_READ0, mtd->writesize, page);
	memset(chip->oob_poi, 0, mtd->oobsize);
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	return chip->oob_poi[0] != 0xff;
}

/* setup mtd and nand structs and init mxs_nand driver */
void nand_init(void)
{
	/* return if already initalized */
	if (nand_chip.numchips)
		return;

	/* init mxs nand driver */
	mxs_nand_init_spl(&nand_chip);
	mtd = nand_to_mtd(&nand_chip);
	/* set mtd functions */
	nand_chip.cmdfunc = mxs_nand_command;
	nand_chip.scan_bbt = nand_default_bbt;
	nand_chip.numchips = 1;

	/* identify flash device */
	if (mxs_flash_ident(mtd)) {
		printf("Failed to identify\n");
		nand_chip.numchips = 0; /* If fail, don't use nand */
		return;
	}

	/* allocate and initialize buffers */
	nand_chip.buffers = memalign(ARCH_DMA_MINALIGN,
				     sizeof(*nand_chip.buffers));
	nand_chip.oob_poi = nand_chip.buffers->databuf + mtd->writesize;
	/* setup flash layout (does not scan as we override that) */
	mtd->size = nand_chip.chipsize;
	nand_chip.scan_bbt(mtd);
	mxs_nand_setup_ecc(mtd);
}

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dst)
{
	unsigned int sz;
	unsigned int block, lastblock;
	unsigned int page, page_offset;
	unsigned int nand_page_per_block;
	struct nand_chip *chip;
	u8 *page_buf = NULL;

	chip = mtd_to_nand(mtd);
	if (!chip->numchips)
		return -ENODEV;

	page_buf = malloc(mtd->writesize);
	if (!page_buf)
		return -ENOMEM;

	/* offs has to be aligned to a page address! */
	block = offs / mtd->erasesize;
	lastblock = (offs + size - 1) / mtd->erasesize;
	page = (offs % mtd->erasesize) / mtd->writesize;
	page_offset = offs % mtd->writesize;
	nand_page_per_block = mtd->erasesize / mtd->writesize;

	while (block <= lastblock && size > 0) {
		if (!is_badblock(mtd, mtd->erasesize * block, 1)) {
			/* Skip bad blocks */
			while (page < nand_page_per_block && size) {
				int curr_page = nand_page_per_block * block + page;

				if (mxs_read_page_ecc(mtd, page_buf, curr_page) < 0) {
					free(page_buf);
					return -EIO;
				}

				if (size > (mtd->writesize - page_offset))
					sz = (mtd->writesize - page_offset);
				else
					sz = size;

				memcpy(dst, page_buf + page_offset, sz);
				dst += sz;
				size -= sz;
				page_offset = 0;
				page++;
			}

			page = 0;
		} else {
			lastblock++;
		}

		block++;
	}

	free(page_buf);

	return 0;
}

int nand_default_bbt(struct mtd_info *mtd)
{
	return 0;
}

unsigned int nand_page_size(void)
{
	return nand_to_mtd(&nand_chip)->writesize;
}

void nand_deselect(void)
{
}

u32 nand_spl_adjust_offset(u32 sector, u32 offs)
{
	unsigned int block, lastblock;

	block = sector / mtd->erasesize;
	lastblock = (sector + offs) / mtd->erasesize;

	while (block <= lastblock) {
		if (is_badblock(mtd, block * mtd->erasesize, 1)) {
			offs += mtd->erasesize;
			lastblock++;
		}

		block++;
	}

	return offs;
}
