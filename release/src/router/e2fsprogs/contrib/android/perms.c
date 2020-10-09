#ifndef _GNU_SOURCE
# define _GNU_SOURCE //asprintf
#endif
#include "perms.h"
#include "support/nls-enable.h"
#include <time.h>
#include <sys/stat.h>

#ifndef XATTR_SELINUX_SUFFIX
# define XATTR_SELINUX_SUFFIX  "selinux"
#endif
#ifndef XATTR_CAPS_SUFFIX
# define XATTR_CAPS_SUFFIX     "capability"
#endif

struct inode_params {
	ext2_filsys fs;
	char *path;
	char *filename;
	char *src_dir;
	char *target_out;
	char *mountpoint;
	fs_config_f fs_config_func;
	struct selabel_handle *sehnd;
	time_t fixed_time;
	const struct ugid_map* uid_map;
	const struct ugid_map* gid_map;
	errcode_t error;
};

static errcode_t ino_add_xattr(ext2_filsys fs, ext2_ino_t ino, const char *name,
			       const void *value, int value_len)
{
	errcode_t retval, close_retval;
	struct ext2_xattr_handle *xhandle;

	retval = ext2fs_xattrs_open(fs, ino, &xhandle);
	if (retval) {
		com_err(__func__, retval, _("while opening inode %u"), ino);
		return retval;
	}
	retval = ext2fs_xattrs_read(xhandle);
	if (retval) {
		com_err(__func__, retval,
			_("while reading xattrs of inode %u"), ino);
		goto xattrs_close;
	}
	retval = ext2fs_xattr_set(xhandle, name, value, value_len);
	if (retval) {
		com_err(__func__, retval,
			_("while setting xattrs of inode %u"), ino);
		goto xattrs_close;
	}
xattrs_close:
	close_retval = ext2fs_xattrs_close(&xhandle);
	if (close_retval) {
		com_err(__func__, close_retval,
			_("while closing xattrs of inode %u"), ino);
		return retval ? retval : close_retval;
	}
	return retval;
}

static errcode_t set_selinux_xattr(ext2_filsys fs, ext2_ino_t ino,
				   struct inode_params *params)
{
	errcode_t retval;
	char *secontext = NULL;
	struct ext2_inode inode;

	if (params->sehnd == NULL)
		return 0;

	retval = ext2fs_read_inode(fs, ino, &inode);
	if (retval) {
		com_err(__func__, retval,
			_("while reading inode %u"), ino);
		return retval;
	}

	retval = selabel_lookup(params->sehnd, &secontext, params->filename,
				inode.i_mode);
	if (retval < 0) {
		int saved_errno = errno;
		com_err(__func__, errno,
			_("searching for label \"%s\""), params->filename);
		return saved_errno;
	}

	retval = ino_add_xattr(fs, ino,  "security." XATTR_SELINUX_SUFFIX,
			       secontext, strlen(secontext) + 1);

	freecon(secontext);
	return retval;
}

/*
 * Returns mapped UID/GID if there is a corresponding entry in |mapping|.
 * Otherwise |id| as is.
 */
static unsigned int resolve_ugid(const struct ugid_map* mapping,
				 unsigned int id)
{
	size_t i;
	for (i = 0; i < mapping->size; ++i) {
		const struct ugid_map_entry* entry = &mapping->entries[i];
		if (entry->parent_id <= id &&
		    id < entry->parent_id + entry->length) {
			return id + entry->child_id - entry->parent_id;
		}
	}

	/* No entry is found. */
	return id;
}

static errcode_t set_perms_and_caps(ext2_filsys fs, ext2_ino_t ino,
				    struct inode_params *params)
{
	errcode_t retval;
	uint64_t capabilities = 0;
	struct ext2_inode inode;
	struct vfs_cap_data cap_data;
	unsigned int uid = 0, gid = 0, imode = 0;

	retval = ext2fs_read_inode(fs, ino, &inode);
	if (retval) {
		com_err(__func__, retval, _("while reading inode %u"), ino);
		return retval;
	}

	/* Permissions */
	if (params->fs_config_func != NULL) {
		const char *filename = params->filename;
		if (strcmp(filename, params->mountpoint) == 0) {
			/* The root of the filesystem needs to be an empty string. */
			filename = "";
		}
		params->fs_config_func(filename, S_ISDIR(inode.i_mode),
				       params->target_out, &uid, &gid, &imode,
				       &capabilities);
		uid = resolve_ugid(params->uid_map, uid);
		gid = resolve_ugid(params->gid_map, gid);
		inode.i_uid = (__u16) uid;
		inode.i_gid = (__u16) gid;
		ext2fs_set_i_uid_high(inode, (__u16) (uid >> 16));
		ext2fs_set_i_gid_high(inode, (__u16) (gid >> 16));
		inode.i_mode = (inode.i_mode & S_IFMT) | (imode & 0xffff);
		retval = ext2fs_write_inode(fs, ino, &inode);
		if (retval) {
			com_err(__func__, retval,
				_("while writing inode %u"), ino);
			return retval;
		}
	}

	/* Capabilities */
	if (!capabilities)
		return 0;
	memset(&cap_data, 0, sizeof(cap_data));
	cap_data.magic_etc = VFS_CAP_REVISION_2 | VFS_CAP_FLAGS_EFFECTIVE;
	cap_data.data[0].permitted = (uint32_t) (capabilities & 0xffffffff);
	cap_data.data[1].permitted = (uint32_t) (capabilities >> 32);
	return ino_add_xattr(fs, ino,  "security." XATTR_CAPS_SUFFIX,
			     &cap_data, sizeof(cap_data));
}

static errcode_t set_timestamp(ext2_filsys fs, ext2_ino_t ino,
			       struct inode_params *params)
{
	errcode_t retval;
	struct ext2_inode inode;
	struct stat stat;
	char *src_filename = NULL;

	retval = ext2fs_read_inode(fs, ino, &inode);
	if (retval) {
		com_err(__func__, retval,
			_("while reading inode %u"), ino);
		return retval;
	}

	if (params->fixed_time == -1 && params->src_dir) {
		/* replace mountpoint from filename with src_dir */
		if (asprintf(&src_filename, "%s/%s", params->src_dir,
			params->filename + strlen(params->mountpoint)) < 0) {
			return -ENOMEM;
		}
		retval = lstat(src_filename, &stat);
		if (retval < 0) {
			com_err(__func__, errno,
				_("while lstat file %s"), src_filename);
			goto end;
		}
		inode.i_atime = inode.i_ctime = inode.i_mtime = stat.st_mtime;
	} else {
		inode.i_atime = inode.i_ctime = inode.i_mtime = params->fixed_time;
	}

	retval = ext2fs_write_inode(fs, ino, &inode);
	if (retval) {
		com_err(__func__, retval,
			_("while writing inode %u"), ino);
		goto end;
	}

end:
	free(src_filename);
	return retval;
}

static int is_dir(ext2_filsys fs, ext2_ino_t ino)
{
	struct ext2_inode inode;

	if (ext2fs_read_inode(fs, ino, &inode))
		return 0;
	return S_ISDIR(inode.i_mode);
}

static errcode_t androidify_inode(ext2_filsys fs, ext2_ino_t ino,
				  struct inode_params *params)
{
	errcode_t retval;

	retval = set_timestamp(fs, ino, params);
	if (retval)
		return retval;

	retval = set_selinux_xattr(fs, ino, params);
	if (retval)
		return retval;

	return set_perms_and_caps(fs, ino, params);
}

static int walk_dir(ext2_ino_t dir EXT2FS_ATTR((unused)),
		    int flags EXT2FS_ATTR((unused)),
		    struct ext2_dir_entry *de,
		    int offset EXT2FS_ATTR((unused)),
		    int blocksize EXT2FS_ATTR((unused)),
		    char *buf EXT2FS_ATTR((unused)), void *priv_data)
{
	__u16 name_len;
	errcode_t retval;
	struct inode_params *params = (struct inode_params *)priv_data;

	name_len = de->name_len & 0xff;
	if (!strncmp(de->name, ".", name_len)
	    || (!strncmp(de->name, "..", name_len)))
		return 0;

	if (asprintf(&params->filename, "%s/%.*s", params->path, name_len,
		     de->name) < 0) {
		params->error = ENOMEM;
		return -ENOMEM;
        }

	if (!strncmp(de->name, "lost+found", 10)) {
		retval = set_selinux_xattr(params->fs, de->inode, params);
		if (retval)
			goto end;
	} else {
		retval = androidify_inode(params->fs, de->inode, params);
		if (retval)
			goto end;
		if (is_dir(params->fs, de->inode)) {
			char *cur_path = params->path;
			char *cur_filename = params->filename;
			params->path = params->filename;
			retval = ext2fs_dir_iterate2(params->fs, de->inode, 0, NULL,
						     walk_dir, params);
			if (retval)
				goto end;
			params->path = cur_path;
			params->filename = cur_filename;
		}
	}

end:
	free(params->filename);
	params->error |= retval;
	return retval;
}

errcode_t __android_configure_fs(ext2_filsys fs, char *src_dir,
				 char *target_out,
				 char *mountpoint,
				 fs_config_f fs_config_func,
				 struct selabel_handle *sehnd,
				 time_t fixed_time,
				 const struct ugid_map* uid_map,
				 const struct ugid_map* gid_map)
{
	errcode_t retval;
	struct inode_params params = {
		.fs = fs,
		.src_dir = src_dir,
		.target_out = target_out,
		.fs_config_func = fs_config_func,
		.sehnd = sehnd,
		.fixed_time = fixed_time,
		.path = mountpoint,
		.filename = mountpoint,
		.mountpoint = mountpoint,
		.uid_map = uid_map,
		.gid_map = gid_map,
		.error = 0
	};

	/* walk_dir will add the "/". Don't add it twice. */
	if (strlen(mountpoint) == 1 && mountpoint[0] == '/')
		params.path = "";

	retval = androidify_inode(fs, EXT2_ROOT_INO, &params);
	if (retval)
		return retval;

	retval = ext2fs_dir_iterate2(fs, EXT2_ROOT_INO, 0, NULL, walk_dir,
				     &params);
	if (retval)
		return retval;
	return params.error;
}

errcode_t android_configure_fs(ext2_filsys fs, char *src_dir, char *target_out,
			       char *mountpoint,
			       struct selinux_opt *seopts EXT2FS_ATTR((unused)),
			       unsigned int nopt EXT2FS_ATTR((unused)),
			       char *fs_config_file, time_t fixed_time,
			       const struct ugid_map* uid_map,
			       const struct ugid_map* gid_map)
{
	errcode_t retval;
	fs_config_f fs_config_func = NULL;
	struct selabel_handle *sehnd = NULL;

	/* Retrieve file contexts */
#if !defined(__ANDROID__)
	if (nopt > 0) {
		sehnd = selabel_open(SELABEL_CTX_FILE, seopts, nopt);
		if (!sehnd) {
			int saved_errno = errno;
			com_err(__func__, errno,
				_("while opening file contexts \"%s\""),
				seopts[0].value);
			return saved_errno;
		}
	}
#else
	sehnd = selinux_android_file_context_handle();
	if (!sehnd) {
		com_err(__func__, EINVAL,
			_("while opening android file_contexts"));
		return EINVAL;
	}
#endif

	/* Load the FS config */
	if (fs_config_file) {
		retval = load_canned_fs_config(fs_config_file);
		if (retval < 0) {
			com_err(__func__, retval,
				_("while loading fs_config \"%s\""),
				fs_config_file);
			return retval;
		}
		fs_config_func = canned_fs_config;
	} else if (mountpoint)
		fs_config_func = fs_config;

	return __android_configure_fs(fs, src_dir, target_out, mountpoint,
				      fs_config_func, sehnd, fixed_time,
				      uid_map, gid_map);
}
