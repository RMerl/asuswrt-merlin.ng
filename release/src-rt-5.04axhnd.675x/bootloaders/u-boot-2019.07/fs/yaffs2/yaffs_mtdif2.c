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

/* mtd interface for YAFFS2 */

/* XXX U-BOOT XXX */
#include <common.h>
#include <linux/errno.h>

#include "yportenv.h"
#include "yaffs_trace.h"

#include "yaffs_mtdif2.h"

#include <linux/mtd/mtd.h>
#include <linux/types.h>
#include <linux/time.h>

#include "yaffs_trace.h"
#include "yaffs_packedtags2.h"

#define yaffs_dev_to_mtd(dev) ((struct mtd_info *)((dev)->driver_context))
#define yaffs_dev_to_lc(dev) ((struct yaffs_linux_context *)((dev)->os_context))


/* NB For use with inband tags....
 * We assume that the data buffer is of size total_bytes_per_chunk so
 * that we can also use it to load the tags.
 */
int nandmtd2_write_chunk_tags(struct yaffs_dev *dev, int nand_chunk,
			      const u8 *data,
			      const struct yaffs_ext_tags *tags)
{
	struct mtd_info *mtd = yaffs_dev_to_mtd(dev);
	struct mtd_oob_ops ops;

	int retval = 0;
	loff_t addr;

	struct yaffs_packed_tags2 pt;

	int packed_tags_size =
	    dev->param.no_tags_ecc ? sizeof(pt.t) : sizeof(pt);
	void *packed_tags_ptr =
	    dev->param.no_tags_ecc ? (void *)&pt.t : (void *)&pt;

	yaffs_trace(YAFFS_TRACE_MTD,
		"nandmtd2_write_chunk_tags chunk %d data %p tags %p",
		nand_chunk, data, tags);

	addr = ((loff_t) nand_chunk) * dev->param.total_bytes_per_chunk;

	/* For yaffs2 writing there must be both data and tags.
	 * If we're using inband tags, then the tags are stuffed into
	 * the end of the data buffer.
	 */
	if (!data || !tags)
		BUG();
	else if (dev->param.inband_tags) {
		struct yaffs_packed_tags2_tags_only *pt2tp;
		pt2tp =
		    (struct yaffs_packed_tags2_tags_only *)(data +
							dev->
							data_bytes_per_chunk);
		yaffs_pack_tags2_tags_only(pt2tp, tags);
	} else {
		yaffs_pack_tags2(&pt, tags, !dev->param.no_tags_ecc);
	}

	ops.mode = MTD_OPS_AUTO_OOB;
	ops.ooblen = (dev->param.inband_tags) ? 0 : packed_tags_size;
	ops.len = dev->param.total_bytes_per_chunk;
	ops.ooboffs = 0;
	ops.datbuf = (u8 *) data;
	ops.oobbuf = (dev->param.inband_tags) ? NULL : packed_tags_ptr;
	retval = mtd_write_oob(mtd, addr, &ops);

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd2_read_chunk_tags(struct yaffs_dev *dev, int nand_chunk,
			     u8 *data, struct yaffs_ext_tags *tags)
{
	struct mtd_info *mtd = yaffs_dev_to_mtd(dev);
	u8 local_spare[128];
	struct mtd_oob_ops ops;
	size_t dummy;
	int retval = 0;
	int local_data = 0;
	struct yaffs_packed_tags2 pt;
	loff_t addr = ((loff_t) nand_chunk) * dev->param.total_bytes_per_chunk;
	int packed_tags_size =
	    dev->param.no_tags_ecc ? sizeof(pt.t) : sizeof(pt);
	void *packed_tags_ptr =
	    dev->param.no_tags_ecc ? (void *)&pt.t : (void *)&pt;

	yaffs_trace(YAFFS_TRACE_MTD,
		"nandmtd2_read_chunk_tags chunk %d data %p tags %p",
		nand_chunk, data, tags);

	if (dev->param.inband_tags) {

		if (!data) {
			local_data = 1;
			data = yaffs_get_temp_buffer(dev);
		}

	}

	if (dev->param.inband_tags || (data && !tags))
		retval = mtd_read(mtd, addr, dev->param.total_bytes_per_chunk,
				   &dummy, data);
	else if (tags) {
		ops.mode = MTD_OPS_AUTO_OOB;
		ops.ooblen = packed_tags_size;
		ops.len = data ? dev->data_bytes_per_chunk : packed_tags_size;
		ops.ooboffs = 0;
		ops.datbuf = data;
		ops.oobbuf = local_spare;
		retval = mtd_read_oob(mtd, addr, &ops);
	}

	if (dev->param.inband_tags) {
		if (tags) {
			struct yaffs_packed_tags2_tags_only *pt2tp;
			pt2tp =
				(struct yaffs_packed_tags2_tags_only *)
				&data[dev->data_bytes_per_chunk];
			yaffs_unpack_tags2_tags_only(tags, pt2tp);
		}
	} else {
		if (tags) {
			memcpy(packed_tags_ptr,
			       local_spare,
			       packed_tags_size);
			yaffs_unpack_tags2(tags, &pt, !dev->param.no_tags_ecc);
		}
	}

	if (local_data)
		yaffs_release_temp_buffer(dev, data);

	if (tags && retval == -EBADMSG
	    && tags->ecc_result == YAFFS_ECC_RESULT_NO_ERROR) {
		tags->ecc_result = YAFFS_ECC_RESULT_UNFIXED;
		dev->n_ecc_unfixed++;
	}
	if (tags && retval == -EUCLEAN
	    && tags->ecc_result == YAFFS_ECC_RESULT_NO_ERROR) {
		tags->ecc_result = YAFFS_ECC_RESULT_FIXED;
		dev->n_ecc_fixed++;
	}
	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}


int nandmtd2_MarkNANDBlockBad(struct yaffs_dev *dev, int blockNo)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->driver_context);
	int retval;

	yaffs_trace(YAFFS_TRACE_MTD,
		"nandmtd2_MarkNANDBlockBad %d", blockNo);

	retval =
	    mtd_block_markbad(mtd,
			       blockNo * dev->param.chunks_per_block *
			       dev->data_bytes_per_chunk);

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;

}

int nandmtd2_QueryNANDBlock(struct yaffs_dev *dev, int blockNo,
			    enum yaffs_block_state *state, u32 *sequenceNumber)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->driver_context);
	int retval;

	yaffs_trace(YAFFS_TRACE_MTD, "nandmtd2_QueryNANDBlock %d", blockNo);
	retval =
	    mtd_block_isbad(mtd,
			     blockNo * dev->param.chunks_per_block *
			     dev->data_bytes_per_chunk);

	if (retval) {
		yaffs_trace(YAFFS_TRACE_MTD, "block is bad");

		*state = YAFFS_BLOCK_STATE_DEAD;
		*sequenceNumber = 0;
	} else {
		struct yaffs_ext_tags t;
		nandmtd2_read_chunk_tags(dev,
					   blockNo *
					   dev->param.chunks_per_block, NULL,
					   &t);

		if (t.chunk_used) {
			*sequenceNumber = t.seq_number;
			*state = YAFFS_BLOCK_STATE_NEEDS_SCAN;
		} else {
			*sequenceNumber = 0;
			*state = YAFFS_BLOCK_STATE_EMPTY;
		}
	}
	yaffs_trace(YAFFS_TRACE_MTD, "block is bad seq %d state %d",
			*sequenceNumber, *state);

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}
