// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI-app board implementation
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <dm.h>
#include <efi.h>
#include <efi_api.h>
#include <errno.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <linux/types.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * efi_bind_block() - bind a new block device to an EFI device
 *
 * Binds a new top-level EFI_MEDIA device as well as a child block device so
 * that the block device can be accessed in U-Boot.
 *
 * The device can then be accessed using 'part list efi 0', 'fat ls efi 0:1',
 * for example, just like any other interface type.
 *
 * @handle: handle of the controller on which this driver is installed
 * @blkio: block io protocol proxied by this driver
 * @device_path: EFI device path structure for this
 * @len: Length of @device_path in bytes
 * @devp: Returns the bound device
 * Return: 0 if OK, -ve on error
 */
int efi_bind_block(efi_handle_t handle, struct efi_block_io *blkio,
		   struct efi_device_path *device_path, int len,
		   struct udevice **devp)
{
	struct efi_media_plat plat;
	struct udevice *dev;
	char name[18];
	int ret;

	plat.handle = handle;
	plat.blkio = blkio;
	plat.device_path = malloc(device_path->length);
	if (!plat.device_path)
		return log_msg_ret("path", -ENOMEM);
	memcpy(plat.device_path, device_path, device_path->length);
	ret = device_bind(dm_root(), DM_DRIVER_GET(efi_media), "efi_media",
			  &plat, ofnode_null(), &dev);
	if (ret)
		return log_msg_ret("bind", ret);

	snprintf(name, sizeof(name), "efi_media_%x", dev_seq(dev));
	device_set_name(dev, name);
	*devp = dev;

	return 0;
}

/**
 * devpath_is_partition() - Figure out if a device path is a partition
 *
 * Checks if a device path refers to a partition on some media device. This
 * works by checking for a valid partition number in a hard-driver media device
 * as the final component of the device path.
 *
 * @path:	device path
 * Return:	true if a partition, false if not
 *		(e.g. it might be media which contains partitions)
 */
static bool devpath_is_partition(const struct efi_device_path *path)
{
	const struct efi_device_path *p;
	bool was_part = false;

	for (p = path; p->type != DEVICE_PATH_TYPE_END;
	     p = (void *)p + p->length) {
		was_part = false;
		if (p->type == DEVICE_PATH_TYPE_MEDIA_DEVICE &&
		    p->sub_type == DEVICE_PATH_SUB_TYPE_HARD_DRIVE_PATH) {
			struct efi_device_path_hard_drive_path *hd =
				(void *)path;

			if (hd->partition_number)
				was_part = true;
		}
	}

	return was_part;
}

/**
 * setup_block() - Find all block devices and setup EFI devices for them
 *
 * Partitions are ignored, since U-Boot has partition handling. Errors with
 * particular devices produce a warning but execution continues to try to
 * find others.
 *
 * Return: 0 if found, -ENOSYS if there is no boot-services table, -ENOTSUPP
 *	if a required protocol is not supported
 */
static int setup_block(void)
{
	efi_guid_t efi_blkio_guid = EFI_BLOCK_IO_PROTOCOL_GUID;
	efi_guid_t efi_devpath_guid = EFI_DEVICE_PATH_PROTOCOL_GUID;
	efi_guid_t efi_pathutil_guid = EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID;
	efi_guid_t efi_pathtext_guid = EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;
	struct efi_boot_services *boot = efi_get_boot();
	struct efi_device_path_utilities_protocol *util;
	struct efi_device_path_to_text_protocol *text;
	struct efi_device_path *path;
	struct efi_block_io *blkio;
	efi_uintn_t num_handles;
	efi_handle_t *handle;
	int ret, i;

	if (!boot)
		return log_msg_ret("sys", -ENOSYS);

	/* Find all devices which support the block I/O protocol */
	ret = boot->locate_handle_buffer(BY_PROTOCOL, &efi_blkio_guid, NULL,
				  &num_handles, &handle);
	if (ret)
		return log_msg_ret("loc", -ENOTSUPP);
	log_debug("Found %d handles:\n", (int)num_handles);

	/* We need to look up the path size and convert it to text */
	ret = boot->locate_protocol(&efi_pathutil_guid, NULL, (void **)&util);
	if (ret)
		return log_msg_ret("util", -ENOTSUPP);
	ret = boot->locate_protocol(&efi_pathtext_guid, NULL, (void **)&text);
	if (ret)
		return log_msg_ret("text", -ENOTSUPP);

	for (i = 0; i < num_handles; i++) {
		struct udevice *dev;
		const u16 *name;
		bool is_part;
		int len;

		ret = boot->handle_protocol(handle[i], &efi_devpath_guid,
					    (void **)&path);
		if (ret) {
			log_warning("- devpath %d failed (ret=%d)\n", i, ret);
			continue;
		}

		ret = boot->handle_protocol(handle[i], &efi_blkio_guid,
					    (void **)&blkio);
		if (ret) {
			log_warning("- blkio %d failed (ret=%d)\n", i, ret);
			continue;
		}

		name = text->convert_device_path_to_text(path, true, false);
		is_part = devpath_is_partition(path);

		if (!is_part) {
			len = util->get_device_path_size(path);
			ret = efi_bind_block(handle[i], blkio, path, len, &dev);
			if (ret) {
				log_warning("- blkio bind %d failed (ret=%d)\n",
					    i, ret);
				continue;
			}
		} else {
			dev = NULL;
		}

		/*
		 * Show the device name if we created one. Otherwise indicate
		 * that it is a partition.
		 */
		printf("%2d: %-12s %ls\n", i, dev ? dev->name : "<partition>",
		       name);
	}
	boot->free_pool(handle);

	return 0;
}

/**
 * board_early_init_r() - Scan for UEFI devices that should be available
 *
 * This sets up block devices within U-Boot for those found in UEFI. With this,
 * U-Boot can access those devices
 *
 * Returns: 0 on success, -ve on error
 */
int board_early_init_r(void)
{
	if (gd->flags & GD_FLG_RELOC) {
		int ret;

		ret = setup_block();
		if (ret)
			return ret;
	}

	return 0;
}
