/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

/* Interface to emulated NAND functions (2k page size) */

#ifndef __YAFFS_NANDEMUL2K_H__
#define __YAFFS_NANDEMUL2K_H__

#include "yaffs_guts.h"

int nandemul2k_WriteChunkWithTagsToNAND(struct yaffs_dev *dev,
					int nand_chunk, const u8 *data,
					const struct yaffs_ext_tags *tags);
int nandemul2k_ReadChunkWithTagsFromNAND(struct yaffs_dev *dev,
					 int nand_chunk, u8 *data,
					 struct yaffs_ext_tags *tags);
int nandemul2k_MarkNANDBlockBad(struct yaffs_dev *dev, int block_no);
int nandemul2k_QueryNANDBlock(struct yaffs_dev *dev, int block_no,
			      enum yaffs_block_state *state, u32 *seq_number);
int nandemul2k_EraseBlockInNAND(struct yaffs_dev *dev,
				int flash_block);
int nandemul2k_InitialiseNAND(struct yaffs_dev *dev);
int nandemul2k_GetBytesPerChunk(void);
int nandemul2k_GetChunksPerBlock(void);
int nandemul2k_GetNumberOfBlocks(void);

#endif
