/*
 * xattrs.h : definitions related to system extended attributes
 *
 * Copyright (c) 2010 Jean-Pierre Andre
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

#ifndef _NTFS_XATTRS_H_
#define _NTFS_XATTRS_H_

/*
 * Flags that modify setxattr() semantics.  These flags are also used by a
 * number of libntfs-3g functions, such as ntfs_set_ntfs_acl(), which were
 * originally tied to extended attributes support but now can be used by
 * applications even if the platform does not support extended attributes.
 *
 * Careful: applications including this header should define HAVE_SETXATTR or
 * HAVE_SYS_XATTR_H if the platform supports extended attributes.  Otherwise the
 * defined flags values may be incorrect (they will be correct for Linux but not
 * necessarily for other platforms).
 */
#if defined(HAVE_SETXATTR) || defined(HAVE_SYS_XATTR_H)
#include <sys/xattr.h>
#else
#include "compat.h" /* may be needed for ENODATA definition */
#define XATTR_CREATE	1
#define XATTR_REPLACE	2
#endif

/*
 *		Identification of data mapped to the system name space
 */

enum SYSTEMXATTRS {
	XATTR_UNMAPPED,
	XATTR_NTFS_ACL,
	XATTR_NTFS_ATTRIB,
	XATTR_NTFS_ATTRIB_BE,
	XATTR_NTFS_EFSINFO,
	XATTR_NTFS_REPARSE_DATA,
	XATTR_NTFS_OBJECT_ID,
	XATTR_NTFS_DOS_NAME,
	XATTR_NTFS_TIMES,
	XATTR_NTFS_TIMES_BE,
	XATTR_NTFS_CRTIME,
	XATTR_NTFS_CRTIME_BE,
	XATTR_NTFS_EA,
	XATTR_POSIX_ACC, 
	XATTR_POSIX_DEF
} ;

struct XATTRMAPPING {
	struct XATTRMAPPING *next;
	enum SYSTEMXATTRS xattr;
	char name[1]; /* variable length */
} ;

#ifdef XATTR_MAPPINGS

struct XATTRMAPPING *ntfs_xattr_build_mapping(ntfs_volume *vol,
			const char *path);
void ntfs_xattr_free_mapping(struct XATTRMAPPING*);

#endif /* XATTR_MAPPINGS */

enum SYSTEMXATTRS ntfs_xattr_system_type(const char *name,
			ntfs_volume *vol);

struct SECURITY_CONTEXT;

int ntfs_xattr_system_getxattr(struct SECURITY_CONTEXT *scx,
			enum SYSTEMXATTRS attr,
			ntfs_inode *ni, ntfs_inode *dir_ni,
			char *value, size_t size);
int ntfs_xattr_system_setxattr(struct SECURITY_CONTEXT *scx,
			enum SYSTEMXATTRS attr,
			ntfs_inode *ni, ntfs_inode *dir_ni,
			const char *value, size_t size, int flags);
int ntfs_xattr_system_removexattr(struct SECURITY_CONTEXT *scx,
			enum SYSTEMXATTRS attr,
			ntfs_inode *ni, ntfs_inode *dir_ni);

#endif /* _NTFS_XATTRS_H_ */
