/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2007 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* XXX U-BOOT XXX */
#include <common.h>

#include "yportenv.h"


#include "yaffs_mtdif.h"

#include <linux/mtd/mtd.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/mtd/rawnand.h>


static inline void translate_spare2oob(const struct yaffs_spare *spare, u8 *oob)
{
	oob[0] = spare->tb0;
	oob[1] = spare->tb1;
	oob[2] = spare->tb2;
	oob[3] = spare->tb3;
	oob[4] = spare->tb4;
	oob[5] = spare->tb5 & 0x3f;
	oob[5] |= spare->block_status == 'Y' ? 0 : 0x80;
	oob[5] |= spare->page_status == 0 ? 0 : 0x40;
	oob[6] = spare->tb6;
	oob[7] = spare->tb7;
}

static inline void translate_oob2spare(struct yaffs_spare *spare, u8 *oob)
{
	struct yaffs_nand_spare *nspare = (struct yaffs_nand_spare *)spare;
	spare->tb0 = oob[0];
	spare->tb1 = oob[1];
	spare->tb2 = oob[2];
	spare->tb3 = oob[3];
	spare->tb4 = oob[4];
	spare->tb5 = oob[5] == 0xff ? 0xff : oob[5] & 0x3f;
	spare->block_status = oob[5] & 0x80 ? 0xff : 'Y';
	spare->page_status = oob[5] & 0x40 ? 0xff : 0;
	spare->ecc1[0] = spare->ecc1[1] = spare->ecc1[2] = 0xff;
	spare->tb6 = oob[6];
	spare->tb7 = oob[7];
	spare->ecc2[0] = spare->ecc2[1] = spare->ecc2[2] = 0xff;

	nspare->eccres1 = nspare->eccres2 = 0; /* FIXME */
}


int nandmtd_WriteChunkToNAND(struct yaffs_dev *dev, int chunkInNAND,
			     const u8 *data, const struct yaffs_spare *spare)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->driver_context);
	struct mtd_oob_ops ops;
	size_t dummy;
	int retval = 0;
	loff_t addr = ((loff_t) chunkInNAND) * dev->data_bytes_per_chunk;
	u8 spareAsBytes[8]; /* OOB */

	if (data && !spare)
		retval = mtd_write(mtd, addr, dev->data_bytes_per_chunk,
				&dummy, data);
	else if (spare) {
		if (dev->param.use_nand_ecc) {
			translate_spare2oob(spare, spareAsBytes);
			ops.mode = MTD_OPS_AUTO_OOB;
			ops.ooblen = 8; /* temp hack */
		} else {
			ops.mode = MTD_OPS_RAW;
			ops.ooblen = YAFFS_BYTES_PER_SPARE;
		}
		ops.len = data ? dev->data_bytes_per_chunk : ops.ooblen;
		ops.datbuf = (u8 *)data;
		ops.ooboffs = 0;
		ops.oobbuf = spareAsBytes;
		retval = mtd_write_oob(mtd, addr, &ops);
	}

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd_ReadChunkFromNAND(struct yaffs_dev *dev, int chunkInNAND, u8 *data,
			      struct yaffs_spare *spare)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->driver_context);
	struct mtd_oob_ops ops;
	size_t dummy;
	int retval = 0;

	loff_t addr = ((loff_t) chunkInNAND) * dev->data_bytes_per_chunk;
	u8 spareAsBytes[8]; /* OOB */

	if (data && !spare)
		retval = mtd_read(mtd, addr, dev->data_bytes_per_chunk,
				&dummy, data);
	else if (spare) {
		if (dev->param.use_nand_ecc) {
			ops.mode = MTD_OPS_AUTO_OOB;
			ops.ooblen = 8; /* temp hack */
		} else {
			ops.mode = MTD_OPS_RAW;
			ops.ooblen = YAFFS_BYTES_PER_SPARE;
		}
		ops.len = data ? dev->data_bytes_per_chunk : ops.ooblen;
		ops.datbuf = data;
		ops.ooboffs = 0;
		ops.oobbuf = spareAsBytes;
		retval = mtd_read_oob(mtd, addr, &ops);
		if (dev->param.use_nand_ecc)
			translate_oob2spare(spare, spareAsBytes);
	}

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd_EraseBlockInNAND(struct yaffs_dev *dev, int blockNumber)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->driver_context);
	__u32 addr =
	    ((loff_t) blockNumber) * dev->data_bytes_per_chunk
		* dev->param.chunks_per_block;
	struct erase_info ei;
	int retval = 0;

	ei.mtd = mtd;
	ei.addr = addr;
	ei.len = dev->data_bytes_per_chunk * dev->param.chunks_per_block;
	ei.time = 1000;
	ei.retries = 2;
	ei.callback = NULL;
	ei.priv = (u_long) dev;

	/* Todo finish off the ei if required */


	retval = mtd_erase(mtd, &ei);

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd_InitialiseNAND(struct yaffs_dev *dev)
{
	return YAFFS_OK;
}
