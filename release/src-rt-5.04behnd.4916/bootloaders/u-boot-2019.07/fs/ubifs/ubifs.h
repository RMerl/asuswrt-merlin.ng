/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * This file is part of UBIFS.
 *
 * Copyright (C) 2006-2008 Nokia Corporation
 *
 * (C) Copyright 2008-2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Authors: Artem Bityutskiy (Битюцкий Артём)
 *          Adrian Hunter
 */

#ifndef __UBIFS_H__
#define __UBIFS_H__

#ifndef __UBOOT__
#include <asm/div64.h>
#include <linux/statfs.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/rwsem.h>
#include <linux/mtd/ubi.h>
#include <linux/pagemap.h>
#include <linux/backing-dev.h>
#include <linux/security.h>
#include "ubifs-media.h"
#else
#include <asm/atomic.h>
#include <asm-generic/atomic-long.h>
#include <ubi_uboot.h>
#include <ubifs_uboot.h>

#include <linux/ctype.h>
#include <linux/time.h>
#include <linux/math64.h>
#include "ubifs-media.h"

struct dentry;
struct file;
struct iattr;
struct kstat;
struct vfsmount;

extern struct super_block *ubifs_sb;

extern unsigned int ubifs_msg_flags;
extern unsigned int ubifs_chk_flags;
extern unsigned int ubifs_tst_flags;

#define pgoff_t		unsigned long

/*
 * We "simulate" the Linux page struct much simpler here
 */
struct page {
	pgoff_t index;
	void *addr;
	struct inode *inode;
};

void iput(struct inode *inode);

/* linux/include/time.h */
#define NSEC_PER_SEC	1000000000L
#define get_seconds()	0
#define CURRENT_TIME_SEC	((struct timespec) { get_seconds(), 0 })

struct timespec {
	time_t	tv_sec;		/* seconds */
	long	tv_nsec;	/* nanoseconds */
};

static struct timespec current_fs_time(struct super_block *sb)
{
	struct timespec now;
	now.tv_sec = 0;
	now.tv_nsec = 0;
	return now;
};

/* linux/include/dcache.h */

/*
 * "quick string" -- eases parameter passing, but more importantly
 * saves "metadata" about the string (ie length and the hash).
 *
 * hash comes first so it snuggles against d_parent in the
 * dentry.
 */
struct qstr {
	unsigned int hash;
	unsigned int len;
#ifndef __UBOOT__
	const char *name;
#else
	char *name;
#endif
};

/* include/linux/fs.h */

/* Possible states of 'frozen' field */
enum {
	SB_UNFROZEN = 0,		/* FS is unfrozen */
	SB_FREEZE_WRITE	= 1,		/* Writes, dir ops, ioctls frozen */
	SB_FREEZE_PAGEFAULT = 2,	/* Page faults stopped as well */
	SB_FREEZE_FS = 3,		/* For internal FS use (e.g. to stop
					 * internal threads if needed) */
	SB_FREEZE_COMPLETE = 4,		/* ->freeze_fs finished successfully */
};

#define SB_FREEZE_LEVELS (SB_FREEZE_COMPLETE - 1)

struct sb_writers {
#ifndef __UBOOT__
	/* Counters for counting writers at each level */
	struct percpu_counter	counter[SB_FREEZE_LEVELS];
#endif
	wait_queue_head_t	wait;		/* queue for waiting for
						   writers / faults to finish */
	int			frozen;		/* Is sb frozen? */
	wait_queue_head_t	wait_unfrozen;	/* queue for waiting for
						   sb to be thawed */
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	lock_map[SB_FREEZE_LEVELS];
#endif
};

struct address_space {
	struct inode		*host;		/* owner: inode, block_device */
#ifndef __UBOOT__
	struct radix_tree_root	page_tree;	/* radix tree of all pages */
#endif
	spinlock_t		tree_lock;	/* and lock protecting it */
	unsigned int		i_mmap_writable;/* count VM_SHARED mappings */
	struct rb_root		i_mmap;		/* tree of private and shared mappings */
	struct list_head	i_mmap_nonlinear;/*list VM_NONLINEAR mappings */
	struct mutex		i_mmap_mutex;	/* protect tree, count, list */
	/* Protected by tree_lock together with the radix tree */
	unsigned long		nrpages;	/* number of total pages */
	pgoff_t			writeback_index;/* writeback starts here */
	const struct address_space_operations *a_ops;	/* methods */
	unsigned long		flags;		/* error bits/gfp mask */
#ifndef __UBOOT__
	struct backing_dev_info *backing_dev_info; /* device readahead, etc */
#endif
	spinlock_t		private_lock;	/* for use by the address_space */
	struct list_head	private_list;	/* ditto */
	void			*private_data;	/* ditto */
} __attribute__((aligned(sizeof(long))));

/*
 * Keep mostly read-only and often accessed (especially for
 * the RCU path lookup and 'stat' data) fields at the beginning
 * of the 'struct inode'
 */
struct inode {
	umode_t			i_mode;
	unsigned short		i_opflags;
	kuid_t			i_uid;
	kgid_t			i_gid;
	unsigned int		i_flags;

#ifdef CONFIG_FS_POSIX_ACL
	struct posix_acl	*i_acl;
	struct posix_acl	*i_default_acl;
#endif

	const struct inode_operations	*i_op;
	struct super_block	*i_sb;
	struct address_space	*i_mapping;

#ifdef CONFIG_SECURITY
	void			*i_security;
#endif

	/* Stat data, not accessed from path walking */
	unsigned long		i_ino;
	/*
	 * Filesystems may only read i_nlink directly.  They shall use the
	 * following functions for modification:
	 *
	 *    (set|clear|inc|drop)_nlink
	 *    inode_(inc|dec)_link_count
	 */
	union {
		const unsigned int i_nlink;
		unsigned int __i_nlink;
	};
	dev_t			i_rdev;
	loff_t			i_size;
	struct timespec		i_atime;
	struct timespec		i_mtime;
	struct timespec		i_ctime;
	spinlock_t		i_lock;	/* i_blocks, i_bytes, maybe i_size */
	unsigned short          i_bytes;
	unsigned int		i_blkbits;
	blkcnt_t		i_blocks;

#ifdef __NEED_I_SIZE_ORDERED
	seqcount_t		i_size_seqcount;
#endif

	/* Misc */
	unsigned long		i_state;
	struct mutex		i_mutex;

	unsigned long		dirtied_when;	/* jiffies of first dirtying */

	struct hlist_node	i_hash;
	struct list_head	i_wb_list;	/* backing dev IO list */
	struct list_head	i_lru;		/* inode LRU list */
	struct list_head	i_sb_list;
	union {
		struct hlist_head	i_dentry;
		struct rcu_head		i_rcu;
	};
	u64			i_version;
	atomic_t		i_count;
	atomic_t		i_dio_count;
	atomic_t		i_writecount;
	const struct file_operations	*i_fop;	/* former ->i_op->default_file_ops */
	struct file_lock	*i_flock;
	struct address_space	i_data;
#ifdef CONFIG_QUOTA
	struct dquot		*i_dquot[MAXQUOTAS];
#endif
	struct list_head	i_devices;
	union {
		struct pipe_inode_info	*i_pipe;
		struct block_device	*i_bdev;
		struct cdev		*i_cdev;
	};

	__u32			i_generation;

#ifdef CONFIG_FSNOTIFY
	__u32			i_fsnotify_mask; /* all events this inode cares about */
	struct hlist_head	i_fsnotify_marks;
#endif

#ifdef CONFIG_IMA
	atomic_t		i_readcount; /* struct files open RO */
#endif
	void			*i_private; /* fs or device private pointer */
};

struct super_operations {
   	struct inode *(*alloc_inode)(struct super_block *sb);
	void (*destroy_inode)(struct inode *);

   	void (*dirty_inode) (struct inode *, int flags);
	int (*write_inode) (struct inode *, struct writeback_control *wbc);
	int (*drop_inode) (struct inode *);
	void (*evict_inode) (struct inode *);
	void (*put_super) (struct super_block *);
	int (*sync_fs)(struct super_block *sb, int wait);
	int (*freeze_fs) (struct super_block *);
	int (*unfreeze_fs) (struct super_block *);
#ifndef __UBOOT__
	int (*statfs) (struct dentry *, struct kstatfs *);
#endif
	int (*remount_fs) (struct super_block *, int *, char *);
	void (*umount_begin) (struct super_block *);

#ifndef __UBOOT__
	int (*show_options)(struct seq_file *, struct dentry *);
	int (*show_devname)(struct seq_file *, struct dentry *);
	int (*show_path)(struct seq_file *, struct dentry *);
	int (*show_stats)(struct seq_file *, struct dentry *);
#endif
#ifdef CONFIG_QUOTA
	ssize_t (*quota_read)(struct super_block *, int, char *, size_t, loff_t);
	ssize_t (*quota_write)(struct super_block *, int, const char *, size_t, loff_t);
#endif
	int (*bdev_try_to_free_page)(struct super_block*, struct page*, gfp_t);
	long (*nr_cached_objects)(struct super_block *, int);
	long (*free_cached_objects)(struct super_block *, long, int);
};

struct super_block {
	struct list_head	s_list;		/* Keep this first */
	dev_t			s_dev;		/* search index; _not_ kdev_t */
	unsigned char		s_blocksize_bits;
	unsigned long		s_blocksize;
	loff_t			s_maxbytes;	/* Max file size */
	struct file_system_type	*s_type;
	const struct super_operations	*s_op;
	const struct dquot_operations	*dq_op;
	const struct quotactl_ops	*s_qcop;
	const struct export_operations *s_export_op;
	unsigned long		s_flags;
	unsigned long		s_magic;
	struct dentry		*s_root;
	struct rw_semaphore	s_umount;
	int			s_count;
	atomic_t		s_active;
#ifdef CONFIG_SECURITY
	void                    *s_security;
#endif
	const struct xattr_handler **s_xattr;

	struct list_head	s_inodes;	/* all inodes */
#ifndef __UBOOT__
	struct hlist_bl_head	s_anon;		/* anonymous dentries for (nfs) exporting */
#endif
	struct list_head	s_mounts;	/* list of mounts; _not_ for fs use */
	struct block_device	*s_bdev;
#ifndef __UBOOT__
	struct backing_dev_info *s_bdi;
#endif
	struct mtd_info		*s_mtd;
#ifndef __UBOOT__
	struct hlist_node	s_instances;
	struct quota_info	s_dquot;	/* Diskquota specific options */
#endif

	struct sb_writers	s_writers;

	char s_id[32];				/* Informational name */
	u8 s_uuid[16];				/* UUID */

	void 			*s_fs_info;	/* Filesystem private info */
	unsigned int		s_max_links;
#ifndef __UBOOT__
	fmode_t			s_mode;
#endif

	/* Granularity of c/m/atime in ns.
	   Cannot be worse than a second */
	u32		   s_time_gran;

	/*
	 * The next field is for VFS *only*. No filesystems have any business
	 * even looking at it. You had been warned.
	 */
	struct mutex s_vfs_rename_mutex;	/* Kludge */

	/*
	 * Filesystem subtype.  If non-empty the filesystem type field
	 * in /proc/mounts will be "type.subtype"
	 */
	char *s_subtype;

#ifndef __UBOOT__
	/*
	 * Saved mount options for lazy filesystems using
	 * generic_show_options()
	 */
	char __rcu *s_options;
#endif
	const struct dentry_operations *s_d_op; /* default d_op for dentries */

	/*
	 * Saved pool identifier for cleancache (-1 means none)
	 */
	int cleancache_poolid;

#ifndef __UBOOT__
	struct shrinker s_shrink;	/* per-sb shrinker handle */
#endif

	/* Number of inodes with nlink == 0 but still referenced */
	atomic_long_t s_remove_count;

	/* Being remounted read-only */
	int s_readonly_remount;

	/* AIO completions deferred from interrupt context */
	struct workqueue_struct *s_dio_done_wq;

#ifndef __UBOOT__
	/*
	 * Keep the lru lists last in the structure so they always sit on their
	 * own individual cachelines.
	 */
	struct list_lru		s_dentry_lru ____cacheline_aligned_in_smp;
	struct list_lru		s_inode_lru ____cacheline_aligned_in_smp;
#endif
	struct rcu_head		rcu;
};

struct file_system_type {
	const char *name;
	int fs_flags;
#define FS_REQUIRES_DEV		1 
#define FS_BINARY_MOUNTDATA	2
#define FS_HAS_SUBTYPE		4
#define FS_USERNS_MOUNT		8	/* Can be mounted by userns root */
#define FS_USERNS_DEV_MOUNT	16 /* A userns mount does not imply MNT_NODEV */
#define FS_RENAME_DOES_D_MOVE	32768	/* FS will handle d_move() during rename() internally. */
	struct dentry *(*mount) (struct file_system_type *, int,
		       const char *, void *);
	void (*kill_sb) (struct super_block *);
	struct module *owner;
	struct file_system_type * next;
	struct hlist_head fs_supers;

#ifndef __UBOOT__
	struct lock_class_key s_lock_key;
	struct lock_class_key s_umount_key;
	struct lock_class_key s_vfs_rename_key;
	struct lock_class_key s_writers_key[SB_FREEZE_LEVELS];

	struct lock_class_key i_lock_key;
	struct lock_class_key i_mutex_key;
	struct lock_class_key i_mutex_dir_key;
#endif
};

/* include/linux/mount.h */
struct vfsmount {
	struct dentry *mnt_root;	/* root of the mounted tree */
	struct super_block *mnt_sb;	/* pointer to superblock */
	int mnt_flags;
};

struct path {
	struct vfsmount *mnt;
	struct dentry *dentry;
};

struct file {
	struct path		f_path;
#define f_dentry	f_path.dentry
#define f_vfsmnt	f_path.mnt
	const struct file_operations	*f_op;
	unsigned int 		f_flags;
	loff_t			f_pos;
	unsigned int		f_uid, f_gid;

	u64			f_version;
#ifdef CONFIG_SECURITY
	void			*f_security;
#endif
	/* needed for tty driver, and maybe others */
	void			*private_data;

#ifdef CONFIG_EPOLL
	/* Used by fs/eventpoll.c to link all the hooks to this file */
	struct list_head	f_ep_links;
	spinlock_t		f_ep_lock;
#endif /* #ifdef CONFIG_EPOLL */
#ifdef CONFIG_DEBUG_WRITECOUNT
	unsigned long f_mnt_write_state;
#endif
};

/*
 * get_seconds() not really needed in the read-only implmentation
 */
#define get_seconds()		0

/* 4k page size */
#define PAGE_CACHE_SHIFT	12
#define PAGE_CACHE_SIZE		(1 << PAGE_CACHE_SHIFT)

/* Page cache limit. The filesystems should put that into their s_maxbytes
   limits, otherwise bad things can happen in VM. */
#if BITS_PER_LONG==32
#define MAX_LFS_FILESIZE	(((u64)PAGE_CACHE_SIZE << (BITS_PER_LONG-1))-1)
#elif BITS_PER_LONG==64
#define MAX_LFS_FILESIZE 	0x7fffffffffffffffUL
#endif

/*
 * These are the fs-independent mount-flags: up to 32 flags are supported
 */
#define MS_RDONLY	 1	/* Mount read-only */
#define MS_NOSUID	 2	/* Ignore suid and sgid bits */
#define MS_NODEV	 4	/* Disallow access to device special files */
#define MS_NOEXEC	 8	/* Disallow program execution */
#define MS_SYNCHRONOUS	16	/* Writes are synced at once */
#define MS_REMOUNT	32	/* Alter flags of a mounted FS */
#define MS_MANDLOCK	64	/* Allow mandatory locks on an FS */
#define MS_DIRSYNC	128	/* Directory modifications are synchronous */
#define MS_NOATIME	1024	/* Do not update access times. */
#define MS_NODIRATIME	2048	/* Do not update directory access times */
#define MS_BIND		4096
#define MS_MOVE		8192
#define MS_REC		16384
#define MS_VERBOSE	32768	/* War is peace. Verbosity is silence.
				   MS_VERBOSE is deprecated. */
#define MS_SILENT	32768
#define MS_POSIXACL	(1<<16)	/* VFS does not apply the umask */
#define MS_UNBINDABLE	(1<<17)	/* change to unbindable */
#define MS_PRIVATE	(1<<18)	/* change to private */
#define MS_SLAVE	(1<<19)	/* change to slave */
#define MS_SHARED	(1<<20)	/* change to shared */
#define MS_RELATIME	(1<<21)	/* Update atime relative to mtime/ctime. */
#define MS_KERNMOUNT	(1<<22) /* this is a kern_mount call */
#define MS_I_VERSION	(1<<23) /* Update inode I_version field */
#define MS_ACTIVE	(1<<30)
#define MS_NOUSER	(1<<31)

#define I_NEW			8

/* Inode flags - they have nothing to superblock flags now */

#define S_SYNC		1	/* Writes are synced at once */
#define S_NOATIME	2	/* Do not update access times */
#define S_APPEND	4	/* Append-only file */
#define S_IMMUTABLE	8	/* Immutable file */
#define S_DEAD		16	/* removed, but still open directory */
#define S_NOQUOTA	32	/* Inode is not counted to quota */
#define S_DIRSYNC	64	/* Directory modifications are synchronous */
#define S_NOCMTIME	128	/* Do not update file c/mtime */
#define S_SWAPFILE	256	/* Do not truncate: swapon got its bmaps */
#define S_PRIVATE	512	/* Inode is fs-internal */

/* include/linux/stat.h */

#define S_IFMT  00170000
#define S_IFSOCK 0140000
#define S_IFLNK	 0120000
#define S_IFREG  0100000
#define S_IFBLK  0060000
#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000

/* include/linux/fs.h */

/*
 * File types
 *
 * NOTE! These match bits 12..15 of stat.st_mode
 * (ie "(i_mode >> 12) & 15").
 */
#define DT_UNKNOWN	0
#define DT_FIFO		1
#define DT_CHR		2
#define DT_DIR		4
#define DT_BLK		6
#define DT_REG		8
#define DT_LNK		10
#define DT_SOCK		12
#define DT_WHT		14

#define I_DIRTY_SYNC		1
#define I_DIRTY_DATASYNC	2
#define I_DIRTY_PAGES		4
#define I_NEW			8
#define I_WILL_FREE		16
#define I_FREEING		32
#define I_CLEAR			64
#define __I_LOCK		7
#define I_LOCK			(1 << __I_LOCK)
#define __I_SYNC		8
#define I_SYNC			(1 << __I_SYNC)

#define I_DIRTY (I_DIRTY_SYNC | I_DIRTY_DATASYNC | I_DIRTY_PAGES)

/* linux/include/dcache.h */

#define DNAME_INLINE_LEN_MIN 36

struct dentry {
	unsigned int d_flags;		/* protected by d_lock */
	spinlock_t d_lock;		/* per dentry lock */
	struct inode *d_inode;		/* Where the name belongs to - NULL is
					 * negative */
	/*
	 * The next three fields are touched by __d_lookup.  Place them here
	 * so they all fit in a cache line.
	 */
	struct hlist_node d_hash;	/* lookup hash list */
	struct dentry *d_parent;	/* parent directory */
	struct qstr d_name;

	struct list_head d_lru;		/* LRU list */
	/*
	 * d_child and d_rcu can share memory
	 */
	struct list_head d_subdirs;	/* our children */
	struct list_head d_alias;	/* inode alias list */
	unsigned long d_time;		/* used by d_revalidate */
	struct super_block *d_sb;	/* The root of the dentry tree */
	void *d_fsdata;			/* fs-specific data */
#ifdef CONFIG_PROFILING
	struct dcookie_struct *d_cookie; /* cookie, if any */
#endif
	int d_mounted;
	unsigned char d_iname[DNAME_INLINE_LEN_MIN];	/* small names */
};

static inline ino_t parent_ino(struct dentry *dentry)
{
	ino_t res;

	spin_lock(&dentry->d_lock);
	res = dentry->d_parent->d_inode->i_ino;
	spin_unlock(&dentry->d_lock);
	return res;
}

/* debug.c */

#define module_param_named(...)

/* misc.h */
#define mutex_lock_nested(...)
#define mutex_unlock_nested(...)
#define mutex_is_locked(...)	1
#endif

/* Version of this UBIFS implementation */
#define UBIFS_VERSION 1

/* Normal UBIFS messages */
#ifdef CONFIG_UBIFS_SILENCE_MSG
#define ubifs_msg(c, fmt, ...)
#else
#define ubifs_msg(c, fmt, ...)                                      \
	pr_notice("UBIFS (ubi%d:%d): " fmt "\n",                    \
		  (c)->vi.ubi_num, (c)->vi.vol_id, ##__VA_ARGS__)
#endif
/* UBIFS error messages */
#ifndef __UBOOT__
#define ubifs_err(c, fmt, ...)                                      \
	pr_err("UBIFS error (ubi%d:%d pid %d): %s: " fmt "\n",      \
	       (c)->vi.ubi_num, (c)->vi.vol_id, current->pid,       \
	       __func__, ##__VA_ARGS__)
/* UBIFS warning messages */
#define ubifs_warn(c, fmt, ...)                                     \
	pr_warn("UBIFS warning (ubi%d:%d pid %d): %s: " fmt "\n",   \
		(c)->vi.ubi_num, (c)->vi.vol_id, current->pid,      \
		__func__, ##__VA_ARGS__)
#else
#define ubifs_err(c, fmt, ...)                                      \
	pr_err("UBIFS error (ubi%d:%d pid %d): %s: " fmt "\n",      \
	       (c)->vi.ubi_num, (c)->vi.vol_id, 0,                  \
	       __func__, ##__VA_ARGS__)
/* UBIFS warning messages */
#define ubifs_warn(c, fmt, ...)                                     \
	pr_warn("UBIFS warning (ubi%d:%d pid %d): %s: " fmt "\n",   \
		(c)->vi.ubi_num, (c)->vi.vol_id, 0,                 \
		__func__, ##__VA_ARGS__)

#endif

/*
 * A variant of 'ubifs_err()' which takes the UBIFS file-sytem description
 * object as an argument.
 */
#define ubifs_errc(c, fmt, ...)                                     \
	do {                                                        \
		if (!(c)->probing)                                  \
			ubifs_err(c, fmt, ##__VA_ARGS__);           \
	} while (0)

/* UBIFS file system VFS magic number */
#define UBIFS_SUPER_MAGIC 0x24051905

/* Number of UBIFS blocks per VFS page */
#define UBIFS_BLOCKS_PER_PAGE (PAGE_CACHE_SIZE / UBIFS_BLOCK_SIZE)
#define UBIFS_BLOCKS_PER_PAGE_SHIFT (PAGE_CACHE_SHIFT - UBIFS_BLOCK_SHIFT)

/* "File system end of life" sequence number watermark */
#define SQNUM_WARN_WATERMARK 0xFFFFFFFF00000000ULL
#define SQNUM_WATERMARK      0xFFFFFFFFFF000000ULL

/*
 * Minimum amount of LEBs reserved for the index. At present the index needs at
 * least 2 LEBs: one for the index head and one for in-the-gaps method (which
 * currently does not cater for the index head and so excludes it from
 * consideration).
 */
#define MIN_INDEX_LEBS 2

/* Minimum amount of data UBIFS writes to the flash */
#define MIN_WRITE_SZ (UBIFS_DATA_NODE_SZ + 8)

/*
 * Currently we do not support inode number overlapping and re-using, so this
 * watermark defines dangerous inode number level. This should be fixed later,
 * although it is difficult to exceed current limit. Another option is to use
 * 64-bit inode numbers, but this means more overhead.
 */
#define INUM_WARN_WATERMARK 0xFFF00000
#define INUM_WATERMARK      0xFFFFFF00

/* Maximum number of entries in each LPT (LEB category) heap */
#define LPT_HEAP_SZ 256

/*
 * Background thread name pattern. The numbers are UBI device and volume
 * numbers.
 */
#define BGT_NAME_PATTERN "ubifs_bgt%d_%d"

/* Write-buffer synchronization timeout interval in seconds */
#define WBUF_TIMEOUT_SOFTLIMIT 3
#define WBUF_TIMEOUT_HARDLIMIT 5

/* Maximum possible inode number (only 32-bit inodes are supported now) */
#define MAX_INUM 0xFFFFFFFF

/* Number of non-data journal heads */
#define NONDATA_JHEADS_CNT 2

/* Shorter names for journal head numbers for internal usage */
#define GCHD   UBIFS_GC_HEAD
#define BASEHD UBIFS_BASE_HEAD
#define DATAHD UBIFS_DATA_HEAD

/* 'No change' value for 'ubifs_change_lp()' */
#define LPROPS_NC 0x80000001

/*
 * There is no notion of truncation key because truncation nodes do not exist
 * in TNC. However, when replaying, it is handy to introduce fake "truncation"
 * keys for truncation nodes because the code becomes simpler. So we define
 * %UBIFS_TRUN_KEY type.
 *
 * But otherwise, out of the journal reply scope, the truncation keys are
 * invalid.
 */
#define UBIFS_TRUN_KEY    UBIFS_KEY_TYPES_CNT
#define UBIFS_INVALID_KEY UBIFS_KEY_TYPES_CNT

/*
 * How much a directory entry/extended attribute entry adds to the parent/host
 * inode.
 */
#define CALC_DENT_SIZE(name_len) ALIGN(UBIFS_DENT_NODE_SZ + (name_len) + 1, 8)

/* How much an extended attribute adds to the host inode */
#define CALC_XATTR_BYTES(data_len) ALIGN(UBIFS_INO_NODE_SZ + (data_len) + 1, 8)

/*
 * Znodes which were not touched for 'OLD_ZNODE_AGE' seconds are considered
 * "old", and znode which were touched last 'YOUNG_ZNODE_AGE' seconds ago are
 * considered "young". This is used by shrinker when selecting znode to trim
 * off.
 */
#define OLD_ZNODE_AGE 20
#define YOUNG_ZNODE_AGE 5

/*
 * Some compressors, like LZO, may end up with more data then the input buffer.
 * So UBIFS always allocates larger output buffer, to be sure the compressor
 * will not corrupt memory in case of worst case compression.
 */
#define WORST_COMPR_FACTOR 2

/*
 * How much memory is needed for a buffer where we compress a data node.
 */
#define COMPRESSED_DATA_NODE_BUF_SZ \
	(UBIFS_DATA_NODE_SZ + UBIFS_BLOCK_SIZE * WORST_COMPR_FACTOR)

/* Maximum expected tree height for use by bottom_up_buf */
#define BOTTOM_UP_HEIGHT 64

/* Maximum number of data nodes to bulk-read */
#define UBIFS_MAX_BULK_READ 32

/*
 * Lockdep classes for UBIFS inode @ui_mutex.
 */
enum {
	WB_MUTEX_1 = 0,
	WB_MUTEX_2 = 1,
	WB_MUTEX_3 = 2,
};

/*
 * Znode flags (actually, bit numbers which store the flags).
 *
 * DIRTY_ZNODE: znode is dirty
 * COW_ZNODE: znode is being committed and a new instance of this znode has to
 *            be created before changing this znode
 * OBSOLETE_ZNODE: znode is obsolete, which means it was deleted, but it is
 *                 still in the commit list and the ongoing commit operation
 *                 will commit it, and delete this znode after it is done
 */
enum {
	DIRTY_ZNODE    = 0,
	COW_ZNODE      = 1,
	OBSOLETE_ZNODE = 2,
};

/*
 * Commit states.
 *
 * COMMIT_RESTING: commit is not wanted
 * COMMIT_BACKGROUND: background commit has been requested
 * COMMIT_REQUIRED: commit is required
 * COMMIT_RUNNING_BACKGROUND: background commit is running
 * COMMIT_RUNNING_REQUIRED: commit is running and it is required
 * COMMIT_BROKEN: commit failed
 */
enum {
	COMMIT_RESTING = 0,
	COMMIT_BACKGROUND,
	COMMIT_REQUIRED,
	COMMIT_RUNNING_BACKGROUND,
	COMMIT_RUNNING_REQUIRED,
	COMMIT_BROKEN,
};

/*
 * 'ubifs_scan_a_node()' return values.
 *
 * SCANNED_GARBAGE:  scanned garbage
 * SCANNED_EMPTY_SPACE: scanned empty space
 * SCANNED_A_NODE: scanned a valid node
 * SCANNED_A_CORRUPT_NODE: scanned a corrupted node
 * SCANNED_A_BAD_PAD_NODE: scanned a padding node with invalid pad length
 *
 * Greater than zero means: 'scanned that number of padding bytes'
 */
enum {
	SCANNED_GARBAGE        = 0,
	SCANNED_EMPTY_SPACE    = -1,
	SCANNED_A_NODE         = -2,
	SCANNED_A_CORRUPT_NODE = -3,
	SCANNED_A_BAD_PAD_NODE = -4,
};

/*
 * LPT cnode flag bits.
 *
 * DIRTY_CNODE: cnode is dirty
 * OBSOLETE_CNODE: cnode is being committed and has been copied (or deleted),
 *                 so it can (and must) be freed when the commit is finished
 * COW_CNODE: cnode is being committed and must be copied before writing
 */
enum {
	DIRTY_CNODE    = 0,
	OBSOLETE_CNODE = 1,
	COW_CNODE      = 2,
};

/*
 * Dirty flag bits (lpt_drty_flgs) for LPT special nodes.
 *
 * LTAB_DIRTY: ltab node is dirty
 * LSAVE_DIRTY: lsave node is dirty
 */
enum {
	LTAB_DIRTY  = 1,
	LSAVE_DIRTY = 2,
};

/*
 * Return codes used by the garbage collector.
 * @LEB_FREED: the logical eraseblock was freed and is ready to use
 * @LEB_FREED_IDX: indexing LEB was freed and can be used only after the commit
 * @LEB_RETAINED: the logical eraseblock was freed and retained for GC purposes
 */
enum {
	LEB_FREED,
	LEB_FREED_IDX,
	LEB_RETAINED,
};

/**
 * struct ubifs_old_idx - index node obsoleted since last commit start.
 * @rb: rb-tree node
 * @lnum: LEB number of obsoleted index node
 * @offs: offset of obsoleted index node
 */
struct ubifs_old_idx {
	struct rb_node rb;
	int lnum;
	int offs;
};

/* The below union makes it easier to deal with keys */
union ubifs_key {
	uint8_t u8[UBIFS_SK_LEN];
	uint32_t u32[UBIFS_SK_LEN/4];
	uint64_t u64[UBIFS_SK_LEN/8];
	__le32 j32[UBIFS_SK_LEN/4];
};

/**
 * struct ubifs_scan_node - UBIFS scanned node information.
 * @list: list of scanned nodes
 * @key: key of node scanned (if it has one)
 * @sqnum: sequence number
 * @type: type of node scanned
 * @offs: offset with LEB of node scanned
 * @len: length of node scanned
 * @node: raw node
 */
struct ubifs_scan_node {
	struct list_head list;
	union ubifs_key key;
	unsigned long long sqnum;
	int type;
	int offs;
	int len;
	void *node;
};

/**
 * struct ubifs_scan_leb - UBIFS scanned LEB information.
 * @lnum: logical eraseblock number
 * @nodes_cnt: number of nodes scanned
 * @nodes: list of struct ubifs_scan_node
 * @endpt: end point (and therefore the start of empty space)
 * @buf: buffer containing entire LEB scanned
 */
struct ubifs_scan_leb {
	int lnum;
	int nodes_cnt;
	struct list_head nodes;
	int endpt;
	void *buf;
};

/**
 * struct ubifs_gced_idx_leb - garbage-collected indexing LEB.
 * @list: list
 * @lnum: LEB number
 * @unmap: OK to unmap this LEB
 *
 * This data structure is used to temporary store garbage-collected indexing
 * LEBs - they are not released immediately, but only after the next commit.
 * This is needed to guarantee recoverability.
 */
struct ubifs_gced_idx_leb {
	struct list_head list;
	int lnum;
	int unmap;
};

/**
 * struct ubifs_inode - UBIFS in-memory inode description.
 * @vfs_inode: VFS inode description object
 * @creat_sqnum: sequence number at time of creation
 * @del_cmtno: commit number corresponding to the time the inode was deleted,
 *             protected by @c->commit_sem;
 * @xattr_size: summarized size of all extended attributes in bytes
 * @xattr_cnt: count of extended attributes this inode has
 * @xattr_names: sum of lengths of all extended attribute names belonging to
 *               this inode
 * @dirty: non-zero if the inode is dirty
 * @xattr: non-zero if this is an extended attribute inode
 * @bulk_read: non-zero if bulk-read should be used
 * @ui_mutex: serializes inode write-back with the rest of VFS operations,
 *            serializes "clean <-> dirty" state changes, serializes bulk-read,
 *            protects @dirty, @bulk_read, @ui_size, and @xattr_size
 * @ui_lock: protects @synced_i_size
 * @synced_i_size: synchronized size of inode, i.e. the value of inode size
 *                 currently stored on the flash; used only for regular file
 *                 inodes
 * @ui_size: inode size used by UBIFS when writing to flash
 * @flags: inode flags (@UBIFS_COMPR_FL, etc)
 * @compr_type: default compression type used for this inode
 * @last_page_read: page number of last page read (for bulk read)
 * @read_in_a_row: number of consecutive pages read in a row (for bulk read)
 * @data_len: length of the data attached to the inode
 * @data: inode's data
 *
 * @ui_mutex exists for two main reasons. At first it prevents inodes from
 * being written back while UBIFS changing them, being in the middle of an VFS
 * operation. This way UBIFS makes sure the inode fields are consistent. For
 * example, in 'ubifs_rename()' we change 3 inodes simultaneously, and
 * write-back must not write any of them before we have finished.
 *
 * The second reason is budgeting - UBIFS has to budget all operations. If an
 * operation is going to mark an inode dirty, it has to allocate budget for
 * this. It cannot just mark it dirty because there is no guarantee there will
 * be enough flash space to write the inode back later. This means UBIFS has
 * to have full control over inode "clean <-> dirty" transitions (and pages
 * actually). But unfortunately, VFS marks inodes dirty in many places, and it
 * does not ask the file-system if it is allowed to do so (there is a notifier,
 * but it is not enough), i.e., there is no mechanism to synchronize with this.
 * So UBIFS has its own inode dirty flag and its own mutex to serialize
 * "clean <-> dirty" transitions.
 *
 * The @synced_i_size field is used to make sure we never write pages which are
 * beyond last synchronized inode size. See 'ubifs_writepage()' for more
 * information.
 *
 * The @ui_size is a "shadow" variable for @inode->i_size and UBIFS uses
 * @ui_size instead of @inode->i_size. The reason for this is that UBIFS cannot
 * make sure @inode->i_size is always changed under @ui_mutex, because it
 * cannot call 'truncate_setsize()' with @ui_mutex locked, because it would
 * deadlock with 'ubifs_writepage()' (see file.c). All the other inode fields
 * are changed under @ui_mutex, so they do not need "shadow" fields. Note, one
 * could consider to rework locking and base it on "shadow" fields.
 */
struct ubifs_inode {
	struct inode vfs_inode;
	unsigned long long creat_sqnum;
	unsigned long long del_cmtno;
	unsigned int xattr_size;
	unsigned int xattr_cnt;
	unsigned int xattr_names;
	unsigned int dirty:1;
	unsigned int xattr:1;
	unsigned int bulk_read:1;
	unsigned int compr_type:2;
	struct mutex ui_mutex;
	spinlock_t ui_lock;
	loff_t synced_i_size;
	loff_t ui_size;
	int flags;
	pgoff_t last_page_read;
	pgoff_t read_in_a_row;
	int data_len;
	void *data;
};

/**
 * struct ubifs_unclean_leb - records a LEB recovered under read-only mode.
 * @list: list
 * @lnum: LEB number of recovered LEB
 * @endpt: offset where recovery ended
 *
 * This structure records a LEB identified during recovery that needs to be
 * cleaned but was not because UBIFS was mounted read-only. The information
 * is used to clean the LEB when remounting to read-write mode.
 */
struct ubifs_unclean_leb {
	struct list_head list;
	int lnum;
	int endpt;
};

/*
 * LEB properties flags.
 *
 * LPROPS_UNCAT: not categorized
 * LPROPS_DIRTY: dirty > free, dirty >= @c->dead_wm, not index
 * LPROPS_DIRTY_IDX: dirty + free > @c->min_idx_node_sze and index
 * LPROPS_FREE: free > 0, dirty < @c->dead_wm, not empty, not index
 * LPROPS_HEAP_CNT: number of heaps used for storing categorized LEBs
 * LPROPS_EMPTY: LEB is empty, not taken
 * LPROPS_FREEABLE: free + dirty == leb_size, not index, not taken
 * LPROPS_FRDI_IDX: free + dirty == leb_size and index, may be taken
 * LPROPS_CAT_MASK: mask for the LEB categories above
 * LPROPS_TAKEN: LEB was taken (this flag is not saved on the media)
 * LPROPS_INDEX: LEB contains indexing nodes (this flag also exists on flash)
 */
enum {
	LPROPS_UNCAT     =  0,
	LPROPS_DIRTY     =  1,
	LPROPS_DIRTY_IDX =  2,
	LPROPS_FREE      =  3,
	LPROPS_HEAP_CNT  =  3,
	LPROPS_EMPTY     =  4,
	LPROPS_FREEABLE  =  5,
	LPROPS_FRDI_IDX  =  6,
	LPROPS_CAT_MASK  = 15,
	LPROPS_TAKEN     = 16,
	LPROPS_INDEX     = 32,
};

/**
 * struct ubifs_lprops - logical eraseblock properties.
 * @free: amount of free space in bytes
 * @dirty: amount of dirty space in bytes
 * @flags: LEB properties flags (see above)
 * @lnum: LEB number
 * @list: list of same-category lprops (for LPROPS_EMPTY and LPROPS_FREEABLE)
 * @hpos: heap position in heap of same-category lprops (other categories)
 */
struct ubifs_lprops {
	int free;
	int dirty;
	int flags;
	int lnum;
	union {
		struct list_head list;
		int hpos;
	};
};

/**
 * struct ubifs_lpt_lprops - LPT logical eraseblock properties.
 * @free: amount of free space in bytes
 * @dirty: amount of dirty space in bytes
 * @tgc: trivial GC flag (1 => unmap after commit end)
 * @cmt: commit flag (1 => reserved for commit)
 */
struct ubifs_lpt_lprops {
	int free;
	int dirty;
	unsigned tgc:1;
	unsigned cmt:1;
};

/**
 * struct ubifs_lp_stats - statistics of eraseblocks in the main area.
 * @empty_lebs: number of empty LEBs
 * @taken_empty_lebs: number of taken LEBs
 * @idx_lebs: number of indexing LEBs
 * @total_free: total free space in bytes (includes all LEBs)
 * @total_dirty: total dirty space in bytes (includes all LEBs)
 * @total_used: total used space in bytes (does not include index LEBs)
 * @total_dead: total dead space in bytes (does not include index LEBs)
 * @total_dark: total dark space in bytes (does not include index LEBs)
 *
 * The @taken_empty_lebs field counts the LEBs that are in the transient state
 * of having been "taken" for use but not yet written to. @taken_empty_lebs is
 * needed to account correctly for @gc_lnum, otherwise @empty_lebs could be
 * used by itself (in which case 'unused_lebs' would be a better name). In the
 * case of @gc_lnum, it is "taken" at mount time or whenever a LEB is retained
 * by GC, but unlike other empty LEBs that are "taken", it may not be written
 * straight away (i.e. before the next commit start or unmount), so either
 * @gc_lnum must be specially accounted for, or the current approach followed
 * i.e. count it under @taken_empty_lebs.
 *
 * @empty_lebs includes @taken_empty_lebs.
 *
 * @total_used, @total_dead and @total_dark fields do not account indexing
 * LEBs.
 */
struct ubifs_lp_stats {
	int empty_lebs;
	int taken_empty_lebs;
	int idx_lebs;
	long long total_free;
	long long total_dirty;
	long long total_used;
	long long total_dead;
	long long total_dark;
};

struct ubifs_nnode;

/**
 * struct ubifs_cnode - LEB Properties Tree common node.
 * @parent: parent nnode
 * @cnext: next cnode to commit
 * @flags: flags (%DIRTY_LPT_NODE or %OBSOLETE_LPT_NODE)
 * @iip: index in parent
 * @level: level in the tree (zero for pnodes, greater than zero for nnodes)
 * @num: node number
 */
struct ubifs_cnode {
	struct ubifs_nnode *parent;
	struct ubifs_cnode *cnext;
	unsigned long flags;
	int iip;
	int level;
	int num;
};

/**
 * struct ubifs_pnode - LEB Properties Tree leaf node.
 * @parent: parent nnode
 * @cnext: next cnode to commit
 * @flags: flags (%DIRTY_LPT_NODE or %OBSOLETE_LPT_NODE)
 * @iip: index in parent
 * @level: level in the tree (always zero for pnodes)
 * @num: node number
 * @lprops: LEB properties array
 */
struct ubifs_pnode {
	struct ubifs_nnode *parent;
	struct ubifs_cnode *cnext;
	unsigned long flags;
	int iip;
	int level;
	int num;
	struct ubifs_lprops lprops[UBIFS_LPT_FANOUT];
};

/**
 * struct ubifs_nbranch - LEB Properties Tree internal node branch.
 * @lnum: LEB number of child
 * @offs: offset of child
 * @nnode: nnode child
 * @pnode: pnode child
 * @cnode: cnode child
 */
struct ubifs_nbranch {
	int lnum;
	int offs;
	union {
		struct ubifs_nnode *nnode;
		struct ubifs_pnode *pnode;
		struct ubifs_cnode *cnode;
	};
};

/**
 * struct ubifs_nnode - LEB Properties Tree internal node.
 * @parent: parent nnode
 * @cnext: next cnode to commit
 * @flags: flags (%DIRTY_LPT_NODE or %OBSOLETE_LPT_NODE)
 * @iip: index in parent
 * @level: level in the tree (always greater than zero for nnodes)
 * @num: node number
 * @nbranch: branches to child nodes
 */
struct ubifs_nnode {
	struct ubifs_nnode *parent;
	struct ubifs_cnode *cnext;
	unsigned long flags;
	int iip;
	int level;
	int num;
	struct ubifs_nbranch nbranch[UBIFS_LPT_FANOUT];
};

/**
 * struct ubifs_lpt_heap - heap of categorized lprops.
 * @arr: heap array
 * @cnt: number in heap
 * @max_cnt: maximum number allowed in heap
 *
 * There are %LPROPS_HEAP_CNT heaps.
 */
struct ubifs_lpt_heap {
	struct ubifs_lprops **arr;
	int cnt;
	int max_cnt;
};

/*
 * Return codes for LPT scan callback function.
 *
 * LPT_SCAN_CONTINUE: continue scanning
 * LPT_SCAN_ADD: add the LEB properties scanned to the tree in memory
 * LPT_SCAN_STOP: stop scanning
 */
enum {
	LPT_SCAN_CONTINUE = 0,
	LPT_SCAN_ADD = 1,
	LPT_SCAN_STOP = 2,
};

struct ubifs_info;

/* Callback used by the 'ubifs_lpt_scan_nolock()' function */
typedef int (*ubifs_lpt_scan_callback)(struct ubifs_info *c,
				       const struct ubifs_lprops *lprops,
				       int in_tree, void *data);

/**
 * struct ubifs_wbuf - UBIFS write-buffer.
 * @c: UBIFS file-system description object
 * @buf: write-buffer (of min. flash I/O unit size)
 * @lnum: logical eraseblock number the write-buffer points to
 * @offs: write-buffer offset in this logical eraseblock
 * @avail: number of bytes available in the write-buffer
 * @used:  number of used bytes in the write-buffer
 * @size: write-buffer size (in [@c->min_io_size, @c->max_write_size] range)
 * @jhead: journal head the mutex belongs to (note, needed only to shut lockdep
 *         up by 'mutex_lock_nested()).
 * @sync_callback: write-buffer synchronization callback
 * @io_mutex: serializes write-buffer I/O
 * @lock: serializes @buf, @lnum, @offs, @avail, @used, @next_ino and @inodes
 *        fields
 * @softlimit: soft write-buffer timeout interval
 * @delta: hard and soft timeouts delta (the timer expire interval is @softlimit
 *         and @softlimit + @delta)
 * @timer: write-buffer timer
 * @no_timer: non-zero if this write-buffer does not have a timer
 * @need_sync: non-zero if the timer expired and the wbuf needs sync'ing
 * @next_ino: points to the next position of the following inode number
 * @inodes: stores the inode numbers of the nodes which are in wbuf
 *
 * The write-buffer synchronization callback is called when the write-buffer is
 * synchronized in order to notify how much space was wasted due to
 * write-buffer padding and how much free space is left in the LEB.
 *
 * Note: the fields @buf, @lnum, @offs, @avail and @used can be read under
 * spin-lock or mutex because they are written under both mutex and spin-lock.
 * @buf is appended to under mutex but overwritten under both mutex and
 * spin-lock. Thus the data between @buf and @buf + @used can be read under
 * spinlock.
 */
struct ubifs_wbuf {
	struct ubifs_info *c;
	void *buf;
	int lnum;
	int offs;
	int avail;
	int used;
	int size;
	int jhead;
	int (*sync_callback)(struct ubifs_info *c, int lnum, int free, int pad);
	struct mutex io_mutex;
	spinlock_t lock;
//	ktime_t softlimit;
//	unsigned long long delta;
//	struct hrtimer timer;
	unsigned int no_timer:1;
	unsigned int need_sync:1;
	int next_ino;
	ino_t *inodes;
};

/**
 * struct ubifs_bud - bud logical eraseblock.
 * @lnum: logical eraseblock number
 * @start: where the (uncommitted) bud data starts
 * @jhead: journal head number this bud belongs to
 * @list: link in the list buds belonging to the same journal head
 * @rb: link in the tree of all buds
 */
struct ubifs_bud {
	int lnum;
	int start;
	int jhead;
	struct list_head list;
	struct rb_node rb;
};

/**
 * struct ubifs_jhead - journal head.
 * @wbuf: head's write-buffer
 * @buds_list: list of bud LEBs belonging to this journal head
 * @grouped: non-zero if UBIFS groups nodes when writing to this journal head
 *
 * Note, the @buds list is protected by the @c->buds_lock.
 */
struct ubifs_jhead {
	struct ubifs_wbuf wbuf;
	struct list_head buds_list;
	unsigned int grouped:1;
};

/**
 * struct ubifs_zbranch - key/coordinate/length branch stored in znodes.
 * @key: key
 * @znode: znode address in memory
 * @lnum: LEB number of the target node (indexing node or data node)
 * @offs: target node offset within @lnum
 * @len: target node length
 */
struct ubifs_zbranch {
	union ubifs_key key;
	union {
		struct ubifs_znode *znode;
		void *leaf;
	};
	int lnum;
	int offs;
	int len;
};

/**
 * struct ubifs_znode - in-memory representation of an indexing node.
 * @parent: parent znode or NULL if it is the root
 * @cnext: next znode to commit
 * @flags: znode flags (%DIRTY_ZNODE, %COW_ZNODE or %OBSOLETE_ZNODE)
 * @time: last access time (seconds)
 * @level: level of the entry in the TNC tree
 * @child_cnt: count of child znodes
 * @iip: index in parent's zbranch array
 * @alt: lower bound of key range has altered i.e. child inserted at slot 0
 * @lnum: LEB number of the corresponding indexing node
 * @offs: offset of the corresponding indexing node
 * @len: length  of the corresponding indexing node
 * @zbranch: array of znode branches (@c->fanout elements)
 *
 * Note! The @lnum, @offs, and @len fields are not really needed - we have them
 * only for internal consistency check. They could be removed to save some RAM.
 */
struct ubifs_znode {
	struct ubifs_znode *parent;
	struct ubifs_znode *cnext;
	unsigned long flags;
	unsigned long time;
	int level;
	int child_cnt;
	int iip;
	int alt;
	int lnum;
	int offs;
	int len;
	struct ubifs_zbranch zbranch[];
};

/**
 * struct bu_info - bulk-read information.
 * @key: first data node key
 * @zbranch: zbranches of data nodes to bulk read
 * @buf: buffer to read into
 * @buf_len: buffer length
 * @gc_seq: GC sequence number to detect races with GC
 * @cnt: number of data nodes for bulk read
 * @blk_cnt: number of data blocks including holes
 * @oef: end of file reached
 */
struct bu_info {
	union ubifs_key key;
	struct ubifs_zbranch zbranch[UBIFS_MAX_BULK_READ];
	void *buf;
	int buf_len;
	int gc_seq;
	int cnt;
	int blk_cnt;
	int eof;
};

/**
 * struct ubifs_node_range - node length range description data structure.
 * @len: fixed node length
 * @min_len: minimum possible node length
 * @max_len: maximum possible node length
 *
 * If @max_len is %0, the node has fixed length @len.
 */
struct ubifs_node_range {
	union {
		int len;
		int min_len;
	};
	int max_len;
};

/**
 * struct ubifs_compressor - UBIFS compressor description structure.
 * @compr_type: compressor type (%UBIFS_COMPR_LZO, etc)
 * @cc: cryptoapi compressor handle
 * @comp_mutex: mutex used during compression
 * @decomp_mutex: mutex used during decompression
 * @name: compressor name
 * @capi_name: cryptoapi compressor name
 */
struct ubifs_compressor {
	int compr_type;
	struct crypto_comp *cc;
	struct mutex *comp_mutex;
	struct mutex *decomp_mutex;
	const char *name;
	const char *capi_name;
#ifdef __UBOOT__
	int (*decompress)(const unsigned char *in, size_t in_len,
			  unsigned char *out, size_t *out_len);
#endif
};

/**
 * struct ubifs_budget_req - budget requirements of an operation.
 *
 * @fast: non-zero if the budgeting should try to acquire budget quickly and
 *        should not try to call write-back
 * @recalculate: non-zero if @idx_growth, @data_growth, and @dd_growth fields
 *               have to be re-calculated
 * @new_page: non-zero if the operation adds a new page
 * @dirtied_page: non-zero if the operation makes a page dirty
 * @new_dent: non-zero if the operation adds a new directory entry
 * @mod_dent: non-zero if the operation removes or modifies an existing
 *            directory entry
 * @new_ino: non-zero if the operation adds a new inode
 * @new_ino_d: now much data newly created inode contains
 * @dirtied_ino: how many inodes the operation makes dirty
 * @dirtied_ino_d: now much data dirtied inode contains
 * @idx_growth: how much the index will supposedly grow
 * @data_growth: how much new data the operation will supposedly add
 * @dd_growth: how much data that makes other data dirty the operation will
 *             supposedly add
 *
 * @idx_growth, @data_growth and @dd_growth are not used in budget request. The
 * budgeting subsystem caches index and data growth values there to avoid
 * re-calculating them when the budget is released. However, if @idx_growth is
 * %-1, it is calculated by the release function using other fields.
 *
 * An inode may contain 4KiB of data at max., thus the widths of @new_ino_d
 * is 13 bits, and @dirtied_ino_d - 15, because up to 4 inodes may be made
 * dirty by the re-name operation.
 *
 * Note, UBIFS aligns node lengths to 8-bytes boundary, so the requester has to
 * make sure the amount of inode data which contribute to @new_ino_d and
 * @dirtied_ino_d fields are aligned.
 */
struct ubifs_budget_req {
	unsigned int fast:1;
	unsigned int recalculate:1;
#ifndef UBIFS_DEBUG
	unsigned int new_page:1;
	unsigned int dirtied_page:1;
	unsigned int new_dent:1;
	unsigned int mod_dent:1;
	unsigned int new_ino:1;
	unsigned int new_ino_d:13;
	unsigned int dirtied_ino:4;
	unsigned int dirtied_ino_d:15;
#else
	/* Not bit-fields to check for overflows */
	unsigned int new_page;
	unsigned int dirtied_page;
	unsigned int new_dent;
	unsigned int mod_dent;
	unsigned int new_ino;
	unsigned int new_ino_d;
	unsigned int dirtied_ino;
	unsigned int dirtied_ino_d;
#endif
	int idx_growth;
	int data_growth;
	int dd_growth;
};

/**
 * struct ubifs_orphan - stores the inode number of an orphan.
 * @rb: rb-tree node of rb-tree of orphans sorted by inode number
 * @list: list head of list of orphans in order added
 * @new_list: list head of list of orphans added since the last commit
 * @cnext: next orphan to commit
 * @dnext: next orphan to delete
 * @inum: inode number
 * @new: %1 => added since the last commit, otherwise %0
 * @cmt: %1 => commit pending, otherwise %0
 * @del: %1 => delete pending, otherwise %0
 */
struct ubifs_orphan {
	struct rb_node rb;
	struct list_head list;
	struct list_head new_list;
	struct ubifs_orphan *cnext;
	struct ubifs_orphan *dnext;
	ino_t inum;
	unsigned new:1;
	unsigned cmt:1;
	unsigned del:1;
};

/**
 * struct ubifs_mount_opts - UBIFS-specific mount options information.
 * @unmount_mode: selected unmount mode (%0 default, %1 normal, %2 fast)
 * @bulk_read: enable/disable bulk-reads (%0 default, %1 disable, %2 enable)
 * @chk_data_crc: enable/disable CRC data checking when reading data nodes
 *                (%0 default, %1 disable, %2 enable)
 * @override_compr: override default compressor (%0 - do not override and use
 *                  superblock compressor, %1 - override and use compressor
 *                  specified in @compr_type)
 * @compr_type: compressor type to override the superblock compressor with
 *              (%UBIFS_COMPR_NONE, etc)
 */
struct ubifs_mount_opts {
	unsigned int unmount_mode:2;
	unsigned int bulk_read:2;
	unsigned int chk_data_crc:2;
	unsigned int override_compr:1;
	unsigned int compr_type:2;
};

/**
 * struct ubifs_budg_info - UBIFS budgeting information.
 * @idx_growth: amount of bytes budgeted for index growth
 * @data_growth: amount of bytes budgeted for cached data
 * @dd_growth: amount of bytes budgeted for cached data that will make
 *             other data dirty
 * @uncommitted_idx: amount of bytes were budgeted for growth of the index, but
 *                   which still have to be taken into account because the index
 *                   has not been committed so far
 * @old_idx_sz: size of index on flash
 * @min_idx_lebs: minimum number of LEBs required for the index
 * @nospace: non-zero if the file-system does not have flash space (used as
 *           optimization)
 * @nospace_rp: the same as @nospace, but additionally means that even reserved
 *              pool is full
 * @page_budget: budget for a page (constant, never changed after mount)
 * @inode_budget: budget for an inode (constant, never changed after mount)
 * @dent_budget: budget for a directory entry (constant, never changed after
 *               mount)
 */
struct ubifs_budg_info {
	long long idx_growth;
	long long data_growth;
	long long dd_growth;
	long long uncommitted_idx;
	unsigned long long old_idx_sz;
	int min_idx_lebs;
	unsigned int nospace:1;
	unsigned int nospace_rp:1;
	int page_budget;
	int inode_budget;
	int dent_budget;
};

struct ubifs_debug_info;

/**
 * struct ubifs_info - UBIFS file-system description data structure
 * (per-superblock).
 * @vfs_sb: VFS @struct super_block object
 * @bdi: backing device info object to make VFS happy and disable read-ahead
 *
 * @highest_inum: highest used inode number
 * @max_sqnum: current global sequence number
 * @cmt_no: commit number of the last successfully completed commit, protected
 *          by @commit_sem
 * @cnt_lock: protects @highest_inum and @max_sqnum counters
 * @fmt_version: UBIFS on-flash format version
 * @ro_compat_version: R/O compatibility version
 * @uuid: UUID from super block
 *
 * @lhead_lnum: log head logical eraseblock number
 * @lhead_offs: log head offset
 * @ltail_lnum: log tail logical eraseblock number (offset is always 0)
 * @log_mutex: protects the log, @lhead_lnum, @lhead_offs, @ltail_lnum, and
 *             @bud_bytes
 * @min_log_bytes: minimum required number of bytes in the log
 * @cmt_bud_bytes: used during commit to temporarily amount of bytes in
 *                 committed buds
 *
 * @buds: tree of all buds indexed by bud LEB number
 * @bud_bytes: how many bytes of flash is used by buds
 * @buds_lock: protects the @buds tree, @bud_bytes, and per-journal head bud
 *             lists
 * @jhead_cnt: count of journal heads
 * @jheads: journal heads (head zero is base head)
 * @max_bud_bytes: maximum number of bytes allowed in buds
 * @bg_bud_bytes: number of bud bytes when background commit is initiated
 * @old_buds: buds to be released after commit ends
 * @max_bud_cnt: maximum number of buds
 *
 * @commit_sem: synchronizes committer with other processes
 * @cmt_state: commit state
 * @cs_lock: commit state lock
 * @cmt_wq: wait queue to sleep on if the log is full and a commit is running
 *
 * @big_lpt: flag that LPT is too big to write whole during commit
 * @space_fixup: flag indicating that free space in LEBs needs to be cleaned up
 * @no_chk_data_crc: do not check CRCs when reading data nodes (except during
 *                   recovery)
 * @bulk_read: enable bulk-reads
 * @default_compr: default compression algorithm (%UBIFS_COMPR_LZO, etc)
 * @rw_incompat: the media is not R/W compatible
 *
 * @tnc_mutex: protects the Tree Node Cache (TNC), @zroot, @cnext, @enext, and
 *             @calc_idx_sz
 * @zroot: zbranch which points to the root index node and znode
 * @cnext: next znode to commit
 * @enext: next znode to commit to empty space
 * @gap_lebs: array of LEBs used by the in-gaps commit method
 * @cbuf: commit buffer
 * @ileb_buf: buffer for commit in-the-gaps method
 * @ileb_len: length of data in ileb_buf
 * @ihead_lnum: LEB number of index head
 * @ihead_offs: offset of index head
 * @ilebs: pre-allocated index LEBs
 * @ileb_cnt: number of pre-allocated index LEBs
 * @ileb_nxt: next pre-allocated index LEBs
 * @old_idx: tree of index nodes obsoleted since the last commit start
 * @bottom_up_buf: a buffer which is used by 'dirty_cow_bottom_up()' in tnc.c
 *
 * @mst_node: master node
 * @mst_offs: offset of valid master node
 *
 * @max_bu_buf_len: maximum bulk-read buffer length
 * @bu_mutex: protects the pre-allocated bulk-read buffer and @c->bu
 * @bu: pre-allocated bulk-read information
 *
 * @write_reserve_mutex: protects @write_reserve_buf
 * @write_reserve_buf: on the write path we allocate memory, which might
 *                     sometimes be unavailable, in which case we use this
 *                     write reserve buffer
 *
 * @log_lebs: number of logical eraseblocks in the log
 * @log_bytes: log size in bytes
 * @log_last: last LEB of the log
 * @lpt_lebs: number of LEBs used for lprops table
 * @lpt_first: first LEB of the lprops table area
 * @lpt_last: last LEB of the lprops table area
 * @orph_lebs: number of LEBs used for the orphan area
 * @orph_first: first LEB of the orphan area
 * @orph_last: last LEB of the orphan area
 * @main_lebs: count of LEBs in the main area
 * @main_first: first LEB of the main area
 * @main_bytes: main area size in bytes
 *
 * @key_hash_type: type of the key hash
 * @key_hash: direntry key hash function
 * @key_fmt: key format
 * @key_len: key length
 * @fanout: fanout of the index tree (number of links per indexing node)
 *
 * @min_io_size: minimal input/output unit size
 * @min_io_shift: number of bits in @min_io_size minus one
 * @max_write_size: maximum amount of bytes the underlying flash can write at a
 *                  time (MTD write buffer size)
 * @max_write_shift: number of bits in @max_write_size minus one
 * @leb_size: logical eraseblock size in bytes
 * @leb_start: starting offset of logical eraseblocks within physical
 *             eraseblocks
 * @half_leb_size: half LEB size
 * @idx_leb_size: how many bytes of an LEB are effectively available when it is
 *                used to store indexing nodes (@leb_size - @max_idx_node_sz)
 * @leb_cnt: count of logical eraseblocks
 * @max_leb_cnt: maximum count of logical eraseblocks
 * @old_leb_cnt: count of logical eraseblocks before re-size
 * @ro_media: the underlying UBI volume is read-only
 * @ro_mount: the file-system was mounted as read-only
 * @ro_error: UBIFS switched to R/O mode because an error happened
 *
 * @dirty_pg_cnt: number of dirty pages (not used)
 * @dirty_zn_cnt: number of dirty znodes
 * @clean_zn_cnt: number of clean znodes
 *
 * @space_lock: protects @bi and @lst
 * @lst: lprops statistics
 * @bi: budgeting information
 * @calc_idx_sz: temporary variable which is used to calculate new index size
 *               (contains accurate new index size at end of TNC commit start)
 *
 * @ref_node_alsz: size of the LEB reference node aligned to the min. flash
 *                 I/O unit
 * @mst_node_alsz: master node aligned size
 * @min_idx_node_sz: minimum indexing node aligned on 8-bytes boundary
 * @max_idx_node_sz: maximum indexing node aligned on 8-bytes boundary
 * @max_inode_sz: maximum possible inode size in bytes
 * @max_znode_sz: size of znode in bytes
 *
 * @leb_overhead: how many bytes are wasted in an LEB when it is filled with
 *                data nodes of maximum size - used in free space reporting
 * @dead_wm: LEB dead space watermark
 * @dark_wm: LEB dark space watermark
 * @block_cnt: count of 4KiB blocks on the FS
 *
 * @ranges: UBIFS node length ranges
 * @ubi: UBI volume descriptor
 * @di: UBI device information
 * @vi: UBI volume information
 *
 * @orph_tree: rb-tree of orphan inode numbers
 * @orph_list: list of orphan inode numbers in order added
 * @orph_new: list of orphan inode numbers added since last commit
 * @orph_cnext: next orphan to commit
 * @orph_dnext: next orphan to delete
 * @orphan_lock: lock for orph_tree and orph_new
 * @orph_buf: buffer for orphan nodes
 * @new_orphans: number of orphans since last commit
 * @cmt_orphans: number of orphans being committed
 * @tot_orphans: number of orphans in the rb_tree
 * @max_orphans: maximum number of orphans allowed
 * @ohead_lnum: orphan head LEB number
 * @ohead_offs: orphan head offset
 * @no_orphs: non-zero if there are no orphans
 *
 * @bgt: UBIFS background thread
 * @bgt_name: background thread name
 * @need_bgt: if background thread should run
 * @need_wbuf_sync: if write-buffers have to be synchronized
 *
 * @gc_lnum: LEB number used for garbage collection
 * @sbuf: a buffer of LEB size used by GC and replay for scanning
 * @idx_gc: list of index LEBs that have been garbage collected
 * @idx_gc_cnt: number of elements on the idx_gc list
 * @gc_seq: incremented for every non-index LEB garbage collected
 * @gced_lnum: last non-index LEB that was garbage collected
 *
 * @infos_list: links all 'ubifs_info' objects
 * @umount_mutex: serializes shrinker and un-mount
 * @shrinker_run_no: shrinker run number
 *
 * @space_bits: number of bits needed to record free or dirty space
 * @lpt_lnum_bits: number of bits needed to record a LEB number in the LPT
 * @lpt_offs_bits: number of bits needed to record an offset in the LPT
 * @lpt_spc_bits: number of bits needed to space in the LPT
 * @pcnt_bits: number of bits needed to record pnode or nnode number
 * @lnum_bits: number of bits needed to record LEB number
 * @nnode_sz: size of on-flash nnode
 * @pnode_sz: size of on-flash pnode
 * @ltab_sz: size of on-flash LPT lprops table
 * @lsave_sz: size of on-flash LPT save table
 * @pnode_cnt: number of pnodes
 * @nnode_cnt: number of nnodes
 * @lpt_hght: height of the LPT
 * @pnodes_have: number of pnodes in memory
 *
 * @lp_mutex: protects lprops table and all the other lprops-related fields
 * @lpt_lnum: LEB number of the root nnode of the LPT
 * @lpt_offs: offset of the root nnode of the LPT
 * @nhead_lnum: LEB number of LPT head
 * @nhead_offs: offset of LPT head
 * @lpt_drty_flgs: dirty flags for LPT special nodes e.g. ltab
 * @dirty_nn_cnt: number of dirty nnodes
 * @dirty_pn_cnt: number of dirty pnodes
 * @check_lpt_free: flag that indicates LPT GC may be needed
 * @lpt_sz: LPT size
 * @lpt_nod_buf: buffer for an on-flash nnode or pnode
 * @lpt_buf: buffer of LEB size used by LPT
 * @nroot: address in memory of the root nnode of the LPT
 * @lpt_cnext: next LPT node to commit
 * @lpt_heap: array of heaps of categorized lprops
 * @dirty_idx: a (reverse sorted) copy of the LPROPS_DIRTY_IDX heap as at
 *             previous commit start
 * @uncat_list: list of un-categorized LEBs
 * @empty_list: list of empty LEBs
 * @freeable_list: list of freeable non-index LEBs (free + dirty == @leb_size)
 * @frdi_idx_list: list of freeable index LEBs (free + dirty == @leb_size)
 * @freeable_cnt: number of freeable LEBs in @freeable_list
 * @in_a_category_cnt: count of lprops which are in a certain category, which
 *                     basically meants that they were loaded from the flash
 *
 * @ltab_lnum: LEB number of LPT's own lprops table
 * @ltab_offs: offset of LPT's own lprops table
 * @ltab: LPT's own lprops table
 * @ltab_cmt: LPT's own lprops table (commit copy)
 * @lsave_cnt: number of LEB numbers in LPT's save table
 * @lsave_lnum: LEB number of LPT's save table
 * @lsave_offs: offset of LPT's save table
 * @lsave: LPT's save table
 * @lscan_lnum: LEB number of last LPT scan
 *
 * @rp_size: size of the reserved pool in bytes
 * @report_rp_size: size of the reserved pool reported to user-space
 * @rp_uid: reserved pool user ID
 * @rp_gid: reserved pool group ID
 *
 * @empty: %1 if the UBI device is empty
 * @need_recovery: %1 if the file-system needs recovery
 * @replaying: %1 during journal replay
 * @mounting: %1 while mounting
 * @probing: %1 while attempting to mount if MS_SILENT mount flag is set
 * @remounting_rw: %1 while re-mounting from R/O mode to R/W mode
 * @replay_list: temporary list used during journal replay
 * @replay_buds: list of buds to replay
 * @cs_sqnum: sequence number of first node in the log (commit start node)
 * @replay_sqnum: sequence number of node currently being replayed
 * @unclean_leb_list: LEBs to recover when re-mounting R/O mounted FS to R/W
 *                    mode
 * @rcvrd_mst_node: recovered master node to write when re-mounting R/O mounted
 *                  FS to R/W mode
 * @size_tree: inode size information for recovery
 * @mount_opts: UBIFS-specific mount options
 *
 * @dbg: debugging-related information
 */
struct ubifs_info {
	struct super_block *vfs_sb;
#ifndef __UBOOT__
	struct backing_dev_info bdi;
#endif

	ino_t highest_inum;
	unsigned long long max_sqnum;
	unsigned long long cmt_no;
	spinlock_t cnt_lock;
	int fmt_version;
	int ro_compat_version;
	unsigned char uuid[16];

	int lhead_lnum;
	int lhead_offs;
	int ltail_lnum;
	struct mutex log_mutex;
	int min_log_bytes;
	long long cmt_bud_bytes;

	struct rb_root buds;
	long long bud_bytes;
	spinlock_t buds_lock;
	int jhead_cnt;
	struct ubifs_jhead *jheads;
	long long max_bud_bytes;
	long long bg_bud_bytes;
	struct list_head old_buds;
	int max_bud_cnt;

	struct rw_semaphore commit_sem;
	int cmt_state;
	spinlock_t cs_lock;
	wait_queue_head_t cmt_wq;

	unsigned int big_lpt:1;
	unsigned int space_fixup:1;
	unsigned int no_chk_data_crc:1;
	unsigned int bulk_read:1;
	unsigned int default_compr:2;
	unsigned int rw_incompat:1;

	struct mutex tnc_mutex;
	struct ubifs_zbranch zroot;
	struct ubifs_znode *cnext;
	struct ubifs_znode *enext;
	int *gap_lebs;
	void *cbuf;
	void *ileb_buf;
	int ileb_len;
	int ihead_lnum;
	int ihead_offs;
	int *ilebs;
	int ileb_cnt;
	int ileb_nxt;
	struct rb_root old_idx;
	int *bottom_up_buf;

	struct ubifs_mst_node *mst_node;
	int mst_offs;

	int max_bu_buf_len;
	struct mutex bu_mutex;
	struct bu_info bu;

	struct mutex write_reserve_mutex;
	void *write_reserve_buf;

	int log_lebs;
	long long log_bytes;
	int log_last;
	int lpt_lebs;
	int lpt_first;
	int lpt_last;
	int orph_lebs;
	int orph_first;
	int orph_last;
	int main_lebs;
	int main_first;
	long long main_bytes;

	uint8_t key_hash_type;
	uint32_t (*key_hash)(const char *str, int len);
	int key_fmt;
	int key_len;
	int fanout;

	int min_io_size;
	int min_io_shift;
	int max_write_size;
	int max_write_shift;
	int leb_size;
	int leb_start;
	int half_leb_size;
	int idx_leb_size;
	int leb_cnt;
	int max_leb_cnt;
	int old_leb_cnt;
	unsigned int ro_media:1;
	unsigned int ro_mount:1;
	unsigned int ro_error:1;

	atomic_long_t dirty_pg_cnt;
	atomic_long_t dirty_zn_cnt;
	atomic_long_t clean_zn_cnt;

	spinlock_t space_lock;
	struct ubifs_lp_stats lst;
	struct ubifs_budg_info bi;
	unsigned long long calc_idx_sz;

	int ref_node_alsz;
	int mst_node_alsz;
	int min_idx_node_sz;
	int max_idx_node_sz;
	long long max_inode_sz;
	int max_znode_sz;

	int leb_overhead;
	int dead_wm;
	int dark_wm;
	int block_cnt;

	struct ubifs_node_range ranges[UBIFS_NODE_TYPES_CNT];
	struct ubi_volume_desc *ubi;
	struct ubi_device_info di;
	struct ubi_volume_info vi;

	struct rb_root orph_tree;
	struct list_head orph_list;
	struct list_head orph_new;
	struct ubifs_orphan *orph_cnext;
	struct ubifs_orphan *orph_dnext;
	spinlock_t orphan_lock;
	void *orph_buf;
	int new_orphans;
	int cmt_orphans;
	int tot_orphans;
	int max_orphans;
	int ohead_lnum;
	int ohead_offs;
	int no_orphs;

	struct task_struct *bgt;
	char bgt_name[sizeof(BGT_NAME_PATTERN) + 9];
	int need_bgt;
	int need_wbuf_sync;

	int gc_lnum;
	void *sbuf;
	struct list_head idx_gc;
	int idx_gc_cnt;
	int gc_seq;
	int gced_lnum;

	struct list_head infos_list;
	struct mutex umount_mutex;
	unsigned int shrinker_run_no;

	int space_bits;
	int lpt_lnum_bits;
	int lpt_offs_bits;
	int lpt_spc_bits;
	int pcnt_bits;
	int lnum_bits;
	int nnode_sz;
	int pnode_sz;
	int ltab_sz;
	int lsave_sz;
	int pnode_cnt;
	int nnode_cnt;
	int lpt_hght;
	int pnodes_have;

	struct mutex lp_mutex;
	int lpt_lnum;
	int lpt_offs;
	int nhead_lnum;
	int nhead_offs;
	int lpt_drty_flgs;
	int dirty_nn_cnt;
	int dirty_pn_cnt;
	int check_lpt_free;
	long long lpt_sz;
	void *lpt_nod_buf;
	void *lpt_buf;
	struct ubifs_nnode *nroot;
	struct ubifs_cnode *lpt_cnext;
	struct ubifs_lpt_heap lpt_heap[LPROPS_HEAP_CNT];
	struct ubifs_lpt_heap dirty_idx;
	struct list_head uncat_list;
	struct list_head empty_list;
	struct list_head freeable_list;
	struct list_head frdi_idx_list;
	int freeable_cnt;
	int in_a_category_cnt;

	int ltab_lnum;
	int ltab_offs;
	struct ubifs_lpt_lprops *ltab;
	struct ubifs_lpt_lprops *ltab_cmt;
	int lsave_cnt;
	int lsave_lnum;
	int lsave_offs;
	int *lsave;
	int lscan_lnum;

	long long rp_size;
	long long report_rp_size;
	kuid_t rp_uid;
	kgid_t rp_gid;

	/* The below fields are used only during mounting and re-mounting */
	unsigned int empty:1;
	unsigned int need_recovery:1;
	unsigned int replaying:1;
	unsigned int mounting:1;
	unsigned int remounting_rw:1;
	unsigned int probing:1;
	struct list_head replay_list;
	struct list_head replay_buds;
	unsigned long long cs_sqnum;
	unsigned long long replay_sqnum;
	struct list_head unclean_leb_list;
	struct ubifs_mst_node *rcvrd_mst_node;
	struct rb_root size_tree;
	struct ubifs_mount_opts mount_opts;

#ifndef __UBOOT__
	struct ubifs_debug_info *dbg;
#endif
};

extern struct list_head ubifs_infos;
extern spinlock_t ubifs_infos_lock;
extern atomic_long_t ubifs_clean_zn_cnt;
extern struct kmem_cache *ubifs_inode_slab;
extern const struct super_operations ubifs_super_operations;
extern const struct xattr_handler *ubifs_xattr_handlers[];
extern const struct address_space_operations ubifs_file_address_operations;
extern const struct file_operations ubifs_file_operations;
extern const struct inode_operations ubifs_file_inode_operations;
extern const struct file_operations ubifs_dir_operations;
extern const struct inode_operations ubifs_dir_inode_operations;
extern const struct inode_operations ubifs_symlink_inode_operations;
extern struct backing_dev_info ubifs_backing_dev_info;
extern struct ubifs_compressor *ubifs_compressors[UBIFS_COMPR_TYPES_CNT];

/* io.c */
void ubifs_ro_mode(struct ubifs_info *c, int err);
int ubifs_leb_read(const struct ubifs_info *c, int lnum, void *buf, int offs,
		   int len, int even_ebadmsg);
int ubifs_leb_write(struct ubifs_info *c, int lnum, const void *buf, int offs,
		    int len);
int ubifs_leb_change(struct ubifs_info *c, int lnum, const void *buf, int len);
int ubifs_leb_unmap(struct ubifs_info *c, int lnum);
int ubifs_leb_map(struct ubifs_info *c, int lnum);
int ubifs_is_mapped(const struct ubifs_info *c, int lnum);
int ubifs_wbuf_write_nolock(struct ubifs_wbuf *wbuf, void *buf, int len);
int ubifs_wbuf_seek_nolock(struct ubifs_wbuf *wbuf, int lnum, int offs);
int ubifs_wbuf_init(struct ubifs_info *c, struct ubifs_wbuf *wbuf);
int ubifs_read_node(const struct ubifs_info *c, void *buf, int type, int len,
		    int lnum, int offs);
int ubifs_read_node_wbuf(struct ubifs_wbuf *wbuf, void *buf, int type, int len,
			 int lnum, int offs);
int ubifs_write_node(struct ubifs_info *c, void *node, int len, int lnum,
		     int offs);
int ubifs_check_node(const struct ubifs_info *c, const void *buf, int lnum,
		     int offs, int quiet, int must_chk_crc);
void ubifs_prepare_node(struct ubifs_info *c, void *buf, int len, int pad);
void ubifs_prep_grp_node(struct ubifs_info *c, void *node, int len, int last);
int ubifs_io_init(struct ubifs_info *c);
void ubifs_pad(const struct ubifs_info *c, void *buf, int pad);
int ubifs_wbuf_sync_nolock(struct ubifs_wbuf *wbuf);
int ubifs_bg_wbufs_sync(struct ubifs_info *c);
void ubifs_wbuf_add_ino_nolock(struct ubifs_wbuf *wbuf, ino_t inum);
int ubifs_sync_wbufs_by_inode(struct ubifs_info *c, struct inode *inode);

/* scan.c */
struct ubifs_scan_leb *ubifs_scan(const struct ubifs_info *c, int lnum,
				  int offs, void *sbuf, int quiet);
void ubifs_scan_destroy(struct ubifs_scan_leb *sleb);
int ubifs_scan_a_node(const struct ubifs_info *c, void *buf, int len, int lnum,
		      int offs, int quiet);
struct ubifs_scan_leb *ubifs_start_scan(const struct ubifs_info *c, int lnum,
					int offs, void *sbuf);
void ubifs_end_scan(const struct ubifs_info *c, struct ubifs_scan_leb *sleb,
		    int lnum, int offs);
int ubifs_add_snod(const struct ubifs_info *c, struct ubifs_scan_leb *sleb,
		   void *buf, int offs);
void ubifs_scanned_corruption(const struct ubifs_info *c, int lnum, int offs,
			      void *buf);

/* log.c */
void ubifs_add_bud(struct ubifs_info *c, struct ubifs_bud *bud);
void ubifs_create_buds_lists(struct ubifs_info *c);
int ubifs_add_bud_to_log(struct ubifs_info *c, int jhead, int lnum, int offs);
struct ubifs_bud *ubifs_search_bud(struct ubifs_info *c, int lnum);
struct ubifs_wbuf *ubifs_get_wbuf(struct ubifs_info *c, int lnum);
int ubifs_log_start_commit(struct ubifs_info *c, int *ltail_lnum);
int ubifs_log_end_commit(struct ubifs_info *c, int new_ltail_lnum);
int ubifs_log_post_commit(struct ubifs_info *c, int old_ltail_lnum);
int ubifs_consolidate_log(struct ubifs_info *c);

/* journal.c */
int ubifs_jnl_update(struct ubifs_info *c, const struct inode *dir,
		     const struct qstr *nm, const struct inode *inode,
		     int deletion, int xent);
int ubifs_jnl_write_data(struct ubifs_info *c, const struct inode *inode,
			 const union ubifs_key *key, const void *buf, int len);
int ubifs_jnl_write_inode(struct ubifs_info *c, const struct inode *inode);
int ubifs_jnl_delete_inode(struct ubifs_info *c, const struct inode *inode);
int ubifs_jnl_rename(struct ubifs_info *c, const struct inode *old_dir,
		     const struct dentry *old_dentry,
		     const struct inode *new_dir,
		     const struct dentry *new_dentry, int sync);
int ubifs_jnl_truncate(struct ubifs_info *c, const struct inode *inode,
		       loff_t old_size, loff_t new_size);
int ubifs_jnl_delete_xattr(struct ubifs_info *c, const struct inode *host,
			   const struct inode *inode, const struct qstr *nm);
int ubifs_jnl_change_xattr(struct ubifs_info *c, const struct inode *inode1,
			   const struct inode *inode2);

/* budget.c */
int ubifs_budget_space(struct ubifs_info *c, struct ubifs_budget_req *req);
void ubifs_release_budget(struct ubifs_info *c, struct ubifs_budget_req *req);
void ubifs_release_dirty_inode_budget(struct ubifs_info *c,
				      struct ubifs_inode *ui);
int ubifs_budget_inode_op(struct ubifs_info *c, struct inode *inode,
			  struct ubifs_budget_req *req);
void ubifs_release_ino_dirty(struct ubifs_info *c, struct inode *inode,
				struct ubifs_budget_req *req);
void ubifs_cancel_ino_op(struct ubifs_info *c, struct inode *inode,
			 struct ubifs_budget_req *req);
long long ubifs_get_free_space(struct ubifs_info *c);
long long ubifs_get_free_space_nolock(struct ubifs_info *c);
int ubifs_calc_min_idx_lebs(struct ubifs_info *c);
void ubifs_convert_page_budget(struct ubifs_info *c);
long long ubifs_reported_space(const struct ubifs_info *c, long long free);
long long ubifs_calc_available(const struct ubifs_info *c, int min_idx_lebs);

/* find.c */
int ubifs_find_free_space(struct ubifs_info *c, int min_space, int *offs,
			  int squeeze);
int ubifs_find_free_leb_for_idx(struct ubifs_info *c);
int ubifs_find_dirty_leb(struct ubifs_info *c, struct ubifs_lprops *ret_lp,
			 int min_space, int pick_free);
int ubifs_find_dirty_idx_leb(struct ubifs_info *c);
int ubifs_save_dirty_idx_lnums(struct ubifs_info *c);

/* tnc.c */
int ubifs_lookup_level0(struct ubifs_info *c, const union ubifs_key *key,
			struct ubifs_znode **zn, int *n);
int ubifs_tnc_lookup_nm(struct ubifs_info *c, const union ubifs_key *key,
			void *node, const struct qstr *nm);
int ubifs_tnc_locate(struct ubifs_info *c, const union ubifs_key *key,
		     void *node, int *lnum, int *offs);
int ubifs_tnc_add(struct ubifs_info *c, const union ubifs_key *key, int lnum,
		  int offs, int len);
int ubifs_tnc_replace(struct ubifs_info *c, const union ubifs_key *key,
		      int old_lnum, int old_offs, int lnum, int offs, int len);
int ubifs_tnc_add_nm(struct ubifs_info *c, const union ubifs_key *key,
		     int lnum, int offs, int len, const struct qstr *nm);
int ubifs_tnc_remove(struct ubifs_info *c, const union ubifs_key *key);
int ubifs_tnc_remove_nm(struct ubifs_info *c, const union ubifs_key *key,
			const struct qstr *nm);
int ubifs_tnc_remove_range(struct ubifs_info *c, union ubifs_key *from_key,
			   union ubifs_key *to_key);
int ubifs_tnc_remove_ino(struct ubifs_info *c, ino_t inum);
struct ubifs_dent_node *ubifs_tnc_next_ent(struct ubifs_info *c,
					   union ubifs_key *key,
					   const struct qstr *nm);
void ubifs_tnc_close(struct ubifs_info *c);
int ubifs_tnc_has_node(struct ubifs_info *c, union ubifs_key *key, int level,
		       int lnum, int offs, int is_idx);
int ubifs_dirty_idx_node(struct ubifs_info *c, union ubifs_key *key, int level,
			 int lnum, int offs);
/* Shared by tnc.c for tnc_commit.c */
void destroy_old_idx(struct ubifs_info *c);
int is_idx_node_in_tnc(struct ubifs_info *c, union ubifs_key *key, int level,
		       int lnum, int offs);
int insert_old_idx_znode(struct ubifs_info *c, struct ubifs_znode *znode);
int ubifs_tnc_get_bu_keys(struct ubifs_info *c, struct bu_info *bu);
int ubifs_tnc_bulk_read(struct ubifs_info *c, struct bu_info *bu);

/* tnc_misc.c */
struct ubifs_znode *ubifs_tnc_levelorder_next(struct ubifs_znode *zr,
					      struct ubifs_znode *znode);
int ubifs_search_zbranch(const struct ubifs_info *c,
			 const struct ubifs_znode *znode,
			 const union ubifs_key *key, int *n);
struct ubifs_znode *ubifs_tnc_postorder_first(struct ubifs_znode *znode);
struct ubifs_znode *ubifs_tnc_postorder_next(struct ubifs_znode *znode);
long ubifs_destroy_tnc_subtree(struct ubifs_znode *zr);
struct ubifs_znode *ubifs_load_znode(struct ubifs_info *c,
				     struct ubifs_zbranch *zbr,
				     struct ubifs_znode *parent, int iip);
int ubifs_tnc_read_node(struct ubifs_info *c, struct ubifs_zbranch *zbr,
			void *node);

/* tnc_commit.c */
int ubifs_tnc_start_commit(struct ubifs_info *c, struct ubifs_zbranch *zroot);
int ubifs_tnc_end_commit(struct ubifs_info *c);

#ifndef __UBOOT__
/* shrinker.c */
unsigned long ubifs_shrink_scan(struct shrinker *shrink,
				struct shrink_control *sc);
unsigned long ubifs_shrink_count(struct shrinker *shrink,
				 struct shrink_control *sc);
#endif

/* commit.c */
int ubifs_bg_thread(void *info);
void ubifs_commit_required(struct ubifs_info *c);
void ubifs_request_bg_commit(struct ubifs_info *c);
int ubifs_run_commit(struct ubifs_info *c);
void ubifs_recovery_commit(struct ubifs_info *c);
int ubifs_gc_should_commit(struct ubifs_info *c);
void ubifs_wait_for_commit(struct ubifs_info *c);

/* master.c */
int ubifs_read_master(struct ubifs_info *c);
int ubifs_write_master(struct ubifs_info *c);

/* sb.c */
int ubifs_read_superblock(struct ubifs_info *c);
struct ubifs_sb_node *ubifs_read_sb_node(struct ubifs_info *c);
int ubifs_write_sb_node(struct ubifs_info *c, struct ubifs_sb_node *sup);
int ubifs_fixup_free_space(struct ubifs_info *c);

/* replay.c */
int ubifs_validate_entry(struct ubifs_info *c,
			 const struct ubifs_dent_node *dent);
int ubifs_replay_journal(struct ubifs_info *c);

/* gc.c */
int ubifs_garbage_collect(struct ubifs_info *c, int anyway);
int ubifs_gc_start_commit(struct ubifs_info *c);
int ubifs_gc_end_commit(struct ubifs_info *c);
void ubifs_destroy_idx_gc(struct ubifs_info *c);
int ubifs_get_idx_gc_leb(struct ubifs_info *c);
int ubifs_garbage_collect_leb(struct ubifs_info *c, struct ubifs_lprops *lp);

/* orphan.c */
int ubifs_add_orphan(struct ubifs_info *c, ino_t inum);
void ubifs_delete_orphan(struct ubifs_info *c, ino_t inum);
int ubifs_orphan_start_commit(struct ubifs_info *c);
int ubifs_orphan_end_commit(struct ubifs_info *c);
int ubifs_mount_orphans(struct ubifs_info *c, int unclean, int read_only);
int ubifs_clear_orphans(struct ubifs_info *c);

/* lpt.c */
int ubifs_calc_lpt_geom(struct ubifs_info *c);
int ubifs_create_dflt_lpt(struct ubifs_info *c, int *main_lebs, int lpt_first,
			  int *lpt_lebs, int *big_lpt);
int ubifs_lpt_init(struct ubifs_info *c, int rd, int wr);
struct ubifs_lprops *ubifs_lpt_lookup(struct ubifs_info *c, int lnum);
struct ubifs_lprops *ubifs_lpt_lookup_dirty(struct ubifs_info *c, int lnum);
int ubifs_lpt_scan_nolock(struct ubifs_info *c, int start_lnum, int end_lnum,
			  ubifs_lpt_scan_callback scan_cb, void *data);

/* Shared by lpt.c for lpt_commit.c */
void ubifs_pack_lsave(struct ubifs_info *c, void *buf, int *lsave);
void ubifs_pack_ltab(struct ubifs_info *c, void *buf,
		     struct ubifs_lpt_lprops *ltab);
void ubifs_pack_pnode(struct ubifs_info *c, void *buf,
		      struct ubifs_pnode *pnode);
void ubifs_pack_nnode(struct ubifs_info *c, void *buf,
		      struct ubifs_nnode *nnode);
struct ubifs_pnode *ubifs_get_pnode(struct ubifs_info *c,
				    struct ubifs_nnode *parent, int iip);
struct ubifs_nnode *ubifs_get_nnode(struct ubifs_info *c,
				    struct ubifs_nnode *parent, int iip);
int ubifs_read_nnode(struct ubifs_info *c, struct ubifs_nnode *parent, int iip);
void ubifs_add_lpt_dirt(struct ubifs_info *c, int lnum, int dirty);
void ubifs_add_nnode_dirt(struct ubifs_info *c, struct ubifs_nnode *nnode);
uint32_t ubifs_unpack_bits(uint8_t **addr, int *pos, int nrbits);
struct ubifs_nnode *ubifs_first_nnode(struct ubifs_info *c, int *hght);
/* Needed only in debugging code in lpt_commit.c */
int ubifs_unpack_nnode(const struct ubifs_info *c, void *buf,
		       struct ubifs_nnode *nnode);

/* lpt_commit.c */
int ubifs_lpt_start_commit(struct ubifs_info *c);
int ubifs_lpt_end_commit(struct ubifs_info *c);
int ubifs_lpt_post_commit(struct ubifs_info *c);
void ubifs_lpt_free(struct ubifs_info *c, int wr_only);

/* lprops.c */
const struct ubifs_lprops *ubifs_change_lp(struct ubifs_info *c,
					   const struct ubifs_lprops *lp,
					   int free, int dirty, int flags,
					   int idx_gc_cnt);
void ubifs_get_lp_stats(struct ubifs_info *c, struct ubifs_lp_stats *lst);
void ubifs_add_to_cat(struct ubifs_info *c, struct ubifs_lprops *lprops,
		      int cat);
void ubifs_replace_cat(struct ubifs_info *c, struct ubifs_lprops *old_lprops,
		       struct ubifs_lprops *new_lprops);
void ubifs_ensure_cat(struct ubifs_info *c, struct ubifs_lprops *lprops);
int ubifs_categorize_lprops(const struct ubifs_info *c,
			    const struct ubifs_lprops *lprops);
int ubifs_change_one_lp(struct ubifs_info *c, int lnum, int free, int dirty,
			int flags_set, int flags_clean, int idx_gc_cnt);
int ubifs_update_one_lp(struct ubifs_info *c, int lnum, int free, int dirty,
			int flags_set, int flags_clean);
int ubifs_read_one_lp(struct ubifs_info *c, int lnum, struct ubifs_lprops *lp);
const struct ubifs_lprops *ubifs_fast_find_free(struct ubifs_info *c);
const struct ubifs_lprops *ubifs_fast_find_empty(struct ubifs_info *c);
const struct ubifs_lprops *ubifs_fast_find_freeable(struct ubifs_info *c);
const struct ubifs_lprops *ubifs_fast_find_frdi_idx(struct ubifs_info *c);
int ubifs_calc_dark(const struct ubifs_info *c, int spc);

/* file.c */
int ubifs_fsync(struct file *file, loff_t start, loff_t end, int datasync);
int ubifs_setattr(struct dentry *dentry, struct iattr *attr);

/* dir.c */
struct inode *ubifs_new_inode(struct ubifs_info *c, const struct inode *dir,
			      umode_t mode);
int ubifs_getattr(struct vfsmount *mnt, struct dentry *dentry,
		  struct kstat *stat);

/* xattr.c */
int ubifs_setxattr(struct dentry *dentry, const char *name,
		   const void *value, size_t size, int flags);
ssize_t ubifs_getxattr(struct dentry *dentry, const char *name, void *buf,
		       size_t size);
ssize_t ubifs_listxattr(struct dentry *dentry, char *buffer, size_t size);
int ubifs_removexattr(struct dentry *dentry, const char *name);
int ubifs_init_security(struct inode *dentry, struct inode *inode,
			const struct qstr *qstr);

/* super.c */
struct inode *ubifs_iget(struct super_block *sb, unsigned long inum);
int ubifs_iput(struct inode *inode);

/* recovery.c */
int ubifs_recover_master_node(struct ubifs_info *c);
int ubifs_write_rcvrd_mst_node(struct ubifs_info *c);
struct ubifs_scan_leb *ubifs_recover_leb(struct ubifs_info *c, int lnum,
					 int offs, void *sbuf, int jhead);
struct ubifs_scan_leb *ubifs_recover_log_leb(struct ubifs_info *c, int lnum,
					     int offs, void *sbuf);
int ubifs_recover_inl_heads(struct ubifs_info *c, void *sbuf);
int ubifs_clean_lebs(struct ubifs_info *c, void *sbuf);
int ubifs_rcvry_gc_commit(struct ubifs_info *c);
int ubifs_recover_size_accum(struct ubifs_info *c, union ubifs_key *key,
			     int deletion, loff_t new_size);
int ubifs_recover_size(struct ubifs_info *c);
void ubifs_destroy_size_tree(struct ubifs_info *c);

/* ioctl.c */
long ubifs_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
void ubifs_set_inode_flags(struct inode *inode);
#ifdef CONFIG_COMPAT
long ubifs_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif

/* compressor.c */
int __init ubifs_compressors_init(void);
void ubifs_compressors_exit(void);
void ubifs_compress(const struct ubifs_info *c, const void *in_buf, int in_len,
		    void *out_buf, int *out_len, int *compr_type);
int ubifs_decompress(const struct ubifs_info *c, const void *buf, int len,
		     void *out, int *out_len, int compr_type);

#include "debug.h"
#include "misc.h"
#include "key.h"

#ifdef __UBOOT__
void ubifs_umount(struct ubifs_info *c);
#endif
#endif /* !__UBIFS_H__ */
