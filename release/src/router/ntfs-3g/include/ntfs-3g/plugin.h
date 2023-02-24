/*
 *		plugin.h : define interface for plugin development
 *
 * Copyright (c) 2015-2021 Jean-Pierre Andre
 *
 */

/*
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

/*
 *	This file defines the interface to ntfs-3g plugins which
 *	add support for processing some type of reparse points.
 */

#ifndef _NTFS_PLUGIN_H
#define _NTFS_PLUGIN_H

#include "layout.h"
#include "inode.h"
#include "dir.h"

struct fuse_file_info;
struct stat;

	/*
	 *	The plugin operations currently defined.
	 * These functions should return a non-negative value when they
	 * succeed, or a negative errno value when they fail.
	 * They must not close or free their arguments.
	 * The file system must be left in a consistent state after
	 * each individual call.
	 * If an operation is not defined, an EOPNOTSUPP error is
	 * returned to caller.
	 */
typedef struct plugin_operations {
	/*
	 *	Set the attributes st_size, st_blocks and st_mode
	 * into a struct stat. The returned st_mode must at least
	 * define the file type. Depending on the permissions options
	 * used for mounting, the umask will be applied to the returned
	 * permissions, or the permissions will be changed according
	 * to the ACL set on the file.
	 */
	int (*getattr)(ntfs_inode *ni, const REPARSE_POINT *reparse,
			struct stat *stbuf);

	/*
	 *	Open a file for reading or writing
	 * The field fi->flags indicates the kind of opening.
	 * The field fi->fh may be used to store some information which
	 * will be available to subsequent reads and writes. When used
	 * this field must be non-null.
	 * The returned value is zero for success and a negative errno
	 * value for failure.
	 */
	int (*open)(ntfs_inode *ni, const REPARSE_POINT *reparse,
			struct fuse_file_info *fi);

	/*
	 *	Release an open file or directory
	 * This is only called if fi->fh has been set to a non-null
	 * value while opening. It may be used to free some context
	 * specific to the open file or directory
	 * The returned value is zero for success or a negative errno
	 * value for failure.
	 */
	int (*release)(ntfs_inode *ni, const REPARSE_POINT *reparse,
			struct fuse_file_info *fi);

	/*
	 *	Read from an open file
	 * The returned value is the count of bytes which were read
	 * or a negative errno value for failure.
	 * If the returned value is positive, the access time stamp
	 * will be updated after the call.
	 */
	int (*read)(ntfs_inode *ni, const REPARSE_POINT *reparse,
			char *buf, size_t size,
			off_t offset, struct fuse_file_info *fi);

	/*
	 *	Write to an open file
	 * The file system must be left consistent after each write call,
	 * the file itself must be at least deletable if the application
	 * writing to it is killed for some reason.
	 * The returned value is the count of bytes which were written
	 * or a negative errno value for failure.
	 * If the returned value is positive, the modified time stamp
	 * will be updated after the call.
	 */
	int (*write)(ntfs_inode *ni, const REPARSE_POINT *reparse,
			const char *buf, size_t size,
			off_t offset, struct fuse_file_info *fi);

	/*
	 *	Get a symbolic link
	 * The symbolic link must be returned in an allocated buffer,
	 * encoded in a zero terminated multibyte string compatible
	 * with the locale mount option.
	 * The returned value is zero for success or a negative errno
	 * value for failure.
	 */
	int (*readlink)(ntfs_inode *ni, const REPARSE_POINT *reparse,
			char **pbuf);

	/*
	 *	Truncate a file (shorten or append zeroes)
	 * The returned value is zero for success or a negative errno
	 * value for failure.
	 * If the returned value is zero, the modified time stamp
	 * will be updated after the call.
	 */
	int (*truncate)(ntfs_inode *ni, const REPARSE_POINT *reparse,
			off_t size);
	/*
	 *	Open a directory
	 * The field fi->flags indicates the kind of opening.
	 * The field fi->fh may be used to store some information which
	 * will be available to subsequent readdir(). When used
	 * this field must be non-null and freed in release().
	 * The returned value is zero for success and a negative errno
	 * value for failure.
	 */
	int (*opendir)(ntfs_inode *ni, const REPARSE_POINT *reparse,
			struct fuse_file_info *fi);

	/*
	 *	Get entries from a directory
	 *
	 * Use the filldir() function with fillctx argument to return
	 * the directory entries.
	 * Names "." and ".." are expected to be returned.
	 * The returned value is zero for success and a negative errno
	 * value for failure.
	 */
	int (*readdir)(ntfs_inode *ni, const REPARSE_POINT *reparse,
			s64 *pos, void *fillctx, ntfs_filldir_t filldir,
			struct fuse_file_info *fi);
	/*
	 *	Create a new file of any type
	 *
	 * The returned value is a pointer to the inode created, or
	 * NULL if failed, with errno telling why.
	 */
	ntfs_inode *(*create)(ntfs_inode *dir_ni, const REPARSE_POINT *reparse,
			le32 securid, ntfschar *name, int name_len,
			mode_t type);
	/*
	 *	Link a new name to a file or directory
	 * Linking a directory is needed for renaming a directory
	 * The returned value is zero for success or a negative errno
	 * value for failure.
	 * If the returned value is zero, the modified time stamp
	 * will be updated after the call.
	 */
	int (*link)(ntfs_inode *dir_ni, const REPARSE_POINT *reparse,
			ntfs_inode *ni, ntfschar *name, int name_len);
	/*
	 *	Unlink a name from a directory
	 * The argument pathname may be NULL
	 * The returned value is zero for success or a negative errno
	 * value for failure.
	 */
	int (*unlink)(ntfs_inode *dir_ni, const REPARSE_POINT *reparse,
			const char *pathname,
			ntfs_inode *ni, ntfschar *name, int name_len);
} plugin_operations_t;


/*
 *		Plugin initialization routine
 *	Returns the entry table if successful, otherwise returns NULL
 *	and sets errno (e.g. to EINVAL if the tag is not supported by
 *	the plugin.)
 */
typedef const struct plugin_operations *(*plugin_init_t)(le32 tag);
const struct plugin_operations *init(le32 tag);

#endif /* _NTFS_PLUGIN_H */
