// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_exception
 *
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This test checks the handling of exceptions.
 *
 * The efi_selftest_miniapp_exception.efi application is loaded into memory
 * and started.
 */

#include <efi_selftest.h>
/* Include containing the UEFI application */
#include "efi_miniapp_file_image_exception.h"

/* Block size of compressed disk image */
#define COMPRESSED_DISK_IMAGE_BLOCK_SIZE 8

/* Binary logarithm of the block size */
#define LB_BLOCK_SIZE 9

/* File device path for LoadImage() */
static struct {
	struct efi_device_path dp;
	u16 filename[8];
	struct efi_device_path end;
} dp = {
	{
		DEVICE_PATH_TYPE_MEDIA_DEVICE,
		DEVICE_PATH_SUB_TYPE_FILE_PATH,
		sizeof(dp.dp) + sizeof(dp.filename),
	},
	u"bug.efi",
	{
		DEVICE_PATH_TYPE_END,
		DEVICE_PATH_SUB_TYPE_END,
		sizeof(dp.end),
	}
};

static efi_handle_t image_handle;
static struct efi_boot_services *boottime;

/* One 8 byte block of the compressed disk image */
struct line {
	size_t addr;
	char *line;
};

/* Compressed file image */
struct compressed_file_image {
	size_t length;
	struct line lines[];
};

static struct compressed_file_image img = EFI_ST_DISK_IMG;

/* Decompressed file image */
static u8 *image;

/*
 * Decompress the disk image.
 *
 * @image	decompressed disk image
 * Return:	status code
 */
static efi_status_t decompress(u8 **image)
{
	u8 *buf;
	size_t i;
	size_t addr;
	size_t len;
	efi_status_t ret;

	ret = boottime->allocate_pool(EFI_LOADER_DATA, img.length,
				      (void **)&buf);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Out of memory\n");
		return ret;
	}
	boottime->set_mem(buf, img.length, 0);

	for (i = 0; ; ++i) {
		if (!img.lines[i].line)
			break;
		addr = img.lines[i].addr;
		len = COMPRESSED_DISK_IMAGE_BLOCK_SIZE;
		if (addr + len > img.length)
			len = img.length - addr;
		boottime->copy_mem(buf + addr, img.lines[i].line, len);
	}
	*image = buf;
	return ret;
}

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	image_handle = handle;
	boottime = systable->boottime;

	/* Load the application image into memory */
	decompress(&image);

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Load and start the application image.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	efi_handle_t handle;

	ret = boottime->load_image(false, image_handle, &dp.dp, image,
				   img.length, &handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to load image\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->start_image(handle, NULL, NULL);

	efi_st_error("Exception not triggered\n");

	return EFI_ST_FAILURE;
}

EFI_UNIT_TEST(exception) = {
	.name = "exception",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.on_request = true,
};
