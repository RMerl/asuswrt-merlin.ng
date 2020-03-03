/*
 * Copyright (c) 2000-2006 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "xfs.h"
#include "xfs_shared.h"
#include "xfs_format.h"
#include "xfs_log_format.h"
#include "xfs_trans_resv.h"
#include "xfs_sb.h"
#include "xfs_mount.h"
#include "xfs_da_format.h"
#include "xfs_inode.h"
#include "xfs_btree.h"
#include "xfs_bmap.h"
#include "xfs_alloc.h"
#include "xfs_error.h"
#include "xfs_fsops.h"
#include "xfs_trans.h"
#include "xfs_buf_item.h"
#include "xfs_log.h"
#include "xfs_log_priv.h"
#include "xfs_da_btree.h"
#include "xfs_dir2.h"
#include "xfs_extfree_item.h"
#include "xfs_mru_cache.h"
#include "xfs_inode_item.h"
#include "xfs_icache.h"
#include "xfs_trace.h"
#include "xfs_icreate_item.h"
#include "xfs_filestream.h"
#include "xfs_quota.h"
#include "xfs_sysfs.h"

#include <linux/namei.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mount.h>
#include <linux/mempool.h>
#include <linux/writeback.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/parser.h>

static const struct super_operations xfs_super_operations;
static kmem_zone_t *xfs_ioend_zone;
mempool_t *xfs_ioend_pool;

static struct kset *xfs_kset;		/* top-level xfs sysfs dir */
#ifdef DEBUG
static struct xfs_kobj xfs_dbg_kobj;	/* global debug sysfs attrs */
#endif

#define MNTOPT_LOGBUFS	"logbufs"	/* number of XFS log buffers */
#define MNTOPT_LOGBSIZE	"logbsize"	/* size of XFS log buffers */
#define MNTOPT_LOGDEV	"logdev"	/* log device */
#define MNTOPT_RTDEV	"rtdev"		/* realtime I/O device */
#define MNTOPT_BIOSIZE	"biosize"	/* log2 of preferred buffered io size */
#define MNTOPT_WSYNC	"wsync"		/* safe-mode nfs compatible mount */
#define MNTOPT_NOALIGN	"noalign"	/* turn off stripe alignment */
#define MNTOPT_SWALLOC	"swalloc"	/* turn on stripe width allocation */
#define MNTOPT_SUNIT	"sunit"		/* data volume stripe unit */
#define MNTOPT_SWIDTH	"swidth"	/* data volume stripe width */
#define MNTOPT_NOUUID	"nouuid"	/* ignore filesystem UUID */
#define MNTOPT_MTPT	"mtpt"		/* filesystem mount point */
#define MNTOPT_GRPID	"grpid"		/* group-ID from parent directory */
#define MNTOPT_NOGRPID	"nogrpid"	/* group-ID from current process */
#define MNTOPT_BSDGROUPS    "bsdgroups"    /* group-ID from parent directory */
#define MNTOPT_SYSVGROUPS   "sysvgroups"   /* group-ID from current process */
#define MNTOPT_ALLOCSIZE    "allocsize"    /* preferred allocation size */
#define MNTOPT_NORECOVERY   "norecovery"   /* don't run XFS recovery */
#define MNTOPT_BARRIER	"barrier"	/* use writer barriers for log write and
					 * unwritten extent conversion */
#define MNTOPT_NOBARRIER "nobarrier"	/* .. disable */
#define MNTOPT_64BITINODE   "inode64"	/* inodes can be allocated anywhere */
#define MNTOPT_32BITINODE   "inode32"	/* inode allocation limited to
					 * XFS_MAXINUMBER_32 */
#define MNTOPT_IKEEP	"ikeep"		/* do not free empty inode clusters */
#define MNTOPT_NOIKEEP	"noikeep"	/* free empty inode clusters */
#define MNTOPT_LARGEIO	   "largeio"	/* report large I/O sizes in stat() */
#define MNTOPT_NOLARGEIO   "nolargeio"	/* do not report large I/O sizes
					 * in stat(). */
#define MNTOPT_ATTR2	"attr2"		/* do use attr2 attribute format */
#define MNTOPT_NOATTR2	"noattr2"	/* do not use attr2 attribute format */
#define MNTOPT_FILESTREAM  "filestreams" /* use filestreams allocator */
#define MNTOPT_QUOTA	"quota"		/* disk quotas (user) */
#define MNTOPT_NOQUOTA	"noquota"	/* no quotas */
#define MNTOPT_USRQUOTA	"usrquota"	/* user quota enabled */
#define MNTOPT_GRPQUOTA	"grpquota"	/* group quota enabled */
#define MNTOPT_PRJQUOTA	"prjquota"	/* project quota enabled */
#define MNTOPT_UQUOTA	"uquota"	/* user quota (IRIX variant) */
#define MNTOPT_GQUOTA	"gquota"	/* group quota (IRIX variant) */
#define MNTOPT_PQUOTA	"pquota"	/* project quota (IRIX variant) */
#define MNTOPT_UQUOTANOENF "uqnoenforce"/* user quota limit enforcement */
#define MNTOPT_GQUOTANOENF "gqnoenforce"/* group quota limit enforcement */
#define MNTOPT_PQUOTANOENF "pqnoenforce"/* project quota limit enforcement */
#define MNTOPT_QUOTANOENF  "qnoenforce"	/* same as uqnoenforce */
#define MNTOPT_DISCARD	   "discard"	/* Discard unused blocks */
#define MNTOPT_NODISCARD   "nodiscard"	/* Do not discard unused blocks */

/*
 * Table driven mount option parser.
 *
 * Currently only used for remount, but it will be used for mount
 * in the future, too.
 */
enum {
	Opt_barrier,
	Opt_nobarrier,
	Opt_inode64,
	Opt_inode32,
	Opt_err
};

static const match_table_t tokens = {
	{Opt_barrier, "barrier"},
	{Opt_nobarrier, "nobarrier"},
	{Opt_inode64, "inode64"},
	{Opt_inode32, "inode32"},
	{Opt_err, NULL}
};


STATIC unsigned long
suffix_kstrtoint(char *s, unsigned int base, int *res)
{
	int	last, shift_left_factor = 0, _res;
	char	*value = s;

	last = strlen(value) - 1;
	if (value[last] == 'K' || value[last] == 'k') {
		shift_left_factor = 10;
		value[last] = '\0';
	}
	if (value[last] == 'M' || value[last] == 'm') {
		shift_left_factor = 20;
		value[last] = '\0';
	}
	if (value[last] == 'G' || value[last] == 'g') {
		shift_left_factor = 30;
		value[last] = '\0';
	}

	if (kstrtoint(s, base, &_res))
		return -EINVAL;
	*res = _res << shift_left_factor;
	return 0;
}

/*
 * This function fills in xfs_mount_t fields based on mount args.
 * Note: the superblock has _not_ yet been read in.
 *
 * Note that this function leaks the various device name allocations on
 * failure.  The caller takes care of them.
 */
STATIC int
xfs_parseargs(
	struct xfs_mount	*mp,
	char			*options)
{
	struct super_block	*sb = mp->m_super;
	char			*this_char, *value;
	int			dsunit = 0;
	int			dswidth = 0;
	int			iosize = 0;
	__uint8_t		iosizelog = 0;

	/*
	 * set up the mount name first so all the errors will refer to the
	 * correct device.
	 */
	mp->m_fsname = kstrndup(sb->s_id, MAXNAMELEN, GFP_KERNEL);
	if (!mp->m_fsname)
		return -ENOMEM;
	mp->m_fsname_len = strlen(mp->m_fsname) + 1;

	/*
	 * Copy binary VFS mount flags we are interested in.
	 */
	if (sb->s_flags & MS_RDONLY)
		mp->m_flags |= XFS_MOUNT_RDONLY;
	if (sb->s_flags & MS_DIRSYNC)
		mp->m_flags |= XFS_MOUNT_DIRSYNC;
	if (sb->s_flags & MS_SYNCHRONOUS)
		mp->m_flags |= XFS_MOUNT_WSYNC;

	/*
	 * Set some default flags that could be cleared by the mount option
	 * parsing.
	 */
	mp->m_flags |= XFS_MOUNT_BARRIER;
	mp->m_flags |= XFS_MOUNT_COMPAT_IOSIZE;

	/*
	 * These can be overridden by the mount option parsing.
	 */
	mp->m_logbufs = -1;
	mp->m_logbsize = -1;

	if (!options)
		goto done;

	while ((this_char = strsep(&options, ",")) != NULL) {
		if (!*this_char)
			continue;
		if ((value = strchr(this_char, '=')) != NULL)
			*value++ = 0;

		if (!strcmp(this_char, MNTOPT_LOGBUFS)) {
			if (!value || !*value) {
				xfs_warn(mp, "%s option requires an argument",
					this_char);
				return -EINVAL;
			}
			if (kstrtoint(value, 10, &mp->m_logbufs))
				return -EINVAL;
		} else if (!strcmp(this_char, MNTOPT_LOGBSIZE)) {
			if (!value || !*value) {
				xfs_warn(mp, "%s option requires an argument",
					this_char);
				return -EINVAL;
			}
			if (suffix_kstrtoint(value, 10, &mp->m_logbsize))
				return -EINVAL;
		} else if (!strcmp(this_char, MNTOPT_LOGDEV)) {
			if (!value || !*value) {
				xfs_warn(mp, "%s option requires an argument",
					this_char);
				return -EINVAL;
			}
			mp->m_logname = kstrndup(value, MAXNAMELEN, GFP_KERNEL);
			if (!mp->m_logname)
				return -ENOMEM;
		} else if (!strcmp(this_char, MNTOPT_MTPT)) {
			xfs_warn(mp, "%s option not allowed on this system",
				this_char);
			return -EINVAL;
		} else if (!strcmp(this_char, MNTOPT_RTDEV)) {
			if (!value || !*value) {
				xfs_warn(mp, "%s option requires an argument",
					this_char);
				return -EINVAL;
			}
			mp->m_rtname = kstrndup(value, MAXNAMELEN, GFP_KERNEL);
			if (!mp->m_rtname)
				return -ENOMEM;
		} else if (!strcmp(this_char, MNTOPT_BIOSIZE)) {
			if (!value || !*value) {
				xfs_warn(mp, "%s option requires an argument",
					this_char);
				return -EINVAL;
			}
			if (kstrtoint(value, 10, &iosize))
				return -EINVAL;
			iosizelog = ffs(iosize) - 1;
		} else if (!strcmp(this_char, MNTOPT_ALLOCSIZE)) {
			if (!value || !*value) {
				xfs_warn(mp, "%s option requires an argument",
					this_char);
				return -EINVAL;
			}
			if (suffix_kstrtoint(value, 10, &iosize))
				return -EINVAL;
			iosizelog = ffs(iosize) - 1;
		} else if (!strcmp(this_char, MNTOPT_GRPID) ||
			   !strcmp(this_char, MNTOPT_BSDGROUPS)) {
			mp->m_flags |= XFS_MOUNT_GRPID;
		} else if (!strcmp(this_char, MNTOPT_NOGRPID) ||
			   !strcmp(this_char, MNTOPT_SYSVGROUPS)) {
			mp->m_flags &= ~XFS_MOUNT_GRPID;
		} else if (!strcmp(this_char, MNTOPT_WSYNC)) {
			mp->m_flags |= XFS_MOUNT_WSYNC;
		} else if (!strcmp(this_char, MNTOPT_NORECOVERY)) {
			mp->m_flags |= XFS_MOUNT_NORECOVERY;
		} else if (!strcmp(this_char, MNTOPT_NOALIGN)) {
			mp->m_flags |= XFS_MOUNT_NOALIGN;
		} else if (!strcmp(this_char, MNTOPT_SWALLOC)) {
			mp->m_flags |= XFS_MOUNT_SWALLOC;
		} else if (!strcmp(this_char, MNTOPT_SUNIT)) {
			if (!value || !*value) {
				xfs_warn(mp, "%s option requires an argument",
					this_char);
				return -EINVAL;
			}
			if (kstrtoint(value, 10, &dsunit))
				return -EINVAL;
		} else if (!strcmp(this_char, MNTOPT_SWIDTH)) {
			if (!value || !*value) {
				xfs_warn(mp, "%s option requires an argument",
					this_char);
				return -EINVAL;
			}
			if (kstrtoint(value, 10, &dswidth))
				return -EINVAL;
		} else if (!strcmp(this_char, MNTOPT_32BITINODE)) {
			mp->m_flags |= XFS_MOUNT_SMALL_INUMS;
		} else if (!strcmp(this_char, MNTOPT_64BITINODE)) {
			mp->m_flags &= ~XFS_MOUNT_SMALL_INUMS;
		} else if (!strcmp(this_char, MNTOPT_NOUUID)) {
			mp->m_flags |= XFS_MOUNT_NOUUID;
		} else if (!strcmp(this_char, MNTOPT_BARRIER)) {
			mp->m_flags |= XFS_MOUNT_BARRIER;
		} else if (!strcmp(this_char, MNTOPT_NOBARRIER)) {
			mp->m_flags &= ~XFS_MOUNT_BARRIER;
		} else if (!strcmp(this_char, MNTOPT_IKEEP)) {
			mp->m_flags |= XFS_MOUNT_IKEEP;
		} else if (!strcmp(this_char, MNTOPT_NOIKEEP)) {
			mp->m_flags &= ~XFS_MOUNT_IKEEP;
		} else if (!strcmp(this_char, MNTOPT_LARGEIO)) {
			mp->m_flags &= ~XFS_MOUNT_COMPAT_IOSIZE;
		} else if (!strcmp(this_char, MNTOPT_NOLARGEIO)) {
			mp->m_flags |= XFS_MOUNT_COMPAT_IOSIZE;
		} else if (!strcmp(this_char, MNTOPT_ATTR2)) {
			mp->m_flags |= XFS_MOUNT_ATTR2;
		} else if (!strcmp(this_char, MNTOPT_NOATTR2)) {
			mp->m_flags &= ~XFS_MOUNT_ATTR2;
			mp->m_flags |= XFS_MOUNT_NOATTR2;
		} else if (!strcmp(this_char, MNTOPT_FILESTREAM)) {
			mp->m_flags |= XFS_MOUNT_FILESTREAMS;
		} else if (!strcmp(this_char, MNTOPT_NOQUOTA)) {
			mp->m_qflags &= ~XFS_ALL_QUOTA_ACCT;
			mp->m_qflags &= ~XFS_ALL_QUOTA_ENFD;
			mp->m_qflags &= ~XFS_ALL_QUOTA_ACTIVE;
		} else if (!strcmp(this_char, MNTOPT_QUOTA) ||
			   !strcmp(this_char, MNTOPT_UQUOTA) ||
			   !strcmp(this_char, MNTOPT_USRQUOTA)) {
			mp->m_qflags |= (XFS_UQUOTA_ACCT | XFS_UQUOTA_ACTIVE |
					 XFS_UQUOTA_ENFD);
		} else if (!strcmp(this_char, MNTOPT_QUOTANOENF) ||
			   !strcmp(this_char, MNTOPT_UQUOTANOENF)) {
			mp->m_qflags |= (XFS_UQUOTA_ACCT | XFS_UQUOTA_ACTIVE);
			mp->m_qflags &= ~XFS_UQUOTA_ENFD;
		} else if (!strcmp(this_char, MNTOPT_PQUOTA) ||
			   !strcmp(this_char, MNTOPT_PRJQUOTA)) {
			mp->m_qflags |= (XFS_PQUOTA_ACCT | XFS_PQUOTA_ACTIVE |
					 XFS_PQUOTA_ENFD);
		} else if (!strcmp(this_char, MNTOPT_PQUOTANOENF)) {
			mp->m_qflags |= (XFS_PQUOTA_ACCT | XFS_PQUOTA_ACTIVE);
			mp->m_qflags &= ~XFS_PQUOTA_ENFD;
		} else if (!strcmp(this_char, MNTOPT_GQUOTA) ||
			   !strcmp(this_char, MNTOPT_GRPQUOTA)) {
			mp->m_qflags |= (XFS_GQUOTA_ACCT | XFS_GQUOTA_ACTIVE |
					 XFS_GQUOTA_ENFD);
		} else if (!strcmp(this_char, MNTOPT_GQUOTANOENF)) {
			mp->m_qflags |= (XFS_GQUOTA_ACCT | XFS_GQUOTA_ACTIVE);
			mp->m_qflags &= ~XFS_GQUOTA_ENFD;
		} else if (!strcmp(this_char, MNTOPT_DISCARD)) {
			mp->m_flags |= XFS_MOUNT_DISCARD;
		} else if (!strcmp(this_char, MNTOPT_NODISCARD)) {
			mp->m_flags &= ~XFS_MOUNT_DISCARD;
		} else {
			xfs_warn(mp, "unknown mount option [%s].", this_char);
			return -EINVAL;
		}
	}

	/*
	 * no recovery flag requires a read-only mount
	 */
	if ((mp->m_flags & XFS_MOUNT_NORECOVERY) &&
	    !(mp->m_flags & XFS_MOUNT_RDONLY)) {
		xfs_warn(mp, "no-recovery mounts must be read-only.");
		return -EINVAL;
	}

	if ((mp->m_flags & XFS_MOUNT_NOALIGN) && (dsunit || dswidth)) {
		xfs_warn(mp,
	"sunit and swidth options incompatible with the noalign option");
		return -EINVAL;
	}

#ifndef CONFIG_XFS_QUOTA
	if (XFS_IS_QUOTA_RUNNING(mp)) {
		xfs_warn(mp, "quota support not available in this kernel.");
		return -EINVAL;
	}
#endif

	if ((dsunit && !dswidth) || (!dsunit && dswidth)) {
		xfs_warn(mp, "sunit and swidth must be specified together");
		return -EINVAL;
	}

	if (dsunit && (dswidth % dsunit != 0)) {
		xfs_warn(mp,
	"stripe width (%d) must be a multiple of the stripe unit (%d)",
			dswidth, dsunit);
		return -EINVAL;
	}

done:
	if (dsunit && !(mp->m_flags & XFS_MOUNT_NOALIGN)) {
		/*
		 * At this point the superblock has not been read
		 * in, therefore we do not know the block size.
		 * Before the mount call ends we will convert
		 * these to FSBs.
		 */
		mp->m_dalign = dsunit;
		mp->m_swidth = dswidth;
	}

	if (mp->m_logbufs != -1 &&
	    mp->m_logbufs != 0 &&
	    (mp->m_logbufs < XLOG_MIN_ICLOGS ||
	     mp->m_logbufs > XLOG_MAX_ICLOGS)) {
		xfs_warn(mp, "invalid logbufs value: %d [not %d-%d]",
			mp->m_logbufs, XLOG_MIN_ICLOGS, XLOG_MAX_ICLOGS);
		return -EINVAL;
	}
	if (mp->m_logbsize != -1 &&
	    mp->m_logbsize !=  0 &&
	    (mp->m_logbsize < XLOG_MIN_RECORD_BSIZE ||
	     mp->m_logbsize > XLOG_MAX_RECORD_BSIZE ||
	     !is_power_of_2(mp->m_logbsize))) {
		xfs_warn(mp,
			"invalid logbufsize: %d [not 16k,32k,64k,128k or 256k]",
			mp->m_logbsize);
		return -EINVAL;
	}

	if (iosizelog) {
		if (iosizelog > XFS_MAX_IO_LOG ||
		    iosizelog < XFS_MIN_IO_LOG) {
			xfs_warn(mp, "invalid log iosize: %d [not %d-%d]",
				iosizelog, XFS_MIN_IO_LOG,
				XFS_MAX_IO_LOG);
			return -EINVAL;
		}

		mp->m_flags |= XFS_MOUNT_DFLT_IOSIZE;
		mp->m_readio_log = iosizelog;
		mp->m_writeio_log = iosizelog;
	}

	return 0;
}

struct proc_xfs_info {
	int	flag;
	char	*str;
};

STATIC int
xfs_showargs(
	struct xfs_mount	*mp,
	struct seq_file		*m)
{
	static struct proc_xfs_info xfs_info_set[] = {
		/* the few simple ones we can get from the mount struct */
		{ XFS_MOUNT_IKEEP,		"," MNTOPT_IKEEP },
		{ XFS_MOUNT_WSYNC,		"," MNTOPT_WSYNC },
		{ XFS_MOUNT_NOALIGN,		"," MNTOPT_NOALIGN },
		{ XFS_MOUNT_SWALLOC,		"," MNTOPT_SWALLOC },
		{ XFS_MOUNT_NOUUID,		"," MNTOPT_NOUUID },
		{ XFS_MOUNT_NORECOVERY,		"," MNTOPT_NORECOVERY },
		{ XFS_MOUNT_ATTR2,		"," MNTOPT_ATTR2 },
		{ XFS_MOUNT_FILESTREAMS,	"," MNTOPT_FILESTREAM },
		{ XFS_MOUNT_GRPID,		"," MNTOPT_GRPID },
		{ XFS_MOUNT_DISCARD,		"," MNTOPT_DISCARD },
		{ XFS_MOUNT_SMALL_INUMS,	"," MNTOPT_32BITINODE },
		{ 0, NULL }
	};
	static struct proc_xfs_info xfs_info_unset[] = {
		/* the few simple ones we can get from the mount struct */
		{ XFS_MOUNT_COMPAT_IOSIZE,	"," MNTOPT_LARGEIO },
		{ XFS_MOUNT_BARRIER,		"," MNTOPT_NOBARRIER },
		{ XFS_MOUNT_SMALL_INUMS,	"," MNTOPT_64BITINODE },
		{ 0, NULL }
	};
	struct proc_xfs_info	*xfs_infop;

	for (xfs_infop = xfs_info_set; xfs_infop->flag; xfs_infop++) {
		if (mp->m_flags & xfs_infop->flag)
			seq_puts(m, xfs_infop->str);
	}
	for (xfs_infop = xfs_info_unset; xfs_infop->flag; xfs_infop++) {
		if (!(mp->m_flags & xfs_infop->flag))
			seq_puts(m, xfs_infop->str);
	}

	if (mp->m_flags & XFS_MOUNT_DFLT_IOSIZE)
		seq_printf(m, "," MNTOPT_ALLOCSIZE "=%dk",
				(int)(1 << mp->m_writeio_log) >> 10);

	if (mp->m_logbufs > 0)
		seq_printf(m, "," MNTOPT_LOGBUFS "=%d", mp->m_logbufs);
	if (mp->m_logbsize > 0)
		seq_printf(m, "," MNTOPT_LOGBSIZE "=%dk", mp->m_logbsize >> 10);

	if (mp->m_logname)
		seq_show_option(m, MNTOPT_LOGDEV, mp->m_logname);
	if (mp->m_rtname)
		seq_show_option(m, MNTOPT_RTDEV, mp->m_rtname);

	if (mp->m_dalign > 0)
		seq_printf(m, "," MNTOPT_SUNIT "=%d",
				(int)XFS_FSB_TO_BB(mp, mp->m_dalign));
	if (mp->m_swidth > 0)
		seq_printf(m, "," MNTOPT_SWIDTH "=%d",
				(int)XFS_FSB_TO_BB(mp, mp->m_swidth));

	if (mp->m_qflags & (XFS_UQUOTA_ACCT|XFS_UQUOTA_ENFD))
		seq_puts(m, "," MNTOPT_USRQUOTA);
	else if (mp->m_qflags & XFS_UQUOTA_ACCT)
		seq_puts(m, "," MNTOPT_UQUOTANOENF);

	if (mp->m_qflags & XFS_PQUOTA_ACCT) {
		if (mp->m_qflags & XFS_PQUOTA_ENFD)
			seq_puts(m, "," MNTOPT_PRJQUOTA);
		else
			seq_puts(m, "," MNTOPT_PQUOTANOENF);
	}
	if (mp->m_qflags & XFS_GQUOTA_ACCT) {
		if (mp->m_qflags & XFS_GQUOTA_ENFD)
			seq_puts(m, "," MNTOPT_GRPQUOTA);
		else
			seq_puts(m, "," MNTOPT_GQUOTANOENF);
	}

	if (!(mp->m_qflags & XFS_ALL_QUOTA_ACCT))
		seq_puts(m, "," MNTOPT_NOQUOTA);

	return 0;
}
__uint64_t
xfs_max_file_offset(
	unsigned int		blockshift)
{
	unsigned int		pagefactor = 1;
	unsigned int		bitshift = BITS_PER_LONG - 1;

	/* Figure out maximum filesize, on Linux this can depend on
	 * the filesystem blocksize (on 32 bit platforms).
	 * __block_write_begin does this in an [unsigned] long...
	 *      page->index << (PAGE_CACHE_SHIFT - bbits)
	 * So, for page sized blocks (4K on 32 bit platforms),
	 * this wraps at around 8Tb (hence MAX_LFS_FILESIZE which is
	 *      (((u64)PAGE_CACHE_SIZE << (BITS_PER_LONG-1))-1)
	 * but for smaller blocksizes it is less (bbits = log2 bsize).
	 * Note1: get_block_t takes a long (implicit cast from above)
	 * Note2: The Large Block Device (LBD and HAVE_SECTOR_T) patch
	 * can optionally convert the [unsigned] long from above into
	 * an [unsigned] long long.
	 */

#if BITS_PER_LONG == 32
# if defined(CONFIG_LBDAF)
	ASSERT(sizeof(sector_t) == 8);
	pagefactor = PAGE_CACHE_SIZE;
	bitshift = BITS_PER_LONG;
# else
	pagefactor = PAGE_CACHE_SIZE >> (PAGE_CACHE_SHIFT - blockshift);
# endif
#endif

	return (((__uint64_t)pagefactor) << bitshift) - 1;
}

/*
 * xfs_set_inode32() and xfs_set_inode64() are passed an agcount
 * because in the growfs case, mp->m_sb.sb_agcount is not updated
 * yet to the potentially higher ag count.
 */
xfs_agnumber_t
xfs_set_inode32(struct xfs_mount *mp, xfs_agnumber_t agcount)
{
	xfs_agnumber_t	index = 0;
	xfs_agnumber_t	maxagi = 0;
	xfs_sb_t	*sbp = &mp->m_sb;
	xfs_agnumber_t	max_metadata;
	xfs_agino_t	agino;
	xfs_ino_t	ino;
	xfs_perag_t	*pag;

	/* Calculate how much should be reserved for inodes to meet
	 * the max inode percentage.
	 */
	if (mp->m_maxicount) {
		__uint64_t	icount;

		icount = sbp->sb_dblocks * sbp->sb_imax_pct;
		do_div(icount, 100);
		icount += sbp->sb_agblocks - 1;
		do_div(icount, sbp->sb_agblocks);
		max_metadata = icount;
	} else {
		max_metadata = agcount;
	}

	agino =	XFS_OFFBNO_TO_AGINO(mp, sbp->sb_agblocks - 1, 0);

	for (index = 0; index < agcount; index++) {
		ino = XFS_AGINO_TO_INO(mp, index, agino);

		if (ino > XFS_MAXINUMBER_32) {
			pag = xfs_perag_get(mp, index);
			pag->pagi_inodeok = 0;
			pag->pagf_metadata = 0;
			xfs_perag_put(pag);
			continue;
		}

		pag = xfs_perag_get(mp, index);
		pag->pagi_inodeok = 1;
		maxagi++;
		if (index < max_metadata)
			pag->pagf_metadata = 1;
		xfs_perag_put(pag);
	}
	mp->m_flags |= (XFS_MOUNT_32BITINODES |
			XFS_MOUNT_SMALL_INUMS);

	return maxagi;
}

xfs_agnumber_t
xfs_set_inode64(struct xfs_mount *mp, xfs_agnumber_t agcount)
{
	xfs_agnumber_t index = 0;

	for (index = 0; index < agcount; index++) {
		struct xfs_perag	*pag;

		pag = xfs_perag_get(mp, index);
		pag->pagi_inodeok = 1;
		pag->pagf_metadata = 0;
		xfs_perag_put(pag);
	}

	/* There is no need for lock protection on m_flags,
	 * the rw_semaphore of the VFS superblock is locked
	 * during mount/umount/remount operations, so this is
	 * enough to avoid concurency on the m_flags field
	 */
	mp->m_flags &= ~(XFS_MOUNT_32BITINODES |
			 XFS_MOUNT_SMALL_INUMS);
	return index;
}

STATIC int
xfs_blkdev_get(
	xfs_mount_t		*mp,
	const char		*name,
	struct block_device	**bdevp)
{
	int			error = 0;

	*bdevp = blkdev_get_by_path(name, FMODE_READ|FMODE_WRITE|FMODE_EXCL,
				    mp);
	if (IS_ERR(*bdevp)) {
		error = PTR_ERR(*bdevp);
		xfs_warn(mp, "Invalid device [%s], error=%d", name, error);
	}

	return error;
}

STATIC void
xfs_blkdev_put(
	struct block_device	*bdev)
{
	if (bdev)
		blkdev_put(bdev, FMODE_READ|FMODE_WRITE|FMODE_EXCL);
}

void
xfs_blkdev_issue_flush(
	xfs_buftarg_t		*buftarg)
{
	blkdev_issue_flush(buftarg->bt_bdev, GFP_NOFS, NULL);
}

STATIC void
xfs_close_devices(
	struct xfs_mount	*mp)
{
	if (mp->m_logdev_targp && mp->m_logdev_targp != mp->m_ddev_targp) {
		struct block_device *logdev = mp->m_logdev_targp->bt_bdev;
		xfs_free_buftarg(mp, mp->m_logdev_targp);
		xfs_blkdev_put(logdev);
	}
	if (mp->m_rtdev_targp) {
		struct block_device *rtdev = mp->m_rtdev_targp->bt_bdev;
		xfs_free_buftarg(mp, mp->m_rtdev_targp);
		xfs_blkdev_put(rtdev);
	}
	xfs_free_buftarg(mp, mp->m_ddev_targp);
}

/*
 * The file system configurations are:
 *	(1) device (partition) with data and internal log
 *	(2) logical volume with data and log subvolumes.
 *	(3) logical volume with data, log, and realtime subvolumes.
 *
 * We only have to handle opening the log and realtime volumes here if
 * they are present.  The data subvolume has already been opened by
 * get_sb_bdev() and is stored in sb->s_bdev.
 */
STATIC int
xfs_open_devices(
	struct xfs_mount	*mp)
{
	struct block_device	*ddev = mp->m_super->s_bdev;
	struct block_device	*logdev = NULL, *rtdev = NULL;
	int			error;

	/*
	 * Open real time and log devices - order is important.
	 */
	if (mp->m_logname) {
		error = xfs_blkdev_get(mp, mp->m_logname, &logdev);
		if (error)
			goto out;
	}

	if (mp->m_rtname) {
		error = xfs_blkdev_get(mp, mp->m_rtname, &rtdev);
		if (error)
			goto out_close_logdev;

		if (rtdev == ddev || rtdev == logdev) {
			xfs_warn(mp,
	"Cannot mount filesystem with identical rtdev and ddev/logdev.");
			error = -EINVAL;
			goto out_close_rtdev;
		}
	}

	/*
	 * Setup xfs_mount buffer target pointers
	 */
	error = -ENOMEM;
	mp->m_ddev_targp = xfs_alloc_buftarg(mp, ddev);
	if (!mp->m_ddev_targp)
		goto out_close_rtdev;

	if (rtdev) {
		mp->m_rtdev_targp = xfs_alloc_buftarg(mp, rtdev);
		if (!mp->m_rtdev_targp)
			goto out_free_ddev_targ;
	}

	if (logdev && logdev != ddev) {
		mp->m_logdev_targp = xfs_alloc_buftarg(mp, logdev);
		if (!mp->m_logdev_targp)
			goto out_free_rtdev_targ;
	} else {
		mp->m_logdev_targp = mp->m_ddev_targp;
	}

	return 0;

 out_free_rtdev_targ:
	if (mp->m_rtdev_targp)
		xfs_free_buftarg(mp, mp->m_rtdev_targp);
 out_free_ddev_targ:
	xfs_free_buftarg(mp, mp->m_ddev_targp);
 out_close_rtdev:
	xfs_blkdev_put(rtdev);
 out_close_logdev:
	if (logdev && logdev != ddev)
		xfs_blkdev_put(logdev);
 out:
	return error;
}

/*
 * Setup xfs_mount buffer target pointers based on superblock
 */
STATIC int
xfs_setup_devices(
	struct xfs_mount	*mp)
{
	int			error;

	error = xfs_setsize_buftarg(mp->m_ddev_targp, mp->m_sb.sb_sectsize);
	if (error)
		return error;

	if (mp->m_logdev_targp && mp->m_logdev_targp != mp->m_ddev_targp) {
		unsigned int	log_sector_size = BBSIZE;

		if (xfs_sb_version_hassector(&mp->m_sb))
			log_sector_size = mp->m_sb.sb_logsectsize;
		error = xfs_setsize_buftarg(mp->m_logdev_targp,
					    log_sector_size);
		if (error)
			return error;
	}
	if (mp->m_rtdev_targp) {
		error = xfs_setsize_buftarg(mp->m_rtdev_targp,
					    mp->m_sb.sb_sectsize);
		if (error)
			return error;
	}

	return 0;
}

STATIC int
xfs_init_mount_workqueues(
	struct xfs_mount	*mp)
{
	mp->m_buf_workqueue = alloc_workqueue("xfs-buf/%s",
			WQ_MEM_RECLAIM|WQ_FREEZABLE, 1, mp->m_fsname);
	if (!mp->m_buf_workqueue)
		goto out;

	mp->m_data_workqueue = alloc_workqueue("xfs-data/%s",
			WQ_MEM_RECLAIM|WQ_FREEZABLE, 0, mp->m_fsname);
	if (!mp->m_data_workqueue)
		goto out_destroy_buf;

	mp->m_unwritten_workqueue = alloc_workqueue("xfs-conv/%s",
			WQ_MEM_RECLAIM|WQ_FREEZABLE, 0, mp->m_fsname);
	if (!mp->m_unwritten_workqueue)
		goto out_destroy_data_iodone_queue;

	mp->m_cil_workqueue = alloc_workqueue("xfs-cil/%s",
			WQ_MEM_RECLAIM|WQ_FREEZABLE, 0, mp->m_fsname);
	if (!mp->m_cil_workqueue)
		goto out_destroy_unwritten;

	mp->m_reclaim_workqueue = alloc_workqueue("xfs-reclaim/%s",
			WQ_FREEZABLE, 0, mp->m_fsname);
	if (!mp->m_reclaim_workqueue)
		goto out_destroy_cil;

	mp->m_log_workqueue = alloc_workqueue("xfs-log/%s",
			WQ_FREEZABLE|WQ_HIGHPRI, 0, mp->m_fsname);
	if (!mp->m_log_workqueue)
		goto out_destroy_reclaim;

	mp->m_eofblocks_workqueue = alloc_workqueue("xfs-eofblocks/%s",
			WQ_FREEZABLE, 0, mp->m_fsname);
	if (!mp->m_eofblocks_workqueue)
		goto out_destroy_log;

	mp->m_sync_workqueue = alloc_workqueue("xfs-sync/%s", WQ_FREEZABLE, 0,
					       mp->m_fsname);
	if (!mp->m_sync_workqueue)
		goto out_destroy_eofb;

	return 0;

out_destroy_eofb:
	destroy_workqueue(mp->m_eofblocks_workqueue);
out_destroy_log:
	destroy_workqueue(mp->m_log_workqueue);
out_destroy_reclaim:
	destroy_workqueue(mp->m_reclaim_workqueue);
out_destroy_cil:
	destroy_workqueue(mp->m_cil_workqueue);
out_destroy_unwritten:
	destroy_workqueue(mp->m_unwritten_workqueue);
out_destroy_data_iodone_queue:
	destroy_workqueue(mp->m_data_workqueue);
out_destroy_buf:
	destroy_workqueue(mp->m_buf_workqueue);
out:
	return -ENOMEM;
}

STATIC void
xfs_destroy_mount_workqueues(
	struct xfs_mount	*mp)
{
	destroy_workqueue(mp->m_sync_workqueue);
	destroy_workqueue(mp->m_eofblocks_workqueue);
	destroy_workqueue(mp->m_log_workqueue);
	destroy_workqueue(mp->m_reclaim_workqueue);
	destroy_workqueue(mp->m_cil_workqueue);
	destroy_workqueue(mp->m_data_workqueue);
	destroy_workqueue(mp->m_unwritten_workqueue);
	destroy_workqueue(mp->m_buf_workqueue);
}

/*
 * Flush all dirty data to disk. Must not be called while holding an XFS_ILOCK
 * or a page lock. We use sync_inodes_sb() here to ensure we block while waiting
 * for IO to complete so that we effectively throttle multiple callers to the
 * rate at which IO is completing.
 */
void
xfs_flush_inodes(
	struct xfs_mount	*mp)
{
	struct super_block	*sb = mp->m_super;

	if (down_read_trylock(&sb->s_umount)) {
		sync_inodes_sb(sb);
		up_read(&sb->s_umount);
	}
}

/* Catch misguided souls that try to use this interface on XFS */
STATIC struct inode *
xfs_fs_alloc_inode(
	struct super_block	*sb)
{
	BUG();
	return NULL;
}

/*
 * Now that the generic code is guaranteed not to be accessing
 * the linux inode, we can reclaim the inode.
 */
STATIC void
xfs_fs_destroy_inode(
	struct inode		*inode)
{
	struct xfs_inode	*ip = XFS_I(inode);

	trace_xfs_destroy_inode(ip);

	XFS_STATS_INC(vn_reclaim);

	ASSERT(XFS_FORCED_SHUTDOWN(ip->i_mount) || ip->i_delayed_blks == 0);

	/*
	 * We should never get here with one of the reclaim flags already set.
	 */
	ASSERT_ALWAYS(!xfs_iflags_test(ip, XFS_IRECLAIMABLE));
	ASSERT_ALWAYS(!xfs_iflags_test(ip, XFS_IRECLAIM));

	/*
	 * We always use background reclaim here because even if the
	 * inode is clean, it still may be under IO and hence we have
	 * to take the flush lock. The background reclaim path handles
	 * this more efficiently than we can here, so simply let background
	 * reclaim tear down all inodes.
	 */
	xfs_inode_set_reclaim_tag(ip);
}

/*
 * Slab object creation initialisation for the XFS inode.
 * This covers only the idempotent fields in the XFS inode;
 * all other fields need to be initialised on allocation
 * from the slab. This avoids the need to repeatedly initialise
 * fields in the xfs inode that left in the initialise state
 * when freeing the inode.
 */
STATIC void
xfs_fs_inode_init_once(
	void			*inode)
{
	struct xfs_inode	*ip = inode;

	memset(ip, 0, sizeof(struct xfs_inode));

	/* vfs inode */
	inode_init_once(VFS_I(ip));

	/* xfs inode */
	atomic_set(&ip->i_pincount, 0);
	spin_lock_init(&ip->i_flags_lock);

	mrlock_init(&ip->i_mmaplock, MRLOCK_ALLOW_EQUAL_PRI|MRLOCK_BARRIER,
		     "xfsino", ip->i_ino);
	mrlock_init(&ip->i_lock, MRLOCK_ALLOW_EQUAL_PRI|MRLOCK_BARRIER,
		     "xfsino", ip->i_ino);
}

STATIC void
xfs_fs_evict_inode(
	struct inode		*inode)
{
	xfs_inode_t		*ip = XFS_I(inode);

	ASSERT(!rwsem_is_locked(&ip->i_iolock.mr_lock));

	trace_xfs_evict_inode(ip);

	truncate_inode_pages_final(&inode->i_data);
	clear_inode(inode);
	XFS_STATS_INC(vn_rele);
	XFS_STATS_INC(vn_remove);

	xfs_inactive(ip);
}

/*
 * We do an unlocked check for XFS_IDONTCACHE here because we are already
 * serialised against cache hits here via the inode->i_lock and igrab() in
 * xfs_iget_cache_hit(). Hence a lookup that might clear this flag will not be
 * racing with us, and it avoids needing to grab a spinlock here for every inode
 * we drop the final reference on.
 */
STATIC int
xfs_fs_drop_inode(
	struct inode		*inode)
{
	struct xfs_inode	*ip = XFS_I(inode);

	return generic_drop_inode(inode) || (ip->i_flags & XFS_IDONTCACHE);
}

STATIC void
xfs_free_fsname(
	struct xfs_mount	*mp)
{
	kfree(mp->m_fsname);
	kfree(mp->m_rtname);
	kfree(mp->m_logname);
}

STATIC int
xfs_fs_sync_fs(
	struct super_block	*sb,
	int			wait)
{
	struct xfs_mount	*mp = XFS_M(sb);

	/*
	 * Doing anything during the async pass would be counterproductive.
	 */
	if (!wait)
		return 0;

	xfs_log_force(mp, XFS_LOG_SYNC);
	if (laptop_mode) {
		/*
		 * The disk must be active because we're syncing.
		 * We schedule log work now (now that the disk is
		 * active) instead of later (when it might not be).
		 */
		flush_delayed_work(&mp->m_log->l_work);
	}

	return 0;
}

STATIC int
xfs_fs_statfs(
	struct dentry		*dentry,
	struct kstatfs		*statp)
{
	struct xfs_mount	*mp = XFS_M(dentry->d_sb);
	xfs_sb_t		*sbp = &mp->m_sb;
	struct xfs_inode	*ip = XFS_I(d_inode(dentry));
	__uint64_t		fakeinos, id;
	__uint64_t		icount;
	__uint64_t		ifree;
	__uint64_t		fdblocks;
	xfs_extlen_t		lsize;
	__int64_t		ffree;

	statp->f_type = XFS_SB_MAGIC;
	statp->f_namelen = MAXNAMELEN - 1;

	id = huge_encode_dev(mp->m_ddev_targp->bt_dev);
	statp->f_fsid.val[0] = (u32)id;
	statp->f_fsid.val[1] = (u32)(id >> 32);

	icount = percpu_counter_sum(&mp->m_icount);
	ifree = percpu_counter_sum(&mp->m_ifree);
	fdblocks = percpu_counter_sum(&mp->m_fdblocks);

	spin_lock(&mp->m_sb_lock);
	statp->f_bsize = sbp->sb_blocksize;
	lsize = sbp->sb_logstart ? sbp->sb_logblocks : 0;
	statp->f_blocks = sbp->sb_dblocks - lsize;
	spin_unlock(&mp->m_sb_lock);

	statp->f_bfree = fdblocks - XFS_ALLOC_SET_ASIDE(mp);
	statp->f_bavail = statp->f_bfree;

	fakeinos = statp->f_bfree << sbp->sb_inopblog;
	statp->f_files = MIN(icount + fakeinos, (__uint64_t)XFS_MAXINUMBER);
	if (mp->m_maxicount)
		statp->f_files = min_t(typeof(statp->f_files),
					statp->f_files,
					mp->m_maxicount);

	/* If sb_icount overshot maxicount, report actual allocation */
	statp->f_files = max_t(typeof(statp->f_files),
					statp->f_files,
					sbp->sb_icount);

	/* make sure statp->f_ffree does not underflow */
	ffree = statp->f_files - (icount - ifree);
	statp->f_ffree = max_t(__int64_t, ffree, 0);


	if ((ip->i_d.di_flags & XFS_DIFLAG_PROJINHERIT) &&
	    ((mp->m_qflags & (XFS_PQUOTA_ACCT|XFS_PQUOTA_ENFD))) ==
			      (XFS_PQUOTA_ACCT|XFS_PQUOTA_ENFD))
		xfs_qm_statvfs(ip, statp);
	return 0;
}

STATIC void
xfs_save_resvblks(struct xfs_mount *mp)
{
	__uint64_t resblks = 0;

	mp->m_resblks_save = mp->m_resblks;
	xfs_reserve_blocks(mp, &resblks, NULL);
}

STATIC void
xfs_restore_resvblks(struct xfs_mount *mp)
{
	__uint64_t resblks;

	if (mp->m_resblks_save) {
		resblks = mp->m_resblks_save;
		mp->m_resblks_save = 0;
	} else
		resblks = xfs_default_resblks(mp);

	xfs_reserve_blocks(mp, &resblks, NULL);
}

/*
 * Trigger writeback of all the dirty metadata in the file system.
 *
 * This ensures that the metadata is written to their location on disk rather
 * than just existing in transactions in the log. This means after a quiesce
 * there is no log replay required to write the inodes to disk - this is the
 * primary difference between a sync and a quiesce.
 *
 * Note: xfs_log_quiesce() stops background log work - the callers must ensure
 * it is started again when appropriate.
 */
static void
xfs_quiesce_attr(
	struct xfs_mount	*mp)
{
	int	error = 0;

	/* wait for all modifications to complete */
	while (atomic_read(&mp->m_active_trans) > 0)
		delay(100);

	/* force the log to unpin objects from the now complete transactions */
	xfs_log_force(mp, XFS_LOG_SYNC);

	/* reclaim inodes to do any IO before the freeze completes */
	xfs_reclaim_inodes(mp, 0);
	xfs_reclaim_inodes(mp, SYNC_WAIT);

	/* Push the superblock and write an unmount record */
	error = xfs_log_sbcount(mp);
	if (error)
		xfs_warn(mp, "xfs_attr_quiesce: failed to log sb changes. "
				"Frozen image may not be consistent.");
	/*
	 * Just warn here till VFS can correctly support
	 * read-only remount without racing.
	 */
	WARN_ON(atomic_read(&mp->m_active_trans) != 0);

	xfs_log_quiesce(mp);
}

STATIC int
xfs_fs_remount(
	struct super_block	*sb,
	int			*flags,
	char			*options)
{
	struct xfs_mount	*mp = XFS_M(sb);
	xfs_sb_t		*sbp = &mp->m_sb;
	substring_t		args[MAX_OPT_ARGS];
	char			*p;
	int			error;

	sync_filesystem(sb);
	while ((p = strsep(&options, ",")) != NULL) {
		int token;

		if (!*p)
			continue;

		token = match_token(p, tokens, args);
		switch (token) {
		case Opt_barrier:
			mp->m_flags |= XFS_MOUNT_BARRIER;
			break;
		case Opt_nobarrier:
			mp->m_flags &= ~XFS_MOUNT_BARRIER;
			break;
		case Opt_inode64:
			mp->m_maxagi = xfs_set_inode64(mp, sbp->sb_agcount);
			break;
		case Opt_inode32:
			mp->m_maxagi = xfs_set_inode32(mp, sbp->sb_agcount);
			break;
		default:
			/*
			 * Logically we would return an error here to prevent
			 * users from believing they might have changed
			 * mount options using remount which can't be changed.
			 *
			 * But unfortunately mount(8) adds all options from
			 * mtab and fstab to the mount arguments in some cases
			 * so we can't blindly reject options, but have to
			 * check for each specified option if it actually
			 * differs from the currently set option and only
			 * reject it if that's the case.
			 *
			 * Until that is implemented we return success for
			 * every remount request, and silently ignore all
			 * options that we can't actually change.
			 */
#if 0
			xfs_info(mp,
		"mount option \"%s\" not supported for remount", p);
			return -EINVAL;
#else
			break;
#endif
		}
	}

	/* ro -> rw */
	if ((mp->m_flags & XFS_MOUNT_RDONLY) && !(*flags & MS_RDONLY)) {
		if (mp->m_flags & XFS_MOUNT_NORECOVERY) {
			xfs_warn(mp,
		"ro->rw transition prohibited on norecovery mount");
			return -EINVAL;
		}

		if (XFS_SB_VERSION_NUM(sbp) == XFS_SB_VERSION_5 &&
		    xfs_sb_has_ro_compat_feature(sbp,
					XFS_SB_FEAT_RO_COMPAT_UNKNOWN)) {
			xfs_warn(mp,
"ro->rw transition prohibited on unknown (0x%x) ro-compat filesystem",
				(sbp->sb_features_ro_compat &
					XFS_SB_FEAT_RO_COMPAT_UNKNOWN));
			return -EINVAL;
		}

		mp->m_flags &= ~XFS_MOUNT_RDONLY;

		/*
		 * If this is the first remount to writeable state we
		 * might have some superblock changes to update.
		 */
		if (mp->m_update_sb) {
			error = xfs_sync_sb(mp, false);
			if (error) {
				xfs_warn(mp, "failed to write sb changes");
				return error;
			}
			mp->m_update_sb = false;
		}

		/*
		 * Fill out the reserve pool if it is empty. Use the stashed
		 * value if it is non-zero, otherwise go with the default.
		 */
		xfs_restore_resvblks(mp);
		xfs_log_work_queue(mp);
	}

	/* rw -> ro */
	if (!(mp->m_flags & XFS_MOUNT_RDONLY) && (*flags & MS_RDONLY)) {
		/*
		 * Before we sync the metadata, we need to free up the reserve
		 * block pool so that the used block count in the superblock on
		 * disk is correct at the end of the remount. Stash the current
		 * reserve pool size so that if we get remounted rw, we can
		 * return it to the same size.
		 */
		xfs_save_resvblks(mp);
		xfs_quiesce_attr(mp);
		mp->m_flags |= XFS_MOUNT_RDONLY;
	}

	return 0;
}

/*
 * Second stage of a freeze. The data is already frozen so we only
 * need to take care of the metadata. Once that's done sync the superblock
 * to the log to dirty it in case of a crash while frozen. This ensures that we
 * will recover the unlinked inode lists on the next mount.
 */
STATIC int
xfs_fs_freeze(
	struct super_block	*sb)
{
	struct xfs_mount	*mp = XFS_M(sb);

	xfs_save_resvblks(mp);
	xfs_quiesce_attr(mp);
	return xfs_sync_sb(mp, true);
}

STATIC int
xfs_fs_unfreeze(
	struct super_block	*sb)
{
	struct xfs_mount	*mp = XFS_M(sb);

	xfs_restore_resvblks(mp);
	xfs_log_work_queue(mp);
	return 0;
}

STATIC int
xfs_fs_show_options(
	struct seq_file		*m,
	struct dentry		*root)
{
	return xfs_showargs(XFS_M(root->d_sb), m);
}

/*
 * This function fills in xfs_mount_t fields based on mount args.
 * Note: the superblock _has_ now been read in.
 */
STATIC int
xfs_finish_flags(
	struct xfs_mount	*mp)
{
	int			ronly = (mp->m_flags & XFS_MOUNT_RDONLY);

	/* Fail a mount where the logbuf is smaller than the log stripe */
	if (xfs_sb_version_haslogv2(&mp->m_sb)) {
		if (mp->m_logbsize <= 0 &&
		    mp->m_sb.sb_logsunit > XLOG_BIG_RECORD_BSIZE) {
			mp->m_logbsize = mp->m_sb.sb_logsunit;
		} else if (mp->m_logbsize > 0 &&
			   mp->m_logbsize < mp->m_sb.sb_logsunit) {
			xfs_warn(mp,
		"logbuf size must be greater than or equal to log stripe size");
			return -EINVAL;
		}
	} else {
		/* Fail a mount if the logbuf is larger than 32K */
		if (mp->m_logbsize > XLOG_BIG_RECORD_BSIZE) {
			xfs_warn(mp,
		"logbuf size for version 1 logs must be 16K or 32K");
			return -EINVAL;
		}
	}

	/*
	 * V5 filesystems always use attr2 format for attributes.
	 */
	if (xfs_sb_version_hascrc(&mp->m_sb) &&
	    (mp->m_flags & XFS_MOUNT_NOATTR2)) {
		xfs_warn(mp,
"Cannot mount a V5 filesystem as %s. %s is always enabled for V5 filesystems.",
			MNTOPT_NOATTR2, MNTOPT_ATTR2);
		return -EINVAL;
	}

	/*
	 * mkfs'ed attr2 will turn on attr2 mount unless explicitly
	 * told by noattr2 to turn it off
	 */
	if (xfs_sb_version_hasattr2(&mp->m_sb) &&
	    !(mp->m_flags & XFS_MOUNT_NOATTR2))
		mp->m_flags |= XFS_MOUNT_ATTR2;

	/*
	 * prohibit r/w mounts of read-only filesystems
	 */
	if ((mp->m_sb.sb_flags & XFS_SBF_READONLY) && !ronly) {
		xfs_warn(mp,
			"cannot mount a read-only filesystem as read-write");
		return -EROFS;
	}

	if ((mp->m_qflags & (XFS_GQUOTA_ACCT | XFS_GQUOTA_ACTIVE)) &&
	    (mp->m_qflags & (XFS_PQUOTA_ACCT | XFS_PQUOTA_ACTIVE)) &&
	    !xfs_sb_version_has_pquotino(&mp->m_sb)) {
		xfs_warn(mp,
		  "Super block does not support project and group quota together");
		return -EINVAL;
	}

	return 0;
}

static int
xfs_init_percpu_counters(
	struct xfs_mount	*mp)
{
	int		error;

	error = percpu_counter_init(&mp->m_icount, 0, GFP_KERNEL);
	if (error)
		return -ENOMEM;

	error = percpu_counter_init(&mp->m_ifree, 0, GFP_KERNEL);
	if (error)
		goto free_icount;

	error = percpu_counter_init(&mp->m_fdblocks, 0, GFP_KERNEL);
	if (error)
		goto free_ifree;

	return 0;

free_ifree:
	percpu_counter_destroy(&mp->m_ifree);
free_icount:
	percpu_counter_destroy(&mp->m_icount);
	return -ENOMEM;
}

void
xfs_reinit_percpu_counters(
	struct xfs_mount	*mp)
{
	percpu_counter_set(&mp->m_icount, mp->m_sb.sb_icount);
	percpu_counter_set(&mp->m_ifree, mp->m_sb.sb_ifree);
	percpu_counter_set(&mp->m_fdblocks, mp->m_sb.sb_fdblocks);
}

static void
xfs_destroy_percpu_counters(
	struct xfs_mount	*mp)
{
	percpu_counter_destroy(&mp->m_icount);
	percpu_counter_destroy(&mp->m_ifree);
	percpu_counter_destroy(&mp->m_fdblocks);
}

STATIC int
xfs_fs_fill_super(
	struct super_block	*sb,
	void			*data,
	int			silent)
{
	struct inode		*root;
	struct xfs_mount	*mp = NULL;
	int			flags = 0, error = -ENOMEM;

	mp = kzalloc(sizeof(struct xfs_mount), GFP_KERNEL);
	if (!mp)
		goto out;

	spin_lock_init(&mp->m_sb_lock);
	mutex_init(&mp->m_growlock);
	atomic_set(&mp->m_active_trans, 0);
	INIT_DELAYED_WORK(&mp->m_reclaim_work, xfs_reclaim_worker);
	INIT_DELAYED_WORK(&mp->m_eofblocks_work, xfs_eofblocks_worker);
	mp->m_kobj.kobject.kset = xfs_kset;

	mp->m_super = sb;
	sb->s_fs_info = mp;

	error = xfs_parseargs(mp, (char *)data);
	if (error)
		goto out_free_fsname;

	sb_min_blocksize(sb, BBSIZE);
	sb->s_xattr = xfs_xattr_handlers;
	sb->s_export_op = &xfs_export_operations;
#ifdef CONFIG_XFS_QUOTA
	sb->s_qcop = &xfs_quotactl_operations;
	sb->s_quota_types = QTYPE_MASK_USR | QTYPE_MASK_GRP | QTYPE_MASK_PRJ;
#endif
	sb->s_op = &xfs_super_operations;

	if (silent)
		flags |= XFS_MFSI_QUIET;

	error = xfs_open_devices(mp);
	if (error)
		goto out_free_fsname;

	error = xfs_init_mount_workqueues(mp);
	if (error)
		goto out_close_devices;

	error = xfs_init_percpu_counters(mp);
	if (error)
		goto out_destroy_workqueues;

	error = xfs_readsb(mp, flags);
	if (error)
		goto out_destroy_counters;

	error = xfs_finish_flags(mp);
	if (error)
		goto out_free_sb;

	error = xfs_setup_devices(mp);
	if (error)
		goto out_free_sb;

	error = xfs_filestream_mount(mp);
	if (error)
		goto out_free_sb;

	/*
	 * we must configure the block size in the superblock before we run the
	 * full mount process as the mount process can lookup and cache inodes.
	 */
	sb->s_magic = XFS_SB_MAGIC;
	sb->s_blocksize = mp->m_sb.sb_blocksize;
	sb->s_blocksize_bits = ffs(sb->s_blocksize) - 1;
	sb->s_maxbytes = xfs_max_file_offset(sb->s_blocksize_bits);
	sb->s_max_links = XFS_MAXLINK;
	sb->s_time_gran = 1;
	set_posix_acl_flag(sb);

	/* version 5 superblocks support inode version counters. */
	if (XFS_SB_VERSION_NUM(&mp->m_sb) == XFS_SB_VERSION_5)
		sb->s_flags |= MS_I_VERSION;

	error = xfs_mountfs(mp);
	if (error)
		goto out_filestream_unmount;

	root = igrab(VFS_I(mp->m_rootip));
	if (!root) {
		error = -ENOENT;
		goto out_unmount;
	}
	sb->s_root = d_make_root(root);
	if (!sb->s_root) {
		error = -ENOMEM;
		goto out_unmount;
	}

	return 0;

 out_filestream_unmount:
	xfs_filestream_unmount(mp);
 out_free_sb:
	xfs_freesb(mp);
 out_destroy_counters:
	xfs_destroy_percpu_counters(mp);
out_destroy_workqueues:
	xfs_destroy_mount_workqueues(mp);
 out_close_devices:
	xfs_close_devices(mp);
 out_free_fsname:
	xfs_free_fsname(mp);
	kfree(mp);
 out:
	return error;

 out_unmount:
	xfs_filestream_unmount(mp);
	xfs_unmountfs(mp);
	goto out_free_sb;
}

STATIC void
xfs_fs_put_super(
	struct super_block	*sb)
{
	struct xfs_mount	*mp = XFS_M(sb);

	xfs_notice(mp, "Unmounting Filesystem");
	xfs_filestream_unmount(mp);
	xfs_unmountfs(mp);

	xfs_freesb(mp);
	xfs_destroy_percpu_counters(mp);
	xfs_destroy_mount_workqueues(mp);
	xfs_close_devices(mp);
	xfs_free_fsname(mp);
	kfree(mp);
}

STATIC struct dentry *
xfs_fs_mount(
	struct file_system_type	*fs_type,
	int			flags,
	const char		*dev_name,
	void			*data)
{
	return mount_bdev(fs_type, flags, dev_name, data, xfs_fs_fill_super);
}

static long
xfs_fs_nr_cached_objects(
	struct super_block	*sb,
	struct shrink_control	*sc)
{
	return xfs_reclaim_inodes_count(XFS_M(sb));
}

static long
xfs_fs_free_cached_objects(
	struct super_block	*sb,
	struct shrink_control	*sc)
{
	return xfs_reclaim_inodes_nr(XFS_M(sb), sc->nr_to_scan);
}

static const struct super_operations xfs_super_operations = {
	.alloc_inode		= xfs_fs_alloc_inode,
	.destroy_inode		= xfs_fs_destroy_inode,
	.evict_inode		= xfs_fs_evict_inode,
	.drop_inode		= xfs_fs_drop_inode,
	.put_super		= xfs_fs_put_super,
	.sync_fs		= xfs_fs_sync_fs,
	.freeze_fs		= xfs_fs_freeze,
	.unfreeze_fs		= xfs_fs_unfreeze,
	.statfs			= xfs_fs_statfs,
	.remount_fs		= xfs_fs_remount,
	.show_options		= xfs_fs_show_options,
	.nr_cached_objects	= xfs_fs_nr_cached_objects,
	.free_cached_objects	= xfs_fs_free_cached_objects,
};

static struct file_system_type xfs_fs_type = {
	.owner			= THIS_MODULE,
	.name			= "xfs",
	.mount			= xfs_fs_mount,
	.kill_sb		= kill_block_super,
	.fs_flags		= FS_REQUIRES_DEV,
};
MODULE_ALIAS_FS("xfs");

STATIC int __init
xfs_init_zones(void)
{

	xfs_ioend_zone = kmem_zone_init(sizeof(xfs_ioend_t), "xfs_ioend");
	if (!xfs_ioend_zone)
		goto out;

	xfs_ioend_pool = mempool_create_slab_pool(4 * MAX_BUF_PER_PAGE,
						  xfs_ioend_zone);
	if (!xfs_ioend_pool)
		goto out_destroy_ioend_zone;

	xfs_log_ticket_zone = kmem_zone_init(sizeof(xlog_ticket_t),
						"xfs_log_ticket");
	if (!xfs_log_ticket_zone)
		goto out_destroy_ioend_pool;

	xfs_bmap_free_item_zone = kmem_zone_init(sizeof(xfs_bmap_free_item_t),
						"xfs_bmap_free_item");
	if (!xfs_bmap_free_item_zone)
		goto out_destroy_log_ticket_zone;

	xfs_btree_cur_zone = kmem_zone_init(sizeof(xfs_btree_cur_t),
						"xfs_btree_cur");
	if (!xfs_btree_cur_zone)
		goto out_destroy_bmap_free_item_zone;

	xfs_da_state_zone = kmem_zone_init(sizeof(xfs_da_state_t),
						"xfs_da_state");
	if (!xfs_da_state_zone)
		goto out_destroy_btree_cur_zone;

	xfs_ifork_zone = kmem_zone_init(sizeof(xfs_ifork_t), "xfs_ifork");
	if (!xfs_ifork_zone)
		goto out_destroy_da_state_zone;

	xfs_trans_zone = kmem_zone_init(sizeof(xfs_trans_t), "xfs_trans");
	if (!xfs_trans_zone)
		goto out_destroy_ifork_zone;

	xfs_log_item_desc_zone =
		kmem_zone_init(sizeof(struct xfs_log_item_desc),
			       "xfs_log_item_desc");
	if (!xfs_log_item_desc_zone)
		goto out_destroy_trans_zone;

	/*
	 * The size of the zone allocated buf log item is the maximum
	 * size possible under XFS.  This wastes a little bit of memory,
	 * but it is much faster.
	 */
	xfs_buf_item_zone = kmem_zone_init(sizeof(struct xfs_buf_log_item),
					   "xfs_buf_item");
	if (!xfs_buf_item_zone)
		goto out_destroy_log_item_desc_zone;

	xfs_efd_zone = kmem_zone_init((sizeof(xfs_efd_log_item_t) +
			((XFS_EFD_MAX_FAST_EXTENTS - 1) *
				 sizeof(xfs_extent_t))), "xfs_efd_item");
	if (!xfs_efd_zone)
		goto out_destroy_buf_item_zone;

	xfs_efi_zone = kmem_zone_init((sizeof(xfs_efi_log_item_t) +
			((XFS_EFI_MAX_FAST_EXTENTS - 1) *
				sizeof(xfs_extent_t))), "xfs_efi_item");
	if (!xfs_efi_zone)
		goto out_destroy_efd_zone;

	xfs_inode_zone =
		kmem_zone_init_flags(sizeof(xfs_inode_t), "xfs_inode",
			KM_ZONE_HWALIGN | KM_ZONE_RECLAIM | KM_ZONE_SPREAD,
			xfs_fs_inode_init_once);
	if (!xfs_inode_zone)
		goto out_destroy_efi_zone;

	xfs_ili_zone =
		kmem_zone_init_flags(sizeof(xfs_inode_log_item_t), "xfs_ili",
					KM_ZONE_SPREAD, NULL);
	if (!xfs_ili_zone)
		goto out_destroy_inode_zone;
	xfs_icreate_zone = kmem_zone_init(sizeof(struct xfs_icreate_item),
					"xfs_icr");
	if (!xfs_icreate_zone)
		goto out_destroy_ili_zone;

	return 0;

 out_destroy_ili_zone:
	kmem_zone_destroy(xfs_ili_zone);
 out_destroy_inode_zone:
	kmem_zone_destroy(xfs_inode_zone);
 out_destroy_efi_zone:
	kmem_zone_destroy(xfs_efi_zone);
 out_destroy_efd_zone:
	kmem_zone_destroy(xfs_efd_zone);
 out_destroy_buf_item_zone:
	kmem_zone_destroy(xfs_buf_item_zone);
 out_destroy_log_item_desc_zone:
	kmem_zone_destroy(xfs_log_item_desc_zone);
 out_destroy_trans_zone:
	kmem_zone_destroy(xfs_trans_zone);
 out_destroy_ifork_zone:
	kmem_zone_destroy(xfs_ifork_zone);
 out_destroy_da_state_zone:
	kmem_zone_destroy(xfs_da_state_zone);
 out_destroy_btree_cur_zone:
	kmem_zone_destroy(xfs_btree_cur_zone);
 out_destroy_bmap_free_item_zone:
	kmem_zone_destroy(xfs_bmap_free_item_zone);
 out_destroy_log_ticket_zone:
	kmem_zone_destroy(xfs_log_ticket_zone);
 out_destroy_ioend_pool:
	mempool_destroy(xfs_ioend_pool);
 out_destroy_ioend_zone:
	kmem_zone_destroy(xfs_ioend_zone);
 out:
	return -ENOMEM;
}

STATIC void
xfs_destroy_zones(void)
{
	/*
	 * Make sure all delayed rcu free are flushed before we
	 * destroy caches.
	 */
	rcu_barrier();
	kmem_zone_destroy(xfs_icreate_zone);
	kmem_zone_destroy(xfs_ili_zone);
	kmem_zone_destroy(xfs_inode_zone);
	kmem_zone_destroy(xfs_efi_zone);
	kmem_zone_destroy(xfs_efd_zone);
	kmem_zone_destroy(xfs_buf_item_zone);
	kmem_zone_destroy(xfs_log_item_desc_zone);
	kmem_zone_destroy(xfs_trans_zone);
	kmem_zone_destroy(xfs_ifork_zone);
	kmem_zone_destroy(xfs_da_state_zone);
	kmem_zone_destroy(xfs_btree_cur_zone);
	kmem_zone_destroy(xfs_bmap_free_item_zone);
	kmem_zone_destroy(xfs_log_ticket_zone);
	mempool_destroy(xfs_ioend_pool);
	kmem_zone_destroy(xfs_ioend_zone);

}

STATIC int __init
xfs_init_workqueues(void)
{
	/*
	 * The allocation workqueue can be used in memory reclaim situations
	 * (writepage path), and parallelism is only limited by the number of
	 * AGs in all the filesystems mounted. Hence use the default large
	 * max_active value for this workqueue.
	 */
	xfs_alloc_wq = alloc_workqueue("xfsalloc",
			WQ_MEM_RECLAIM|WQ_FREEZABLE, 0);
	if (!xfs_alloc_wq)
		return -ENOMEM;

	return 0;
}

STATIC void
xfs_destroy_workqueues(void)
{
	destroy_workqueue(xfs_alloc_wq);
}

STATIC int __init
init_xfs_fs(void)
{
	int			error;

	printk(KERN_INFO XFS_VERSION_STRING " with "
			 XFS_BUILD_OPTIONS " enabled\n");

	xfs_dir_startup();

	error = xfs_init_zones();
	if (error)
		goto out;

	error = xfs_init_workqueues();
	if (error)
		goto out_destroy_zones;

	error = xfs_mru_cache_init();
	if (error)
		goto out_destroy_wq;

	error = xfs_buf_init();
	if (error)
		goto out_mru_cache_uninit;

	error = xfs_init_procfs();
	if (error)
		goto out_buf_terminate;

	error = xfs_sysctl_register();
	if (error)
		goto out_cleanup_procfs;

	xfs_kset = kset_create_and_add("xfs", NULL, fs_kobj);
	if (!xfs_kset) {
		error = -ENOMEM;
		goto out_sysctl_unregister;;
	}

#ifdef DEBUG
	xfs_dbg_kobj.kobject.kset = xfs_kset;
	error = xfs_sysfs_init(&xfs_dbg_kobj, &xfs_dbg_ktype, NULL, "debug");
	if (error)
		goto out_kset_unregister;
#endif

	error = xfs_qm_init();
	if (error)
		goto out_remove_kobj;

	error = register_filesystem(&xfs_fs_type);
	if (error)
		goto out_qm_exit;
	return 0;

 out_qm_exit:
	xfs_qm_exit();
 out_remove_kobj:
#ifdef DEBUG
	xfs_sysfs_del(&xfs_dbg_kobj);
 out_kset_unregister:
#endif
	kset_unregister(xfs_kset);
 out_sysctl_unregister:
	xfs_sysctl_unregister();
 out_cleanup_procfs:
	xfs_cleanup_procfs();
 out_buf_terminate:
	xfs_buf_terminate();
 out_mru_cache_uninit:
	xfs_mru_cache_uninit();
 out_destroy_wq:
	xfs_destroy_workqueues();
 out_destroy_zones:
	xfs_destroy_zones();
 out:
	return error;
}

STATIC void __exit
exit_xfs_fs(void)
{
	xfs_qm_exit();
	unregister_filesystem(&xfs_fs_type);
#ifdef DEBUG
	xfs_sysfs_del(&xfs_dbg_kobj);
#endif
	kset_unregister(xfs_kset);
	xfs_sysctl_unregister();
	xfs_cleanup_procfs();
	xfs_buf_terminate();
	xfs_mru_cache_uninit();
	xfs_destroy_workqueues();
	xfs_destroy_zones();
}

module_init(init_xfs_fs);
module_exit(exit_xfs_fs);

MODULE_AUTHOR("Silicon Graphics, Inc.");
MODULE_DESCRIPTION(XFS_VERSION_STRING " with " XFS_BUILD_OPTIONS " enabled");
MODULE_LICENSE("GPL");
