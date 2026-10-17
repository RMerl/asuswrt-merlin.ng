/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "yportenv.h"
#include "yaffs_guts.h"
#include <malloc.h>


#include "yaffs_nandif.h"
#include "yaffs_packedtags2.h"

#include "yramsim.h"

#include "yaffs_trace.h"
#include "yaffsfs.h"


/* NB For use with inband tags....
 * We assume that the data buffer is of size totalBytersPerChunk so that
 * we can also use it to load the tags.
 */
int ynandif_WriteChunkWithTagsToNAND(struct yaffs_dev *dev, int nand_chunk,
				      const u8 *data,
				      const struct yaffs_ext_tags *tags)
{

	int retval = 0;
	struct yaffs_packed_tags2 pt;
	void *spare;
	unsigned spareSize = 0;
	struct ynandif_Geometry *geometry = (struct ynandif_Geometry *)(dev->driver_context);

	yaffs_trace(YAFFS_TRACE_MTD,
		"nandmtd2_WriteChunkWithTagsToNAND chunk %d data %p tags %p",
		nand_chunk, data, tags);


	/* For yaffs2 writing there must be both data and tags.
	 * If we're using inband tags, then the tags are stuffed into
	 * the end of the data buffer.
	 */

	if (dev->param.inband_tags) {
		struct yaffs_packed_tags2_tags_only *pt2tp;

		pt2tp = (struct yaffs_packed_tags2_tags_only *)
			(data + dev->data_bytes_per_chunk);
		yaffs_pack_tags2_tags_only(pt2tp, tags);
		spare = NULL;
		spareSize = 0;
	} else {
		yaffs_pack_tags2(&pt, tags, !dev->param.no_tags_ecc);
		spare = &pt;
		spareSize = sizeof(struct yaffs_packed_tags2);
	}

	retval = geometry->writeChunk(dev, nand_chunk,
				data, dev->param.total_bytes_per_chunk,
				spare, spareSize);

	return retval;
}

int ynandif_ReadChunkWithTagsFromNAND(struct yaffs_dev *dev, int nand_chunk,
				       u8 *data, struct yaffs_ext_tags *tags)
{
	struct yaffs_packed_tags2 pt;
	int localData = 0;
	void *spare = NULL;
	unsigned spareSize;
	int retval = 0;
	int eccStatus; /* 0 = ok, 1 = fixed, -1 = unfixed */
	struct ynandif_Geometry *geometry = (struct ynandif_Geometry *)(dev->driver_context);

	yaffs_trace(YAFFS_TRACE_MTD,
		"nandmtd2_ReadChunkWithTagsFromNAND chunk %d data %p tags %p",
		nand_chunk, data, tags);

	if (!tags) {
		spare = NULL;
		spareSize = 0;
	} else if (dev->param.inband_tags) {

		if (!data) {
			localData = 1;
			data = yaffs_get_temp_buffer(dev);
		}
		spare = NULL;
		spareSize = 0;
	} else {
		spare = &pt;
		spareSize = sizeof(struct yaffs_packed_tags2);
	}

	retval = geometry->readChunk(dev, nand_chunk,
				 data,
				 data ? dev->param.total_bytes_per_chunk : 0,
				 spare, spareSize,
				 &eccStatus);

	if (dev->param.inband_tags) {
		if (tags) {
			struct yaffs_packed_tags2_tags_only *pt2tp;
			pt2tp = (struct yaffs_packed_tags2_tags_only *)
					&data[dev->data_bytes_per_chunk];
			yaffs_unpack_tags2_tags_only(tags, pt2tp);
		}
	} else {
		if (tags)
			yaffs_unpack_tags2(tags, &pt, !dev->param.no_tags_ecc);
	}

	if (tags && tags->chunk_used) {
		if (eccStatus < 0 ||
		   tags->ecc_result == YAFFS_ECC_RESULT_UNFIXED)
			tags->ecc_result = YAFFS_ECC_RESULT_UNFIXED;
		else if (eccStatus > 0 ||
			     tags->ecc_result == YAFFS_ECC_RESULT_FIXED)
			tags->ecc_result = YAFFS_ECC_RESULT_FIXED;
		else
			tags->ecc_result = YAFFS_ECC_RESULT_NO_ERROR;
	}

	if (localData)
		yaffs_release_temp_buffer(dev, data);

	return retval;
}

int ynandif_MarkNANDBlockBad(struct yaffs_dev *dev, int blockId)
{
	struct ynandif_Geometry *geometry = (struct ynandif_Geometry *)(dev->driver_context);

	return geometry->markBlockBad(dev, blockId);
}

int ynandif_EraseBlockInNAND(struct yaffs_dev *dev, int blockId)
{
	struct ynandif_Geometry *geometry = (struct ynandif_Geometry *)(dev->driver_context);

	return geometry->eraseBlock(dev, blockId);

}


static int ynandif_IsBlockOk(struct yaffs_dev *dev, int blockId)
{
	struct ynandif_Geometry *geometry = (struct ynandif_Geometry *)(dev->driver_context);

	return geometry->checkBlockOk(dev, blockId);
}

int ynandif_QueryNANDBlock(struct yaffs_dev *dev, int blockId,
		enum yaffs_block_state *state, u32 *seq_number)
{
	unsigned chunkNo;
	struct yaffs_ext_tags tags;

	*seq_number = 0;

	chunkNo = blockId * dev->param.chunks_per_block;

	if (!ynandif_IsBlockOk(dev, blockId)) {
		*state = YAFFS_BLOCK_STATE_DEAD;
	} else {
		ynandif_ReadChunkWithTagsFromNAND(dev, chunkNo, NULL, &tags);

		if (!tags.chunk_used) {
			*state = YAFFS_BLOCK_STATE_EMPTY;
		} else {
			*state = YAFFS_BLOCK_STATE_NEEDS_SCAN;
			*seq_number = tags.seq_number;
		}
	}

	return YAFFS_OK;
}


int ynandif_InitialiseNAND(struct yaffs_dev *dev)
{
	struct ynandif_Geometry *geometry = (struct ynandif_Geometry *)(dev->driver_context);

	geometry->initialise(dev);

	return YAFFS_OK;
}

int ynandif_Deinitialise_flash_fn(struct yaffs_dev *dev)
{
	struct ynandif_Geometry *geometry = (struct ynandif_Geometry *)(dev->driver_context);

	geometry->deinitialise(dev);

	return YAFFS_OK;
}


struct yaffs_dev *
	yaffs_add_dev_from_geometry(const YCHAR *name,
					const struct ynandif_Geometry *geometry)
{
	YCHAR *clonedName = malloc(sizeof(YCHAR) *
				(strnlen(name, YAFFS_MAX_NAME_LENGTH)+1));
	struct yaffs_dev *dev = malloc(sizeof(struct yaffs_dev));
	struct yaffs_param *param;

	if (dev && clonedName) {
		memset(dev, 0, sizeof(struct yaffs_dev));
		strcpy(clonedName, name);

		param = &dev->param;

		param->name = clonedName;
		param->write_chunk_tags_fn = ynandif_WriteChunkWithTagsToNAND;
		param->read_chunk_tags_fn = ynandif_ReadChunkWithTagsFromNAND;
		param->erase_fn = ynandif_EraseBlockInNAND;
		param->initialise_flash_fn = ynandif_InitialiseNAND;
		param->query_block_fn = ynandif_QueryNANDBlock;
		param->bad_block_fn = ynandif_MarkNANDBlockBad;
		param->n_caches = 20;
		param->start_block = geometry->start_block;
		param->end_block   = geometry->end_block;
		param->total_bytes_per_chunk  = geometry->dataSize;
		param->spare_bytes_per_chunk  = geometry->spareSize;
		param->inband_tags		  = geometry->inband_tags;
		param->chunks_per_block	  = geometry->pagesPerBlock;
		param->use_nand_ecc		  = geometry->hasECC;
		param->is_yaffs2		  = geometry->useYaffs2;
		param->n_reserved_blocks	  = 5;
		dev->driver_context		  = (void *)geometry;

		yaffs_add_device(dev);

		return dev;
	}

	free(dev);
	free(clonedName);

	return NULL;
}
