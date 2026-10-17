// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI boot manager
 *
 *  Copyright (c) 2017 Rob Clark
 */

#define LOG_CATEGORY LOGC_EFI

#include <blk.h>
#include <blkmap.h>
#include <charset.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <efi_default_filename.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <asm/unaligned.h>

static const struct efi_boot_services *bs;
static const struct efi_runtime_services *rs;

/**
 * struct uridp_context - uri device path resource
 *
 * @image_size:		image size
 * @image_addr:		image address
 * @loaded_dp:		pointer to loaded device path
 * @ramdisk_blk_dev:	pointer to the ramdisk blk device
 * @mem_handle:		efi_handle to the loaded PE-COFF image
 */
struct uridp_context {
	ulong image_size;
	ulong image_addr;
	struct efi_device_path *loaded_dp;
	struct udevice *ramdisk_blk_dev;
	efi_handle_t mem_handle;
};

const efi_guid_t efi_guid_bootmenu_auto_generated =
		EFICONFIG_AUTO_GENERATED_ENTRY_GUID;

/*
 * bootmgr implements the logic of trying to find a payload to boot
 * based on the BootOrder + BootXXXX variables, and then loading it.
 *
 * TODO detecting a special key held (f9?) and displaying a boot menu
 * like you would get on a PC would be clever.
 *
 * TODO if we had a way to write and persist variables after the OS
 * has started, we'd also want to check OsIndications to see if we
 * should do normal or recovery boot.
 */

/**
 * expand_media_path() - expand a device path for default file name
 * @device_path:	device path to check against
 *
 * If @device_path is a media or disk partition which houses a file
 * system, this function returns a full device path which contains
 * an architecture-specific default file name for removable media.
 *
 * Return:	a newly allocated device path
 */
static
struct efi_device_path *expand_media_path(struct efi_device_path *device_path)
{
	struct efi_device_path *rem, *full_path;
	efi_handle_t handle;

	if (!device_path)
		return NULL;

	/*
	 * If device_path is a (removable) media or partition which provides
	 * simple file system protocol, append a default file name to support
	 * booting from removable media.
	 */
	handle = efi_dp_find_obj(device_path,
				 &efi_simple_file_system_protocol_guid, &rem);
	if (handle) {
		if (rem->type == DEVICE_PATH_TYPE_END) {
			full_path = efi_dp_from_file(device_path,
						     "/EFI/BOOT/" BOOTEFI_NAME);
		} else {
			full_path = efi_dp_dup(device_path);
		}
	} else {
		full_path = efi_dp_dup(device_path);
	}

	return full_path;
}

/**
 * try_load_from_file_path() - try to load a file
 *
 * Given a file media path iterate through a list of handles and try to
 * to load the file from each of them until the first success.
 *
 * @fs_handles: array of handles with the simple file protocol
 * @num:	number of handles in fs_handles
 * @fp:		file path to open
 * @handle:	on return pointer to handle for loaded image
 * @removable:	if true only consider removable media, else only non-removable
 */
static efi_status_t try_load_from_file_path(efi_handle_t *fs_handles,
					    efi_uintn_t num,
					    struct efi_device_path *fp,
					    efi_handle_t *handle,
					    bool removable)
{
	struct efi_handler *handler;
	struct efi_device_path *dp;
	int i;
	efi_status_t ret;

	for (i = 0; i < num; i++) {
		if (removable != efi_disk_is_removable(fs_handles[i]))
			continue;

		ret = efi_search_protocol(fs_handles[i], &efi_guid_device_path,
					  &handler);
		if (ret != EFI_SUCCESS)
			continue;

		dp = handler->protocol_interface;
		if (!dp)
			continue;

		dp = efi_dp_concat(dp, fp, false);
		if (!dp)
			continue;

		ret = EFI_CALL(efi_load_image(true, efi_root, dp, NULL, 0,
					      handle));
		efi_free_pool(dp);
		if (ret == EFI_SUCCESS)
			return ret;
	}

	return EFI_NOT_FOUND;
}

/**
 * try_load_from_short_path
 * @fp:		file path
 * @handle:	pointer to handle for newly installed image
 *
 * Enumerate all the devices which support file system operations,
 * prepend its media device path to the file path, @fp, and
 * try to load the file.
 * This function should be called when handling a short-form path
 * which is starting with a file device path.
 *
 * Return:	status code
 */
static efi_status_t try_load_from_short_path(struct efi_device_path *fp,
					     efi_handle_t *handle)
{
	efi_handle_t *fs_handles;
	efi_uintn_t num;
	efi_status_t ret;

	ret = EFI_CALL(efi_locate_handle_buffer(
					BY_PROTOCOL,
					&efi_simple_file_system_protocol_guid,
					NULL,
					&num, &fs_handles));
	if (ret != EFI_SUCCESS)
		return ret;
	if (!num)
		return EFI_NOT_FOUND;

	/* removable media first */
	ret = try_load_from_file_path(fs_handles, num, fp, handle, true);
	if (ret == EFI_SUCCESS)
		goto out;

	/* fixed media */
	ret = try_load_from_file_path(fs_handles, num, fp, handle, false);
	if (ret == EFI_SUCCESS)
		goto out;

out:
	return ret;
}

/**
 * mount_image() - mount the image with blkmap
 *
 * @lo_label:	u16 label string of load option
 * @addr:	image address
 * @size:	image size
 * Return:	pointer to the UCLASS_BLK udevice, NULL if failed
 */
static struct udevice *mount_image(u16 *lo_label, ulong addr, ulong size)
{
	int err;
	struct blkmap *bm;
	struct udevice *bm_dev;
	char *label = NULL, *p;

	label = efi_alloc(utf16_utf8_strlen(lo_label) + 1);
	if (!label)
		return NULL;

	p = label;
	utf16_utf8_strcpy(&p, lo_label);
	err = blkmap_create_ramdisk(label, addr, size, &bm_dev);
	if (err) {
		efi_free_pool(label);
		return NULL;
	}
	bm = dev_get_plat(bm_dev);

	efi_free_pool(label);

	return bm->blk;
}

/**
 * search_default_file() - search default file
 *
 * @dev:	pointer to the UCLASS_BLK or UCLASS_PARTITION udevice
 * @loaded_dp:	pointer to default file device path
 * Return:	status code
 */
static efi_status_t search_default_file(struct udevice *dev,
					struct efi_device_path **loaded_dp)
{
	efi_status_t ret;
	efi_handle_t handle;
	u16 *default_file_name = NULL;
	struct efi_file_handle *root, *f;
	struct efi_device_path *dp = NULL, *fp = NULL;
	struct efi_simple_file_system_protocol *file_system;
	struct efi_device_path *device_path, *full_path = NULL;

	if (dev_tag_get_ptr(dev, DM_TAG_EFI, (void **)&handle)) {
		log_warning("DM_TAG_EFI not found\n");
		return EFI_INVALID_PARAMETER;
	}

	ret = EFI_CALL(bs->open_protocol(handle, &efi_guid_device_path,
					 (void **)&device_path, efi_root, NULL,
					 EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	if (ret != EFI_SUCCESS)
		return ret;

	ret = EFI_CALL(bs->open_protocol(handle, &efi_simple_file_system_protocol_guid,
					 (void **)&file_system, efi_root, NULL,
					 EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	if (ret != EFI_SUCCESS)
		return ret;

	ret = EFI_CALL(file_system->open_volume(file_system, &root));
	if (ret != EFI_SUCCESS)
		return ret;

	full_path = expand_media_path(device_path);
	ret = efi_dp_split_file_path(full_path, &dp, &fp);
	if (ret != EFI_SUCCESS)
		goto err;

	default_file_name = efi_dp_str(fp);
	efi_free_pool(dp);
	efi_free_pool(fp);
	if (!default_file_name) {
		ret = EFI_OUT_OF_RESOURCES;
		goto err;
	}

	ret = EFI_CALL(root->open(root, &f, default_file_name,
				  EFI_FILE_MODE_READ, 0));
	efi_free_pool(default_file_name);
	if (ret != EFI_SUCCESS)
		goto err;

	EFI_CALL(f->close(f));
	EFI_CALL(root->close(root));

	*loaded_dp = full_path;

	return EFI_SUCCESS;

err:
	EFI_CALL(root->close(root));
	efi_free_pool(full_path);

	return ret;
}

/**
 * fill_default_file_path() - get fallback boot device path for block device
 *
 * Provide the device path to the fallback UEFI boot file, e.g.
 * EFI/BOOT/BOOTAA64.EFI if that file exists on the block device @blk.
 *
 * @blk:	pointer to the UCLASS_BLK udevice
 * @dp:		pointer to store the fallback boot device path
 * Return:	status code
 */
static efi_status_t fill_default_file_path(struct udevice *blk,
					   struct efi_device_path **dp)
{
	efi_status_t ret;
	struct udevice *partition;

	/* image that has no partition table but a file system */
	ret = search_default_file(blk, dp);
	if (ret == EFI_SUCCESS)
		return ret;

	/* try the partitions */
	device_foreach_child(partition, blk) {
		enum uclass_id id;

		id = device_get_uclass_id(partition);
		if (id != UCLASS_PARTITION)
			continue;

		ret = search_default_file(partition, dp);
		if (ret == EFI_SUCCESS)
			return ret;
	}

	return EFI_NOT_FOUND;
}

/**
 * prepare_loaded_image() - prepare ramdisk for downloaded image
 *
 * @label:	label of load option
 * @addr:	image address
 * @size:	image size
 * @dp:		pointer to default file device path
 * @blk:	pointer to created blk udevice
 * Return:	status code
 */
static efi_status_t prepare_loaded_image(u16 *label, ulong addr, ulong size,
					 struct efi_device_path **dp,
					 struct udevice **blk)
{
	efi_status_t ret;
	struct udevice *ramdisk_blk;

	ramdisk_blk = mount_image(label, addr, size);
	if (!ramdisk_blk)
		return EFI_LOAD_ERROR;

	ret = fill_default_file_path(ramdisk_blk, dp);
	if (ret != EFI_SUCCESS) {
		log_info("Cannot boot from downloaded image\n");
		goto err;
	}

	/*
	 * TODO: expose the ramdisk to OS.
	 * Need to pass the ramdisk information by the architecture-specific
	 * methods such as 'pmem' device-tree node.
	 */
	ret = efi_add_memory_map(addr, size, EFI_RESERVED_MEMORY_TYPE);
	if (ret != EFI_SUCCESS) {
		log_err("Memory reservation failed\n");
		goto err;
	}

	*blk = ramdisk_blk;

	return EFI_SUCCESS;

err:
	if (blkmap_destroy(ramdisk_blk->parent))
		log_err("Destroying blkmap failed\n");

	return ret;
}

/**
 * efi_bootmgr_release_uridp_resource() - cleanup uri device path resource
 *
 * @ctx:	event context
 * Return:	status code
 */
efi_status_t efi_bootmgr_release_uridp_resource(struct uridp_context *ctx)
{
	efi_status_t ret = EFI_SUCCESS;

	if (!ctx)
		return ret;

	/* cleanup for iso or img image */
	if (ctx->ramdisk_blk_dev) {
		ret = efi_add_memory_map(ctx->image_addr, ctx->image_size,
					 EFI_CONVENTIONAL_MEMORY);
		if (ret != EFI_SUCCESS)
			log_err("Reclaiming memory failed\n");

		if (blkmap_destroy(ctx->ramdisk_blk_dev->parent)) {
			log_err("Destroying blkmap failed\n");
			ret = EFI_DEVICE_ERROR;
		}
	}

	/* cleanup for PE-COFF image */
	if (ctx->mem_handle) {
		ret = efi_uninstall_multiple_protocol_interfaces(
			ctx->mem_handle, &efi_guid_device_path, ctx->loaded_dp,
			NULL);
		if (ret != EFI_SUCCESS)
			log_err("Uninstall device_path protocol failed\n");
	}

	efi_free_pool(ctx->loaded_dp);
	free(ctx);

	return ret;
}

/**
 * efi_bootmgr_image_return_notify() - return to efibootmgr callback
 *
 * @event:	the event for which this notification function is registered
 * @context:	event context
 */
static void EFIAPI efi_bootmgr_image_return_notify(struct efi_event *event,
						   void *context)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %p", event, context);
	ret = efi_bootmgr_release_uridp_resource(context);
	EFI_EXIT(ret);
}

/**
 * try_load_from_uri_path() - Handle the URI device path
 *
 * @uridp:	uri device path
 * @lo_label:	label of load option
 * @handle:	pointer to handle for newly installed image
 * Return:	status code
 */
static efi_status_t try_load_from_uri_path(struct efi_device_path_uri *uridp,
					   u16 *lo_label,
					   efi_handle_t *handle)
{
	char *s;
	int err;
	int uri_len;
	efi_status_t ret;
	void *source_buffer;
	efi_uintn_t source_size;
	struct uridp_context *ctx;
	struct udevice *blk = NULL;
	struct efi_event *event = NULL;
	efi_handle_t mem_handle = NULL;
	struct efi_device_path *loaded_dp;
	static ulong image_size, image_addr;

	ctx = calloc(1, sizeof(struct uridp_context));
	if (!ctx)
		return EFI_OUT_OF_RESOURCES;

	s = env_get("loadaddr");
	if (!s) {
		log_err("Error: loadaddr is not set\n");
		ret = EFI_INVALID_PARAMETER;
		goto err;
	}

	image_addr = hextoul(s, NULL);
	err = wget_with_dns(image_addr, uridp->uri);
	if (err < 0) {
		ret = EFI_INVALID_PARAMETER;
		goto err;
	}

	image_size = env_get_hex("filesize", 0);
	if (!image_size) {
		ret = EFI_INVALID_PARAMETER;
		goto err;
	}

	/*
	 * If the file extension is ".iso" or ".img", mount it and try to load
	 * the default file.
	 * If the file is PE-COFF image, load the downloaded file.
	 */
	uri_len = strlen(uridp->uri);
	if (!strncmp(&uridp->uri[uri_len - 4], ".iso", 4) ||
	    !strncmp(&uridp->uri[uri_len - 4], ".img", 4)) {
		ret = prepare_loaded_image(lo_label, image_addr, image_size,
					   &loaded_dp, &blk);
		if (ret != EFI_SUCCESS)
			goto err;

		source_buffer = NULL;
		source_size = 0;
	} else if (efi_check_pe((void *)image_addr, image_size, NULL) == EFI_SUCCESS) {
		/*
		 * loaded_dp must exist until efi application returns,
		 * will be freed in return_to_efibootmgr event callback.
		 */
		loaded_dp = efi_dp_from_mem(EFI_RESERVED_MEMORY_TYPE,
					    (uintptr_t)image_addr, image_size);
		ret = efi_install_multiple_protocol_interfaces(
			&mem_handle, &efi_guid_device_path, loaded_dp, NULL);
		if (ret != EFI_SUCCESS)
			goto err;

		source_buffer = (void *)image_addr;
		source_size = image_size;
	} else {
		log_err("Error: file type is not supported\n");
		ret = EFI_UNSUPPORTED;
		goto err;
	}

	ctx->image_size = image_size;
	ctx->image_addr = image_addr;
	ctx->loaded_dp = loaded_dp;
	ctx->ramdisk_blk_dev = blk;
	ctx->mem_handle = mem_handle;

	ret = EFI_CALL(efi_load_image(false, efi_root, loaded_dp, source_buffer,
				      source_size, handle));
	if (ret != EFI_SUCCESS)
		goto err;

	/* create event for cleanup when the image returns or error occurs */
	ret = efi_create_event(EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
			       efi_bootmgr_image_return_notify, ctx,
			       &efi_guid_event_group_return_to_efibootmgr,
			       &event);
	if (ret != EFI_SUCCESS) {
		log_err("Creating event failed\n");
		goto err;
	}

	return ret;

err:
	efi_bootmgr_release_uridp_resource(ctx);

	return ret;
}

/**
 * try_load_from_media() - load file from media
 *
 * @file_path:		file path
 * @handle_img:		on return handle for the newly installed image
 *
 * If @file_path contains a file name, load the file.
 * If @file_path does not have a file name, search the architecture-specific
 * fallback boot file and load it.
 * TODO: If the FilePathList[0] device does not support
 * EFI_SIMPLE_FILE_SYSTEM_PROTOCOL but supports EFI_BLOCK_IO_PROTOCOL,
 * call EFI_BOOT_SERVICES.ConnectController()
 * TODO: FilePathList[0] device supports the EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
 * not based on EFI_BLOCK_IO_PROTOCOL
 *
 * Return:	status code
 */
static efi_status_t try_load_from_media(struct efi_device_path *file_path,
					efi_handle_t *handle_img)
{
	efi_handle_t handle_blkdev;
	efi_status_t ret = EFI_SUCCESS;
	struct efi_device_path *rem, *dp = NULL;
	struct efi_device_path *final_dp = file_path;

	handle_blkdev = efi_dp_find_obj(file_path, &efi_block_io_guid, &rem);
	if (handle_blkdev) {
		if (rem->type == DEVICE_PATH_TYPE_END) {
			/* no file name present, try default file */
			ret = fill_default_file_path(handle_blkdev->dev, &dp);
			if (ret != EFI_SUCCESS)
				return ret;

			final_dp = dp;
		}
	}

	ret = EFI_CALL(efi_load_image(true, efi_root, final_dp, NULL, 0, handle_img));

	efi_free_pool(dp);

	return ret;
}

/**
 * try_load_entry() - try to load image for boot option
 *
 * Attempt to load load-option number 'n', returning device_path and file_path
 * if successful. This checks that the EFI_LOAD_OPTION is active (enabled)
 * and that the specified file to boot exists.
 *
 * @n:			number of the boot option, e.g. 0x0a13 for Boot0A13
 * @handle:		on return handle for the newly installed image
 * @load_options:	load options set on the loaded image protocol
 * Return:		status code
 */
static efi_status_t try_load_entry(u16 n, efi_handle_t *handle,
				   void **load_options)
{
	struct efi_load_option lo;
	u16 varname[9];
	void *load_option;
	efi_uintn_t size;
	efi_status_t ret;

	efi_create_indexed_name(varname, sizeof(varname), "Boot", n);

	load_option = efi_get_var(varname, &efi_global_variable_guid, &size);
	if (!load_option)
		return EFI_LOAD_ERROR;

	ret = efi_deserialize_load_option(&lo, load_option, &size);
	if (ret != EFI_SUCCESS) {
		log_warning("Invalid load option for %ls\n", varname);
		goto error;
	}

	if (lo.attributes & LOAD_OPTION_ACTIVE) {
		u32 attributes;

		log_debug("trying to load \"%ls\" from %pD\n", lo.label,
			  lo.file_path);

		if (EFI_DP_TYPE(lo.file_path, MEDIA_DEVICE, FILE_PATH)) {
			/* file_path doesn't contain a device path */
			ret = try_load_from_short_path(lo.file_path, handle);
		} else if (EFI_DP_TYPE(lo.file_path, MESSAGING_DEVICE, MSG_URI)) {
			if (IS_ENABLED(CONFIG_EFI_HTTP_BOOT))
				ret = try_load_from_uri_path(
					(struct efi_device_path_uri *)lo.file_path,
					lo.label, handle);
			else
				ret = EFI_LOAD_ERROR;
		} else {
			ret = try_load_from_media(lo.file_path, handle);
		}
		if (ret != EFI_SUCCESS) {
			log_warning("Loading %ls '%ls' failed\n",
				    varname, lo.label);
			goto error;
		}

		attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS |
			     EFI_VARIABLE_RUNTIME_ACCESS;
		ret = efi_set_variable_int(u"BootCurrent",
					   &efi_global_variable_guid,
					   attributes, sizeof(n), &n, false);
		if (ret != EFI_SUCCESS)
			goto unload;
		/* try to register load file2 for initrd's */
		if (IS_ENABLED(CONFIG_EFI_LOAD_FILE2_INITRD)) {
			ret = efi_initrd_register();
			if (ret != EFI_SUCCESS)
				goto unload;
		}

		log_info("Booting: %ls\n", lo.label);
	} else {
		ret = EFI_LOAD_ERROR;
	}

	/* Set load options */
	if (size >= sizeof(efi_guid_t) &&
	    !guidcmp(lo.optional_data, &efi_guid_bootmenu_auto_generated))
		size = 0;

	if (size) {
		*load_options = malloc(size);
		if (!*load_options) {
			ret = EFI_OUT_OF_RESOURCES;
			goto error;
		}
		memcpy(*load_options, lo.optional_data, size);
		ret = efi_set_load_options(*handle, size, *load_options);
	} else {
		*load_options = NULL;
	}

error:
	free(load_option);

	return ret;

unload:
	if (EFI_CALL(efi_unload_image(*handle)) != EFI_SUCCESS)
		log_err("Unloading image failed\n");
	free(load_option);

	return ret;
}

/**
 * efi_bootmgr_load() - try to load from BootNext or BootOrder
 *
 * Attempt to load from BootNext or in the order specified by BootOrder
 * EFI variable, the available load-options, finding and returning
 * the first one that can be loaded successfully.
 *
 * @handle:		on return handle for the newly installed image
 * @load_options:	load options set on the loaded image protocol
 * Return:		status code
 */
efi_status_t efi_bootmgr_load(efi_handle_t *handle, void **load_options)
{
	u16 bootnext, *bootorder;
	efi_uintn_t size;
	int i, num;
	efi_status_t ret;

	bs = systab.boottime;
	rs = systab.runtime;

	/* BootNext */
	size = sizeof(bootnext);
	ret = efi_get_variable_int(u"BootNext",
				   &efi_global_variable_guid,
				   NULL, &size, &bootnext, NULL);
	if (ret == EFI_SUCCESS || ret == EFI_BUFFER_TOO_SMALL) {
		/* BootNext does exist here */
		if (ret == EFI_BUFFER_TOO_SMALL || size != sizeof(u16))
			log_err("BootNext must be 16-bit integer\n");

		/* delete BootNext */
		ret = efi_set_variable_int(u"BootNext",
					   &efi_global_variable_guid,
					   0, 0, NULL, false);

		/* load BootNext */
		if (ret == EFI_SUCCESS) {
			if (size == sizeof(u16)) {
				ret = try_load_entry(bootnext, handle,
						     load_options);
				if (ret == EFI_SUCCESS)
					return ret;
				log_warning(
					"Loading from BootNext failed, falling back to BootOrder\n");
			}
		} else {
			log_err("Deleting BootNext failed\n");
		}
	}

	/* BootOrder */
	bootorder = efi_get_var(u"BootOrder", &efi_global_variable_guid, &size);
	if (!bootorder) {
		log_info("BootOrder not defined\n");
		ret = EFI_NOT_FOUND;
		goto error;
	}

	num = size / sizeof(uint16_t);
	for (i = 0; i < num; i++) {
		log_debug("trying to load Boot%04X\n", bootorder[i]);
		ret = try_load_entry(bootorder[i], handle, load_options);
		if (ret == EFI_SUCCESS)
			break;
	}

	free(bootorder);

error:
	return ret;
}

/**
 * efi_bootmgr_enumerate_boot_options() - enumerate the possible bootable media
 *
 * @opt:		pointer to the media boot option structure
 * @index:		index of the opt array to store the boot option
 * @handles:		pointer to block device handles
 * @count:		On entry number of handles to block devices.
 *			On exit number of boot options.
 * @removable:		flag to parse removable only
 * Return:		status code
 */
static efi_status_t
efi_bootmgr_enumerate_boot_options(struct eficonfig_media_boot_option *opt,
				   efi_uintn_t index, efi_handle_t *handles,
				   efi_uintn_t *count, bool removable)
{
	u32 i, num = index;
	struct efi_handler *handler;
	efi_status_t ret = EFI_SUCCESS;

	for (i = 0; i < *count; i++) {
		u16 *p;
		u16 dev_name[BOOTMENU_DEVICE_NAME_MAX];
		char *optional_data;
		struct efi_load_option lo;
		char buf[BOOTMENU_DEVICE_NAME_MAX];
		struct efi_device_path *device_path;
		struct efi_device_path *short_dp;
		struct efi_block_io *blkio;

		ret = efi_search_protocol(handles[i], &efi_block_io_guid, &handler);
		blkio = handler->protocol_interface;

		if (blkio->media->logical_partition)
			continue;

		if (removable != (blkio->media->removable_media != 0))
			continue;

		ret = efi_search_protocol(handles[i], &efi_guid_device_path, &handler);
		if (ret != EFI_SUCCESS)
			continue;
		ret = efi_protocol_open(handler, (void **)&device_path,
					efi_root, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (ret != EFI_SUCCESS)
			continue;

		ret = efi_disk_get_device_name(handles[i], buf, BOOTMENU_DEVICE_NAME_MAX);
		if (ret != EFI_SUCCESS)
			continue;

		p = dev_name;
		utf8_utf16_strncpy(&p, buf, strlen(buf));

		/* prefer to short form device path */
		short_dp = efi_dp_shorten(device_path);
		if (short_dp)
			device_path = short_dp;

		lo.label = dev_name;
		lo.attributes = LOAD_OPTION_ACTIVE;
		lo.file_path = device_path;
		lo.file_path_length = efi_dp_size(device_path) + sizeof(END);
		/*
		 * Set the dedicated guid to optional_data, it is used to identify
		 * the boot option that automatically generated by the bootmenu.
		 * efi_serialize_load_option() expects optional_data is null-terminated
		 * utf8 string, so set the "1234567" string to allocate enough space
		 * to store guid, instead of realloc the load_option.
		 */
		lo.optional_data = "1234567";
		opt[num].size = efi_serialize_load_option(&lo, (u8 **)&opt[num].lo);
		if (!opt[num].size) {
			ret = EFI_OUT_OF_RESOURCES;
			goto out;
		}
		/* set the guid */
		optional_data = (char *)opt[num].lo + (opt[num].size - u16_strsize(u"1234567"));
		memcpy(optional_data, &efi_guid_bootmenu_auto_generated, sizeof(efi_guid_t));
		num++;

		if (num >= *count)
			break;
	}

out:
	*count = num;

	return ret;
}

/**
 * efi_bootmgr_delete_invalid_boot_option() - delete non-existing boot option
 *
 * @opt:		pointer to the media boot option structure
 * @count:		number of media boot option structure
 * Return:		status code
 */
static efi_status_t efi_bootmgr_delete_invalid_boot_option(struct eficonfig_media_boot_option *opt,
							   efi_status_t count)
{
	efi_uintn_t size;
	void *load_option;
	u32 i, list_size = 0;
	struct efi_load_option lo;
	u16 *var_name16 = NULL;
	u16 varname[] = u"Boot####";
	efi_status_t ret = EFI_SUCCESS;
	u16 *delete_index_list = NULL, *p;
	efi_uintn_t buf_size;

	buf_size = 128;
	var_name16 = malloc(buf_size);
	if (!var_name16)
		return EFI_OUT_OF_RESOURCES;

	var_name16[0] = 0;
	for (;;) {
		int index;
		efi_guid_t guid;
		efi_uintn_t tmp;

		ret = efi_next_variable_name(&buf_size, &var_name16, &guid);
		if (ret == EFI_NOT_FOUND) {
			/*
			 * EFI_NOT_FOUND indicates we retrieved all EFI variables.
			 * This should be treated as success.
			 */
			ret = EFI_SUCCESS;
			break;
		}

		if (ret != EFI_SUCCESS)
			goto out;

		if (!efi_varname_is_load_option(var_name16, &index))
			continue;

		efi_create_indexed_name(varname, sizeof(varname), "Boot", index);
		load_option = efi_get_var(varname, &efi_global_variable_guid, &size);
		if (!load_option)
			continue;

		tmp = size;
		ret = efi_deserialize_load_option(&lo, load_option, &size);
		if (ret != EFI_SUCCESS)
			goto next;

		if (size >= sizeof(efi_guid_bootmenu_auto_generated) &&
		    !guidcmp(lo.optional_data, &efi_guid_bootmenu_auto_generated)) {
			for (i = 0; i < count; i++) {
				if (opt[i].size == tmp &&
				    memcmp(opt[i].lo, load_option, tmp) == 0) {
					opt[i].exist = true;
					break;
				}
			}

			/*
			 * The entire list of variables must be retrieved by
			 * efi_get_next_variable_name_int() before deleting the invalid
			 * boot option, just save the index here.
			 */
			if (i == count) {
				p = realloc(delete_index_list, sizeof(u32) *
					    (list_size + 1));
				if (!p) {
					ret = EFI_OUT_OF_RESOURCES;
					goto out;
				}
				delete_index_list = p;
				delete_index_list[list_size++] = index;
			}
		}
next:
		free(load_option);
	}

	/* delete all invalid boot options */
	for (i = 0; i < list_size; i++) {
		ret = efi_bootmgr_delete_boot_option(delete_index_list[i]);
		if (ret != EFI_SUCCESS)
			goto out;
	}

out:
	free(var_name16);
	free(delete_index_list);

	return ret;
}

/**
 * efi_bootmgr_get_unused_bootoption() - get unused "Boot####" index
 *
 * @buf:	pointer to the buffer to store boot option variable name
 * @buf_size:	buffer size
 * @index:	pointer to store the index in the BootOrder variable
 * Return:	status code
 */
efi_status_t efi_bootmgr_get_unused_bootoption(u16 *buf, efi_uintn_t buf_size,
					       unsigned int *index)
{
	u32 i;
	efi_status_t ret;
	efi_uintn_t size;

	if (buf_size < u16_strsize(u"Boot####"))
		return EFI_BUFFER_TOO_SMALL;

	for (i = 0; i <= 0xFFFF; i++) {
		size = 0;
		efi_create_indexed_name(buf, buf_size, "Boot", i);
		ret = efi_get_variable_int(buf, &efi_global_variable_guid,
					   NULL, &size, NULL, NULL);
		if (ret == EFI_BUFFER_TOO_SMALL)
			continue;
		else
			break;
	}

	if (i > 0xFFFF)
		return EFI_OUT_OF_RESOURCES;

	*index = i;

	return EFI_SUCCESS;
}

/**
 * efi_bootmgr_append_bootorder() - append new boot option in BootOrder variable
 *
 * @index:	"Boot####" index to append to BootOrder variable
 * Return:	status code
 */
efi_status_t efi_bootmgr_append_bootorder(u16 index)
{
	u16 *bootorder;
	efi_status_t ret;
	u16 *new_bootorder = NULL;
	efi_uintn_t last, size, new_size;

	/* append new boot option */
	bootorder = efi_get_var(u"BootOrder", &efi_global_variable_guid, &size);
	last = size / sizeof(u16);
	new_size = size + sizeof(u16);
	new_bootorder = calloc(1, new_size);
	if (!new_bootorder) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	memcpy(new_bootorder, bootorder, size);
	new_bootorder[last] = index;

	ret = efi_set_variable_int(u"BootOrder", &efi_global_variable_guid,
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   new_size, new_bootorder, false);
	if (ret != EFI_SUCCESS)
		goto out;

out:
	free(bootorder);
	free(new_bootorder);

	return ret;
}

/**
 * efi_bootmgr_delete_boot_option() - delete selected boot option
 *
 * @boot_index:	boot option index to delete
 * Return:	status code
 */
efi_status_t efi_bootmgr_delete_boot_option(u16 boot_index)
{
	u16 *bootorder;
	u16 varname[9];
	efi_status_t ret;
	unsigned int index;
	efi_uintn_t num, size;

	efi_create_indexed_name(varname, sizeof(varname),
				"Boot", boot_index);
	ret = efi_set_variable_int(varname, &efi_global_variable_guid,
				   0, 0, NULL, false);
	if (ret != EFI_SUCCESS) {
		log_err("delete boot option(%ls) failed\n", varname);
		return ret;
	}

	/* update BootOrder if necessary */
	bootorder = efi_get_var(u"BootOrder", &efi_global_variable_guid, &size);
	if (!bootorder)
		return EFI_SUCCESS;

	num = size / sizeof(u16);
	if (!efi_search_bootorder(bootorder, num, boot_index, &index))
		return EFI_SUCCESS;

	memmove(&bootorder[index], &bootorder[index + 1],
		(num - index - 1) * sizeof(u16));
	size -= sizeof(u16);
	ret = efi_set_variable_int(u"BootOrder", &efi_global_variable_guid,
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   size, bootorder, false);

	return ret;
}

/**
 * efi_bootmgr_update_media_device_boot_option() - generate the media device boot option
 *
 * This function enumerates all BlockIo devices and add the boot option for it.
 * This function also provide the BOOT#### variable maintenance for
 * the media device entries.
 * - Automatically create the BOOT#### variable for the newly detected device,
 * this BOOT#### variable is distinguished by the special GUID
 * stored in the EFI_LOAD_OPTION.optional_data
 * - If the device is not attached to the system, the associated BOOT#### variable
 * is automatically deleted.
 *
 * Return:	status code
 */
efi_status_t efi_bootmgr_update_media_device_boot_option(void)
{
	u32 i;
	efi_status_t ret;
	efi_uintn_t count, num, total;
	efi_handle_t *handles = NULL;
	struct eficonfig_media_boot_option *opt = NULL;

	ret = efi_locate_handle_buffer_int(BY_PROTOCOL,
					   &efi_block_io_guid,
					   NULL, &count,
					   (efi_handle_t **)&handles);
	if (ret != EFI_SUCCESS)
		goto out;

	opt = calloc(count, sizeof(struct eficonfig_media_boot_option));
	if (!opt) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

	/* parse removable block io followed by fixed block io */
	num = count;
	ret = efi_bootmgr_enumerate_boot_options(opt, 0, handles, &num, true);
	if (ret != EFI_SUCCESS)
		goto out;

	total = num;
	num = count;
	ret = efi_bootmgr_enumerate_boot_options(opt, total, handles, &num, false);
	if (ret != EFI_SUCCESS)
		goto out;

	total = num;

	/*
	 * System hardware configuration may vary depending on the user setup.
	 * The boot option is automatically added by the bootmenu.
	 * If the device is not attached to the system, the boot option needs
	 * to be deleted.
	 */
	ret = efi_bootmgr_delete_invalid_boot_option(opt, total);
	if (ret != EFI_SUCCESS)
		goto out;

	/* add non-existent boot option */
	for (i = 0; i < total; i++) {
		u32 boot_index;
		u16 var_name[9];

		if (!opt[i].exist) {
			ret = efi_bootmgr_get_unused_bootoption(var_name, sizeof(var_name),
								&boot_index);
			if (ret != EFI_SUCCESS)
				goto out;

			ret = efi_set_variable_int(var_name, &efi_global_variable_guid,
						   EFI_VARIABLE_NON_VOLATILE |
						   EFI_VARIABLE_BOOTSERVICE_ACCESS |
						   EFI_VARIABLE_RUNTIME_ACCESS,
						   opt[i].size, opt[i].lo, false);
			if (ret != EFI_SUCCESS)
				goto out;

			ret = efi_bootmgr_append_bootorder(boot_index);
			if (ret != EFI_SUCCESS) {
				efi_set_variable_int(var_name, &efi_global_variable_guid,
						     0, 0, NULL, false);
				goto out;
			}
		}
	}

out:
	if (opt) {
		for (i = 0; i < total; i++)
			free(opt[i].lo);
	}
	free(opt);
	efi_free_pool(handles);

	if (ret == EFI_NOT_FOUND)
		return EFI_SUCCESS;
	return ret;
}

/**
 * efi_bootmgr_run() - execute EFI boot manager
 * @fdt:	Flat device tree
 *
 * Invoke EFI boot manager and execute a binary depending on
 * boot options. If @fdt is not NULL, it will be passed to
 * the executed binary.
 *
 * Return:	status code
 */
efi_status_t efi_bootmgr_run(void *fdt)
{
	efi_handle_t handle;
	void *load_options;
	efi_status_t ret;

	/* Initialize EFI drivers */
	ret = efi_init_obj_list();
	if (ret != EFI_SUCCESS) {
		log_err("Error: Cannot initialize UEFI sub-system, r = %lu\n",
			ret & ~EFI_ERROR_MASK);
		return CMD_RET_FAILURE;
	}

	ret = efi_install_fdt(fdt);
	if (ret != EFI_SUCCESS)
		return ret;

	ret = efi_bootmgr_load(&handle, &load_options);
	if (ret != EFI_SUCCESS) {
		log_notice("EFI boot manager: Cannot load any image\n");
		return ret;
	}

	return do_bootefi_exec(handle, load_options);
}
