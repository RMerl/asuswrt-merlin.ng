/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

/*
 * Header file for using yaffs in an application via
 * a direct interface.
 */


#ifndef __YAFFSFS_H__
#define __YAFFSFS_H__

#include "yaffscfg.h"
#include "yportenv.h"


#ifndef NAME_MAX
#define NAME_MAX	256
#endif

#define YAFFS_MAX_FILE_SIZE (0x800000000LL - 1)


struct yaffs_dirent {
	long d_ino;			/* inode number */
	off_t d_off;			/* offset to this dirent */
	unsigned short d_reclen;	/* length of this dirent */
	YUCHAR d_type;			/* type of this record */
	YCHAR d_name[NAME_MAX+1];	/* file name (null-terminated) */
	unsigned d_dont_use;		/* debug: not for public consumption */
};

typedef struct opaque_structure yaffs_DIR;



struct yaffs_stat {
	int		st_dev;		/* device */
	int		st_ino;		/* inode */
	unsigned	st_mode;	/* protection */
	int		st_nlink;	/* number of hard links */
	int		st_uid;		/* user ID of owner */
	int		st_gid;		/* group ID of owner */
	unsigned	st_rdev;	/* device type (if inode device) */
	loff_t		st_size;	/* total size, in bytes */
	unsigned long	st_blksize;	/* blocksize for filesystem I/O */
	unsigned long	st_blocks;	/* number of blocks allocated */
#ifdef CONFIG_YAFFS_WINCE
	/* Special 64-bit times for WinCE */
	unsigned long	yst_wince_atime[2];
	unsigned long	yst_wince_mtime[2];
	unsigned long	yst_wince_ctime[2];
#else
	unsigned long	yst_atime;	/* time of last access */
	unsigned long	yst_mtime;	/* time of last modification */
	unsigned long	yst_ctime;	/* time of last change */
#endif
};


struct yaffs_utimbuf {
	unsigned long actime;
	unsigned long modtime;
};


int yaffs_open(const YCHAR *path, int oflag, int mode) ;

int yaffs_close(int fd) ;
int yaffs_fsync(int fd) ;
int yaffs_fdatasync(int fd) ;
int yaffs_flush(int fd) ; /* same as yaffs_fsync() */

int yaffs_access(const YCHAR *path, int amode);

int yaffs_dup(int fd);

int yaffs_read(int fd, void *buf, unsigned int nbyte) ;
int yaffs_write(int fd, const void *buf, unsigned int nbyte) ;

int yaffs_pread(int fd, void *buf, unsigned int nbyte, loff_t offset);
int yaffs_pwrite(int fd, const void *buf, unsigned int nbyte, loff_t offset);

loff_t yaffs_lseek(int fd, loff_t offset, int whence) ;

int yaffs_truncate(const YCHAR *path, loff_t new_size);
int yaffs_ftruncate(int fd, loff_t new_size);

int yaffs_unlink(const YCHAR *path) ;
int yaffs_rename(const YCHAR *oldPath, const YCHAR *newPath) ;

int yaffs_stat(const YCHAR *path, struct yaffs_stat *buf) ;
int yaffs_lstat(const YCHAR *path, struct yaffs_stat *buf) ;
int yaffs_fstat(int fd, struct yaffs_stat *buf) ;

int yaffs_utime(const YCHAR *path, const struct yaffs_utimbuf *buf);
int yaffs_futime(int fd, const struct yaffs_utimbuf *buf);


int yaffs_setxattr(const char *path, const char *name,
			const void *data, int size, int flags);
int yaffs_lsetxattr(const char *path, const char *name,
			const void *data, int size, int flags);
int yaffs_fsetxattr(int fd, const char *name,
			const void *data, int size, int flags);

int yaffs_getxattr(const char *path, const char *name,
			void *data, int size);
int yaffs_lgetxattr(const char *path, const char *name,
			void *data, int size);
int yaffs_fgetxattr(int fd, const char *name,
			void *data, int size);

int yaffs_removexattr(const char *path, const char *name);
int yaffs_lremovexattr(const char *path, const char *name);
int yaffs_fremovexattr(int fd, const char *name);

int yaffs_listxattr(const char *path, char *list, int size);
int yaffs_llistxattr(const char *path, char *list, int size);
int yaffs_flistxattr(int fd, char *list, int size);


#ifdef CONFIG_YAFFS_WINCE

int yaffs_set_wince_times(int fd,
			const unsigned *wctime,
			const unsigned *watime,
			const unsigned *wmtime);
int yaffs_get_wince_times(int fd,
			unsigned *wctime,
			unsigned *watime,
			unsigned *wmtime);

#endif

int yaffs_chmod(const YCHAR *path, mode_t mode);
int yaffs_fchmod(int fd, mode_t mode);

int yaffs_mkdir(const YCHAR *path, mode_t mode) ;
int yaffs_rmdir(const YCHAR *path) ;

yaffs_DIR *yaffs_opendir(const YCHAR *dirname) ;
struct yaffs_dirent *yaffs_readdir(yaffs_DIR *dirp) ;
void yaffs_rewinddir(yaffs_DIR *dirp) ;
int yaffs_closedir(yaffs_DIR *dirp) ;

int yaffs_mount(const YCHAR *path) ;
int yaffs_mount2(const YCHAR *path, int read_only);
int yaffs_mount_common(const YCHAR *path, int read_only, int skip_checkpt);

int yaffs_unmount(const YCHAR *path) ;
int yaffs_unmount2(const YCHAR *path, int force);
int yaffs_remount(const YCHAR *path, int force, int read_only);


int yaffs_sync(const YCHAR *path) ;

int yaffs_symlink(const YCHAR *oldpath, const YCHAR *newpath);
int yaffs_readlink(const YCHAR *path, YCHAR *buf, int bufsiz);

int yaffs_link(const YCHAR *oldpath, const YCHAR *newpath);
int yaffs_mknod(const YCHAR *pathname, mode_t mode, dev_t dev);

loff_t yaffs_freespace(const YCHAR *path);
loff_t yaffs_totalspace(const YCHAR *path);

int yaffs_inodecount(const YCHAR *path);

int yaffs_n_handles(const YCHAR *path);

#define YAFFS_SHARE_READ  1
#define YAFFS_SHARE_WRITE 2
int yaffs_open_sharing(const YCHAR *path, int oflag, int mode, int shareMode);

struct yaffs_dev;
void yaffs_add_device(struct yaffs_dev *dev);

int yaffs_start_up(void);
int yaffsfs_GetLastError(void);

/* Functions to iterate through devices. NB Use with extreme care! */
void yaffs_dev_rewind(void);
struct yaffs_dev *yaffs_next_dev(void);

/* Function to get the last error */
int yaffs_get_error(void);
const char *yaffs_error_to_str(int err);

/* Function only for debugging */
void *yaffs_getdev(const YCHAR *path);
int yaffs_dump_dev(const YCHAR *path);
int yaffs_set_error(int error);

/* Trace control functions */
unsigned  yaffs_set_trace(unsigned tm);
unsigned  yaffs_get_trace(void);
#endif
