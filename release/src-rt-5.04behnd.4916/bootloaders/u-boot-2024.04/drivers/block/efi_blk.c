// SPDX-License-Identifier: GPL-2.0+
/*
 * Block driver for EFI devices
 * This supports a media driver of UCLASS_EFI with a child UCLASS_BLK
 * It allows block-level access to EFI devices made available via EFI boot
 * services
 *
 * Copyright 2021 Google LLC
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <efi.h>
#include <efi_api.h>

struct efi_block_plat {
	struct efi_block_io *blkio;
};

/**
 * Read from block device
 *
 * @dev:	device
 * @blknr:	first block to be read
 * @blkcnt:	number of blocks to read
 * @buffer:	output buffer
 * Return:	number of blocks transferred
 */
static ulong efi_bl_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
			 void *buffer)
{
	struct efi_block_plat *plat = dev_get_plat(dev);
	struct efi_block_io *io = plat->blkio;
	efi_status_t ret;

	log_debug("read buf=%p, block=%lx, count=%lx: ", buffer, (ulong)blknr,
		  (ulong)blkcnt);
	ret = io->read_blocks(io, io->media->media_id, blknr,
			      blkcnt * io->media->block_size, buffer);
	log_debug("ret=%lx (dec %ld)\n", ret & ~EFI_ERROR_MASK,
		  ret & ~EFI_ERROR_MASK);
	if (ret)
		return 0;

	return blkcnt;
}

/**
 * Write to block device
 *
 * @dev:	device
 * @blknr:	first block to be write
 * @blkcnt:	number of blocks to write
 * @buffer:	input buffer
 * Return:	number of blocks transferred
 */
static ulong efi_bl_write(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
			  const void *buffer)
{
	struct efi_block_plat *plat = dev_get_plat(dev);
	struct efi_block_io *io = plat->blkio;
	efi_status_t ret;

	log_debug("write buf=%p, block=%lx, count=%lx: ", buffer, (ulong)blknr,
		  (ulong)blkcnt);
	ret = io->write_blocks(io, io->media->media_id, blknr,
			       blkcnt * io->media->block_size, (void *)buffer);
	log_debug("ret=%lx (dec %ld)\n", ret & ~EFI_ERROR_MASK,
		  ret & ~EFI_ERROR_MASK);
	if (ret)
		return 0;

	return blkcnt;
}

/* Block device driver operators */
static const struct blk_ops efi_blk_ops = {
	.read	= efi_bl_read,
	.write	= efi_bl_write,
};

U_BOOT_DRIVER(efi_block) = {
	.name		= "efi_block",
	.id		= UCLASS_BLK,
	.ops		= &efi_blk_ops,
	.plat_auto	= sizeof(struct efi_block_plat),
};

static int efi_media_bind(struct udevice *dev)
{
	struct efi_media_plat *plat = dev_get_plat(dev);
	struct efi_block_plat *blk_plat;
	struct udevice *blk;
	int ret;

	ret = blk_create_devicef(dev, "efi_block", "blk", UCLASS_EFI_MEDIA,
				 dev_seq(dev), plat->blkio->media->block_size,
				 plat->blkio->media->last_block, &blk);
	if (ret) {
		debug("Cannot create block device\n");
		return ret;
	}
	blk_plat = dev_get_plat(blk);
	blk_plat->blkio = plat->blkio;

	return 0;
}

U_BOOT_DRIVER(efi_media) = {
	.name		= "efi_media",
	.id		= UCLASS_EFI_MEDIA,
	.bind		= efi_media_bind,
	.plat_auto	= sizeof(struct efi_media_plat),
};
