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
#include <fuse_lowlevel.h>

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
#include "bitmap.h"
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
 *	the LPERMSCONFIG value in param.h
 */

/*	ACLS may be checked by kernel (requires a fuse patch) or here */
#define KERNELACLS ((LPERMSCONFIG > 6) & (LPERMSCONFIG < 10))
/*	basic permissions may be checked by kernel or here */
#define KERNELPERMS (((LPERMSCONFIG - 1) % 6) < 3)
/*	may want to use fuse/kernel cacheing */
#define CACHEING (!(LPERMSCONFIG % 3))

#if KERNELACLS & !KERNELPERMS
#error "Incompatible options KERNELACLS and KERNELPERMS"
#endif

#if !CACHEING
#define ATTR_TIMEOUT (ctx->ro ? TIMEOUT_RO : 0.0)
#define ENTRY_TIMEOUT (ctx->ro ? TIMEOUT_RO : 0.0)
#else
#if defined(__sun) && defined (__SVR4)
#define ATTR_TIMEOUT (ctx->ro ? TIMEOUT_RO : 10.0)
#define ENTRY_TIMEOUT (ctx->ro ? TIMEOUT_RO : 10.0)
#else /* defined(__sun) && defined (__SVR4) */
	/*
	 * FUSE cacheing is only usable with basic permissions
	 * checked by the kernel with external fuse >= 2.8
	 */
#if !KERNELPERMS
#warning "Fuse cacheing is only usable with basic permissions checked by kernel"
#endif
#if KERNELACLS
#define ATTR_TIMEOUT (ctx->ro ? TIMEOUT_RO : 10.0)
#define ENTRY_TIMEOUT (ctx->ro ? TIMEOUT_RO : 10.0)
#else /* KERNELACLS */
#define ATTR_TIMEOUT (ctx->ro ? TIMEOUT_RO : \
	(ctx->vol->secure_flags & (1 << SECURITY_DEFAULT) ? 10.0 : 0.0))
#define ENTRY_TIMEOUT (ctx->ro ? TIMEOUT_RO : \
	(ctx->vol->secure_flags & (1 << SECURITY_DEFAULT) ? 10.0 : 0.0))
#endif /* KERNELACLS */
#endif /* defined(__sun) && defined (__SVR4) */
#endif /* !CACHEING */
#define GHOSTLTH 40 /* max length of a ghost file name - see ghostformat */

		/* sometimes the kernel cannot check access */
#define ntfs_real_allowed_access(scx, ni, type) ntfs_allowed_access(scx, ni, type)
#if POSIXACLS & KERNELPERMS & !KERNELACLS
		/* short-circuit if PERMS checked by kernel and ACLs by fs */
#define ntfs_allowed_access(scx, ni, type) \
	((scx)->vol->secure_flags & (1 << SECURITY_DEFAULT) \
	    ? 1 : ntfs_allowed_access(scx, ni, type))
#endif

#define set_archive(ni) (ni)->flags |= FILE_ATTR_ARCHIVE
#define INODE(ino) ((ino) == 1 ? (MFT_REF)FILE_root : (MFT_REF)(ino))

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

typedef struct fill_item {
	struct fill_item *next;
	size_t bufsize;
	size_t off;
	char buf[0];
} ntfs_fuse_fill_item_t;

typedef struct fill_context {
	struct fill_item *first;
	struct fill_item *last;
#ifndef DISABLE_PLUGINS
	u64 fh;
#endif /* DISABLE_PLUGINS */
	off_t off;
	fuse_req_t req;
	fuse_ino_t ino;
	BOOL filled;
} ntfs_fuse_fill_context_t;

struct open_file {
	struct open_file *next;
	struct open_file *previous;
	long long ghost;
	fuse_ino_t ino;
	fuse_ino_t parent;
	int state;
#ifndef DISABLE_PLUGINS
	struct fuse_file_info fi;
#endif /* DISABLE_PLUGINS */
} ;

enum {
	CLOSE_GHOST = 1,
	CLOSE_COMPRESSED = 2,
	CLOSE_ENCRYPTED = 4,
	CLOSE_DMTIME = 8,
	CLOSE_REPARSE = 16
};

enum RM_TYPES {
	RM_LINK,
	RM_DIR,
	RM_ANY,
} ;

static struct ntfs_options opts;

const char *EXEC_NAME = "lowntfs-3g";

static ntfs_fuse_context_t *ctx;
static u32 ntfs_sequence;
static const char ghostformat[] = ".ghost-ntfs-3g-%020llu";

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
"Example: lowntfs-3g /dev/sda1 /mnt/windows\n"
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
 *	Fill a security context as needed by security functions
 *	returns TRUE if there is a user mapping,
 *		FALSE if there is none
 *			This is not an error and the context is filled anyway,
 *			it is used for implicit Windows-like inheritance
 */

static BOOL ntfs_fuse_fill_security_context(fuse_req_t req,
			struct SECURITY_CONTEXT *scx)
{
	const struct fuse_ctx *fusecontext;

	scx->vol = ctx->vol;
	scx->mapping[MAPUSERS] = ctx->security.mapping[MAPUSERS];
	scx->mapping[MAPGROUPS] = ctx->security.mapping[MAPGROUPS];
	scx->pseccache = &ctx->seccache;
	if (req) {
		fusecontext = fuse_req_ctx(req);
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

	} else {
		scx->uid = 0;
		scx->gid = 0;
		scx->tid = 0;
		scx->umask = 0;
	}
	return (ctx->security.mapping[MAPUSERS] != (struct MAPPING*)NULL);
}

static u64 ntfs_fuse_inode_lookup(fuse_ino_t parent, const char *name)
{
	u64 ino = (u64)-1;
	u64 inum;
	ntfs_inode *dir_ni;

	/* Open target directory. */
	dir_ni = ntfs_inode_open(ctx->vol, INODE(parent));
	if (dir_ni) {
		/* Lookup file */
		inum = ntfs_inode_lookup_by_mbsname(dir_ni, name);
			/* never return inodes 0 and 1 */
		if (MREF(inum) <= 1) {
			inum = (u64)-1;
			errno = ENOENT;
		}
		if (ntfs_inode_close(dir_ni)
		    || (inum == (u64)-1))
			ino = (u64)-1;
		else
			ino = MREF(inum);
	}
	return (ino);
}

#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)

/*
 *		Check access to parent directory
 *
 *	file inode is only opened when not fed in and S_ISVTX is requested,
 *	when already open and S_ISVTX, it *HAS TO* be fed in.
 *
 *	returns 1 if allowed,
 *		0 if not allowed or some error occurred (errno tells why)
 */

static int ntfs_allowed_dir_access(struct SECURITY_CONTEXT *scx,
			ntfs_inode *dir_ni, fuse_ino_t ino,
			ntfs_inode *ni, mode_t accesstype)
{
	int allowed;
	ntfs_inode *ni2;
	struct stat stbuf;

	allowed = ntfs_allowed_access(scx, dir_ni, accesstype);
		/*
		 * for an not-owned sticky directory, have to
		 * check whether file itself is owned
		 */
	if ((accesstype == (S_IWRITE + S_IEXEC + S_ISVTX))
	   && (allowed == 2)) {
		if (ni)
			ni2 = ni;
		else
			ni2 = ntfs_inode_open(ctx->vol, INODE(ino));
		allowed = 0;
		if (ni2) {
			allowed = (ntfs_get_owner_mode(scx,ni2,&stbuf) >= 0)
				&& (stbuf.st_uid == scx->uid);
			if (!ni)
				ntfs_inode_close(ni2);
		}
	}
	return (allowed);
}

#endif /* !KERNELPERMS | (POSIXACLS & !KERNELACLS) */

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

static void ntfs_fuse_statfs(fuse_req_t req,
			fuse_ino_t ino __attribute__((unused)))
{
	struct statvfs sfs;
	s64 size;
	int delta_bits;
	ntfs_volume *vol;

	vol = ctx->vol;
	if (vol) {
	/* 
	 * File system block size. Used to calculate used/free space by df.
	 * Incorrectly documented as "optimal transfer block size". 
	 */
		sfs.f_bsize = vol->cluster_size;

	/* Fundamental file system block size, used as the unit. */
		sfs.f_frsize = vol->cluster_size;

	/*
	 * Total number of blocks on file system in units of f_frsize.
	 * Since inodes are also stored in blocks ($MFT is a file) hence
	 * this is the number of clusters on the volume.
	 */
		sfs.f_blocks = vol->nr_clusters;

	/* Free blocks available for all and for non-privileged processes. */
		size = vol->free_clusters;
		if (size < 0)
			size = 0;
		sfs.f_bavail = sfs.f_bfree = size;

	/* Free inodes on the free space */
		delta_bits = vol->cluster_size_bits - vol->mft_record_size_bits;
		if (delta_bits >= 0)
			size <<= delta_bits;
		else
			size >>= -delta_bits;

	/* Number of inodes at this point in time. */
		sfs.f_files = (vol->mftbmp_na->allocated_size << 3) + size;

	/* Free inodes available for all and for non-privileged processes. */
		size += vol->free_mft_records;
		if (size < 0)
			size = 0;
		sfs.f_ffree = sfs.f_favail = size;

	/* Maximum length of filenames. */
		sfs.f_namemax = NTFS_MAX_NAME_LEN;
		fuse_reply_statfs(req, &sfs);
	} else
		fuse_reply_err(req, ENODEV);

}

static void set_fuse_error(int *err)
{
	if (!*err)
		*err = -errno;
}

#if 0 && (defined(__APPLE__) || defined(__DARWIN__)) /* Unfinished. */
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
	crtime->tv_sec = ni->creation_time;
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
		ni->creation_time = tv->tv_sec;
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
	ni = ntfs_pathname_to_inode(ctx-&gt;vol, NULL, path);
	if (!ni)
		return -errno;

	if (tv) {
		ni-&gt;last_mft_change_time = tv-&gt;tv_sec;
		ntfs_fuse_update_times(ni, 0);
	}

	if (ntfs_inode_close(ni))
		set_fuse_error(&amp;res);
	return res;
}
#endif /* defined(__APPLE__) || defined(__DARWIN__) */

static void ntfs_init(void *userdata __attribute__((unused)),
			struct fuse_conn_info *conn)
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
}

#ifndef DISABLE_PLUGINS

/*
 *		Define attributes for a junction or symlink
 *		(internal plugin)
 */

static int junction_getstat(ntfs_inode *ni,
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

static int wsl_getstat(ntfs_inode *ni, const REPARSE_POINT *reparse,
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
 *		Apply permission masks to st_mode returned by reparse handler
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

static int ntfs_fuse_getstat(struct SECURITY_CONTEXT *scx,
				ntfs_inode *ni, struct stat *stbuf)
{
	int res = 0;
	ntfs_attr *na;
	BOOL withusermapping;

	memset(stbuf, 0, sizeof(struct stat));
	withusermapping = (scx->mapping[MAPUSERS] != (struct MAPPING*)NULL);
	stbuf->st_nlink = le16_to_cpu(ni->mrec->link_count);
	if (ctx->posix_nlink
	    && !(ni->flags & FILE_ATTR_REPARSE_POINT))
		stbuf->st_nlink = ntfs_dir_link_cnt(ni);
	if ((ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)
	    || (ni->flags & FILE_ATTR_REPARSE_POINT)) {
		if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
			const plugin_operations_t *ops;
			REPARSE_POINT *reparse;

			res = CALL_REPARSE_PLUGIN(ni, getattr, stbuf);
			if (!res) {
				apply_umask(stbuf);
			} else {
				stbuf->st_size = ntfs_bad_reparse_lth;
				stbuf->st_blocks =
					(ni->allocated_size + 511) >> 9;
				stbuf->st_mode = S_IFLNK;
				res = 0;
			}
			goto ok;
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
				stbuf->st_blocks =
					(ni->allocated_size + 511) >> 9;
				stbuf->st_nlink =
					le16_to_cpu(ni->mrec->link_count);
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
				na = ntfs_attr_open(ni, AT_INDEX_ALLOCATION,
						NTFS_INDEX_I30, 4);
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
		if (ni->flags & FILE_ATTR_SYSTEM) {
			na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
			if (!na) {
				stbuf->st_ino = ni->mft_no;
				goto nodata;
			}
			/* Check whether it's Interix FIFO or socket. */
			if (!(ni->flags & FILE_ATTR_HIDDEN)) {
				/* FIFO. */
				if (na->data_size == 0)
					stbuf->st_mode = S_IFIFO;
				/* Socket link. */
				if (na->data_size == 1)
					stbuf->st_mode = S_IFSOCK;
			}
			/*
			 * Check whether it's Interix symbolic link, block or
			 * character device.
			 */
			if ((u64)na->data_size <= sizeof(INTX_FILE_TYPES)
					+ sizeof(ntfschar) * PATH_MAX
				&& (u64)na->data_size >
					sizeof(INTX_FILE_TYPES)) {
				INTX_FILE *intx_file;

				intx_file =
					(INTX_FILE*)ntfs_malloc(na->data_size);
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
						na->data_size == (s64)offsetof(
						INTX_FILE, device_end)) {
					stbuf->st_mode = S_IFBLK;
					stbuf->st_rdev = makedev(le64_to_cpu(
							intx_file->major),
							le64_to_cpu(
							intx_file->minor));
				}
				if (intx_file->magic == INTX_CHARACTER_DEVICE &&
						na->data_size == (s64)offsetof(
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
		if (ntfs_get_owner_mode(scx,ni,stbuf) < 0)
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
	return (res);
}

static void ntfs_fuse_getattr(fuse_req_t req, fuse_ino_t ino,
			struct fuse_file_info *fi __attribute__((unused)))
{
	int res;
	ntfs_inode *ni;
	struct stat stbuf;
	struct SECURITY_CONTEXT security;

	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni)
		res = -errno;
	else {
		ntfs_fuse_fill_security_context(req, &security);
		res = ntfs_fuse_getstat(&security, ni, &stbuf);
		if (ntfs_inode_close(ni))
			set_fuse_error(&res);
	}
	if (!res)
		fuse_reply_attr(req, &stbuf, ATTR_TIMEOUT);
	else
		fuse_reply_err(req, -res);
}

static __inline__ BOOL ntfs_fuse_fillstat(struct SECURITY_CONTEXT *scx,
			struct fuse_entry_param *pentry, u64 iref)
{
	ntfs_inode *ni;
	BOOL ok = FALSE;

	pentry->ino = MREF(iref);
	ni = ntfs_inode_open(ctx->vol, pentry->ino);
	if (ni) {
		if (!ntfs_fuse_getstat(scx, ni, &pentry->attr)) {
			pentry->generation = 1;
			pentry->attr_timeout = ATTR_TIMEOUT;
			pentry->entry_timeout = ENTRY_TIMEOUT;
			ok = TRUE;
		}
		if (ntfs_inode_close(ni))
		       ok = FALSE;
	}
	return (ok);
}


static void ntfs_fuse_lookup(fuse_req_t req, fuse_ino_t parent,
			const char *name)
{
	struct SECURITY_CONTEXT security;
	struct fuse_entry_param entry;
	ntfs_inode *dir_ni;
	u64 iref;
	BOOL ok = FALSE;

	if (strlen(name) < 256) {
		dir_ni = ntfs_inode_open(ctx->vol, INODE(parent));
		if (dir_ni) {
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
			/*
			 * make sure the parent directory is searchable
			 */
			if (ntfs_fuse_fill_security_context(req, &security)
			    && !ntfs_allowed_access(&security,dir_ni,S_IEXEC)) {
				ntfs_inode_close(dir_ni);
				errno = EACCES;
			} else {
#else
				ntfs_fuse_fill_security_context(req, &security);
#endif
				iref = ntfs_inode_lookup_by_mbsname(dir_ni,
								name);
					/* never return inodes 0 and 1 */
				if (MREF(iref) <= 1) {
					iref = (u64)-1;
					errno = ENOENT;
				}
				ok = !ntfs_inode_close(dir_ni)
					&& (iref != (u64)-1)
					&& ntfs_fuse_fillstat(
						&security,&entry,iref);
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
			}
#endif
		}
	} else
		errno = ENAMETOOLONG;
	if (!ok)
		fuse_reply_err(req, errno);
	else
		fuse_reply_entry(req, &entry);
}

#ifndef DISABLE_PLUGINS

/*
 *		Get the link defined by a junction or symlink
 *		(internal plugin)
 */

static int junction_readlink(ntfs_inode *ni,
			const REPARSE_POINT *reparse, char **pbuf)
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

static void ntfs_fuse_readlink(fuse_req_t req, fuse_ino_t ino)
{
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	INTX_FILE *intx_file = NULL;
	char *buf = (char*)NULL;
	int res = 0;

	/* Get inode. */
	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni) {
		res = -errno;
		goto exit;
	}
		/*
		 * Reparse point : analyze as a junction point
		 */
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
		REPARSE_POINT *reparse;
		le32 tag;
		int lth;
#ifndef DISABLE_PLUGINS
		const plugin_operations_t *ops;

		res = CALL_REPARSE_PLUGIN(ni, readlink, &buf);
			/* plugin missing or reparse tag failing the check */
		if (res && ((errno == ELIBACC) || (errno == EINVAL)))
			errno = EOPNOTSUPP;
#else /* DISABLE_PLUGINS */
		errno = 0;
		res = 0;
		buf = ntfs_make_symlink(ni, ctx->abs_mnt_point);
#endif /* DISABLE_PLUGINS */
		if (!buf && (errno == EOPNOTSUPP)) {
			buf = (char*)malloc(ntfs_bad_reparse_lth + 1);
			if (buf) {
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
				if (lth != ntfs_bad_reparse_lth) {
					free(buf);
					buf = (char*)NULL;
				}
			}
		}
		if (!buf)
			res = -errno;
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
	intx_file = (INTX_FILE*)ntfs_malloc(na->data_size);
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
			&buf, 0) < 0) {
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

	if (res < 0)
		fuse_reply_err(req, -res);
	else
		fuse_reply_readlink(req, buf);
	free(buf);
}

static int ntfs_fuse_filler(ntfs_fuse_fill_context_t *fill_ctx,
		const ntfschar *name, const int name_len, const int name_type,
		const s64 pos __attribute__((unused)), const MFT_REF mref,
		const unsigned dt_type __attribute__((unused)))
{
	char *filename = NULL;
	int ret = 0;
	int filenamelen = -1;
	size_t sz;
	ntfs_fuse_fill_item_t *current;
	ntfs_fuse_fill_item_t *newone;

	if (name_type == FILE_NAME_DOS)
		return 0;
        
	if ((filenamelen = ntfs_ucstombs(name, name_len, &filename, 0)) < 0) {
		ntfs_log_perror("Filename decoding failed (inode %llu)",
				(unsigned long long)MREF(mref));
		return -1;
	}
		/* never return inodes 0 and 1 */
	if (MREF(mref) > 1) {
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
	
		current = fill_ctx->last;
		sz = fuse_add_direntry(fill_ctx->req,
				&current->buf[current->off],
				current->bufsize - current->off,
				filename, &st, current->off + fill_ctx->off);
		if (!sz || ((current->off + sz) > current->bufsize)) {
			newone = (ntfs_fuse_fill_item_t*)ntfs_malloc
				(sizeof(ntfs_fuse_fill_item_t)
				     + current->bufsize);
			if (newone) {
				newone->off = 0;
				newone->bufsize = current->bufsize;
				newone->next = (ntfs_fuse_fill_item_t*)NULL;
				current->next = newone;
				fill_ctx->last = newone;
				fill_ctx->off += current->off;
				current = newone;
				sz = fuse_add_direntry(fill_ctx->req,
					current->buf,
					current->bufsize - current->off,
					filename, &st, fill_ctx->off);
				if (!sz) {
					errno = EIO;
					ntfs_log_error("Could not add a"
						" directory entry (inode %lld)\n",
						(unsigned long long)MREF(mref));
				}
			} else {
				sz = 0;
				errno = ENOMEM;
			}
		}
		if (sz) {
			current->off += sz;
		} else {
			ret = -1;
		}
	}
        
	free(filename);
	return ret;
}

static void ntfs_fuse_opendir(fuse_req_t req, fuse_ino_t ino,
			 struct fuse_file_info *fi)
{
	int res = 0;
	ntfs_inode *ni;
	int accesstype;
	ntfs_fuse_fill_context_t *fill;
	struct SECURITY_CONTEXT security;

	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (ni) {
		if (ntfs_fuse_fill_security_context(req, &security)) {
			if (fi->flags & O_WRONLY)
				accesstype = S_IWRITE;
			else
				if (fi->flags & O_RDWR)
					accesstype = S_IWRITE | S_IREAD;
				else
					accesstype = S_IREAD;
			if (!ntfs_allowed_access(&security,ni,accesstype))
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
		if (!res) {
			fill = (ntfs_fuse_fill_context_t*)
				ntfs_malloc(sizeof(ntfs_fuse_fill_context_t));
			if (!fill)
				res = -errno;
			else {
				fill->first = fill->last
					= (ntfs_fuse_fill_item_t*)NULL;
				fill->filled = FALSE;
				fill->ino = ino;
				fill->off = 0;
#ifndef DISABLE_PLUGINS
				fill->fh = fi->fh;
#endif /* DISABLE_PLUGINS */
			}
			fi->fh = (long)fill;
		}
	} else
		res = -errno;
	if (!res)
		fuse_reply_open(req, fi);
	else
		fuse_reply_err(req, -res);
}


static void ntfs_fuse_releasedir(fuse_req_t req,
			fuse_ino_t ino __attribute__((unused)),
			struct fuse_file_info *fi)
{
#ifndef DISABLE_PLUGINS
	struct fuse_file_info ufi;
	ntfs_inode *ni;
#endif /* DISABLE_PLUGINS */
	ntfs_fuse_fill_context_t *fill;
	ntfs_fuse_fill_item_t *current;
	int res;

	res = 0;
	fill = (ntfs_fuse_fill_context_t*)(long)fi->fh;
	if (fill && (fill->ino == ino)) {
			/* make sure to clear results */
		current = fill->first;
		while (current) {
			current = current->next;
			free(fill->first);
			fill->first = current;
		}
#ifndef DISABLE_PLUGINS
		if (fill->fh) {
			const plugin_operations_t *ops;
			REPARSE_POINT *reparse;

			ni = ntfs_inode_open(ctx->vol, INODE(ino));
			if (ni) {
				if (ni->flags & FILE_ATTR_REPARSE_POINT) {
					memcpy(&ufi, fi, sizeof(ufi));
					ufi.fh = fill->fh;
					res = CALL_REPARSE_PLUGIN(ni, release,
								 &ufi);
				}
				if (ntfs_inode_close(ni) && !res)
					res = -errno;
			} else
				res = -errno;
		}
#endif /* DISABLE_PLUGINS */
		fill->ino = 0;
		free(fill);
	}
	fuse_reply_err(req, -res);
}

static void ntfs_fuse_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
			off_t off __attribute__((unused)),
			struct fuse_file_info *fi __attribute__((unused)))
{
#ifndef DISABLE_PLUGINS
	struct fuse_file_info ufi;
#endif /* DISABLE_PLUGINS */
	ntfs_fuse_fill_item_t *first;
	ntfs_fuse_fill_item_t *current;
	ntfs_fuse_fill_context_t *fill;
	ntfs_inode *ni;
	s64 pos = 0;
	int err = 0;

	fill = (ntfs_fuse_fill_context_t*)(long)fi->fh;
	if (fill && (fill->ino == ino)) {
		if (fill->filled && !off) {
			/* Rewinding : make sure to clear existing results */   
			current = fill->first;
			while (current) {
				current = current->next;
				free(fill->first);
				fill->first = current;
			}
			fill->filled = FALSE;
		}
		if (!fill->filled) {
				/* initial call : build the full list */
			current = (ntfs_fuse_fill_item_t*)NULL;
			first = (ntfs_fuse_fill_item_t*)ntfs_malloc
				(sizeof(ntfs_fuse_fill_item_t) + size);
			if (first) {
				first->bufsize = size;
				first->off = 0;
				first->next = (ntfs_fuse_fill_item_t*)NULL;
				fill->req = req;
				fill->first = first;
				fill->last = first;
				fill->off = 0;
				ni = ntfs_inode_open(ctx->vol,INODE(ino));
				if (!ni)
					err = -errno;
				else {
					if (ni->flags
						& FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
						const plugin_operations_t *ops;
						REPARSE_POINT *reparse;

						memcpy(&ufi, fi, sizeof(ufi));
						ufi.fh = fill->fh;
						err = CALL_REPARSE_PLUGIN(ni,
							readdir, &pos, fill,
							(ntfs_filldir_t)
							ntfs_fuse_filler, &ufi);
#else /* DISABLE_PLUGINS */
						err = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
					} else {
						if (ntfs_readdir(ni, &pos, fill,
							(ntfs_filldir_t)
							ntfs_fuse_filler))
							err = -errno;
					}
					fill->filled = TRUE;
					ntfs_fuse_update_times(ni,
						NTFS_UPDATE_ATIME);
					if (ntfs_inode_close(ni))
						set_fuse_error(&err);
				}
				if (!err) {
					off_t loc = 0;
				/*
				 * In some circumstances, the queue gets
				 * reinitialized by releasedir() + opendir(),
				 * apparently always on end of partial buffer.
				 * Files may be missing or duplicated.
				 */
					while (first
					    && ((loc < off) || !first->off)) {
						loc += first->off;
						fill->first = first->next;
						free(first);
						first = fill->first;
					}
					current = first;
				}
			} else
				err = -errno;
		} else {
			/* subsequent call : return next non-empty buffer */
			current = fill->first;
			while (current && !current->off) {
				current = current->next;
				free(fill->first);
				fill->first = current;
			}
		}
		if (!err) {
			if (current) {
				fuse_reply_buf(req, current->buf, current->off);
				fill->first = current->next;
				free(current);
			} else {
				fuse_reply_buf(req, (char*)NULL, 0);
				/* reply sent, now must exit with no error */
			}
		}
	} else {
		errno = EIO;
		err = -errno;
		ntfs_log_error("Uninitialized fuse_readdir()\n");
	}
	if (err)
		fuse_reply_err(req, -err);
}

static void ntfs_fuse_open(fuse_req_t req, fuse_ino_t ino,
		      struct fuse_file_info *fi)
{
	ntfs_inode *ni;
	ntfs_attr *na = NULL;
	struct open_file *of;
	int state = 0;
	int res = 0;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	int accesstype;
	struct SECURITY_CONTEXT security;
#endif

	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (ni) {
		if (!(ni->flags & FILE_ATTR_REPARSE_POINT)) {
			na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
			if (!na) {
				res = -errno;
				goto close;
			}
		}
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		if (ntfs_fuse_fill_security_context(req, &security)) {
			if (fi->flags & O_WRONLY)
				accesstype = S_IWRITE;
			else
				if (fi->flags & O_RDWR)
					accesstype = S_IWRITE | S_IREAD;
				else
					accesstype = S_IREAD;
		     /* check whether requested access is allowed */
			if (!ntfs_allowed_access(&security,
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
			if (!res && fi->fh) {
				state = CLOSE_REPARSE;
			}
#else /* DISABLE_PLUGINS */
			res = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
			goto close;
		}
		if ((res >= 0)
		    && (fi->flags & (O_WRONLY | O_RDWR))) {
		/* mark a future need to compress the last chunk */
			if (na->data_flags & ATTR_COMPRESSION_MASK)
				state |= CLOSE_COMPRESSED;
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
		/* mark a future need to fixup encrypted inode */
			if (ctx->efs_raw
			    && !(na->data_flags & ATTR_IS_ENCRYPTED)
			    && (ni->flags & FILE_ATTR_ENCRYPTED))
				state |= CLOSE_ENCRYPTED;
#endif /* HAVE_SETXATTR */
		/* mark a future need to update the mtime */
			if (ctx->dmtime)
				state |= CLOSE_DMTIME;
			/* deny opening metadata files for writing */
			if (ino < FILE_first_user)
				res = -EPERM;
		}
		ntfs_attr_close(na);
close:
		if (ntfs_inode_close(ni))
			set_fuse_error(&res);
	} else
		res = -errno;
	if (res >= 0) {
		of = (struct open_file*)malloc(sizeof(struct open_file));
		if (of) {
			of->parent = 0;
			of->ino = ino;
			of->state = state;
#ifndef DISABLE_PLUGINS
			memcpy(&of->fi, fi, sizeof(struct fuse_file_info));
#endif /* DISABLE_PLUGINS */
			of->next = ctx->open_files;
			of->previous = (struct open_file*)NULL;
			if (ctx->open_files)
				ctx->open_files->previous = of;
			ctx->open_files = of;
			fi->fh = (long)of;
		}
	}
	if (res)
		fuse_reply_err(req, -res);
	else
		fuse_reply_open(req, fi);
}

static void ntfs_fuse_read(fuse_req_t req, fuse_ino_t ino, size_t size,
			off_t offset,
			struct fuse_file_info *fi __attribute__((unused)))
{
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	int res;
	char *buf = (char*)NULL;
	s64 total = 0;
	s64 max_read;

	if (!size) {
		res = 0;
		goto exit;
	}
	buf = (char*)ntfs_malloc(size);
	if (!buf) {
		res = -errno;
		goto exit;
	}

	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni) {
		res = -errno;
		goto exit;
	}
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
		const plugin_operations_t *ops;
		REPARSE_POINT *reparse;
		struct open_file *of;

		of = (struct open_file*)(long)fi->fh;
		res = CALL_REPARSE_PLUGIN(ni, read, buf, size, offset, &of->fi);
		if (res >= 0) {
			goto stamps;
		}
#else /* DISABLE_PLUGINS */
		res = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
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
			ntfs_log_perror("ntfs_attr_pread error reading inode %lld at "
				"offset %lld: %lld <> %lld", (long long)ni->mft_no,
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
stamps :
#endif /* DISABLE_PLUGINS */
	ntfs_fuse_update_times(ni, NTFS_UPDATE_ATIME);
exit:
	if (na)
		ntfs_attr_close(na);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	if (res < 0)
		fuse_reply_err(req, -res);
	else
		fuse_reply_buf(req, buf, res);
	free(buf);
}

static void ntfs_fuse_write(fuse_req_t req, fuse_ino_t ino, const char *buf, 
			size_t size, off_t offset,
			struct fuse_file_info *fi __attribute__((unused)))
{
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	int res, total = 0;

	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni) {
		res = -errno;
		goto exit;
	}
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
		const plugin_operations_t *ops;
		REPARSE_POINT *reparse;
		struct open_file *of;

		of = (struct open_file*)(long)fi->fh;
		res = CALL_REPARSE_PLUGIN(ni, write, buf, size, offset,
								&of->fi);
		if (res >= 0) {
			goto stamps;
		}
#else /* DISABLE_PLUGINS */
		res = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
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
stamps :
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
	if (res < 0)
		fuse_reply_err(req, -res);
	else
		fuse_reply_write(req, res);
}

static int ntfs_fuse_chmod(struct SECURITY_CONTEXT *scx, fuse_ino_t ino,
		mode_t mode, struct stat *stbuf)
{
	int res = 0;
	ntfs_inode *ni;

	  /* Unsupported if inherit or no user mapping has been defined */
	if ((!scx->mapping[MAPUSERS] || ctx->inherit)
	    && !ctx->silent) {
		res = -EOPNOTSUPP;
	} else {
		ni = ntfs_inode_open(ctx->vol, INODE(ino));
		if (!ni)
			res = -errno;
		else {
			/* ignore if Windows inheritance is forced */
			if (scx->mapping[MAPUSERS] && !ctx->inherit) {
				if (ntfs_set_mode(scx, ni, mode))
					res = -errno;
				else {
					ntfs_fuse_update_times(ni,
							NTFS_UPDATE_CTIME);
					/*
					 * Must return updated times, and
					 * inode has been updated, so hope
					 * we get no further errors
					 */
					res = ntfs_fuse_getstat(scx, ni, stbuf);
				}
				NInoSetDirty(ni);
			} else
				res = ntfs_fuse_getstat(scx, ni, stbuf);
			if (ntfs_inode_close(ni))
				set_fuse_error(&res);
		}
	}
	return res;
}

static int ntfs_fuse_chown(struct SECURITY_CONTEXT *scx, fuse_ino_t ino,
			uid_t uid, gid_t gid, struct stat *stbuf)
{
	ntfs_inode *ni;
	int res;

	  /* Unsupported if inherit or no user mapping has been defined */
	if ((!scx->mapping[MAPUSERS] || ctx->inherit)
			&& !ctx->silent
			&& ((uid != ctx->uid) || (gid != ctx->gid)))
		res = -EOPNOTSUPP;
	else {
		res = 0;
		ni = ntfs_inode_open(ctx->vol, INODE(ino));
		if (!ni)
			res = -errno;
		else {
			/* ignore if Windows inheritance is forced */
			if (scx->mapping[MAPUSERS]
			  && !ctx->inherit
			  && (((int)uid != -1) || ((int)gid != -1))) {
				if (ntfs_set_owner(scx, ni, uid, gid))
					res = -errno;
				else {
					ntfs_fuse_update_times(ni,
							NTFS_UPDATE_CTIME);
				/*
				 * Must return updated times, and
				 * inode has been updated, so hope
				 * we get no further errors
				 */
					res = ntfs_fuse_getstat(scx, ni, stbuf);
				}
			} else
				res = ntfs_fuse_getstat(scx, ni, stbuf);
			if (ntfs_inode_close(ni))
				set_fuse_error(&res);
		}
	}
	return (res);
}

static int ntfs_fuse_chownmod(struct SECURITY_CONTEXT *scx, fuse_ino_t ino,
			uid_t uid, gid_t gid, mode_t mode, struct stat *stbuf)
{
	ntfs_inode *ni;
	int res;

	  /* Unsupported if inherit or no user mapping has been defined */
	if ((!scx->mapping[MAPUSERS] || ctx->inherit)
			&& !ctx->silent
			&& ((uid != ctx->uid) || (gid != ctx->gid)))
		res = -EOPNOTSUPP;
	else {
		res = 0;
		ni = ntfs_inode_open(ctx->vol, INODE(ino));
		if (!ni)
			res = -errno;
		else {
			/* ignore if Windows inheritance is forced */
			if (scx->mapping[MAPUSERS] && !ctx->inherit) {
				if (ntfs_set_ownmod(scx, ni, uid, gid, mode))
					res = -errno;
				else {
					ntfs_fuse_update_times(ni,
							NTFS_UPDATE_CTIME);
					/*
					 * Must return updated times, and
					 * inode has been updated, so hope
					 * we get no further errors
					 */
					res = ntfs_fuse_getstat(scx, ni, stbuf);
				}
			} else
				res = ntfs_fuse_getstat(scx, ni, stbuf);
			if (ntfs_inode_close(ni))
				set_fuse_error(&res);
		}
	}
	return (res);
}

static int ntfs_fuse_trunc(struct SECURITY_CONTEXT *scx, fuse_ino_t ino, 
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		off_t size, BOOL chkwrite, struct stat *stbuf)
#else
		off_t size, BOOL chkwrite __attribute__((unused)),
		struct stat *stbuf)
#endif
{
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	int res;
	s64 oldsize;

	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni)
		goto exit;

	/* deny truncating metadata files */
	if (ino < FILE_first_user) {
		errno = EPERM;
		goto exit;
	}
	if (!(ni->flags & FILE_ATTR_REPARSE_POINT)) {
		na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
		if (!na)
			goto exit;
	}
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	/*
	 * deny truncation if cannot write to file
	 * (already checked for ftruncate())
	 */
	if (scx->mapping[MAPUSERS]
	    && chkwrite
	    && !ntfs_allowed_access(scx, ni, S_IWRITE)) {
		errno = EACCES;
		goto exit;
	}
#endif
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
		const plugin_operations_t *ops;
		REPARSE_POINT *reparse;

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
		/*
		 * for compressed files, upsizing is done by inserting a final
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
stamps :
#endif /* DISABLE_PLUGINS */
	ntfs_fuse_update_times(ni, NTFS_UPDATE_MCTIME);
	res = ntfs_fuse_getstat(scx, ni, stbuf);
	errno = (res ? -res : 0);
exit:
	res = -errno;
	ntfs_attr_close(na);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

#if defined(HAVE_UTIMENSAT) & defined(FUSE_SET_ATTR_ATIME_NOW)

static int ntfs_fuse_utimens(struct SECURITY_CONTEXT *scx, fuse_ino_t ino,
		struct stat *stin, struct stat *stbuf, int to_set)
{
	ntfs_inode *ni;
	int res = 0;

	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni)
		return -errno;

			/* no check or update if both UTIME_OMIT */
	if (to_set & (FUSE_SET_ATTR_ATIME + FUSE_SET_ATTR_MTIME)) {
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		if (ntfs_allowed_as_owner(scx, ni)
		    || ((to_set & FUSE_SET_ATTR_ATIME_NOW)
			&& (to_set & FUSE_SET_ATTR_MTIME_NOW)
			&& ntfs_allowed_access(scx, ni, S_IWRITE))) {
#endif
			ntfs_time_update_flags mask = NTFS_UPDATE_CTIME;

			if (to_set & FUSE_SET_ATTR_ATIME_NOW)
				mask |= NTFS_UPDATE_ATIME;
			else
				if (to_set & FUSE_SET_ATTR_ATIME) {
#ifdef HAVE_STRUCT_STAT_ST_ATIMESPEC
					ni->last_access_time
						= timespec2ntfs(stin->st_atimespec);
#elif defined(HAVE_STRUCT_STAT_ST_ATIM)
					ni->last_access_time
						= timespec2ntfs(stin->st_atim);
#else
					ni->last_access_time.tv_sec
						= stin->st_atime;
#ifdef HAVE_STRUCT_STAT_ST_ATIMENSEC
					ni->last_access_time.tv_nsec
						= stin->st_atimensec;
#endif
#endif
				}
			if (to_set & FUSE_SET_ATTR_MTIME_NOW)
				mask |= NTFS_UPDATE_MTIME;
			else
				if (to_set & FUSE_SET_ATTR_MTIME) {
#ifdef HAVE_STRUCT_STAT_ST_ATIMESPEC
					ni->last_data_change_time
						= timespec2ntfs(stin->st_mtimespec);
#elif defined(HAVE_STRUCT_STAT_ST_ATIM)
					ni->last_data_change_time 
						= timespec2ntfs(stin->st_mtim);
#else
					ni->last_data_change_time.tv_sec
						= stin->st_mtime;
#ifdef HAVE_STRUCT_STAT_ST_ATIMENSEC
					ni->last_data_change_time.tv_nsec
						= stin->st_mtimensec;
#endif
#endif
				}
			ntfs_inode_update_times(ni, mask);
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		} else
			res = -errno;
#endif
	}
	if (!res)
		res = ntfs_fuse_getstat(scx, ni, stbuf);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

#else /* defined(HAVE_UTIMENSAT) & defined(FUSE_SET_ATTR_ATIME_NOW) */

static int ntfs_fuse_utime(struct SECURITY_CONTEXT *scx, fuse_ino_t ino,
		struct stat *stin, struct stat *stbuf)
{
	ntfs_inode *ni;
	int res = 0;
	struct timespec actime;
	struct timespec modtime;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	BOOL ownerok;
	BOOL writeok;
#endif

	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni)
		return -errno;
        
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	ownerok = ntfs_allowed_as_owner(scx, ni);
	if (stin) {
		/*
		 * fuse never calls with a NULL buf and we do not
		 * know whether the specific condition can be applied
		 * So we have to accept updating by a non-owner having
		 * write access.
		 */
		writeok = !ownerok
			&& (stin->st_atime == stin->st_mtime)
			&& ntfs_allowed_access(scx, ni, S_IWRITE);
			/* Must be owner */
		if (!ownerok && !writeok)
			res = (stin->st_atime == stin->st_mtime
					? -EACCES : -EPERM);
		else {
			actime.tv_sec = stin->st_atime;
			actime.tv_nsec = 0;
			modtime.tv_sec = stin->st_mtime;
			modtime.tv_nsec = 0;
			ni->last_access_time = timespec2ntfs(actime);
			ni->last_data_change_time = timespec2ntfs(modtime);
			ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
		}
	} else {
			/* Must be owner or have write access */
		writeok = !ownerok
			&& ntfs_allowed_access(scx, ni, S_IWRITE);
		if (!ownerok && !writeok)
			res = -EACCES;
		else
			ntfs_inode_update_times(ni, NTFS_UPDATE_AMCTIME);
	}
#else
	if (stin) {
		actime.tv_sec = stin->st_atime;
		actime.tv_nsec = 0;
		modtime.tv_sec = stin->st_mtime;
		modtime.tv_nsec = 0;
		ni->last_access_time = timespec2ntfs(actime);
		ni->last_data_change_time = timespec2ntfs(modtime);
		ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
	} else
		ntfs_inode_update_times(ni, NTFS_UPDATE_AMCTIME);
#endif

	res = ntfs_fuse_getstat(scx, ni, stbuf);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
	return res;
}

#endif /* defined(HAVE_UTIMENSAT) & defined(FUSE_SET_ATTR_ATIME_NOW) */

static void ntfs_fuse_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr,
			 int to_set, struct fuse_file_info *fi __attribute__((unused)))
{
	struct stat stbuf;
	ntfs_inode *ni;
	int res;
	struct SECURITY_CONTEXT security;

	res = 0;
	ntfs_fuse_fill_security_context(req, &security);
						/* no flags */
	if (!(to_set
		    & (FUSE_SET_ATTR_MODE
			| FUSE_SET_ATTR_UID | FUSE_SET_ATTR_GID
			| FUSE_SET_ATTR_SIZE
			| FUSE_SET_ATTR_ATIME | FUSE_SET_ATTR_MTIME))) {
		ni = ntfs_inode_open(ctx->vol, INODE(ino));
		if (!ni)
			res = -errno;
		else {
			res = ntfs_fuse_getstat(&security, ni, &stbuf);
			if (ntfs_inode_close(ni))
				set_fuse_error(&res);
		}
	}
						/* some set of uid/gid/mode */
	if (to_set
		    & (FUSE_SET_ATTR_MODE
			| FUSE_SET_ATTR_UID | FUSE_SET_ATTR_GID)) {
		switch (to_set
			    & (FUSE_SET_ATTR_MODE
				| FUSE_SET_ATTR_UID | FUSE_SET_ATTR_GID)) {
		case FUSE_SET_ATTR_MODE :
			res = ntfs_fuse_chmod(&security, ino,
						attr->st_mode & 07777, &stbuf);
			break;
		case FUSE_SET_ATTR_UID :
			res = ntfs_fuse_chown(&security, ino, attr->st_uid,
						(gid_t)-1, &stbuf);
			break;
		case FUSE_SET_ATTR_GID :
			res = ntfs_fuse_chown(&security, ino, (uid_t)-1,
						attr->st_gid, &stbuf);
			break;
		case FUSE_SET_ATTR_UID + FUSE_SET_ATTR_GID :
			res = ntfs_fuse_chown(&security, ino, attr->st_uid,
						attr->st_gid, &stbuf);
			break;
		case FUSE_SET_ATTR_UID + FUSE_SET_ATTR_MODE:
			res = ntfs_fuse_chownmod(&security, ino, attr->st_uid,
						(gid_t)-1,attr->st_mode,
						&stbuf);
			break;
		case FUSE_SET_ATTR_GID + FUSE_SET_ATTR_MODE:
			res = ntfs_fuse_chownmod(&security, ino, (uid_t)-1,
						attr->st_gid,attr->st_mode,
						&stbuf);
			break;
		case FUSE_SET_ATTR_UID + FUSE_SET_ATTR_GID + FUSE_SET_ATTR_MODE:
			res = ntfs_fuse_chownmod(&security, ino, attr->st_uid,
					attr->st_gid,attr->st_mode, &stbuf);
			break;
		default :
			break;
		}
	}
						/* size */
	if (!res && (to_set & FUSE_SET_ATTR_SIZE)) {
		res = ntfs_fuse_trunc(&security, ino, attr->st_size,
					!fi, &stbuf);
	}
						/* some set of atime/mtime */
	if (!res && (to_set & (FUSE_SET_ATTR_ATIME + FUSE_SET_ATTR_MTIME))) {
#if defined(HAVE_UTIMENSAT) & defined(FUSE_SET_ATTR_ATIME_NOW)
		res = ntfs_fuse_utimens(&security, ino, attr, &stbuf, to_set);
#else /* defined(HAVE_UTIMENSAT) & defined(FUSE_SET_ATTR_ATIME_NOW) */
		res = ntfs_fuse_utime(&security, ino, attr, &stbuf);
#endif /* defined(HAVE_UTIMENSAT) & defined(FUSE_SET_ATTR_ATIME_NOW) */
	}
	if (res)
		fuse_reply_err(req, -res);
	else
		fuse_reply_attr(req, &stbuf, ATTR_TIMEOUT);
}

#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)

static void ntfs_fuse_access(fuse_req_t req, fuse_ino_t ino, int mask)
{
	int res = 0;
	int mode;
	ntfs_inode *ni;
	struct SECURITY_CONTEXT security;

	  /* JPA return unsupported if no user mapping has been defined */
	if (!ntfs_fuse_fill_security_context(req, &security)) {
		if (ctx->silent)
			res = 0;
		else
			res = -EOPNOTSUPP;
	} else {
		ni = ntfs_inode_open(ctx->vol, INODE(ino));
		if (!ni) {
			res = -errno;
		} else {
			mode = 0;
			if (mask & (X_OK | W_OK | R_OK)) {
				if (mask & X_OK) mode += S_IEXEC;
				if (mask & W_OK) mode += S_IWRITE;
				if (mask & R_OK) mode += S_IREAD;
				if (!ntfs_allowed_access(&security,
						ni, mode))
					res = -errno;
			}
			if (ntfs_inode_close(ni))
				set_fuse_error(&res);
		}
	}
	if (res < 0)
		fuse_reply_err(req, -res);
	else
		fuse_reply_err(req, 0);
}

#endif /* !KERNELPERMS | (POSIXACLS & !KERNELACLS) */

static int ntfs_fuse_create(fuse_req_t req, fuse_ino_t parent, const char *name,
		mode_t typemode, dev_t dev,
		struct fuse_entry_param *e,
		const char *target, struct fuse_file_info *fi)
{
	ntfschar *uname = NULL, *utarget = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	struct open_file *of;
	int state = 0;
	le32 securid;
	gid_t gid;
	mode_t dsetgid;
	mode_t type = typemode & ~07777;
	mode_t perm;
	struct SECURITY_CONTEXT security;
	int res = 0, uname_len, utarget_len;

	/* Generate unicode filename. */
	uname_len = ntfs_mbstoucs(name, &uname);
	if ((uname_len < 0)
	    || (ctx->windows_names
		&& ntfs_forbidden_names(ctx->vol,uname,uname_len,TRUE))) {
		res = -errno;
		goto exit;
	}
	/* Deny creating into $Extend */
	if (parent == FILE_Extend) {
		res = -EPERM;
		goto exit;
	}
	/* Open parent directory. */
	dir_ni = ntfs_inode_open(ctx->vol, INODE(parent));
	if (!dir_ni) {
		res = -errno;
		goto exit;
	}
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		/* make sure parent directory is writeable and executable */
	if (!ntfs_fuse_fill_security_context(req, &security)
	       || ntfs_allowed_create(&security,
				dir_ni, &gid, &dsetgid)) {
#else
		ntfs_fuse_fill_security_context(req, &security);
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
			ni = (ntfs_inode*)NULL;
			errno = EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
		} else {
			switch (type) {
				case S_IFCHR:
				case S_IFBLK:
					ni = ntfs_create_device(dir_ni, securid,
							uname, uname_len,
							type, dev);
					break;
				case S_IFLNK:
					utarget_len = ntfs_mbstoucs(target,
							&utarget);
					if (utarget_len < 0) {
						res = -errno;
						goto exit;
					}
					ni = ntfs_create_symlink(dir_ni,
							securid,
							uname, uname_len,
							utarget, utarget_len);
					break;
				default:
					ni = ntfs_create(dir_ni, securid, uname,
							uname_len, type);
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
				state |= CLOSE_COMPRESSED;
			}
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
			/* mark a future need to fixup encrypted inode */
			if (fi
			    && ctx->efs_raw
			    && (ni->flags & FILE_ATTR_ENCRYPTED))
				state |= CLOSE_ENCRYPTED;
#endif /* HAVE_SETXATTR */
			if (fi && ctx->dmtime)
				state |= CLOSE_DMTIME;
			ntfs_inode_update_mbsname(dir_ni, name, ni->mft_no);
			NInoSetDirty(ni);
			e->ino = ni->mft_no;
			e->generation = 1;
			e->attr_timeout = ATTR_TIMEOUT;
			e->entry_timeout = ENTRY_TIMEOUT;
			res = ntfs_fuse_getstat(&security, ni, &e->attr);
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

exit:
	free(uname);
	if (ntfs_inode_close(dir_ni))
		set_fuse_error(&res);
	if (utarget)
		free(utarget);
	if ((res >= 0) && fi) {
		of = (struct open_file*)malloc(sizeof(struct open_file));
		if (of) {
			of->parent = 0;
			of->ino = e->ino;
			of->state = state;
			of->next = ctx->open_files;
			of->previous = (struct open_file*)NULL;
			if (ctx->open_files)
				ctx->open_files->previous = of;
			ctx->open_files = of;
			fi->fh = (long)of;
		}
	}
	return res;
}

static void ntfs_fuse_create_file(fuse_req_t req, fuse_ino_t parent,
			const char *name, mode_t mode,
			struct fuse_file_info *fi)
{
	int res;
	struct fuse_entry_param entry;

	res = ntfs_fuse_create(req, parent, name, mode & (S_IFMT | 07777),
				0, &entry, NULL, fi);
	if (res < 0)
		fuse_reply_err(req, -res);
	else
		fuse_reply_create(req, &entry, fi);
}

static void ntfs_fuse_mknod(fuse_req_t req, fuse_ino_t parent, const char *name,
		       mode_t mode, dev_t rdev)
{
	int res;
	struct fuse_entry_param e;

	res = ntfs_fuse_create(req, parent, name, mode & (S_IFMT | 07777),
				rdev, &e,NULL,(struct fuse_file_info*)NULL);
	if (res < 0)
		fuse_reply_err(req, -res);
	else
		fuse_reply_entry(req, &e);
}

static void ntfs_fuse_symlink(fuse_req_t req, const char *target,
			fuse_ino_t parent, const char *name)
{
	int res;
	struct fuse_entry_param entry;

	res = ntfs_fuse_create(req, parent, name, S_IFLNK, 0,
			&entry, target, (struct fuse_file_info*)NULL);
	if (res < 0)
		fuse_reply_err(req, -res);
	else
		fuse_reply_entry(req, &entry);
}


static int ntfs_fuse_newlink(fuse_req_t req __attribute__((unused)),
			fuse_ino_t ino, fuse_ino_t newparent,
			const char *newname, struct fuse_entry_param *e)
{
	ntfschar *uname = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	int res = 0, uname_len;
	struct SECURITY_CONTEXT security;

	/* Open file for which create hard link. */
	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni) {
		res = -errno;
		goto exit;
	}
        
	/* Do not accept linking to a directory (except for renaming) */
	if (e && (ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)) {
		errno = EPERM;
		res = -errno;
		goto exit;
	}
	/* Generate unicode filename. */
	uname_len = ntfs_mbstoucs(newname, &uname);
	if ((uname_len < 0)
            || (ctx->windows_names
                && ntfs_forbidden_names(ctx->vol,uname,uname_len,TRUE))) {
		res = -errno;
		goto exit;
	}
	/* Open parent directory. */
	dir_ni = ntfs_inode_open(ctx->vol, INODE(newparent));
	if (!dir_ni) {
		res = -errno;
		goto exit;
	}

#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		/* make sure the target parent directory is writeable */
	if (ntfs_fuse_fill_security_context(req, &security)
	    && !ntfs_allowed_access(&security,dir_ni,S_IWRITE + S_IEXEC))
		res = -EACCES;
	else
#else
	ntfs_fuse_fill_security_context(req, &security);
#endif
		{
		if (dir_ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
			const plugin_operations_t *ops;
			REPARSE_POINT *reparse;

			res = CALL_REPARSE_PLUGIN(dir_ni, link,
					ni, uname, uname_len);
			if (res < 0)
				goto exit;
#else /* DISABLE_PLUGINS */
			res = -EOPNOTSUPP;
			goto exit;
#endif /* DISABLE_PLUGINS */
		} else {
			if (ntfs_link(ni, dir_ni, uname, uname_len)) {
				res = -errno;
				goto exit;
			}
		}
		ntfs_inode_update_mbsname(dir_ni, newname, ni->mft_no);
		if (e) {
			e->ino = ni->mft_no;
			e->generation = 1;
			e->attr_timeout = ATTR_TIMEOUT;
			e->entry_timeout = ENTRY_TIMEOUT;
			res = ntfs_fuse_getstat(&security, ni, &e->attr);
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
	return (res);
}

static void ntfs_fuse_link(fuse_req_t req, fuse_ino_t ino,
			fuse_ino_t newparent, const char *newname)
{
	struct fuse_entry_param entry;
	int res;

	res = ntfs_fuse_newlink(req, ino, newparent, newname, &entry);
	if (res)
		fuse_reply_err(req, -res);
	else
		fuse_reply_entry(req, &entry);
}

static int ntfs_fuse_rm(fuse_req_t req, fuse_ino_t parent, const char *name,
			enum RM_TYPES rm_type __attribute__((unused)))
{
	ntfschar *uname = NULL;
	ntfschar *ugname;
	ntfs_inode *dir_ni = NULL, *ni = NULL;
	int res = 0, uname_len;
	int ugname_len;
	u64 iref;
	fuse_ino_t ino;
	struct open_file *of;
	char ghostname[GHOSTLTH];
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	struct SECURITY_CONTEXT security;
#endif
#if defined(__sun) && defined (__SVR4)
	int isdir;
#endif /* defined(__sun) && defined (__SVR4) */

	/* Deny removing from $Extend */
	if (parent == FILE_Extend) {
		res = -EPERM;
		goto exit;
	}
	/* Open parent directory. */
	dir_ni = ntfs_inode_open(ctx->vol, INODE(parent));
	if (!dir_ni) {
		res = -errno;
		goto exit;
	}
	/* Generate unicode filename. */
	uname_len = ntfs_mbstoucs(name, &uname);
	if (uname_len < 0) {
		res = -errno;
		goto exit;
	}
	/* Open object for delete. */
	iref = ntfs_inode_lookup_by_mbsname(dir_ni, name);
	if (iref == (u64)-1) {
		res = -errno;
		goto exit;
	}
	ino = (fuse_ino_t)MREF(iref);
	/* deny unlinking metadata files */
	if (ino < FILE_first_user) {
		res = -EPERM;
		goto exit;
	}

	ni = ntfs_inode_open(ctx->vol, ino);
	if (!ni) {
		res = -errno;
		goto exit;
	}
        
#if defined(__sun) && defined (__SVR4)
	/* on Solaris : deny unlinking directories */
	isdir = ni->mrec->flags & MFT_RECORD_IS_DIRECTORY;
#ifndef DISABLE_PLUGINS
		/* get emulated type from plugin if available */
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
		struct stat st;
		const plugin_operations_t *ops;
		REPARSE_POINT *reparse;

			/* Avoid double opening of parent directory */
		res = ntfs_inode_close(dir_ni);
		if (res)
			goto exit;
		dir_ni = (ntfs_inode*)NULL;
		res = CALL_REPARSE_PLUGIN(ni, getattr, &st);
		if (res)
			goto exit;
		isdir = S_ISDIR(st.st_mode);
		dir_ni = ntfs_inode_open(ctx->vol, INODE(parent));
		if (!dir_ni) {
			res = -errno;
			goto exit;
		}
	}
#endif /* DISABLE_PLUGINS */
	if (rm_type == (isdir ? RM_LINK : RM_DIR)) {
		errno = (isdir ? EISDIR : ENOTDIR);
		res = -errno;
		goto exit;
	}
#endif /* defined(__sun) && defined (__SVR4) */

#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	/* JPA deny unlinking if directory is not writable and executable */
	if (ntfs_fuse_fill_security_context(req, &security)
	    && !ntfs_allowed_dir_access(&security, dir_ni, ino, ni,
				   S_IEXEC + S_IWRITE + S_ISVTX)) {
		errno = EACCES;
		res = -errno;
		goto exit;
	}
#endif
		/*
		 * We keep one open_file record per opening, to avoid
		 * having to check the list of open files when opening
		 * and closing (which are more frequent than unlinking).
		 * As a consequence, we may have to create several
		 * ghosts names for the same file.
		 * The file may have been opened with a different name
		 * in a different parent directory. The ghost is
		 * nevertheless created in the parent directory of the
		 * name being unlinked, and permissions to do so are the
		 * same as required for unlinking.
		 */
	for (of=ctx->open_files; of; of = of->next) {
		if ((of->ino == ino) && !(of->state & CLOSE_GHOST)) {
			/* file was open, create a ghost in unlink parent */
			ntfs_inode *gni;
			u64 gref;

			/* ni has to be closed for linking ghost */
			if (ni) {
				if (ntfs_inode_close(ni)) {
					res = -errno;
					goto exit;
				}
				ni = (ntfs_inode*)NULL;
			}
			of->state |= CLOSE_GHOST;
			of->parent = parent;
			of->ghost = ++ctx->latest_ghost;
			sprintf(ghostname,ghostformat,of->ghost);
				/* Generate unicode filename. */
			ugname = (ntfschar*)NULL;
			ugname_len = ntfs_mbstoucs(ghostname, &ugname);
			if (ugname_len < 0) {
				res = -errno;
				goto exit;
			}
			/* sweep existing ghost if any, ignoring errors */
			gref = ntfs_inode_lookup_by_mbsname(dir_ni, ghostname);
			if (gref != (u64)-1) {
				gni = ntfs_inode_open(ctx->vol, MREF(gref));
				ntfs_delete(ctx->vol, (char*)NULL, gni, dir_ni,
					 ugname, ugname_len);
				/* ntfs_delete() always closes gni and dir_ni */
				dir_ni = (ntfs_inode*)NULL;
			} else {
				if (ntfs_inode_close(dir_ni)) {
					res = -errno;
					goto out;
				}
				dir_ni = (ntfs_inode*)NULL;
			}
			free(ugname);
			res = ntfs_fuse_newlink(req, of->ino, parent, ghostname,
					(struct fuse_entry_param*)NULL);
			if (res)
				goto out;
				/* now reopen then parent directory */
			dir_ni = ntfs_inode_open(ctx->vol, INODE(parent));
			if (!dir_ni) {
				res = -errno;
				goto exit;
			}
		}
	}
	if (!ni) {
		ni = ntfs_inode_open(ctx->vol, ino);
		if (!ni) {
			res = -errno;
			goto exit;
		}
	}
	if (dir_ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
		const plugin_operations_t *ops;
		REPARSE_POINT *reparse;

		res = CALL_REPARSE_PLUGIN(dir_ni, unlink, (char*)NULL,
				ni, uname, uname_len);
#else /* DISABLE_PLUGINS */
		res = -EOPNOTSUPP;
#endif /* DISABLE_PLUGINS */
	} else
		if (ntfs_delete(ctx->vol, (char*)NULL, ni, dir_ni,
					 uname, uname_len))
			res = -errno;
		/* ntfs_delete() always closes ni and dir_ni */
	ni = dir_ni = NULL;
exit:
	if (ntfs_inode_close(ni) && !res)
		res = -errno;
	if (ntfs_inode_close(dir_ni) && !res)
		res = -errno;
out :
	free(uname);
	return res;
}

static void ntfs_fuse_unlink(fuse_req_t req, fuse_ino_t parent,
				const char *name)
{
	int res;

	res = ntfs_fuse_rm(req, parent, name, RM_LINK);
	if (res)
		fuse_reply_err(req, -res);
	else
		fuse_reply_err(req, 0);
}

static int ntfs_fuse_safe_rename(fuse_req_t req, fuse_ino_t ino,
			fuse_ino_t parent, const char *name, fuse_ino_t xino,
			fuse_ino_t newparent, const char *newname,
			const char *tmp)
{
	int ret;

	ntfs_log_trace("Entering\n");
        
	ret = ntfs_fuse_newlink(req, xino, newparent, tmp,
				(struct fuse_entry_param*)NULL);
	if (ret)
		return ret;
        
	ret = ntfs_fuse_rm(req, newparent, newname, RM_ANY);
	if (!ret) {
	        
		ret = ntfs_fuse_newlink(req, ino, newparent, newname,
					(struct fuse_entry_param*)NULL);
		if (ret)
			goto restore;
	        
		ret = ntfs_fuse_rm(req, parent, name, RM_ANY);
		if (ret) {
			if (ntfs_fuse_rm(req, newparent, newname, RM_ANY))
				goto err;
			goto restore;
		}
	}
        
	goto cleanup;
restore:
	if (ntfs_fuse_newlink(req, xino, newparent, newname,
				(struct fuse_entry_param*)NULL)) {
err:
		ntfs_log_perror("Rename failed. Existing file '%s' was renamed "
				"to '%s'", newname, tmp);
	} else {
cleanup:
		/*
		 * Condition for this unlink has already been checked in
		 * "ntfs_fuse_rename_existing_dest()", so it should never
		 * fail (unless concurrent access to directories when fuse
		 * is multithreaded)
		 */
		if (ntfs_fuse_rm(req, newparent, tmp, RM_ANY) < 0)
			ntfs_log_perror("Rename failed. Existing file '%s' still present "
				"as '%s'", newname, tmp);
	}
	return	ret;
}

static int ntfs_fuse_rename_existing_dest(fuse_req_t req, fuse_ino_t ino,
			fuse_ino_t parent, const char *name,
			fuse_ino_t xino, fuse_ino_t newparent,
			const char *newname)
{
	int ret, len;
	char *tmp;
	const char *ext = ".ntfs-3g-";
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	ntfs_inode *newdir_ni;
	struct SECURITY_CONTEXT security;
#endif

	ntfs_log_trace("Entering\n");
        
	len = strlen(newname) + strlen(ext) + 10 + 1; /* wc(str(2^32)) + \0 */
	tmp = (char*)ntfs_malloc(len);
	if (!tmp)
		return -errno;
        
	ret = snprintf(tmp, len, "%s%s%010d", newname, ext, ++ntfs_sequence);
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
		newdir_ni = ntfs_inode_open(ctx->vol, INODE(newparent));
		if (newdir_ni) {
			if (!ntfs_fuse_fill_security_context(req,&security)
			    || ntfs_allowed_dir_access(&security, newdir_ni,
					xino, (ntfs_inode*)NULL,
					S_IEXEC + S_IWRITE + S_ISVTX)) {
				if (ntfs_inode_close(newdir_ni))
					ret = -errno;
				else
					ret = ntfs_fuse_safe_rename(req, ino,
							parent, name, xino,
							newparent, newname,
							tmp);
			} else {
				ntfs_inode_close(newdir_ni);
				ret = -EACCES;
			}
		} else
			ret = -errno;
#else
		ret = ntfs_fuse_safe_rename(req, ino, parent, name,
					xino, newparent, newname, tmp);
#endif
	}
	free(tmp);
	return	ret;
}

static void ntfs_fuse_rename(fuse_req_t req, fuse_ino_t parent,
			const char *name, fuse_ino_t newparent,
			const char *newname)
{
	int ret;
	fuse_ino_t ino;
	fuse_ino_t xino;
	ntfs_inode *ni;
        
	ntfs_log_debug("rename: old: '%s'  new: '%s'\n", name, newname);
        
	/*
	 *  FIXME: Rename should be atomic.
	 */
        
	ino = ntfs_fuse_inode_lookup(parent, name);
	if (ino == (fuse_ino_t)-1) {
		ret = -errno;
		goto out;
	}
	/* Check whether target is present */
	xino = ntfs_fuse_inode_lookup(newparent, newname);
	if (xino != (fuse_ino_t)-1) {
			/*
			 * Target exists : no need to check whether it
			 * designates the same inode, this has already
			 * been checked (by fuse ?)
			 */
		ni = ntfs_inode_open(ctx->vol, INODE(xino));
		if (!ni)
			ret = -errno;
		else {
			ret = ntfs_check_empty_dir(ni);
			if (ret < 0) {
				ret = -errno;
				ntfs_inode_close(ni);
				goto out;
			}
	        
			if (ntfs_inode_close(ni)) {
				set_fuse_error(&ret);
				goto out;
			}
			ret = ntfs_fuse_rename_existing_dest(req, ino, parent,
						name, xino, newparent, newname);
		}
	} else {
			/* target does not exist */
		ret = ntfs_fuse_newlink(req, ino, newparent, newname,
					(struct fuse_entry_param*)NULL);
		if (ret)
			goto out;
        
		ret = ntfs_fuse_rm(req, parent, name, RM_ANY);
		if (ret)
			ntfs_fuse_rm(req, newparent, newname, RM_ANY);
	}
out:
	if (ret)
		fuse_reply_err(req, -ret);
	else
		fuse_reply_err(req, 0);
}

static void ntfs_fuse_release(fuse_req_t req, fuse_ino_t ino,
			 struct fuse_file_info *fi)
{
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	struct open_file *of;
	char ghostname[GHOSTLTH];
	int res;

	of = (struct open_file*)(long)fi->fh;
	/* Only for marked descriptors there is something to do */
	if (!of
	    || !(of->state & (CLOSE_COMPRESSED | CLOSE_ENCRYPTED
				| CLOSE_DMTIME | CLOSE_REPARSE))) {
		res = 0;
		goto out;
	}
	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni) {
		res = -errno;
		goto exit;
	}
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
#ifndef DISABLE_PLUGINS
		const plugin_operations_t *ops;
		REPARSE_POINT *reparse;

		res = CALL_REPARSE_PLUGIN(ni, release, &of->fi);
		if (!res) {
			goto stamps;
		}
#else /* DISABLE_PLUGINS */
			/* Assume release() was not needed */
		res = 0;
#endif /* DISABLE_PLUGINS */
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		res = -errno;
		goto exit;
	}
	res = 0;
	if (of->state & CLOSE_COMPRESSED)
		res = ntfs_attr_pclose(na);
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
	if (of->state & CLOSE_ENCRYPTED)
		res = ntfs_efs_fixup_attribute(NULL, na);
#endif /* HAVE_SETXATTR */
#ifndef DISABLE_PLUGINS
stamps :
#endif /* DISABLE_PLUGINS */
	if (of->state & CLOSE_DMTIME)
		ntfs_inode_update_times(ni,NTFS_UPDATE_MCTIME);
exit:
	if (na)
		ntfs_attr_close(na);
	if (ntfs_inode_close(ni))
		set_fuse_error(&res);
out:    
		/* remove the associate ghost file (even if release failed) */
	if (of) {
		if (of->state & CLOSE_GHOST) {
			sprintf(ghostname,ghostformat,of->ghost);
			ntfs_fuse_rm(req, of->parent, ghostname, RM_ANY);
		}
			/* remove from open files list */
		if (of->next)
			of->next->previous = of->previous;
		if (of->previous)
			of->previous->next = of->next;
		else
			ctx->open_files = of->next;
		free(of);
	}
	if (res)
		fuse_reply_err(req, -res);
	else
		fuse_reply_err(req, 0);
}

static void ntfs_fuse_mkdir(fuse_req_t req, fuse_ino_t parent,
		       const char *name, mode_t mode)
{
	int res;
	struct fuse_entry_param entry;

	res = ntfs_fuse_create(req, parent, name, S_IFDIR | (mode & 07777),
			0, &entry, (char*)NULL, (struct fuse_file_info*)NULL);
	if (res < 0)
		fuse_reply_err(req, -res);
	else
		fuse_reply_entry(req, &entry);
}

static void ntfs_fuse_rmdir(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	int res;

	res = ntfs_fuse_rm(req, parent, name, RM_DIR);
	if (res)
		fuse_reply_err(req, -res);
	else
		fuse_reply_err(req, 0);
}

static void ntfs_fuse_fsync(fuse_req_t req,
			fuse_ino_t ino __attribute__((unused)),
			int type __attribute__((unused)),
			struct fuse_file_info *fi __attribute__((unused)))
{
		/* sync the full device */
	if (ntfs_device_sync(ctx->vol->dev))
		fuse_reply_err(req, errno);
	else
		fuse_reply_err(req, 0);
}

#if defined(FUSE_INTERNAL) || (FUSE_VERSION >= 28)
static void ntfs_fuse_ioctl(fuse_req_t req __attribute__((unused)),
			fuse_ino_t ino __attribute__((unused)),
			int cmd, void *arg,
			struct fuse_file_info *fi __attribute__((unused)),
			unsigned flags, const void *data,
			size_t in_bufsz, size_t out_bufsz)
{
	ntfs_inode *ni;
	char *buf = (char*)NULL;
	int bufsz;
	int ret = 0;

	if (flags & FUSE_IOCTL_COMPAT) {
		ret = -ENOSYS;
	} else {
		ni = ntfs_inode_open(ctx->vol, INODE(ino));
		if (!ni) {
			ret = -errno;
			goto fail;
		}
		bufsz = (in_bufsz > out_bufsz ? in_bufsz : out_bufsz);
		if (bufsz) {
			buf = ntfs_malloc(bufsz);
			if (!buf) {
				ret = ENOMEM;
				goto fail;
			}
			memcpy(buf, data, in_bufsz);
		}
		/*
		 * Linux defines the request argument of ioctl() to be an
		 * unsigned long, which fuse 2.x forwards as a signed int
		 * into which the request sometimes does not fit.
		 * So we must expand the value and make sure it is not
		 * sign-extended.
		 */
		ret = ntfs_ioctl(ni, (unsigned int)cmd, arg, flags, buf);
		if (ntfs_inode_close (ni))
			set_fuse_error(&ret);
	}
	if (ret)
fail :
		fuse_reply_err(req, -ret);
	else
		fuse_reply_ioctl(req, 0, buf, out_bufsz);
	if (buf)
		free(buf);
}
#endif /* defined(FUSE_INTERNAL) || (FUSE_VERSION >= 28) */

static void ntfs_fuse_bmap(fuse_req_t req, fuse_ino_t ino, size_t blocksize,
		      uint64_t vidx)
{
	ntfs_inode *ni;
	ntfs_attr *na;
	LCN lcn;
	uint64_t lidx = 0;
	int ret = 0; 
	int cl_per_bl = ctx->vol->cluster_size / blocksize;

	if (blocksize > ctx->vol->cluster_size) {
		ret = -EINVAL;
		goto done;
	}

	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni) {
		ret = -errno;
		goto done;
	}

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
        
	lcn = ntfs_rl_vcn_to_lcn(na->rl, vidx / cl_per_bl);
	lidx = (lcn > 0) ? lcn * cl_per_bl + vidx % cl_per_bl : 0;
        
close_attr:
	ntfs_attr_close(na);
close_inode:
	if (ntfs_inode_close(ni))
		set_fuse_error(&ret);
done :
	if (ret < 0)
		fuse_reply_err(req, -ret);
	else
		fuse_reply_bmap(req, lidx);
}

#ifdef HAVE_SETXATTR

/*
 *		  Name space identifications and prefixes
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

static ntfs_inode *ntfs_check_access_xattr(fuse_req_t req,
			struct SECURITY_CONTEXT *security,
			fuse_ino_t ino, int attr, BOOL setting)
{
	ntfs_inode *dir_ni;
	ntfs_inode *ni;
	BOOL foracl;
	BOOL bad;
	mode_t acctype;

	ni = (ntfs_inode*)NULL;
	foracl = (attr == XATTR_POSIX_ACC)
		 || (attr == XATTR_POSIX_DEF);
	/*
	 * When accessing Posix ACL, return unsupported if ACL
	 * were disabled or no user mapping has been defined,
	 * or trying to change a Windows-inherited ACL.
	 * However no error will be returned to getfacl
	 */
	if (((!ntfs_fuse_fill_security_context(req, security)
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
		ni = ntfs_inode_open(ctx->vol, INODE(ino));
			/* basic access was checked previously in a lookup */
		if (ni && (acctype != S_IEXEC)) {
			bad = FALSE;
				/* do not reopen root */
			if (ni->mft_no == FILE_root) {
				/* forbid getting/setting names on root */
				if ((attr == XATTR_NTFS_DOS_NAME)
				    || !ntfs_real_allowed_access(security,
						ni, acctype))
					bad = TRUE;
			} else {
				dir_ni = ntfs_dir_parent_inode(ni);
				if (dir_ni) {
					if (!ntfs_real_allowed_access(security,
							dir_ni, acctype))
						bad = TRUE;
					if (ntfs_inode_close(dir_ni))
						bad = TRUE;
				} else
					bad = TRUE;
			}
			if (bad) {
				ntfs_inode_close(ni);
				ni = (ntfs_inode*)NULL;
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
		prefixed = (char*)ntfs_malloc(strlen(xattr_ntfs_3g)
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

static void ntfs_fuse_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size)
{
	ntfs_attr_search_ctx *actx = NULL;
	ntfs_inode *ni;
	char *list = (char*)NULL;
	int ret = 0;
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	struct SECURITY_CONTEXT security;
#endif

#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	ntfs_fuse_fill_security_context(req, &security);
#endif
	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni) {
		ret = -errno;
		goto out;
	}
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
	if (size) {
		list = (char*)malloc(size);
		if (!list) {
			ret = -errno;
			goto exit;
		}
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
out :
	if (ret < 0)
		fuse_reply_err(req, -ret);
	else
		if (size)
			fuse_reply_buf(req, list, ret);
		else
			fuse_reply_xattr(req, ret);
	free(list);
}

#if defined(__APPLE__) || defined(__DARWIN__)
static void ntfs_fuse_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
			  size_t size, uint32_t position)
#else
static void ntfs_fuse_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
			  size_t size)
#endif
{
#if !(defined(__APPLE__) || defined(__DARWIN__))
	static const unsigned int position = 0U;
#endif

	ntfs_inode *ni;
	ntfs_inode *dir_ni;
	ntfs_attr *na = NULL;
	char *value = (char*)NULL;
	ntfschar *lename = (ntfschar*)NULL;
	int lename_len;
	int res;
	s64 rsize;
	enum SYSTEMXATTRS attr;
	int namespace;
	struct SECURITY_CONTEXT security;

#if defined(__APPLE__) || defined(__DARWIN__)
	/* If the attribute is not a resource fork attribute and the position
	 * parameter is non-zero, we return with EINVAL as requesting position
	 * is not permitted for non-resource fork attributes. */
	if (position && strcmp(name, XATTR_RESOURCEFORK_NAME)) {
		fuse_reply_err(req, EINVAL);
		return;
	}
#endif

	attr = ntfs_xattr_system_type(name,ctx->vol);
	if (attr != XATTR_UNMAPPED) {
		/*
		 * hijack internal data and ACL retrieval, whatever
		 * mode was selected for xattr (from the user's
		 * point of view, ACLs are not xattr)
		 */
		if (size)
			value = (char*)ntfs_malloc(size);
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		if (!size || value) {
			ni = ntfs_check_access_xattr(req, &security, ino,
					attr, FALSE);
			if (ni) {
				if (ntfs_allowed_access(&security,ni,S_IREAD)) {
					if (attr == XATTR_NTFS_DOS_NAME)
						dir_ni = ntfs_dir_parent_inode(ni);
					else
						dir_ni = (ntfs_inode*)NULL;
					res = ntfs_xattr_system_getxattr(&security,
						attr, ni, dir_ni, value, size);
					if (dir_ni && ntfs_inode_close(dir_ni))
						set_fuse_error(&res);
				} else
					res = -errno;
				if (ntfs_inode_close(ni))
					set_fuse_error(&res);
			} else
				res = -errno;
#else
			/*
			 * Standard access control has been done by fuse/kernel
			 */
		if (!size || value) {
			ni = ntfs_inode_open(ctx->vol, INODE(ino));
			if (ni) {
					/* user mapping not mandatory */
				ntfs_fuse_fill_security_context(req, &security);
				if (attr == XATTR_NTFS_DOS_NAME)
					dir_ni = ntfs_dir_parent_inode(ni);
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
#endif
		} else
			res = -errno;
		if (res < 0)
			fuse_reply_err(req, -res);
		else
			if (size)
				fuse_reply_buf(req, value, res);
			else
				fuse_reply_xattr(req, res);
		free(value);
		return;
	}
	if (ctx->streams == NF_STREAMS_INTERFACE_NONE) {
		res = -EOPNOTSUPP;
		goto out;
	}
	namespace = xattr_namespace(name);
	if (namespace == XATTRNS_NONE) {
		res = -EOPNOTSUPP;
		goto out;
	}
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	ntfs_fuse_fill_security_context(req,&security);
		/* trusted only readable by root */
	if ((namespace == XATTRNS_TRUSTED)
	    && security.uid) {
		res = -NTFS_NOXATTR_ERRNO;
		goto out;
	}
#endif
	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni) {
		res = -errno;
		goto out;
	}
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
			value = (char*)ntfs_malloc(rsize);
			if (value)
				res = ntfs_attr_pread(na, position, rsize,
						value);
			if (!value || (res != rsize))
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

out :
	if (res < 0)
		fuse_reply_err(req, -res);
	else
		if (size)
			fuse_reply_buf(req, value, res);
		else
			fuse_reply_xattr(req, res);
	free(value);
}

#if defined(__APPLE__) || defined(__DARWIN__)
static void ntfs_fuse_setxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
			  const char *value, size_t size, int flags,
			  uint32_t position)
#else
static void ntfs_fuse_setxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
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
		fuse_reply_err(req, EINVAL);
		return;
	}
#endif

	attr = ntfs_xattr_system_type(name,ctx->vol);
	if (attr != XATTR_UNMAPPED) {
		/*
		 * hijack internal data and ACL setting, whatever
		 * mode was selected for xattr (from the user's
		 * point of view, ACLs are not xattr)
		 * Note : ctime updated on successful settings
		 */
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
		ni = ntfs_check_access_xattr(req,&security,ino,attr,TRUE);
		if (ni) {
			if (ntfs_allowed_as_owner(&security, ni)) {
				if (attr == XATTR_NTFS_DOS_NAME)
					dir_ni = ntfs_dir_parent_inode(ni);
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
		/* creation of a new name is not controlled by fuse */
		if (attr == XATTR_NTFS_DOS_NAME)
			ni = ntfs_check_access_xattr(req, &security,
					ino, attr, TRUE);
		else
			ni = ntfs_inode_open(ctx->vol, INODE(ino));
		if (ni) {
				/*
				 * user mapping is not mandatory
				 * if defined, only owner is allowed
				 */
			if (!ntfs_fuse_fill_security_context(req, &security)
			   || ntfs_allowed_as_owner(&security, ni)) {
				if (attr == XATTR_NTFS_DOS_NAME)
					dir_ni = ntfs_dir_parent_inode(ni);
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
#endif
#if CACHEING && !defined(FUSE_INTERNAL) && FUSE_VERSION >= 28
		/*
		 * Most of system xattr settings cause changes to some
		 * file attribute (st_mode, st_nlink, st_mtime, etc.),
		 * so we must invalidate cached data when cacheing is
		 * in use (not possible with internal fuse or external
		 * fuse before 2.8)
		 */
		if ((res >= 0)
		    && fuse_lowlevel_notify_inval_inode(ctx->fc, ino, -1, 0))
			res = -errno;
#endif
		if (res < 0)
			fuse_reply_err(req, -res);
		else
			fuse_reply_err(req, 0);
		return;
	}
	if ((ctx->streams != NF_STREAMS_INTERFACE_XATTR)
	    && (ctx->streams != NF_STREAMS_INTERFACE_OPENXATTR)) {
		res = -EOPNOTSUPP;
		goto out;
		}
	namespace = xattr_namespace(name);
	if (namespace == XATTRNS_NONE) {
		res = -EOPNOTSUPP;
		goto out;
	}
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	ntfs_fuse_fill_security_context(req,&security);
		/* security and trusted only settable by root */
	if (((namespace == XATTRNS_SECURITY)
	   || (namespace == XATTRNS_TRUSTED))
		&& security.uid) {
			res = -EPERM;
			goto out;
		}
#endif
	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni) {
		res = -errno;
		goto out;
	}
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
		if (!ntfs_allowed_as_owner(&security, ni)) {
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
out :
	if (res < 0)
		fuse_reply_err(req, -res);
	else
		fuse_reply_err(req, 0);
}

static void ntfs_fuse_removexattr(fuse_req_t req, fuse_ino_t ino, const char *name)
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
			ni = ntfs_check_access_xattr(req, &security, ino,
					attr,TRUE);
			if (ni) {
				if (ntfs_allowed_as_owner(&security, ni)) {
					if (attr == XATTR_NTFS_DOS_NAME)
						dir_ni = ntfs_dir_parent_inode(ni);
					else
						dir_ni = (ntfs_inode*)NULL;
					res = ntfs_xattr_system_removexattr(&security,
							attr, ni, dir_ni);
					if (res)
						res = -errno;
					/* never have to close dir_ni */
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
			/* creation of a new name is not controlled by fuse */
			if (attr == XATTR_NTFS_DOS_NAME)
				ni = ntfs_check_access_xattr(req, &security,
						ino, attr, TRUE);
			else
				ni = ntfs_inode_open(ctx->vol, INODE(ino));
			if (ni) {
				/*
				 * user mapping is not mandatory
				 * if defined, only owner is allowed
				 */
				if (!ntfs_fuse_fill_security_context(req, &security)
				   || ntfs_allowed_as_owner(&security, ni)) {
					if (attr == XATTR_NTFS_DOS_NAME)
						dir_ni = ntfs_dir_parent_inode(ni);
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
#if CACHEING && !defined(FUSE_INTERNAL) && FUSE_VERSION >= 28
		/*
		 * Some allowed system xattr removals cause changes to
		 * some file attribute (st_mode, st_nlink, etc.),
		 * so we must invalidate cached data when cacheing is
		 * in use (not possible with internal fuse or external
		 * fuse before 2.8)
		 */
			if ((res >= 0)
			    && fuse_lowlevel_notify_inval_inode(ctx->fc,
						ino, -1, 0))
				res = -errno;
#endif
			break;
		}
		if (res < 0)
			fuse_reply_err(req, -res);
		else
			fuse_reply_err(req, 0);
		return;
	}
	if ((ctx->streams != NF_STREAMS_INTERFACE_XATTR)
	    && (ctx->streams != NF_STREAMS_INTERFACE_OPENXATTR)) {
		res = -EOPNOTSUPP;
		goto out;
	}
	namespace = xattr_namespace(name);
	if (namespace == XATTRNS_NONE) {
		res = -EOPNOTSUPP;
		goto out;
	}
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	ntfs_fuse_fill_security_context(req,&security);
		/* security and trusted only settable by root */
	if (((namespace == XATTRNS_SECURITY)
	   || (namespace == XATTRNS_TRUSTED))
		&& security.uid) {
			res = -EACCES;
			goto out;
		}
#endif
	ni = ntfs_inode_open(ctx->vol, INODE(ino));
	if (!ni) {
		res = -errno;
		goto out;
	}
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
		if (!ntfs_allowed_as_owner(&security, ni)) {
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
out :
	if (res < 0)
		fuse_reply_err(req, -res);
	else
		fuse_reply_err(req, 0);
	return;

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
		.getattr = junction_getstat,
		.readlink = junction_readlink,
	} ;
	static const plugin_operations_t wsl_ops = {
		.getattr = wsl_getstat,
	} ;
	register_reparse_plugin(ctx, IO_REPARSE_TAG_MOUNT_POINT,
					&ops, (void*)NULL);
	register_reparse_plugin(ctx, IO_REPARSE_TAG_SYMLINK,
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
		if (ntfs_fuse_fill_security_context((fuse_req_t)NULL, &security)) {
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

static void ntfs_fuse_destroy2(void *notused __attribute__((unused)))
{
	ntfs_close();
}

static struct fuse_lowlevel_ops ntfs_3g_ops = {
	.lookup 	= ntfs_fuse_lookup,
	.getattr	= ntfs_fuse_getattr,
	.readlink	= ntfs_fuse_readlink,
	.opendir	= ntfs_fuse_opendir,
	.readdir	= ntfs_fuse_readdir,
	.releasedir	= ntfs_fuse_releasedir,
	.open		= ntfs_fuse_open,
	.release	= ntfs_fuse_release,
	.read		= ntfs_fuse_read,
	.write		= ntfs_fuse_write,
	.setattr	= ntfs_fuse_setattr,
	.statfs 	= ntfs_fuse_statfs,
	.create 	= ntfs_fuse_create_file,
	.mknod		= ntfs_fuse_mknod,
	.symlink	= ntfs_fuse_symlink,
	.link		= ntfs_fuse_link,
	.unlink 	= ntfs_fuse_unlink,
	.rename 	= ntfs_fuse_rename,
	.mkdir		= ntfs_fuse_mkdir,
	.rmdir		= ntfs_fuse_rmdir,
	.fsync		= ntfs_fuse_fsync,
	.fsyncdir	= ntfs_fuse_fsync,
	.bmap		= ntfs_fuse_bmap,
	.destroy	= ntfs_fuse_destroy2,
#if defined(FUSE_INTERNAL) || (FUSE_VERSION >= 28)
	.ioctl		= ntfs_fuse_ioctl,
#endif /* defined(FUSE_INTERNAL) || (FUSE_VERSION >= 28) */
#if !KERNELPERMS | (POSIXACLS & !KERNELACLS)
	.access 	= ntfs_fuse_access,
#endif
#ifdef HAVE_SETXATTR
	.getxattr	= ntfs_fuse_getxattr,
	.setxattr	= ntfs_fuse_setxattr,
	.removexattr	= ntfs_fuse_removexattr,
	.listxattr	= ntfs_fuse_listxattr,
#endif /* HAVE_SETXATTR */
#if 0 && (defined(__APPLE__) || defined(__DARWIN__)) /* Unfinished. */
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
	ctx = (ntfs_fuse_context_t*)ntfs_calloc(sizeof(ntfs_fuse_context_t));
	if (!ctx)
		return -1;
        
	*ctx = (ntfs_fuse_context_t) {
		.uid	 = getuid(),
		.gid	 = getgid(),
#if defined(linux)		        
		.streams = NF_STREAMS_INTERFACE_XATTR,
#else		        
		.streams = NF_STREAMS_INTERFACE_NONE,
#endif		        
		.atime	 = ATIME_RELATIVE,
		.silent  = TRUE,
		.recover = TRUE
	};
	return 0;
}

static int ntfs_open(const char *device)
{
	unsigned long flags = 0;
	ntfs_volume *vol;
        
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

	ctx->vol = vol = ntfs_mount(device, flags);
	if (!vol) {
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

	if (ctx->ignore_case && ntfs_set_ignore_case(vol))
		goto err_out;
        
	if (ntfs_volume_get_free_space(ctx->vol)) {
		ntfs_log_perror("Failed to read NTFS $Bitmap");
		goto err_out;
	}

	vol->free_mft_records = ntfs_get_nr_free_mft_records(vol);
	if (vol->free_mft_records < 0) {
		ntfs_log_perror("Failed to calculate free MFT records");
		goto err_out;
	}

	if (ctx->hiberfile && ntfs_volume_check_hiberfile(vol, 0)) {
		if (errno != EPERM)
			goto err_out;
		if (ntfs_fuse_rm((fuse_req_t)NULL,FILE_root,"hiberfil.sys",
					RM_LINK))
			goto err_out;
	}
        
	errno = 0;
	goto out;
err_out:
	if (!errno)	/* Make sure to return an error */
		errno = EIO;
out :
	return ntfs_volume_error(errno);
}

static void usage(void)
{
	ntfs_log_info(usage_msg, EXEC_NAME, VERSION, FUSE_TYPE, fuse_version(),
			5 + POSIXACLS*6 - KERNELPERMS*3 + CACHEING,
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

static struct fuse_session *mount_fuse(char *parsed_options)
{
	struct fuse_session *se = NULL;
	struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
        
	ctx->fc = try_fuse_mount(parsed_options);
	if (!ctx->fc)
		return NULL;
        
	if (fuse_opt_add_arg(&args, "") == -1)
		goto err;
	if (ctx->debug)
		if (fuse_opt_add_arg(&args, "-odebug") == -1)
			goto err;
        
	se = fuse_lowlevel_new(&args , &ntfs_3g_ops, sizeof(ntfs_3g_ops), NULL);
	if (!se)
		goto err;
        
        
	if (fuse_set_signal_handlers(se))
		goto err_destroy;
	fuse_session_add_chan(se, ctx->fc);
out:
	fuse_opt_free_args(&args);
	return se;
err_destroy:
	fuse_session_destroy(se);
	se = NULL;
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
	struct fuse_session *se;
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
        
	parsed_options = parse_mount_options(ctx, &opts, TRUE);
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
	ctx->vol->special_files = ctx->special_files;
	ctx->security.vol = ctx->vol;
	ctx->vol->secure_flags = ctx->secure_flags;
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
			if (ntfs_strinsert(&parsed_options,
					",default_permissions,acl")) {
				err = NTFS_VOLUME_SYNTAX_ERROR;
				goto err_out;
			}
#endif /* KERNELACLS */
		}
#else /* POSIXACLS */
		if (!(ctx->vol->secure_flags
			& ((1 << SECURITY_DEFAULT) | (1 << SECURITY_ACL)))) {
			/*
			 * No explicit option but user mapping found
			 * force default security
			 */
#if KERNELPERMS
			ctx->vol->secure_flags |= (1 << SECURITY_DEFAULT);
			if (ntfs_strinsert(&parsed_options, ",default_permissions")) {
				err = NTFS_VOLUME_SYNTAX_ERROR;
				goto err_out;
			}
#endif /* KERNELPERMS */
		}
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

	se = mount_fuse(parsed_options);
	if (!se) {
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
			5 + POSIXACLS*6 - KERNELPERMS*3 + CACHEING);
        
	fuse_session_loop(se);
	fuse_remove_signal_handlers(se);
        
	err = 0;

	fuse_unmount(opts.mnt_point, ctx->fc);
	fuse_session_destroy(se);
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

