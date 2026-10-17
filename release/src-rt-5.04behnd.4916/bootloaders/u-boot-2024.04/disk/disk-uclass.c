// SPDX-License-Identifier: GPL-2.0+
/*
 *  Software partition device (UCLASS_PARTITION)
 *
 *  Copyright (c) 2021 Linaro Limited
 *			Author: AKASHI Takahiro
 */

#define LOG_CATEGORY UCLASS_PARTITION

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <log.h>
#include <part.h>
#include <vsprintf.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

/**
 * disk_blk_part_validate() - Check whether access to partition is within limits
 *
 * @dev: Device (partition udevice)
 * @start: Start block for the access(from start of partition)
 * @blkcnt: Number of blocks to access (within the partition)
 * @return 0 on valid block range, or -ve on error.
 */
static int disk_blk_part_validate(struct udevice *dev, lbaint_t start, lbaint_t blkcnt)
{
	struct disk_part *part = dev_get_uclass_plat(dev);

	if (device_get_uclass_id(dev) != UCLASS_PARTITION)
		return -ENOSYS;

	if (start >= part->gpt_part_info.size)
		return -E2BIG;

	if ((start + blkcnt) > part->gpt_part_info.size)
		return -ERANGE;

	return 0;
}

/**
 * disk_blk_part_offset() - Compute offset from start of block device
 *
 * @dev: Device (partition udevice)
 * @start: Start block for the access (from start of partition)
 * @return Start block for the access (from start of block device)
 */
static lbaint_t disk_blk_part_offset(struct udevice *dev, lbaint_t start)
{
	struct disk_part *part = dev_get_uclass_plat(dev);

	return start + part->gpt_part_info.start;
}

/*
 * BLOCK IO APIs
 */
/**
 * disk_blk_read() - Read from a block device partition
 *
 * @dev: Device to read from (partition udevice)
 * @start: Start block for the read (from start of partition)
 * @blkcnt: Number of blocks to read (within the partition)
 * @buffer: Place to put the data
 * @return number of blocks read (which may be less than @blkcnt),
 * or -ve on error. This never returns 0 unless @blkcnt is 0
 */
unsigned long disk_blk_read(struct udevice *dev, lbaint_t start,
			    lbaint_t blkcnt, void *buffer)
{
	int ret = disk_blk_part_validate(dev, start, blkcnt);

	if (ret)
		return ret;

	return blk_read(dev_get_parent(dev), disk_blk_part_offset(dev, start),
			blkcnt, buffer);
}

/**
 * disk_blk_write() - Write to a block device
 *
 * @dev: Device to write to (partition udevice)
 * @start: Start block for the write (from start of partition)
 * @blkcnt: Number of blocks to write (within the partition)
 * @buffer: Data to write
 * @return number of blocks written (which may be less than @blkcnt),
 * or -ve on error. This never returns 0 unless @blkcnt is 0
 */
unsigned long disk_blk_write(struct udevice *dev, lbaint_t start,
			     lbaint_t blkcnt, const void *buffer)
{
	int ret = disk_blk_part_validate(dev, start, blkcnt);

	if (ret)
		return ret;

	return blk_write(dev_get_parent(dev), disk_blk_part_offset(dev, start),
			 blkcnt, buffer);
}

/**
 * disk_blk_erase() - Erase part of a block device
 *
 * @dev: Device to erase (partition udevice)
 * @start: Start block for the erase (from start of partition)
 * @blkcnt: Number of blocks to erase (within the partition)
 * @return number of blocks erased (which may be less than @blkcnt),
 * or -ve on error. This never returns 0 unless @blkcnt is 0
 */
unsigned long disk_blk_erase(struct udevice *dev, lbaint_t start,
			     lbaint_t blkcnt)
{
	int ret = disk_blk_part_validate(dev, start, blkcnt);

	if (ret)
		return ret;

	return blk_erase(dev_get_parent(dev), disk_blk_part_offset(dev, start),
			 blkcnt);
}

UCLASS_DRIVER(partition) = {
	.id		= UCLASS_PARTITION,
	.per_device_plat_auto	= sizeof(struct disk_part),
	.name		= "partition",
};

static const struct blk_ops blk_part_ops = {
	.read	= disk_blk_read,
	.write	= disk_blk_write,
	.erase	= disk_blk_erase,
};

U_BOOT_DRIVER(blk_partition) = {
	.name		= "blk_partition",
	.id		= UCLASS_PARTITION,
	.ops		= &blk_part_ops,
};
