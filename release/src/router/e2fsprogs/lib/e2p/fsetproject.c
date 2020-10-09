/*
 * fgetproject.c --- get project id
 *
 * Copyright (C) 1999  Theodore Ts'o <tytso@mit.edu>
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include "config.h"
#if HAVE_ERRNO_H
#include <errno.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_EXT2_IOCTLS
#include <fcntl.h>
#include <sys/ioctl.h>
#include "project.h"
#endif

#include "e2p.h"

#ifdef O_LARGEFILE
#define OPEN_FLAGS (O_RDONLY|O_NONBLOCK|O_LARGEFILE)
#else
#define OPEN_FLAGS (O_RDONLY|O_NONBLOCK)
#endif

int fsetproject(const char *name, unsigned long project)
{
#ifndef FS_IOC_FSGETXATTR
	errno = EOPNOTSUPP;
	return -1;
#else
	int fd, r, save_errno = 0;
	struct fsxattr fsx;

	fd = open (name, OPEN_FLAGS);
	if (fd == -1)
		return -1;
	r = ioctl (fd, FS_IOC_FSGETXATTR, &fsx);
	if (r == -1) {
		save_errno = errno;
		goto errout;
	}
	fsx.fsx_projid = project;
	r = ioctl (fd, FS_IOC_FSSETXATTR, &fsx);
	if (r == -1)
		save_errno = errno;
errout:
	close (fd);
	if (save_errno)
		errno = save_errno;
	return r;
#endif
}
