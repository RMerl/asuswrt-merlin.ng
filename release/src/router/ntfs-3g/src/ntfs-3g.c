/**
 * ntfs-3g - Third Generation NTFS Driver
 *
 * Copyright (c) 2005-2007 Yura Pakhuchiy
 * Copyright (c) 2005 Yuval Fledel
 * Copyright (c) 2006-2009 Szabolcs Szakacsits
 * Copyright (c) 2007-2021 Jean-Pierre Andre
 * Copyright (c) 2009 Erik Larsson
 *
 * This file is originated from the Linux-NTFS project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <fuse.h>

#if !defined(FUSE_VERSION) || (FUSE_VERSION < 26)
#error "***********************************************************"
#error "*                                                         *"
#error "*     Compilation requires at least FUSE version 2.6.0!   *"
#error "*                                                         *"
#error "***********************************************************"
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <signal.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <syslog.h>
#include <sys/wait.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif

#if defined(__APPLE__) || defined(__DARWIN__)
#include <sys/dirent.h>
#elif defined(__sun) && defined (__SVR4)
#include <sys/param.h>
#endif /* defined(__APPLE__) || defined(__DARWIN__), ... */

#ifndef FUSE_CAP_POSIX_ACL  /* until defined in <fuse/fuse_common.h> */
#define FUSE_CAP_POSIX_ACL (1 << 18)
#endif /* FUSE_CAP_POSIX_ACL */

#include "compat.h"
#include "attrib.h"
#include "inode.h"
#include "volume.h"
#include "dir.h"
#include "unistr.h"
#include "layout.h"
#include "index.h"
#include "ntfstime.h"
#include "security.h"
#include "reparse.h"
#include "ea.h"
#include "object_id.h"
#include "efs.h"
#include "logging.h"
#include "xattrs.h"
#include "misc.h"
#include "ioctl.h"
#include "plugin.h"

#include "ntfs-3g_common.h"

/*
 *	The following permission checking modes are governed by
 *	the HPERMSCONFIG value in param.h
 */

/*	ACLS may be checked by kernel (requires a fuse patch) or here */
#define KERNELACLS ((HPERMSCONFIG > 6) & (HPERMSCONFIG < 10))
/*	basic permissions may be checked by kernel or here */
#define KERNELPERMS (((HPERMSCONFIG - 1) % 6) < 3)
/*	may want to use fuse/kernel cacheing */
#define CACHEING (!(HPERMSCONFIG % 3))

#if KERNELACLS & !KERNELPERMS
#error Incompatible options KERNELACLS and KERNELPERMS
#endif

		/* sometimes the kernel cannot check access */
#define ntfs_real_allowed_access(scx, ni, type) ntfs_allowed_access(scx, ni, type)
#if POSIXACLS & KERNELPERMS & !KERNELACLS
		/* short-circuit if PERMS checked by kernel and ACLs by fs */
#define ntfs_allowed_access(scx, ni, type) \
	((scx)->vol->secure_flags & (1 << SECURITY_DEFAULT) \
	    ? 1 : ntfs_allowed_access(scx, ni, type))
#endif

#define set_archive(ni) (ni)->flags |= FILE_ATTR_ARCHIVE

/*
 *		Call a function from a reparse plugin (variable arguments)
 *	Requires "reparse" and "ops" to have been defined
 *
 *	Returns a non-negative value if successful,
 *		and a negative error code if something fails.
 */
#define CALL_REPARSE_PLUGIN(ni, op_name, ...)			\
	 (reparse = (REPARSE_POINT*)NULL,			 \
	 ops = select_reparse_plugin(ctx, ni, &reparse),	 \
	 (!ops ? -errno						 \
		 : (ops->op_name ?				 \
			 ops->op_name(ni, reparse, __VA_ARGS__)  \
			 : -EOPNOTSUPP))),			 \
		 free(reparse)

typedef enum {
	FSTYPE_NONE,
	FSTYPE_UNKNOWN,
	FSTYPE_FUSE,
	FSTYPE_FUSEBLK
} fuse_fstype;

typedef struct {
	fuse_fill_dir_t filler;
	void *buf;
} ntfs_fuse_fill_context_t;

enum {
	CLOSE_COMPRESSED = 1,
	CLOSE_ENCRYPTED = 2,
	CLOSE_DMTIME = 4,
	CLOSE_REPARSE = 8
};

static struct ntfs_options opts;

const char *EXEC_NAME = "ntfs-3g";

static ntfs_fuse_context_t *ctx;
static u32 ntfs_sequence;

static const char *usage_msg = 
"\n"
"%s %s %s %d - Third Generation NTFS Driver\n"
"\t\tConfiguration type %d, "
#ifdef HAVE_SETXATTR
"XATTRS are on, "
#else
"XATTRS are off, "
#endif
#if POSIXACLS
"POSIX ACLS are on\n"
#else
"POSIX ACLS are off\n"
#endif
"\n"
"Copyright (C) 2005-2007 Yura Pakhuchiy\n"
"Copyright (C) 2006-2009 Szabolcs Szakacsits\n"
"Copyright (C) 2007-2022 Jean-Pierre Andre\n"
"Copyright (C) 2009-2020 Erik Larsson\n"
"\n"
"Usage:    %s [-o option[,...]] <device|image_file> <mount_point>\n"
"\n"
"Options:  ro (read-only mount), windows_names, uid=, gid=,\n" 
"          umask=, fmask=, dmask=, streams_interface=.\n"
"          Please see the details in the manual (type: man ntfs-3g).\n"
"\n"
"Example: ntfs-3g /dev/sda1 /mnt/windows\n"
"\n"
#ifdef PLUGIN_DIR 
"Plugin path: " PLUGIN_DIR "\n\n"
#endif /* PLUGIN_DIR */
"%s";

static const char ntfs_bad_reparse[] = "unsupported reparse tag 0x%08lx";
	 /* exact length of target text, without the terminator */
#define ntfs_bad_reparse_lth (sizeof(ntfs_bad_reparse) + 2)

#ifdef FUSE_INTERNAL
int drop_privs(void);
int restore_privs(void);
#else
/*
 * setuid and setgid root ntfs-3g denies to start with external FUSE, 
 * therefore the below functions are no-op in such case.
 */
static int drop_privs(void)    { return 0; }
#if defined(linux) || defined(__uClinux__)
static int restore_privs(void) { return 0; }
#endif

static const char *setuid_msg =
"Mount is denied because setuid and setgid root ntfs-3g is insecure with the\n"
"external FUSE library. Either remove the setuid/setgid bit from the binary\n"
"or rebuild NTFS-3G with integrated FUSE support and make it setuid root.\n"
"Please see more information at\n"
"https://github.com/tuxera/ntfs-3g/wiki/NTFS-3G-FAQ\n";

static const char *unpriv_fuseblk_msg =
"Unprivileged user can not mount NTFS block devices using the external FUSE\n"
"library. Either mount the volume as root, or rebuild NTFS-3G with integrated\n"
"FUSE support and make it setuid root. Please see more information at\n"
"https://github.com/tuxera/ntfs-3g/wiki/NTFS-3G-FAQ\n";
#endif	


/**
 * ntfs_fuse_is_named_data_stream - check path to be to named data stream
 * @path:	path to check
 *
 * Returns 1 if path is to named data stream or 0 otherwise.
 */
static int ntfs_fuse_is_named_data_stream(const char *path)
{
	if (strchr(path, ':') && ctx->streams == NF_STREAMS_INTERFACE_WINDOWS)
		return 1;
	return 0;
}

static void ntfs_fuse_update_times(ntfs_inode *ni, ntfs_time_update_flags mask)
{
	if (ctx->atime == ATIME_DISABLED)
		mask &= ~NTFS_UPDATE_ATIME;
	else if (ctx->atime == ATIME_RELATIVE && mask == NTFS_UPDATE_ATIME &&
			(sle64_to_cpu(ni->last_access_time)
				>= sle64_to_cpu(ni->last_data_change_time)) &&
			(sle64_to_cpu(ni->last_access_time)
				>= sle64_to_cpu(ni->last_mft_change_time)))
		return;
	ntfs_inode_update_times(ni, mask);
}

static s64 ntfs_get_nr_free_mft_records(ntfs_volume *vol)
{
	ntfs_attr *na = vol->mftbmp_na;
	s64 nr_free = ntfs_attr_get_free_bits(na);

	if (nr_free >= 0)
		nr_free += (na->allocated_size - na->data_size) << 3;
	return nr_free;
}

/*
 *      Fill a security context as needed by security functions
 *      returns TRUE if there is a user mapping,
 *              FALSE if there is none
 *			This is not an error and the context is filled anyway,
 *			it is used for implicit Windows-like inheritance
 */

static BOOL ntfs_fuse_fill_security_context(struct SECURITY_CONTEXT *scx)
{
	struct fuse_context *fusecontext;

	scx->vol = ctx->vol;
	scx->mapping[MAPUSERS] = ctx->security.mapping[MAPUSERS];
	scx->mapping[MAPGROUPS] = ctx->security.mapping[MAPGROUPS];
	scx->pseccache = &ctx->seccache;
	fusecontext = fuse_get_context();
	scx->uid = fusecontext->uid;
	scx->gid = fusecontext->gid;
	scx->tid = fusecontext->pid;
#ifdef FUSE_CAP_DONT_MASK
		/* the umask can be processed by the file system */
	scx->umask = fusecontext->umask;
#else
		/* the umask if forced by fuse on creation */
	scx->umask = 0;
#endif

	return (ctx->security.mapping[MAPUSERS] != (struct MAPPING*)NULL);
}

#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)

/*
 *		Check access to parent directory
 *
 *	directory and file inodes are only opened when not fed in,
 *	they *HAVE TO* be fed in when already open, however
 *	file inode is only useful when S_ISVTX is requested
 *
 *	returns 1 if allowed,
 *		0 if not allowed or some error occurred (errno tells why)
 */

static int ntfs_allowed_dir_access(struct SECURITY_CONTEXT *scx,
			const char *path, ntfs_inode *dir_ni,
			ntfs_inode *ni, mode_t accesstype)
{
	int allowed;
	ntfs_inode *ni2;
	ntfs_inode *dir_ni2;
	char *dirpath;
	char *name;
	struct stat stbuf;

#if POSIXACLS & KERNELPERMS & !KERNELACLS
		/* short-circuit if PERMS checked by kernel and ACLs by fs */
	if (scx->vol->secure_flags & (1 << SECURITY_DEFAULT))
		allowed = 1;
	else
#endif
	{
		if (dir_ni)
			allowed = ntfs_real_allowed_access(scx, dir_ni,
					accesstype);
		else {
			allowed = 0;
			dirpath = strdup(path);
			if (dirpath) {
		/* the root of file system is seen as a parent of itself */
		/* is that correct ? */
				name = strrchr(dirpath, '/');
				*name = 0;
				dir_ni2 = ntfs_pathname_to_inode(scx->vol,
						NULL, dirpath);
				if (dir_ni2) {
					allowed = ntfs_real_allowed_access(scx,
						 dir_ni2, accesstype);
					if (ntfs_inode_close(dir_ni2))
						allowed = 0;
				}
				free(dirpath);
			}
		}
			/*
			 * for a not-owned sticky directory, have to
			 * check whether file itself is owned
			 */
		if ((accesstype == (S_IWRITE + S_IEXEC + S_ISVTX))
		   && (allowed == 2)) {
			if (ni)
				ni2 = ni;
			else
				ni2 = ntfs_pathname_to_inode(scx->vol, NULL,
					path);
			allowed = 0;
			if (ni2) {
				allowed = (ntfs_get_owner_mode(scx,ni2,&stbuf)
						>= 0)
					&& (stbuf.st_uid == scx->uid);
				if (!ni)
					ntfs_inode_close(ni2);
			}
		}
	}
	return (allowed);
}

#endif

#ifdef HAVE_SETXATTR	/* extended attributes interface required */

/*
 *		Check access to parent directory
 *
 *	for non-standard cases where access control cannot be checked by kernel
 *
 *	no known situations where S_ISVTX is requested
 *
 *	returns 1 if allowed,
 *		0 if not allowed or some error occurred (errno tells why)
 */

static int ntfs_allowed_real_dir_access(struct SECURITY_CONTEXT *scx,
			const char *path, ntfs_inode *dir_ni,
			mode_t accesstype)
{
	int allowed;
	ntfs_inode *dir_ni2;
	char *dirpath;
	char *name;

	if (dir_ni)
		allowed = ntfs_real_allowed_access(scx, dir_ni, accesstype);
	else {
		allowed = 0;
		dirpath = strdup(path);
		if (dirpath) {
		/* the root of file system is seen as a parent of itself */
		/* is that correct ? */
			name = strrchr(dirpath, '/');
			*name = 0;
			dir_ni2 = ntfs_pathname_to_inode(scx->vol, NULL,
					dirpath);
			if (dir_ni2) {
				allowed = ntfs_real_allowed_access(scx,
					dir_ni2, accesstype);
				if (ntfs_inode_close(dir_ni2))
					allowed = 0;
			}
			free(dirpath);
		}
	}
	return (allowed);
}

static ntfs_inode *get_parent_dir(const char *path)
{
	ntfs_inode *dir_ni;
	char *dirpath;
	char *p;

	dirpath = strdup(path);
	dir_ni = (ntfs_inode*)NULL;
	if (dirpath) {
		p = strrchr(dirpath,'/');
		if (p) {  /* always present, be safe */
			*p = 0;
			dir_ni = ntfs_pathname_to_inode(ctx->vol,
						NULL, dirpath);
		}			
		free(dirpath);
	} else
		errno = ENOMEM;
	return (dir_ni);
}


#endif /* HAVE_SETXATTR */

/**
 * ntfs_fuse_statfs - return information about mounted NTFS volume
 * @path:	ignored (but fuse requires it)
 * @sfs:	statfs structure in which to return the information
 *
 * Return information about the mounted NTFS volume @sb in the statfs structure
 * pointed to by @sfs (this is initialized with zeros before ntfs_statfs is
 * called). We interpret the values to be correct of the moment in time at
 * which we are called. Most values are variable otherwise and this isn't just
 * the free values but the totals as well. For example we can increase the
 * total number of file nodes if we run out and we can keep doing this until
 * there is no more space on the volume left at all.
 *
 * This code based on ntfs_statfs from ntfs kernel driver.
 *
 * Returns 0 on success or -errno on error.
 */
static int ntfs_fuse_statfs(const char *path __attribute__((unused)),
			    struct statvfs *sfs)
{
	s64 size;
	int delta_bits;
	ntfs_volume *vol;

	vol = ctx->vol;
	if (!vol)
		return -ENODEV;
	
	/* 
	 * File system block size. Used to calculate used/free space by df.
	 * Incorrectly documented as "optimal transfer block size". 
	 */
	sfs->f_bsize = vol->cluster_size;
	
	/* Fundamental file system block size, used as the unit. */
	sfs->f_frsize = vol->cluster_size;
	
	/*
	 * Total number of blocks on file system in units of f_frsize.
	 * Since inodes are also stored in blocks ($MFT is a file) hence
	 * this is the number of clusters on the volume.
	 */
	sfs->f_blocks = vol->nr_clusters;
	
	/* Free blocks available for all and for non-privileged processes. */
	size = vol->free_clusters;
	if (size < 0)
		size = 0;
	sfs->f_bavail = sfs->f_bfree = size;
	
	/* Free inodes on the free space */
	delta_bits = vol->cluster_size_bits - vol->mft_record_size_bits;
	if (delta_bits >= 0)
		size <<= delta_bits;
	else
		size >>= -delta_bits;
	
	/* Number of inodes at this point in time. */
	sfs->f_files = (vol->mftbmp_na->allocated_size << 3) + size;
	
	/* Free inodes available for all and for non-privileged processes. */
	size += vol->free_mft_records;
	if (size < 0)
		size = 0;
	sfs->f_ffree = sfs->f_favail = size;
	
	/* Maximum length of filenames. */
	sfs->f_namemax = NTFS_MAX_NAME_LEN;
	return 0;
}

/**
 * ntfs_fuse_parse_path - split path to path and stream name.
 * @org_path:		path to split
 * @path:		pointer to buffer in which parsed path saved
 * @stream_name:	pointer to buffer where stream name in unicode saved
 *
 * This function allocates buffers for @*path and @*stream, user must free them
 * after use.
 *
 * Return values:
 *	<0	Error occurred, return -errno;
 *	 0	No stream name, @*stream is not allocated and set to AT_UNNAMED.
 *	>0	Stream name length in unicode characters.
 */
static int ntfs_fuse_parse_path(const char *org_path, char **path,
		ntfschar **stream_name)
{
	char *stream_name_mbs;
	int res;

	stream_name_mbs = strdup(org_path);
	if (!stream_name_mbs)
		return -errno;
	if (ctx->streams == NF_STREAMS_INTERFACE_WINDOWS) {
		*path = strsep(&stream_name_mbs, ":");
		if (stream_name_mbs) {
			*stream_name = NULL;
			res = ntfs_mbstoucs(stream_name_mbs, stream_name);
			if (res < 0) {
				free(*path);
				*path = NULL;
				return -errno;
			}
			return res;
		}
	} else
		*path = stream_name_mbs;
	*stream_name = AT_UNNAMED;
	return 0;
}

static void set_fuse_error(int *err)
{
	if (!*err)
		*err = -errno;
}

#if defined(__APPLE__) || defined(__DARWIN__)
static int ntfs_macfuse_getxtimes(const char *org_path,
		struct timespec *bkuptime, struct timespec *crtime)
{
	int res = 0;
	ntfs_inode *ni;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	memset(bkuptime, 0, sizeof(struct timespec));
	memset(crtime, 0, sizeof(struct timespec));
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	
	/* We have no backup timestamp in NTFS. */
	crtime->tv_sec = sle64_to_cpu(ni->creation_time);
exit:
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

int ntfs_macfuse_setcrtime(const char *path, const struct timespec *tv)
{
	ntfs_inode *ni;
	int res = 0;

	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	
	if (tv) {
		ni->creation_time = cpu_to_sle64(tv->tv_sec);
		ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
	}

	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

int ntfs_macfuse_setbkuptime(const char *path, const struct timespec *tv)
{
	ntfs_inode *ni;
	int res = 0;
	
	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	
	/* 
	 * Only pretending to set backup time successfully to please the APIs of
	 * Mac OS X. In reality, NTFS has no backup time.
	 */
	
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

int ntfs_macfuse_setchgtime(const char *path, const struct timespec *tv)
{
	ntfs_inode *ni;
	int res = 0;

	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;

	if (tv) {
		ni->last_mft_change_time = cpu_to_sle64(tv->tv_sec);
		ntfs_fuse_update_times(ni, 0);
	}

	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}
#endif /* defined(__APPLE__) || defined(__DARWIN__) */

static void *ntfs_init(struct fuse_conn_info *conn)
{
#if defined(__APPLE__) || defined(__DARWIN__)
	FUSE_ENABLE_XTIMES(conn);
#endif
#ifdef FUSE_CAP_DONT_MASK
		/* request umask not to be enforced by fuse */
	conn->want |= FUSE_CAP_DONT_MASK;
#endif /* defined FUSE_CAP_DONT_MASK */
#if POSIXACLS & KERNELACLS
		/* request ACLs to be checked by kernel */
	conn->want |= FUSE_CAP_POSIX_ACL;
#endif /* POSIXACLS & KERNELACLS */
#ifdef FUSE_CAP_BIG_WRITES
	if (ctx->big_writes
	    && ((ctx->vol->nr_clusters << ctx->vol->cluster_size_bits)
			>= SAFE_CAPACITY_FOR_BIG_WRITES))
		conn->want |= FUSE_CAP_BIG_WRITES;
#endif
#ifdef FUSE_CAP_IOCTL_DIR
	conn->want |= FUSE_CAP_IOCTL_DIR;
#endif /* defined(FUSE_CAP_IOCTL_DIR) */
	return NULL;
}

#ifndef DISABLE_PLUGINS

/*
 *		Define attributes for a junction or symlink
 *		(internal plugin)
 */

static int junction_getattr(ntfs_inode *ni,
		const REPARSE_POINT *reparse __attribute__((unused)),
		struct stat *stbuf)
{
	char *target;
	int res;

	errno = 0;
	target = ntfs_make_symlink(ni, ctx->abs_mnt_point);
		/*
		 * If the reparse point is not a valid
		 * directory junction, and there is no error
		 * we still display as a symlink
		 */
	if (target || (errno == EOPNOTSUPP)) {
		if (target)
			stbuf->st_size = strlen(target);
		else
			stbuf->st_size = ntfs_bad_reparse_lth;
		stbuf->st_blocks = (ni->allocated_size + 511) >> 9;
		stbuf->st_mode = S_IFLNK;
		free(target);
		res = 0;
	} else {
		res = -errno;
	}
	return (res);
}

static int wsl_getattr(ntfs_inode *ni, const REPARSE_POINT *reparse,
			struct stat *stbuf)
{
	dev_t rdev;
	int res;

	res = ntfs_reparse_check_wsl(ni, reparse);
	if (!res) {
		switch (reparse->reparse_tag) {
		case IO_REPARSE_TAG_AF_UNIX :
			stbuf->st_mode = S_IFSOCK;
			break;
		case IO_REPARSE_TAG_LX_FIFO :
			stbuf->st_mode = S_IFIFO;
			break;
		case IO_REPARSE_TAG_LX_CHR :
			stbuf->st_mode = S_IFCHR;
			res = ntfs_ea_check_wsldev(ni, &rdev);
			stbuf->st_rdev = rdev;
			break;
		case IO_REPARSE_TAG_LX_BLK :
			stbuf->st_mode = S_IFBLK;
			res = ntfs_ea_check_wsldev(ni, &rdev);
			stbuf->st_rdev = rdev;
			break;
		default :
			stbuf->st_size = ntfs_bad_reparse_lth;
			stbuf->st_mode = S_IFLNK;
			break;
		}
	}
		/*
		 * If the reparse point is not a valid wsl special file
		 * we display as a symlink
		 */
	if (res) {
		stbuf->st_size = ntfs_bad_reparse_lth;
		stbuf->st_mode = S_IFLNK;
		res = 0;
	}
	return (res);
}

/*
 *		Apply permission masks to st_mode returned by a reparse handler
 */

static void apply_umask(struct stat *stbuf)
{
	switch (stbuf->st_mode & S_IFMT) {
	case S_IFREG :
		stbuf->st_mode &= ~ctx->fmask;
		break;
	case S_IFDIR :
		stbuf->st_mode &= ~ctx->dmask;
		break;
	case S_IFLNK :
		stbuf->st_mode = (stbuf->st_mode & S_IFMT) | 0777;
		break;
	default :
		break;
	}
}

#endif /* DISABLE_PLUGINS */

static int ntfs_fuse_getattr(const char *org_path, struct stat *stbuf)
{
	int res = 0;
	ntfs_inode *ni;
	ntfs_attr *na;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;
	BOOL withusermapping;
	struct SECURITY_CONTEXT security;

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	memset(stbuf, 0, sizeof(struct stat));
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	withusermapping = ntfs_fuse_fill_security_context(&security);
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		/*
		 * make sure the parent directory is searchable
		 */
	if (withusermapping
	    && !ntfs_allowed_dir_access(&security,path,
			(!strcmp(org_path,"/") ? ni : (ntfs_inode*)NULL),
			ni, S_IEXEC)) {
               	res = -EACCES;
               	goto exit;
	}
#endif
	stbuf->st_nlink = le16_to_cpu(ni->mrec->link_count);
	if (ctx->posix_nlink
	    && !(ni->flags & FILE_ATTR_REPARSE_POINT))
		stbuf->st_nlink = ntfs_dir_link_cnt(ni);

	if (((ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)
		|| (ni->flags & FILE_ATTR_REPARSE_POINT))
	    && !stream_name_len) {
		if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
			const plugin_operations_t *ops;
			REPARSE_POINT *reparse;

			res = CALL_REPARSE_PLUGIN(ni, getattr, stbuf);
			if (!res) {
				apply_umask(stbuf);
				goto ok;
			} else {
				stbuf->st_size = ntfs_bad_reparse_lth;
				stbuf->st_blocks =
					(ni->allocated_size + 511) >> 9;
				stbuf->st_mode = S_IFLNK;
				res = 0;
				goto ok;
			}
			goto exit;
#else /* DISABLE_PLUGINS */
			char *target;

			errno = 0;
			target = ntfs_make_symlink(ni, ctx->abs_mnt_point);
				/*
				 * If the reparse point is not a valid
				 * directory junction, and there is no error
				 * we still display as a symlink
				 */
			if (target || (errno == EOPNOTSUPP)) {
				if (target)
					stbuf->st_size = strlen(target);
				else
					stbuf->st_size = ntfs_bad_reparse_lth;
				stbuf->st_blocks = (ni->allocated_size + 511) >> 9;
				stbuf->st_nlink = le16_to_cpu(ni->mrec->link_count);
				stbuf->st_mode = S_IFLNK;
				free(target);
			} else {
				res = -errno;
				goto exit;
			}
#endif /* DISABLE_PLUGINS */
		} else {
			/* Directory. */
			stbuf->st_mode = S_IFDIR | (0777 & ~ctx->dmask);
			/* get index size, if not known */
			if (!test_nino_flag(ni, KnownSize)) {
				na = ntfs_attr_open(ni, AT_INDEX_ALLOCATION, NTFS_INDEX_I30, 4);
				if (na) {
					ni->data_size = na->data_size;
					ni->allocated_size = na->allocated_size;
					set_nino_flag(ni, KnownSize);
					ntfs_attr_close(na);
				}
			}
			stbuf->st_size = ni->data_size;
			stbuf->st_blocks = ni->allocated_size >> 9;
			if (!ctx->posix_nlink)
				stbuf->st_nlink = 1;	/* Make find(1) work */
		}
	} else {
		/* Regular or Interix (INTX) file. */
		stbuf->st_mode = S_IFREG;
		stbuf->st_size = ni->data_size;
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
		/*
		 * return data size rounded to next 512 byte boundary for
		 * encrypted files to include padding required for decryption
		 * also include 2 bytes for padding info
		*/
		if (ctx->efs_raw
		    && (ni->flags & FILE_ATTR_ENCRYPTED)
		    && ni->data_size)
			stbuf->st_size = ((ni->data_size + 511) & ~511) + 2;
#endif /* HAVE_SETXATTR */
		/* 
		 * Temporary fix to make ActiveSync work via Samba 3.0.
		 * See more on the ntfs-3g-devel list.
		 */
		stbuf->st_blocks = (ni->allocated_size + 511) >> 9;
		if (ni->flags & FILE_ATTR_SYSTEM || stream_name_len) {
			na = ntfs_attr_open(ni, AT_DATA, stream_name,
					stream_name_len);
			if (!na) {
				if (stream_name_len) {
					res = -ENOENT;
					goto exit;
				} else
					goto nodata;
			}
			if (stream_name_len) {
				stbuf->st_size = na->data_size;
				stbuf->st_blocks = na->allocated_size >> 9;
			}
			/* Check whether it's Interix FIFO or socket. */
			if (!(ni->flags & FILE_ATTR_HIDDEN) &&
					!stream_name_len) {
				/* FIFO. */
				if (na->data_size == 0)
					stbuf->st_mode = S_IFIFO;
				/* Socket link. */
				if (na->data_size == 1)
					stbuf->st_mode = S_IFSOCK;
			}
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
			/* encrypted named stream */
			/* round size up to next 512 byte boundary */
			if (ctx->efs_raw && stream_name_len && 
			    (na->data_flags & ATTR_IS_ENCRYPTED) &&
			    NAttrNonResident(na)) 
				stbuf->st_size = ((na->data_size+511) & ~511)+2;
#endif /* HAVE_SETXATTR */
			/*
			 * Check whether it's Interix symbolic link, block or
			 * character device.
			 */
			if ((u64)na->data_size <= sizeof(INTX_FILE_TYPES)
					+ sizeof(ntfschar) * PATH_MAX
				&& (u64)na->data_size >
					sizeof(INTX_FILE_TYPES)
				&& !stream_name_len) {
				
				INTX_FILE *intx_file;

				intx_file = ntfs_malloc(na->data_size);
				if (!intx_file) {
					res = -errno;
					ntfs_attr_close(na);
					goto exit;
				}
				if (ntfs_attr_pread(na, 0, na->data_size,
						intx_file) != na->data_size) {
					res = -errno;
					free(intx_file);
					ntfs_attr_close(na);
					goto exit;
				}
				if (intx_file->magic == INTX_BLOCK_DEVICE &&
						na->data_size == offsetof(
						INTX_FILE, device_end)) {
					stbuf->st_mode = S_IFBLK;
					stbuf->st_rdev = makedev(le64_to_cpu(
							intx_file->major),
							le64_to_cpu(
							intx_file->minor));
				}
				if (intx_file->magic == INTX_CHARACTER_DEVICE &&
						na->data_size == offsetof(
						INTX_FILE, device_end)) {
					stbuf->st_mode = S_IFCHR;
					stbuf->st_rdev = makedev(le64_to_cpu(
							intx_file->major),
							le64_to_cpu(
							intx_file->minor));
				}
				if (intx_file->magic == INTX_SYMBOLIC_LINK) {
					char *target = NULL;
					int len;

					/* st_size should be set to length of
					 * symlink target as multibyte string */
					len = ntfs_ucstombs(
							intx_file->target,
							(na->data_size -
							    offsetof(INTX_FILE,
								     target)) /
							       sizeof(ntfschar),
							     &target, 0);
					if (len < 0) {
						res = -errno;
						free(intx_file);
						ntfs_attr_close(na);
						goto exit;
					}
					free(target);
					stbuf->st_mode = S_IFLNK;
					stbuf->st_size = len;
				}
				free(intx_file);
			}
			ntfs_attr_close(na);
		}
		stbuf->st_mode |= (0777 & ~ctx->fmask);
	}
#ifndef DISABLE_PLUGINS
ok:
#endif /* DISABLE_PLUGINS */
	if (withusermapping) {
		if (ntfs_get_owner_mode(&security,ni,stbuf) < 0)
			set_fuse_error(&res);
	} else {
		stbuf->st_uid = ctx->uid;
       		stbuf->st_gid = ctx->gid;
	}
	if (S_ISLNK(stbuf->st_mode))
		stbuf->st_mode |= 0777;
nodata :
	stbuf->st_ino = ni->mft_no;
#ifdef HAVE_STRUCT_STAT_ST_ATIMESPEC
	stbuf->st_atimespec = ntfs2timespec(ni->last_access_time);
	stbuf->st_ctimespec = ntfs2timespec(ni->last_mft_change_time);
	stbuf->st_mtimespec = ntfs2timespec(ni->last_data_change_time);
#elif defined(HAVE_STRUCT_STAT_ST_ATIM)
 	stbuf->st_atim = ntfs2timespec(ni->last_access_time);
 	stbuf->st_ctim = ntfs2timespec(ni->last_mft_change_time);
 	stbuf->st_mtim = ntfs2timespec(ni->last_data_change_time);
#elif defined(HAVE_STRUCT_STAT_ST_ATIMENSEC)
	{
	struct timespec ts;

	ts = ntfs2timespec(ni->last_access_time);
	stbuf->st_atime = ts.tv_sec;
	stbuf->st_atimensec = ts.tv_nsec;
	ts = ntfs2timespec(ni->last_mft_change_time);
	stbuf->st_ctime = ts.tv_sec;
	stbuf->st_ctimensec = ts.tv_nsec;
	ts = ntfs2timespec(ni->last_data_change_time);
	stbuf->st_mtime = ts.tv_sec;
	stbuf->st_mtimensec = ts.tv_nsec;
	}
#else
#warning "No known way to set nanoseconds in struct stat !"
	{
	struct timespec ts;

	ts = ntfs2timespec(ni->last_access_time);
	stbuf->st_atime = ts.tv_sec;
	ts = ntfs2timespec(ni->last_mft_change_time);
	stbuf->st_ctime = ts.tv_sec;
	ts = ntfs2timespec(ni->last_data_change_time);
	stbuf->st_mtime = ts.tv_sec;
	}
#endif
exit:
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

#ifndef DISABLE_PLUGINS

/*
 *		Get the link defined by a junction or symlink
 *		(internal plugin)
 */

static int junction_readlink(ntfs_inode *ni,
			const REPARSE_POINT *reparse __attribute__((unused)),
			char **pbuf)
{
	int res;
	le32 tag;
	int lth;

	errno = 0;
	res = 0;
	*pbuf = ntfs_make_symlink(ni, ctx->abs_mnt_point);
	if (!*pbuf) {
		if (errno == EOPNOTSUPP) {
			*pbuf = (char*)ntfs_malloc(ntfs_bad_reparse_lth + 1);
			if (*pbuf) {
				if (reparse)
					tag = reparse->reparse_tag;
				else
					tag = const_cpu_to_le32(0);
				lth = snprintf(*pbuf, ntfs_bad_reparse_lth + 1,
						ntfs_bad_reparse,
						(long)le32_to_cpu(tag));
				if (lth != ntfs_bad_reparse_lth) {
					free(*pbuf);
					*pbuf = (char*)NULL;
					res = -errno;
				}
			} else
				res = -ENOMEM;
		} else
			res = -errno;
	}
	return (res);
}

#endif /* DISABLE_PLUGINS */

static int ntfs_fuse_readlink(const char *org_path, char *buf, size_t buf_size)
{
	char *path = NULL;
	ntfschar *stream_name;
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	INTX_FILE *intx_file = NULL;
	int stream_name_len, res = 0;
	REPARSE_POINT *reparse;
	le32 tag;
	int lth;

	/* Get inode. */
	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	if (stream_name_len > 0) {
		res = -EINVAL;
		goto exit;
	}
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
		/*
		 * Reparse point : analyze as a junction point
		 */
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
		char *gotlink;
		const plugin_operations_t *ops;

		gotlink = (char*)NULL;
		res = CALL_REPARSE_PLUGIN(ni, readlink, &gotlink);
		if (gotlink) {
			strncpy(buf, gotlink, buf_size);
			free(gotlink);
			res = 0;
		} else {
			errno = EOPNOTSUPP;
			res = -EOPNOTSUPP;
		}
#else /* DISABLE_PLUGINS */
		char *target;

		errno = 0;
		res = 0;
		target = ntfs_make_symlink(ni, ctx->abs_mnt_point);
		if (target) {
			strncpy(buf,target,buf_size);
			free(target);
		} else
			res = -errno;
#endif /* DISABLE_PLUGINS */
		if (res == -EOPNOTSUPP) {
			reparse = ntfs_get_reparse_point(ni);
			if (reparse) {
				tag = reparse->reparse_tag;
				free(reparse);
			} else
				tag = const_cpu_to_le32(0);
			lth = snprintf(buf, ntfs_bad_reparse_lth + 1,
					ntfs_bad_reparse,
					(long)le32_to_cpu(tag));
			res = 0;
			if (lth != ntfs_bad_reparse_lth)
				res = -errno;
		}
		goto exit;
	}
	/* Sanity checks. */
	if (!(ni->flags & FILE_ATTR_SYSTEM)) {
		res = -EINVAL;
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		res = -errno;
		goto exit;
	}
	if ((size_t)na->data_size <= sizeof(INTX_FILE_TYPES)) {
		res = -EINVAL;
		goto exit;
	}
	if ((size_t)na->data_size > sizeof(INTX_FILE_TYPES) +
			sizeof(ntfschar) * PATH_MAX) {
		res = -ENAMETOOLONG;
		goto exit;
	}
	/* Receive file content. */
	intx_file = ntfs_malloc(na->data_size);
	if (!intx_file) {
		res = -errno;
		goto exit;
	}
	if (ntfs_attr_pread(na, 0, na->data_size, intx_file) != na->data_size) {
		res = -errno;
		goto exit;
	}
	/* Sanity check. */
	if (intx_file->magic != INTX_SYMBOLIC_LINK) {
		res = -EINVAL;
		goto exit;
	}
	/* Convert link from unicode to local encoding. */
	if (ntfs_ucstombs(intx_file->target, (na->data_size -
			offsetof(INTX_FILE, target)) / sizeof(ntfschar),
			&buf, buf_size) < 0) {
		res = -errno;
		goto exit;
	}
exit:
	if (intx_file)
		free(intx_file);
	if (na)
		ntfs_attr_close(na);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_filler(ntfs_fuse_fill_context_t *fill_ctx,
		const ntfschar *name, const int name_len, const int name_type,
		const s64 pos __attribute__((unused)), const MFT_REF mref,
		const unsigned dt_type __attribute__((unused)))
{
	char *filename = NULL;
	int ret = 0;
	int filenamelen = -1;

	if (name_type == FILE_NAME_DOS)
		return 0;
	
	if ((filenamelen = ntfs_ucstombs(name, name_len, &filename, 0)) < 0) {
		ntfs_log_perror("Filename decoding failed (inode %llu)",
				(unsigned long long)MREF(mref));
		return -1;
	}
	
	if (ntfs_fuse_is_named_data_stream(filename)) {
		ntfs_log_error("Unable to access '%s' (inode %llu) with "
				"current named streams access interface.\n",
				filename, (unsigned long long)MREF(mref));
		free(filename);
		return 0;
	} else {
		struct stat st = { .st_ino = MREF(mref) };
#ifndef DISABLE_PLUGINS
		ntfs_inode *ni;
#endif /* DISABLE_PLUGINS */
		 
		switch (dt_type) {
		case NTFS_DT_DIR :
			st.st_mode = S_IFDIR | (0777 & ~ctx->dmask); 
			break;
		case NTFS_DT_LNK :
			st.st_mode = S_IFLNK | 0777;
			break;
		case NTFS_DT_FIFO :
			st.st_mode = S_IFIFO;
			break;
		case NTFS_DT_SOCK :
			st.st_mode = S_IFSOCK;
			break;
		case NTFS_DT_BLK :
			st.st_mode = S_IFBLK;
			break;
		case NTFS_DT_CHR :
			st.st_mode = S_IFCHR;
			break;
		case NTFS_DT_REPARSE :
			st.st_mode = S_IFLNK | 0777; /* default */
#ifndef DISABLE_PLUGINS
			/* get emulated type from plugin if available */
			ni = ntfs_inode_open(ctx->vol, mref);
			if (ni && (ni->flags & FILE_ATTR_REPARSE_POINT)) {
				const plugin_operations_t *ops;
				REPARSE_POINT *reparse;
				int res;

				res = CALL_REPARSE_PLUGIN(ni, getattr, &st);
				if (!res)
					apply_umask(&st);
				else
					st.st_mode = S_IFLNK;
			}
			if (ni)
				ntfs_inode_close(ni);
#endif /* DISABLE_PLUGINS */
			break;
		default : /* unexpected types shown as plain files */
		case NTFS_DT_REG :
			st.st_mode = S_IFREG | (0777 & ~ctx->fmask);
			break;
		}
		
#if defined(__APPLE__) || defined(__DARWIN__)
		/* 
		 * Returning file names larger than MAXNAMLEN (255) bytes
		 * causes Darwin/Mac OS X to bug out and skip the entry. 
		 */
		if (filenamelen > MAXNAMLEN) {
			ntfs_log_debug("Truncating %d byte filename to "
				       "%d bytes.\n", filenamelen, MAXNAMLEN);
			ntfs_log_debug("  before: '%s'\n", filename);
			memset(filename + MAXNAMLEN, 0, filenamelen - MAXNAMLEN);
			ntfs_log_debug("   after: '%s'\n", filename);
		}
#elif defined(__sun) && defined (__SVR4)
		/*
		 * Returning file names larger than MAXNAMELEN (256) bytes
		 * causes Solaris/Illumos to return an I/O error from the system
		 * call.
		 * However we also need space for a terminating NULL, or user
		 * space tools will bug out since they expect a NULL terminator.
		 * Effectively the maximum length of a file name is MAXNAMELEN -
		 * 1 (255).
		 */
		if (filenamelen > (MAXNAMELEN - 1)) {
			ntfs_log_debug("Truncating %d byte filename to %d "
				"bytes.\n", filenamelen, MAXNAMELEN - 1);
			ntfs_log_debug("  before: '%s'\n", filename);
			memset(&filename[MAXNAMELEN - 1], 0,
				filenamelen - (MAXNAMELEN - 1));
			ntfs_log_debug("   after: '%s'\n", filename);
		}
#endif /* defined(__APPLE__) || defined(__DARWIN__), ... */
	
		ret = fill_ctx->filler(fill_ctx->buf, filename, &st, 0);
	}
	
	free(filename);
	return ret;
}

#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)

static int ntfs_fuse_opendir(const char *path,
		struct fuse_file_info *fi)
{
	int res = 0;
	ntfs_inode *ni;
	int accesstype;
	struct SECURITY_CONTEXT security;

	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */

	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (ni) {
		if (ntfs_fuse_fill_security_context(&security)) {
			if (fi->flags & O_WRONLY)
				accesstype = S_IWRITE;
			else
				if (fi->flags & O_RDWR)
					accesstype = S_IWRITE | S_IREAD;
				else
					accesstype = S_IREAD;
				/*
				 * directory must be searchable
				 * and requested access be allowed
				 */
			if (!strcmp(path,"/")
				? !ntfs_allowed_dir_access(&security,
					path, ni, ni, accesstype | S_IEXEC)
				: !ntfs_allowed_dir_access(&security, path,
						(ntfs_inode*)NULL, ni, S_IEXEC)
				     || !ntfs_allowed_access(&security,
						ni,accesstype))
				res = -EACCES;
		}
		if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
			const plugin_operations_t *ops;
			REPARSE_POINT *reparse;

			fi->fh = 0;
			res = CALL_REPARSE_PLUGIN(ni, opendir, fi);
#else /* DISABLE_PLUGINS */
			res = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
		}
		if (ntfs_inode_close(ni))
			set_fuse_error(&res);
	} else
		res = -errno;
	return res;
}

#endif

static int ntfs_fuse_readdir(const char *path, void *buf,
		fuse_fill_dir_t filler, off_t offset __attribute__((unused)),
		struct fuse_file_info *fi __attribute__((unused)))
{
	ntfs_fuse_fill_context_t fill_ctx;
	ntfs_inode *ni;
	s64 pos = 0;
	int err = 0;

	fill_ctx.filler = filler;
	fill_ctx.buf = buf;
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;

	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
		const plugin_operations_t *ops;
		REPARSE_POINT *reparse;

		err = CALL_REPARSE_PLUGIN(ni, readdir, &pos, &fill_ctx,
				(ntfs_filldir_t)ntfs_fuse_filler, fi);
#else /* DISABLE_PLUGINS */
		err = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
	} else {
		if (ntfs_readdir(ni, &pos, &fill_ctx,
				(ntfs_filldir_t)ntfs_fuse_filler))
			err = -errno;
	}
	ntfs_fuse_update_times(ni, NTFS_UPDATE_ATIME);
	if (ntfs_inode_close(ni))
		set_fuse_error(&err);
	return err;
}

static int ntfs_fuse_open(const char *org_path,
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		struct fuse_file_info *fi)
#else
		struct fuse_file_info *fi __attribute__((unused)))
#endif
{
	ntfs_inode *ni;
	ntfs_attr *na = NULL;
	int res = 0;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	int accesstype;
	struct SECURITY_CONTEXT security;
#endif

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (ni) {
		if (!(ni->flags & FILE_ATTR_REPARSE_POINT)) {
			na = ntfs_attr_open(ni, AT_DATA, stream_name, stream_name_len);
			if (!na) {
				res = -errno;
				goto close;
			}
		}
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		if (ntfs_fuse_fill_security_context(&security)) {
			if (fi->flags & O_WRONLY)
				accesstype = S_IWRITE;
			else
				if (fi->flags & O_RDWR)
					 accesstype = S_IWRITE | S_IREAD;
				else
					accesstype = S_IREAD;
			/*
			 * directory must be searchable
			 * and requested access allowed
			 */
			if (!ntfs_allowed_dir_access(&security,
				    path,(ntfs_inode*)NULL,ni,S_IEXEC)
			  || !ntfs_allowed_access(&security,
					ni,accesstype))
				res = -EACCES;
		}
#endif
		if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
			const plugin_operations_t *ops;
			REPARSE_POINT *reparse;

			fi->fh = 0;
			res = CALL_REPARSE_PLUGIN(ni, open, fi);
#else /* DISABLE_PLUGINS */
			res = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
			goto close;
		}
		if ((res >= 0)
		    && (fi->flags & (O_WRONLY | O_RDWR))) {
		/* mark a future need to compress the last chunk */
			if (na->data_flags & ATTR_COMPRESSION_MASK)
				fi->fh |= CLOSE_COMPRESSED;
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
			/* mark a future need to fixup encrypted inode */
			if (ctx->efs_raw
			    && !(na->data_flags & ATTR_IS_ENCRYPTED)
			    && (ni->flags & FILE_ATTR_ENCRYPTED))
				fi->fh |= CLOSE_ENCRYPTED;
#endif /* HAVE_SETXATTR */
		/* mark a future need to update the mtime */
			if (ctx->dmtime)
				fi->fh |= CLOSE_DMTIME;
		/* deny opening metadata files for writing */
			if (ni->mft_no < FILE_first_user)
				res = -EPERM;
		}
		ntfs_attr_close(na);
close:
		if (ntfs_inode_close(ni))
			set_fuse_error(&res);
	} else
		res = -errno;
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_read(const char *org_path, char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi __attribute__((unused)))
{
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len, res;
	s64 total = 0;
	s64 max_read;

	if (!size)
		return 0;

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
		const plugin_operations_t *ops;
		REPARSE_POINT *reparse;

		if (stream_name_len || !fi) {
			res = -EINVAL;
			goto exit;
		}
		res = CALL_REPARSE_PLUGIN(ni, read, buf, size, offset, fi);
		if (res >= 0) {
			goto stamps;
		}
#else /* DISABLE_PLUGINS */
		res = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, stream_name, stream_name_len);
	if (!na) {
		res = -errno;
		goto exit;
	}
	max_read = na->data_size;
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
	/* limit reads at next 512 byte boundary for encrypted attributes */
	if (ctx->efs_raw
	    && max_read
	    && (na->data_flags & ATTR_IS_ENCRYPTED)
	    && NAttrNonResident(na)) {
		max_read = ((na->data_size+511) & ~511) + 2;
	}
#endif /* HAVE_SETXATTR */
	if (offset + (off_t)size > max_read) {
		if (max_read < offset)
			goto ok;
		size = max_read - offset;
	}
	while (size > 0) {
		s64 ret = ntfs_attr_pread(na, offset, size, buf + total);
		if (ret != (s64)size)
			ntfs_log_perror("ntfs_attr_pread error reading '%s' at "
				"offset %lld: %lld <> %lld", org_path, 
				(long long)offset, (long long)size, (long long)ret);
		if (ret <= 0 || ret > (s64)size) {
			res = (ret < 0) ? -errno : -EIO;
			goto exit;
		}
		size -= ret;
		offset += ret;
		total += ret;
	}
ok:
	res = total;
#ifndef DISABLE_PLUGINS
stamps:
#endif /* DISABLE_PLUGINS */
	ntfs_fuse_update_times(ni, NTFS_UPDATE_ATIME);
exit:
	if (na)
		ntfs_attr_close(na);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_write(const char *org_path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi __attribute__((unused)))
{
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len, res, total = 0;

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0) {
		res = stream_name_len;
		goto out;
	}
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
		const plugin_operations_t *ops;
		REPARSE_POINT *reparse;

		if (stream_name_len || !fi) {
			res = -EINVAL;
			goto exit;
		}
		res = CALL_REPARSE_PLUGIN(ni, write, buf, size, offset, fi);
		if (res >= 0) {
			goto stamps;
		}
#else /* DISABLE_PLUGINS */
		res = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, stream_name, stream_name_len);
	if (!na) {
		res = -errno;
		goto exit;
	}
	while (size) {
		s64 ret = ntfs_attr_pwrite(na, offset, size, buf + total);
		if (ret <= 0) {
			res = -errno;
			goto exit;
		}
		size   -= ret;
		offset += ret;
		total  += ret;
	}
	res = total;
#ifndef DISABLE_PLUGINS
stamps: 
#endif /* DISABLE_PLUGINS */
	if ((res > 0)
	    && (!ctx->dmtime
		|| (sle64_to_cpu(ntfs_current_time())
		     - sle64_to_cpu(ni->last_data_change_time)) > ctx->dmtime))
		ntfs_fuse_update_times(ni, NTFS_UPDATE_MCTIME);
exit:
	if (na)
		ntfs_attr_close(na);
	if (res > 0)
		set_archive(ni);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	free(path);
	if (stream_name_len)
		free(stream_name);
out:	
	return res;
}

static int ntfs_fuse_release(const char *org_path,
		struct fuse_file_info *fi)
{
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len, res;

	if (!fi) {
		res = -EINVAL;
		goto out;
	}

	/* Only for marked descriptors there is something to do */
	
	if (!fi->fh) {
		res = 0;
		goto out;
	}
	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0) {
		res = stream_name_len;
		goto out;
	}
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
		const plugin_operations_t *ops;
		REPARSE_POINT *reparse;

		if (stream_name_len) {
			res = -EINVAL;
			goto exit;
		}
		res = CALL_REPARSE_PLUGIN(ni, release, fi);
		if (!res) {
			goto stamps;
		}
#else /* DISABLE_PLUGINS */
			/* Assume release() was not needed */
		res = 0;
#endif /* DISABLE_PLUGINS */
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, stream_name, stream_name_len);
	if (!na) {
		res = -errno;
		goto exit;
	}
	res = 0;
	if (fi->fh & CLOSE_COMPRESSED)
		res = ntfs_attr_pclose(na);
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
	if (fi->fh & CLOSE_ENCRYPTED)
		res = ntfs_efs_fixup_attribute(NULL, na);
#endif /* HAVE_SETXATTR */
#ifndef DISABLE_PLUGINS
stamps:
#endif /* DISABLE_PLUGINS */
	if (fi->fh & CLOSE_DMTIME)
		ntfs_inode_update_times(ni,NTFS_UPDATE_MCTIME);
exit:
	if (na)
		ntfs_attr_close(na);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	free(path);
	if (stream_name_len)
		free(stream_name);
out:	
	return res;
}

/*
 *	Common part for truncate() and ftruncate()
 */

static int ntfs_fuse_trunc(const char *org_path, off_t size,
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
			BOOL chkwrite)
#else
			BOOL chkwrite __attribute__((unused)))
#endif
{
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	int res;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;
	s64 oldsize;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	struct SECURITY_CONTEXT security;
#endif

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		goto exit;
	/* deny truncating metadata files */
	if (ni->mft_no < FILE_first_user) {
		errno = EPERM;
		goto exit;
	}

	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
		const plugin_operations_t *ops;
		REPARSE_POINT *reparse;

		if (stream_name_len) {
			res = -EINVAL;
			goto exit;
		}
		res = CALL_REPARSE_PLUGIN(ni, truncate, size);
		if (!res) {
			set_archive(ni);
			goto stamps;
		}
#else /* DISABLE_PLUGINS */
		res = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, stream_name, stream_name_len);
	if (!na)
		goto exit;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	/*
	 * JPA deny truncation if cannot search in parent directory
	 * or cannot write to file (already checked for ftruncate())
	 */
	if (ntfs_fuse_fill_security_context(&security)
		&& (!ntfs_allowed_dir_access(&security, path,
			 (ntfs_inode*)NULL, ni, S_IEXEC)
	          || (chkwrite
		     && !ntfs_allowed_access(&security, ni, S_IWRITE)))) {
		errno = EACCES;
		goto exit;
	}
#endif
		/*
		 * For compressed files, upsizing is done by inserting a final
		 * zero, which is optimized as creating a hole when possible. 
		 */
	oldsize = na->data_size;
	if ((na->data_flags & ATTR_COMPRESSION_MASK)
	    && (size > na->initialized_size)) {
		char zero = 0;
		if (ntfs_attr_pwrite(na, size - 1, 1, &zero) <= 0)
			goto exit;
	} else
		if (ntfs_attr_truncate(na, size))
			goto exit;
	if (oldsize != size)
		set_archive(ni);

#ifndef DISABLE_PLUGINS	
stamps:
#endif /* DISABLE_PLUGINS */
	ntfs_fuse_update_times(ni, NTFS_UPDATE_MCTIME);
	errno = 0;
exit:
	res = -errno;
	ntfs_attr_close(na);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_truncate(const char *org_path, off_t size)
{
	return ntfs_fuse_trunc(org_path, size, TRUE);
}

static int ntfs_fuse_ftruncate(const char *org_path, off_t size,
			struct fuse_file_info *fi __attribute__((unused)))
{
	/*
	 * in ->ftruncate() the file handle is guaranteed
	 * to have been opened for write.
	 */
	return (ntfs_fuse_trunc(org_path, size, FALSE));
}

static int ntfs_fuse_chmod(const char *path,
		mode_t mode)
{
	int res = 0;
	ntfs_inode *ni;
	struct SECURITY_CONTEXT security;

	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */

		/*
		 * Return unsupported if no user mapping has been defined
		 * or enforcing Windows-type inheritance
		 */
	if (ctx->inherit
	    || !ntfs_fuse_fill_security_context(&security)) {
		if (ctx->silent)
			res = 0;
		else
			res = -EOPNOTSUPP;
	} else {
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		   /* parent directory must be executable */
		if (ntfs_allowed_dir_access(&security,path,
				(ntfs_inode*)NULL,(ntfs_inode*)NULL,S_IEXEC)) {
#endif
			ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
			if (!ni)
				res = -errno;
			else {
				if (ntfs_set_mode(&security,ni,mode))
					res = -errno;
				else
					ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
				NInoSetDirty(ni);
				if (ntfs_inode_close(ni))
					set_fuse_error(&res);
			}
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		} else
			res = -errno;
#endif
	}
	return res;
}

static int ntfs_fuse_chown(const char *path, uid_t uid, gid_t gid)
{
	ntfs_inode *ni;
	int res;
	struct SECURITY_CONTEXT security;

	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
		/*
		 * Return unsupported if no user mapping has been defined
		 * or enforcing Windows-type inheritance
		 */
	if (ctx->inherit
	    || !ntfs_fuse_fill_security_context(&security)) {
		if (ctx->silent)
			return 0;
		if (uid == ctx->uid && gid == ctx->gid)
			return 0;
		return -EOPNOTSUPP;
	} else {
		res = 0;
		if (((int)uid != -1) || ((int)gid != -1)) {
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
			   /* parent directory must be executable */
		
			if (ntfs_allowed_dir_access(&security,path,
				(ntfs_inode*)NULL,(ntfs_inode*)NULL,S_IEXEC)) {
#endif
				ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
				if (!ni)
					res = -errno;
				else {
					if (ntfs_set_owner(&security,
							ni,uid,gid))
						res = -errno;
					else
						ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
					if (ntfs_inode_close(ni))
						set_fuse_error(&res);
				}
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
			} else
				res = -errno;
#endif
		}
	}
	return (res);
}

#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)

static int ntfs_fuse_access(const char *path, int type)
{
	int res = 0;
	int mode;
	ntfs_inode *ni;
	struct SECURITY_CONTEXT security;

	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */

	  /* JPA return unsupported if no user mapping has been defined */
	if (!ntfs_fuse_fill_security_context(&security)) {
		if (ctx->silent)
			res = 0;
		else
			res = -EOPNOTSUPP;
	} else {
		   /* parent directory must be seachable */
		if (ntfs_allowed_dir_access(&security,path,(ntfs_inode*)NULL,
				(ntfs_inode*)NULL,S_IEXEC)) {
			ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
			if (!ni) {
				res = -errno;
			} else {
				mode = 0;
				if (type & (X_OK | W_OK | R_OK)) {
					if (type & X_OK) mode += S_IEXEC;
					if (type & W_OK) mode += S_IWRITE;
					if (type & R_OK) mode += S_IREAD;
					if (!ntfs_allowed_access(&security,
							ni, mode))
						res = -errno;
				}
				if (ntfs_inode_close(ni))
					set_fuse_error(&res);
			}
		} else
			res = -errno;
	}
	return (res);
}

#endif

static int ntfs_fuse_create(const char *org_path, mode_t typemode, dev_t dev,
		const char *target, struct fuse_file_info *fi)
{
	char *name;
	ntfschar *uname = NULL, *utarget = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	char *dir_path;
	le32 securid;
	char *path = NULL;
	gid_t gid;
	mode_t dsetgid;
	ntfschar *stream_name;
	int stream_name_len;
	mode_t type = typemode & ~07777;
	mode_t perm;
	struct SECURITY_CONTEXT security;
	int res = 0, uname_len, utarget_len;

	dir_path = strdup(org_path);
	if (!dir_path)
		return -errno;
	/* Generate unicode filename. */
	name = strrchr(dir_path, '/');
	name++;
	uname_len = ntfs_mbstoucs(name, &uname);
	if ((uname_len < 0)
	    || (ctx->windows_names
		&& ntfs_forbidden_names(ctx->vol,uname,uname_len,TRUE))) {
		res = -errno;
		goto exit;
	}
	stream_name_len = ntfs_fuse_parse_path(org_path,
					 &path, &stream_name);
		/* stream name validity has been checked previously */
	if (stream_name_len < 0) {
		res = stream_name_len;
		goto exit;
	}
	/* Open parent directory. */
	*--name = 0;
	dir_ni = ntfs_pathname_to_inode(ctx->vol, NULL, dir_path);
		/* Deny creating files in $Extend */
	if (!dir_ni || (dir_ni->mft_no == FILE_Extend)) {
		free(path);
		res = -errno;
		if (dir_ni)
			res = -EPERM;
		goto exit;
	}
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		/* make sure parent directory is writeable and executable */
	if (!ntfs_fuse_fill_security_context(&security)
	       || ntfs_allowed_create(&security,
				dir_ni, &gid, &dsetgid)) {
#else
		ntfs_fuse_fill_security_context(&security);
		ntfs_allowed_create(&security, dir_ni, &gid, &dsetgid);
#endif
		if (S_ISDIR(type))
			perm = (typemode & ~ctx->dmask & 0777)
				| (dsetgid & S_ISGID);
		else
			if ((ctx->special_files == NTFS_FILES_WSL)
			    && S_ISLNK(type))
				perm = typemode | 0777;
			else
				perm = typemode & ~ctx->fmask & 0777;
			/*
			 * Try to get a security id available for
			 * file creation (from inheritance or argument).
			 * This is not possible for NTFS 1.x, and we will
			 * have to build a security attribute later.
			 */
		if (!ctx->security.mapping[MAPUSERS])
			securid = const_cpu_to_le32(0);
		else
			if (ctx->inherit)
				securid = ntfs_inherited_id(&security,
					dir_ni, S_ISDIR(type));
			else
#if POSIXACLS
				securid = ntfs_alloc_securid(&security,
					security.uid, gid,
					dir_ni, perm, S_ISDIR(type));
#else
				securid = ntfs_alloc_securid(&security,
					security.uid, gid,
					perm & ~security.umask, S_ISDIR(type));
#endif
		/* Create object specified in @type. */
		if (dir_ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
			const plugin_operations_t *ops;
			REPARSE_POINT *reparse;

			reparse = (REPARSE_POINT*)NULL;
			ops = select_reparse_plugin(ctx, dir_ni, &reparse);
			if (ops && ops->create) {
				ni = (*ops->create)(dir_ni, reparse,
					securid, uname, uname_len, type);
			} else {
				ni = (ntfs_inode*)NULL;
				errno = EOPNOTSUPP;
			}
			free(reparse);
#else /* DISABLE_PLUGINS */
			errno = EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
		} else {
			switch (type) {
				case S_IFCHR:
				case S_IFBLK:
					ni = ntfs_create_device(dir_ni, securid,
						uname, uname_len, type,	dev);
					break;
				case S_IFLNK:
					utarget_len = ntfs_mbstoucs(target,
							&utarget);
					if (utarget_len < 0) {
						res = -errno;
						goto exit;
					}
					ni = ntfs_create_symlink(dir_ni,
						securid, uname, uname_len,
						utarget, utarget_len);
					break;
				default:
					ni = ntfs_create(dir_ni, securid,
						uname, uname_len, type);
					break;
			}
		}
		if (ni) {
				/*
				 * set the security attribute if a security id
				 * could not be allocated (eg NTFS 1.x)
				 */
			if (ctx->security.mapping[MAPUSERS]) {
#if POSIXACLS
			   	if (!securid
				   && ntfs_set_inherited_posix(&security, ni,
					security.uid, gid,
					dir_ni, perm) < 0)
					set_fuse_error(&res);
#else
			   	if (!securid
				   && ntfs_set_owner_mode(&security, ni,
					security.uid, gid, 
					perm & ~security.umask) < 0)
					set_fuse_error(&res);
#endif
			}
			set_archive(ni);
			/* mark a need to compress the end of file */
			if (fi && (ni->flags & FILE_ATTR_COMPRESSED)) {
				fi->fh |= CLOSE_COMPRESSED;
			}
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
			/* mark a future need to fixup encrypted inode */
			if (fi
			    && ctx->efs_raw
			    && (ni->flags & FILE_ATTR_ENCRYPTED))
				fi->fh |= CLOSE_ENCRYPTED;
#endif /* HAVE_SETXATTR */
			/* mark a need to update the mtime */
			if (fi && ctx->dmtime)
				fi->fh |= CLOSE_DMTIME;
			NInoSetDirty(ni);
			/*
			 * closing ni requires access to dir_ni to
			 * synchronize the index, avoid double opening.
			 */
			if (ntfs_inode_close_in_dir(ni, dir_ni))
				set_fuse_error(&res);
			ntfs_fuse_update_times(dir_ni, NTFS_UPDATE_MCTIME);
		} else
			res = -errno;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	} else
		res = -errno;
#endif
	free(path);

exit:
	free(uname);
	if (ntfs_inode_close(dir_ni))
		set_fuse_error(&res);
	if (utarget)
		free(utarget);
	free(dir_path);
	return res;
}

static int ntfs_fuse_create_stream(const char *path,
		ntfschar *stream_name, const int stream_name_len,
		struct fuse_file_info *fi)
{
	ntfs_inode *ni;
	int res = 0;

	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		if (res == -ENOENT) {
			/*
			 * If such file does not exist, create it and try once
			 * again to add stream to it.
			 * Note : no fuse_file_info for creation of main file
			 */
			res = ntfs_fuse_create(path, S_IFREG, 0, NULL,
					(struct fuse_file_info*)NULL);
			if (!res)
				return ntfs_fuse_create_stream(path,
						stream_name, stream_name_len,fi);
			else
				res = -errno;
		}
		return res;
	}
	if (ntfs_attr_add(ni, AT_DATA, stream_name, stream_name_len, NULL, 0))
		res = -errno;
	else
		set_archive(ni);

	if ((res >= 0)
	    && fi
	    && (fi->flags & (O_WRONLY | O_RDWR))) {
		/* mark a future need to compress the last block */
		if (ni->flags & FILE_ATTR_COMPRESSED)
			fi->fh |= CLOSE_COMPRESSED;
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
		/* mark a future need to fixup encrypted inode */
		if (ctx->efs_raw
		    && (ni->flags & FILE_ATTR_ENCRYPTED))
			fi->fh |= CLOSE_ENCRYPTED;
#endif /* HAVE_SETXATTR */
		if (ctx->dmtime)
			fi->fh |= CLOSE_DMTIME;
	}

	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

static int ntfs_fuse_mknod_common(const char *org_path, mode_t mode, dev_t dev,
				struct fuse_file_info *fi)
{
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;
	int res = 0;

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	if (stream_name_len
	    && (!S_ISREG(mode)
		|| (ctx->windows_names
		    && ntfs_forbidden_names(ctx->vol,stream_name,
					stream_name_len, TRUE)))) {
		res = -EINVAL;
		goto exit;
	}
	if (!stream_name_len)
		res = ntfs_fuse_create(path, mode & (S_IFMT | 07777), dev, 
					NULL,fi);
	else
		res = ntfs_fuse_create_stream(path, stream_name,
				stream_name_len,fi);
exit:
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_mknod(const char *path, mode_t mode, dev_t dev)
{
	return ntfs_fuse_mknod_common(path, mode, dev,
			(struct fuse_file_info*)NULL);
}

static int ntfs_fuse_create_file(const char *path, mode_t mode,
			    struct fuse_file_info *fi)
{
	return ntfs_fuse_mknod_common(path, mode, 0, fi);
}

static int ntfs_fuse_symlink(const char *to, const char *from)
{
	if (ntfs_fuse_is_named_data_stream(from))
		return -EINVAL; /* n/a for named data streams. */
	return ntfs_fuse_create(from, S_IFLNK, 0, to,
			(struct fuse_file_info*)NULL);
}

static int ntfs_fuse_link(const char *old_path, const char *new_path)
{
	char *name;
	ntfschar *uname = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	char *path;
	int res = 0, uname_len;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	BOOL samedir;
	struct SECURITY_CONTEXT security;
#endif

	if (ntfs_fuse_is_named_data_stream(old_path))
		return -EINVAL; /* n/a for named data streams. */
	if (ntfs_fuse_is_named_data_stream(new_path))
		return -EINVAL; /* n/a for named data streams. */
	path = strdup(new_path);
	if (!path)
		return -errno;
	/* Open file for which create hard link. */
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, old_path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	
	/* Generate unicode filename. */
	name = strrchr(path, '/');
	name++;
	uname_len = ntfs_mbstoucs(name, &uname);
	if ((uname_len < 0)
	    || (ctx->windows_names
		&& ntfs_forbidden_names(ctx->vol,uname,uname_len,TRUE))) {
		res = -errno;
		goto exit;
	}
	/* Open parent directory. */
	*--name = 0;
	dir_ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!dir_ni) {
		res = -errno;
		goto exit;
	}

#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	samedir = !strncmp(old_path, path, strlen(path))
			&& (old_path[strlen(path)] == '/');
		/* JPA make sure the parent directories are writeable */
	if (ntfs_fuse_fill_security_context(&security)
	   && ((!samedir && !ntfs_allowed_dir_access(&security,old_path,
			(ntfs_inode*)NULL,ni,S_IWRITE + S_IEXEC))
	      || !ntfs_allowed_access(&security,dir_ni,S_IWRITE + S_IEXEC)))
		res = -EACCES;
	else
#endif
	{
		if (dir_ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
			const plugin_operations_t *ops;
			REPARSE_POINT *reparse;

			res = CALL_REPARSE_PLUGIN(dir_ni, link,
					ni, uname, uname_len);
#else /* DISABLE_PLUGINS */
			errno = EOPNOTSUPP;
			res = -errno;
#endif /* DISABLE_PLUGINS */
			if (res)
				goto exit;
		} else
			if (ntfs_link(ni, dir_ni, uname, uname_len)) {
					res = -errno;
				goto exit;
			}
	
		set_archive(ni);
		ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
		ntfs_fuse_update_times(dir_ni, NTFS_UPDATE_MCTIME);
	}
exit:
	/* 
	 * Must close dir_ni first otherwise ntfs_inode_sync_file_name(ni)
	 * may fail because ni may not be in parent's index on the disk yet.
	 */
	if (ntfs_inode_close(dir_ni))
		set_fuse_error(&res);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	free(uname);
	free(path);
	return res;
}

static int ntfs_fuse_rm(const char *org_path)
{
	char *name;
	ntfschar *uname = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	char *path;
	int res = 0, uname_len;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	struct SECURITY_CONTEXT security;
#endif

	path = strdup(org_path);
	if (!path)
		return -errno;
	/* Open object for delete. */
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	/* deny unlinking metadata files */
	if (ni->mft_no < FILE_first_user) {
		errno = EPERM;
		res = -errno;
		goto exit;
	}

	/* Generate unicode filename. */
	name = strrchr(path, '/');
	name++;
	uname_len = ntfs_mbstoucs(name, &uname);
	if (uname_len < 0) {
		res = -errno;
		goto exit;
	}
	/* Open parent directory. */
	*--name = 0;
	dir_ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
		/* deny unlinking metadata files from $Extend */
	if (!dir_ni || (dir_ni->mft_no == FILE_Extend)) {
		res = -errno;
		if (dir_ni)
			res = -EPERM;
		goto exit;
	}
	
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	/* JPA deny unlinking if directory is not writable and executable */
	if (!ntfs_fuse_fill_security_context(&security)
	    || ntfs_allowed_dir_access(&security, org_path, dir_ni, ni,
				   S_IEXEC + S_IWRITE + S_ISVTX)) {
#endif
		if (dir_ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
			const plugin_operations_t *ops;
			REPARSE_POINT *reparse;

			res = CALL_REPARSE_PLUGIN(dir_ni, unlink,
					org_path, ni, uname, uname_len);
#else /* DISABLE_PLUGINS */
			res = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
		} else
			if (ntfs_delete(ctx->vol, org_path, ni, dir_ni,
					 uname, uname_len))
				res = -errno;
		/* ntfs_delete() always closes ni and dir_ni */
		ni = dir_ni = NULL;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	} else
		res = -EACCES;
#endif
exit:
	if (ntfs_inode_close(dir_ni))
		set_fuse_error(&res);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	free(uname);
	free(path);
	return res;
}

static int ntfs_fuse_rm_stream(const char *path, ntfschar *stream_name,
		const int stream_name_len)
{
	ntfs_inode *ni;
	int res = 0;

	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	
	if (ntfs_attr_remove(ni, AT_DATA, stream_name, stream_name_len))
		res = -errno;

	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

static int ntfs_fuse_unlink(const char *org_path)
{
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;
	int res = 0;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	struct SECURITY_CONTEXT security;
#endif

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	if (!stream_name_len)
		res = ntfs_fuse_rm(path);
	else {
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
			/*
			 * JPA deny unlinking stream if directory is not
			 * writable and executable (debatable)
			 */
		if (!ntfs_fuse_fill_security_context(&security)
		   || ntfs_allowed_dir_access(&security, path,
				(ntfs_inode*)NULL, (ntfs_inode*)NULL,
				S_IEXEC + S_IWRITE + S_ISVTX))
			res = ntfs_fuse_rm_stream(path, stream_name,
					stream_name_len);
		else
			res = -errno;
#else
		res = ntfs_fuse_rm_stream(path, stream_name, stream_name_len);
#endif
	}
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_safe_rename(const char *old_path, 
				 const char *new_path, 
				 const char *tmp)
{
	int ret;

	ntfs_log_trace("Entering\n");
	
	ret = ntfs_fuse_link(new_path, tmp);
	if (ret)
		return ret;
	
	ret = ntfs_fuse_unlink(new_path);
	if (!ret) {
		
		ret = ntfs_fuse_link(old_path, new_path);
		if (ret)
			goto restore;
		
		ret = ntfs_fuse_unlink(old_path);
		if (ret) {
			if (ntfs_fuse_unlink(new_path))
				goto err;
			goto restore;
		}
	}
	
	goto cleanup;
restore:
	if (ntfs_fuse_link(tmp, new_path)) {
err:
		ntfs_log_perror("Rename failed. Existing file '%s' was renamed "
				"to '%s'", new_path, tmp);
	} else {
cleanup:
		/*
		 * Condition for this unlink has already been checked in
		 * "ntfs_fuse_rename_existing_dest()", so it should never
		 * fail (unless concurrent access to directories when fuse
		 * is multithreaded)
		 */
		if (ntfs_fuse_unlink(tmp) < 0)
			ntfs_log_perror("Rename failed. Existing file '%s' still present "
				"as '%s'", new_path, tmp);
	}
	return 	ret;
}

static int ntfs_fuse_rename_existing_dest(const char *old_path, const char *new_path)
{
	int ret, len;
	char *tmp;
	const char *ext = ".ntfs-3g-";
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	struct SECURITY_CONTEXT security;
#endif

	ntfs_log_trace("Entering\n");
	
	len = strlen(new_path) + strlen(ext) + 10 + 1; /* wc(str(2^32)) + \0 */
	tmp = ntfs_malloc(len);
	if (!tmp)
		return -errno;
	
	ret = snprintf(tmp, len, "%s%s%010d", new_path, ext, ++ntfs_sequence);
	if (ret != len - 1) {
		ntfs_log_error("snprintf failed: %d != %d\n", ret, len - 1);
		ret = -EOVERFLOW;
	} else {
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
			/*
			 * Make sure existing dest can be removed.
			 * This is only needed if parent directory is
			 * sticky, because in this situation condition
			 * for unlinking is different from condition for
			 * linking
			 */
		if (!ntfs_fuse_fill_security_context(&security)
		  || ntfs_allowed_dir_access(&security, new_path,
				(ntfs_inode*)NULL, (ntfs_inode*)NULL,
				S_IEXEC + S_IWRITE + S_ISVTX))
			ret = ntfs_fuse_safe_rename(old_path, new_path, tmp);
		else
			ret = -EACCES;
#else
		ret = ntfs_fuse_safe_rename(old_path, new_path, tmp);
#endif
	}
	free(tmp);
	return 	ret;
}

static int ntfs_fuse_rename(const char *old_path, const char *new_path)
{
	int ret, stream_name_len;
	char *path = NULL;
	ntfschar *stream_name;
	ntfs_inode *ni;
	u64 inum;
	BOOL same;
	
	ntfs_log_debug("rename: old: '%s'  new: '%s'\n", old_path, new_path);
	
	/*
	 *  FIXME: Rename should be atomic.
	 */
	stream_name_len = ntfs_fuse_parse_path(new_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (ni) {
		ret = ntfs_check_empty_dir(ni);
		if (ret < 0) {
			ret = -errno;
			ntfs_inode_close(ni);
			goto out;
		}
		
		inum = ni->mft_no;
		if (ntfs_inode_close(ni)) {
			set_fuse_error(&ret);
			goto out;
		}

		free(path);
		path = (char*)NULL;
		if (stream_name_len)
			free(stream_name);

			/* silently ignore a rename to same inode */
		stream_name_len = ntfs_fuse_parse_path(old_path,
						&path, &stream_name);
		if (stream_name_len < 0)
			return stream_name_len;
	
		ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
		if (ni) {
			same = ni->mft_no == inum;
			if (ntfs_inode_close(ni))
				ret = -errno;
			else
				if (!same)
					ret = ntfs_fuse_rename_existing_dest(
							old_path, new_path);
		} else
			ret = -errno;
		goto out;
	}

	ret = ntfs_fuse_link(old_path, new_path);
	if (ret)
		goto out;
	
	ret = ntfs_fuse_unlink(old_path);
	if (ret)
		ntfs_fuse_unlink(new_path);
out:
	free(path);
	if (stream_name_len)
		free(stream_name);
	return ret;
}

static int ntfs_fuse_mkdir(const char *path,
		mode_t mode)
{
	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	return ntfs_fuse_create(path, S_IFDIR | (mode & 07777), 0, NULL,
			(struct fuse_file_info*)NULL);
}

static int ntfs_fuse_rmdir(const char *path)
{
	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	return ntfs_fuse_rm(path);
}

#ifdef HAVE_UTIMENSAT

static int ntfs_fuse_utimens(const char *path, const struct timespec tv[2])
{
	ntfs_inode *ni;
	int res = 0;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	struct SECURITY_CONTEXT security;
#endif

	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		   /* parent directory must be executable */
	if (ntfs_fuse_fill_security_context(&security)
	    && !ntfs_allowed_dir_access(&security,path,
			(ntfs_inode*)NULL,(ntfs_inode*)NULL,S_IEXEC)) {
		return (-errno);
	}
#endif
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;

			/* no check or update if both UTIME_OMIT */
	if ((tv[0].tv_nsec != UTIME_OMIT) || (tv[1].tv_nsec != UTIME_OMIT)) {
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		if (ntfs_allowed_as_owner(&security, ni)
		    || ((tv[0].tv_nsec == UTIME_NOW)
			&& (tv[1].tv_nsec == UTIME_NOW)
			&& ntfs_allowed_access(&security, ni, S_IWRITE))) {
#endif
			ntfs_time_update_flags mask = NTFS_UPDATE_CTIME;

			if (tv[0].tv_nsec == UTIME_NOW)
				mask |= NTFS_UPDATE_ATIME;
			else
				if (tv[0].tv_nsec != UTIME_OMIT)
					ni->last_access_time
						= timespec2ntfs(tv[0]);
			if (tv[1].tv_nsec == UTIME_NOW)
				mask |= NTFS_UPDATE_MTIME;
			else
				if (tv[1].tv_nsec != UTIME_OMIT)
					ni->last_data_change_time
						= timespec2ntfs(tv[1]);
			ntfs_inode_update_times(ni, mask);
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		} else
			res = -errno;
#endif
	}
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

#else /* HAVE_UTIMENSAT */

static int ntfs_fuse_utime(const char *path, struct utimbuf *buf)
{
	ntfs_inode *ni;
	int res = 0;
	struct timespec actime;
	struct timespec modtime;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	BOOL ownerok;
	BOOL writeok;
	struct SECURITY_CONTEXT security;
#endif

	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		   /* parent directory must be executable */
	if (ntfs_fuse_fill_security_context(&security)
	    && !ntfs_allowed_dir_access(&security,path,
			(ntfs_inode*)NULL,(ntfs_inode*)NULL,S_IEXEC)) {
		return (-errno);
	}
#endif
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	ownerok = ntfs_allowed_as_owner(&security, ni);
	if (buf) {
		/*
		 * fuse never calls with a NULL buf and we do not
		 * know whether the specific condition can be applied
		 * So we have to accept updating by a non-owner having
		 * write access.
		 */
		writeok = !ownerok
			&& (buf->actime == buf->modtime)
			&& ntfs_allowed_access(&security, ni, S_IWRITE);
			/* Must be owner */
		if (!ownerok && !writeok)
			res = (buf->actime == buf->modtime ? -EACCES : -EPERM);
		else {
			actime.tv_sec = buf->actime;
			actime.tv_nsec = 0;
			modtime.tv_sec = buf->modtime;
			modtime.tv_nsec = 0;
			ni->last_access_time = timespec2ntfs(actime);
			ni->last_data_change_time = timespec2ntfs(modtime);
			ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
		}
	} else {
			/* Must be owner or have write access */
		writeok = !ownerok
			&& ntfs_allowed_access(&security, ni, S_IWRITE);
		if (!ownerok && !writeok)
			res = -EACCES;
		else
			ntfs_inode_update_times(ni, NTFS_UPDATE_AMCTIME);
	}
#else
	if (buf) {
		actime.tv_sec = buf->actime;
		actime.tv_nsec = 0;
		modtime.tv_sec = buf->modtime;
		modtime.tv_nsec = 0;
		ni->last_access_time = timespec2ntfs(actime);
		ni->last_data_change_time = timespec2ntfs(modtime);
		ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
	} else
		ntfs_inode_update_times(ni, NTFS_UPDATE_AMCTIME);
#endif

	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

#endif /* HAVE_UTIMENSAT */

static int ntfs_fuse_fsync(const char *path __attribute__((unused)),
			int type __attribute__((unused)),
			struct fuse_file_info *fi __attribute__((unused)))
{
	int ret;

		/* sync the full device */
	ret = ntfs_device_sync(ctx->vol->dev);
	if (ret)
		ret = -errno;
	return (ret);
}

#if defined(FUSE_INTERNAL) || (FUSE_VERSION >= 28)
static int ntfs_fuse_ioctl(const char *path,
			int cmd, void *arg,
			struct fuse_file_info *fi __attribute__((unused)),
			unsigned int flags, void *data)
{
	ntfs_inode *ni;
	int ret;

	if (flags & FUSE_IOCTL_COMPAT)
		return -ENOSYS;

	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;

	/*
	 * Linux defines the request argument of ioctl() to be an
	 * unsigned long, which fuse 2.x forwards as a signed int into
	 * which the request sometimes does not fit.
	 * So we must expand the value and make sure it is not sign-extended.
	 */
	ret = ntfs_ioctl(ni, (unsigned int)cmd, arg, flags, data);

	if (ntfs_inode_close (ni))
		set_fuse_error(&ret);
	return ret;
}
#endif /* defined(FUSE_INTERNAL) || (FUSE_VERSION >= 28) */

static int ntfs_fuse_bmap(const char *path, size_t blocksize, uint64_t *idx)
{
	ntfs_inode *ni;
	ntfs_attr *na;
	LCN lcn;
	int ret = 0; 
	int cl_per_bl = ctx->vol->cluster_size / blocksize;

	if (blocksize > ctx->vol->cluster_size)
		return -EINVAL;
	
	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL;
	
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;

	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		ret = -errno;
		goto close_inode;
	}
	
	if ((na->data_flags & (ATTR_COMPRESSION_MASK | ATTR_IS_ENCRYPTED))
			 || !NAttrNonResident(na)) {
		ret = -EINVAL;
		goto close_attr;
	}
	
	if (ntfs_attr_map_whole_runlist(na)) {
		ret = -errno;
		goto close_attr;
	}
	
	lcn = ntfs_rl_vcn_to_lcn(na->rl, *idx / cl_per_bl);
	*idx = (lcn > 0) ? lcn * cl_per_bl + *idx % cl_per_bl : 0;
	
close_attr:
	ntfs_attr_close(na);
close_inode:
	if (ntfs_inode_close(ni))
		set_fuse_error(&ret);
	return ret;
}

#ifdef HAVE_SETXATTR

/*
 *                Name space identifications and prefixes
 */

enum {
	XATTRNS_NONE,
	XATTRNS_USER,
	XATTRNS_SYSTEM,
	XATTRNS_SECURITY,
	XATTRNS_TRUSTED,
	XATTRNS_OPEN
} ;

/*
 *		Check whether access to internal data as an extended
 *	attribute in system name space is allowed
 *
 *	Returns pointer to inode if allowed,
 *		NULL and errno set if not allowed
 */

static ntfs_inode *ntfs_check_access_xattr(struct SECURITY_CONTEXT *security,
			const char *path, int attr, BOOL setting)
{
	ntfs_inode *ni;
	BOOL foracl;
	mode_t acctype;

	ni = (ntfs_inode*)NULL;
	if (ntfs_fuse_is_named_data_stream(path))
		errno = EINVAL; /* n/a for named data streams. */
	else {
		foracl = (attr == XATTR_POSIX_ACC)
			 || (attr == XATTR_POSIX_DEF);
		/*
		 * When accessing Posix ACL, return unsupported if ACL
		 * were disabled or no user mapping has been defined,
		 * or trying to change a Windows-inherited ACL.
		 * However no error will be returned to getfacl
		 */
		if (((!ntfs_fuse_fill_security_context(security)
			|| (ctx->secure_flags
			    & ((1 << SECURITY_DEFAULT) | (1 << SECURITY_RAW))))
			|| !(ctx->secure_flags & (1 << SECURITY_ACL))
			|| (setting && ctx->inherit))
		    && foracl) {
			if (ctx->silent && !ctx->security.mapping[MAPUSERS])
				errno = 0;
			else
				errno = EOPNOTSUPP;
		} else {
				/*
				 * parent directory must be executable, and
				 * for setting a DOS name it must be writeable
				 */
			if (setting && (attr == XATTR_NTFS_DOS_NAME))
				acctype = S_IEXEC | S_IWRITE;
			else
				acctype = S_IEXEC;
			if ((attr == XATTR_NTFS_DOS_NAME)
			    && !strcmp(path,"/"))
				/* forbid getting/setting names on root */
				errno = EPERM;
			else
				if (ntfs_allowed_real_dir_access(security, path,
					   (ntfs_inode*)NULL ,acctype)) {
					ni = ntfs_pathname_to_inode(ctx->vol,
							NULL, path);
			}
		}
	}
	return (ni);
}

/*
 *		Determine the name space of an extended attribute
 */

static int xattr_namespace(const char *name)
{
	int namespace;

	if (ctx->streams == NF_STREAMS_INTERFACE_XATTR) {
		namespace = XATTRNS_NONE;
		if (!strncmp(name, nf_ns_user_prefix, 
			nf_ns_user_prefix_len)
		    && (strlen(name) != (size_t)nf_ns_user_prefix_len))
			namespace = XATTRNS_USER;
		else if (!strncmp(name, nf_ns_system_prefix, 
			nf_ns_system_prefix_len)
		    && (strlen(name) != (size_t)nf_ns_system_prefix_len))
			namespace = XATTRNS_SYSTEM;
		else if (!strncmp(name, nf_ns_security_prefix, 
			nf_ns_security_prefix_len)
		    && (strlen(name) != (size_t)nf_ns_security_prefix_len))
			namespace = XATTRNS_SECURITY;
		else if (!strncmp(name, nf_ns_trusted_prefix, 
			nf_ns_trusted_prefix_len)
		    && (strlen(name) != (size_t)nf_ns_trusted_prefix_len))
			namespace = XATTRNS_TRUSTED;
	} else
		namespace = XATTRNS_OPEN;
	return (namespace);
}

/*
 *		Fix the prefix of an extended attribute
 */

static int fix_xattr_prefix(const char *name, int namespace, ntfschar **lename)
{
	int len;
	char *prefixed;

	*lename = (ntfschar*)NULL;
	switch (namespace) {
	case XATTRNS_USER :
		/*
		 * user name space : remove user prefix
		 */
		len = ntfs_mbstoucs(name + nf_ns_user_prefix_len, lename);
		break;
	case XATTRNS_SYSTEM :
	case XATTRNS_SECURITY :
	case XATTRNS_TRUSTED :
		/*
		 * security, trusted and unmapped system name spaces :
		 * insert ntfs-3g prefix
		 */
		prefixed = ntfs_malloc(strlen(xattr_ntfs_3g)
			 + strlen(name) + 1);
		if (prefixed) {
			strcpy(prefixed,xattr_ntfs_3g);
			strcat(prefixed,name);
			len = ntfs_mbstoucs(prefixed, lename);
			free(prefixed);
		} else
			len = -1;
		break;
	case XATTRNS_OPEN :
		/*
		 * in open name space mode : do no fix prefix
		 */
		len = ntfs_mbstoucs(name, lename);
		break;
	default :
		len = -1;
	}
	return (len);
}

static int ntfs_fuse_listxattr(const char *path, char *list, size_t size)
{
	ntfs_attr_search_ctx *actx = NULL;
	ntfs_inode *ni;
	int ret = 0;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	struct SECURITY_CONTEXT security;
#endif
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		   /* parent directory must be executable */
	if (ntfs_fuse_fill_security_context(&security)
	    && !ntfs_allowed_dir_access(&security,path,(ntfs_inode*)NULL,
			(ntfs_inode*)NULL,S_IEXEC)) {
		return (-errno);
	}
#endif
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
		/* Return with no result for symlinks, fifo, etc. */
	if (!user_xattrs_allowed(ctx, ni))
		goto exit;
		/* otherwise file must be readable */
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	if (!ntfs_allowed_access(&security,ni,S_IREAD)) {
		ret = -EACCES;
		goto exit;
	}
#endif
	actx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!actx) {
		ret = -errno;
		goto exit;
	}

	if ((ctx->streams == NF_STREAMS_INTERFACE_XATTR)
	    || (ctx->streams == NF_STREAMS_INTERFACE_OPENXATTR)) {
		ret = ntfs_fuse_listxattr_common(ni, actx, list, size,
				ctx->streams == NF_STREAMS_INTERFACE_XATTR);
		if (ret < 0)
			goto exit;
	}
	if (errno != ENOENT)
		ret = -errno;
exit:
	if (actx)
		ntfs_attr_put_search_ctx(actx);
	if (ntfs_inode_close(ni))
		set_fuse_error(&ret);
	return ret;
}

static int ntfs_fuse_getxattr_windows(const char *path, const char *name,
				char *value, size_t size)
{
	ntfs_attr_search_ctx *actx = NULL;
	ntfs_inode *ni;
	char *to = value;
	int ret = 0;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	struct SECURITY_CONTEXT security;
#endif

	if (strcmp(name, "ntfs.streams.list"))
		return -EOPNOTSUPP;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		   /* parent directory must be executable */
	if (ntfs_fuse_fill_security_context(&security)
	    && !ntfs_allowed_dir_access(&security,path,(ntfs_inode*)NULL,
			(ntfs_inode*)NULL,S_IEXEC)) {
		return (-errno);
	}
#endif
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	if (!ntfs_allowed_access(&security,ni,S_IREAD)) {
		ret = -errno;
		goto exit;
	}
#endif
	actx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!actx) {
		ret = -errno;
		goto exit;
	}
	while (!ntfs_attr_lookup(AT_DATA, NULL, 0, CASE_SENSITIVE,
				0, NULL, 0, actx)) {
		char *tmp_name = NULL;
		int tmp_name_len;

		if (!actx->attr->name_length)
			continue;
		tmp_name_len = ntfs_ucstombs((ntfschar *)((u8*)actx->attr +
				le16_to_cpu(actx->attr->name_offset)),
				actx->attr->name_length, &tmp_name, 0);
		if (tmp_name_len < 0) {
			ret = -errno;
			goto exit;
		}
		if (ret)
			ret++; /* For space delimiter. */
		ret += tmp_name_len;
		if (size) {
			if ((size_t)ret <= size) {
				/* Don't add space to the beginning of line. */
				if (to != value) {
					*to = '\0';
					to++;
				}
				strncpy(to, tmp_name, tmp_name_len);
				to += tmp_name_len;
			} else {
				free(tmp_name);
				ret = -ERANGE;
				goto exit;
			}
		}
		free(tmp_name);
	}
	if (errno != ENOENT)
		ret = -errno;
exit:
	if (actx)
		ntfs_attr_put_search_ctx(actx);
	if (ntfs_inode_close(ni))
		set_fuse_error(&ret);
	return ret;
}

#if defined(__APPLE__) || defined(__DARWIN__)
static int ntfs_fuse_getxattr(const char *path, const char *name,
				char *value, size_t size, uint32_t position)
#else
static int ntfs_fuse_getxattr(const char *path, const char *name,
				char *value, size_t size)
#endif
{
#if !(defined(__APPLE__) || defined(__DARWIN__))
	static const unsigned int position = 0U;
#endif

	ntfs_inode *ni;
	ntfs_inode *dir_ni;
	ntfs_attr *na = NULL;
	ntfschar *lename = NULL;
	int res, lename_len;
	s64 rsize;
	enum SYSTEMXATTRS attr;
	int namespace;
	struct SECURITY_CONTEXT security;

#if defined(__APPLE__) || defined(__DARWIN__)
	/* If the attribute is not a resource fork attribute and the position
	 * parameter is non-zero, we return with EINVAL as requesting position
	 * is not permitted for non-resource fork attributes. */
	if (position && strcmp(name, XATTR_RESOURCEFORK_NAME)) {
		return -EINVAL;
	}
#endif

	attr = ntfs_xattr_system_type(name,ctx->vol);
	if (attr != XATTR_UNMAPPED) {
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
			/*
			 * hijack internal data and ACL retrieval, whatever
			 * mode was selected for xattr (from the user's
			 * point of view, ACLs are not xattr)
			 */
		ni = ntfs_check_access_xattr(&security, path, attr, FALSE);
		if (ni) {
			if (ntfs_allowed_access(&security,ni,S_IREAD)) {
				if (attr == XATTR_NTFS_DOS_NAME)
					dir_ni = get_parent_dir(path);
				else
					dir_ni = (ntfs_inode*)NULL;
				res = ntfs_xattr_system_getxattr(&security,
					attr, ni, dir_ni, value, size);
				if (dir_ni && ntfs_inode_close(dir_ni))
					set_fuse_error(&res);
			} else {
				res = -errno;
                        }
			if (ntfs_inode_close(ni))
				set_fuse_error(&res);
		} else
			res = -errno;
#else
			/*
			 * Only hijack NTFS ACL retrieval if POSIX ACLS
			 * option is not selected
			 * Access control is done by fuse
			 */
		if (ntfs_fuse_is_named_data_stream(path))
			res = -EINVAL; /* n/a for named data streams. */
		else {
			ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
			if (ni) {
					/* user mapping not mandatory */
				ntfs_fuse_fill_security_context(&security);
				if (attr == XATTR_NTFS_DOS_NAME)
					dir_ni = get_parent_dir(path);
				else
					dir_ni = (ntfs_inode*)NULL;
				res = ntfs_xattr_system_getxattr(&security,
					attr, ni, dir_ni, value, size);
				if (dir_ni && ntfs_inode_close(dir_ni))
					set_fuse_error(&res);
				if (ntfs_inode_close(ni))
					set_fuse_error(&res);
			} else
				res = -errno;
		}
#endif
		return (res);
	}
	if (ctx->streams == NF_STREAMS_INTERFACE_WINDOWS)
		return ntfs_fuse_getxattr_windows(path, name, value, size);
	if (ctx->streams == NF_STREAMS_INTERFACE_NONE)
		return -EOPNOTSUPP;
	namespace = xattr_namespace(name);
	if (namespace == XATTRNS_NONE)
		return -EOPNOTSUPP;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		   /* parent directory must be executable */
	if (ntfs_fuse_fill_security_context(&security)
	    && !ntfs_allowed_dir_access(&security,path,(ntfs_inode*)NULL,
			(ntfs_inode*)NULL,S_IEXEC)) {
		return (-errno);
	}
		/* trusted only readable by root */
	if ((namespace == XATTRNS_TRUSTED)
	    && security.uid)
		    return -NTFS_NOXATTR_ERRNO;
#endif
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
		/* Return with no result for symlinks, fifo, etc. */
	if (!user_xattrs_allowed(ctx, ni)) {
		res = -NTFS_NOXATTR_ERRNO;
		goto exit;
	}
		/* otherwise file must be readable */
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	if (!ntfs_allowed_access(&security, ni, S_IREAD)) {
		res = -errno;
		goto exit;
	}
#endif
	lename_len = fix_xattr_prefix(name, namespace, &lename);
	if (lename_len == -1) {
		res = -errno;
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, lename, lename_len);
	if (!na) {
		res = -NTFS_NOXATTR_ERRNO;
		goto exit;
	}
	rsize = na->data_size;
	if (ctx->efs_raw
	    && rsize
	    && (na->data_flags & ATTR_IS_ENCRYPTED)
	    && NAttrNonResident(na))
		rsize = ((na->data_size + 511) & ~511) + 2;
	rsize -= position;
	if (size) {
		if (size >= (size_t)rsize) {
			res = ntfs_attr_pread(na, position, rsize, value);
			if (res != rsize)
				res = -errno;
		} else
			res = -ERANGE;
	} else
		res = rsize;
exit:
	if (na)
		ntfs_attr_close(na);
	free(lename);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

#if defined(__APPLE__) || defined(__DARWIN__)
static int ntfs_fuse_setxattr(const char *path, const char *name,
				const char *value, size_t size, int flags,
				uint32_t position)
#else
static int ntfs_fuse_setxattr(const char *path, const char *name,
				const char *value, size_t size, int flags)
#endif
{
#if !(defined(__APPLE__) || defined(__DARWIN__))
	static const unsigned int position = 0U;
#else
	BOOL is_resource_fork;
#endif

	ntfs_inode *ni;
	ntfs_inode *dir_ni;
	ntfs_attr *na = NULL;
	ntfschar *lename = NULL;
	int res, lename_len;
	size_t total;
	s64 part;
	enum SYSTEMXATTRS attr;
	int namespace;
	struct SECURITY_CONTEXT security;

#if defined(__APPLE__) || defined(__DARWIN__)
	/* If the attribute is not a resource fork attribute and the position
	 * parameter is non-zero, we return with EINVAL as requesting position
	 * is not permitted for non-resource fork attributes. */
	is_resource_fork = strcmp(name, XATTR_RESOURCEFORK_NAME) ? FALSE : TRUE;
	if (position && !is_resource_fork) {
		return -EINVAL;
	}
#endif

	attr = ntfs_xattr_system_type(name,ctx->vol);
	if (attr != XATTR_UNMAPPED) {
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
			/*
			 * hijack internal data and ACL setting, whatever
			 * mode was selected for xattr (from the user's
			 * point of view, ACLs are not xattr)
			 * Note : ctime updated on successful settings
			 */
		ni = ntfs_check_access_xattr(&security,path,attr,TRUE);
		if (ni) {
			if (ntfs_allowed_as_owner(&security,ni)) {
				if (attr == XATTR_NTFS_DOS_NAME)
					dir_ni = get_parent_dir(path);
				else
					dir_ni = (ntfs_inode*)NULL;
				res = ntfs_xattr_system_setxattr(&security,
					attr, ni, dir_ni, value, size, flags);
				/* never have to close dir_ni */
				if (res)
					res = -errno;
			} else
				res = -errno;
			if (attr != XATTR_NTFS_DOS_NAME) {
				if (!res)
					ntfs_fuse_update_times(ni,
							NTFS_UPDATE_CTIME);
				if (ntfs_inode_close(ni))
					set_fuse_error(&res);
			}
		} else
			res = -errno;
#else
			/*
			 * Only hijack NTFS ACL setting if POSIX ACLS
			 * option is not selected
			 * Access control is partially done by fuse
			 */
		if (ntfs_fuse_is_named_data_stream(path))
			res = -EINVAL; /* n/a for named data streams. */
		else {
			/* creation of a new name is not controlled by fuse */
			if (attr == XATTR_NTFS_DOS_NAME)
				ni = ntfs_check_access_xattr(&security,path,attr,TRUE);
			else
				ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
			if (ni) {
					/*
					 * user mapping is not mandatory
					 * if defined, only owner is allowed
					 */
				if (!ntfs_fuse_fill_security_context(&security)
				   || ntfs_allowed_as_owner(&security,ni)) {
					if (attr == XATTR_NTFS_DOS_NAME)
						dir_ni = get_parent_dir(path);
					else
						dir_ni = (ntfs_inode*)NULL;
					res = ntfs_xattr_system_setxattr(&security,
						attr, ni, dir_ni, value,
						size, flags);
					/* never have to close dir_ni */
					if (res)
						res = -errno;
				} else
					res = -errno;
				if (attr != XATTR_NTFS_DOS_NAME) {
					if (!res)
						ntfs_fuse_update_times(ni,
							NTFS_UPDATE_CTIME);
					if (ntfs_inode_close(ni))
						set_fuse_error(&res);
				}
			} else
				res = -errno;
		}
#endif
		return (res);
	}
	if ((ctx->streams != NF_STREAMS_INTERFACE_XATTR)
	    && (ctx->streams != NF_STREAMS_INTERFACE_OPENXATTR))
		return -EOPNOTSUPP;
	namespace = xattr_namespace(name);
	if (namespace == XATTRNS_NONE)
		return -EOPNOTSUPP;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		   /* parent directory must be executable */
	if (ntfs_fuse_fill_security_context(&security)
	    && !ntfs_allowed_dir_access(&security,path,(ntfs_inode*)NULL,
			(ntfs_inode*)NULL,S_IEXEC)) {
		return (-errno);
	}
		/* security and trusted only settable by root */
	if (((namespace == XATTRNS_SECURITY)
	   || (namespace == XATTRNS_TRUSTED))
		&& security.uid)
		    return -EPERM;
#endif
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	switch (namespace) {
	case XATTRNS_SECURITY :
	case XATTRNS_TRUSTED :
		if (security.uid) {
			res = -EPERM;
			goto exit;
		}
		break;
	case XATTRNS_SYSTEM :
		if (!ntfs_allowed_as_owner(&security,ni)) {
			res = -EACCES;
			goto exit;
		}
		break;
	default :
		/* User xattr not allowed for symlinks, fifo, etc. */
		if (!user_xattrs_allowed(ctx, ni)) {
			res = -EPERM;
			goto exit;
		}
		if (!ntfs_allowed_access(&security,ni,S_IWRITE)) {
			res = -EACCES;
			goto exit;
		}
		break;
	}
#else
		/* User xattr not allowed for symlinks, fifo, etc. */
	if ((namespace == XATTRNS_USER)
	    && !user_xattrs_allowed(ctx, ni)) {
		res = -EPERM;
		goto exit;
	}
#endif
	lename_len = fix_xattr_prefix(name, namespace, &lename);
	if ((lename_len == -1)
	    || (ctx->windows_names
		&& ntfs_forbidden_chars(lename,lename_len,TRUE))) {
		res = -errno;
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, lename, lename_len);
	if (na && flags == XATTR_CREATE) {
		res = -EEXIST;
		goto exit;
	}
	if (!na) {
		if (flags == XATTR_REPLACE) {
			res = -NTFS_NOXATTR_ERRNO;
			goto exit;
		}
		if (ntfs_attr_add(ni, AT_DATA, lename, lename_len, NULL, 0)) {
			res = -errno;
			goto exit;
		}
		if (!(ni->flags & FILE_ATTR_ARCHIVE)) {
			set_archive(ni);
			NInoFileNameSetDirty(ni);
		}
		na = ntfs_attr_open(ni, AT_DATA, lename, lename_len);
		if (!na) {
			res = -errno;
			goto exit;
		}
#if defined(__APPLE__) || defined(__DARWIN__)
	} else if (is_resource_fork) {
		/* In macOS, the resource fork is a special case. It doesn't
		 * ever shrink (it would have to be removed and re-added). */
#endif
	} else {
			/* currently compressed streams can only be wiped out */
		if (ntfs_attr_truncate(na, (s64)0 /* size */)) {
			res = -errno;
			goto exit;
		}
	}
	total = 0;
	res = 0;
	if (size) {
		do {
			part = ntfs_attr_pwrite(na, position + total,
					 size - total, &value[total]);
			if (part > 0)
				total += part;
		} while ((part > 0) && (total < size));
	}
	if ((total != size) || ntfs_attr_pclose(na))
		res = -errno;
	else {
		if (ctx->efs_raw 
		   && (ni->flags & FILE_ATTR_ENCRYPTED)) {
			if (ntfs_efs_fixup_attribute(NULL,na))
				res = -errno;
		}
	}
	if (!res) {
		ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
		if (!(ni->flags & FILE_ATTR_ARCHIVE)) {
			set_archive(ni);
			NInoFileNameSetDirty(ni);
		}
	}
exit:
	if (na)
		ntfs_attr_close(na);
	free(lename);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

static int ntfs_fuse_removexattr(const char *path, const char *name)
{
	ntfs_inode *ni;
	ntfs_inode *dir_ni;
	ntfschar *lename = NULL;
	int res = 0, lename_len;
	enum SYSTEMXATTRS attr;
	int namespace;
	struct SECURITY_CONTEXT security;

	attr = ntfs_xattr_system_type(name,ctx->vol);
	if (attr != XATTR_UNMAPPED) {
		switch (attr) {
			/*
			 * Removal of NTFS ACL, ATTRIB, EFSINFO or TIMES
			 * is never allowed
			 */
		case XATTR_NTFS_ACL :
		case XATTR_NTFS_ATTRIB :
		case XATTR_NTFS_ATTRIB_BE :
		case XATTR_NTFS_EFSINFO :
		case XATTR_NTFS_TIMES :
		case XATTR_NTFS_TIMES_BE :
		case XATTR_NTFS_CRTIME :
		case XATTR_NTFS_CRTIME_BE :
			res = -EPERM;
			break;
		default :
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
			/*
			 * hijack internal data and ACL removal, whatever
			 * mode was selected for xattr (from the user's
			 * point of view, ACLs are not xattr)
			 * Note : ctime updated on successful settings
			 */
			ni = ntfs_check_access_xattr(&security,path,attr,TRUE);
			if (ni) {
				if (ntfs_allowed_as_owner(&security,ni)) {
					if (attr == XATTR_NTFS_DOS_NAME)
						dir_ni = get_parent_dir(path);
					else
						dir_ni = (ntfs_inode*)NULL;
					res = ntfs_xattr_system_removexattr(&security,
						attr, ni, dir_ni);
					/* never have to close dir_ni */
					if (res)
						res = -errno;
				} else
					res = -errno;
				if (attr != XATTR_NTFS_DOS_NAME) {
					if (!res)
						ntfs_fuse_update_times(ni,
							NTFS_UPDATE_CTIME);
					if (ntfs_inode_close(ni))
						set_fuse_error(&res);
				}
			} else
				res = -errno;
#else
			/*
			 * Only hijack NTFS ACL setting if POSIX ACLS
			 * option is not selected
			 * Access control is partially done by fuse
			 */
			/* creation of a new name is not controlled by fuse */
			if (attr == XATTR_NTFS_DOS_NAME)
				ni = ntfs_check_access_xattr(&security,
							path, attr, TRUE);
			else {
				if (ntfs_fuse_is_named_data_stream(path)) {
					ni = (ntfs_inode*)NULL;
					errno = EINVAL; /* n/a for named data streams. */
				} else
					ni = ntfs_pathname_to_inode(ctx->vol,
							NULL, path);
			}
			if (ni) {
				/*
				 * user mapping is not mandatory
				 * if defined, only owner is allowed
				 */
				if (!ntfs_fuse_fill_security_context(&security)
				   || ntfs_allowed_as_owner(&security,ni)) {
					if (attr == XATTR_NTFS_DOS_NAME)
						dir_ni = get_parent_dir(path);
					else
						dir_ni = (ntfs_inode*)NULL;
					res = ntfs_xattr_system_removexattr(&security,
						attr, ni, dir_ni);
					/* never have to close dir_ni */
					if (res)
						res = -errno;
				} else
					res = -errno;
				if (attr != XATTR_NTFS_DOS_NAME) {
					if (!res)
						ntfs_fuse_update_times(ni,
							NTFS_UPDATE_CTIME);
					if (ntfs_inode_close(ni))
						set_fuse_error(&res);
				}
			} else
				res = -errno;
#endif
			break;
		}
		return (res);
	}
	if ((ctx->streams != NF_STREAMS_INTERFACE_XATTR)
	    && (ctx->streams != NF_STREAMS_INTERFACE_OPENXATTR))
		return -EOPNOTSUPP;
	namespace = xattr_namespace(name);
	if (namespace == XATTRNS_NONE)
		return -EOPNOTSUPP;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		   /* parent directory must be executable */
	if (ntfs_fuse_fill_security_context(&security)
	    && !ntfs_allowed_dir_access(&security,path,(ntfs_inode*)NULL,
			(ntfs_inode*)NULL,S_IEXEC)) {
		return (-errno);
	}
		/* security and trusted only settable by root */
	if (((namespace == XATTRNS_SECURITY)
	   || (namespace == XATTRNS_TRUSTED))
		&& security.uid)
		    return -EACCES;
#endif
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	switch (namespace) {
	case XATTRNS_SECURITY :
	case XATTRNS_TRUSTED :
		if (security.uid) {
			res = -EPERM;
			goto exit;
		}
		break;
	case XATTRNS_SYSTEM :
		if (!ntfs_allowed_as_owner(&security,ni)) {
			res = -EACCES;
			goto exit;
		}
		break;
	default :
		/* User xattr not allowed for symlinks, fifo, etc. */
		if (!user_xattrs_allowed(ctx, ni)) {
			res = -EPERM;
			goto exit;
		}
		if (!ntfs_allowed_access(&security,ni,S_IWRITE)) {
			res = -EACCES;
			goto exit;
		}
		break;
	}
#else
		/* User xattr not allowed for symlinks, fifo, etc. */
	if ((namespace == XATTRNS_USER)
	    && !user_xattrs_allowed(ctx, ni)) {
		res = -EPERM;
		goto exit;
	}
#endif
	lename_len = fix_xattr_prefix(name, namespace, &lename);
	if (lename_len == -1) {
		res = -errno;
		goto exit;
	}
	if (ntfs_attr_remove(ni, AT_DATA, lename, lename_len)) {
		if (errno == ENOENT)
			errno = NTFS_NOXATTR_ERRNO;
		res = -errno;
	}
	if (!res) {
		ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
		if (!(ni->flags & FILE_ATTR_ARCHIVE)) {
			set_archive(ni);
			NInoFileNameSetDirty(ni);
		}
	}
exit:
	free(lename);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

#else
#if POSIXACLS
#error "Option inconsistency : POSIXACLS requires SETXATTR"
#endif
#endif /* HAVE_SETXATTR */

#ifndef DISABLE_PLUGINS
static void register_internal_reparse_plugins(void)
{
	static const plugin_operations_t ops = {
		.getattr = junction_getattr,
		.readlink = junction_readlink,
	} ;
	static const plugin_operations_t wsl_ops = {
		.getattr = wsl_getattr,
	} ;

	register_reparse_plugin(ctx, IO_REPARSE_TAG_MOUNT_POINT,
					&ops, (void*)NULL);
	register_reparse_plugin(ctx, IO_REPARSE_TAG_SYMLINK,
					&ops, (void*)NULL);
	register_reparse_plugin(ctx, IO_REPARSE_TAG_LX_SYMLINK,
					&ops, (void*)NULL);
	register_reparse_plugin(ctx, IO_REPARSE_TAG_LX_SYMLINK,
					&ops, (void*)NULL);
	register_reparse_plugin(ctx, IO_REPARSE_TAG_AF_UNIX,
					&wsl_ops, (void*)NULL);
	register_reparse_plugin(ctx, IO_REPARSE_TAG_LX_FIFO,
					&wsl_ops, (void*)NULL);
	register_reparse_plugin(ctx, IO_REPARSE_TAG_LX_CHR,
					&wsl_ops, (void*)NULL);
	register_reparse_plugin(ctx, IO_REPARSE_TAG_LX_BLK,
					&wsl_ops, (void*)NULL);
}
#endif /* DISABLE_PLUGINS */

static void ntfs_close(void)
{
	struct SECURITY_CONTEXT security;

	if (!ctx)
		return;
	
	if (!ctx->vol)
		return;
	
	if (ctx->mounted) {
		ntfs_log_info("Unmounting %s (%s)\n", opts.device, 
			      ctx->vol->vol_name);
		if (ntfs_fuse_fill_security_context(&security)) {
			if (ctx->seccache && ctx->seccache->head.p_reads) {
				ntfs_log_info("Permissions cache : %lu writes, "
				"%lu reads, %lu.%1lu%% hits\n",
			      ctx->seccache->head.p_writes,
			      ctx->seccache->head.p_reads,
			      100 * ctx->seccache->head.p_hits
			         / ctx->seccache->head.p_reads,
			      1000 * ctx->seccache->head.p_hits
			         / ctx->seccache->head.p_reads % 10);
			}
		}
		ntfs_destroy_security_context(&security);
	}
	
	if (ntfs_umount(ctx->vol, FALSE))
		ntfs_log_perror("Failed to close volume %s", opts.device);
	
	ctx->vol = NULL;
}

static void ntfs_fuse_destroy2(void *unused __attribute__((unused)))
{
	ntfs_close();
}

static struct fuse_operations ntfs_3g_ops = {
	.getattr	= ntfs_fuse_getattr,
	.readlink	= ntfs_fuse_readlink,
	.readdir	= ntfs_fuse_readdir,
	.open		= ntfs_fuse_open,
	.release	= ntfs_fuse_release,
	.read		= ntfs_fuse_read,
	.write		= ntfs_fuse_write,
	.truncate	= ntfs_fuse_truncate,
	.ftruncate	= ntfs_fuse_ftruncate,
	.statfs		= ntfs_fuse_statfs,
	.chmod		= ntfs_fuse_chmod,
	.chown		= ntfs_fuse_chown,
	.create		= ntfs_fuse_create_file,
	.mknod		= ntfs_fuse_mknod,
	.symlink	= ntfs_fuse_symlink,
	.link		= ntfs_fuse_link,
	.unlink		= ntfs_fuse_unlink,
	.rename		= ntfs_fuse_rename,
	.mkdir		= ntfs_fuse_mkdir,
	.rmdir		= ntfs_fuse_rmdir,
#ifdef HAVE_UTIMENSAT
	.utimens	= ntfs_fuse_utimens,
#if defined(linux) & !defined(FUSE_INTERNAL) & (FUSE_VERSION < 30)
	.flag_utime_omit_ok = 1,
#endif /* defined(linux) & !defined(FUSE_INTERNAL) */
#else
	.utime		= ntfs_fuse_utime,
#endif
	.fsync		= ntfs_fuse_fsync,
	.fsyncdir	= ntfs_fuse_fsync,
	.bmap		= ntfs_fuse_bmap,
	.destroy        = ntfs_fuse_destroy2,
#if defined(FUSE_INTERNAL) || (FUSE_VERSION >= 28)
        .ioctl		= ntfs_fuse_ioctl,
#endif /* defined(FUSE_INTERNAL) || (FUSE_VERSION >= 28) */
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	.access		= ntfs_fuse_access,
	.opendir	= ntfs_fuse_opendir,
	.releasedir	= ntfs_fuse_release,
#endif
#ifdef HAVE_SETXATTR
	.getxattr	= ntfs_fuse_getxattr,
	.setxattr	= ntfs_fuse_setxattr,
	.removexattr	= ntfs_fuse_removexattr,
	.listxattr	= ntfs_fuse_listxattr,
#endif /* HAVE_SETXATTR */
#if defined(__APPLE__) || defined(__DARWIN__)
	/* MacFUSE extensions. */
	.getxtimes	= ntfs_macfuse_getxtimes,
	.setcrtime	= ntfs_macfuse_setcrtime,
	.setbkuptime	= ntfs_macfuse_setbkuptime,
	.setchgtime	= ntfs_macfuse_setchgtime,
#endif /* defined(__APPLE__) || defined(__DARWIN__) */
	.init		= ntfs_init
};

static int ntfs_fuse_init(void)
{
	ctx = ntfs_calloc(sizeof(ntfs_fuse_context_t));
	if (!ctx)
		return -1;
	
	*ctx = (ntfs_fuse_context_t) {
		.uid     = getuid(),
		.gid     = getgid(),
#if defined(linux)			
		.streams = NF_STREAMS_INTERFACE_XATTR,
#else			
		.streams = NF_STREAMS_INTERFACE_NONE,
#endif			
		.atime   = ATIME_RELATIVE,
		.silent  = TRUE,
		.recover = TRUE
	};
	return 0;
}

static int ntfs_open(const char *device)
{
	unsigned long flags = 0;
	
	if (!ctx->blkdev)
		flags |= NTFS_MNT_EXCLUSIVE;
	if (ctx->ro)
		flags |= NTFS_MNT_RDONLY;
	else
		if (!ctx->hiberfile)
			flags |= NTFS_MNT_MAY_RDONLY;
	if (ctx->recover)
		flags |= NTFS_MNT_RECOVER;
	if (ctx->hiberfile)
		flags |= NTFS_MNT_IGNORE_HIBERFILE;

	ctx->vol = ntfs_mount(device, flags);
	if (!ctx->vol) {
		ntfs_log_perror("Failed to mount '%s'", device);
		goto err_out;
	}
	if (ctx->sync && ctx->vol->dev)
		NDevSetSync(ctx->vol->dev);
	if (ctx->compression)
		NVolSetCompression(ctx->vol);
	else
		NVolClearCompression(ctx->vol);
#ifdef HAVE_SETXATTR
			/* archivers must see hidden files */
	if (ctx->efs_raw)
		ctx->hide_hid_files = FALSE;
#endif
	if (ntfs_set_shown_files(ctx->vol, ctx->show_sys_files,
				!ctx->hide_hid_files, ctx->hide_dot_files))
		goto err_out;
	
	if (ntfs_volume_get_free_space(ctx->vol)) {
		ntfs_log_perror("Failed to read NTFS $Bitmap");
		goto err_out;
	}

	ctx->vol->free_mft_records = ntfs_get_nr_free_mft_records(ctx->vol);
	if (ctx->vol->free_mft_records < 0) {
		ntfs_log_perror("Failed to calculate free MFT records");
		goto err_out;
	}

	if (ctx->hiberfile && ntfs_volume_check_hiberfile(ctx->vol, 0)) {
		if (errno != EPERM)
			goto err_out;
		if (ntfs_fuse_rm("/hiberfil.sys"))
			goto err_out;
	}
	
	errno = 0;
	goto out;
err_out:
	if (!errno)
		errno = EIO;
out :
	return ntfs_volume_error(errno);
}

static void usage(void)
{
	ntfs_log_info(usage_msg, EXEC_NAME, VERSION, FUSE_TYPE, fuse_version(),
			4 + POSIXACLS*6 - KERNELPERMS*3 + CACHEING,
			EXEC_NAME, ntfs_home);
}

#if defined(linux) || defined(__uClinux__)

static const char *dev_fuse_msg =
"HINT: You should be root, or make ntfs-3g setuid root, or load the FUSE\n"
"      kernel module as root ('modprobe fuse' or 'insmod <path_to>/fuse.ko'"
"      or insmod <path_to>/fuse.o'). Make also sure that the fuse device"
"      exists. It's usually either /dev/fuse or /dev/misc/fuse.";

static const char *fuse26_kmod_msg =
"WARNING: Deficient Linux kernel detected. Some driver features are\n"
"         not available (swap file on NTFS, boot from NTFS by LILO), and\n"
"         unmount is not safe unless it's made sure the ntfs-3g process\n"
"         naturally terminates after calling 'umount'. If you wish this\n"
"         message to disappear then you should upgrade to at least kernel\n"
"         version 2.6.20, or request help from your distribution to fix\n"
"         the kernel problem. The below web page has more information:\n"
"         https://github.com/tuxera/ntfs-3g/wiki/NTFS-3G-FAQ\n"
"\n";

static void mknod_dev_fuse(const char *dev)
{
	struct stat st;
	
	if (stat(dev, &st) && (errno == ENOENT)) {
		mode_t mask = umask(0); 
		if (mknod(dev, S_IFCHR | 0666, makedev(10, 229))) {
			ntfs_log_perror("Failed to create '%s'", dev);
			if (errno == EPERM)
				ntfs_log_error("%s", dev_fuse_msg);
		}
		umask(mask);
	}
}

static void create_dev_fuse(void)
{
	mknod_dev_fuse("/dev/fuse");

#ifdef __UCLIBC__
	{
		struct stat st;
		/* The fuse device is under /dev/misc using devfs. */
		if (stat("/dev/misc", &st) && (errno == ENOENT)) {
			mode_t mask = umask(0); 
			mkdir("/dev/misc", 0775);
			umask(mask);
		}
		mknod_dev_fuse("/dev/misc/fuse");
	}
#endif
}

static fuse_fstype get_fuse_fstype(void)
{
	char buf[256];
	fuse_fstype fstype = FSTYPE_NONE;
	
	FILE *f = fopen("/proc/filesystems", "r");
	if (!f) {
		ntfs_log_perror("Failed to open /proc/filesystems");
		return FSTYPE_UNKNOWN;
	}
	
	while (fgets(buf, sizeof(buf), f)) {
		if (strstr(buf, "fuseblk\n")) {
			fstype = FSTYPE_FUSEBLK;
			break;
		}
		if (strstr(buf, "fuse\n"))
			fstype = FSTYPE_FUSE;
	}
	
	fclose(f);
	return fstype;
}

static fuse_fstype load_fuse_module(void)
{
	int i;
	struct stat st;
	pid_t pid;
	const char *cmd = "/sbin/modprobe";
	char *env = (char*)NULL;
	struct timespec req = { 0, 100000000 };   /* 100 msec */
	fuse_fstype fstype;
	
	if (!stat(cmd, &st) && !geteuid()) {
		pid = fork();
		if (!pid) {
			execle(cmd, cmd, "fuse", (char*)NULL, &env);
			_exit(1);
		} else if (pid != -1)
			waitpid(pid, NULL, 0);
	}
	
	for (i = 0; i < 10; i++) {
		/* 
		 * We sleep first because despite the detection of the loaded
		 * FUSE kernel module, fuse_mount() can still fail if it's not 
		 * fully functional/initialized. Note, of course this is still
		 * unreliable but usually helps.
		 */  
		nanosleep(&req, NULL);
		fstype = get_fuse_fstype();
		if (fstype != FSTYPE_NONE)
			break;
	}
	return fstype;
}

#endif

static struct fuse_chan *try_fuse_mount(char *parsed_options)
{
	struct fuse_chan *fc = NULL;
	struct fuse_args margs = FUSE_ARGS_INIT(0, NULL);
	
	/* The fuse_mount() options get modified, so we always rebuild it */
	if ((fuse_opt_add_arg(&margs, EXEC_NAME) == -1 ||
	     fuse_opt_add_arg(&margs, "-o") == -1 ||
	     fuse_opt_add_arg(&margs, parsed_options) == -1)) {
		ntfs_log_error("Failed to set FUSE options.\n");
		goto free_args;
	}
	
	fc = fuse_mount(opts.mnt_point, &margs);
free_args:
	fuse_opt_free_args(&margs);
	return fc;
		
}
		
static int set_fuseblk_options(char **parsed_options)
{
	char options[64];
	long pagesize; 
	u32 blksize = ctx->vol->cluster_size;
	
	pagesize = sysconf(_SC_PAGESIZE);
	if (pagesize < 1)
		pagesize = 4096;
	
	if (blksize > (u32)pagesize)
		blksize = pagesize;
	
	snprintf(options, sizeof(options), ",blkdev,blksize=%u", blksize);
	if (ntfs_strappend(parsed_options, options))
		return -1;
	return 0;
}

static struct fuse *mount_fuse(char *parsed_options)
{
	struct fuse *fh = NULL;
	struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
	
	ctx->fc = try_fuse_mount(parsed_options);
	if (!ctx->fc)
		return NULL;
	
	if (fuse_opt_add_arg(&args, "") == -1)
		goto err;
	if (ctx->ro) {
		char buf[128];
		int len;
        
		len = snprintf(buf, sizeof(buf), "-ouse_ino,kernel_cache"
				",attr_timeout=%d,entry_timeout=%d",
				(int)TIMEOUT_RO, (int)TIMEOUT_RO);
		if ((len < 0)
		    || (len >= (int)sizeof(buf))
		    || (fuse_opt_add_arg(&args, buf) == -1))
			goto err;
	} else {
#if !CACHEING
		if (fuse_opt_add_arg(&args, "-ouse_ino,kernel_cache"
				",attr_timeout=0") == -1)
			goto err;
#else
		if (fuse_opt_add_arg(&args, "-ouse_ino,kernel_cache"
				",attr_timeout=1") == -1)
			goto err;
#endif
	}
	if (ctx->debug)
		if (fuse_opt_add_arg(&args, "-odebug") == -1)
			goto err;
	
	fh = fuse_new(ctx->fc, &args , &ntfs_3g_ops, sizeof(ntfs_3g_ops), NULL);
	if (!fh)
		goto err;
	
	if (fuse_set_signal_handlers(fuse_get_session(fh)))
		goto err_destory;
out:
	fuse_opt_free_args(&args);
	return fh;
err_destory:
	fuse_destroy(fh);
	fh = NULL;
err:	
	fuse_unmount(opts.mnt_point, ctx->fc);
	goto out;
}

static void setup_logging(char *parsed_options)
{
	if (!ctx->no_detach) {
		if (daemon(0, ctx->debug))
			ntfs_log_error("Failed to daemonize.\n");
		else if (!ctx->debug) {
#ifndef DEBUG
			ntfs_log_set_handler(ntfs_log_handler_syslog);
			/* Override default libntfs identify. */
			openlog(EXEC_NAME, LOG_PID, LOG_DAEMON);
#endif
		}
	}

	ctx->seccache = (struct PERMISSIONS_CACHE*)NULL;

	ntfs_log_info("Version %s %s %d\n", VERSION, FUSE_TYPE, fuse_version());
	if (strcmp(opts.arg_device,opts.device))
		ntfs_log_info("Requested device %s canonicalized as %s\n",
				opts.arg_device,opts.device);
	ntfs_log_info("Mounted %s (%s, label \"%s\", NTFS %d.%d)\n",
			opts.device, (ctx->ro) ? "Read-Only" : "Read-Write",
			ctx->vol->vol_name, ctx->vol->major_ver,
			ctx->vol->minor_ver);
	ntfs_log_info("Cmdline options: %s\n", opts.options ? opts.options : "");
	ntfs_log_info("Mount options: %s\n", parsed_options);
}

int main(int argc, char *argv[])
{
	char *parsed_options = NULL;
	struct fuse *fh;
#if !(defined(__sun) && defined (__SVR4))
	fuse_fstype fstype = FSTYPE_UNKNOWN;
#endif
	const char *permissions_mode = (const char*)NULL;
	const char *failed_secure = (const char*)NULL;
#if defined(HAVE_SETXATTR) && defined(XATTR_MAPPINGS)
	struct XATTRMAPPING *xattr_mapping = (struct XATTRMAPPING*)NULL;
#endif /* defined(HAVE_SETXATTR) && defined(XATTR_MAPPINGS) */
	struct stat sbuf;
	unsigned long existing_mount;
	int err, fd;

	/*
	 * Make sure file descriptors 0, 1 and 2 are open, 
	 * otherwise chaos would ensue.
	 */
	do {
		fd = open("/dev/null", O_RDWR);
		if (fd > 2)
			close(fd);
	} while (fd >= 0 && fd <= 2);

#ifndef FUSE_INTERNAL
	if ((getuid() != geteuid()) || (getgid() != getegid())) {
		fprintf(stderr, "%s", setuid_msg);
		return NTFS_VOLUME_INSECURE;
	}
#endif
	if (drop_privs())
		return NTFS_VOLUME_NO_PRIVILEGE;
	
	ntfs_set_locale();
	ntfs_log_set_handler(ntfs_log_handler_stderr);

	if (ntfs_parse_options(&opts, usage, argc, argv)) {
		usage();
		return NTFS_VOLUME_SYNTAX_ERROR;
	}

	if (ntfs_fuse_init()) {
		err = NTFS_VOLUME_OUT_OF_MEMORY;
		goto err2;
	}
	
	parsed_options = parse_mount_options(ctx, &opts, FALSE);
	if (!parsed_options) {
		err = NTFS_VOLUME_SYNTAX_ERROR;
		goto err_out;
	}
	if (!ntfs_check_if_mounted(opts.device,&existing_mount)
	    && (existing_mount & NTFS_MF_MOUNTED)
		/* accept multiple read-only mounts */
	    && (!(existing_mount & NTFS_MF_READONLY) || !ctx->ro)) {
		err = NTFS_VOLUME_LOCKED;
		goto err_out;
	}

			/* need absolute mount point for junctions */
	if (opts.mnt_point[0] == '/')
		ctx->abs_mnt_point = strdup(opts.mnt_point);
	else {
		ctx->abs_mnt_point = (char*)ntfs_malloc(PATH_MAX);
		if (ctx->abs_mnt_point) {
			if ((strlen(opts.mnt_point) < PATH_MAX)
			    && getcwd(ctx->abs_mnt_point,
				     PATH_MAX - strlen(opts.mnt_point) - 1)) {
				strcat(ctx->abs_mnt_point, "/");
				strcat(ctx->abs_mnt_point, opts.mnt_point);
#if defined(__sun) && defined (__SVR4)
			/* Solaris also wants the absolute mount point */
				opts.mnt_point = ctx->abs_mnt_point;
#endif /* defined(__sun) && defined (__SVR4) */
			} else {
				free(ctx->abs_mnt_point);
				ctx->abs_mnt_point = (char*)NULL;
			}
		}
	}
	if (!ctx->abs_mnt_point) {
		err = NTFS_VOLUME_OUT_OF_MEMORY;
		goto err_out;
	}

	ctx->security.uid = 0;
	ctx->security.gid = 0;
	if ((opts.mnt_point[0] == '/')
	   && !stat(opts.mnt_point,&sbuf)) {
		/* collect owner of mount point, useful for default mapping */
		ctx->security.uid = sbuf.st_uid;
		ctx->security.gid = sbuf.st_gid;
	}

#if defined(linux) || defined(__uClinux__)
	fstype = get_fuse_fstype();

	err = NTFS_VOLUME_NO_PRIVILEGE;
	if (restore_privs())
		goto err_out;

	if (fstype == FSTYPE_NONE || fstype == FSTYPE_UNKNOWN)
		fstype = load_fuse_module();
	create_dev_fuse();

	if (drop_privs())
		goto err_out;
#endif	
	if (stat(opts.device, &sbuf)) {
		ntfs_log_perror("Failed to access '%s'", opts.device);
		err = NTFS_VOLUME_NO_PRIVILEGE;
		goto err_out;
	}

#if !(defined(__sun) && defined (__SVR4))
	/* Always use fuseblk for block devices unless it's surely missing. */
	if (S_ISBLK(sbuf.st_mode) && (fstype != FSTYPE_FUSE))
		ctx->blkdev = TRUE;
#endif

#ifndef FUSE_INTERNAL
	if (getuid() && ctx->blkdev) {
		ntfs_log_error("%s", unpriv_fuseblk_msg);
		err = NTFS_VOLUME_NO_PRIVILEGE;
		goto err2;
	}
#endif
	err = ntfs_open(opts.device);
	if (err)
		goto err_out;
	
	/* Force read-only mount if the device was found read-only */
	if (!ctx->ro && NVolReadOnly(ctx->vol)) {
		ctx->rw = FALSE;
		ctx->ro = TRUE;
		if (ntfs_strinsert(&parsed_options, ",ro")) 
                	goto err_out;
		ntfs_log_info("Could not mount read-write, trying read-only\n");
	} else
		if (ctx->rw && ntfs_strinsert(&parsed_options, ",rw"))
			goto err_out;
	/* We must do this after ntfs_open() to be able to set the blksize */
	if (ctx->blkdev && set_fuseblk_options(&parsed_options))
		goto err_out;

	ctx->vol->abs_mnt_point = ctx->abs_mnt_point;
	ctx->security.vol = ctx->vol;
	ctx->vol->secure_flags = ctx->secure_flags;
	ctx->vol->special_files = ctx->special_files;
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
	ctx->vol->efs_raw = ctx->efs_raw;
#endif /* HAVE_SETXATTR */
	if (!ntfs_build_mapping(&ctx->security,ctx->usermap_path,
		(ctx->vol->secure_flags
			& ((1 << SECURITY_DEFAULT) | (1 << SECURITY_ACL)))
		&& !ctx->inherit
		&& !(ctx->vol->secure_flags & (1 << SECURITY_WANTED)))) {
#if POSIXACLS
		/* use basic permissions if requested */
		if (ctx->vol->secure_flags & (1 << SECURITY_DEFAULT))
			permissions_mode = "User mapping built, Posix ACLs not used";
		else {
			permissions_mode = "User mapping built, Posix ACLs in use";
#if KERNELACLS
			if (ntfs_strinsert(&parsed_options, ",default_permissions,acl")) {
				err = NTFS_VOLUME_SYNTAX_ERROR;
				goto err_out;
			}
#endif /* KERNELACLS */
		}
#else /* POSIXACLS */
#if KERNELPERMS
		if (!(ctx->vol->secure_flags
			& ((1 << SECURITY_DEFAULT) | (1 << SECURITY_ACL)))) {
			/*
			 * No explicit option but user mapping found
			 * force default security
			 */
			ctx->vol->secure_flags |= (1 << SECURITY_DEFAULT);
			if (ntfs_strinsert(&parsed_options, ",default_permissions")) {
				err = NTFS_VOLUME_SYNTAX_ERROR;
				goto err_out;
			}
		}
#endif /* KERNELPERMS */
		permissions_mode = "User mapping built";
#endif /* POSIXACLS */
		ctx->dmask = ctx->fmask = 0;
	} else {
		ctx->security.uid = ctx->uid;
		ctx->security.gid = ctx->gid;
		/* same ownership/permissions for all files */
		ctx->security.mapping[MAPUSERS] = (struct MAPPING*)NULL;
		ctx->security.mapping[MAPGROUPS] = (struct MAPPING*)NULL;
		if ((ctx->vol->secure_flags & (1 << SECURITY_WANTED))
		   && !(ctx->vol->secure_flags & (1 << SECURITY_DEFAULT))) {
			ctx->vol->secure_flags |= (1 << SECURITY_DEFAULT);
			if (ntfs_strinsert(&parsed_options, ",default_permissions")) {
				err = NTFS_VOLUME_SYNTAX_ERROR;
				goto err_out;
			}
		}
		if (ctx->vol->secure_flags & (1 << SECURITY_DEFAULT)) {
			ctx->vol->secure_flags |= (1 << SECURITY_RAW);
			permissions_mode = "Global ownership and permissions enforced";
		} else {
			ctx->vol->secure_flags &= ~(1 << SECURITY_RAW);
			permissions_mode = "Ownership and permissions disabled";
		}
	}
	if (ctx->usermap_path)
		free (ctx->usermap_path);

#if defined(HAVE_SETXATTR) && defined(XATTR_MAPPINGS)
	xattr_mapping = ntfs_xattr_build_mapping(ctx->vol,
				ctx->xattrmap_path);
	ctx->vol->xattr_mapping = xattr_mapping;
	/*
	 * Errors are logged, do not refuse mounting, it would be
	 * too difficult to fix the unmountable mapping file.
	 */
	if (ctx->xattrmap_path)
		free(ctx->xattrmap_path);
#endif /* defined(HAVE_SETXATTR) && defined(XATTR_MAPPINGS) */

#ifndef DISABLE_PLUGINS
	register_internal_reparse_plugins();
#endif /* DISABLE_PLUGINS */

	fh = mount_fuse(parsed_options);
	if (!fh) {
		err = NTFS_VOLUME_FUSE_ERROR;
		goto err_out;
	}
	
	ctx->mounted = TRUE;

#if defined(linux) || defined(__uClinux__)
	if (S_ISBLK(sbuf.st_mode) && (fstype == FSTYPE_FUSE))
		ntfs_log_info("%s", fuse26_kmod_msg);
#endif	
	setup_logging(parsed_options);
	if (failed_secure)
	        ntfs_log_info("%s\n",failed_secure);
	if (permissions_mode)
	        ntfs_log_info("%s, configuration type %d\n",permissions_mode,
			4 + POSIXACLS*6 - KERNELPERMS*3 + CACHEING);
	if ((ctx->vol->secure_flags & (1 << SECURITY_RAW))
	    && !ctx->uid && ctx->gid)
		ntfs_log_error("Warning : using problematic uid==0 and gid!=0\n");
	
	fuse_loop(fh);
	
	err = 0;

	fuse_unmount(opts.mnt_point, ctx->fc);
	fuse_destroy(fh);
err_out:
	ntfs_mount_error(opts.device, opts.mnt_point, err);
	if (ctx->abs_mnt_point)
		free(ctx->abs_mnt_point);
#if defined(HAVE_SETXATTR) && defined(XATTR_MAPPINGS)
	ntfs_xattr_free_mapping(xattr_mapping);
#endif /* defined(HAVE_SETXATTR) && defined(XATTR_MAPPINGS) */
err2:
	ntfs_close();
#ifndef DISABLE_PLUGINS
	close_reparse_plugins(ctx);
#endif /* DISABLE_PLUGINS */
	free(ctx);
	free(parsed_options);
	free(opts.options);
	free(opts.device);
	return err;
}
