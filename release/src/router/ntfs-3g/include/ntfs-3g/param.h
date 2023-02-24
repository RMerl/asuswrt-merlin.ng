/*
 * param.h - Parameter values for ntfs-3g
 *
 * Copyright (c) 2009-2010 Jean-Pierre Andre
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _NTFS_PARAM_H
#define _NTFS_PARAM_H

#define CACHE_INODE_SIZE 32	/* inode cache, zero or >= 3 and not too big */
#define CACHE_NIDATA_SIZE 64	/* idata cache, zero or >= 3 and not too big */
#define CACHE_LOOKUP_SIZE 64	/* lookup cache, zero or >= 3 and not too big */
#define CACHE_SECURID_SIZE 16    /* securid cache, zero or >= 3 and not too big */
#define CACHE_LEGACY_SIZE 8    /* legacy cache size, zero or >= 3 and not too big */

#define FORCE_FORMAT_v1x 0	/* Insert security data as in NTFS v1.x */
#define OWNERFROMACL 1		/* Get the owner from ACL (not Windows owner) */

		/* default security sub-authorities */
enum {
	DEFSECAUTH1 = -1153374643, /* 3141592653 */
	DEFSECAUTH2 = 589793238,
	DEFSECAUTH3 = 462843383,
	DEFSECBASE = 10000
};

/*
 *		Parameters for formatting
 */

		/* Up to Windows 10, the cluster size was limited to 64K */
#define NTFS_MAX_CLUSTER_SIZE 2097152 /* Windows 10 Creators allows 2MB */

/*
 *		Parameters for compression
 */

	/* default option for compression */
#define DEFAULT_COMPRESSION 1
	/* (log2 of) number of clusters in a compression block for new files */
#define STANDARD_COMPRESSION_UNIT 4
	/* maximum cluster size for allowing compression for new files */
#define MAX_COMPRESSION_CLUSTER_SIZE 4096

/*
 *		Parameters for default options
 */

#define DEFAULT_DMTIME 60 /* default 1mn for delay_mtime */

/*
 *		Use of big write buffers
 *
 *	With small volumes, the cluster allocator may fail to allocate
 *	enough clusters when the volume is nearly full. At most a run
 *	can be allocated per bitmap chunk. So, there is a danger when the
 *	number of chunks (capacity/(32768*clsiz)) is less than the number
 *	of clusters in the biggest write buffer (131072/clsiz). Hence
 *	a safe minimal capacity is 4GB
 */

#define SAFE_CAPACITY_FOR_BIG_WRITES 0x100000000LL

/*
 *		Parameters for runlists
 */

	/* only update the final extent of a runlist when appending data */
#define PARTIAL_RUNLIST_UPDATING 1

/*
 *		Parameters for upper-case table
 */

	/* Create upper-case tables as defined by Windows 6.1 (Win7) */
#define UPCASE_MAJOR 6
#define UPCASE_MINOR 1

/*
 *		Parameters for user and xattr mappings
 */

#define XATTRMAPPINGFILE ".NTFS-3G/XattrMapping" /* default mapping file */

/*
 *		Parameters for path canonicalization
 */

#define MAPPERNAMELTH 256

/*
 *		Permission checking modes for high level and low level
 *
 *	The choices for high and low lowel are independent, they have
 *	no effect on the library
 *
 *	Stick to the recommended values unless you understand the consequences
 *	on protection and performances. Use of cacheing is good for
 *	performances, but bad on security with internal fuse or external
 *	fuse older than 2.8
 *
 *	On Linux, cacheing is discouraged for the high level interface
 *	in order to get proper support of hard links. As a consequence,
 *	having access control in the file system leads to fewer requests
 *	to the file system and fewer context switches.
 *
 *	Irrespective of the selected mode, cacheing is always used
 *	in read-only mounts
 *
 *	Possible values for high level :
 *		1 : no cache, kernel control (recommended)
 *		4 : no cache, file system control
 *		6 : kernel/fuse cache, file system control (OpenIndiana only)
 *		7 : no cache, kernel control for ACLs
 *
 *	Possible values for low level :
 *		2 : no cache, kernel control
 *		3 : use kernel/fuse cache, kernel control (recommended)
 *		5 : no cache, file system control
 *		6 : kernel/fuse cache, file system control (OpenIndiana only)
 *		8 : no cache, kernel control for ACLs
 *		9 : kernel/fuse cache, kernel control for ACLs (target)
 *
 *	Use of options 7, 8 and 9 requires a fuse module upgrade
 *	When Posix ACLs are selected in the configure options, a value
 *	of 6 is added in the mount report.
 */

#define TIMEOUT_RO 600 /* Attribute time out for read-only mounts */
#if defined(__sun) && defined(__SVR4)
/*
 *	Access control by kernel is not implemented on OpenIndiana,
 *	however care is taken of cacheing hard-linked files.
 */
#define HPERMSCONFIG 6
#define LPERMSCONFIG 6
#else /* defined(__sun) && defined(__SVR4) */
/*
 *	Cacheing by kernel is buggy on Linux when access control is done
 *	by the file system, and also when using hard-linked files on
 *	the fuse high level interface.
 *	Also ACL checks by recent kernels do not prove satisfactory.
 */
#define HPERMSCONFIG 1
#define LPERMSCONFIG 3
#endif /* defined(__sun) && defined(__SVR4) */

#endif /* defined _NTFS_PARAM_H */
