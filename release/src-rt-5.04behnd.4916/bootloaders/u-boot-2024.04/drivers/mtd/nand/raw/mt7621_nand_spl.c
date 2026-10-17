// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <image.h>
#include <malloc.h>
#include <linux/sizes.h>
#include <linux/delay.h>
#include <linux/mtd/rawnand.h>
#include "mt7621_nand.h"

static struct mt7621_nfc nfc_dev;
static u8 *buffer;
static int nand_valid;

static void nand_command_lp(struct mtd_info *mtd, unsigned int command,
			    int column, int page_addr)
{
	register struct nand_chip *chip = mtd_to_nand(mtd);

	/* Command latch cycle */
	chip->cmd_ctrl(mtd, command, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);

	if (column != -1 || page_addr != -1) {
		int ctrl = NAND_CTRL_CHANGE | NAND_NCE | NAND_ALE;

		/* Serially input address */
		if (column != -1) {
			chip->cmd_ctrl(mtd, column, ctrl);
			ctrl &= ~NAND_CTRL_CHANGE;
			if (command != NAND_CMD_READID)
				chip->cmd_ctrl(mtd, column >> 8, ctrl);
		}
		if (page_addr != -1) {
			chip->cmd_ctrl(mtd, page_addr, ctrl);
			chip->cmd_ctrl(mtd, page_addr >> 8,
				       NAND_NCE | NAND_ALE);
			if (chip->options & NAND_ROW_ADDR_3)
				chip->cmd_ctrl(mtd, page_addr >> 16,
					       NAND_NCE | NAND_ALE);
		}
	}
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	/*
	 * Program and erase have their own busy handlers status, sequential
	 * in and status need no delay.
	 */
	switch (command) {
	case NAND_CMD_STATUS:
	case NAND_CMD_READID:
	case NAND_CMD_SET_FEATURES:
		return;

	case NAND_CMD_READ0:
		chip->cmd_ctrl(mtd, NAND_CMD_READSTART,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);
	}

	/*
	 * Apply this short delay always to ensure that we do wait tWB in
	 * any case on any machine.
	 */
	ndelay(100);

	nand_wait_ready(mtd);
}

static int nfc_read_page_hwecc(struct mtd_info *mtd, void *buf,
			       unsigned int page)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	int ret;

	chip->cmdfunc(mtd, NAND_CMD_READ0, 0x0, page);

	ret = chip->ecc.read_page(mtd, chip, buf, 1, page);
	if (ret < 0 || ret > chip->ecc.strength)
		return -1;

	return 0;
}

static int nfc_read_oob_hwecc(struct mtd_info *mtd, void *buf, u32 len,
			      unsigned int page)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	int ret;

	chip->cmdfunc(mtd, NAND_CMD_READ0, 0x0, page);

	ret = chip->ecc.read_page(mtd, chip, NULL, 1, page);
	if (ret < 0)
		return -1;

	if (len > mtd->oobsize)
		len = mtd->oobsize;

	memcpy(buf, chip->oob_poi, len);

	return 0;
}

static int nfc_check_bad_block(struct mtd_info *mtd, unsigned int page)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	u32 pages_per_block, i = 0;
	int ret;
	u8 bad;

	pages_per_block = 1 << (mtd->erasesize_shift - mtd->writesize_shift);

	/* Read from first/last page(s) if necessary */
	if (chip->bbt_options & NAND_BBT_SCANLASTPAGE) {
		page += pages_per_block - 1;
		if (chip->bbt_options & NAND_BBT_SCAN2NDPAGE)
			page--;
	}

	do {
		ret = nfc_read_oob_hwecc(mtd, &bad, 1, page);
		if (ret)
			return ret;

		ret = bad != 0xFF;

		i++;
		page++;
	} while (!ret && (chip->bbt_options & NAND_BBT_SCAN2NDPAGE) && i < 2);

	return ret;
}

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dest)
{
	struct mt7621_nfc *nfc = &nfc_dev;
	struct nand_chip *chip = &nfc->nand;
	struct mtd_info *mtd = &chip->mtd;
	u32 addr, col, page, chksz;
	bool check_bad = true;

	if (!nand_valid)
		return -ENODEV;

	while (size) {
		if (check_bad || !(offs & mtd->erasesize_mask)) {
			addr = offs & (~mtd->erasesize_mask);
			page = addr >> mtd->writesize_shift;
			if (nfc_check_bad_block(mtd, page)) {
				/* Skip bad block */
				if (addr >= mtd->size - mtd->erasesize)
					return -1;

				offs += mtd->erasesize;
				continue;
			}

			check_bad = false;
		}

		col = offs & mtd->writesize_mask;
		page = offs >> mtd->writesize_shift;
		chksz = min(mtd->writesize - col, (uint32_t)size);

		if (unlikely(chksz < mtd->writesize)) {
			/* Not reading a full page */
			if (nfc_read_page_hwecc(mtd, buffer, page))
				return -1;

			memcpy(dest, buffer + col, chksz);
		} else {
			if (nfc_read_page_hwecc(mtd, dest, page))
				return -1;
		}

		dest += chksz;
		offs += chksz;
		size -= chksz;
	}

	return 0;
}

int nand_default_bbt(struct mtd_info *mtd)
{
	return 0;
}

unsigned long nand_size(void)
{
	if (!nand_valid)
		return 0;

	/* Unlikely that NAND size > 2GBytes */
	if (nfc_dev.nand.chipsize <= SZ_2G)
		return nfc_dev.nand.chipsize;

	return SZ_2G;
}

unsigned int nand_page_size(void)
{
	return nfc_dev.nand.mtd.writesize;
}

void nand_deselect(void)
{
}

void nand_init(void)
{
	struct mtd_info *mtd;
	struct nand_chip *chip;

	if (nand_valid)
		return;

	mt7621_nfc_spl_init(&nfc_dev);

	chip = &nfc_dev.nand;
	mtd = &chip->mtd;
	chip->cmdfunc = nand_command_lp;

	if (mt7621_nfc_spl_post_init(&nfc_dev))
		return;

	mtd->erasesize_shift = ffs(mtd->erasesize) - 1;
	mtd->writesize_shift = ffs(mtd->writesize) - 1;
	mtd->erasesize_mask = (1 << mtd->erasesize_shift) - 1;
	mtd->writesize_mask = (1 << mtd->writesize_shift) - 1;

	buffer = malloc(mtd->writesize);
	if (!buffer)
		return;

	nand_valid = 1;
}
