// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022, Sean Anderson <sean.anderson@seco.com>
 * Copyright (c) 2012, Google Inc.
 */

#include <common.h>
#include <fs.h>
#include <malloc.h>
#include <os.h>
#include <semihosting.h>
#include <semihostingfs.h>

int smh_fs_set_blk_dev(struct blk_desc *rbdd, struct disk_partition *info)
{
	/*
	 * Only accept a NULL struct blk_desc for the semihosting, which is when
	 * hostfs interface is used
	 */
	return !!rbdd;
}

static int smh_fs_read_at(const char *filename, loff_t pos, void *buffer,
			  loff_t maxsize, loff_t *actread)
{
	long fd, size, ret;

	fd = smh_open(filename, MODE_READ | MODE_BINARY);
	if (fd < 0)
		return fd;
	ret = smh_seek(fd, pos);
	if (ret < 0) {
		smh_close(fd);
		return ret;
	}
	if (!maxsize) {
		size = smh_flen(fd);
		if (ret < 0) {
			smh_close(fd);
			return size;
		}

		maxsize = size;
	}

	size = smh_read(fd, buffer, maxsize);
	smh_close(fd);
	if (size < 0)
		return size;

	*actread = size;
	return 0;
}

static int smh_fs_write_at(const char *filename, loff_t pos, void *buffer,
			   loff_t towrite, loff_t *actwrite)
{
	long fd, size, ret;

	/* Try to open existing file */
	fd = smh_open(filename, MODE_READ | MODE_BINARY | MODE_PLUS);
	if (fd < 0)
		/* Create new file */
		fd = smh_open(filename, MODE_WRITE | MODE_BINARY);
	if (fd < 0)
		return fd;
	ret = smh_seek(fd, pos);
	if (ret < 0) {
		smh_close(fd);
		return ret;
	}

	ret = smh_write(fd, buffer, towrite, &size);
	smh_close(fd);
	*actwrite = size;
	return ret;
}

int smh_fs_size(const char *filename, loff_t *result)
{
	long fd, size;

	fd = smh_open(filename, MODE_READ | MODE_BINARY);
	if (fd < 0)
		return fd;

	size = smh_flen(fd);
	smh_close(fd);

	if (size < 0)
		return size;

	*result = size;
	return 0;
}

int smh_fs_read(const char *filename, void *buf, loff_t offset, loff_t len,
		loff_t *actread)
{
	int ret;

	ret = smh_fs_read_at(filename, offset, buf, len, actread);
	if (ret)
		printf("** Unable to read file %s **\n", filename);

	return ret;
}

int smh_fs_write(const char *filename, void *buf, loff_t offset,
		 loff_t len, loff_t *actwrite)
{
	int ret;

	ret = smh_fs_write_at(filename, offset, buf, len, actwrite);
	if (ret)
		printf("** Unable to write file %s **\n", filename);

	return ret;
}
