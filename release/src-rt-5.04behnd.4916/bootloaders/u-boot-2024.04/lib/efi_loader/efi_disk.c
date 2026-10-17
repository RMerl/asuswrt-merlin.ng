// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application disk support
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#define LOG_CATEGORY LOGC_EFI

#include <blk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/tag.h>
#include <event.h>
#include <efi_driver.h>
#include <efi_loader.h>
#include <fs.h>
#include <log.h>
#include <part.h>
#include <malloc.h>

struct efi_system_partition efi_system_partition = {
	.uclass_id = UCLASS_INVALID,
};

const efi_guid_t efi_block_io_guid = EFI_BLOCK_IO_PROTOCOL_GUID;
const efi_guid_t efi_system_partition_guid = PARTITION_SYSTEM_GUID;

/**
 * struct efi_disk_obj - EFI disk object
 *
 * @header:	EFI object header
 * @ops:	EFI disk I/O protocol interface
 * @media:	block I/O media information
 * @dp:		device path to the block device
 * @volume:	simple file system protocol of the partition
 */
struct efi_disk_obj {
	struct efi_object header;
	struct efi_block_io ops;
	struct efi_block_io_media media;
	struct efi_device_path *dp;
	struct efi_simple_file_system_protocol *volume;
};

/**
 * efi_disk_reset() - reset block device
 *
 * This function implements the Reset service of the EFI_BLOCK_IO_PROTOCOL.
 *
 * As U-Boot's block devices do not have a reset function simply return
 * EFI_SUCCESS.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:			pointer to the BLOCK_IO_PROTOCOL
 * @extended_verification:	extended verification
 * Return:			status code
 */
static efi_status_t EFIAPI efi_disk_reset(struct efi_block_io *this,
			char extended_verification)
{
	EFI_ENTRY("%p, %x", this, extended_verification);
	return EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_disk_is_removable() - check if the device is removable media
 * @handle:		efi object handle;
 *
 * Examine the device and determine if the device is a local block device
 * and removable media.
 *
 * Return:		true if removable, false otherwise
 */
bool efi_disk_is_removable(efi_handle_t handle)
{
	struct efi_handler *handler;
	struct efi_block_io *io;
	efi_status_t ret;

	ret = efi_search_protocol(handle, &efi_block_io_guid, &handler);
	if (ret != EFI_SUCCESS)
		return false;

	io = handler->protocol_interface;

	if (!io || !io->media)
		return false;

	return (bool)io->media->removable_media;
}

enum efi_disk_direction {
	EFI_DISK_READ,
	EFI_DISK_WRITE,
};

static efi_status_t efi_disk_rw_blocks(struct efi_block_io *this,
			u32 media_id, u64 lba, unsigned long buffer_size,
			void *buffer, enum efi_disk_direction direction)
{
	struct efi_disk_obj *diskobj;
	int blksz;
	int blocks;
	unsigned long n;

	diskobj = container_of(this, struct efi_disk_obj, ops);
	blksz = diskobj->media.block_size;
	blocks = buffer_size / blksz;

	EFI_PRINT("blocks=%x lba=%llx blksz=%x dir=%d\n",
		  blocks, lba, blksz, direction);

	/* We only support full block access */
	if (buffer_size & (blksz - 1))
		return EFI_BAD_BUFFER_SIZE;

	if (CONFIG_IS_ENABLED(PARTITIONS) &&
	    device_get_uclass_id(diskobj->header.dev) == UCLASS_PARTITION) {
		if (direction == EFI_DISK_READ)
			n = disk_blk_read(diskobj->header.dev, lba, blocks,
					  buffer);
		else
			n = disk_blk_write(diskobj->header.dev, lba, blocks,
					   buffer);
	} else {
		/* dev is a block device (UCLASS_BLK) */
		struct blk_desc *desc;

		desc = dev_get_uclass_plat(diskobj->header.dev);
		if (direction == EFI_DISK_READ)
			n = blk_dread(desc, lba, blocks, buffer);
		else
			n = blk_dwrite(desc, lba, blocks, buffer);
	}

	/* We don't do interrupts, so check for timers cooperatively */
	efi_timer_check();

	EFI_PRINT("n=%lx blocks=%x\n", n, blocks);

	if (n != blocks)
		return EFI_DEVICE_ERROR;

	return EFI_SUCCESS;
}

/**
 * efi_disk_read_blocks() - reads blocks from device
 *
 * This function implements the ReadBlocks service of the EFI_BLOCK_IO_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:			pointer to the BLOCK_IO_PROTOCOL
 * @media_id:			id of the medium to be read from
 * @lba:			starting logical block for reading
 * @buffer_size:		size of the read buffer
 * @buffer:			pointer to the destination buffer
 * Return:			status code
 */
static efi_status_t EFIAPI efi_disk_read_blocks(struct efi_block_io *this,
			u32 media_id, u64 lba, efi_uintn_t buffer_size,
			void *buffer)
{
	void *real_buffer = buffer;
	efi_status_t r;

	if (!this)
		return EFI_INVALID_PARAMETER;
	/* TODO: check for media changes */
	if (media_id != this->media->media_id)
		return EFI_MEDIA_CHANGED;
	if (!this->media->media_present)
		return EFI_NO_MEDIA;
	/* media->io_align is a power of 2 or 0 */
	if (this->media->io_align &&
	    (uintptr_t)buffer & (this->media->io_align - 1))
		return EFI_INVALID_PARAMETER;
	if (lba * this->media->block_size + buffer_size >
	    (this->media->last_block + 1) * this->media->block_size)
		return EFI_INVALID_PARAMETER;

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
	if (buffer_size > EFI_LOADER_BOUNCE_BUFFER_SIZE) {
		r = efi_disk_read_blocks(this, media_id, lba,
			EFI_LOADER_BOUNCE_BUFFER_SIZE, buffer);
		if (r != EFI_SUCCESS)
			return r;
		return efi_disk_read_blocks(this, media_id, lba +
			EFI_LOADER_BOUNCE_BUFFER_SIZE / this->media->block_size,
			buffer_size - EFI_LOADER_BOUNCE_BUFFER_SIZE,
			buffer + EFI_LOADER_BOUNCE_BUFFER_SIZE);
	}

	real_buffer = efi_bounce_buffer;
#endif

	EFI_ENTRY("%p, %x, %llx, %zx, %p", this, media_id, lba,
		  buffer_size, buffer);

	r = efi_disk_rw_blocks(this, media_id, lba, buffer_size, real_buffer,
			       EFI_DISK_READ);

	/* Copy from bounce buffer to real buffer if necessary */
	if ((r == EFI_SUCCESS) && (real_buffer != buffer))
		memcpy(buffer, real_buffer, buffer_size);

	return EFI_EXIT(r);
}

/**
 * efi_disk_write_blocks() - writes blocks to device
 *
 * This function implements the WriteBlocks service of the
 * EFI_BLOCK_IO_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:			pointer to the BLOCK_IO_PROTOCOL
 * @media_id:			id of the medium to be written to
 * @lba:			starting logical block for writing
 * @buffer_size:		size of the write buffer
 * @buffer:			pointer to the source buffer
 * Return:			status code
 */
static efi_status_t EFIAPI efi_disk_write_blocks(struct efi_block_io *this,
			u32 media_id, u64 lba, efi_uintn_t buffer_size,
			void *buffer)
{
	void *real_buffer = buffer;
	efi_status_t r;

	if (!this)
		return EFI_INVALID_PARAMETER;
	if (this->media->read_only)
		return EFI_WRITE_PROTECTED;
	/* TODO: check for media changes */
	if (media_id != this->media->media_id)
		return EFI_MEDIA_CHANGED;
	if (!this->media->media_present)
		return EFI_NO_MEDIA;
	/* media->io_align is a power of 2 or 0 */
	if (this->media->io_align &&
	    (uintptr_t)buffer & (this->media->io_align - 1))
		return EFI_INVALID_PARAMETER;
	if (lba * this->media->block_size + buffer_size >
	    (this->media->last_block + 1) * this->media->block_size)
		return EFI_INVALID_PARAMETER;

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
	if (buffer_size > EFI_LOADER_BOUNCE_BUFFER_SIZE) {
		r = efi_disk_write_blocks(this, media_id, lba,
			EFI_LOADER_BOUNCE_BUFFER_SIZE, buffer);
		if (r != EFI_SUCCESS)
			return r;
		return efi_disk_write_blocks(this, media_id, lba +
			EFI_LOADER_BOUNCE_BUFFER_SIZE / this->media->block_size,
			buffer_size - EFI_LOADER_BOUNCE_BUFFER_SIZE,
			buffer + EFI_LOADER_BOUNCE_BUFFER_SIZE);
	}

	real_buffer = efi_bounce_buffer;
#endif

	EFI_ENTRY("%p, %x, %llx, %zx, %p", this, media_id, lba,
		  buffer_size, buffer);

	/* Populate bounce buffer if necessary */
	if (real_buffer != buffer)
		memcpy(real_buffer, buffer, buffer_size);

	r = efi_disk_rw_blocks(this, media_id, lba, buffer_size, real_buffer,
			       EFI_DISK_WRITE);

	return EFI_EXIT(r);
}

/**
 * efi_disk_flush_blocks() - flushes modified data to the device
 *
 * This function implements the FlushBlocks service of the
 * EFI_BLOCK_IO_PROTOCOL.
 *
 * As we always write synchronously nothing is done here.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:			pointer to the BLOCK_IO_PROTOCOL
 * Return:			status code
 */
static efi_status_t EFIAPI efi_disk_flush_blocks(struct efi_block_io *this)
{
	EFI_ENTRY("%p", this);
	return EFI_EXIT(EFI_SUCCESS);
}

static const struct efi_block_io block_io_disk_template = {
	.reset = &efi_disk_reset,
	.read_blocks = &efi_disk_read_blocks,
	.write_blocks = &efi_disk_write_blocks,
	.flush_blocks = &efi_disk_flush_blocks,
};

/**
 * efi_fs_from_path() - retrieve simple file system protocol
 *
 * Gets the simple file system protocol for a file device path.
 *
 * The full path provided is split into device part and into a file
 * part. The device part is used to find the handle on which the
 * simple file system protocol is installed.
 *
 * @full_path:	device path including device and file
 * Return:	simple file system protocol
 */
struct efi_simple_file_system_protocol *
efi_fs_from_path(struct efi_device_path *full_path)
{
	struct efi_object *efiobj;
	struct efi_handler *handler;
	struct efi_device_path *device_path;
	struct efi_device_path *file_path;
	efi_status_t ret;

	/* Split the path into a device part and a file part */
	ret = efi_dp_split_file_path(full_path, &device_path, &file_path);
	if (ret != EFI_SUCCESS)
		return NULL;
	efi_free_pool(file_path);

	/* Get the EFI object for the partition */
	efiobj = efi_dp_find_obj(device_path, NULL, NULL);
	efi_free_pool(device_path);
	if (!efiobj)
		return NULL;

	/* Find the simple file system protocol */
	ret = efi_search_protocol(efiobj, &efi_simple_file_system_protocol_guid,
				  &handler);
	if (ret != EFI_SUCCESS)
		return NULL;

	/* Return the simple file system protocol for the partition */
	return handler->protocol_interface;
}

/**
 * efi_fs_exists() - check if a partition bears a file system
 *
 * @desc:	block device descriptor
 * @part:	partition number
 * Return:	1 if a file system exists on the partition
 *		0 otherwise
 */
static int efi_fs_exists(struct blk_desc *desc, int part)
{
	if (fs_set_blk_dev_with_part(desc, part))
		return 0;

	if (fs_get_type() == FS_TYPE_ANY)
		return 0;

	fs_close();

	return 1;
}

static void efi_disk_free_diskobj(struct efi_disk_obj *diskobj)
{
	struct efi_device_path *dp = diskobj->dp;
	struct efi_simple_file_system_protocol *volume = diskobj->volume;

	/*
	 * ignore error of efi_delete_handle() since this function
	 * is expected to be called in error path.
	 */
	efi_delete_handle(&diskobj->header);
	efi_free_pool(dp);
	free(volume);
}

/**
 * efi_disk_add_dev() - create a handle for a partition or disk
 *
 * @parent:		parent handle
 * @dp_parent:		parent device path
 * @desc:		internal block device
 * @part_info:		partition info
 * @part:		partition
 * @disk:		pointer to receive the created handle
 * @agent_handle:	handle of the EFI block driver
 * Return:		disk object
 */
static efi_status_t efi_disk_add_dev(
				efi_handle_t parent,
				struct efi_device_path *dp_parent,
				struct blk_desc *desc,
				struct disk_partition *part_info,
				unsigned int part,
				struct efi_disk_obj **disk,
				efi_handle_t agent_handle)
{
	struct efi_disk_obj *diskobj;
	struct efi_object *handle;
	const efi_guid_t *esp_guid = NULL;
	efi_status_t ret;

	/* Don't add empty devices */
	if (!desc->lba)
		return EFI_NOT_READY;

	diskobj = calloc(1, sizeof(*diskobj));
	if (!diskobj)
		return EFI_OUT_OF_RESOURCES;

	/* Hook up to the device list */
	efi_add_handle(&diskobj->header);

	/* Fill in object data */
	if (part_info) {
		struct efi_device_path *node = efi_dp_part_node(desc, part);
		struct efi_handler *handler;
		void *protocol_interface;

		if (!node) {
			ret = EFI_OUT_OF_RESOURCES;
			log_debug("no node\n");
			goto error;
		}

		/* Parent must expose EFI_BLOCK_IO_PROTOCOL */
		ret = efi_search_protocol(parent, &efi_block_io_guid, &handler);
		if (ret != EFI_SUCCESS) {
			log_debug("search failed\n");
			goto error;
		}

		/*
		 * Link the partition (child controller) to the block device
		 * (controller).
		 */
		ret = efi_protocol_open(handler, &protocol_interface, NULL,
					&diskobj->header,
					EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER);
		if (ret != EFI_SUCCESS) {
			log_debug("prot open failed\n");
			goto error;
		}

		diskobj->dp = efi_dp_append_node(dp_parent, node);
		efi_free_pool(node);
		diskobj->media.last_block = part_info->size - 1;
		if (part_info->bootable & PART_EFI_SYSTEM_PARTITION)
			esp_guid = &efi_system_partition_guid;
	} else {
		diskobj->dp = efi_dp_from_part(desc, part);
		diskobj->media.last_block = desc->lba - 1;
	}

	/*
	 * Install the device path and the block IO protocol.
	 *
	 * InstallMultipleProtocolInterfaces() checks if the device path is
	 * already installed on an other handle and returns EFI_ALREADY_STARTED
	 * in this case.
	 */
	handle = &diskobj->header;
	ret = efi_install_multiple_protocol_interfaces(
					&handle,
					&efi_guid_device_path, diskobj->dp,
					&efi_block_io_guid, &diskobj->ops,
					/*
					 * esp_guid must be last entry as it
					 * can be NULL. Its interface is NULL.
					 */
					esp_guid, NULL,
					NULL);
	if (ret != EFI_SUCCESS) {
		log_debug("install failed %lx\n", ret);
		goto error;
	}

	/*
	 * On partitions or whole disks without partitions install the
	 * simple file system protocol if a file system is available.
	 */
	if ((part || desc->part_type == PART_TYPE_UNKNOWN) &&
	    efi_fs_exists(desc, part)) {
		ret = efi_create_simple_file_system(desc, part, diskobj->dp,
						    &diskobj->volume);
		if (ret != EFI_SUCCESS)
			goto error;

		ret = efi_add_protocol(&diskobj->header,
				       &efi_simple_file_system_protocol_guid,
				       diskobj->volume);
		if (ret != EFI_SUCCESS)
			goto error;
	}
	diskobj->ops = block_io_disk_template;

	/* Fill in EFI IO Media info (for read/write callbacks) */
	diskobj->media.removable_media = desc->removable;
	diskobj->media.media_present = 1;
	/*
	 * MediaID is just an arbitrary counter.
	 * We have to change it if the medium is removed or changed.
	 */
	diskobj->media.media_id = 1;
	diskobj->media.block_size = desc->blksz;
	diskobj->media.io_align = desc->blksz;
	if (part)
		diskobj->media.logical_partition = 1;
	diskobj->ops.media = &diskobj->media;
	if (disk)
		*disk = diskobj;

	EFI_PRINT("BlockIO: part %u, present %d, logical %d, removable %d"
		  ", last_block %llu\n",
		  part,
		  diskobj->media.media_present,
		  diskobj->media.logical_partition,
		  diskobj->media.removable_media,
		  diskobj->media.last_block);

	/* Store first EFI system partition */
	if (part && efi_system_partition.uclass_id == UCLASS_INVALID) {
		if (part_info->bootable & PART_EFI_SYSTEM_PARTITION) {
			efi_system_partition.uclass_id = desc->uclass_id;
			efi_system_partition.devnum = desc->devnum;
			efi_system_partition.part = part;
			EFI_PRINT("EFI system partition: %s %x:%x\n",
				  blk_get_uclass_name(desc->uclass_id),
				  desc->devnum, part);
		}
	}
	return EFI_SUCCESS;
error:
	efi_disk_free_diskobj(diskobj);
	return ret;
}

/**
 * efi_disk_create_raw() - create a handle for a whole raw disk
 *
 * @dev:		udevice (UCLASS_BLK)
 * @agent_handle:	handle of the EFI block driver
 *
 * Create an efi_disk object which is associated with @dev.
 * The type of @dev must be UCLASS_BLK.
 *
 * Return:		0 on success, -1 otherwise
 */
static int efi_disk_create_raw(struct udevice *dev, efi_handle_t agent_handle)
{
	struct efi_disk_obj *disk;
	struct blk_desc *desc;
	int diskid;
	efi_status_t ret;

	desc = dev_get_uclass_plat(dev);
	diskid = desc->devnum;

	ret = efi_disk_add_dev(NULL, NULL, desc,
			       NULL, 0, &disk, agent_handle);
	if (ret != EFI_SUCCESS) {
		if (ret == EFI_NOT_READY) {
			log_notice("Disk %s not ready\n", dev->name);
			ret = -EBUSY;
		} else {
			log_err("Adding block device %s failed, r = %lu\n",
				dev->name, ret & ~EFI_ERROR_MASK);
			ret = -ENOENT;
		}

		return ret;
	}
	if (efi_link_dev(&disk->header, dev)) {
		efi_disk_free_diskobj(disk);

		return -EINVAL;
	}

	return 0;
}

/**
 * efi_disk_create_part() - create a handle for a disk partition
 *
 * @dev:		udevice (UCLASS_PARTITION)
 * @agent_handle:	handle of the EFI block driver
 *
 * Create an efi_disk object which is associated with @dev.
 * The type of @dev must be UCLASS_PARTITION.
 *
 * Return:		0 on success, -1 otherwise
 */
static int efi_disk_create_part(struct udevice *dev, efi_handle_t agent_handle)
{
	efi_handle_t parent;
	struct blk_desc *desc;
	struct disk_part *part_data;
	struct disk_partition *info;
	unsigned int part;
	int diskid;
	struct efi_handler *handler;
	struct efi_device_path *dp_parent;
	struct efi_disk_obj *disk;
	efi_status_t ret;

	if (dev_tag_get_ptr(dev_get_parent(dev), DM_TAG_EFI, (void **)&parent))
		return -1;

	desc = dev_get_uclass_plat(dev_get_parent(dev));
	diskid = desc->devnum;

	part_data = dev_get_uclass_plat(dev);
	part = part_data->partnum;
	info = &part_data->gpt_part_info;

	ret = efi_search_protocol(parent, &efi_guid_device_path, &handler);
	if (ret != EFI_SUCCESS)
		return -1;
	dp_parent = (struct efi_device_path *)handler->protocol_interface;

	ret = efi_disk_add_dev(parent, dp_parent, desc,
			       info, part, &disk, agent_handle);
	if (ret != EFI_SUCCESS) {
		log_err("Adding partition for %s failed\n", dev->name);
		return -1;
	}
	if (efi_link_dev(&disk->header, dev)) {
		efi_disk_free_diskobj(disk);

		/* TODO: closing the parent EFI_BLOCK_IO_PROTOCOL is missing. */

		return -1;
	}

	return 0;
}

/**
 * efi_disk_probe() - create efi_disk objects for a block device
 *
 * @ctx:	event context - driver binding protocol
 * @event:	EV_PM_POST_PROBE event
 *
 * Create efi_disk objects for partitions as well as a raw disk
 * which is associated with @dev.
 * The type of @dev must be UCLASS_BLK.
 * This function is expected to be called at EV_PM_POST_PROBE.
 *
 * Return:	0 on success, -1 otherwise
 */
int efi_disk_probe(void *ctx, struct event *event)
{
	struct udevice *dev;
	enum uclass_id id;
	struct blk_desc *desc;
	struct udevice *child;
	struct efi_driver_binding_extended_protocol *db_prot = ctx;
	efi_handle_t agent_handle = db_prot->bp.driver_binding_handle;
	int ret;

	dev = event->data.dm.dev;
	id = device_get_uclass_id(dev);

	/* We won't support partitions in a partition */
	if (id != UCLASS_BLK)
		return 0;

	/*
	 * Avoid creating duplicated objects now that efi_driver
	 * has already created an efi_disk at this moment.
	 */
	desc = dev_get_uclass_plat(dev);
	if (desc->uclass_id != UCLASS_EFI_LOADER) {
		ret = efi_disk_create_raw(dev, agent_handle);
		if (ret)
			return -1;
	}

	device_foreach_child(child, dev) {
		ret = efi_disk_create_part(child, agent_handle);
		if (ret)
			return -1;
	}

	/* only do the boot option management when UEFI sub-system is initialized */
	if (IS_ENABLED(CONFIG_CMD_BOOTEFI_BOOTMGR) && efi_obj_list_initialized == EFI_SUCCESS) {
		ret = efi_bootmgr_update_media_device_boot_option();
		if (ret != EFI_SUCCESS)
			return -1;
	}

	return 0;
}

/**
 * efi_disk_remove - delete an efi_disk object for a block device or partition
 *
 * @ctx:	event context: driver binding protocol
 * @event:	EV_PM_PRE_REMOVE event
 *
 * Delete an efi_disk object which is associated with the UCLASS_BLK or
 * UCLASS_PARTITION device for which the EV_PM_PRE_REMOVE event is raised.
 *
 * Return:	0 on success, -1 otherwise
 */
int efi_disk_remove(void *ctx, struct event *event)
{
	enum uclass_id id;
	struct udevice *dev = event->data.dm.dev;
	efi_handle_t handle;
	struct blk_desc *desc;
	struct efi_device_path *dp = NULL;
	struct efi_disk_obj *diskobj = NULL;
	struct efi_simple_file_system_protocol *volume = NULL;
	efi_status_t ret;

	if (dev_tag_get_ptr(dev, DM_TAG_EFI, (void **)&handle))
		return 0;

	id = device_get_uclass_id(dev);
	switch (id) {
	case UCLASS_BLK:
		desc = dev_get_uclass_plat(dev);
		if (desc && desc->uclass_id == UCLASS_EFI_LOADER)
			/*
			 * EFI application/driver manages the EFI handle,
			 * no need to delete EFI handle.
			 */
			return 0;

		diskobj = (struct efi_disk_obj *)handle;
		break;
	case UCLASS_PARTITION:
		diskobj = (struct efi_disk_obj *)handle;

		/* TODO: closing the parent EFI_BLOCK_IO_PROTOCOL is missing. */

		break;
	default:
		return 0;
	}

	dp = diskobj->dp;
	volume = diskobj->volume;

	ret = efi_delete_handle(handle);
	/* Do not delete DM device if there are still EFI drivers attached. */
	if (ret != EFI_SUCCESS)
		return -1;

	efi_free_pool(dp);
	free(volume);
	dev_tag_del(dev, DM_TAG_EFI);

	return 0;

	/*
	 * TODO A flag to distinguish below 2 different scenarios of this
	 * function call is needed:
	 * a) Unplugging of a removable media under U-Boot
	 * b) U-Boot exiting and booting an OS
	 * In case of scenario a), efi_bootmgr_update_media_device_boot_option()
	 * needs to be invoked here to update the boot options and remove the
	 * unnecessary ones.
	 */

}

/**
 * efi_disk_get_device_name() - get U-Boot device name associated with EFI handle
 *
 * @handle:	pointer to the EFI handle
 * @buf:	pointer to the buffer to store the string
 * @size:	size of buffer
 * Return:	status code
 */
efi_status_t efi_disk_get_device_name(const efi_handle_t handle, char *buf, int size)
{
	int count;
	int diskid;
	enum uclass_id id;
	unsigned int part;
	struct udevice *dev;
	struct blk_desc *desc;
	const char *if_typename;
	bool is_partition = false;
	struct disk_part *part_data;

	if (!handle || !buf || !size)
		return EFI_INVALID_PARAMETER;

	dev = handle->dev;
	id = device_get_uclass_id(dev);
	if (id == UCLASS_BLK) {
		desc = dev_get_uclass_plat(dev);
	} else if (id == UCLASS_PARTITION) {
		desc = dev_get_uclass_plat(dev_get_parent(dev));
		is_partition = true;
	} else {
		return EFI_INVALID_PARAMETER;
	}
	if_typename = blk_get_uclass_name(desc->uclass_id);
	diskid = desc->devnum;

	if (is_partition) {
		part_data = dev_get_uclass_plat(dev);
		part = part_data->partnum;
		count = snprintf(buf, size, "%s %d:%u", if_typename, diskid,
				 part);
	} else {
		count = snprintf(buf, size, "%s %d", if_typename, diskid);
	}

	if (count < 0 || (count + 1) > size)
		return EFI_INVALID_PARAMETER;

	return EFI_SUCCESS;
}

/**
 * efi_disks_register() - ensure all block devices are available in UEFI
 *
 * The function probes all block devices. As we store UEFI variables on the
 * EFI system partition this function has to be called before enabling
 * variable services.
 */
efi_status_t efi_disks_register(void)
{
	struct udevice *dev;

	uclass_foreach_dev_probe(UCLASS_BLK, dev) {
	}

	return EFI_SUCCESS;
}
