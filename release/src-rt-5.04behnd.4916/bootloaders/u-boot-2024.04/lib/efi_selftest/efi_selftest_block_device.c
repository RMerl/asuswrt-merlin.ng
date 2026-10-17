// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_block
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This test checks the driver for block IO devices.
 * A disk image is created in memory.
 * A handle is created for the new block IO device.
 * The block I/O protocol is installed on the handle.
 * ConnectController is used to setup partitions and to install the simple
 * file protocol.
 * A known file is read from the file system and verified.
 * The same block is read via the EFI_BLOCK_IO_PROTOCOL and compared to the file
 * contents.
 */

#include <efi_selftest.h>
#include "efi_selftest_disk_image.h"
#include <asm/cache.h>

/* Block size of compressed disk image */
#define COMPRESSED_DISK_IMAGE_BLOCK_SIZE 8

/* Binary logarithm of the block size */
#define LB_BLOCK_SIZE 9

static struct efi_boot_services *boottime;

static const efi_guid_t block_io_protocol_guid = EFI_BLOCK_IO_PROTOCOL_GUID;
static const efi_guid_t guid_device_path = EFI_DEVICE_PATH_PROTOCOL_GUID;
static const efi_guid_t guid_simple_file_system_protocol =
					EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
static const efi_guid_t guid_file_system_info = EFI_FILE_SYSTEM_INFO_GUID;
static efi_guid_t guid_vendor =
	EFI_GUID(0xdbca4c98, 0x6cb0, 0x694d,
		 0x08, 0x72, 0x81, 0x9c, 0x65, 0x0c, 0xb7, 0xb8);

static struct efi_device_path *dp;

/* One 8 byte block of the compressed disk image */
struct line {
	size_t addr;
	char *line;
};

/* Compressed disk image */
struct compressed_disk_image {
	size_t length;
	struct line lines[];
};

static const struct compressed_disk_image img = EFI_ST_DISK_IMG;

/* Decompressed disk image */
static u8 *image;

/*
 * Reset service of the block IO protocol.
 *
 * @this	block IO protocol
 * Return:	status code
 */
static efi_status_t EFIAPI reset(
			struct efi_block_io *this,
			char extended_verification)
{
	return EFI_SUCCESS;
}

/*
 * Read service of the block IO protocol.
 *
 * @this	block IO protocol
 * @media_id	media id
 * @lba		start of the read in logical blocks
 * @buffer_size	number of bytes to read
 * @buffer	target buffer
 * Return:	status code
 */
static efi_status_t EFIAPI read_blocks(
			struct efi_block_io *this, u32 media_id, u64 lba,
			efi_uintn_t buffer_size, void *buffer)
{
	u8 *start;

	if ((lba << LB_BLOCK_SIZE) + buffer_size > img.length)
		return EFI_INVALID_PARAMETER;
	start = image + (lba << LB_BLOCK_SIZE);

	boottime->copy_mem(buffer, start, buffer_size);

	return EFI_SUCCESS;
}

/*
 * Write service of the block IO protocol.
 *
 * @this	block IO protocol
 * @media_id	media id
 * @lba		start of the write in logical blocks
 * @buffer_size	number of bytes to read
 * @buffer	source buffer
 * Return:	status code
 */
static efi_status_t EFIAPI write_blocks(
			struct efi_block_io *this, u32 media_id, u64 lba,
			efi_uintn_t buffer_size, void *buffer)
{
	u8 *start;

	if ((lba << LB_BLOCK_SIZE) + buffer_size > img.length)
		return EFI_INVALID_PARAMETER;
	start = image + (lba << LB_BLOCK_SIZE);

	boottime->copy_mem(start, buffer, buffer_size);

	return EFI_SUCCESS;
}

/*
 * Flush service of the block IO protocol.
 *
 * @this	block IO protocol
 * Return:	status code
 */
static efi_status_t EFIAPI flush_blocks(struct efi_block_io *this)
{
	return EFI_SUCCESS;
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
	return ret;
}

static struct efi_block_io_media media;

static struct efi_block_io block_io = {
	.media = &media,
	.reset = reset,
	.read_blocks = read_blocks,
	.write_blocks = write_blocks,
	.flush_blocks = flush_blocks,
};

/* Handle for the block IO device */
static efi_handle_t disk_handle;

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
	efi_status_t ret;
	struct efi_device_path_vendor vendor_node;
	struct efi_device_path end_node;

	boottime = systable->boottime;

	decompress(&image);

	block_io.media->block_size = 1 << LB_BLOCK_SIZE;
	block_io.media->last_block = (img.length >> LB_BLOCK_SIZE) - 1;

	ret = boottime->install_protocol_interface(
				&disk_handle, &block_io_protocol_guid,
				EFI_NATIVE_INTERFACE, &block_io);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to install block I/O protocol\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->allocate_pool(EFI_LOADER_DATA,
				      sizeof(struct efi_device_path_vendor) +
				      sizeof(struct efi_device_path),
				      (void **)&dp);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Out of memory\n");
		return EFI_ST_FAILURE;
	}
	vendor_node.dp.type = DEVICE_PATH_TYPE_HARDWARE_DEVICE;
	vendor_node.dp.sub_type = DEVICE_PATH_SUB_TYPE_VENDOR;
	vendor_node.dp.length = sizeof(struct efi_device_path_vendor);

	boottime->copy_mem(&vendor_node.guid, &guid_vendor,
			   sizeof(efi_guid_t));
	boottime->copy_mem(dp, &vendor_node,
			   sizeof(struct efi_device_path_vendor));
	end_node.type = DEVICE_PATH_TYPE_END;
	end_node.sub_type = DEVICE_PATH_SUB_TYPE_END;
	end_node.length = sizeof(struct efi_device_path);

	boottime->copy_mem((char *)dp + sizeof(struct efi_device_path_vendor),
			   &end_node, sizeof(struct efi_device_path));
	ret = boottime->install_protocol_interface(&disk_handle,
						   &guid_device_path,
						   EFI_NATIVE_INTERFACE,
						   dp);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	efi_status_t r = EFI_ST_SUCCESS;

	if (disk_handle) {
		r = boottime->uninstall_protocol_interface(disk_handle,
							   &guid_device_path,
							   dp);
		if (r != EFI_SUCCESS) {
			efi_st_error("Uninstall device path failed\n");
			return EFI_ST_FAILURE;
		}
		r = boottime->uninstall_protocol_interface(
				disk_handle, &block_io_protocol_guid,
				&block_io);
		if (r != EFI_SUCCESS) {
			efi_st_error(
				"Failed to uninstall block I/O protocol\n");
			return EFI_ST_FAILURE;
		}
	}

	if (image) {
		r = boottime->free_pool(image);
		if (r != EFI_SUCCESS) {
			efi_st_error("Failed to free image\n");
			return EFI_ST_FAILURE;
		}
	}
	return r;
}

/*
 * Get length of device path without end tag.
 *
 * @dp		device path
 * Return:	length of device path in bytes
 */
static efi_uintn_t dp_size(struct efi_device_path *dp)
{
	struct efi_device_path *pos = dp;

	while (pos->type != DEVICE_PATH_TYPE_END)
		pos = (struct efi_device_path *)((char *)pos + pos->length);
	return (char *)pos - (char *)dp;
}

/*
 * Execute unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	efi_uintn_t no_handles, i, len;
	efi_handle_t *handles;
	efi_handle_t handle_partition = NULL;
	struct efi_device_path *dp_partition;
	struct efi_block_io *block_io_protocol;
	struct efi_simple_file_system_protocol *file_system;
	struct efi_file_handle *root, *file;
	struct {
		struct efi_file_system_info info;
		u16 label[12];
	} system_info;
	efi_uintn_t buf_size;
	char buf[16] __aligned(ARCH_DMA_MINALIGN);
	u32 part1_size;
	u64 pos;
	char block_io_aligned[1 << LB_BLOCK_SIZE] __aligned(1 << LB_BLOCK_SIZE);

	/* Connect controller to virtual disk */
	ret = boottime->connect_controller(disk_handle, NULL, NULL, 1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to connect controller\n");
		return EFI_ST_FAILURE;
	}

	/* Get the handle for the partition */
	ret = boottime->locate_handle_buffer(
				BY_PROTOCOL, &guid_device_path, NULL,
				&no_handles, &handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to locate handles\n");
		return EFI_ST_FAILURE;
	}
	len = dp_size(dp);
	for (i = 0; i < no_handles; ++i) {
		ret = boottime->open_protocol(handles[i], &guid_device_path,
					      (void **)&dp_partition,
					      NULL, NULL,
					      EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to open device path protocol\n");
			return EFI_ST_FAILURE;
		}
		if (len >= dp_size(dp_partition))
			continue;
		if (memcmp(dp, dp_partition, len))
			continue;
		handle_partition = handles[i];
		break;
	}
	ret = boottime->free_pool(handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to free pool memory\n");
		return EFI_ST_FAILURE;
	}
	if (!handle_partition) {
		efi_st_error("Partition handle not found\n");
		return EFI_ST_FAILURE;
	}

	/* Open the block_io_protocol */
	ret = boottime->open_protocol(handle_partition,
				      &block_io_protocol_guid,
				      (void **)&block_io_protocol, NULL, NULL,
				      EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to open block IO protocol\n");
		return EFI_ST_FAILURE;
	}
	/* Get size of first MBR partition */
	memcpy(&part1_size, image + 0x1ca, sizeof(u32));
	if (block_io_protocol->media->last_block != part1_size - 1) {
		efi_st_error("Last LBA of partition %x, expected %x\n",
			     (unsigned int)block_io_protocol->media->last_block,
			     part1_size - 1);
		return EFI_ST_FAILURE;
	}
	/* Open the simple file system protocol */
	ret = boottime->open_protocol(handle_partition,
				      &guid_simple_file_system_protocol,
				      (void **)&file_system, NULL, NULL,
				      EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to open simple file system protocol\n");
		return EFI_ST_FAILURE;
	}

	/* Open volume */
	ret = file_system->open_volume(file_system, &root);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to open volume\n");
		return EFI_ST_FAILURE;
	}
	buf_size = sizeof(system_info);
	ret = root->getinfo(root, &guid_file_system_info, &buf_size,
			    &system_info);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to get file system info\n");
		return EFI_ST_FAILURE;
	}
	if (system_info.info.block_size != 512) {
		efi_st_error("Wrong block size %u, expected 512\n",
			     system_info.info.block_size);
		return EFI_ST_FAILURE;
	}
	if (efi_st_strcmp_16_8(system_info.info.volume_label, "U-BOOT TEST")) {
		efi_st_todo(
			"Wrong volume label '%ps', expected 'U-BOOT TEST'\n",
			system_info.info.volume_label);
	}

	/* Read file */
	ret = root->open(root, &file, u"hello.txt", EFI_FILE_MODE_READ,
			 0);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to open file\n");
		return EFI_ST_FAILURE;
	}
	ret = file->setpos(file, 1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("SetPosition failed\n");
		return EFI_ST_FAILURE;
	}
	buf_size = sizeof(buf) - 1;
	ret = file->read(file, &buf_size, buf);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to read file\n");
		return EFI_ST_FAILURE;
	}
	if (buf_size != 12) {
		efi_st_error("Wrong number of bytes read: %u\n",
			     (unsigned int)buf_size);
		return EFI_ST_FAILURE;
	}
	if (memcmp(buf, "ello world!", 11)) {
		efi_st_error("Unexpected file content\n");
		return EFI_ST_FAILURE;
	}
	ret = file->getpos(file, &pos);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetPosition failed\n");
		return EFI_ST_FAILURE;
	}
	if (pos != 13) {
		efi_st_error("GetPosition returned %u, expected 13\n",
			     (unsigned int)pos);
		return EFI_ST_FAILURE;
	}
	ret = file->close(file);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to close file\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Test that read_blocks() can read same file data.
	 *
	 * In the test data, the partition starts at block 1 and the file
	 * hello.txt with the content 'Hello world!' is located at 0x5000
	 * of the disk. Here we read block 0x27 (offset 0x4e00 of the
	 * partition) and expect the string 'Hello world!' to be at the
	 * start of block.
	 */
	ret = block_io_protocol->read_blocks(block_io_protocol,
				      block_io_protocol->media->media_id,
				      (0x5000 >> LB_BLOCK_SIZE) - 1,
				      block_io_protocol->media->block_size,
				      block_io_aligned);
	if (ret != EFI_SUCCESS) {
		efi_st_error("ReadBlocks failed\n");
		return EFI_ST_FAILURE;
	}

	if (memcmp(block_io_aligned + 1, buf, 11)) {
		efi_st_error("Unexpected block content\n");
		return EFI_ST_FAILURE;
	}

#ifdef CONFIG_FAT_WRITE
	/* Write file */
	ret = root->open(root, &file, u"u-boot.txt", EFI_FILE_MODE_READ |
			 EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to open file\n");
		return EFI_ST_FAILURE;
	}
	buf_size = 7;
	boottime->set_mem(buf, sizeof(buf), 0);
	boottime->copy_mem(buf, "U-Boot", buf_size);
	ret = file->write(file, &buf_size, buf);
	if (ret != EFI_SUCCESS || buf_size != 7) {
		efi_st_error("Failed to write file\n");
		return EFI_ST_FAILURE;
	}
	ret = file->getpos(file, &pos);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetPosition failed\n");
		return EFI_ST_FAILURE;
	}
	if (pos != 7) {
		efi_st_error("GetPosition returned %u, expected 7\n",
			     (unsigned int)pos);
		return EFI_ST_FAILURE;
	}
	ret = file->close(file);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to close file\n");
		return EFI_ST_FAILURE;
	}

	/* Verify file */
	boottime->set_mem(buf, sizeof(buf), 0);
	ret = root->open(root, &file, u"u-boot.txt", EFI_FILE_MODE_READ,
			 0);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to open file\n");
		return EFI_ST_FAILURE;
	}
	buf_size = sizeof(buf) - 1;
	ret = file->read(file, &buf_size, buf);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to read file\n");
		return EFI_ST_FAILURE;
	}
	if (buf_size != 7) {
		efi_st_error("Wrong number of bytes read: %u\n",
			     (unsigned int)buf_size);
		return EFI_ST_FAILURE;
	}
	if (memcmp(buf, "U-Boot", 7)) {
		efi_st_error("Unexpected file content %s\n", buf);
		return EFI_ST_FAILURE;
	}
	ret = file->close(file);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to close file\n");
		return EFI_ST_FAILURE;
	}
#else
	efi_st_todo("CONFIG_FAT_WRITE is not set\n");
#endif /* CONFIG_FAT_WRITE */

	/* Close volume */
	ret = root->close(root);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to close volume\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(blkdev) = {
	.name = "block device",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
