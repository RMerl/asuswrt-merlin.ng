/*
 * fuse2fs.c - FUSE server for e2fsprogs.
 *
 * Copyright (C) 2014 Oracle.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */
#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 29
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "config.h"
#include <pthread.h>
#ifdef __linux__
# include <linux/fs.h>
# include <linux/falloc.h>
# include <linux/xattr.h>
# define FUSE_PLATFORM_OPTS	",big_writes"
# ifdef HAVE_SYS_ACL_H
#  define TRANSLATE_LINUX_ACLS
# endif
#else
# define FUSE_PLATFORM_OPTS	""
#endif
#ifdef TRANSLATE_LINUX_ACLS
# include <sys/acl.h>
#endif
#include <sys/ioctl.h>
#include <unistd.h>
#include <fuse.h>
#include <inttypes.h>
#include "ext2fs/ext2fs.h"
#include "ext2fs/ext2_fs.h"

#include "../version.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(a) (gettext(a))
#ifdef gettext_noop
#define N_(a) gettext_noop(a)
#else
#define N_(a) (a)
#endif
#define P_(singular, plural, n) (ngettext(singular, plural, n))
#ifndef NLS_CAT_NAME
#define NLS_CAT_NAME "e2fsprogs"
#endif
#ifndef LOCALEDIR
#define LOCALEDIR "/usr/share/locale"
#endif
#else
#define _(a) (a)
#define N_(a) a
#define P_(singular, plural, n) ((n) == 1 ? (singular) : (plural))
#endif

static ext2_filsys global_fs; /* Try not to use this directly */

#undef DEBUG

#ifdef DEBUG
# define dbg_printf(f, a...)  do {printf("FUSE2FS-" f, ## a); \
	fflush(stdout); \
} while (0)
#else
# define dbg_printf(f, a...)
#endif

#if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 8)
# ifdef _IOR
#  ifdef _IOW
#   define SUPPORT_I_FLAGS
#  endif
# endif
#endif

#ifdef FALLOC_FL_KEEP_SIZE
# define FL_KEEP_SIZE_FLAG FALLOC_FL_KEEP_SIZE
# define SUPPORT_FALLOCATE
#else
# define FL_KEEP_SIZE_FLAG (0)
#endif

#ifdef FALLOC_FL_PUNCH_HOLE
# define FL_PUNCH_HOLE_FLAG FALLOC_FL_PUNCH_HOLE
#else
# define FL_PUNCH_HOLE_FLAG (0)
#endif

errcode_t ext2fs_run_ext3_journal(ext2_filsys *fs);

#ifdef CONFIG_JBD_DEBUG		/* Enabled by configure --enable-jbd-debug */
int journal_enable_debug = -1;
#endif

/* ACL translation stuff */
#ifdef TRANSLATE_LINUX_ACLS
/*
 * Copied from acl_ea.h in libacl source; ACLs have to be sent to and from fuse
 * in this format... at least on Linux.
 */
#define ACL_EA_ACCESS		"system.posix_acl_access"
#define ACL_EA_DEFAULT		"system.posix_acl_default"

#define ACL_EA_VERSION		0x0002

typedef struct {
	u_int16_t	e_tag;
	u_int16_t	e_perm;
	u_int32_t	e_id;
} acl_ea_entry;

typedef struct {
	u_int32_t	a_version;
#if __GNUC_PREREQ (4, 8)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
	acl_ea_entry	a_entries[0];
#if __GNUC_PREREQ (4, 8)
#pragma GCC diagnostic pop
#endif
} acl_ea_header;

static inline size_t acl_ea_size(int count)
{
	return sizeof(acl_ea_header) + count * sizeof(acl_ea_entry);
}

static inline int acl_ea_count(size_t size)
{
	if (size < sizeof(acl_ea_header))
		return -1;
	size -= sizeof(acl_ea_header);
	if (size % sizeof(acl_ea_entry))
		return -1;
	return size / sizeof(acl_ea_entry);
}

/*
 * ext4 ACL structures, copied from fs/ext4/acl.h.
 */
#define EXT4_ACL_VERSION	0x0001

typedef struct {
	__u16		e_tag;
	__u16		e_perm;
	__u32		e_id;
} ext4_acl_entry;

typedef struct {
	__u16		e_tag;
	__u16		e_perm;
} ext4_acl_entry_short;

typedef struct {
	__u32		a_version;
} ext4_acl_header;

static inline size_t ext4_acl_size(int count)
{
	if (count <= 4) {
		return sizeof(ext4_acl_header) +
		       count * sizeof(ext4_acl_entry_short);
	} else {
		return sizeof(ext4_acl_header) +
		       4 * sizeof(ext4_acl_entry_short) +
		       (count - 4) * sizeof(ext4_acl_entry);
	}
}

static inline int ext4_acl_count(size_t size)
{
	ssize_t s;

	size -= sizeof(ext4_acl_header);
	s = size - 4 * sizeof(ext4_acl_entry_short);
	if (s < 0) {
		if (size % sizeof(ext4_acl_entry_short))
			return -1;
		return size / sizeof(ext4_acl_entry_short);
	}
	if (s % sizeof(ext4_acl_entry))
		return -1;
	return s / sizeof(ext4_acl_entry) + 4;
}

static errcode_t fuse_to_ext4_acl(acl_ea_header *facl, size_t facl_sz,
				  ext4_acl_header **eacl, size_t *eacl_sz)
{
	int i, facl_count;
	ext4_acl_header *h;
	size_t h_sz;
	ext4_acl_entry *e;
	acl_ea_entry *a;
	unsigned char *hptr;
	errcode_t err;

	facl_count = acl_ea_count(facl_sz);
	h_sz = ext4_acl_size(facl_count);
	if (facl_count < 0 || facl->a_version != ACL_EA_VERSION)
		return EXT2_ET_INVALID_ARGUMENT;

	err = ext2fs_get_mem(h_sz, &h);
	if (err)
		return err;

	h->a_version = ext2fs_cpu_to_le32(EXT4_ACL_VERSION);
	hptr = (unsigned char *) (h + 1);
	for (i = 0, a = facl->a_entries; i < facl_count; i++, a++) {
		e = (ext4_acl_entry *) hptr;
		e->e_tag = ext2fs_cpu_to_le16(a->e_tag);
		e->e_perm = ext2fs_cpu_to_le16(a->e_perm);

		switch (a->e_tag) {
		case ACL_USER:
		case ACL_GROUP:
			e->e_id = ext2fs_cpu_to_le32(a->e_id);
			hptr += sizeof(ext4_acl_entry);
			break;
		case ACL_USER_OBJ:
		case ACL_GROUP_OBJ:
		case ACL_MASK:
		case ACL_OTHER:
			hptr += sizeof(ext4_acl_entry_short);
			break;
		default:
			err = EXT2_ET_INVALID_ARGUMENT;
			goto out;
		}
	}

	*eacl = h;
	*eacl_sz = h_sz;
	return err;
out:
	ext2fs_free_mem(&h);
	return err;
}

static errcode_t ext4_to_fuse_acl(acl_ea_header **facl, size_t *facl_sz,
				  ext4_acl_header *eacl, size_t eacl_sz)
{
	int i, eacl_count;
	acl_ea_header *f;
	ext4_acl_entry *e;
	acl_ea_entry *a;
	size_t f_sz;
	unsigned char *hptr;
	errcode_t err;

	eacl_count = ext4_acl_count(eacl_sz);
	f_sz = acl_ea_size(eacl_count);
	if (eacl_count < 0 ||
	    eacl->a_version != ext2fs_cpu_to_le32(EXT4_ACL_VERSION))
		return EXT2_ET_INVALID_ARGUMENT;

	err = ext2fs_get_mem(f_sz, &f);
	if (err)
		return err;

	f->a_version = ACL_EA_VERSION;
	hptr = (unsigned char *) (eacl + 1);
	for (i = 0, a = f->a_entries; i < eacl_count; i++, a++) {
		e = (ext4_acl_entry *) hptr;
		a->e_tag = ext2fs_le16_to_cpu(e->e_tag);
		a->e_perm = ext2fs_le16_to_cpu(e->e_perm);

		switch (a->e_tag) {
		case ACL_USER:
		case ACL_GROUP:
			a->e_id = ext2fs_le32_to_cpu(e->e_id);
			hptr += sizeof(ext4_acl_entry);
			break;
		case ACL_USER_OBJ:
		case ACL_GROUP_OBJ:
		case ACL_MASK:
		case ACL_OTHER:
			hptr += sizeof(ext4_acl_entry_short);
			break;
		default:
			err = EXT2_ET_INVALID_ARGUMENT;
			goto out;
		}
	}

	*facl = f;
	*facl_sz = f_sz;
	return err;
out:
	ext2fs_free_mem(&f);
	return err;
}
#endif /* TRANSLATE_LINUX_ACLS */

/*
 * ext2_file_t contains a struct inode, so we can't leave files open.
 * Use this as a proxy instead.
 */
#define FUSE2FS_FILE_MAGIC	(0xEF53DEAFUL)
struct fuse2fs_file_handle {
	unsigned long magic;
	ext2_ino_t ino;
	int open_flags;
};

/* Main program context */
#define FUSE2FS_MAGIC		(0xEF53DEADUL)
struct fuse2fs {
	unsigned long magic;
	ext2_filsys fs;
	pthread_mutex_t bfl;
	char *device;
	int ro;
	int debug;
	int no_default_opts;
	int panic_on_error;
	int minixdf;
	int fakeroot;
	int alloc_all_blocks;
	FILE *err_fp;
	unsigned int next_generation;
};

#define FUSE2FS_CHECK_MAGIC(fs, ptr, num) do {if ((ptr)->magic != (num)) \
	return translate_error((fs), 0, EXT2_ET_MAGIC_EXT2_FILE); \
} while (0)

#define FUSE2FS_CHECK_CONTEXT(ptr) do {if ((ptr)->magic != FUSE2FS_MAGIC) \
	return translate_error(global_fs, 0, EXT2_ET_BAD_MAGIC); \
} while (0)

static int __translate_error(ext2_filsys fs, errcode_t err, ext2_ino_t ino,
			     const char *file, int line);
#define translate_error(fs, ino, err) __translate_error((fs), (err), (ino), \
			__FILE__, __LINE__)

/* for macosx */
#ifndef W_OK
#  define W_OK 2
#endif

#ifndef R_OK
#  define R_OK 4
#endif

#define EXT4_EPOCH_BITS 2
#define EXT4_EPOCH_MASK ((1 << EXT4_EPOCH_BITS) - 1)
#define EXT4_NSEC_MASK  (~0UL << EXT4_EPOCH_BITS)

/*
 * Extended fields will fit into an inode if the filesystem was formatted
 * with large inodes (-I 256 or larger) and there are not currently any EAs
 * consuming all of the available space. For new inodes we always reserve
 * enough space for the kernel's known extended fields, but for inodes
 * created with an old kernel this might not have been the case. None of
 * the extended inode fields is critical for correct filesystem operation.
 * This macro checks if a certain field fits in the inode. Note that
 * inode-size = GOOD_OLD_INODE_SIZE + i_extra_isize
 */
#define EXT4_FITS_IN_INODE(ext4_inode, field)		\
	((offsetof(typeof(*ext4_inode), field) +	\
	  sizeof((ext4_inode)->field))			\
	 <= ((size_t) EXT2_GOOD_OLD_INODE_SIZE +		\
	    (ext4_inode)->i_extra_isize))		\

static inline __u32 ext4_encode_extra_time(const struct timespec *time)
{
	__u32 extra = sizeof(time->tv_sec) > 4 ?
			((time->tv_sec - (__s32)time->tv_sec) >> 32) &
			EXT4_EPOCH_MASK : 0;
	return extra | (time->tv_nsec << EXT4_EPOCH_BITS);
}

static inline void ext4_decode_extra_time(struct timespec *time, __u32 extra)
{
	if (sizeof(time->tv_sec) > 4 && (extra & EXT4_EPOCH_MASK)) {
		__u64 extra_bits = extra & EXT4_EPOCH_MASK;
		/*
		 * Prior to kernel 3.14?, we had a broken decode function,
		 * wherein we effectively did this:
		 * if (extra_bits == 3)
		 *     extra_bits = 0;
		 */
		time->tv_sec += extra_bits << 32;
	}
	time->tv_nsec = ((extra) & EXT4_NSEC_MASK) >> EXT4_EPOCH_BITS;
}

#define EXT4_INODE_SET_XTIME(xtime, timespec, raw_inode)		       \
do {									       \
	(raw_inode)->xtime = (timespec)->tv_sec;			       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime ## _extra))		       \
		(raw_inode)->xtime ## _extra =				       \
				ext4_encode_extra_time(timespec);	       \
} while (0)

#define EXT4_EINODE_SET_XTIME(xtime, timespec, raw_inode)		       \
do {									       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime))			       \
		(raw_inode)->xtime = (timespec)->tv_sec;		       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime ## _extra))		       \
		(raw_inode)->xtime ## _extra =				       \
				ext4_encode_extra_time(timespec);	       \
} while (0)

#define EXT4_INODE_GET_XTIME(xtime, timespec, raw_inode)		       \
do {									       \
	(timespec)->tv_sec = (signed)((raw_inode)->xtime);		       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime ## _extra))		       \
		ext4_decode_extra_time((timespec),			       \
				       (raw_inode)->xtime ## _extra);	       \
	else								       \
		(timespec)->tv_nsec = 0;				       \
} while (0)

#define EXT4_EINODE_GET_XTIME(xtime, timespec, raw_inode)		       \
do {									       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime))			       \
		(timespec)->tv_sec =					       \
			(signed)((raw_inode)->xtime);			       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime ## _extra))		       \
		ext4_decode_extra_time((timespec),			       \
				       raw_inode->xtime ## _extra);	       \
	else								       \
		(timespec)->tv_nsec = 0;				       \
} while (0)

static void get_now(struct timespec *now)
{
#ifdef CLOCK_REALTIME
	if (!clock_gettime(CLOCK_REALTIME, now))
		return;
#endif

	now->tv_sec = time(NULL);
	now->tv_nsec = 0;
}

static void increment_version(struct ext2_inode_large *inode)
{
	__u64 ver;

	ver = inode->osd1.linux1.l_i_version;
	if (EXT4_FITS_IN_INODE(inode, i_version_hi))
		ver |= (__u64)inode->i_version_hi << 32;
	ver++;
	inode->osd1.linux1.l_i_version = ver;
	if (EXT4_FITS_IN_INODE(inode, i_version_hi))
		inode->i_version_hi = ver >> 32;
}

static void init_times(struct ext2_inode_large *inode)
{
	struct timespec now;

	get_now(&now);
	EXT4_INODE_SET_XTIME(i_atime, &now, inode);
	EXT4_INODE_SET_XTIME(i_ctime, &now, inode);
	EXT4_INODE_SET_XTIME(i_mtime, &now, inode);
	EXT4_EINODE_SET_XTIME(i_crtime, &now, inode);
	increment_version(inode);
}

static int update_ctime(ext2_filsys fs, ext2_ino_t ino,
			struct ext2_inode_large *pinode)
{
	errcode_t err;
	struct timespec now;
	struct ext2_inode_large inode;

	get_now(&now);

	/* If user already has a inode buffer, just update that */
	if (pinode) {
		increment_version(pinode);
		EXT4_INODE_SET_XTIME(i_ctime, &now, pinode);
		return 0;
	}

	/* Otherwise we have to read-modify-write the inode */
	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err)
		return translate_error(fs, ino, err);

	increment_version(&inode);
	EXT4_INODE_SET_XTIME(i_ctime, &now, &inode);

	err = ext2fs_write_inode_full(fs, ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err)
		return translate_error(fs, ino, err);

	return 0;
}

static int update_atime(ext2_filsys fs, ext2_ino_t ino)
{
	errcode_t err;
	struct ext2_inode_large inode, *pinode;
	struct timespec atime, mtime, now;

	if (!(fs->flags & EXT2_FLAG_RW))
		return 0;
	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err)
		return translate_error(fs, ino, err);

	pinode = &inode;
	EXT4_INODE_GET_XTIME(i_atime, &atime, pinode);
	EXT4_INODE_GET_XTIME(i_mtime, &mtime, pinode);
	get_now(&now);
	/*
	 * If atime is newer than mtime and atime hasn't been updated in thirty
	 * seconds, skip the atime update.  Same idea as Linux "relatime".
	 */
	if (atime.tv_sec >= mtime.tv_sec && atime.tv_sec >= now.tv_sec - 30)
		return 0;
	EXT4_INODE_SET_XTIME(i_atime, &now, &inode);

	err = ext2fs_write_inode_full(fs, ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err)
		return translate_error(fs, ino, err);

	return 0;
}

static int update_mtime(ext2_filsys fs, ext2_ino_t ino,
			struct ext2_inode_large *pinode)
{
	errcode_t err;
	struct ext2_inode_large inode;
	struct timespec now;

	if (pinode) {
		get_now(&now);
		EXT4_INODE_SET_XTIME(i_mtime, &now, pinode);
		EXT4_INODE_SET_XTIME(i_ctime, &now, pinode);
		increment_version(pinode);
		return 0;
	}

	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err)
		return translate_error(fs, ino, err);

	get_now(&now);
	EXT4_INODE_SET_XTIME(i_mtime, &now, &inode);
	EXT4_INODE_SET_XTIME(i_ctime, &now, &inode);
	increment_version(&inode);

	err = ext2fs_write_inode_full(fs, ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err)
		return translate_error(fs, ino, err);

	return 0;
}

static int ext2_file_type(unsigned int mode)
{
	if (LINUX_S_ISREG(mode))
		return EXT2_FT_REG_FILE;

	if (LINUX_S_ISDIR(mode))
		return EXT2_FT_DIR;

	if (LINUX_S_ISCHR(mode))
		return EXT2_FT_CHRDEV;

	if (LINUX_S_ISBLK(mode))
		return EXT2_FT_BLKDEV;

	if (LINUX_S_ISLNK(mode))
		return EXT2_FT_SYMLINK;

	if (LINUX_S_ISFIFO(mode))
		return EXT2_FT_FIFO;

	if (LINUX_S_ISSOCK(mode))
		return EXT2_FT_SOCK;

	return 0;
}

static int fs_can_allocate(struct fuse2fs *ff, blk64_t num)
{
	ext2_filsys fs = ff->fs;
	blk64_t reserved;

	dbg_printf("%s: Asking for %llu; alloc_all=%d total=%llu free=%llu "
		   "rsvd=%llu\n", __func__, num, ff->alloc_all_blocks,
		   ext2fs_blocks_count(fs->super),
		   ext2fs_free_blocks_count(fs->super),
		   ext2fs_r_blocks_count(fs->super));
	if (num > ext2fs_blocks_count(fs->super))
		return 0;

	if (ff->alloc_all_blocks)
		return 1;

	/*
	 * Different meaning for r_blocks -- libext2fs has bugs where the FS
	 * can get corrupted if it totally runs out of blocks.  Avoid this
	 * by refusing to allocate any of the reserve blocks to anybody.
	 */
	reserved = ext2fs_r_blocks_count(fs->super);
	if (reserved == 0)
		reserved = ext2fs_blocks_count(fs->super) / 10;
	return ext2fs_free_blocks_count(fs->super) > reserved + num;
}

static int fs_writeable(ext2_filsys fs)
{
	return (fs->flags & EXT2_FLAG_RW) && (fs->super->s_error_count == 0);
}

static int check_inum_access(ext2_filsys fs, ext2_ino_t ino, mode_t mask)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct ext2_inode inode;
	mode_t perms;
	errcode_t err;

	/* no writing to read-only or broken fs */
	if ((mask & W_OK) && !fs_writeable(fs))
		return -EROFS;

	err = ext2fs_read_inode(fs, ino, &inode);
	if (err)
		return translate_error(fs, ino, err);
	perms = inode.i_mode & 0777;

	dbg_printf("access ino=%d mask=e%s%s%s perms=0%o fuid=%d fgid=%d "
		   "uid=%d gid=%d\n", ino,
		   (mask & R_OK ? "r" : ""), (mask & W_OK ? "w" : ""),
		   (mask & X_OK ? "x" : ""), perms, inode_uid(inode),
		   inode_gid(inode), ctxt->uid, ctxt->gid);

	/* existence check */
	if (mask == 0)
		return 0;

	/* is immutable? */
	if ((mask & W_OK) &&
	    (inode.i_flags & EXT2_IMMUTABLE_FL))
		return -EACCES;

	/* Figure out what root's allowed to do */
	if (ff->fakeroot || ctxt->uid == 0) {
		/* Non-file access always ok */
		if (!LINUX_S_ISREG(inode.i_mode))
			return 0;

		/* R/W access to a file always ok */
		if (!(mask & X_OK))
			return 0;

		/* X access to a file ok if a user/group/other can X */
		if (perms & 0111)
			return 0;

		/* Trying to execute a file that's not executable. BZZT! */
		return -EACCES;
	}

	/* allow owner, if perms match */
	if (inode_uid(inode) == ctxt->uid) {
		if ((mask & (perms >> 6)) == mask)
			return 0;
		return -EACCES;
	}

	/* allow group, if perms match */
	if (inode_gid(inode) == ctxt->gid) {
		if ((mask & (perms >> 3)) == mask)
			return 0;
		return -EACCES;
	}

	/* otherwise check other */
	if ((mask & perms) == mask)
		return 0;
	return -EACCES;
}

static void op_destroy(void *p EXT2FS_ATTR((unused)))
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;

	if (ff->magic != FUSE2FS_MAGIC) {
		translate_error(global_fs, 0, EXT2_ET_BAD_MAGIC);
		return;
	}
	fs = ff->fs;
	dbg_printf("%s: dev=%s\n", __func__, fs->device_name);
	if (fs->flags & EXT2_FLAG_RW) {
		fs->super->s_state |= EXT2_VALID_FS;
		if (fs->super->s_error_count)
			fs->super->s_state |= EXT2_ERROR_FS;
		ext2fs_mark_super_dirty(fs);
		err = ext2fs_set_gdt_csum(fs);
		if (err)
			translate_error(fs, 0, err);

		err = ext2fs_flush2(fs, 0);
		if (err)
			translate_error(fs, 0, err);
	}
}

static void *op_init(struct fuse_conn_info *conn)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;

	if (ff->magic != FUSE2FS_MAGIC) {
		translate_error(global_fs, 0, EXT2_ET_BAD_MAGIC);
		return NULL;
	}
	fs = ff->fs;
	dbg_printf("%s: dev=%s\n", __func__, fs->device_name);
#ifdef FUSE_CAP_IOCTL_DIR
	conn->want |= FUSE_CAP_IOCTL_DIR;
#endif
	if (fs->flags & EXT2_FLAG_RW) {
		fs->super->s_mnt_count++;
		fs->super->s_mtime = time(NULL);
		fs->super->s_state &= ~EXT2_VALID_FS;
		ext2fs_mark_super_dirty(fs);
		err = ext2fs_flush2(fs, 0);
		if (err)
			translate_error(fs, 0, err);
	}
	return ff;
}

static int stat_inode(ext2_filsys fs, ext2_ino_t ino, struct stat *statbuf)
{
	struct ext2_inode_large inode;
	dev_t fakedev = 0;
	errcode_t err;
	int ret = 0;
	struct timespec tv;

	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err)
		return translate_error(fs, ino, err);

	memcpy(&fakedev, fs->super->s_uuid, sizeof(fakedev));
	statbuf->st_dev = fakedev;
	statbuf->st_ino = ino;
	statbuf->st_mode = inode.i_mode;
	statbuf->st_nlink = inode.i_links_count;
	statbuf->st_uid = inode_uid(inode);
	statbuf->st_gid = inode_gid(inode);
	statbuf->st_size = EXT2_I_SIZE(&inode);
	statbuf->st_blksize = fs->blocksize;
	statbuf->st_blocks = ext2fs_get_stat_i_blocks(fs,
						(struct ext2_inode *)&inode);
	EXT4_INODE_GET_XTIME(i_atime, &tv, &inode);
	statbuf->st_atime = tv.tv_sec;
	EXT4_INODE_GET_XTIME(i_mtime, &tv, &inode);
	statbuf->st_mtime = tv.tv_sec;
	EXT4_INODE_GET_XTIME(i_ctime, &tv, &inode);
	statbuf->st_ctime = tv.tv_sec;
	if (LINUX_S_ISCHR(inode.i_mode) ||
	    LINUX_S_ISBLK(inode.i_mode)) {
		if (inode.i_block[0])
			statbuf->st_rdev = inode.i_block[0];
		else
			statbuf->st_rdev = inode.i_block[1];
	}

	return ret;
}

static int op_getattr(const char *path, struct stat *statbuf)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf("%s: path=%s\n", __func__, path);
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	ret = stat_inode(fs, ino, statbuf);
out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_readlink(const char *path, char *buf, size_t len)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t ino;
	struct ext2_inode inode;
	unsigned int got;
	ext2_file_t file;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf("%s: path=%s\n", __func__, path);
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}

	err = ext2fs_read_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	if (!LINUX_S_ISLNK(inode.i_mode)) {
		ret = -EINVAL;
		goto out;
	}

	len--;
	if (inode.i_size < len)
		len = inode.i_size;
	if (ext2fs_is_fast_symlink(&inode))
		memcpy(buf, (char *)inode.i_block, len);
	else {
		/* big/inline symlink */

		err = ext2fs_file_open(fs, ino, 0, &file);
		if (err) {
			ret = translate_error(fs, ino, err);
			goto out;
		}

		err = ext2fs_file_read(file, buf, len, &got);
		if (err || got != len) {
			ext2fs_file_close(file);
			ret = translate_error(fs, ino, err);
			goto out2;
		}

out2:
		err = ext2fs_file_close(file);
		if (ret)
			goto out;
		if (err) {
			ret = translate_error(fs, ino, err);
			goto out;
		}
	}
	buf[len] = 0;

	if (fs_writeable(fs)) {
		ret = update_atime(fs, ino);
		if (ret)
			goto out;
	}

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_mknod(const char *path, mode_t mode, dev_t dev)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t parent, child;
	char *temp_path;
	errcode_t err;
	char *node_name, a;
	int filetype;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf("%s: path=%s mode=0%o dev=0x%x\n", __func__, path, mode,
		   (unsigned int)dev);
	temp_path = strdup(path);
	if (!temp_path) {
		ret = -ENOMEM;
		goto out;
	}
	node_name = strrchr(temp_path, '/');
	if (!node_name) {
		ret = -ENOMEM;
		goto out;
	}
	node_name++;
	a = *node_name;
	*node_name = 0;

	pthread_mutex_lock(&ff->bfl);
	if (!fs_can_allocate(ff, 2)) {
		ret = -ENOSPC;
		goto out2;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &parent);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	ret = check_inum_access(fs, parent, W_OK);
	if (ret)
		goto out2;

	*node_name = a;

	if (LINUX_S_ISCHR(mode))
		filetype = EXT2_FT_CHRDEV;
	else if (LINUX_S_ISBLK(mode))
		filetype = EXT2_FT_BLKDEV;
	else if (LINUX_S_ISFIFO(mode))
		filetype = EXT2_FT_FIFO;
	else if (LINUX_S_ISSOCK(mode))
		filetype = EXT2_FT_SOCK;
	else {
		ret = -EINVAL;
		goto out2;
	}

	err = ext2fs_new_inode(fs, parent, mode, 0, &child);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	dbg_printf("%s: create ino=%d/name=%s in dir=%d\n", __func__, child,
		   node_name, parent);
	err = ext2fs_link(fs, parent, node_name, child, filetype);
	if (err == EXT2_ET_DIR_NO_SPACE) {
		err = ext2fs_expand_dir(fs, parent);
		if (err) {
			ret = translate_error(fs, parent, err);
			goto out2;
		}

		err = ext2fs_link(fs, parent, node_name, child,
				     filetype);
	}
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	ret = update_mtime(fs, parent, NULL);
	if (ret)
		goto out2;

	memset(&inode, 0, sizeof(inode));
	inode.i_mode = mode;

	if (dev & ~0xFFFF)
		inode.i_block[1] = dev;
	else
		inode.i_block[0] = dev;
	inode.i_links_count = 1;
	inode.i_extra_isize = sizeof(struct ext2_inode_large) -
		EXT2_GOOD_OLD_INODE_SIZE;
	inode.i_uid = ctxt->uid;
	ext2fs_set_i_uid_high(inode, ctxt->uid >> 16);
	inode.i_gid = ctxt->gid;
	ext2fs_set_i_gid_high(inode, ctxt->gid >> 16);

	err = ext2fs_write_new_inode(fs, child, (struct ext2_inode *)&inode);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	inode.i_generation = ff->next_generation++;
	init_times(&inode);
	err = ext2fs_write_inode_full(fs, child, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	ext2fs_inode_alloc_stats2(fs, child, 1, 0);

out2:
	pthread_mutex_unlock(&ff->bfl);
out:
	free(temp_path);
	return ret;
}

static int op_mkdir(const char *path, mode_t mode)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t parent, child;
	char *temp_path;
	errcode_t err;
	char *node_name, a;
	struct ext2_inode_large inode;
	char *block;
	blk64_t blk;
	int ret = 0;
	mode_t parent_sgid;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf("%s: path=%s mode=0%o\n", __func__, path, mode);
	temp_path = strdup(path);
	if (!temp_path) {
		ret = -ENOMEM;
		goto out;
	}
	node_name = strrchr(temp_path, '/');
	if (!node_name) {
		ret = -ENOMEM;
		goto out;
	}
	node_name++;
	a = *node_name;
	*node_name = 0;

	pthread_mutex_lock(&ff->bfl);
	if (!fs_can_allocate(ff, 1)) {
		ret = -ENOSPC;
		goto out2;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &parent);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	ret = check_inum_access(fs, parent, W_OK);
	if (ret)
		goto out2;

	/* Is the parent dir sgid? */
	err = ext2fs_read_inode_full(fs, parent, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}
	parent_sgid = inode.i_mode & S_ISGID;

	*node_name = a;

	err = ext2fs_mkdir(fs, parent, 0, node_name);
	if (err == EXT2_ET_DIR_NO_SPACE) {
		err = ext2fs_expand_dir(fs, parent);
		if (err) {
			ret = translate_error(fs, parent, err);
			goto out2;
		}

		err = ext2fs_mkdir(fs, parent, 0, node_name);
	}
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	ret = update_mtime(fs, parent, NULL);
	if (ret)
		goto out2;

	/* Still have to update the uid/gid of the dir */
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &child);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}
	dbg_printf("%s: created ino=%d/path=%s in dir=%d\n", __func__, child,
		   node_name, parent);

	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, child, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	inode.i_uid = ctxt->uid;
	ext2fs_set_i_uid_high(inode, ctxt->uid >> 16);
	inode.i_gid = ctxt->gid;
	ext2fs_set_i_gid_high(inode, ctxt->gid >> 16);
	inode.i_mode = LINUX_S_IFDIR | (mode & ~(S_ISUID | fs->umask)) |
		       parent_sgid;
	inode.i_generation = ff->next_generation++;

	err = ext2fs_write_inode_full(fs, child, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	/* Rewrite the directory block checksum, having set i_generation */
	if ((inode.i_flags & EXT4_INLINE_DATA_FL) ||
	    !ext2fs_has_feature_metadata_csum(fs->super))
		goto out2;
	err = ext2fs_new_dir_block(fs, child, parent, &block);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}
	err = ext2fs_bmap2(fs, child, (struct ext2_inode *)&inode, NULL, 0, 0,
			   NULL, &blk);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out3;
	}
	err = ext2fs_write_dir_block4(fs, blk, block, 0, child);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out3;
	}

out3:
	ext2fs_free_mem(&block);
out2:
	pthread_mutex_unlock(&ff->bfl);
out:
	free(temp_path);
	return ret;
}

static int unlink_file_by_name(ext2_filsys fs, const char *path)
{
	errcode_t err;
	ext2_ino_t dir;
	char *filename = strdup(path);
	char *base_name;
	int ret;

	base_name = strrchr(filename, '/');
	if (base_name) {
		*base_name++ = '\0';
		err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, filename,
				   &dir);
		if (err) {
			free(filename);
			return translate_error(fs, 0, err);
		}
	} else {
		dir = EXT2_ROOT_INO;
		base_name = filename;
	}

	ret = check_inum_access(fs, dir, W_OK);
	if (ret) {
		free(filename);
		return ret;
	}

	dbg_printf("%s: unlinking name=%s from dir=%d\n", __func__,
		   base_name, dir);
	err = ext2fs_unlink(fs, dir, base_name, 0, 0);
	free(filename);
	if (err)
		return translate_error(fs, dir, err);

	return update_mtime(fs, dir, NULL);
}

static int remove_inode(struct fuse2fs *ff, ext2_ino_t ino)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	struct ext2_inode_large inode;
	int ret = 0;

	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}
	dbg_printf("%s: put ino=%d links=%d\n", __func__, ino,
		   inode.i_links_count);

	switch (inode.i_links_count) {
	case 0:
		return 0; /* XXX: already done? */
	case 1:
		inode.i_links_count--;
		inode.i_dtime = fs->now ? fs->now : time(0);
		break;
	default:
		inode.i_links_count--;
	}

	ret = update_ctime(fs, ino, &inode);
	if (ret)
		goto out;

	if (inode.i_links_count)
		goto write_out;

	/* Nobody holds this file; free its blocks! */
	err = ext2fs_free_ext_attr(fs, ino, &inode);
	if (err)
		goto write_out;

	if (ext2fs_inode_has_valid_blocks2(fs, (struct ext2_inode *)&inode)) {
		err = ext2fs_punch(fs, ino, (struct ext2_inode *)&inode, NULL,
				   0, ~0ULL);
		if (err) {
			ret = translate_error(fs, ino, err);
			goto write_out;
		}
	}

	ext2fs_inode_alloc_stats2(fs, ino, -1,
				  LINUX_S_ISDIR(inode.i_mode));

write_out:
	err = ext2fs_write_inode_full(fs, ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}
out:
	return ret;
}

static int __op_unlink(struct fuse2fs *ff, const char *path)
{
	ext2_filsys fs = ff->fs;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}

	ret = unlink_file_by_name(fs, path);
	if (ret)
		goto out;

	ret = remove_inode(ff, ino);
	if (ret)
		goto out;
out:
	return ret;
}

static int op_unlink(const char *path)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	int ret;

	FUSE2FS_CHECK_CONTEXT(ff);
	pthread_mutex_lock(&ff->bfl);
	ret = __op_unlink(ff, path);
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

struct rd_struct {
	ext2_ino_t	parent;
	int		empty;
};

static int rmdir_proc(ext2_ino_t dir EXT2FS_ATTR((unused)),
		      int	entry EXT2FS_ATTR((unused)),
		      struct ext2_dir_entry *dirent,
		      int	offset EXT2FS_ATTR((unused)),
		      int	blocksize EXT2FS_ATTR((unused)),
		      char	*buf EXT2FS_ATTR((unused)),
		      void	*private)
{
	struct rd_struct *rds = (struct rd_struct *) private;

	if (dirent->inode == 0)
		return 0;
	if (((dirent->name_len & 0xFF) == 1) && (dirent->name[0] == '.'))
		return 0;
	if (((dirent->name_len & 0xFF) == 2) && (dirent->name[0] == '.') &&
	    (dirent->name[1] == '.')) {
		rds->parent = dirent->inode;
		return 0;
	}
	rds->empty = 0;
	return 0;
}

static int __op_rmdir(struct fuse2fs *ff, const char *path)
{
	ext2_filsys fs = ff->fs;
	ext2_ino_t child;
	errcode_t err;
	struct ext2_inode_large inode;
	struct rd_struct rds;
	int ret = 0;

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &child);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf("%s: rmdir path=%s ino=%d\n", __func__, path, child);

	rds.parent = 0;
	rds.empty = 1;

	err = ext2fs_dir_iterate2(fs, child, 0, 0, rmdir_proc, &rds);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out;
	}

	if (rds.empty == 0) {
		ret = -ENOTEMPTY;
		goto out;
	}

	ret = unlink_file_by_name(fs, path);
	if (ret)
		goto out;
	/* Directories have to be "removed" twice. */
	ret = remove_inode(ff, child);
	if (ret)
		goto out;
	ret = remove_inode(ff, child);
	if (ret)
		goto out;

	if (rds.parent) {
		dbg_printf("%s: decr dir=%d link count\n", __func__,
			   rds.parent);
		err = ext2fs_read_inode_full(fs, rds.parent,
					     (struct ext2_inode *)&inode,
					     sizeof(inode));
		if (err) {
			ret = translate_error(fs, rds.parent, err);
			goto out;
		}
		if (inode.i_links_count > 1)
			inode.i_links_count--;
		ret = update_mtime(fs, rds.parent, &inode);
		if (ret)
			goto out;
		err = ext2fs_write_inode_full(fs, rds.parent,
					      (struct ext2_inode *)&inode,
					      sizeof(inode));
		if (err) {
			ret = translate_error(fs, rds.parent, err);
			goto out;
		}
	}

out:
	return ret;
}

static int op_rmdir(const char *path)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	int ret;

	FUSE2FS_CHECK_CONTEXT(ff);
	pthread_mutex_lock(&ff->bfl);
	ret = __op_rmdir(ff, path);
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_symlink(const char *src, const char *dest)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t parent, child;
	char *temp_path;
	errcode_t err;
	char *node_name, a;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf("%s: symlink %s to %s\n", __func__, src, dest);
	temp_path = strdup(dest);
	if (!temp_path) {
		ret = -ENOMEM;
		goto out;
	}
	node_name = strrchr(temp_path, '/');
	if (!node_name) {
		ret = -ENOMEM;
		goto out;
	}
	node_name++;
	a = *node_name;
	*node_name = 0;

	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &parent);
	*node_name = a;
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	ret = check_inum_access(fs, parent, W_OK);
	if (ret)
		goto out2;


	/* Create symlink */
	err = ext2fs_symlink(fs, parent, 0, node_name, src);
	if (err == EXT2_ET_DIR_NO_SPACE) {
		err = ext2fs_expand_dir(fs, parent);
		if (err) {
			ret = translate_error(fs, parent, err);
			goto out2;
		}

		err = ext2fs_symlink(fs, parent, 0, node_name, src);
	}
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	/* Update parent dir's mtime */
	ret = update_mtime(fs, parent, NULL);
	if (ret)
		goto out2;

	/* Still have to update the uid/gid of the symlink */
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &child);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}
	dbg_printf("%s: symlinking ino=%d/name=%s to dir=%d\n", __func__,
		   child, node_name, parent);

	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, child, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	inode.i_uid = ctxt->uid;
	ext2fs_set_i_uid_high(inode, ctxt->uid >> 16);
	inode.i_gid = ctxt->gid;
	ext2fs_set_i_gid_high(inode, ctxt->gid >> 16);
	inode.i_generation = ff->next_generation++;

	err = ext2fs_write_inode_full(fs, child, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}
out2:
	pthread_mutex_unlock(&ff->bfl);
out:
	free(temp_path);
	return ret;
}

struct update_dotdot {
	ext2_ino_t new_dotdot;
};

static int update_dotdot_helper(ext2_ino_t dir EXT2FS_ATTR((unused)),
				int entry EXT2FS_ATTR((unused)),
				struct ext2_dir_entry *dirent,
				int offset EXT2FS_ATTR((unused)),
				int blocksize EXT2FS_ATTR((unused)),
				char *buf EXT2FS_ATTR((unused)),
				void *priv_data)
{
	struct update_dotdot *ud = priv_data;

	if (ext2fs_dirent_name_len(dirent) == 2 &&
	    dirent->name[0] == '.' && dirent->name[1] == '.') {
		dirent->inode = ud->new_dotdot;
		return DIRENT_CHANGED | DIRENT_ABORT;
	}

	return 0;
}

static int op_rename(const char *from, const char *to)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t from_ino, to_ino, to_dir_ino, from_dir_ino;
	char *temp_to = NULL, *temp_from = NULL;
	char *cp, a;
	struct ext2_inode inode;
	struct update_dotdot ud;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf("%s: renaming %s to %s\n", __func__, from, to);
	pthread_mutex_lock(&ff->bfl);
	if (!fs_can_allocate(ff, 5)) {
		ret = -ENOSPC;
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, from, &from_ino);
	if (err || from_ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, to, &to_ino);
	if (err && err != EXT2_ET_FILE_NOT_FOUND) {
		ret = translate_error(fs, 0, err);
		goto out;
	}

	if (err == EXT2_ET_FILE_NOT_FOUND)
		to_ino = 0;

	/* Already the same file? */
	if (to_ino != 0 && to_ino == from_ino) {
		ret = 0;
		goto out;
	}

	temp_to = strdup(to);
	if (!temp_to) {
		ret = -ENOMEM;
		goto out;
	}

	temp_from = strdup(from);
	if (!temp_from) {
		ret = -ENOMEM;
		goto out2;
	}

	/* Find parent dir of the source and check write access */
	cp = strrchr(temp_from, '/');
	if (!cp) {
		ret = -EINVAL;
		goto out2;
	}

	a = *(cp + 1);
	*(cp + 1) = 0;
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_from,
			   &from_dir_ino);
	*(cp + 1) = a;
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}
	if (from_dir_ino == 0) {
		ret = -ENOENT;
		goto out2;
	}

	ret = check_inum_access(fs, from_dir_ino, W_OK);
	if (ret)
		goto out2;

	/* Find parent dir of the destination and check write access */
	cp = strrchr(temp_to, '/');
	if (!cp) {
		ret = -EINVAL;
		goto out2;
	}

	a = *(cp + 1);
	*(cp + 1) = 0;
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_to,
			   &to_dir_ino);
	*(cp + 1) = a;
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}
	if (to_dir_ino == 0) {
		ret = -ENOENT;
		goto out2;
	}

	ret = check_inum_access(fs, to_dir_ino, W_OK);
	if (ret)
		goto out2;

	/* If the target exists, unlink it first */
	if (to_ino != 0) {
		err = ext2fs_read_inode(fs, to_ino, &inode);
		if (err) {
			ret = translate_error(fs, to_ino, err);
			goto out2;
		}

		dbg_printf("%s: unlinking %s ino=%d\n", __func__,
			   LINUX_S_ISDIR(inode.i_mode) ? "dir" : "file",
			   to_ino);
		if (LINUX_S_ISDIR(inode.i_mode))
			ret = __op_rmdir(ff, to);
		else
			ret = __op_unlink(ff, to);
		if (ret)
			goto out2;
	}

	/* Get ready to do the move */
	err = ext2fs_read_inode(fs, from_ino, &inode);
	if (err) {
		ret = translate_error(fs, from_ino, err);
		goto out2;
	}

	/* Link in the new file */
	dbg_printf("%s: linking ino=%d/path=%s to dir=%d\n", __func__,
		   from_ino, cp + 1, to_dir_ino);
	err = ext2fs_link(fs, to_dir_ino, cp + 1, from_ino,
			  ext2_file_type(inode.i_mode));
	if (err == EXT2_ET_DIR_NO_SPACE) {
		err = ext2fs_expand_dir(fs, to_dir_ino);
		if (err) {
			ret = translate_error(fs, to_dir_ino, err);
			goto out2;
		}

		err = ext2fs_link(fs, to_dir_ino, cp + 1, from_ino,
				     ext2_file_type(inode.i_mode));
	}
	if (err) {
		ret = translate_error(fs, to_dir_ino, err);
		goto out2;
	}

	/* Update '..' pointer if dir */
	err = ext2fs_read_inode(fs, from_ino, &inode);
	if (err) {
		ret = translate_error(fs, from_ino, err);
		goto out2;
	}

	if (LINUX_S_ISDIR(inode.i_mode)) {
		ud.new_dotdot = to_dir_ino;
		dbg_printf("%s: updating .. entry for dir=%d\n", __func__,
			   to_dir_ino);
		err = ext2fs_dir_iterate2(fs, from_ino, 0, NULL,
					  update_dotdot_helper, &ud);
		if (err) {
			ret = translate_error(fs, from_ino, err);
			goto out2;
		}

		/* Decrease from_dir_ino's links_count */
		dbg_printf("%s: moving linkcount from dir=%d to dir=%d\n",
			   __func__, from_dir_ino, to_dir_ino);
		err = ext2fs_read_inode(fs, from_dir_ino, &inode);
		if (err) {
			ret = translate_error(fs, from_dir_ino, err);
			goto out2;
		}
		inode.i_links_count--;
		err = ext2fs_write_inode(fs, from_dir_ino, &inode);
		if (err) {
			ret = translate_error(fs, from_dir_ino, err);
			goto out2;
		}

		/* Increase to_dir_ino's links_count */
		err = ext2fs_read_inode(fs, to_dir_ino, &inode);
		if (err) {
			ret = translate_error(fs, to_dir_ino, err);
			goto out2;
		}
		inode.i_links_count++;
		err = ext2fs_write_inode(fs, to_dir_ino, &inode);
		if (err) {
			ret = translate_error(fs, to_dir_ino, err);
			goto out2;
		}
	}

	/* Update timestamps */
	ret = update_ctime(fs, from_ino, NULL);
	if (ret)
		goto out2;

	ret = update_mtime(fs, to_dir_ino, NULL);
	if (ret)
		goto out2;

	/* Remove the old file */
	ret = unlink_file_by_name(fs, from);
	if (ret)
		goto out2;

	/* Flush the whole mess out */
	err = ext2fs_flush2(fs, 0);
	if (err)
		ret = translate_error(fs, 0, err);

out2:
	free(temp_from);
	free(temp_to);
out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_link(const char *src, const char *dest)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	char *temp_path;
	errcode_t err;
	char *node_name, a;
	ext2_ino_t parent, ino;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf("%s: src=%s dest=%s\n", __func__, src, dest);
	temp_path = strdup(dest);
	if (!temp_path) {
		ret = -ENOMEM;
		goto out;
	}
	node_name = strrchr(temp_path, '/');
	if (!node_name) {
		ret = -ENOMEM;
		goto out;
	}
	node_name++;
	a = *node_name;
	*node_name = 0;

	pthread_mutex_lock(&ff->bfl);
	if (!fs_can_allocate(ff, 2)) {
		ret = -ENOSPC;
		goto out2;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &parent);
	*node_name = a;
	if (err) {
		err = -ENOENT;
		goto out2;
	}

	ret = check_inum_access(fs, parent, W_OK);
	if (ret)
		goto out2;


	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, src, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	inode.i_links_count++;
	ret = update_ctime(fs, ino, &inode);
	if (ret)
		goto out2;

	err = ext2fs_write_inode_full(fs, ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	dbg_printf("%s: linking ino=%d/name=%s to dir=%d\n", __func__, ino,
		   node_name, parent);
	err = ext2fs_link(fs, parent, node_name, ino,
			  ext2_file_type(inode.i_mode));
	if (err == EXT2_ET_DIR_NO_SPACE) {
		err = ext2fs_expand_dir(fs, parent);
		if (err) {
			ret = translate_error(fs, parent, err);
			goto out2;
		}

		err = ext2fs_link(fs, parent, node_name, ino,
				     ext2_file_type(inode.i_mode));
	}
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	ret = update_mtime(fs, parent, NULL);
	if (ret)
		goto out2;

out2:
	pthread_mutex_unlock(&ff->bfl);
out:
	free(temp_path);
	return ret;
}

static int op_chmod(const char *path, mode_t mode)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t ino;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf("%s: path=%s mode=0%o ino=%d\n", __func__, path, mode, ino);

	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	if (!ff->fakeroot && ctxt->uid != 0 && ctxt->uid != inode_uid(inode)) {
		ret = -EPERM;
		goto out;
	}

	/*
	 * XXX: We should really check that the inode gid is not in /any/
	 * of the user's groups, but FUSE only tells us about the primary
	 * group.
	 */
	if (!ff->fakeroot && ctxt->uid != 0 && ctxt->gid != inode_gid(inode))
		mode &= ~S_ISGID;

	inode.i_mode &= ~0xFFF;
	inode.i_mode |= mode & 0xFFF;
	ret = update_ctime(fs, ino, &inode);
	if (ret)
		goto out;

	err = ext2fs_write_inode_full(fs, ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_chown(const char *path, uid_t owner, gid_t group)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t ino;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf("%s: path=%s owner=%d group=%d ino=%d\n", __func__,
		   path, owner, group, ino);

	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	/* FUSE seems to feed us ~0 to mean "don't change" */
	if (owner != (uid_t) ~0) {
		/* Only root gets to change UID. */
		if (!ff->fakeroot && ctxt->uid != 0 &&
		    !(inode_uid(inode) == ctxt->uid && owner == ctxt->uid)) {
			ret = -EPERM;
			goto out;
		}
		inode.i_uid = owner;
		ext2fs_set_i_uid_high(inode, owner >> 16);
	}

	if (group != (gid_t) ~0) {
		/* Only root or the owner get to change GID. */
		if (!ff->fakeroot && ctxt->uid != 0 &&
		    inode_uid(inode) != ctxt->uid) {
			ret = -EPERM;
			goto out;
		}

		/* XXX: We /should/ check group membership but FUSE */
		inode.i_gid = group;
		ext2fs_set_i_gid_high(inode, group >> 16);
	}

	ret = update_ctime(fs, ino, &inode);
	if (ret)
		goto out;

	err = ext2fs_write_inode_full(fs, ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_truncate(const char *path, off_t len)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t ino;
	ext2_file_t file;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf("%s: ino=%d len=%jd\n", __func__, ino, len);

	ret = check_inum_access(fs, ino, W_OK);
	if (ret)
		goto out;

	err = ext2fs_file_open(fs, ino, EXT2_FILE_WRITE, &file);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	err = ext2fs_file_set_size2(file, len);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

out2:
	err = ext2fs_file_close(file);
	if (ret)
		goto out;
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	ret = update_mtime(fs, ino, NULL);

out:
	pthread_mutex_unlock(&ff->bfl);
	return err;
}

#ifdef __linux__
static void detect_linux_executable_open(int kernel_flags, int *access_check,
				  int *e2fs_open_flags)
{
	/*
	 * On Linux, execve will bleed __FMODE_EXEC into the file mode flags,
	 * and FUSE is more than happy to let that slip through.
	 */
	if (kernel_flags & 0x20) {
		*access_check = X_OK;
		*e2fs_open_flags &= ~EXT2_FILE_WRITE;
	}
}
#else
static void detect_linux_executable_open(int kernel_flags, int *access_check,
				  int *e2fs_open_flags)
{
	/* empty */
}
#endif /* __linux__ */

static int __op_open(struct fuse2fs *ff, const char *path,
		     struct fuse_file_info *fp)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	struct fuse2fs_file_handle *file;
	int check = 0, ret = 0;

	dbg_printf("%s: path=%s\n", __func__, path);
	err = ext2fs_get_mem(sizeof(*file), &file);
	if (err)
		return translate_error(fs, 0, err);
	file->magic = FUSE2FS_FILE_MAGIC;

	file->open_flags = 0;
	switch (fp->flags & O_ACCMODE) {
	case O_RDONLY:
		check = R_OK;
		break;
	case O_WRONLY:
		check = W_OK;
		file->open_flags |= EXT2_FILE_WRITE;
		break;
	case O_RDWR:
		check = R_OK | W_OK;
		file->open_flags |= EXT2_FILE_WRITE;
		break;
	}

	detect_linux_executable_open(fp->flags, &check, &file->open_flags);

	if (fp->flags & O_CREAT)
		file->open_flags |= EXT2_FILE_CREATE;

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &file->ino);
	if (err || file->ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf("%s: ino=%d\n", __func__, file->ino);

	ret = check_inum_access(fs, file->ino, check);
	if (ret) {
		/*
		 * In a regular (Linux) fs driver, the kernel will open
		 * binaries for reading if the user has --x privileges (i.e.
		 * execute without read).  Since the kernel doesn't have any
		 * way to tell us if it's opening a file via execve, we'll
		 * just assume that allowing access is ok if asking for ro mode
		 * fails but asking for x mode succeeds.  Of course we can
		 * also employ undocumented hacks (see above).
		 */
		if (check == R_OK) {
			ret = check_inum_access(fs, file->ino, X_OK);
			if (ret)
				goto out;
		} else
			goto out;
	}
	fp->fh = (uintptr_t)file;

out:
	if (ret)
		ext2fs_free_mem(&file);
	return ret;
}

static int op_open(const char *path, struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	int ret;

	FUSE2FS_CHECK_CONTEXT(ff);
	pthread_mutex_lock(&ff->bfl);
	ret = __op_open(ff, path, fp);
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_read(const char *path EXT2FS_ATTR((unused)), char *buf,
		   size_t len, off_t offset,
		   struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	ext2_file_t efp;
	errcode_t err;
	unsigned int got = 0;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: ino=%d off=%jd len=%jd\n", __func__, fh->ino, offset,
		   len);
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_file_open(fs, fh->ino, fh->open_flags, &efp);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	err = ext2fs_file_llseek(efp, offset, SEEK_SET, NULL);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

	err = ext2fs_file_read(efp, buf, len, &got);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

out2:
	err = ext2fs_file_close(efp);
	if (ret)
		goto out;
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	if (fs_writeable(fs)) {
		ret = update_atime(fs, fh->ino);
		if (ret)
			goto out;
	}
out:
	pthread_mutex_unlock(&ff->bfl);
	return got ? (int) got : ret;
}

static int op_write(const char *path EXT2FS_ATTR((unused)),
		    const char *buf, size_t len, off_t offset,
		    struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	ext2_file_t efp;
	errcode_t err;
	unsigned int got = 0;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: ino=%d off=%jd len=%jd\n", __func__, fh->ino, offset,
		   len);
	pthread_mutex_lock(&ff->bfl);
	if (!fs_writeable(fs)) {
		ret = -EROFS;
		goto out;
	}

	if (!fs_can_allocate(ff, len / fs->blocksize)) {
		ret = -ENOSPC;
		goto out;
	}

	err = ext2fs_file_open(fs, fh->ino, fh->open_flags, &efp);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	err = ext2fs_file_llseek(efp, offset, SEEK_SET, NULL);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

	err = ext2fs_file_write(efp, buf, len, &got);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

	err = ext2fs_file_flush(efp);
	if (err) {
		got = 0;
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

out2:
	err = ext2fs_file_close(efp);
	if (ret)
		goto out;
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	ret = update_mtime(fs, fh->ino, NULL);
	if (ret)
		goto out;

out:
	pthread_mutex_unlock(&ff->bfl);
	return got ? (int) got : ret;
}

static int op_release(const char *path EXT2FS_ATTR((unused)),
		      struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: ino=%d\n", __func__, fh->ino);
	pthread_mutex_lock(&ff->bfl);
	if (fs_writeable(fs) && fh->open_flags & EXT2_FILE_WRITE) {
		err = ext2fs_flush2(fs, EXT2_FLAG_FLUSH_NO_SYNC);
		if (err)
			ret = translate_error(fs, fh->ino, err);
	}
	fp->fh = 0;
	pthread_mutex_unlock(&ff->bfl);

	ext2fs_free_mem(&fh);

	return ret;
}

static int op_fsync(const char *path EXT2FS_ATTR((unused)),
		    int datasync EXT2FS_ATTR((unused)),
		    struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: ino=%d\n", __func__, fh->ino);
	/* For now, flush everything, even if it's slow */
	pthread_mutex_lock(&ff->bfl);
	if (fs_writeable(fs) && fh->open_flags & EXT2_FILE_WRITE) {
		err = ext2fs_flush2(fs, 0);
		if (err)
			ret = translate_error(fs, fh->ino, err);
	}
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}

static int op_statfs(const char *path EXT2FS_ATTR((unused)),
		     struct statvfs *buf)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	uint64_t fsid, *f;
	blk64_t overhead, reserved, free;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf("%s: path=%s\n", __func__, path);
	buf->f_bsize = fs->blocksize;
	buf->f_frsize = 0;

	if (ff->minixdf)
		overhead = 0;
	else
		overhead = fs->desc_blocks +
			   (blk64_t)fs->group_desc_count *
			   (fs->inode_blocks_per_group + 2);
	reserved = ext2fs_r_blocks_count(fs->super);
	if (!reserved)
		reserved = ext2fs_blocks_count(fs->super) / 10;
	free = ext2fs_free_blocks_count(fs->super);

	buf->f_blocks = ext2fs_blocks_count(fs->super) - overhead;
	buf->f_bfree = free;
	if (free < reserved)
		buf->f_bavail = 0;
	else
		buf->f_bavail = free - reserved;
	buf->f_files = fs->super->s_inodes_count;
	buf->f_ffree = fs->super->s_free_inodes_count;
	buf->f_favail = fs->super->s_free_inodes_count;
	f = (uint64_t *)fs->super->s_uuid;
	fsid = *f;
	f++;
	fsid ^= *f;
	buf->f_fsid = fsid;
	buf->f_flag = 0;
	if (fs->flags & EXT2_FLAG_RW)
		buf->f_flag |= ST_RDONLY;
	buf->f_namemax = EXT2_NAME_LEN;

	return 0;
}

typedef errcode_t (*xattr_xlate_get)(void **cooked_buf, size_t *cooked_sz,
				     const void *raw_buf, size_t raw_sz);
typedef errcode_t (*xattr_xlate_set)(const void *cooked_buf, size_t cooked_sz,
				     const void **raw_buf, size_t *raw_sz);
struct xattr_translate {
	const char *prefix;
	xattr_xlate_get get;
	xattr_xlate_set set;
};

#define XATTR_TRANSLATOR(p, g, s) \
	{.prefix = (p), \
	 .get = (xattr_xlate_get)(g), \
	 .set = (xattr_xlate_set)(s)}

static struct xattr_translate xattr_translators[] = {
#ifdef TRANSLATE_LINUX_ACLS
	XATTR_TRANSLATOR(ACL_EA_ACCESS, ext4_to_fuse_acl, fuse_to_ext4_acl),
	XATTR_TRANSLATOR(ACL_EA_DEFAULT, ext4_to_fuse_acl, fuse_to_ext4_acl),
#endif
	XATTR_TRANSLATOR(NULL, NULL, NULL),
};
#undef XATTR_TRANSLATOR

static int op_getxattr(const char *path, const char *key, char *value,
		       size_t len)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	struct ext2_xattr_handle *h;
	struct xattr_translate *xt;
	void *ptr, *cptr;
	size_t plen, clen;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	if (!ext2fs_has_feature_xattr(fs->super)) {
		ret = -ENOTSUP;
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf("%s: ino=%d\n", __func__, ino);

	ret = check_inum_access(fs, ino, R_OK);
	if (ret)
		goto out;

	err = ext2fs_xattrs_open(fs, ino, &h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	err = ext2fs_xattrs_read(h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	err = ext2fs_xattr_get(h, key, &ptr, &plen);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	for (xt = xattr_translators; xt->prefix != NULL; xt++) {
		if (strncmp(key, xt->prefix, strlen(xt->prefix)) == 0) {
			err = xt->get(&cptr, &clen, ptr, plen);
			if (err)
				goto out3;
			ext2fs_free_mem(&ptr);
			ptr = cptr;
			plen = clen;
		}
	}

	if (!len) {
		ret = plen;
	} else if (len < plen) {
		ret = -ERANGE;
	} else {
		memcpy(value, ptr, plen);
		ret = plen;
	}

out3:
	ext2fs_free_mem(&ptr);
out2:
	err = ext2fs_xattrs_close(&h);
	if (err)
		ret = translate_error(fs, ino, err);
out:
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}

static int count_buffer_space(char *name, char *value EXT2FS_ATTR((unused)),
			      size_t value_len EXT2FS_ATTR((unused)),
			      void *data)
{
	unsigned int *x = data;

	*x = *x + strlen(name) + 1;
	return 0;
}

static int copy_names(char *name, char *value EXT2FS_ATTR((unused)),
		      size_t value_len EXT2FS_ATTR((unused)), void *data)
{
	char **b = data;

	strncpy(*b, name, strlen(name));
	*b = *b + strlen(name) + 1;

	return 0;
}

static int op_listxattr(const char *path, char *names, size_t len)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	struct ext2_xattr_handle *h;
	unsigned int bufsz;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	if (!ext2fs_has_feature_xattr(fs->super)) {
		ret = -ENOTSUP;
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, ino, err);
		goto out;
	}
	dbg_printf("%s: ino=%d\n", __func__, ino);

	ret = check_inum_access(fs, ino, R_OK);
	if (ret)
		goto out;

	err = ext2fs_xattrs_open(fs, ino, &h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	err = ext2fs_xattrs_read(h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	/* Count buffer space needed for names */
	bufsz = 0;
	err = ext2fs_xattrs_iterate(h, count_buffer_space, &bufsz);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	if (len == 0) {
		ret = bufsz;
		goto out2;
	} else if (len < bufsz) {
		ret = -ERANGE;
		goto out2;
	}

	/* Copy names out */
	memset(names, 0, len);
	err = ext2fs_xattrs_iterate(h, copy_names, &names);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}
	ret = bufsz;
out2:
	err = ext2fs_xattrs_close(&h);
	if (err)
		ret = translate_error(fs, ino, err);
out:
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}

static int op_setxattr(const char *path EXT2FS_ATTR((unused)),
		       const char *key, const char *value,
		       size_t len, int flags EXT2FS_ATTR((unused)))
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	struct ext2_xattr_handle *h;
	struct xattr_translate *xt;
	const void *cvalue;
	size_t clen;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	if (!ext2fs_has_feature_xattr(fs->super)) {
		ret = -ENOTSUP;
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf("%s: ino=%d\n", __func__, ino);

	ret = check_inum_access(fs, ino, W_OK);
	if (ret == -EACCES) {
		ret = -EPERM;
		goto out;
	} else if (ret)
		goto out;

	err = ext2fs_xattrs_open(fs, ino, &h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	err = ext2fs_xattrs_read(h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	cvalue = value;
	clen = len;
	for (xt = xattr_translators; xt->prefix != NULL; xt++) {
		if (strncmp(key, xt->prefix, strlen(xt->prefix)) == 0) {
			err = xt->set(value, len, &cvalue, &clen);
			if (err)
				goto out3;
		}
	}

	err = ext2fs_xattr_set(h, key, cvalue, clen);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out3;
	}

	ret = update_ctime(fs, ino, NULL);
out3:
	if (cvalue != value)
		ext2fs_free_mem(&cvalue);
out2:
	err = ext2fs_xattrs_close(&h);
	if (!ret && err)
		ret = translate_error(fs, ino, err);
out:
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}

static int op_removexattr(const char *path, const char *key)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	struct ext2_xattr_handle *h;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	if (!ext2fs_has_feature_xattr(fs->super)) {
		ret = -ENOTSUP;
		goto out;
	}

	if (!fs_can_allocate(ff, 1)) {
		ret = -ENOSPC;
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf("%s: ino=%d\n", __func__, ino);

	ret = check_inum_access(fs, ino, W_OK);
	if (ret)
		goto out;

	err = ext2fs_xattrs_open(fs, ino, &h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	err = ext2fs_xattrs_read(h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	err = ext2fs_xattr_remove(h, key);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	ret = update_ctime(fs, ino, NULL);
out2:
	err = ext2fs_xattrs_close(&h);
	if (err)
		ret = translate_error(fs, ino, err);
out:
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}

struct readdir_iter {
	void *buf;
	fuse_fill_dir_t func;
};

static int op_readdir_iter(ext2_ino_t dir EXT2FS_ATTR((unused)),
			   int entry EXT2FS_ATTR((unused)),
			   struct ext2_dir_entry *dirent,
			   int offset EXT2FS_ATTR((unused)),
			   int blocksize EXT2FS_ATTR((unused)),
			   char *buf EXT2FS_ATTR((unused)), void *data)
{
	struct readdir_iter *i = data;
	char namebuf[EXT2_NAME_LEN + 1];
	int ret;

	memcpy(namebuf, dirent->name, dirent->name_len & 0xFF);
	namebuf[dirent->name_len & 0xFF] = 0;
	ret = i->func(i->buf, namebuf, NULL, 0);
	if (ret)
		return DIRENT_ABORT;

	return 0;
}

static int op_readdir(const char *path EXT2FS_ATTR((unused)),
		      void *buf, fuse_fill_dir_t fill_func,
		      off_t offset EXT2FS_ATTR((unused)),
		      struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	errcode_t err;
	struct readdir_iter i;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: ino=%d\n", __func__, fh->ino);
	pthread_mutex_lock(&ff->bfl);
	i.buf = buf;
	i.func = fill_func;
	err = ext2fs_dir_iterate2(fs, fh->ino, 0, NULL, op_readdir_iter, &i);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	if (fs_writeable(fs)) {
		ret = update_atime(fs, fh->ino);
		if (ret)
			goto out;
	}
out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_access(const char *path, int mask)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t ino;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf("%s: path=%s mask=0x%x\n", __func__, path, mask);
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}

	ret = check_inum_access(fs, ino, mask);
	if (ret)
		goto out;

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_create(const char *path, mode_t mode, struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t parent, child;
	char *temp_path;
	errcode_t err;
	char *node_name, a;
	int filetype;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf("%s: path=%s mode=0%o\n", __func__, path, mode);
	temp_path = strdup(path);
	if (!temp_path) {
		ret = -ENOMEM;
		goto out;
	}
	node_name = strrchr(temp_path, '/');
	if (!node_name) {
		ret = -ENOMEM;
		goto out;
	}
	node_name++;
	a = *node_name;
	*node_name = 0;

	pthread_mutex_lock(&ff->bfl);
	if (!fs_can_allocate(ff, 1)) {
		ret = -ENOSPC;
		goto out2;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &parent);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	ret = check_inum_access(fs, parent, W_OK);
	if (ret)
		goto out2;

	*node_name = a;

	filetype = ext2_file_type(mode);

	err = ext2fs_new_inode(fs, parent, mode, 0, &child);
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	dbg_printf("%s: creating ino=%d/name=%s in dir=%d\n", __func__, child,
		   node_name, parent);
	err = ext2fs_link(fs, parent, node_name, child, filetype);
	if (err == EXT2_ET_DIR_NO_SPACE) {
		err = ext2fs_expand_dir(fs, parent);
		if (err) {
			ret = translate_error(fs, parent, err);
			goto out2;
		}

		err = ext2fs_link(fs, parent, node_name, child,
				     filetype);
	}
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	ret = update_mtime(fs, parent, NULL);
	if (ret)
		goto out2;

	memset(&inode, 0, sizeof(inode));
	inode.i_mode = mode;
	inode.i_links_count = 1;
	inode.i_extra_isize = sizeof(struct ext2_inode_large) -
		EXT2_GOOD_OLD_INODE_SIZE;
	inode.i_uid = ctxt->uid;
	ext2fs_set_i_uid_high(inode, ctxt->uid >> 16);
	inode.i_gid = ctxt->gid;
	ext2fs_set_i_gid_high(inode, ctxt->gid >> 16);
	if (ext2fs_has_feature_extents(fs->super)) {
		ext2_extent_handle_t handle;

		inode.i_flags &= ~EXT4_EXTENTS_FL;
		ret = ext2fs_extent_open2(fs, child,
					  (struct ext2_inode *)&inode, &handle);
		if (ret)
			return ret;
		ext2fs_extent_free(handle);
	}

	err = ext2fs_write_new_inode(fs, child, (struct ext2_inode *)&inode);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	inode.i_generation = ff->next_generation++;
	init_times(&inode);
	err = ext2fs_write_inode_full(fs, child, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	ext2fs_inode_alloc_stats2(fs, child, 1, 0);

	ret = __op_open(ff, path, fp);
	if (ret)
		goto out2;
out2:
	pthread_mutex_unlock(&ff->bfl);
out:
	free(temp_path);
	return ret;
}

static int op_ftruncate(const char *path EXT2FS_ATTR((unused)),
			off_t len, struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	ext2_file_t efp;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: ino=%d len=%jd\n", __func__, fh->ino, len);
	pthread_mutex_lock(&ff->bfl);
	if (!fs_writeable(fs)) {
		ret = -EROFS;
		goto out;
	}

	err = ext2fs_file_open(fs, fh->ino, fh->open_flags, &efp);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	err = ext2fs_file_set_size2(efp, len);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

out2:
	err = ext2fs_file_close(efp);
	if (ret)
		goto out;
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	ret = update_mtime(fs, fh->ino, NULL);
	if (ret)
		goto out;

out:
	pthread_mutex_unlock(&ff->bfl);
	return 0;
}

static int op_fgetattr(const char *path EXT2FS_ATTR((unused)),
		       struct stat *statbuf,
		       struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: ino=%d\n", __func__, fh->ino);
	pthread_mutex_lock(&ff->bfl);
	ret = stat_inode(fs, fh->ino, statbuf);
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}

static int op_utimens(const char *path, const struct timespec ctv[2])
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct timespec tv[2];
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t ino;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf("%s: ino=%d\n", __func__, ino);

	ret = check_inum_access(fs, ino, W_OK);
	if (ret)
		goto out;

	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	tv[0] = ctv[0];
	tv[1] = ctv[1];
#ifdef UTIME_NOW
	if (tv[0].tv_nsec == UTIME_NOW)
		get_now(tv);
	if (tv[1].tv_nsec == UTIME_NOW)
		get_now(tv + 1);
#endif /* UTIME_NOW */
#ifdef UTIME_OMIT
	if (tv[0].tv_nsec != UTIME_OMIT)
		EXT4_INODE_SET_XTIME(i_atime, tv, &inode);
	if (tv[1].tv_nsec != UTIME_OMIT)
		EXT4_INODE_SET_XTIME(i_mtime, tv + 1, &inode);
#endif /* UTIME_OMIT */
	ret = update_ctime(fs, ino, &inode);
	if (ret)
		goto out;

	err = ext2fs_write_inode_full(fs, ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

#ifdef SUPPORT_I_FLAGS
static int ioctl_getflags(ext2_filsys fs, struct fuse2fs_file_handle *fh,
			  void *data)
{
	errcode_t err;
	struct ext2_inode_large inode;

	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: ino=%d\n", __func__, fh->ino);
	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, fh->ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err)
		return translate_error(fs, fh->ino, err);

	*(__u32 *)data = inode.i_flags & EXT2_FL_USER_VISIBLE;
	return 0;
}

#define FUSE2FS_MODIFIABLE_IFLAGS \
	(EXT2_IMMUTABLE_FL | EXT2_APPEND_FL | EXT2_NODUMP_FL | \
	 EXT2_NOATIME_FL | EXT3_JOURNAL_DATA_FL | EXT2_DIRSYNC_FL | \
	 EXT2_TOPDIR_FL)

static int ioctl_setflags(ext2_filsys fs, struct fuse2fs_file_handle *fh,
			  void *data)
{
	errcode_t err;
	struct ext2_inode_large inode;
	int ret;
	__u32 flags = *(__u32 *)data;
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;

	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: ino=%d\n", __func__, fh->ino);
	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, fh->ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err)
		return translate_error(fs, fh->ino, err);

	if (!ff->fakeroot && ctxt->uid != 0 && inode_uid(inode) != ctxt->uid)
		return -EPERM;

	if ((inode.i_flags ^ flags) & ~FUSE2FS_MODIFIABLE_IFLAGS)
		return -EINVAL;

	inode.i_flags = (inode.i_flags & ~FUSE2FS_MODIFIABLE_IFLAGS) |
			(flags & FUSE2FS_MODIFIABLE_IFLAGS);

	ret = update_ctime(fs, fh->ino, &inode);
	if (ret)
		return ret;

	err = ext2fs_write_inode_full(fs, fh->ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err)
		return translate_error(fs, fh->ino, err);

	return 0;
}

static int ioctl_getversion(ext2_filsys fs, struct fuse2fs_file_handle *fh,
			    void *data)
{
	errcode_t err;
	struct ext2_inode_large inode;

	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: ino=%d\n", __func__, fh->ino);
	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, fh->ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err)
		return translate_error(fs, fh->ino, err);

	*(__u32 *)data = inode.i_generation;
	return 0;
}

static int ioctl_setversion(ext2_filsys fs, struct fuse2fs_file_handle *fh,
			    void *data)
{
	errcode_t err;
	struct ext2_inode_large inode;
	int ret;
	__u32 generation = *(__u32 *)data;
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;

	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: ino=%d\n", __func__, fh->ino);
	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, fh->ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err)
		return translate_error(fs, fh->ino, err);

	if (!ff->fakeroot && ctxt->uid != 0 && inode_uid(inode) != ctxt->uid)
		return -EPERM;

	inode.i_generation = generation;

	ret = update_ctime(fs, fh->ino, &inode);
	if (ret)
		return ret;

	err = ext2fs_write_inode_full(fs, fh->ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err)
		return translate_error(fs, fh->ino, err);

	return 0;
}
#endif /* SUPPORT_I_FLAGS */

#ifdef FITRIM
static int ioctl_fitrim(ext2_filsys fs, struct fuse2fs_file_handle *fh,
			void *data)
{
	struct fstrim_range *fr = data;
	blk64_t start, end, max_blocks, b, cleared;
	errcode_t err = 0;

	start = fr->start / fs->blocksize;
	end = (fr->start + fr->len - 1) / fs->blocksize;
	dbg_printf("%s: start=%llu end=%llu\n", __func__, start, end);

	if (start < fs->super->s_first_data_block)
		start = fs->super->s_first_data_block;
	if (start >= ext2fs_blocks_count(fs->super))
		start = ext2fs_blocks_count(fs->super) - 1;

	if (end < fs->super->s_first_data_block)
		end = fs->super->s_first_data_block;
	if (end >= ext2fs_blocks_count(fs->super))
		end = ext2fs_blocks_count(fs->super) - 1;

	cleared = 0;
	max_blocks = 2048ULL * 1024 * 1024 / fs->blocksize;

	fr->len = 0;
	while (start <= end) {
		err = ext2fs_find_first_zero_block_bitmap2(fs->block_map,
							   start, end, &start);
		if (err == ENOENT)
			return 0;
		else if (err)
			return translate_error(fs, fh->ino, err);

		b = start + max_blocks < end ? start + max_blocks : end;
		err =  ext2fs_find_first_set_block_bitmap2(fs->block_map,
							   start, b, &b);
		if (err && err != ENOENT)
			return translate_error(fs, fh->ino, err);
		if (b - start >= fr->minlen) {
			err = io_channel_discard(fs->io, start, b - start);
			if (err)
				return translate_error(fs, fh->ino, err);
			cleared += b - start;
			fr->len = cleared * fs->blocksize;
		}
		start = b + 1;
	}

	return err;
}
#endif /* FITRIM */

#if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 8)
static int op_ioctl(const char *path EXT2FS_ATTR((unused)), int cmd,
		    void *arg EXT2FS_ATTR((unused)),
		    struct fuse_file_info *fp,
		    unsigned int flags EXT2FS_ATTR((unused)), void *data)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	switch ((unsigned long) cmd) {
#ifdef SUPPORT_I_FLAGS
	case EXT2_IOC_GETFLAGS:
		ret = ioctl_getflags(fs, fh, data);
		break;
	case EXT2_IOC_SETFLAGS:
		ret = ioctl_setflags(fs, fh, data);
		break;
	case EXT2_IOC_GETVERSION:
		ret = ioctl_getversion(fs, fh, data);
		break;
	case EXT2_IOC_SETVERSION:
		ret = ioctl_setversion(fs, fh, data);
		break;
#endif
#ifdef FITRIM
	case FITRIM:
		ret = ioctl_fitrim(fs, fh, data);
		break;
#endif
	default:
		dbg_printf("%s: Unknown ioctl %d\n", __func__, cmd);
		ret = -ENOTTY;
	}
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}
#endif /* FUSE 28 */

static int op_bmap(const char *path, size_t blocksize EXT2FS_ATTR((unused)),
		   uint64_t *idx)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf("%s: ino=%d blk=%"PRIu64"\n", __func__, ino, *idx);

	err = ext2fs_bmap2(fs, ino, NULL, NULL, 0, *idx, 0, (blk64_t *)idx);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

#if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 9)
# ifdef SUPPORT_FALLOCATE
static int fallocate_helper(struct fuse_file_info *fp, int mode, off_t offset,
			    off_t len)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	struct ext2_inode_large inode;
	blk64_t start, end;
	__u64 fsize;
	errcode_t err;
	int flags;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	start = offset / fs->blocksize;
	end = (offset + len - 1) / fs->blocksize;
	dbg_printf("%s: ino=%d mode=0x%x start=%jd end=%llu\n", __func__,
		   fh->ino, mode, offset / fs->blocksize, end);
	if (!fs_can_allocate(ff, len / fs->blocksize))
		return -ENOSPC;

	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, fh->ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err)
		return err;
	fsize = EXT2_I_SIZE(&inode);

	/* Allocate a bunch of blocks */
	flags = (mode & FL_KEEP_SIZE_FLAG ? 0 :
			EXT2_FALLOCATE_INIT_BEYOND_EOF);
	err = ext2fs_fallocate(fs, flags, fh->ino,
			       (struct ext2_inode *)&inode,
			       ~0ULL, start, end - start + 1);
	if (err && err != EXT2_ET_BLOCK_ALLOC_FAIL)
		return translate_error(fs, fh->ino, err);

	/* Update i_size */
	if (!(mode & FL_KEEP_SIZE_FLAG)) {
		if ((__u64) offset + len > fsize) {
			err = ext2fs_inode_size_set(fs,
						(struct ext2_inode *)&inode,
						offset + len);
			if (err)
				return translate_error(fs, fh->ino, err);
		}
	}

	err = update_mtime(fs, fh->ino, &inode);
	if (err)
		return err;

	err = ext2fs_write_inode_full(fs, fh->ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err)
		return translate_error(fs, fh->ino, err);

	return err;
}

static errcode_t clean_block_middle(ext2_filsys fs, ext2_ino_t ino,
				  struct ext2_inode_large *inode, off_t offset,
				  off_t len, char **buf)
{
	blk64_t blk;
	off_t residue;
	int retflags;
	errcode_t err;

	residue = offset % fs->blocksize;
	if (residue == 0)
		return 0;

	if (!*buf) {
		err = ext2fs_get_mem(fs->blocksize, buf);
		if (err)
			return err;
	}

	err = ext2fs_bmap2(fs, ino, (struct ext2_inode *)inode, *buf, 0,
			   offset / fs->blocksize, &retflags, &blk);
	if (err)
		return err;
	if (!blk || (retflags & BMAP_RET_UNINIT))
		return 0;

	err = io_channel_read_blk(fs->io, blk, 1, *buf);
	if (err)
		return err;

	memset(*buf + residue, 0, len);

	return io_channel_write_blk(fs->io, blk, 1, *buf);
}

static errcode_t clean_block_edge(ext2_filsys fs, ext2_ino_t ino,
				  struct ext2_inode_large *inode, off_t offset,
				  int clean_before, char **buf)
{
	blk64_t blk;
	int retflags;
	off_t residue;
	errcode_t err;

	residue = offset % fs->blocksize;
	if (residue == 0)
		return 0;

	if (!*buf) {
		err = ext2fs_get_mem(fs->blocksize, buf);
		if (err)
			return err;
	}

	err = ext2fs_bmap2(fs, ino, (struct ext2_inode *)inode, *buf, 0,
			   offset / fs->blocksize, &retflags, &blk);
	if (err)
		return err;

	err = io_channel_read_blk(fs->io, blk, 1, *buf);
	if (err)
		return err;
	if (!blk || (retflags & BMAP_RET_UNINIT))
		return 0;

	if (clean_before)
		memset(*buf, 0, residue);
	else
		memset(*buf + residue, 0, fs->blocksize - residue);

	return io_channel_write_blk(fs->io, blk, 1, *buf);
}

static int punch_helper(struct fuse_file_info *fp, int mode, off_t offset,
			off_t len)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	struct ext2_inode_large inode;
	blk64_t start, end;
	errcode_t err;
	char *buf = NULL;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf("%s: offset=%jd len=%jd\n", __func__, offset, len);

	/* kernel ext4 punch requires this flag to be set */
	if (!(mode & FL_KEEP_SIZE_FLAG))
		return -EINVAL;

	/* Punch out a bunch of blocks */
	start = (offset + fs->blocksize - 1) / fs->blocksize;
	end = (offset + len - fs->blocksize) / fs->blocksize;
	dbg_printf("%s: ino=%d mode=0x%x start=%llu end=%llu\n", __func__,
		   fh->ino, mode, start, end);

	memset(&inode, 0, sizeof(inode));
	err = ext2fs_read_inode_full(fs, fh->ino, (struct ext2_inode *)&inode,
				     sizeof(inode));
	if (err)
		return translate_error(fs, fh->ino, err);

	/* Zero everything before the first block and after the last block */
	if ((offset / fs->blocksize) == ((offset + len) / fs->blocksize))
		err = clean_block_middle(fs, fh->ino, &inode, offset,
					 len, &buf);
	else {
		err = clean_block_edge(fs, fh->ino, &inode, offset, 0, &buf);
		if (!err)
			err = clean_block_edge(fs, fh->ino, &inode,
					       offset + len, 1, &buf);
	}
	if (buf)
		ext2fs_free_mem(&buf);
	if (err)
		return translate_error(fs, fh->ino, err);

	/* Unmap full blocks in the middle */
	if (start <= end) {
		err = ext2fs_punch(fs, fh->ino, (struct ext2_inode *)&inode,
				   NULL, start, end);
		if (err)
			return translate_error(fs, fh->ino, err);
	}

	err = update_mtime(fs, fh->ino, &inode);
	if (err)
		return err;

	err = ext2fs_write_inode_full(fs, fh->ino, (struct ext2_inode *)&inode,
				      sizeof(inode));
	if (err)
		return translate_error(fs, fh->ino, err);

	return 0;
}

static int op_fallocate(const char *path EXT2FS_ATTR((unused)), int mode,
			off_t offset, off_t len,
			struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs = ff->fs;
	int ret;

	/* Catch unknown flags */
	if (mode & ~(FL_PUNCH_HOLE_FLAG | FL_KEEP_SIZE_FLAG))
		return -EINVAL;

	pthread_mutex_lock(&ff->bfl);
	if (!fs_writeable(fs)) {
		ret = -EROFS;
		goto out;
	}
	if (mode & FL_PUNCH_HOLE_FLAG)
		ret = punch_helper(fp, mode, offset, len);
	else
		ret = fallocate_helper(fp, mode, offset, len);
out:
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}
# endif /* SUPPORT_FALLOCATE */
#endif /* FUSE 29 */

static struct fuse_operations fs_ops = {
	.init = op_init,
	.destroy = op_destroy,
	.getattr = op_getattr,
	.readlink = op_readlink,
	.mknod = op_mknod,
	.mkdir = op_mkdir,
	.unlink = op_unlink,
	.rmdir = op_rmdir,
	.symlink = op_symlink,
	.rename = op_rename,
	.link = op_link,
	.chmod = op_chmod,
	.chown = op_chown,
	.truncate = op_truncate,
	.open = op_open,
	.read = op_read,
	.write = op_write,
	.statfs = op_statfs,
	.release = op_release,
	.fsync = op_fsync,
	.setxattr = op_setxattr,
	.getxattr = op_getxattr,
	.listxattr = op_listxattr,
	.removexattr = op_removexattr,
	.opendir = op_open,
	.readdir = op_readdir,
	.releasedir = op_release,
	.fsyncdir = op_fsync,
	.access = op_access,
	.create = op_create,
	.ftruncate = op_ftruncate,
	.fgetattr = op_fgetattr,
	.utimens = op_utimens,
#if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 9)
# if defined(UTIME_NOW) || defined(UTIME_OMIT)
	.flag_utime_omit_ok = 1,
# endif
#endif
	.bmap = op_bmap,
#ifdef SUPERFLUOUS
	.lock = op_lock,
	.poll = op_poll,
#endif
#if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 8)
	.ioctl = op_ioctl,
	.flag_nullpath_ok = 1,
#endif
#if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 9)
	.flag_nopath = 1,
# ifdef SUPPORT_FALLOCATE
	.fallocate = op_fallocate,
# endif
#endif
};

static int get_random_bytes(void *p, size_t sz)
{
	int fd;
	ssize_t r;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		perror("/dev/urandom");
		return 0;
	}

	r = read(fd, p, sz);

	close(fd);
	return (size_t) r == sz;
}

enum {
	FUSE2FS_VERSION,
	FUSE2FS_HELP,
	FUSE2FS_HELPFULL,
};

#define FUSE2FS_OPT(t, p, v) { t, offsetof(struct fuse2fs, p), v }

static struct fuse_opt fuse2fs_opts[] = {
	FUSE2FS_OPT("ro",		ro,			1),
	FUSE2FS_OPT("errors=panic",	panic_on_error,		1),
	FUSE2FS_OPT("minixdf",		minixdf,		1),
	FUSE2FS_OPT("fakeroot",		fakeroot,		1),
	FUSE2FS_OPT("fuse2fs_debug",	debug,			1),
	FUSE2FS_OPT("no_default_opts",	no_default_opts,	1),

	FUSE_OPT_KEY("-V",             FUSE2FS_VERSION),
	FUSE_OPT_KEY("--version",      FUSE2FS_VERSION),
	FUSE_OPT_KEY("-h",             FUSE2FS_HELP),
	FUSE_OPT_KEY("--help",         FUSE2FS_HELP),
	FUSE_OPT_KEY("--helpfull",     FUSE2FS_HELPFULL),
	FUSE_OPT_END
};


static int fuse2fs_opt_proc(void *data, const char *arg,
			    int key, struct fuse_args *outargs)
{
	struct fuse2fs *ff = data;

	switch (key) {
	case FUSE_OPT_KEY_NONOPT:
		if (!ff->device) {
			ff->device = strdup(arg);
			return 0;
		}
		return 1;
	case FUSE2FS_HELP:
	case FUSE2FS_HELPFULL:
		fprintf(stderr,
	"usage: %s device/image mountpoint [options]\n"
	"\n"
	"general options:\n"
	"    -o opt,[opt...]  mount options\n"
	"    -h   --help      print help\n"
	"    -V   --version   print version\n"
	"\n"
	"fuse2fs options:\n"
	"    -o ro                  read-only mount\n"
	"    -o errors=panic        dump core on error\n"
	"    -o minixdf             minix-style df\n"
	"    -o fakeroot            pretend to be root for permission checks\n"
	"    -o no_default_opts     do not include default fuse options\n"
	"    -o fuse2fs_debug       enable fuse2fs debugging\n"
	"\n",
			outargs->argv[0]);
		if (key == FUSE2FS_HELPFULL) {
			fuse_opt_add_arg(outargs, "-ho");
			fuse_main(outargs->argc, outargs->argv, &fs_ops, NULL);
		} else {
			fprintf(stderr, "Try --helpfull to get a list of "
				"all flags, including the FUSE options.\n");
		}
		exit(1);

	case FUSE2FS_VERSION:
		fprintf(stderr, "fuse2fs %s (%s)\n", E2FSPROGS_VERSION,
			E2FSPROGS_DATE);
		fuse_opt_add_arg(outargs, "--version");
		fuse_main(outargs->argc, outargs->argv, &fs_ops, NULL);
		exit(0);
	}
	return 1;
}

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse2fs fctx;
	errcode_t err;
	char *logfile;
	char extra_args[BUFSIZ];
	int ret = 0, flags = EXT2_FLAG_64BITS | EXT2_FLAG_EXCLUSIVE;

	memset(&fctx, 0, sizeof(fctx));
	fctx.magic = FUSE2FS_MAGIC;

	fuse_opt_parse(&args, &fctx, fuse2fs_opts, fuse2fs_opt_proc);
	if (fctx.device == NULL) {
		fprintf(stderr, "Missing ext4 device/image\n");
		fprintf(stderr, "See '%s -h' for usage\n", argv[0]);
		exit(1);
	}

	if (fctx.ro)
		printf("%s", _("Mounting read-only.\n"));

#ifdef ENABLE_NLS
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	bindtextdomain(NLS_CAT_NAME, LOCALEDIR);
	textdomain(NLS_CAT_NAME);
	set_com_err_gettext(gettext);
#endif
	add_error_table(&et_ext2_error_table);

	/* Set up error logging */
	logfile = getenv("FUSE2FS_LOGFILE");
	if (logfile) {
		fctx.err_fp = fopen(logfile, "a");
		if (!fctx.err_fp) {
			perror(logfile);
			goto out;
		}
	} else
		fctx.err_fp = stderr;

	/* Will we allow users to allocate every last block? */
	if (getenv("FUSE2FS_ALLOC_ALL_BLOCKS")) {
		printf(_("%s: Allowing users to allocate all blocks. "
		       "This is dangerous!\n"), fctx.device);
		fctx.alloc_all_blocks = 1;
	}

	/* Start up the fs (while we still can use stdout) */
	ret = 2;
	if (!fctx.ro)
		flags |= EXT2_FLAG_RW;
	err = ext2fs_open2(fctx.device, NULL, flags, 0, 0, unix_io_manager,
			   &global_fs);
	if (err) {
		printf(_("%s: %s.\n"), fctx.device, error_message(err));
		printf(_("Please run e2fsck -fy %s.\n"), fctx.device);
		goto out;
	}
	fctx.fs = global_fs;
	global_fs->priv_data = &fctx;

	ret = 3;

	if (ext2fs_has_feature_journal_needs_recovery(global_fs->super)) {
		if (!fctx.ro) {
			printf(_("%s: recovering journal\n"), fctx.device);
			err = ext2fs_run_ext3_journal(&global_fs);
			if (err) {
				printf(_("%s: %s.\n"), fctx.device,
				       error_message(err));
				printf(_("Please run e2fsck -fy %s.\n"),
				       fctx.device);
				goto out;
			}
			ext2fs_clear_feature_journal_needs_recovery(global_fs->super);
			ext2fs_mark_super_dirty(global_fs);
		} else {
			printf("%s", _("Journal needs recovery; running "
			       "`e2fsck -E journal_only' is required.\n"));
			goto out;
		}
	}

	if (!fctx.ro) {
		if (ext2fs_has_feature_journal(global_fs->super))
			printf(_("%s: Writing to the journal is not supported.\n"),
			       fctx.device);
		err = ext2fs_read_inode_bitmap(global_fs);
		if (err) {
			translate_error(global_fs, 0, err);
			goto out;
		}
		err = ext2fs_read_block_bitmap(global_fs);
		if (err) {
			translate_error(global_fs, 0, err);
			goto out;
		}
	}

	if (!(global_fs->super->s_state & EXT2_VALID_FS))
		printf("%s", _("Warning: Mounting unchecked fs, running e2fsck "
		       "is recommended.\n"));
	if (global_fs->super->s_max_mnt_count > 0 &&
	    global_fs->super->s_mnt_count >= global_fs->super->s_max_mnt_count)
		printf("%s", _("Warning: Maximal mount count reached, running "
		       "e2fsck is recommended.\n"));
	if (global_fs->super->s_checkinterval > 0 &&
	    (time_t) (global_fs->super->s_lastcheck +
		      global_fs->super->s_checkinterval) <= time(0))
		printf("%s", _("Warning: Check time reached; running e2fsck "
		       "is recommended.\n"));
	if (global_fs->super->s_last_orphan)
		printf("%s",
		       _("Orphans detected; running e2fsck is recommended.\n"));

	if (global_fs->super->s_state & EXT2_ERROR_FS) {
		printf("%s",
		       _("Errors detected; running e2fsck is required.\n"));
		goto out;
	}

	/* Initialize generation counter */
	get_random_bytes(&fctx.next_generation, sizeof(unsigned int));

	/* Set up default fuse parameters */
	snprintf(extra_args, BUFSIZ, "-okernel_cache,subtype=ext4,use_ino,"
		 "fsname=%s,attr_timeout=0" FUSE_PLATFORM_OPTS,
		 fctx.device);
	if (fctx.no_default_opts == 0)
		fuse_opt_add_arg(&args, extra_args);

	if (fctx.fakeroot) {
#ifdef HAVE_MOUNT_NODEV
		fuse_opt_add_arg(&args,"-onodev");
#endif
#ifdef HAVE_MOUNT_NOSUID
		fuse_opt_add_arg(&args,"-onosuid");
#endif
	}

	if (fctx.debug) {
		int	i;

		printf("fuse arguments:");
		for (i = 0; i < args.argc; i++)
			printf(" '%s'", args.argv[i]);
		printf("\n");
	}

	pthread_mutex_init(&fctx.bfl, NULL);
	fuse_main(args.argc, args.argv, &fs_ops, &fctx);
	pthread_mutex_destroy(&fctx.bfl);

	ret = 0;
out:
	if (global_fs) {
		err = ext2fs_close(global_fs);
		if (err)
			com_err(argv[0], err, "while closing fs");
		global_fs = NULL;
	}
	return ret;
}

static int __translate_error(ext2_filsys fs, errcode_t err, ext2_ino_t ino,
			     const char *file, int line)
{
	struct timespec now;
	int ret = err;
	struct fuse2fs *ff = fs->priv_data;
	int is_err = 0;

	/* Translate ext2 error to unix error code */
	if (err < EXT2_ET_BASE)
		goto no_translation;
	switch (err) {
	case EXT2_ET_NO_MEMORY:
	case EXT2_ET_TDB_ERR_OOM:
		ret = -ENOMEM;
		break;
	case EXT2_ET_INVALID_ARGUMENT:
	case EXT2_ET_LLSEEK_FAILED:
		ret = -EINVAL;
		break;
	case EXT2_ET_NO_DIRECTORY:
		ret = -ENOTDIR;
		break;
	case EXT2_ET_FILE_NOT_FOUND:
		ret = -ENOENT;
		break;
	case EXT2_ET_DIR_NO_SPACE:
		is_err = 1;
		/* fallthrough */
	case EXT2_ET_TOOSMALL:
	case EXT2_ET_BLOCK_ALLOC_FAIL:
	case EXT2_ET_INODE_ALLOC_FAIL:
	case EXT2_ET_EA_NO_SPACE:
		ret = -ENOSPC;
		break;
	case EXT2_ET_SYMLINK_LOOP:
		ret = -EMLINK;
		break;
	case EXT2_ET_FILE_TOO_BIG:
		ret = -EFBIG;
		break;
	case EXT2_ET_TDB_ERR_EXISTS:
	case EXT2_ET_FILE_EXISTS:
		ret = -EEXIST;
		break;
	case EXT2_ET_MMP_FAILED:
	case EXT2_ET_MMP_FSCK_ON:
		ret = -EBUSY;
		break;
	case EXT2_ET_EA_KEY_NOT_FOUND:
#ifdef ENODATA
		ret = -ENODATA;
#else
		ret = -ENOENT;
#endif
		break;
	/* Sometimes fuse returns a garbage file handle pointer to us... */
	case EXT2_ET_MAGIC_EXT2_FILE:
		ret = -EFAULT;
		break;
	case EXT2_ET_UNIMPLEMENTED:
		ret = -EOPNOTSUPP;
		break;
	default:
		is_err = 1;
		ret = -EIO;
		break;
	}

no_translation:
	if (!is_err)
		return ret;

	if (ino)
		fprintf(ff->err_fp, "FUSE2FS (%s): %s (inode #%d) at %s:%d.\n",
			fs->device_name ? fs->device_name : "???",
			error_message(err), ino, file, line);
	else
		fprintf(ff->err_fp, "FUSE2FS (%s): %s at %s:%d.\n",
			fs->device_name ? fs->device_name : "???",
			error_message(err), file, line);
	fflush(ff->err_fp);

	/* Make a note in the error log */
	get_now(&now);
	fs->super->s_last_error_time = now.tv_sec;
	fs->super->s_last_error_ino = ino;
	fs->super->s_last_error_line = line;
	fs->super->s_last_error_block = err; /* Yeah... */
	strncpy((char *)fs->super->s_last_error_func, file,
		sizeof(fs->super->s_last_error_func));
	if (fs->super->s_first_error_time == 0) {
		fs->super->s_first_error_time = now.tv_sec;
		fs->super->s_first_error_ino = ino;
		fs->super->s_first_error_line = line;
		fs->super->s_first_error_block = err;
		strncpy((char *)fs->super->s_first_error_func, file,
			sizeof(fs->super->s_first_error_func));
	}

	fs->super->s_error_count++;
	ext2fs_mark_super_dirty(fs);
	ext2fs_flush(fs);
	if (ff->panic_on_error)
		abort();

	return ret;
}
