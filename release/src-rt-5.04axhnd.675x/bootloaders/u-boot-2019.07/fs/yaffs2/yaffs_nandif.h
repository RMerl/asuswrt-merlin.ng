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


#ifndef __YNANDIF_H__
#define __YNANDIF_H__

#include "yaffs_guts.h"

struct ynandif_Geometry {
	unsigned start_block;
	unsigned end_block;
	unsigned dataSize;
	unsigned spareSize;
	unsigned pagesPerBlock;
	unsigned hasECC;
	unsigned inband_tags;
	unsigned useYaffs2;

	int (*initialise)(struct yaffs_dev *dev);
	int (*deinitialise)(struct yaffs_dev *dev);

	int (*readChunk) (struct yaffs_dev *dev,
					  unsigned pageId,
					  unsigned char *data,
					  unsigned dataLength,
					  unsigned char *spare,
					  unsigned spareLength,
					  int *eccStatus);
	/* ECC status is set to 0 for OK, 1 for fixed, -1 for unfixed. */

	int (*writeChunk)(struct yaffs_dev *dev,
					  unsigned pageId,
					  const unsigned char *data,
					  unsigned dataLength,
					  const unsigned char *spare,
					  unsigned spareLength);

	int (*eraseBlock)(struct yaffs_dev *dev, unsigned blockId);

	int (*checkBlockOk)(struct yaffs_dev *dev, unsigned blockId);
	int (*markBlockBad)(struct yaffs_dev *dev, unsigned blockId);

	void *privateData;

};

struct yaffs_dev *
	yaffs_add_dev_from_geometry(const YCHAR *name,
					const struct ynandif_Geometry *geometry);


#endif
