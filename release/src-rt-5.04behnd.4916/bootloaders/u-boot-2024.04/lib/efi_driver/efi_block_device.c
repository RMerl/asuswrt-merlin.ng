// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI block driver
 *
 *  Copyright (c) 2017 Heinrich Schuchardt
 *
 * The EFI uclass creates a handle for this driver and installs the
 * driver binding protocol on it.
 *
 * The EFI block driver binds to controllers implementing the block io
 * protocol.
 *
 * When the bind function of the EFI block driver is called it creates a
 * new U-Boot block device. It installs child handles for all partitions and
 * installs the simple file protocol on these.
 *
 * The read and write functions of the EFI block driver delegate calls to the
 * controller that it is bound to.
 *
 * A usage example is as following:
 *
 * U-Boot loads the iPXE snp.efi executable. iPXE connects an iSCSI drive and
 * exposes a handle with the block IO protocol. It calls ConnectController.
 *
 * Now the EFI block driver installs the partitions with the simple file
 * protocol.
 *
 * iPXE uses the simple file protocol to load Grub or the Linux Kernel.
 */

#include <blk.h>
#include <dm.h>
#include <efi_driver.h>
#include <malloc.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/tag.h>

/**
 * struct efi_blk_plat - attributes of a block device
 *
 * @handle:	handle of the controller on which this driver is installed
 * @io:		block io protocol proxied by this driver
 */
struct efi_blk_plat {
	efi_handle_t		handle;
	struct efi_block_io	*io;
};

/**
 * efi_bl_read() - read from block device
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
	struct efi_blk_plat *plat = dev_get_plat(dev);
	struct efi_block_io *io = plat->io;
	efi_status_t ret;

	EFI_PRINT("%s: read '%s', from block " LBAFU ", " LBAFU " blocks\n",
		  __func__, dev->name, blknr, blkcnt);
	ret = EFI_CALL(io->read_blocks(
				io, io->media->media_id, (u64)blknr,
				(efi_uintn_t)blkcnt *
				(efi_uintn_t)io->media->block_size, buffer));
	EFI_PRINT("%s: r = %u\n", __func__,
		  (unsigned int)(ret & ~EFI_ERROR_MASK));
	if (ret != EFI_SUCCESS)
		return 0;
	return blkcnt;
}

/**
 * efi_bl_write() - write to block device
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
	struct efi_blk_plat *plat = dev_get_plat(dev);
	struct efi_block_io *io = plat->io;
	efi_status_t ret;

	EFI_PRINT("%s: write '%s', from block " LBAFU ", " LBAFU " blocks\n",
		  __func__, dev->name, blknr, blkcnt);
	ret = EFI_CALL(io->write_blocks(
				io, io->media->media_id, (u64)blknr,
				(efi_uintn_t)blkcnt *
				(efi_uintn_t)io->media->block_size,
				(void *)buffer));
	EFI_PRINT("%s: r = %u\n", __func__,
		  (unsigned int)(ret & ~EFI_ERROR_MASK));
	if (ret != EFI_SUCCESS)
		return 0;
	return blkcnt;
}

/**
 * efi_bl_create_block_device() - create a block device for a handle
 *
 * @handle:	handle
 * @interface:	block io protocol
 * Return:	status code
 */
static efi_status_t
efi_bl_create_block_device(efi_handle_t handle, void *interface)
{
	struct udevice *bdev = NULL, *parent = dm_root();
	efi_status_t ret;
	int devnum;
	char *name;
	struct efi_block_io *io = interface;
	struct efi_blk_plat *plat;

	devnum = blk_next_free_devnum(UCLASS_EFI_LOADER);
	if (devnum < 0)
		return EFI_OUT_OF_RESOURCES;

	name = calloc(1, 18); /* strlen("efiblk#2147483648") + 1 */
	if (!name)
		return EFI_OUT_OF_RESOURCES;
	sprintf(name, "efiblk#%d", devnum);

	/* Create driver model udevice for the EFI block io device */
	if (blk_create_device(parent, "efi_blk", name, UCLASS_EFI_LOADER,
			      devnum, io->media->block_size,
			      (lbaint_t)io->media->last_block, &bdev)) {
		ret = EFI_OUT_OF_RESOURCES;
		free(name);
		goto err;
	}
	/* Set the DM_FLAG_NAME_ALLOCED flag to avoid a memory leak */
	device_set_name_alloced(bdev);

	plat = dev_get_plat(bdev);
	plat->handle = handle;
	plat->io = interface;

	if (efi_link_dev(handle, bdev)) {
		ret = EFI_OUT_OF_RESOURCES;
		goto err;
	}

	if (device_probe(bdev)) {
		ret = EFI_DEVICE_ERROR;
		goto err;
	}
	EFI_PRINT("%s: block device '%s' created\n", __func__, bdev->name);

	return EFI_SUCCESS;

err:
	efi_unlink_dev(handle);
	if (bdev)
		device_unbind(bdev);

	return ret;
}

/**
 * efi_bl_bind() - bind to a block io protocol
 *
 * @this:	driver binding protocol
 * @handle:	handle
 * @interface:	block io protocol
 * Return:	status code
 */
static efi_status_t efi_bl_bind(
			struct efi_driver_binding_extended_protocol *this,
			efi_handle_t handle, void *interface)
{
	efi_status_t ret = EFI_SUCCESS;
	struct efi_object *obj = efi_search_obj(handle);

	EFI_PRINT("%s: handle %p, interface %p\n", __func__, handle, interface);

	if (!obj || !interface)
		return EFI_INVALID_PARAMETER;

	if (!handle->dev)
		ret = efi_bl_create_block_device(handle, interface);

	return ret;
}

/**
 * efi_bl_init() - initialize block device driver
 *
 * @this:	extended driver binding protocol
 */
static efi_status_t
efi_bl_init(struct efi_driver_binding_extended_protocol *this)
{
	int ret;

	ret = event_register("efi_disk add", EVT_DM_POST_PROBE,
			     efi_disk_probe, this);
	if (ret) {
		log_err("Event registration for efi_disk add failed\n");
		return EFI_OUT_OF_RESOURCES;
	}

	ret = event_register("efi_disk del", EVT_DM_PRE_REMOVE,
			     efi_disk_remove, this);
	if (ret) {
		log_err("Event registration for efi_disk del failed\n");
		return EFI_OUT_OF_RESOURCES;
	}

	return EFI_SUCCESS;
}

/* Block device driver operators */
static const struct blk_ops efi_blk_ops = {
	.read	= efi_bl_read,
	.write	= efi_bl_write,
};

/* Identify as block device driver */
U_BOOT_DRIVER(efi_blk) = {
	.name		= "efi_blk",
	.id		= UCLASS_BLK,
	.ops		= &efi_blk_ops,
	.plat_auto	= sizeof(struct efi_blk_plat),
};

/* EFI driver operators */
static const struct efi_driver_ops driver_ops = {
	.protocol	= &efi_block_io_guid,
	.child_protocol = &efi_block_io_guid,
	.init		= efi_bl_init,
	.bind		= efi_bl_bind,
};

/* Identify as EFI driver */
U_BOOT_DRIVER(efi_block) = {
	.name		= "EFI block driver",
	.id		= UCLASS_EFI_LOADER,
	.ops		= &driver_ops,
};
