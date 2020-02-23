#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
    <:copyright-BRCM:2012:DUAL/GPL:standard
    
       Copyright (c) 2012 Broadcom 
       All Rights Reserved
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>

    File: brcmnand_cet.c

   Broadcom NAND Correctable Error Table Support
   ---------------------------------------------
   In case of a single bit correctable error, the block in which correctable error
   occured is refreshed (i.e., read->erase->write the entire block). Following a
   refresh a success value is returned by brcmnand_read() i.e., the error is
   hidden from the file system. The Correctable Error Table (CET) keeps a history
   (bit-vector) of per page correctable errors. If a correctable error happens
   on the same page twice, an error is returned to the file system.

   The CET starts from the opposite end of BBT with 1-bit per page. The CET is
   initialized to all 1's. On the first correctable error the bit corresponding
   to a page is reset. On an erase, all the bits of the corresponding block are
   set. The CET can span across multiple blocks therefore a signature 'CET#'
   where # is the block number is kept in the OOB area of the first page of a
   CET block. Also, the total correctable error count is kept in the second
   page OOB of the first CET block.

   There is an in-memory correctable error table during runtime which is flushed
   to the flash every 10 mins (CET_SYNC_FREQ).

    Description:
   when	who     what
   -----	---	----
   080519	sidc	initial code
   080910  sidc	MLC support
 */

#include <linux/types.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/bitops.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include "brcmnand_priv.h"

#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING

#define PRINTK(...)

#define BBT_SLC_PARTITION       (1 << 20)
#define BBT_MAX_BLKS_SLC        4
#define CET_START_BLK_SLC(x, y) (uint32_t)(((x) >> ((y)->bbt_erase_shift)) - (BBT_SLC_PARTITION / (y)->blockSize))

#define BBT_MLC_PARTITION       (4 << 20)
#define BBT_MAX_BLKS_MLC(x)     (BBT_MLC_PARTITION >> ((x)->bbt_erase_shift))
#define CET_START_BLK_MLC(x, y, z)      (uint32_t)(((x) >> ((y)->bbt_erase_shift)) - ((z) / (y)->blockSize))

#define CET_GOOD_BLK    0x00
#define CET_BAD_WEAR    0x01
#define CET_BBT_USE     0x02
#define CET_BAD_FACTORY 0x03

#define CET_SYNC_FREQ   (10 * 60 * HZ)


static char cet_pattern[] = { 'C', 'E', 'T', 0 };
static struct brcmnand_cet_descr cet_descr = {
	.offs		= 9,
	.len		= 4,
	.pattern	= cet_pattern
};

/*
 * This also applies to Large Page SLC flashes with BCH-4 ECC.
 * We don't support BCH-4 on Small Page SLCs because there are not
 * enough free bytes for the OOB, but we don't enforce it,
 * in order to allow page aggregation like in YAFFS2 on small page SLCs.
 */
static struct brcmnand_cet_descr cet_descr_mlc = {
	.offs		= 1,
	.len		= 4,
	.pattern	= cet_pattern
};
static void sync_cet(struct work_struct *work);
static int search_cet_blks(struct mtd_info *, struct brcmnand_cet_descr *, char);
extern char gClearCET;

/*
 * Private: Read OOB area in RAW mode
 */
static inline int brcmnand_cet_read_oob(struct mtd_info *mtd, uint8_t *buf, loff_t offs)
{
	struct mtd_oob_ops ops;

	ops.mode = MTD_OPS_RAW;
	ops.len = mtd->oobsize;
	ops.ooblen = mtd->oobsize;
	ops.datbuf = NULL;
	ops.oobbuf = buf;
	ops.ooboffs = 0;

	return mtd_read_oob(mtd, offs, &ops);
}

/*
 * Private: Write to the OOB area only
 */
static inline int brcmnand_cet_write_oob(struct mtd_info *mtd, uint8_t *buf, loff_t offs)
{
#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	struct mtd_oob_ops ops;

	ops.mode = MTD_OPS_PLACE_OOB;
	ops.len = mtd->writesize;
	ops.ooblen = mtd->oobsize;
	ops.datbuf = NULL;
	ops.oobbuf = buf;
	ops.ooboffs = 0;

	return mtd_write_oob(mtd, offs, &ops);
#else
	struct mtd_oob_ops ops;
	uint8_t databuf[mtd->writesize];

	memset(databuf, 0xff, mtd->writesize);
	ops.mode = MTD_OPS_PLACE_OOB;
	ops.len = mtd->writesize;
	ops.ooblen = mtd->oobsize;
	ops.datbuf = databuf;
	ops.oobbuf = buf;
	ops.ooboffs = 0;

	return mtd_write_oob(mtd, offs, &ops);
#endif
}

/*
 * Private: write one page of data and OOB to flash
 */
static int brcmnand_cet_write(struct mtd_info *mtd, loff_t offs, size_t len,
			      uint8_t *buf, uint8_t *oob)
{
	struct mtd_oob_ops ops;
	int ret;

	ops.mode = MTD_OPS_PLACE_OOB;
	ops.ooboffs = 0;
	ops.ooblen = mtd->oobsize;
	ops.datbuf = buf;
	ops.oobbuf = oob;
	ops.len = len;
	ret = mtd_write_oob(mtd, offs, &ops);

	return ret;
}

/*
 * bitcount - MIT Hackmem count implementation which is O(1)
 * http://infolab.stanford.edu/~manku/bitcount/bitcount.html
 * Counts the number of 1s in a given unsigned int n
 */
static inline int bitcount(uint32_t n)
{
	uint32_t tmp;

	tmp = n - ((n >> 1) & 033333333333)
	      - ((n >> 2) & 011111111111);
	return ((tmp + (tmp >> 3)) & 030707070707) % 63;
}

/*
 * Private debug function: Print OOBs
 */
static void cet_printpg_oob(struct mtd_info *mtd, struct brcmnand_cet_descr *cet, int count)
{
	uint8_t oobbuf[mtd->oobsize];
	loff_t offs;
	int i, gdebug = 0;
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;

	offs = ((loff_t)cet->startblk) << this->bbt_erase_shift;
	if (gdebug) {
		printk(KERN_INFO "%s: %x\n", __FUNCTION__, (unsigned int)offs);
	}
	for (i = 0; i < count; i++) {
		memset(oobbuf, 0, mtd->oobsize);
		if (brcmnand_cet_read_oob(mtd, oobbuf, offs)) {
			return;
		}
		print_oobbuf((const char*)oobbuf, mtd->oobsize);
		offs = offs + cet->sign * this->pageSize;
	}
	return;
}

/*
 * Private debug function: Prints first OOB area of all blocks <block#, page0>
 */
static void cet_printblk_oob(struct mtd_info *mtd, struct brcmnand_cet_descr *cet)
{
	uint8_t *oobbuf;
	loff_t offs;
	int i;
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;

	if ((oobbuf = (uint8_t*)vmalloc(sizeof(uint8_t) * mtd->oobsize)) == NULL) {
		printk(KERN_ERR "brcmnandCET: %s vmalloc failed\n", __FUNCTION__);
		return;
	}
	for (i = 0; i < this->bbt_td->maxblocks; i++) {
		memset(oobbuf, 0, mtd->oobsize);
		offs = ((loff_t)cet->startblk + ((cet->sign) * i)) << this->bbt_erase_shift;
		if (brcmnand_cet_read_oob(mtd, oobbuf, offs)) {
			vfree(oobbuf);
			return;
		}
		print_oobbuf((const char*)oobbuf, mtd->oobsize);
	}
	vfree(oobbuf);
	return;
}

/*
 * Private debug function: erase all blocks belonging to CET
 * Use for testing purposes only
 */
static void cet_eraseall(struct mtd_info *mtd, struct brcmnand_cet_descr *cet)
{
	int i, ret;
	loff_t from;
	struct erase_info einfo;
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;
	int gdebug = 0;

	for (i = 0; i < cet->numblks; i++) {
		if (cet->memtbl[i].blk != -1) {
			from = (uint64_t)cet->memtbl[i].blk << this->bbt_erase_shift;
			if (unlikely(gdebug)) {
				printk(KERN_INFO "DEBUG -> Erasing blk %x\n", cet->memtbl[i].blk);
			}
			memset(&einfo, 0, sizeof(einfo));
			einfo.mtd = mtd;
			einfo.addr = from;
			einfo.len = mtd->erasesize;
			ret = this->erase_bbt(mtd, &einfo, 1, 1);
			if (unlikely(ret < 0)) {
				printk(KERN_ERR "brcmnandCET: %s Error erasing block %llx\n", __FUNCTION__, einfo.addr);
			}
		}
	}

	return;
}

/*
 * Private: Check if a block is factory marked bad block
 * Derived from brcmnand_isbad_bbt()
 * Return values:
 * 0x00 Good block
 * 0x01 Marked bad due to wear
 * 0x02 Reserved for BBT
 * 0x03 Factory marked bad
 */
static inline int check_badblk(struct mtd_info *mtd, loff_t offs)
{
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;
	uint32_t blk;
	int res;

	blk = (uint32_t)(offs >> (this->bbt_erase_shift - 1));
	res = (this->bbt[blk >> 3] >> blk & 0x06) & 0x03;

	return res;
}

/*
 * Check for CET pattern in the OOB buffer
 * return the blk number present in the CET
 */
static inline int found_cet_pattern(struct brcmnand_chip *this, uint8_t *buf)
{
	struct brcmnand_cet_descr *cet = this->cet;
	int i;

	for (i = 0; i < cet->len - 1; i++) {
		if (buf[cet->offs + i] != cet_pattern[i]) {
			return -1;
		}
	}
	return (int)buf[cet->offs + cet->len - 1];
}

/*
 * Check for BBT/Mirror BBT pattern
 * Similar to the implementation in brcmnand_bbt.c
 */
static inline int found_bbt_pattern(uint8_t *buf, struct nand_bbt_descr *bd)
{
	int i;

	for (i = 0; i < bd->len; i++) {
		if (buf[bd->offs + i] != bd->pattern[i]) {
			return 0;
		}
	}
	return 1;
}

/*
 * Check OOB area to test if the block is erased
 */
static inline int cet_iserased(struct mtd_info *mtd, uint8_t *oobbuf)
{
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;
	struct nand_ecclayout *oobinfo = this->ecclayout;
	int i;

	for (i = 0; i < oobinfo->eccbytes; i++) {
		if (oobbuf[oobinfo->eccpos[i]] != 0xff) {
			return 0;
		}
	}
	return 1;
}

/*
 * Process kernel command line showcet
 * If the CET is loaded, display which blocks of flash the CET is in
 */
static inline void cmdline_showcet(struct mtd_info *mtd, struct brcmnand_cet_descr *cet)
{
	int i;
	loff_t offs;
	uint8_t oobbuf[mtd->oobsize];
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;

	if (cet->flags == BRCMNAND_CET_DISABLED) {
		printk(KERN_INFO "brcmnandCET: Disabled\n");
		return;
	}
	printk(KERN_INFO "brcmnandCET: Correctable error count is 0x%x\n", cet->cerr_count);
	if (cet->flags == BRCMNAND_CET_LAZY) {
		printk(KERN_INFO "brcmnandCET: Deferred until next correctable error\n");
		return;
	}
	printk(KERN_INFO "brcmnandCET: Displaying first OOB area of all CET blocks ...\n");
	for (i = 0; i < cet->numblks; i++) {
		if (cet->memtbl[i].blk == -1)
			continue;
		offs = ((loff_t)cet->memtbl[i].blk) << this->bbt_erase_shift;
		printk(KERN_INFO "brcmnandCET: Block[%d] @ %x\n", i, (unsigned int)offs);
		if (brcmnand_cet_read_oob(mtd, oobbuf, offs)) {
			return;
		}
		print_oobbuf((const char*)oobbuf, mtd->oobsize);
	}
	return;
}

/*
 * Reset CET to all 0xffs
 */
static inline int cmdline_resetcet(struct mtd_info *mtd, struct brcmnand_cet_descr *cet)
{
	int i;

	cet_eraseall(mtd, cet);
	for (i = 0; i < cet->numblks; i++) {
		cet->memtbl[i].isdirty = 0;
		cet->memtbl[i].blk = -1;
		cet->memtbl[i].bitvec = NULL;
	}
	printk(KERN_INFO "brcmnandCET: Recreating ... \n");

	return search_cet_blks(mtd, cet, 0);
}

/*
 * Create a CET pattern in the OOB area.
 */
static int create_cet_blks(struct mtd_info *mtd, struct brcmnand_cet_descr *cet)
{
	int i, j, ret, gdebug = 0;
	loff_t from;
	struct nand_bbt_descr *td, *md;
	struct erase_info einfo;
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;
	uint8_t oobbuf[mtd->oobsize];
	char *oobptr, count = 0;

	td = this->bbt_td;
	md = this->bbt_md;
	if (unlikely(gdebug)) {
		printk(KERN_INFO "brcmnandCET: Inside %s\n", __FUNCTION__);
	}
	for (i = 0; i < td->maxblocks; i++) {
		from = ((loff_t)cet->startblk + i * cet->sign) << this->bbt_erase_shift;
		/* Skip if bad block */
		ret = check_badblk(mtd, from);
		if (ret == CET_BAD_FACTORY || ret == CET_BAD_WEAR) {
			continue;
		}
		memset(oobbuf, 0, mtd->oobsize);
		if (brcmnand_cet_read_oob(mtd, oobbuf, from)) {
			printk(KERN_INFO "brcmnandCET: %s %d Error reading OOB\n", __FUNCTION__, __LINE__);
			return -1;
		}
		/* If BBT/MBT block found  we have no space left */
		if (found_bbt_pattern(oobbuf, td) || found_bbt_pattern(oobbuf, md)) {
			printk(KERN_INFO "brcmnandCET: %s blk %x is BBT\n", __FUNCTION__, cet->startblk + i * cet->sign);
			return -1;
		}
		//if (!cet_iserased(mtd, oobbuf)) {
		if (unlikely(gdebug)) {
			printk(KERN_INFO "brcmnandCET: block %x is erased\n", cet->startblk + i * cet->sign);
		}
		/* Erase */
		memset(&einfo, 0, sizeof(einfo));
		einfo.mtd = mtd;
		einfo.addr = from;
		einfo.len = mtd->erasesize;
		ret = this->erase_bbt(mtd, &einfo, 1, 1);
		if (unlikely(ret < 0)) {
			printk(KERN_ERR "brcmnandCET: %s Error erasing block %x\n", __FUNCTION__, cet->startblk + i * cet->sign);
			return -1;
		}
		//}
		/* Write 'CET#' pattern to the OOB area */
		memset(oobbuf, 0xff, mtd->oobsize);
		if (unlikely(gdebug)) {
			printk(KERN_INFO "brcmnandCET: writing CET %d to OOB area\n", (int)count);
		}
		oobptr = (char*)oobbuf;
		for (j = 0; j < cet->len - 1; j++) {
			oobptr[cet->offs + j] = cet->pattern[j];
		}
		oobptr[cet->offs + j] = count;
		if (brcmnand_cet_write_oob(mtd, oobbuf, from)) {
			printk(KERN_ERR "brcmnandCET: %s Error writing to OOB# %x\n", __FUNCTION__, (unsigned int)from);
			return -1;
		}
		/* If this is the first CET block, init the correctable erase count to 0 */
		if (count == 0) {
			memset(oobbuf, 0xff, mtd->oobsize);
			oobptr = (char*)oobbuf;
			*((uint32_t*)(oobptr + cet->offs)) = 0x00000000;
			from += this->pageSize;
			if (unlikely(gdebug)) {
				printk(KERN_INFO "DEBUG -> 0: from = %x\n", (unsigned int)from);
				printk(KERN_INFO "brcmnandCET: Writing cer_count to page %x\n", (unsigned int)from);
			}
			if (brcmnand_cet_write_oob(mtd, oobbuf, from)) {
				printk(KERN_INFO "brcmnandCET: %s Error writing to OOB# %x\n", __FUNCTION__, (unsigned int)from);
				return -1;
			}
		}
		count++;
		if (((int)count) == cet->numblks) {
			return 0;
		}
	}
	return -1;
}

/*
 * Search for CET blocks
 * force => 1 Force creation of tables, do not defer for later
 */
static int search_cet_blks(struct mtd_info *mtd, struct brcmnand_cet_descr *cet, char force)
{
	int i, count = 0, ret;
	loff_t from;
	struct nand_bbt_descr *td, *md;
	uint8_t oobbuf[mtd->oobsize];
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;
	int gdebug = 0;

	td = this->bbt_td;
	md = this->bbt_md;
	if (unlikely(gdebug)) {
		printk(KERN_INFO "DEBUG -> Inside search_cet_blks\n");
	}
	for (i = 0; i < td->maxblocks; i++) {
		from = ((loff_t)cet->startblk + i * cet->sign) << this->bbt_erase_shift;
		/* Skip if bad block */
		ret = check_badblk(mtd, from);
		if (ret == CET_BAD_FACTORY || ret == CET_BAD_WEAR) {
			continue;
		}
		/* Read the OOB area of the first page of the block */
		memset(oobbuf, 0, mtd->oobsize);
		if (brcmnand_cet_read_oob(mtd, oobbuf, from)) {
			printk(KERN_INFO "brcmnandCET: %s %d Error reading OOB\n", __FUNCTION__, __LINE__);
			cet->flags = BRCMNAND_CET_DISABLED;
			return -1;
		}
		if (unlikely(gdebug)) {
			print_oobbuf(oobbuf, mtd->oobsize);
		}
		/* Return -1 if BBT/MBT block => no space left for CET */
		if (found_bbt_pattern(oobbuf, td) || found_bbt_pattern(oobbuf, md)) {
			printk(KERN_INFO "brcmnandCET: %s blk %x is BBT\n", __FUNCTION__, cet->startblk + i * cet->sign);
			cet->flags = BRCMNAND_CET_DISABLED;
			return -1;
		}
		/* Check for CET pattern */
		ret = found_cet_pattern(this, oobbuf);
		if (unlikely(gdebug)) {
			print_oobbuf((const char*)oobbuf, mtd->oobsize);
		}
		if (ret < 0 || ret >= cet->numblks) {
			/* No CET pattern found due to
			   1. first time being booted => normal so create
			   2. Did not find CET pattern when we're supposed to
			      error => recreate, in either case we call create_cet_blks();
			   3. Found an incorrect > cet->numblks count => error => recreate
			 */
			printk(KERN_INFO "brcmnandCET: Did not find CET, recreating\n");
			if (create_cet_blks(mtd, cet) < 0) {
				cet->flags = BRCMNAND_CET_DISABLED;
				return ret;
			}
			cet->flags = BRCMNAND_CET_LAZY;
			return 0;
		}
		/* Found CET pattern */
		if (unlikely(gdebug)) {
			printk(KERN_INFO "brcmnandCET: Found CET block#%d\n", count);
		}
		/* If this is the first block do some extra stuff ... */
		if (count == 0) {
			/* The global cerr_count is in the 2nd page's OOB area */
			from += this->pageSize;
			if (brcmnand_cet_read_oob(mtd, oobbuf, from)) {
				printk(KERN_ERR "brcmnandCET: %s %d Error reading OOB\n", __FUNCTION__, __LINE__);
				cet->flags = BRCMNAND_CET_DISABLED;
				return -1;
			}
			cet->cerr_count = *((uint32_t*)(oobbuf + cet->offs));
			/* TODO - Fix this -> recreate */
			if (cet->cerr_count == 0xffffffff) {
				/* Reset it to 0 */
				cet->cerr_count = 0;
				cet->memtbl[0].isdirty = 1;
			}
			if (unlikely(gdebug)) {
				printk(KERN_INFO "brcmnandCET: correctable error count = %x\n", cet->cerr_count);
			}
			/* If force then go thru all CET blks even if cerr_count is 0 */
			if (!force) {
				if (cet->cerr_count == 0) {
					cet->flags = BRCMNAND_CET_LAZY;
					return 0;
				}
			}
		}
		cet->memtbl[ret].blk = cet->startblk + i * cet->sign;
		count++;
#if 0
		printk(KERN_INFO "DEBUG -> count = %d, nblks = %d blk = %d\n", count, cet->numblks, cet->memtbl[ret].blk);
#endif
		if (count == cet->numblks) {
			cet->flags = BRCMNAND_CET_LOADED;
			return 0;
		}
	}
	/* This should never happen */
	cet->flags = BRCMNAND_CET_DISABLED;
	return -1;
}

/*
 * flush pending in-memory CET data to the flash. Called as part of a
 * callback function from workqueue that is invoked every SYNC_FREQ seconds
 */
static int flush_memcet(struct mtd_info *mtd)
{
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;
	struct brcmnand_cet_descr *cet = this->cet;
	struct erase_info einfo;
	int i, j, k = 0, ret, pg_idx = 0, gdebug = 0;
	uint8_t oobbuf[mtd->oobsize];
	loff_t from, to;
	char *oobptr, count = 0;

	/* If chip is locked reset timer for a later time */
	if (spin_is_locked(&this->ctrl->chip_lock)) {
		printk(KERN_INFO "brcmnandCET: flash locked reseting timer\n");
		return -1;
	}
	if (unlikely(gdebug)) {
		printk(KERN_INFO "brcmnandCET: Inside %s\n", __FUNCTION__);
	}
	/* For each in-mem dirty block, sync with flash
	   sync => erase -> write */
	for (i = 0; i < cet->numblks; i++) {
		if (cet->memtbl[i].isdirty && cet->memtbl[i].blk != -1) {
			/* Erase */
			from = ((loff_t)cet->memtbl[i].blk) << this->bbt_erase_shift;
			to = from;
			memset(&einfo, 0, sizeof(einfo));
			einfo.mtd = mtd;
			einfo.addr = from;
			einfo.len = mtd->erasesize;
			ret = this->erase_bbt(mtd, &einfo, 1, 1);
			if (unlikely(ret < 0)) {
				printk(KERN_ERR "brcmnandCET: %s Error erasing block %x\n", __FUNCTION__, cet->memtbl[i].blk);
				return -1;
			}
			if (unlikely(gdebug)) {
				printk(KERN_INFO "DEBUG -> brcmnandCET: After erasing ...\n");
				cet_printpg_oob(mtd, cet, 3);
			}
			pg_idx = 0;
			/* Write pages i.e., flush */
			for (j = 0; j < mtd->erasesize / this->pageSize; j++) {
				memset(oobbuf, 0xff, mtd->oobsize);
				oobptr = (char*)oobbuf;
				if (j == 0) { /* Write CET# */
					for (k = 0; k < cet->len - 1; k++) {
						oobptr[cet->offs + k] = cet->pattern[k];
					}
					oobptr[cet->offs + k] = count;
					if (unlikely(gdebug)) {
						print_oobbuf((const char*)oobbuf, mtd->oobsize);
					}
				}
				if (j == 1 && count == 0) { /* Write cerr_count */
					*((uint32_t*)(oobptr + cet->offs)) = cet->cerr_count;
				}
				ret = brcmnand_cet_write(mtd, to, (size_t)this->pageSize, cet->memtbl[i].bitvec + pg_idx, oobbuf);
				if (ret < 0) {
					printk(KERN_ERR "brcmnandCET: %s Error writing to page %x\n", __FUNCTION__, (unsigned int)to);
					return ret;
				}
				to += mtd->writesize;
				pg_idx += mtd->writesize;
			}
			cet->memtbl[i].isdirty = 0;
			if (unlikely(gdebug)) {
				printk(KERN_INFO "brcmnandCET: flushing CET block %d\n", i);
			}
		}
		count++;
	}

	return 0;
}

/*
 * The callback function for kernel workq task
 * Checks if there is any work to be done, if so calls flush_memcet
 * Resets timer before returning in any case
 */
static void sync_cet(struct work_struct *work)
{
	int i;
	struct delayed_work *d = container_of(work, struct delayed_work, work);
	struct brcmnand_cet_descr *cet = container_of(d, struct brcmnand_cet_descr, cet_flush);
	struct mtd_info *mtd = cet->mtd;

	/* Check if all blocks are clean */
	for (i = 0; i < cet->numblks; i++) {
		if (cet->memtbl[i].isdirty) break;
	}
	/* Avoid function call cost if there are no dirty blocks */
	if (i != cet->numblks)
		flush_memcet(mtd);
	schedule_delayed_work(&cet->cet_flush, CET_SYNC_FREQ);

	return;
}


/*
 * brcmnand_create_cet - Create a CET (Correctable Error Table)
 * @param mtd		MTD device structure
 *
 * Called during mtd init. Checks if a CET already exists or needs
 * to be created. Initializes in-memory CET.
 */
int brcmnand_create_cet(struct mtd_info *mtd)
{
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;
	struct brcmnand_cet_descr *cet;
	int gdebug = 0, i, ret, rem;
	uint64_t tmpdiv;

	if (unlikely(gdebug)) {
		printk(KERN_INFO "brcmnandCET: Creating correctable error table ...\n");
	}

	if (NAND_IS_MLC(this) || /* MLC flashes */
	    /* SLC w/ BCH-n; We don't check for pageSize, and let it be */
	    (this->ecclevel >= BRCMNAND_ECC_BCH_1 && this->ecclevel <= BRCMNAND_ECC_BCH_12)) {
		this->cet = cet = &cet_descr_mlc;
		if (gdebug) printk("%s: CET = cet_desc_mlc\n", __FUNCTION__);
	}else  {
		this->cet = cet = &cet_descr;
		if (gdebug) printk("%s: CET = cet_descr\n", __FUNCTION__);
	}
	cet->flags = 0x00;
	/* Check that BBT table and mirror exist */
	if (unlikely(!this->bbt_td && !this->bbt_md)) {
		printk(KERN_INFO "brcmnandCET: BBT tables not found, disabling\n");
		cet->flags = BRCMNAND_CET_DISABLED;
		return -1;
	}
	/* Per chip not supported. We do not use per chip BBT, but this
	   is just a safety net */
	if (unlikely(this->bbt_td->options & NAND_BBT_PERCHIP)) {
		printk(KERN_INFO "brcmnandCET: per chip CET not supported, disabling\n");
		cet->flags = BRCMNAND_CET_DISABLED;
		return -1;
	}
	/* Calculate max blocks based on 1-bit per page */
	tmpdiv = this->mtdSize;
	do_div(tmpdiv, this->pageSize);
	do_div(tmpdiv, (8 * this->blockSize));
	cet->numblks = (uint32_t)tmpdiv;
	//cet->numblks = (this->mtdSize/this->pageSize)/(8*this->blockSize);
	tmpdiv = this->mtdSize;
	do_div(tmpdiv, this->pageSize);
	do_div(tmpdiv, 8);
	rem = do_div(tmpdiv, this->blockSize);
	//if (((this->mtdSize/this->pageSize)/8)%this->blockSize) {
	if (rem) {
		cet->numblks++;
	}
	/* Allocate twice the size in case we have bad blocks */
	cet->maxblks = cet->numblks * 2;
	/* Determine the direction of CET based on reverse direction of BBT */
	cet->sign = (this->bbt_td->options & NAND_BBT_LASTBLOCK) ? 1 : -1;
	/* For flash size <= 512MB BBT and CET share the last 1MB
	   for flash size > 512MB CET is at the 512th MB of flash */
#if 0
	if (NAND_IS_MLC(this)) {
	} else {
		if (this->mtdSize < (1 << 29)) {
			if (cet->maxblks + BBT_MAX_BLKS > get_bbt_partition(this) / this->blockSize) {
				printk(KERN_INFO "brcmnandCET: Not enough space to store CET, disabling CET\n");
				cet->flags = BRCMNAND_CET_DISABLED;
				return -1;
			}
			if (cet->sign) {
				cet->startblk = CET_START_BLK(this->mtdSize, this);
			} else {
				cet->startblk = (uint32_t)(this->mtdSize >> this->bbt_erase_shift) - 1;
			}

		} else {
			if (cet->maxblks > (get_bbt_partition(this)) / this->blockSize) {
				printk(KERN_INFO "brcmnandCET: Not enough space to store CET, disabling CET\n");
				cet->flags = BRCMNAND_CET_DISABLED;
				return -1;
			}
			cet->startblk = CET_START_BLK((1 << 29), this);
		}
	}
#endif
	if (NAND_IS_MLC(this)) {
		if (this->mtdSize < (1 << 29)) {
			if (cet->maxblks + BBT_MAX_BLKS_MLC(this) > BBT_MLC_PARTITION / this->blockSize) {
				printk(KERN_INFO "brcmnandCET: Not enough space to store CET, disabling CET\n");
				cet->flags = BRCMNAND_CET_DISABLED;
				return -1;
			}
			/* Reverse direction of BBT */
			if (cet->sign) {
				cet->startblk = CET_START_BLK_MLC(this->mtdSize, this, BBT_MLC_PARTITION);
			} else {
				cet->startblk = (uint32_t)(this->mtdSize >> this->bbt_erase_shift) - 1;
			}
		} else {
			/* 512th MB used by CET */
			if (cet->maxblks > (1 << 29) / this->blockSize) {
				printk(KERN_INFO "brcmnandCET: Not enough space to store CET, disabling CET\n");
				cet->flags = BRCMNAND_CET_DISABLED;
				return -1;
			}
#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
			cet->startblk = CET_START_BLK_SLC(this->mtdSize, this);
#else
			cet->startblk = CET_START_BLK_MLC((1 << 29), this, (1 << 20));
#endif
		}
	} else {
		if (this->mtdSize < (1 << 29)) {
			if (cet->maxblks + BBT_MAX_BLKS_SLC > BBT_SLC_PARTITION / this->blockSize) {
				printk(KERN_INFO "brcmnandCET: Not enough space to store CET, disabling CET\n");
				cet->flags = BRCMNAND_CET_DISABLED;
				return -1;
			}
			/* Reverse direction of BBT */
			if (cet->sign) {
				cet->startblk = CET_START_BLK_SLC(this->mtdSize, this);
			} else {
				cet->startblk = (uint32_t)(this->mtdSize >> this->bbt_erase_shift) - 1;
			}
		} else {
			/* 512th MB used by CET */
			if (cet->maxblks > BBT_SLC_PARTITION / this->blockSize) {
				printk(KERN_INFO "brcmnandCET: Not enough space to store CET, disabling CET\n");
				cet->flags = BRCMNAND_CET_DISABLED;
				return -1;
			}
#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
			cet->startblk = CET_START_BLK_SLC(this->mtdSize, this);
#else
			cet->startblk = CET_START_BLK_SLC((1 << 29), this);
#endif
		}
	}
	if (gdebug) {
		printk(KERN_INFO "brcmnandCET: start blk = %x, numblks = %x\n", cet->startblk, cet->numblks);
	}

	/* Init memory based CET */
	cet->memtbl = (struct brcmnand_cet_memtable *)vmalloc(cet->numblks * sizeof(struct brcmnand_cet_memtable));
	if (cet->memtbl == NULL) {
		printk(KERN_ERR "brcmnandCET: vmalloc failed %s\n", __FUNCTION__);
		cet->flags = BRCMNAND_CET_DISABLED;
		return -1;
	}
	for (i = 0; i < cet->numblks; i++) {
		cet->memtbl[i].isdirty = 0;
		cet->memtbl[i].blk = -1;
		cet->memtbl[i].bitvec = NULL;
	}
	ret = search_cet_blks(mtd, cet, 0);
	if (unlikely(gClearCET == 1)) {         /* kernel cmdline showcet */
		cmdline_showcet(mtd, cet);
	}
	if (unlikely(gClearCET == 2)) {         /* kernel cmdline resetcet */
		if (cmdline_resetcet(mtd, cet) < 0) {
			cet->flags = BRCMNAND_CET_DISABLED;
			return -1;
		}
	}
	if (unlikely(gClearCET == 3)) {         /* kernel cmdline disable */
		cet->flags = BRCMNAND_CET_DISABLED;
		ret = -1;
	}
	//cet_printpg_oob(mtd, cet, 3);
	switch (cet->flags) {
	case BRCMNAND_CET_DISABLED:
		printk(KERN_INFO "brcmnandCET: Status -> Disabled\n");
		break;
	case BRCMNAND_CET_LAZY:
		printk(KERN_INFO "brcmnandCET: Status -> Deferred\n");
		break;
	case BRCMNAND_CET_LOADED:
		printk(KERN_INFO "brcmnandCET: Status -> Loaded\n");
		break;
	default:
		printk(KERN_INFO "brcmnandCET: Status -> Fatal error CET disabled\n");
		cet->flags = BRCMNAND_CET_DISABLED;
		break;
	}
	if (unlikely(gdebug)) {
		cet_printpg_oob(mtd, cet, 3);
		cet_printblk_oob(mtd, cet);
	}

	INIT_DELAYED_WORK(&cet->cet_flush, sync_cet);
	cet->mtd = mtd;
	schedule_delayed_work(&cet->cet_flush, CET_SYNC_FREQ);

	return ret;
}

/*
 * brcmnand_cet_erasecallback: Called every time there is an erase due to
 *                             userspace activity
 *
 * @param mtd		MTD device structure
 * @param addr		Address of the block that was erased by fs/userspace
 *
 * Assumption: cet->flag != BRCMNAND_CET_DISABLED || BRCMNAND_CET_LAZY
 * is checked by the caller
 * flag == BRCMNAND_CET_DISABLED => CET not being used
 * flag == BRCMNAND_CET_LAZY => correctable error count is 0 so need of callback
 *
 * TODO Optimize, add comments, check all return paths
 */
int brcmnand_cet_erasecallback(struct mtd_info *mtd, u_int32_t addr)
{
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;
	struct brcmnand_cet_descr *cet = this->cet;
	uint32_t page = 0;
	int blkbegin, blk, i, ret, retlen, pg_idx = 0, numzeros = 0, byte, gdebug = 0;
	uint32_t *ptr;
	unsigned int pos;
	loff_t origaddr = addr;

	/* Find out which entry in the memtbl does the addr map to */
	page = (uint32_t)(addr >> this->page_shift);
	blk = page / (this->blockSize << 3);
	if (unlikely(cet->memtbl[blk].blk == -1)) {
		printk(KERN_INFO "brcmnandCET: %s invalid block# in CET\n", __FUNCTION__);
		return -1;
	}
	blkbegin = cet->memtbl[blk].blk;
	/* Start page of the block */
	addr = ((loff_t)blkbegin) << this->bbt_erase_shift;
	if (cet->memtbl[blk].bitvec == NULL) {
		if (gdebug) {
			printk(KERN_INFO "DEBUG -> brcmnandCET: bitvec is null, reloading\n");
		}
		/* using kmalloc as the erase typically are called from image update which has interrupt disabled. We can only
		   support 32bit addressing 4GB maximum size NAND. These NAND use 128K block and 2048 byte page. So it takes 4GB/2048/8
		   = 256KB or 2 block. Our kernel should have enough kmalloc ATMOIC memory for 2x128KB allocations */
		cet->memtbl[blk].bitvec = (char*)kmalloc(this->blockSize, GFP_ATOMIC);
		if (cet->memtbl[blk].bitvec == NULL) {
			printk(KERN_INFO "brcmnandCET: %s kmalloc failed\n", __FUNCTION__);
			return -1;
		}

		memset(cet->memtbl[blk].bitvec, 0xff, sizeof(this->blockSize));
		/* Read an entire block */
		for (i = 0; i < mtd->erasesize / mtd->writesize; i++) {
			if (gdebug) {
				printk(KERN_INFO "DEBUG -> brcmnandCET: Reading page %d\n", i);
			}
			ret = mtd_read(mtd, addr, this->pageSize, &retlen, (uint8_t*)(cet->memtbl[blk].bitvec + pg_idx));
			if (ret < 0 || (retlen != this->pageSize)) {
				kfree(cet->memtbl[blk].bitvec);
				return -1;
			}
			pg_idx += mtd->writesize;
			addr += this->pageSize;
		}
	}
	page = (uint32_t)((origaddr & (~(mtd->erasesize - 1))) >> this->page_shift);
	pos = page % (this->blockSize << 3);
	byte = pos / (1 << 3);
	ptr = (uint32_t*)((char*)cet->memtbl[blk].bitvec + byte);
	/* numpages/8bits per byte/4byte per uint32 */
	for (i = 0; i < ((mtd->erasesize / mtd->writesize) >> 3) >> 2; i++) {
		/* Count the number of 0s for in the bitvec */
		numzeros += bitcount(~ptr[i]);
	}
	if (likely(numzeros == 0)) {
		if (gdebug) {
			printk(KERN_INFO "DEBUG -> brcmnandCET: returning 0 numzeros = 0\n");
		}
		return 0;
	}
	if (cet->cerr_count < numzeros) {
		if (gdebug) {
			printk(KERN_ERR "brcmnandCET: Erroneous correctable error count");
		}
		return -1;
	}
	cet->cerr_count -= numzeros;
	/* Make bits corresponding to this block all 1s */
	memset(cet->memtbl[blk].bitvec + byte, 0xff, (mtd->erasesize / mtd->writesize) >> 3);
	cet->memtbl[blk].isdirty = 1;

	return 0;
}

/*
 * brcmnand_cet_update: Called every time a single correctable error is
 *                      encountered.
 * @param mtd		MTD device structure
 * @param from		Page address at which correctable error occured
 * @param status	Return status
 *			1 => This page had a correctable errror in past,
 *			therefore, return correctable error to filesystem
 *			0 => First occurence of a correctable error for
 *			this page. return a success to the filesystem
 *
 * Check the in memory CET bitvector to see if this page (loff_t from)
 * had a correctable error in past, if not set this page's bit to '0'
 * in the bitvector.
 *
 */
int brcmnand_cet_update(struct mtd_info *mtd, loff_t from, int *status)
{
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;
	struct brcmnand_cet_descr *cet = this->cet;
	int gdebug = 0, ret, blk, byte, bit, retlen = 0, blkbegin, i;
	uint32_t page = 0;
	unsigned int pg_idx = 0, pos = 0;
	unsigned char c, mask;

	if (gdebug) {
		printk(KERN_INFO "DEBUG -> brcmnandCET: Inside %s\n", __FUNCTION__);
	}
	if (cet->flags == BRCMNAND_CET_LAZY) {
		/* Force creation of the CET and the mem table */
		ret = search_cet_blks(mtd, cet, 1);
		if (ret < 0) {
			cet->flags = BRCMNAND_CET_DISABLED;
			return ret;
		}
		cet->flags = BRCMNAND_CET_LOADED;
	}
	/* Find out which entry in memtbl does the from address map to */
	page = (uint32_t)(from >> this->page_shift);
	/* each bit is one page << 3 for 8 bits per byte */
	blk = page / (this->blockSize << 3);
	if (unlikely(cet->memtbl[blk].blk == -1)) {
		printk(KERN_INFO "brcmnandCET: %s invalid block# in CET\n", __FUNCTION__);
		return -1;
	}
	blkbegin = cet->memtbl[blk].blk;
	/* Start page of the block */
	from = ((loff_t)blkbegin) << this->bbt_erase_shift;
	/* If bitvec == NULL, load the block from flash */
	if (cet->memtbl[blk].bitvec == NULL) {
		if (gdebug) {
			printk(KERN_INFO "DEBUG -> brcmnandCET: bitvec null .... loading ...\n");
		}
		/* using kmalloc to be consisent with erasecallback function. see erasecallback for details */
		cet->memtbl[blk].bitvec = (char*)kmalloc(this->blockSize, GFP_ATOMIC);
		if (cet->memtbl[blk].bitvec == NULL) {
			printk(KERN_ERR "brcmnandCET: %s kmalloc failed\n", __FUNCTION__);
			return -1;
		}
		memset(cet->memtbl[blk].bitvec, 0xff, this->blockSize);
		/* Read an entire block */
		if (gdebug) {
			printk(KERN_INFO "DEBUG -> brcmnandCET: Reading pages starting @ %x\n", (unsigned int)from);
		}
		for (i = 0; i < mtd->erasesize / mtd->writesize; i++) {
			ret = mtd_read(mtd, from, this->pageSize, &retlen, (uint8_t*)(cet->memtbl[blk].bitvec + pg_idx));
			if (ret < 0 || (retlen != this->pageSize)) {
				kfree(cet->memtbl[blk].bitvec);
				return -1;
			}
			pg_idx += mtd->writesize;
			from += this->pageSize;
		}
	}
	pos = page % (this->blockSize << 3);
	byte = pos / (1 << 3);
	bit = pos % (1 << 3);
	c = cet->memtbl[blk].bitvec[byte];
	mask = 1 << bit;
	if ((c & mask) == mask) { /* First time error mark it but return a good status */
		*status = 0;
		c = (c & ~mask);
		cet->memtbl[blk].bitvec[byte] = c;
		cet->memtbl[blk].isdirty = 1;
	} else {
		*status = 1; /* This page had a previous error so return a bad status */
	}
	cet->cerr_count++;
#if 0
	printk(KERN_INFO "DEBUG -> count = %d, byte = %d, bit = %d, blk = %x status = %d c = %d addr = %x\n", cet->cerr_count \
	       , byte, bit, blk, *status, cet->memtbl[blk].bitvec[byte], cet->memtbl[blk].bitvec + byte);
	printk(KERN_INFO "DEBUG -> CET: Exiting %s\n", __FUNCTION__);
#endif

	return 0;
}

EXPORT_SYMBOL(brcmnand_cet_update);

/*
 * brcmnand_cet_prepare_reboot Call flush_memcet to flush any in-mem dirty data
 *
 * @param mtd		MTD device structure
 *
 * Flush any pending in-mem CET blocks to flash before reboot
 */
int brcmnand_cet_prepare_reboot(struct mtd_info *mtd)
{
	int gdebug = 0;
	struct brcmnand_chip *this = (struct brcmnand_chip *)mtd->priv;
	struct brcmnand_cet_descr *cet = this->cet;

#if 0
	// Disable for MLC
	if (NAND_IS_MLC(this)) {
		return 0;
	}
#endif
	if (unlikely(gdebug)) {
		printk(KERN_INFO "DEBUG -> brcmnandCET: flushing pending CET\n");
	}
	if (unlikely(cet->flags == BRCMNAND_CET_DISABLED)) {
		return 0;
	}
	flush_memcet(mtd);

	return 0;
}


#endif

#endif
