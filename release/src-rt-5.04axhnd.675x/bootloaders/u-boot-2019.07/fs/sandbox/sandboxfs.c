// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012, Google Inc.
 */

#include <common.h>
#include <fs.h>
#include <os.h>

int sandbox_fs_set_blk_dev(struct blk_desc *rbdd, disk_partition_t *info)
{
	/*
	 * Only accept a NULL struct blk_desc for the sandbox, which is when
	 * hostfs interface is used
	 */
	return rbdd != NULL;
}

int sandbox_fs_read_at(const char *filename, loff_t pos, void *buffer,
		       loff_t maxsize, loff_t *actread)
{
	loff_t size;
	int fd, ret;

	fd = os_open(filename, OS_O_RDONLY);
	if (fd < 0)
		return fd;
	ret = os_lseek(fd, pos, OS_SEEK_SET);
	if (ret == -1) {
		os_close(fd);
		return ret;
	}
	if (!maxsize) {
		ret = os_get_filesize(filename, &size);
		if (ret) {
			os_close(fd);
			return ret;
		}

		maxsize = size;
	}

	size = os_read(fd, buffer, maxsize);
	os_close(fd);

	if (size < 0) {
		ret = -1;
	} else {
		ret = 0;
		*actread = size;
	}

	return ret;
}

int sandbox_fs_write_at(const char *filename, loff_t pos, void *buffer,
			loff_t towrite, loff_t *actwrite)
{
	ssize_t size;
	int fd, ret;

	fd = os_open(filename, OS_O_RDWR | OS_O_CREAT);
	if (fd < 0)
		return fd;
	ret = os_lseek(fd, pos, OS_SEEK_SET);
	if (ret == -1) {
		os_close(fd);
		return ret;
	}
	size = os_write(fd, buffer, towrite);
	os_close(fd);

	if (size == -1) {
		ret = -1;
	} else {
		ret = 0;
		*actwrite = size;
	}

	return ret;
}

int sandbox_fs_ls(const char *dirname)
{
	struct os_dirent_node *head, *node;
	int ret;

	ret = os_dirent_ls(dirname, &head);
	if (ret)
		goto out;

	for (node = head; node; node = node->next) {
		printf("%s %10lu %s\n", os_dirent_get_typename(node->type),
		       node->size, node->name);
	}
out:
	os_dirent_free(head);

	return ret;
}

int sandbox_fs_exists(const char *filename)
{
	loff_t size;
	int ret;

	ret = os_get_filesize(filename, &size);
	return ret == 0;
}

int sandbox_fs_size(const char *filename, loff_t *size)
{
	return os_get_filesize(filename, size);
}

void sandbox_fs_close(void)
{
}

int fs_read_sandbox(const char *filename, void *buf, loff_t offset, loff_t len,
		    loff_t *actread)
{
	int ret;

	ret = sandbox_fs_read_at(filename, offset, buf, len, actread);
	if (ret)
		printf("** Unable to read file %s **\n", filename);

	return ret;
}

int fs_write_sandbox(const char *filename, void *buf, loff_t offset,
		     loff_t len, loff_t *actwrite)
{
	int ret;

	ret = sandbox_fs_write_at(filename, offset, buf, len, actwrite);
	if (ret)
		printf("** Unable to write file %s **\n", filename);

	return ret;
}
