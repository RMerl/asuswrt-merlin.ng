// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Linaro Limited
 */

#define LOG_CATEGORY LOGC_EFI
#include <efi_loader.h>
#include <efi_load_initrd.h>
#include <efi_variable.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>

static efi_status_t EFIAPI
efi_load_file2_initrd(struct efi_load_file_protocol *this,
		      struct efi_device_path *file_path, bool boot_policy,
		      efi_uintn_t *buffer_size, void *buffer);

static const struct efi_load_file_protocol efi_lf2_protocol = {
	.load_file = efi_load_file2_initrd,
};

/*
 * Device path defined by Linux to identify the handle providing the
 * EFI_LOAD_FILE2_PROTOCOL used for loading the initial ramdisk.
 */
static const struct efi_initrd_dp dp_lf2_handle = {
	.vendor = {
		{
		   DEVICE_PATH_TYPE_MEDIA_DEVICE,
		   DEVICE_PATH_SUB_TYPE_VENDOR_PATH,
		   sizeof(dp_lf2_handle.vendor),
		},
		EFI_INITRD_MEDIA_GUID,
	},
	.end = {
		DEVICE_PATH_TYPE_END,
		DEVICE_PATH_SUB_TYPE_END,
		sizeof(dp_lf2_handle.end),
	}
};

static efi_handle_t efi_initrd_handle;

/**
 * get_initrd_fp() - Get initrd device path from a FilePathList device path
 *
 * @initrd_fp:	the final initrd filepath
 *
 * Return:	status code. Caller must free initrd_fp
 */
static efi_status_t get_initrd_fp(struct efi_device_path **initrd_fp)
{
	struct efi_device_path *dp = NULL;

	/*
	 * if bootmgr is setup with and initrd, the device path will be
	 * in the FilePathList[] of our load options in Boot####.
	 * The first device path of the multi instance device path will
	 * start with a VenMedia and the initrds will follow.
	 *
	 * If the device path is not found return EFI_INVALID_PARAMETER.
	 * We can then use this specific return value and not install the
	 * protocol, while allowing the boot to continue
	 */
	dp = efi_get_dp_from_boot(efi_lf2_initrd_guid);
	if (!dp)
		return EFI_INVALID_PARAMETER;

	*initrd_fp = dp;
	return EFI_SUCCESS;
}

/**
 * efi_load_file2_initrd() - load initial RAM disk
 *
 * This function implements the LoadFile service of the EFI_LOAD_FILE2_PROTOCOL
 * in order to load an initial RAM disk requested by the Linux kernel stub.
 *
 * See the UEFI spec for details.
 *
 * @this:			EFI_LOAD_FILE2_PROTOCOL instance
 * @file_path:			media device path of the file, "" in this case
 * @boot_policy:		must be false
 * @buffer_size:		size of allocated buffer
 * @buffer:			buffer to load the file
 *
 * Return:			status code
 */
static efi_status_t EFIAPI
efi_load_file2_initrd(struct efi_load_file_protocol *this,
		      struct efi_device_path *file_path, bool boot_policy,
		      efi_uintn_t *buffer_size, void *buffer)
{
	struct efi_device_path *initrd_fp = NULL;
	efi_status_t ret = EFI_NOT_FOUND;
	struct efi_file_handle *f = NULL;
	efi_uintn_t bs;

	EFI_ENTRY("%p, %p, %d, %p, %p", this, file_path, boot_policy,
		  buffer_size, buffer);

	if (!this || this != &efi_lf2_protocol ||
	    !buffer_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (file_path->type != dp_lf2_handle.end.type ||
	    file_path->sub_type != dp_lf2_handle.end.sub_type) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (boot_policy) {
		ret = EFI_UNSUPPORTED;
		goto out;
	}

	ret = get_initrd_fp(&initrd_fp);
	if (ret != EFI_SUCCESS)
		goto out;

	/* Open file */
	f = efi_file_from_path(initrd_fp);
	if (!f) {
		log_err("Can't find initrd specified in Boot####\n");
		ret = EFI_NOT_FOUND;
		goto out;
	}

	/* Get file size */
	ret = efi_file_size(f, &bs);
	if (ret != EFI_SUCCESS)
		goto out;

	if (!buffer || *buffer_size < bs) {
		ret = EFI_BUFFER_TOO_SMALL;
		*buffer_size = bs;
	} else {
		ret = EFI_CALL(f->read(f, &bs, (void *)(uintptr_t)buffer));
		*buffer_size = bs;
	}

out:
	efi_free_pool(initrd_fp);
	if (f)
		EFI_CALL(f->close(f));
	return EFI_EXIT(ret);
}

/**
 * check_initrd() - Determine if the file defined as an initrd in Boot####
 *		    load_options device path is present
 *
 * Return:	status code
 */
static efi_status_t check_initrd(void)
{
	struct efi_device_path *initrd_fp = NULL;
	struct efi_file_handle *f;
	efi_status_t ret;

	ret = get_initrd_fp(&initrd_fp);
	if (ret != EFI_SUCCESS)
		goto out;

	/*
	 * If the file is not found, but the file path is set, return an error
	 * and trigger the bootmgr fallback
	 */
	f = efi_file_from_path(initrd_fp);
	if (!f) {
		log_err("Can't find initrd specified in Boot####\n");
		ret = EFI_NOT_FOUND;
		goto out;
	}

	EFI_CALL(f->close(f));

out:
	efi_free_pool(initrd_fp);
	return ret;
}

/**
 * efi_initrd_register() - create handle for loading initial RAM disk
 *
 * This function creates a new handle and installs a Linux specific vendor
 * device path and an EFI_LOAD_FILE2_PROTOCOL. Linux uses the device path
 * to identify the handle and then calls the LoadFile service of the
 * EFI_LOAD_FILE2_PROTOCOL to read the initial RAM disk.
 *
 * Return:	status code
 */
efi_status_t efi_initrd_register(void)
{
	efi_status_t ret;

	/*
	 * Allow the user to continue if Boot#### file path is not set for
	 * an initrd
	 */
	ret = check_initrd();
	if (ret == EFI_INVALID_PARAMETER)
		return EFI_SUCCESS;
	if (ret != EFI_SUCCESS)
		return ret;

	ret = efi_install_multiple_protocol_interfaces(&efi_initrd_handle,
						       /* initramfs */
						       &efi_guid_device_path, &dp_lf2_handle,
						       /* LOAD_FILE2 */
						       &efi_guid_load_file2_protocol,
						       &efi_lf2_protocol,
						       NULL);

	return ret;
}

/**
 * efi_initrd_deregister() - delete the handle for loading initial RAM disk
 *
 * This will delete the handle containing the Linux specific vendor device
 * path and EFI_LOAD_FILE2_PROTOCOL for loading an initrd
 *
 * Return:	status code
 */
efi_status_t efi_initrd_deregister(void)
{
	efi_status_t ret;

	if (!efi_initrd_handle)
		return EFI_SUCCESS;

	ret = efi_uninstall_multiple_protocol_interfaces(efi_initrd_handle,
							 /* initramfs */
							 &efi_guid_device_path,
							 &dp_lf2_handle,
							 /* LOAD_FILE2 */
							 &efi_guid_load_file2_protocol,
							 &efi_lf2_protocol,
							 NULL);
	efi_initrd_handle = NULL;

	return ret;
}
