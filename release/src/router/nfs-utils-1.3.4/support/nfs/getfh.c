/*
 * support/nfs/getfh.c
 *
 * Get the FH for a given client and directory. This function takes
 * the NFS protocol version number as an additional argument.
 *
 * This function has nothing in common with the SunOS getfh function,
 * which is a front-end to the RPC mount call.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include "nfslib.h"

/**
 * getfh_old - ask the kernel for an NFSv2 file handle via nfsctl()
 * @sin: pointer to IPv4 address of a client
 * @dev: device number of device where requested object resides
 * @ino: inode number of requested object
 *
 * Returns a pointer to an NFSv2 file handle, or NULL if some error
 * occurred.  errno is set to reflect the specifics of the error.
 */
struct nfs_fh_len *
getfh_old(const struct sockaddr_in *sin, const dev_t dev, const ino_t ino)
{
	union nfsctl_res	res;
	struct nfsctl_arg	arg;
	static struct nfs_fh_len rfh;

	if (sin->sin_family != AF_INET) {
		errno = EAFNOSUPPORT;
		return NULL;
	}

	memset(&arg, 0, sizeof(arg));
	memset(&res, 0, sizeof(res));

	arg.ca_version = NFSCTL_VERSION;
	arg.ca_getfh.gf_version = 2;	/* obsolete */
	arg.ca_getfh.gf_dev = dev;
	arg.ca_getfh.gf_ino = ino;
	memcpy(&arg.ca_getfh.gf_addr, sin, sizeof(*sin));

	if (nfsctl(NFSCTL_GETFH, &arg, &res) < 0)
		return NULL;

	memset(&rfh, 0, sizeof(rfh));
	rfh.fh_size = 32;
	memcpy(rfh.fh_handle, &res.cr_getfh, 32);
	return &rfh;
}

/**
 * getfh - ask the kernel for an NFSv2 file handle via nfsctl()
 * @sin: pointer to IPv4 address of a client
 * @path: pointer to a '\0'-terminated ASCII string containing an pathname
 *
 * Returns a pointer to an NFSv2 file handle, or NULL if some error
 * occurred.  errno is set to reflect the specifics of the error.
 */
struct nfs_fh_len *
getfh(const struct sockaddr_in *sin, const char *path)
{
	static union nfsctl_res res;
        struct nfsctl_arg       arg;
	static struct nfs_fh_len rfh;

	if (sin->sin_family != AF_INET) {
		errno = EAFNOSUPPORT;
		return NULL;
	}

	memset(&arg, 0, sizeof(arg));
	memset(&res, 0, sizeof(res));

        arg.ca_version = NFSCTL_VERSION;
        arg.ca_getfd.gd_version = 2;    /* obsolete */
        strncpy(arg.ca_getfd.gd_path, path,
		sizeof(arg.ca_getfd.gd_path) - 1);
	arg.ca_getfd.gd_path[sizeof (arg.ca_getfd.gd_path) - 1] = '\0';
	memcpy(&arg.ca_getfd.gd_addr, sin, sizeof(*sin));

        if (nfsctl(NFSCTL_GETFD, &arg, &res) < 0)
                return NULL;

	memset(&rfh, 0, sizeof(rfh));
	rfh.fh_size = 32;
	memcpy(rfh.fh_handle, &res.cr_getfh, 32);
	return &rfh;
}

/**
 * getfh_size - ask the kernel for a file handle via nfsctl()
 * @sin: pointer to IPv4 address of a client
 * @path: pointer to a '\0'-terminated ASCII string containing an pathname
 * @size: maximum size, in bytes, of the returned file handle
 *
 * Returns a pointer to an NFSv3 file handle, or NULL if some error
 * occurred.  errno is set to reflect the specifics of the error.
 */
struct nfs_fh_len *
getfh_size(const struct sockaddr_in *sin, const char *path, const int size)
{
        static union nfsctl_res res;
        struct nfsctl_arg       arg;

	if (sin->sin_family != AF_INET) {
		errno = EAFNOSUPPORT;
		return NULL;
	}

	memset(&arg, 0, sizeof(arg));
	memset(&res, 0, sizeof(res));

        arg.ca_version = NFSCTL_VERSION;
        strncpy(arg.ca_getfs.gd_path, path,
		sizeof(arg.ca_getfs.gd_path) - 1);
	arg.ca_getfs.gd_path[sizeof (arg.ca_getfs.gd_path) - 1] = '\0';
	memcpy(&arg.ca_getfs.gd_addr, sin, sizeof(*sin));
	arg.ca_getfs.gd_maxlen = size;

        if (nfsctl(NFSCTL_GETFS, &arg, &res) < 0)
                return NULL;

        return &res.cr_getfs;
}
