/*
 * fgetversion.c	- Get a file version on an ext2 file system
 *
 * Copyright (C) 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                           Laboratoire MASI, Institut Blaise Pascal
 *                           Universite Pierre et Marie Curie (Paris VI)
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */

/*
 * History:
 * 93/10/30	- Creation
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
#include <fcntl.h>
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#include "e2p.h"

#ifdef O_LARGEFILE
#define OPEN_FLAGS (O_RDONLY|O_NONBLOCK|O_LARGEFILE)
#else
#define OPEN_FLAGS (O_RDONLY|O_NONBLOCK)
#endif

int fgetversion(const char *name, unsigned long *version)
{
	unsigned int ver = -1;
	int rc = -1;
#if HAVE_EXT2_IOCTLS
# if !APPLE_DARWIN
	int fd, save_errno = 0;

	fd = open(name, OPEN_FLAGS);
	if (fd == -1)
		return -1;

	rc = ioctl(fd, EXT2_IOC_GETVERSION, &ver);
	if (rc == -1)
		save_errno = errno;
	close(fd);
	if (rc == -1)
		errno = save_errno;
# else /* APPLE_DARWIN */
	rc = syscall(SYS_fsctl, name, EXT2_IOC_GETVERSION, &ver, 0);
# endif /* !APPLE_DARWIN */
#else /* ! HAVE_EXT2_IOCTLS */
	extern int errno;

	errno = EOPNOTSUPP;
#endif /* ! HAVE_EXT2_IOCTLS */
	if (rc == 0)
		*version = ver;

	return rc;
}
