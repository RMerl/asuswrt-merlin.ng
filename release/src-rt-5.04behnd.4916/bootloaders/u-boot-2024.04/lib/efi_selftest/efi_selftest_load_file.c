// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_load_file
 *
 * Copyright (c) 2020 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This test checks the handling of the LOAD_FILE and the LOAD_FILE2 protocol
 * by the LoadImage() service.
 */

#include <efi_selftest.h>
/* Include containing the miniapp.efi application */
#include "efi_miniapp_file_image_exit.h"

/* Block size of compressed disk image */
#define COMPRESSED_DISK_IMAGE_BLOCK_SIZE 8

/* Binary logarithm of the block size */
#define LB_BLOCK_SIZE 9

#define GUID_VENDOR \
	EFI_GUID(0xdbca4c98, 0x6cb0, 0x694d, \
		 0x08, 0x72, 0x81, 0x9c, 0x65, 0xfc, 0xbb, 0xd1)

#define GUID_VENDOR2 \
	EFI_GUID(0xdbca4c98, 0x6cb0, 0x694d, \
		 0x08, 0x72, 0x81, 0x9c, 0x65, 0xfc, 0xbb, 0xd2)

#define FILE_NAME_SIZE 16

static const efi_guid_t efi_st_guid_load_file_protocol =
					EFI_LOAD_FILE_PROTOCOL_GUID;
static const efi_guid_t efi_st_guid_load_file2_protocol =
					EFI_LOAD_FILE2_PROTOCOL_GUID;
static const efi_guid_t efi_st_guid_device_path =
					EFI_DEVICE_PATH_PROTOCOL_GUID;

static efi_handle_t image_handle;
static struct efi_boot_services *boottime;
static efi_handle_t handle_lf;
static efi_handle_t handle_lf2;

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

static int load_file_call_count;
static int load_file2_call_count;

/* Decompressed file image */
static u8 *image;

static struct {
	struct efi_device_path_vendor v;
	struct efi_device_path d;
} dp_lf_prot = {
	{
		{
			DEVICE_PATH_TYPE_HARDWARE_DEVICE,
			DEVICE_PATH_SUB_TYPE_VENDOR,
			sizeof(struct efi_device_path_vendor),
		},
		GUID_VENDOR,
	},
	{
		DEVICE_PATH_TYPE_END,
		DEVICE_PATH_SUB_TYPE_END,
		sizeof(struct efi_device_path),
	},
};

static struct {
	struct efi_device_path_vendor v;
	struct efi_device_path_file_path f;
	u16 file_name[FILE_NAME_SIZE];
	struct efi_device_path e;
} dp_lf_file = {
	{
		{
			DEVICE_PATH_TYPE_HARDWARE_DEVICE,
			DEVICE_PATH_SUB_TYPE_VENDOR,
			sizeof(struct efi_device_path_vendor),
		},
		GUID_VENDOR,
	},
	{
		{
			DEVICE_PATH_TYPE_MEDIA_DEVICE,
			DEVICE_PATH_SUB_TYPE_FILE_PATH,
			sizeof(struct efi_device_path_file_path) +
			FILE_NAME_SIZE * sizeof(u16),
		}
	},
	u"\\lf.efi",
	{
		DEVICE_PATH_TYPE_END,
		DEVICE_PATH_SUB_TYPE_END,
		sizeof(struct efi_device_path),
	},
};

struct efi_device_path *dp_lf_file_remainder = &dp_lf_file.f.dp;

static struct {
	struct efi_device_path_vendor v;
	struct efi_device_path d;
} dp_lf2_prot = {
	{
		{
			DEVICE_PATH_TYPE_HARDWARE_DEVICE,
			DEVICE_PATH_SUB_TYPE_VENDOR,
			sizeof(struct efi_device_path_vendor),
		},
		GUID_VENDOR2,
	},
	{
		DEVICE_PATH_TYPE_END,
		DEVICE_PATH_SUB_TYPE_END,
		sizeof(struct efi_device_path),
	},
};

static struct {
	struct efi_device_path_vendor v;
	struct efi_device_path_file_path f;
	u16 file_name[FILE_NAME_SIZE];
	struct efi_device_path e;
} dp_lf2_file = {
	{
		{
			DEVICE_PATH_TYPE_HARDWARE_DEVICE,
			DEVICE_PATH_SUB_TYPE_VENDOR,
			sizeof(struct efi_device_path_vendor),
		},
		GUID_VENDOR2,
	},
	{
		{
			DEVICE_PATH_TYPE_MEDIA_DEVICE,
			DEVICE_PATH_SUB_TYPE_FILE_PATH,
			sizeof(struct efi_device_path_file_path) +
			FILE_NAME_SIZE * sizeof(u16),
		}
	},
	u"\\lf2.efi",
	{
		DEVICE_PATH_TYPE_END,
		DEVICE_PATH_SUB_TYPE_END,
		sizeof(struct efi_device_path),
	},
};

struct efi_device_path *dp_lf2_file_remainder = &dp_lf2_file.f.dp;

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
 * load_file() - LoadFile() service of a EFI_LOAD_FILE_PROTOCOL
 *
 * @this:		instance of EFI_LOAD_FILE_PROTOCOL
 * @file_path:		remaining device path
 * @boot_policy:	true if called by boot manager
 * @buffer_size:	(required) buffer size
 * @buffer:		buffer to which the file is to be loaded
 */
static efi_status_t EFIAPI load_file(struct efi_load_file_protocol *this,
				     struct efi_device_path *file_path,
				     bool boot_policy,
				     efi_uintn_t *buffer_size,
				     void *buffer)
{
	++load_file_call_count;
	if (memcmp(file_path, dp_lf_file_remainder,
	    sizeof(struct efi_device_path_file_path) +
	    FILE_NAME_SIZE * sizeof(u16) +
	    sizeof(struct efi_device_path))) {
		efi_st_error("Wrong remaining device path\n");
		return EFI_NOT_FOUND;
	}
	if (this->load_file != load_file) {
		efi_st_error("wrong this\n");
		return EFI_INVALID_PARAMETER;
	}
	if (*buffer_size < img.length) {
		*buffer_size = img.length;
		return EFI_BUFFER_TOO_SMALL;
	}
	memcpy(buffer, image, img.length);
	*buffer_size = img.length;
	return EFI_SUCCESS;
}


/*
 * load_file2() - LoadFile() service of a EFI_LOAD_FILE2_PROTOCOL
 *
 * @this:		instance of EFI_LOAD_FILE2_PROTOCOL
 * @file_path:		remaining device path
 * @boot_policy:	true if called by boot manager
 * @buffer_size:	(required) buffer size
 * @buffer:		buffer to which the file is to be loaded
 */
static efi_status_t EFIAPI load_file2(struct efi_load_file_protocol *this,
				      struct efi_device_path *file_path,
				      bool boot_policy,
				      efi_uintn_t *buffer_size,
				      void *buffer)
{
	++load_file2_call_count;
	if (memcmp(file_path, dp_lf2_file_remainder,
	    sizeof(struct efi_device_path_file_path) +
	    FILE_NAME_SIZE * sizeof(u16) +
	    sizeof(struct efi_device_path))) {
		efi_st_error("Wrong remaining device path\n");
		return EFI_NOT_FOUND;
	}
	if (this->load_file != load_file2) {
		efi_st_error("wrong this\n");
		return EFI_INVALID_PARAMETER;
	}
	if (boot_policy) {
		efi_st_error("LOAD_FILE2 called with boot_policy = true");
		return EFI_INVALID_PARAMETER;
	}
	if (*buffer_size < img.length) {
		*buffer_size = img.length;
		return EFI_BUFFER_TOO_SMALL;
	}
	memcpy(buffer, image, img.length);
	*buffer_size = img.length;
	return EFI_SUCCESS;
}

static struct efi_load_file_protocol lf_prot = {load_file};
static struct efi_load_file_protocol lf2_prot = {load_file2};

/*
 * Setup unit test.
 *
 * Install an EFI_LOAD_FILE_PROTOCOL and an EFI_LOAD_FILE2_PROTOCOL.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int efi_st_load_file_setup(const efi_handle_t handle,
				  const struct efi_system_table *systable)
{
	efi_status_t ret;

	image_handle = handle;
	boottime = systable->boottime;

	/* Load the application image into memory */
	decompress(&image);

	ret = boottime->install_multiple_protocol_interfaces(
		&handle_lf,
		&efi_st_guid_device_path,
		&dp_lf_prot,
		&efi_st_guid_load_file_protocol,
		&lf_prot,
		NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallMultipleProtocolInterfaces failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->install_multiple_protocol_interfaces(
		&handle_lf2,
		&efi_st_guid_device_path,
		&dp_lf2_prot,
		&efi_st_guid_load_file2_protocol,
		&lf2_prot,
		NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallMultipleProtocolInterfaces failed\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int efi_st_load_file_teardown(void)
{
	efi_status_t ret = EFI_ST_SUCCESS;

	if (handle_lf) {
		ret = boottime->uninstall_multiple_protocol_interfaces(
			handle_lf,
			&efi_st_guid_device_path,
			&dp_lf_prot,
			&efi_st_guid_load_file_protocol,
			&lf_prot,
			NULL);
		if (ret != EFI_SUCCESS) {
			efi_st_error(
				"UninstallMultipleProtocolInterfaces failed\n");
			return EFI_ST_FAILURE;
		}
	}
	if (handle_lf2) {
		ret = boottime->uninstall_multiple_protocol_interfaces(
			handle_lf2,
			&efi_st_guid_device_path,
			&dp_lf2_prot,
			&efi_st_guid_load_file2_protocol,
			&lf2_prot,
			NULL);
		if (ret != EFI_SUCCESS) {
			efi_st_error(
				"UninstallMultipleProtocolInterfaces failed\n");
			return EFI_ST_FAILURE;
		}
	}

	if (image) {
		ret = boottime->free_pool(image);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to free image\n");
			return EFI_ST_FAILURE;
		}
	}
	return ret;
}

/*
 * Execute unit test.
 *
 * Try loading an image via the EFI_LOAD_FILE_PROTOCOL and the
 * EFI_LOAD_FILE2_PROTOCOL. Finally execute the image.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int efi_st_load_file_execute(void)
{
	efi_status_t ret;
	efi_handle_t handle;
	efi_uintn_t exit_data_size = 0;
	u16 *exit_data = NULL;
	u16 expected_text[] = EFI_ST_SUCCESS_STR;

	load_file_call_count = 0;
	load_file2_call_count = 0;
	handle = NULL;
	ret = boottime->load_image(true, image_handle, &dp_lf_file.v.dp, NULL,
				   0, &handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to load image\n");
		return EFI_ST_FAILURE;
	}
	if (load_file2_call_count || !load_file_call_count) {
		efi_st_error("Wrong image loaded\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->unload_image(handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to unload image\n");
		return EFI_ST_FAILURE;
	}

	load_file_call_count = 0;
	load_file2_call_count = 0;
	handle = NULL;
	ret = boottime->load_image(false, image_handle, &dp_lf_file.v.dp, NULL,
				   0, &handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to load image\n");
		return EFI_ST_FAILURE;
	}
	if (load_file2_call_count || !load_file_call_count) {
		efi_st_error("Wrong image loaded\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->unload_image(handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to unload image\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->load_image(true, image_handle, &dp_lf2_file.v.dp, NULL,
				   0, &handle);
	if (ret != EFI_NOT_FOUND) {
		efi_st_error(
			"Boot manager should not use LOAD_FILE2_PROTOCOL\n");
		return EFI_ST_FAILURE;
	}

	load_file_call_count = 0;
	load_file2_call_count = 0;
	handle = NULL;
	ret = boottime->load_image(false, image_handle, &dp_lf2_file.v.dp, NULL,
				   0, &handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to load image\n");
		return EFI_ST_FAILURE;
	}
	if (!load_file2_call_count || load_file_call_count) {
		efi_st_error("Wrong image loaded\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->start_image(handle, &exit_data_size, &exit_data);
	if (ret != EFI_UNSUPPORTED) {
		efi_st_error("Wrong return value from application\n");
		return EFI_ST_FAILURE;
	}
	if (!exit_data || exit_data_size != sizeof(expected_text) ||
	    memcmp(exit_data, expected_text, sizeof(expected_text))) {
		efi_st_error("Incorrect exit data\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(exit_data);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to free exit data\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(load_file_protocol) = {
	.name = "load file protocol",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = efi_st_load_file_setup,
	.execute = efi_st_load_file_execute,
	.teardown = efi_st_load_file_teardown,
};
