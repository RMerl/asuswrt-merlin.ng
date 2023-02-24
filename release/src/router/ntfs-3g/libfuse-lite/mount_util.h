/*
    FUSE: Filesystem in Userspace
    Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

    This program can be distributed under the terms of the GNU LGPLv2.
    See the file COPYING.LIB.
*/

#include <sys/types.h>

int fuse_mnt_add_mount(const char *progname, const char *fsname,
                       const char *mnt, const char *type, const char *opts);
int fuse_mnt_umount(const char *progname, const char *mnt, int lazy);
char *fuse_mnt_resolve_path(const char *progname, const char *orig);
int fuse_mnt_check_fuseblk(void);

#ifdef __SOLARIS__

int fuse_mnt_check_empty(const char *progname, const char *mnt,
			 mode_t rootmode, off_t rootsize);

#else /* __SOLARIS__ */

int fusermount(int unmount, int quiet, int lazy, const char *opts, 
	       const char *origmnt);

#endif /* __SOLARIS__ */
