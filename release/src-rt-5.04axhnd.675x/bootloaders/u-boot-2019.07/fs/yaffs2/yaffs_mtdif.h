/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2007 Aleph One Ltd.
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

#ifndef __YAFFS_MTDIF_H__
#define __YAFFS_MTDIF_H__

#include "yaffs_guts.h"

int nandmtd_WriteChunkToNAND(struct yaffs_dev *dev, int chunkInNAND,
			     const u8 *data, const struct yaffs_spare *spare);
int nandmtd_ReadChunkFromNAND(struct yaffs_dev *dev, int chunkInNAND, u8 *data,
			      struct yaffs_spare *spare);
int nandmtd_EraseBlockInNAND(struct yaffs_dev *dev, int blockNumber);
int nandmtd_InitialiseNAND(struct yaffs_dev *dev);
#endif
