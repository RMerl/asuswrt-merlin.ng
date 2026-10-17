// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_loadimage
 *
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This test checks the LoadImage and StartImage boot service.
 *
 * The efi_selftest_miniapp_exit.efi application is loaded via a file device
 * path and started.
 */

#include <efi_selftest.h>
/* Include containing the efi_selftest_miniapp_exit.efi application */
#include "efi_miniapp_file_image_exit.h"

/* Block size of compressed disk image */
#define COMPRESSED_DISK_IMAGE_BLOCK_SIZE 8

/* Binary logarithm of the block size */
#define LB_BLOCK_SIZE 9

#define FILE_NAME u"app.efi"
#define VOLUME_NAME u"EfiDisk"

static struct efi_boot_services *boottime;
static efi_handle_t handle_image;
static efi_handle_t handle_volume;

static const efi_guid_t guid_device_path = EFI_DEVICE_PATH_PROTOCOL_GUID;
static const efi_guid_t guid_simple_file_system_protocol =
		EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
static const efi_guid_t guid_file_info = EFI_FILE_INFO_GUID;
static const efi_guid_t guid_file_system_info = EFI_FILE_SYSTEM_INFO_GUID;

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

/* File info including file name */
struct file_info {
	struct efi_file_info info;
	u16 file_name[sizeof(FILE_NAME)];
};

/* File system info including volume name */
struct file_system_info {
	struct efi_file_system_info info;
	u16 file_name[sizeof(VOLUME_NAME)];
};

/* Compressed file image */
static struct compressed_file_image img = EFI_ST_DISK_IMG;

/* Pointer to decompressed file image */
static u8 *image;

/* File info */
static struct file_info priv_file_info = {
	{
		.size = sizeof(struct file_info),
		.attribute = EFI_FILE_READ_ONLY,
	},
	FILE_NAME,
};

/* Pointer to file info */
struct efi_file_info *file_info = &priv_file_info.info;

/* Volume device path */
static struct {
	struct efi_device_path_vendor vendor;
	struct efi_device_path end;
} __packed dp_volume = {
	.vendor = {
		.dp = {
			.type =	DEVICE_PATH_TYPE_HARDWARE_DEVICE,
			.sub_type = DEVICE_PATH_SUB_TYPE_VENDOR,
			.length = sizeof(struct efi_device_path_vendor),
		},
		.guid = EFI_GUID(0x4f9a0ebf, 0xa179, 0x88a6, 0x25, 0x68,
				 0x10, 0x72, 0xb1, 0x93, 0x51, 0x71),
	},
	.end = {
		.type = DEVICE_PATH_TYPE_END,
		.sub_type = DEVICE_PATH_SUB_TYPE_END,
		.length = sizeof(struct efi_device_path),
	}
};

/* File device path */
static struct {
	struct efi_device_path_vendor vendor;
	struct efi_device_path path;
	u16 file[sizeof(FILE_NAME)];
	struct efi_device_path end;
} __packed dp_file = {
	.vendor = {
		.dp = {
			.type =	DEVICE_PATH_TYPE_HARDWARE_DEVICE,
			.sub_type = DEVICE_PATH_SUB_TYPE_VENDOR,
			.length = sizeof(struct efi_device_path_vendor),
		},
		.guid = EFI_GUID(0x4f9a0ebf, 0xa179, 0x88a6, 0x25, 0x68,
				 0x10, 0x72, 0xb1, 0x93, 0x51, 0x71),
	},
	.path = {
		.type = DEVICE_PATH_TYPE_MEDIA_DEVICE,
		.sub_type = DEVICE_PATH_SUB_TYPE_FILE_PATH,
		.length = sizeof(struct efi_device_path) + sizeof(dp_file.file),
	},
	.file = FILE_NAME,
	.end = {
		.type = DEVICE_PATH_TYPE_END,
		.sub_type = DEVICE_PATH_SUB_TYPE_END,
		.length = sizeof(struct efi_device_path),
	}
};

/* File system info */
static struct file_system_info priv_file_system_info = {
	{
		.size = sizeof(struct file_system_info),
		.read_only = true,
		.volume_size = 0x100000,
		.free_space = 0x0,
		.block_size = 0x200,
	},
	VOLUME_NAME
};

/* Pointer to file system info */
static struct efi_file_system_info *file_system_info =
	&priv_file_system_info.info;

/* Forward definitions of file and file system functions */
static efi_status_t EFIAPI efi_st_open_volume
	(struct efi_simple_file_system_protocol *this,
	 struct efi_file_handle **root);

static efi_status_t EFIAPI efi_st_open
	(struct efi_file_handle *this,
	 struct efi_file_handle **new_handle,
	 u16 *file_name, u64 open_mode, u64 attributes);

static efi_status_t EFIAPI efi_st_close(struct efi_file_handle *this);

static efi_status_t EFIAPI efi_st_delete(struct efi_file_handle *this);

static efi_status_t EFIAPI efi_st_read
	(struct efi_file_handle *this, efi_uintn_t *buffer_size, void *buffer);

static efi_status_t EFIAPI efi_st_write
	(struct efi_file_handle *this, efi_uintn_t *buffer_size, void *buffer);

static efi_status_t EFIAPI efi_st_getpos(struct efi_file_handle *this,
					 u64 *pos);

static efi_status_t EFIAPI efi_st_setpos(struct efi_file_handle *this, u64 pos);

static efi_status_t EFIAPI efi_st_getinfo
	(struct efi_file_handle *this, const efi_guid_t *info_type,
	 efi_uintn_t *buffer_size, void *buffer);

static efi_status_t EFIAPI efi_st_setinfo
	(struct efi_file_handle *this, const efi_guid_t *info_type,
	 efi_uintn_t buffer_size, void *buffer);

static efi_status_t EFIAPI efi_st_flush(struct efi_file_handle *this);

/* Internal information about status of file system */
static struct {
	/* Difference of volume open count minus volume close count */
	int volume_open_count;
	/* Difference of file open count minus file close count */
	int file_open_count;
	/* File size */
	u64 file_size;
	/* Current position in file */
	u64 file_pos;
} priv;

/* EFI_FILE_PROTOCOL for file */
static struct efi_file_handle file = {
	.rev = 0x00010000,
	.open = efi_st_open,
	.close = efi_st_close,
	.delete = efi_st_delete,
	.read = efi_st_read,
	.write = efi_st_write,
	.getpos = efi_st_getpos,
	.setpos = efi_st_setpos,
	.getinfo = efi_st_getinfo,
	.setinfo = efi_st_setinfo,
	.flush = efi_st_flush,
};

/* EFI_FILE_PROTOCOL for root directory */
static struct efi_file_handle volume = {
	.rev = 0x00010000,
	.open = efi_st_open,
	.close = efi_st_close,
	.delete = efi_st_delete,
	.read = efi_st_read,
	.write = efi_st_write,
	.getpos = efi_st_getpos,
	.setpos = efi_st_setpos,
	.getinfo = efi_st_getinfo,
	.setinfo = efi_st_setinfo,
	.flush = efi_st_flush,
};

/* EFI_SIMPLE_FILE_SYSTEM_PROTOCOL of the block device */
struct efi_simple_file_system_protocol file_system = {
	.rev = 0x00010000,
	.open_volume = efi_st_open_volume,
};

static efi_status_t EFIAPI efi_st_open_volume
	(struct efi_simple_file_system_protocol *this,
	 struct efi_file_handle **root)
{
	if (this != &file_system || !root)
		return EFI_INVALID_PARAMETER;

	*root = &volume;
	priv.volume_open_count++;

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_st_open
	(struct efi_file_handle *this,
	 struct efi_file_handle **new_handle,
	 u16 *file_name, u64 open_mode, u64 attributes)
{
	if (this != &volume)
		return EFI_INVALID_PARAMETER;

	*new_handle = &file;
	priv.file_pos = 0;
	priv.file_open_count++;

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_st_close(struct efi_file_handle *this)
{
	if (this == &file)
		priv.file_open_count--;
	else if (this == &volume)
		priv.volume_open_count--;
	else
		return EFI_INVALID_PARAMETER;

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_st_delete(struct efi_file_handle *this)
{
	if (this != &file)
		return EFI_INVALID_PARAMETER;

	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_st_read
	(struct efi_file_handle *this, efi_uintn_t *buffer_size, void *buffer)
{
	if (this != &file)
		return EFI_INVALID_PARAMETER;

	if (priv.file_pos >= img.length)
		*buffer_size = 0;
	else if (priv.file_pos + *buffer_size > img.length)
		*buffer_size = img.length - priv.file_pos;

	boottime->copy_mem(buffer, &image[priv.file_pos], *buffer_size);
	priv.file_pos += *buffer_size;

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_st_write
	(struct efi_file_handle *this, efi_uintn_t *buffer_size, void *buffer)
{
	if (this != &file)
		return EFI_INVALID_PARAMETER;

	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_st_getpos(struct efi_file_handle *this, u64 *pos)
{
	if (this != &file)
		return EFI_INVALID_PARAMETER;

	*pos = priv.file_pos;

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_st_setpos(struct efi_file_handle *this, u64 pos)
{
	if (this != &file)
		return EFI_INVALID_PARAMETER;

	priv.file_pos = pos;

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_st_getinfo
	(struct efi_file_handle *this, const efi_guid_t *info_type,
	 efi_uintn_t *buffer_size, void *buffer)
{
	if (this == &file) {
		if (memcmp(info_type, &guid_file_info, sizeof(efi_guid_t)))
			return EFI_INVALID_PARAMETER;
		if (*buffer_size >= sizeof(struct file_info)) {
			boottime->copy_mem(buffer, file_info,
					   sizeof(struct file_info));
		} else {
			*buffer_size = sizeof(struct file_info);
			return EFI_BUFFER_TOO_SMALL;
		}
	} else if (this == &volume) {
		if (memcmp(info_type, &guid_file_system_info,
			   sizeof(efi_guid_t)))
			return EFI_INVALID_PARAMETER;
		if (*buffer_size >= sizeof(struct file_system_info)) {
			boottime->copy_mem(buffer, file_system_info,
					   sizeof(struct file_system_info));
		} else {
			*buffer_size = sizeof(struct file_system_info);
			return EFI_BUFFER_TOO_SMALL;
		}
	} else {
		return EFI_INVALID_PARAMETER;
	}
	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_st_setinfo
	(struct efi_file_handle *this, const efi_guid_t *info_type,
	 efi_uintn_t buffer_size, void *buffer)
{
	if (this != &file)
		return EFI_INVALID_PARAMETER;

	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_st_flush(struct efi_file_handle *this)
{
	if (this != &file)
		return EFI_INVALID_PARAMETER;

	return EFI_UNSUPPORTED;
}

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
	priv.file_size = img.length;
	file_info->file_size = img.length;
	return ret;
}

/*
 * Setup unit test.
 *
 * Decompress application image and provide a handle for the in memory block
 * device.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	handle_image = handle;
	boottime = systable->boottime;

	/* Load the application image into memory */
	decompress(&image);

	ret = boottime->install_protocol_interface
		(&handle_volume, &guid_device_path, EFI_NATIVE_INTERFACE,
		 &dp_volume);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to install device path\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->install_protocol_interface
		(&handle_volume, &guid_simple_file_system_protocol,
		 EFI_NATIVE_INTERFACE, &file_system);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to install simple file system protocol\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * Uninstall protocols and free memory.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	efi_status_t ret = EFI_ST_SUCCESS;

	if (handle_volume) {
		ret = boottime->uninstall_protocol_interface
			(handle_volume, &guid_simple_file_system_protocol,
			 &file_system);
		if (ret != EFI_SUCCESS) {
			efi_st_error
				("Failed to uninstall simple file system protocol\n");
			return EFI_ST_FAILURE;
		}
		ret = boottime->uninstall_protocol_interface
			(handle_volume, &guid_device_path, &dp_volume);
		if (ret != EFI_SUCCESS) {
			efi_st_error
				("Failed to uninstall device path protocol\n");
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
 * Load and start the application image.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	efi_handle_t handle;

	ret = boottime->load_image(false, handle_image, &dp_file.vendor.dp,
				   NULL, 0, &handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to load image\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->start_image(handle, NULL, NULL);
	if (ret != EFI_UNSUPPORTED) {
		efi_st_error("Wrong return value from application\n");
		return EFI_ST_FAILURE;
	}

	if (priv.file_open_count) {
		efi_st_error("File open count = %d, expected 0\n",
			     priv.file_open_count);
		return EFI_ST_FAILURE;
	}
	if (priv.volume_open_count) {
		efi_st_error("Volume open count = %d, expected 0\n",
			     priv.volume_open_count);
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(loadimage) = {
	.name = "load image from file",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
