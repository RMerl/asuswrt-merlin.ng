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

#include <efi_driver.h>
#include <dm/device-internal.h>
#include <dm/root.h>

/*
 * EFI attributes of the udevice handled by this driver.
 *
 * handle	handle of the controller on which this driver is installed
 * io		block io protocol proxied by this driver
 */
struct efi_blk_platdata {
	efi_handle_t		handle;
	struct efi_block_io	*io;
};

/*
 * Read from block device
 *
 * @dev		device
 * @blknr	first block to be read
 * @blkcnt	number of blocks to read
 * @buffer	output buffer
 * @return	number of blocks transferred
 */
static ulong efi_bl_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
			 void *buffer)
{
	struct efi_blk_platdata *platdata = dev_get_platdata(dev);
	struct efi_block_io *io = platdata->io;
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

/*
 * Write to block device
 *
 * @dev		device
 * @blknr	first block to be write
 * @blkcnt	number of blocks to write
 * @buffer	input buffer
 * @return	number of blocks transferred
 */
static ulong efi_bl_write(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
			  const void *buffer)
{
	struct efi_blk_platdata *platdata = dev_get_platdata(dev);
	struct efi_block_io *io = platdata->io;
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

/*
 * Create partions for the block device.
 *
 * @handle	EFI handle of the block device
 * @dev		udevice of the block device
 */
static int efi_bl_bind_partitions(efi_handle_t handle, struct udevice *dev)
{
	struct blk_desc *desc;
	const char *if_typename;

	desc = dev_get_uclass_platdata(dev);
	if_typename = blk_get_if_type_name(desc->if_type);

	return efi_disk_create_partitions(handle, desc, if_typename,
					  desc->devnum, dev->name);
}

/*
 * Create a block device for a handle
 *
 * @handle	handle
 * @interface	block io protocol
 * @return	0 = success
 */
static int efi_bl_bind(efi_handle_t handle, void *interface)
{
	struct udevice *bdev, *parent = dm_root();
	int ret, devnum;
	char *name;
	struct efi_object *obj = efi_search_obj(handle);
	struct efi_block_io *io = interface;
	int disks;
	struct efi_blk_platdata *platdata;

	EFI_PRINT("%s: handle %p, interface %p\n", __func__, handle, io);

	if (!obj)
		return -ENOENT;

	devnum = blk_find_max_devnum(IF_TYPE_EFI);
	if (devnum == -ENODEV)
		devnum = 0;
	else if (devnum < 0)
		return devnum;

	name = calloc(1, 18); /* strlen("efiblk#2147483648") + 1 */
	if (!name)
		return -ENOMEM;
	sprintf(name, "efiblk#%d", devnum);

	/* Create driver model udevice for the EFI block io device */
	ret = blk_create_device(parent, "efi_blk", name, IF_TYPE_EFI, devnum,
				io->media->block_size,
				(lbaint_t)io->media->last_block, &bdev);
	if (ret)
		return ret;
	if (!bdev)
		return -ENOENT;
	/* Set the DM_FLAG_NAME_ALLOCED flag to avoid a memory leak */
	device_set_name_alloced(bdev);

	platdata = dev_get_platdata(bdev);
	platdata->handle = handle;
	platdata->io = interface;

	ret = device_probe(bdev);
	if (ret)
		return ret;
	EFI_PRINT("%s: block device '%s' created\n", __func__, bdev->name);

	/* Create handles for the partions of the block device */
	disks = efi_bl_bind_partitions(handle, bdev);
	EFI_PRINT("Found %d partitions\n", disks);

	return 0;
}

/* Block device driver operators */
static const struct blk_ops efi_blk_ops = {
	.read	= efi_bl_read,
	.write	= efi_bl_write,
};

/* Identify as block device driver */
U_BOOT_DRIVER(efi_blk) = {
	.name			= "efi_blk",
	.id			= UCLASS_BLK,
	.ops			= &efi_blk_ops,
	.platdata_auto_alloc_size = sizeof(struct efi_blk_platdata),
};

/* EFI driver operators */
static const struct efi_driver_ops driver_ops = {
	.protocol	= &efi_block_io_guid,
	.child_protocol = &efi_block_io_guid,
	.bind		= efi_bl_bind,
};

/* Identify as EFI driver */
U_BOOT_DRIVER(efi_block) = {
	.name		= "EFI block driver",
	.id		= UCLASS_EFI,
	.ops		= &driver_ops,
};
