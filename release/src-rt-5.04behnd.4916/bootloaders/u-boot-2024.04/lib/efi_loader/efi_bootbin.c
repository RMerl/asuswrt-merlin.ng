// SPDX-License-Identifier: GPL-2.0+
/*
 *  For the code moved from cmd/bootefi.c
 *  Copyright (c) 2016 Alexander Graf
 */

#define LOG_CATEGORY LOGC_EFI

#include <charset.h>
#include <efi.h>
#include <efi_loader.h>
#include <env.h>
#include <image.h>
#include <log.h>
#include <malloc.h>

static struct efi_device_path *bootefi_image_path;
static struct efi_device_path *bootefi_device_path;
static void *image_addr;
static size_t image_size;

/**
 * efi_get_image_parameters() - return image parameters
 *
 * @img_addr:		address of loaded image in memory
 * @img_size:		size of loaded image
 */
void efi_get_image_parameters(void **img_addr, size_t *img_size)
{
	*img_addr = image_addr;
	*img_size = image_size;
}

/**
 * efi_clear_bootdev() - clear boot device
 */
void efi_clear_bootdev(void)
{
	efi_free_pool(bootefi_device_path);
	efi_free_pool(bootefi_image_path);
	bootefi_device_path = NULL;
	bootefi_image_path = NULL;
	image_addr = NULL;
	image_size = 0;
}

/**
 * efi_set_bootdev() - set boot device
 *
 * This function is called when a file is loaded, e.g. via the 'load' command.
 * We use the path to this file to inform the UEFI binary about the boot device.
 *
 * @dev:		device, e.g. "MMC"
 * @devnr:		number of the device, e.g. "1:2"
 * @path:		path to file loaded
 * @buffer:		buffer with file loaded
 * @buffer_size:	size of file loaded
 */
void efi_set_bootdev(const char *dev, const char *devnr, const char *path,
		     void *buffer, size_t buffer_size)
{
	struct efi_device_path *device, *image;
	efi_status_t ret;

	log_debug("dev=%s, devnr=%s, path=%s, buffer=%p, size=%zx\n", dev,
		  devnr, path, buffer, buffer_size);

	/* Forget overwritten image */
	if (buffer + buffer_size >= image_addr &&
	    image_addr + image_size >= buffer)
		efi_clear_bootdev();

	/* Remember only PE-COFF and FIT images */
	if (efi_check_pe(buffer, buffer_size, NULL) != EFI_SUCCESS) {
		if (IS_ENABLED(CONFIG_FIT) &&
		    !fit_check_format(buffer, IMAGE_SIZE_INVAL)) {
			/*
			 * FIT images of type EFI_OS are started via command
			 * bootm. We should not use their boot device with the
			 * bootefi command.
			 */
			buffer = 0;
			buffer_size = 0;
		} else {
			log_debug("- not remembering image\n");
			return;
		}
	}

	/* efi_set_bootdev() is typically called repeatedly, recover memory */
	efi_clear_bootdev();

	image_addr = buffer;
	image_size = buffer_size;

	ret = efi_dp_from_name(dev, devnr, path, &device, &image);
	if (ret == EFI_SUCCESS) {
		bootefi_device_path = device;
		if (image) {
			/* FIXME: image should not contain device */
			struct efi_device_path *image_tmp = image;

			efi_dp_split_file_path(image, &device, &image);
			efi_free_pool(image_tmp);
		}
		bootefi_image_path = image;
		log_debug("- boot device %pD\n", device);
		if (image)
			log_debug("- image %pD\n", image);
	} else {
		log_debug("- efi_dp_from_name() failed, err=%lx\n", ret);
		efi_clear_bootdev();
	}
}

/**
 * efi_run_image() - run loaded UEFI image
 *
 * @source_buffer:	memory address of the UEFI image
 * @source_size:	size of the UEFI image
 * Return:		status code
 */
efi_status_t efi_run_image(void *source_buffer, efi_uintn_t source_size)
{
	efi_handle_t mem_handle = NULL, handle;
	struct efi_device_path *file_path = NULL;
	struct efi_device_path *msg_path;
	efi_status_t ret;
	u16 *load_options;

	if (!bootefi_device_path || !bootefi_image_path) {
		log_debug("Not loaded from disk\n");
		/*
		 * Special case for efi payload not loaded from disk,
		 * such as 'bootefi hello' or for example payload
		 * loaded directly into memory via JTAG, etc:
		 */
		file_path = efi_dp_from_mem(EFI_RESERVED_MEMORY_TYPE,
					    (uintptr_t)source_buffer,
					    source_size);
		/*
		 * Make sure that device for device_path exist
		 * in load_image(). Otherwise, shell and grub will fail.
		 */
		ret = efi_install_multiple_protocol_interfaces(&mem_handle,
							       &efi_guid_device_path,
							       file_path, NULL);
		if (ret != EFI_SUCCESS)
			goto out;
		msg_path = file_path;
	} else {
		file_path = efi_dp_concat(bootefi_device_path,
					  bootefi_image_path, false);
		msg_path = bootefi_image_path;
		log_debug("Loaded from disk\n");
	}

	log_info("Booting %pD\n", msg_path);

	ret = EFI_CALL(efi_load_image(false, efi_root, file_path, source_buffer,
				      source_size, &handle));
	if (ret != EFI_SUCCESS) {
		log_err("Loading image failed\n");
		goto out;
	}

	/* Transfer environment variable as load options */
	ret = efi_env_set_load_options(handle, "bootargs", &load_options);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = do_bootefi_exec(handle, load_options);

out:
	if (mem_handle) {
		efi_status_t r;

		r = efi_uninstall_multiple_protocol_interfaces(
			mem_handle, &efi_guid_device_path, file_path, NULL);
		if (r != EFI_SUCCESS)
			log_err("Uninstalling protocol interfaces failed\n");
	}
	efi_free_pool(file_path);

	return ret;
}

/**
 * efi_binary_run() - run loaded UEFI image
 *
 * @image:	memory address of the UEFI image
 * @size:	size of the UEFI image
 * @fdt:	device-tree
 *
 * Execute an EFI binary image loaded at @image.
 * @size may be zero if the binary is loaded with U-Boot load command.
 *
 * Return:	status code
 */
efi_status_t efi_binary_run(void *image, size_t size, void *fdt)
{
	efi_status_t ret;

	/* Initialize EFI drivers */
	ret = efi_init_obj_list();
	if (ret != EFI_SUCCESS) {
		log_err("Error: Cannot initialize UEFI sub-system, r = %lu\n",
			ret & ~EFI_ERROR_MASK);
		return -1;
	}

	ret = efi_install_fdt(fdt);
	if (ret != EFI_SUCCESS)
		return ret;

	return efi_run_image(image, size);
}
