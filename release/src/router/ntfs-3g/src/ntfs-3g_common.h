/*
 * ntfs-3g_common.h - Common declarations for ntfs-3g and lowntfs-3g.
 *
 * Copyright (c) 2010-2011 Jean-Pierre Andre
 * Copyright (c) 2010      Erik Larsson
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

#ifndef _NTFS_3G_COMMON_H
#define _NTFS_3G_COMMON_H

#include "inode.h"

struct ntfs_options {
        char    *mnt_point;     /* Mount point */    
        char    *options;       /* Mount options */  
        char    *device;        /* Device to mount */
	char	*arg_device;	/* Device requested in argv */
} ;

typedef enum {
	NF_STREAMS_INTERFACE_NONE,	/* No access to named data streams. */
	NF_STREAMS_INTERFACE_XATTR,	/* Map named data streams to xattrs. */
	NF_STREAMS_INTERFACE_OPENXATTR,	/* Same, not limited to "user." */
	NF_STREAMS_INTERFACE_WINDOWS,	/* "file:stream" interface. */
} ntfs_fuse_streams_interface;

struct DEFOPTION {
	const char *name;
	int type;
	int flags;
} ;
			/* Options, order not significant */
enum {
	OPT_RO,
	OPT_NOATIME,
	OPT_ATIME,
	OPT_RELATIME,
	OPT_DMTIME,
	OPT_RW,
	OPT_FAKE_RW,
	OPT_FSNAME,
	OPT_NO_DEF_OPTS,
	OPT_DEFAULT_PERMISSIONS,
	OPT_PERMISSIONS,
	OPT_ACL,
	OPT_UMASK,
	OPT_FMASK,
	OPT_DMASK,
	OPT_UID,
	OPT_GID,
	OPT_SHOW_SYS_FILES,
	OPT_HIDE_HID_FILES,
	OPT_HIDE_DOT_FILES,
	OPT_IGNORE_CASE,
	OPT_WINDOWS_NAMES,
	OPT_COMPRESSION,
	OPT_NOCOMPRESSION,
	OPT_SILENT,
	OPT_RECOVER,
	OPT_NORECOVER,
	OPT_REMOVE_HIBERFILE,
	OPT_SYNC,
	OPT_BIG_WRITES,
	OPT_LOCALE,
	OPT_NFCONV,
	OPT_NONFCONV,
	OPT_STREAMS_INTERFACE,
	OPT_USER_XATTR,
	OPT_NOAUTO,
	OPT_DEBUG,
	OPT_NO_DETACH,
	OPT_REMOUNT,
	OPT_BLKSIZE,
	OPT_INHERIT,
	OPT_ADDSECURIDS,
	OPT_STATICGRPS,
	OPT_USERMAPPING,
	OPT_XATTRMAPPING,
	OPT_EFS_RAW,
	OPT_POSIX_NLINK,
	OPT_SPECIAL_FILES,
	OPT_HELP,
	OPT_VERSION,
} ;

			/* Option flags */
enum {
	FLGOPT_BOGUS = 1,
	FLGOPT_STRING = 2,
	FLGOPT_OCTAL = 4,
	FLGOPT_DECIMAL = 8,
	FLGOPT_APPEND = 16,
	FLGOPT_NOSUPPORT = 32,
	FLGOPT_OPTIONAL = 64
} ;

typedef enum {
	ATIME_ENABLED,
	ATIME_DISABLED,
	ATIME_RELATIVE
} ntfs_atime_t;

typedef enum {
	ERR_PLUGIN = 1
} single_log_t;

#ifndef DISABLE_PLUGINS

typedef struct plugin_list {
	struct plugin_list *next;
	void *handle;
	const plugin_operations_t *ops;
	le32 tag;
} plugin_list_t;

#endif /* DISABLE_PLUGINS */

typedef struct {
	ntfs_volume *vol;
	unsigned int uid;
	unsigned int gid;
	unsigned int fmask;
	unsigned int dmask;
	ntfs_fuse_streams_interface streams;
	ntfs_atime_t atime;
	s64 dmtime;
	BOOL ro;
	BOOL rw;
	BOOL show_sys_files;
	BOOL hide_hid_files;
	BOOL hide_dot_files;
	BOOL windows_names;
	BOOL ignore_case;
	BOOL compression;
	BOOL acl;
	BOOL silent;
	BOOL recover;
	BOOL hiberfile;
	BOOL sync;
	BOOL big_writes;
	BOOL debug;
	BOOL no_detach;
	BOOL blkdev;
	BOOL mounted;
	BOOL posix_nlink;
	ntfs_volume_special_files special_files;
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
	BOOL efs_raw;
#ifdef XATTR_MAPPINGS
	char *xattrmap_path;
#endif /* XATTR_MAPPINGS */
#endif /* HAVE_SETXATTR */
	struct fuse_chan *fc;
	BOOL inherit;
	unsigned int secure_flags;
	single_log_t errors_logged;
	char *usermap_path;
	char *abs_mnt_point;
#ifndef DISABLE_PLUGINS
	plugin_list_t *plugins;
#endif /* DISABLE_PLUGINS */
	struct PERMISSIONS_CACHE *seccache;
	struct SECURITY_CONTEXT security;
	struct open_file *open_files; /* only defined in lowntfs-3g */
	u64 latest_ghost;
} ntfs_fuse_context_t;

extern const char *EXEC_NAME;

#ifdef FUSE_INTERNAL
#define FUSE_TYPE	"integrated FUSE"
#else
#define FUSE_TYPE	"external FUSE"
#endif

extern const char xattr_ntfs_3g[];

extern const char nf_ns_user_prefix[];
extern const int nf_ns_user_prefix_len;
extern const char nf_ns_system_prefix[];
extern const int nf_ns_system_prefix_len;
extern const char nf_ns_security_prefix[];
extern const int nf_ns_security_prefix_len;
extern const char nf_ns_trusted_prefix[];
extern const int nf_ns_trusted_prefix_len;

int ntfs_strappend(char **dest, const char *append);
int ntfs_strinsert(char **dest, const char *append);
char *parse_mount_options(ntfs_fuse_context_t *ctx,
			const struct ntfs_options *popts, BOOL low_fuse);
int ntfs_parse_options(struct ntfs_options *popts, void (*usage)(void),
			int argc, char *argv[]);

int ntfs_fuse_listxattr_common(ntfs_inode *ni, ntfs_attr_search_ctx *actx,
 			char *list, size_t size, BOOL prefixing);
BOOL user_xattrs_allowed(ntfs_fuse_context_t *ctx, ntfs_inode *ni);

#ifndef DISABLE_PLUGINS

void close_reparse_plugins(ntfs_fuse_context_t *ctx);
const struct plugin_operations *select_reparse_plugin(ntfs_fuse_context_t *ctx,
				ntfs_inode *ni, REPARSE_POINT **reparse);
int register_reparse_plugin(ntfs_fuse_context_t *ctx, le32 tag,
                                const plugin_operations_t *ops, void *handle);

#endif /* DISABLE_PLUGINS */

#endif /* _NTFS_3G_COMMON_H */
