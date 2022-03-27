/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/uio.h>

#include "peripheral/efivars.h"

#define SYSFS_EFIVARS "/sys/firmware/efi/efivars"

typedef struct {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t  data4[8];
} efi_guid_t;

#define VENDOR_GUID \
	(efi_guid_t) { 0xd5f9d775, 0x1a09, 0x4e89, \
			{ 0x96, 0xcf, 0x1d, 0x19, 0x55, 0x4d, 0xa6, 0x67 } }

static void efivars_pathname(const char *name, char *pathname, size_t size)
{
	static efi_guid_t guid = VENDOR_GUID;

	snprintf(pathname, size - 1,
		"%s/%s-%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		SYSFS_EFIVARS, name, guid.data1, guid.data2, guid.data3,
		guid.data4[0], guid.data4[1], guid.data4[2], guid.data4[3],
		guid.data4[4], guid.data4[5], guid.data4[6], guid.data4[7]);
}

int efivars_read(const char *name, uint32_t *attributes,
					void *data, size_t size)
{
	char pathname[PATH_MAX];
	struct iovec iov[2];
	uint32_t attr;
	ssize_t len;
	int fd;

	efivars_pathname(name, pathname, PATH_MAX);

	fd = open(pathname, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return -EIO;

	iov[0].iov_base = &attr;
	iov[0].iov_len = sizeof(attr);
	iov[1].iov_base = data;
	iov[1].iov_len = size;

	len = readv(fd, iov, 2);

	close(fd);

	if (len < 0)
		return -EIO;

	if (attributes)
		*attributes = attr;

	return 0;
}

int efivars_write(const char *name, uint32_t attributes,
					const void *data, size_t size)
{
	char pathname[PATH_MAX];
	void *buf;
	ssize_t written;
	int fd;

	efivars_pathname(name, pathname, PATH_MAX);

	buf = malloc(size + sizeof(attributes));
	if (!buf)
		return -ENOMEM;

	fd = open(pathname, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		free(buf);
		return -EIO;
	}

	memcpy(buf, &attributes, sizeof(attributes));
	memcpy(buf + sizeof(attributes), data, size);

	written = write(fd, buf, size + sizeof(attributes));

	close(fd);
	free(buf);

	if (written < 0)
		return -EIO;

	return 0;
}
