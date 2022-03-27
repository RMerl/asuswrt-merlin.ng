/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/param.h>

#include "textfile.h"

static int create_dirs(const char *filename, const mode_t mode)
{
	struct stat st;
	char dir[PATH_MAX + 1], *prev, *next;
	int err;

	err = stat(filename, &st);
	if (!err && S_ISREG(st.st_mode))
		return 0;

	memset(dir, 0, PATH_MAX + 1);
	strcat(dir, "/");

	prev = strchr(filename, '/');

	while (prev) {
		next = strchr(prev + 1, '/');
		if (!next)
			break;

		if (next - prev == 1) {
			prev = next;
			continue;
		}

		strncat(dir, prev + 1, next - prev);
		mkdir(dir, mode);

		prev = next;
	}

	return 0;
}

int create_file(const char *filename, const mode_t mode)
{
	int fd;

	create_dirs(filename, S_IRUSR | S_IWUSR | S_IXUSR);

	fd = open(filename, O_RDWR | O_CREAT, mode);
	if (fd < 0)
		return fd;

	close(fd);

	return 0;
}

int create_name(char *buf, size_t size, const char *path, const char *address, const char *name)
{
	return snprintf(buf, size, "%s/%s/%s", path, address, name);
}

static inline char *find_key(char *map, size_t size, const char *key, size_t len, int icase)
{
	char *ptr = map;
	size_t ptrlen = size;

	while (ptrlen > len + 1) {
		int cmp = (icase) ? strncasecmp(ptr, key, len) : strncmp(ptr, key, len);
		if (cmp == 0) {
			if (ptr == map && *(ptr + len) == ' ')
				return ptr;

			if ((*(ptr - 1) == '\r' || *(ptr - 1) == '\n') &&
							*(ptr + len) == ' ')
				return ptr;
		}

		if (icase) {
			char *p1 = memchr(ptr + 1, tolower(*key), ptrlen - 1);
			char *p2 = memchr(ptr + 1, toupper(*key), ptrlen - 1);

			if (!p1)
				ptr = p2;
			else if (!p2)
				ptr = p1;
			else
				ptr = (p1 < p2) ? p1 : p2;
		} else
			ptr = memchr(ptr + 1, *key, ptrlen - 1);

		if (!ptr)
			return NULL;

		ptrlen = size - (ptr - map);
	}

	return NULL;
}

static inline int write_key_value(int fd, const char *key, const char *value)
{
	char *str;
	size_t size;
	int err = 0;

	size = strlen(key) + strlen(value) + 2;

	str = malloc(size + 1);
	if (!str)
		return ENOMEM;

	sprintf(str, "%s %s\n", key, value);

	if (write(fd, str, size) < 0)
		err = -errno;

	free(str);

	return err;
}

static char *strnpbrk(const char *s, ssize_t len, const char *accept)
{
	const char *p = s;
	const char *end;

	end = s + len - 1;

	while (p <= end && *p) {
		const char *a = accept;

		while (*a) {
			if (*p == *a)
				return (char *) p;
			a++;
		}

		p++;
	}

	return NULL;
}

static int write_key(const char *pathname, const char *key, const char *value, int icase)
{
	struct stat st;
	char *map, *off, *end, *str;
	off_t size;
	size_t base;
	int fd, len, err = 0;

	fd = open(pathname, O_RDWR);
	if (fd < 0)
		return -errno;

	if (flock(fd, LOCK_EX) < 0) {
		err = -errno;
		goto close;
	}

	if (fstat(fd, &st) < 0) {
		err = -errno;
		goto unlock;
	}

	size = st.st_size;

	if (!size) {
		if (value) {
			lseek(fd, size, SEEK_SET);
			err = write_key_value(fd, key, value);
		}
		goto unlock;
	}

	map = mmap(NULL, size, PROT_READ | PROT_WRITE,
					MAP_PRIVATE | MAP_LOCKED, fd, 0);
	if (!map || map == MAP_FAILED) {
		err = -errno;
		goto unlock;
	}

	len = strlen(key);
	off = find_key(map, size, key, len, icase);
	if (!off) {
		munmap(map, size);
		if (value) {
			lseek(fd, size, SEEK_SET);
			err = write_key_value(fd, key, value);
		}
		goto unlock;
	}

	base = off - map;

	end = strnpbrk(off, size, "\r\n");
	if (!end) {
		err = -EILSEQ;
		goto unmap;
	}

	if (value && ((ssize_t) strlen(value) == end - off - len - 1) &&
			!strncmp(off + len + 1, value, end - off - len - 1))
		goto unmap;

	len = strspn(end, "\r\n");
	end += len;

	len = size - (end - map);
	if (!len) {
		munmap(map, size);
		if (ftruncate(fd, base) < 0) {
			err = -errno;
			goto unlock;
		}
		lseek(fd, base, SEEK_SET);
		if (value)
			err = write_key_value(fd, key, value);

		goto unlock;
	}

	if (len < 0 || len > size) {
		err = -EILSEQ;
		goto unmap;
	}

	str = malloc(len);
	if (!str) {
		err = -errno;
		goto unmap;
	}

	memcpy(str, end, len);

	munmap(map, size);
	if (ftruncate(fd, base) < 0) {
		err = -errno;
		free(str);
		goto unlock;
	}
	lseek(fd, base, SEEK_SET);
	if (value)
		err = write_key_value(fd, key, value);

	if (write(fd, str, len) < 0)
		err = -errno;

	free(str);

	goto unlock;

unmap:
	munmap(map, size);

unlock:
	flock(fd, LOCK_UN);

close:
	fdatasync(fd);

	close(fd);
	errno = -err;

	return err;
}

static char *read_key(const char *pathname, const char *key, int icase)
{
	struct stat st;
	char *map, *off, *end, *str = NULL;
	off_t size; size_t len;
	int fd, err = 0;

	fd = open(pathname, O_RDONLY);
	if (fd < 0)
		return NULL;

	if (flock(fd, LOCK_SH) < 0) {
		err = -errno;
		goto close;
	}

	if (fstat(fd, &st) < 0) {
		err = -errno;
		goto unlock;
	}

	size = st.st_size;

	map = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (!map || map == MAP_FAILED) {
		err = -errno;
		goto unlock;
	}

	len = strlen(key);
	off = find_key(map, size, key, len, icase);
	if (!off) {
		err = -EILSEQ;
		goto unmap;
	}

	end = strnpbrk(off, size - (off - map), "\r\n");
	if (!end) {
		err = -EILSEQ;
		goto unmap;
	}

	str = malloc(end - off - len);
	if (!str) {
		err = -EILSEQ;
		goto unmap;
	}

	memset(str, 0, end - off - len);
	strncpy(str, off + len + 1, end - off - len - 1);

unmap:
	munmap(map, size);

unlock:
	flock(fd, LOCK_UN);

close:
	close(fd);
	errno = -err;

	return str;
}

int textfile_put(const char *pathname, const char *key, const char *value)
{
	return write_key(pathname, key, value, 0);
}

int textfile_del(const char *pathname, const char *key)
{
	return write_key(pathname, key, NULL, 0);
}

char *textfile_get(const char *pathname, const char *key)
{
	return read_key(pathname, key, 0);
}

int textfile_foreach(const char *pathname, textfile_cb func, void *data)
{
	struct stat st;
	char *map, *off, *end, *key, *value;
	off_t size; size_t len;
	int fd, err = 0;

	fd = open(pathname, O_RDONLY);
	if (fd < 0)
		return -errno;

	if (flock(fd, LOCK_SH) < 0) {
		err = -errno;
		goto close;
	}

	if (fstat(fd, &st) < 0) {
		err = -errno;
		goto unlock;
	}

	size = st.st_size;

	map = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (!map || map == MAP_FAILED) {
		err = -errno;
		goto unlock;
	}

	off = map;

	while (size - (off - map) > 0) {
		end = strnpbrk(off, size - (off - map), " ");
		if (!end) {
			err = -EILSEQ;
			break;
		}

		len = end - off;

		key = malloc(len + 1);
		if (!key) {
			err = -errno;
			break;
		}

		memset(key, 0, len + 1);
		memcpy(key, off, len);

		off = end + 1;

		if (size - (off - map) < 0) {
			err = -EILSEQ;
			free(key);
			break;
		}

		end = strnpbrk(off, size - (off - map), "\r\n");
		if (!end) {
			err = -EILSEQ;
			free(key);
			break;
		}

		len = end - off;

		value = malloc(len + 1);
		if (!value) {
			err = -errno;
			free(key);
			break;
		}

		memset(value, 0, len + 1);
		memcpy(value, off, len);

		func(key, value, data);

		free(key);
		free(value);

		off = end + 1;
	}

	munmap(map, size);

unlock:
	flock(fd, LOCK_UN);

close:
	close(fd);
	errno = -err;

	return 0;
}
