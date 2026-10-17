// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 * Copyright 2014 Broadcom Corporation
 */

#include <log.h>
#include <semihosting.h>
#include <linux/errno.h>
#include <linux/string.h>

#define SYSOPEN		0x01
#define SYSCLOSE	0x02
#define SYSWRITEC	0x03
#define SYSWRITE0	0x04
#define SYSWRITE	0x05
#define SYSREAD		0x06
#define SYSREADC	0x07
#define SYSISERROR	0x08
#define SYSSEEK		0x0A
#define SYSFLEN		0x0C
#define SYSERRNO	0x13

#if CONFIG_IS_ENABLED(SEMIHOSTING_FALLBACK)
static bool _semihosting_enabled = true;
static bool try_semihosting = true;

bool semihosting_enabled(void)
{
	if (try_semihosting) {
		smh_trap(SYSERRNO, NULL);
		try_semihosting = false;
	}

	return _semihosting_enabled;
}

void disable_semihosting(void)
{
	_semihosting_enabled = false;
}
#endif

/**
 * smh_errno() - Read the host's errno
 *
 * This gets the value of the host's errno and negates it. The host's errno may
 * or may not be set, so only call this function if a previous semihosting call
 * has failed.
 *
 * Return: a negative error value
 */
static int smh_errno(void)
{
	long ret = smh_trap(SYSERRNO, NULL);

	if (ret > 0 && ret < INT_MAX)
		return -ret;
	return -EIO;
}

long smh_open(const char *fname, enum smh_open_mode mode)
{
	long fd;
	struct smh_open_s {
		const char *fname;
		unsigned long mode;
		size_t len;
	} open;

	debug("%s: file \'%s\', mode \'%u\'\n", __func__, fname, mode);

	open.fname = fname;
	open.len = strlen(fname);
	open.mode = mode;

	/* Open the file on the host */
	fd = smh_trap(SYSOPEN, &open);
	if (fd == -1)
		return smh_errno();
	return fd;
}

/**
 * struct smg_rdwr_s - Arguments for read and write
 * @fd: A file descriptor returned from smh_open()
 * @memp: Pointer to a buffer of memory of at least @len bytes
 * @len: The number of bytes to read or write
 */
struct smh_rdwr_s {
	long fd;
	void *memp;
	size_t len;
};

long smh_read(long fd, void *memp, size_t len)
{
	long ret;
	struct smh_rdwr_s read;

	debug("%s: fd %ld, memp %p, len %zu\n", __func__, fd, memp, len);

	read.fd = fd;
	read.memp = memp;
	read.len = len;

	ret = smh_trap(SYSREAD, &read);
	if (ret < 0)
		return smh_errno();
	return len - ret;
}

long smh_write(long fd, const void *memp, size_t len, ulong *written)
{
	long ret;
	struct smh_rdwr_s write;

	debug("%s: fd %ld, memp %p, len %zu\n", __func__, fd, memp, len);

	write.fd = fd;
	write.memp = (void *)memp;
	write.len = len;

	ret = smh_trap(SYSWRITE, &write);
	*written = len - ret;
	if (ret)
		return smh_errno();
	return 0;
}

long smh_close(long fd)
{
	long ret;

	debug("%s: fd %ld\n", __func__, fd);

	ret = smh_trap(SYSCLOSE, &fd);
	if (ret == -1)
		return smh_errno();
	return 0;
}

long smh_flen(long fd)
{
	long ret;

	debug("%s: fd %ld\n", __func__, fd);

	ret = smh_trap(SYSFLEN, &fd);
	if (ret == -1)
		return smh_errno();
	return ret;
}

long smh_seek(long fd, long pos)
{
	long ret;
	struct smh_seek_s {
		long fd;
		long pos;
	} seek;

	debug("%s: fd %ld pos %ld\n", __func__, fd, pos);

	seek.fd = fd;
	seek.pos = pos;

	ret = smh_trap(SYSSEEK, &seek);
	if (ret)
		return smh_errno();
	return 0;
}

int smh_getc(void)
{
	return smh_trap(SYSREADC, NULL);
}

void smh_putc(char ch)
{
	smh_trap(SYSWRITEC, &ch);
}

void smh_puts(const char *s)
{
	smh_trap(SYSWRITE0, (char *)s);
}
