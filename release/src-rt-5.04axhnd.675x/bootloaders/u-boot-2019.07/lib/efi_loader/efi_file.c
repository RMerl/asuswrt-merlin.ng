// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI utils
 *
 *  Copyright (c) 2017 Rob Clark
 */

#include <common.h>
#include <charset.h>
#include <efi_loader.h>
#include <malloc.h>
#include <mapmem.h>
#include <fs.h>

/* GUID for file system information */
const efi_guid_t efi_file_system_info_guid = EFI_FILE_SYSTEM_INFO_GUID;

struct file_system {
	struct efi_simple_file_system_protocol base;
	struct efi_device_path *dp;
	struct blk_desc *desc;
	int part;
};
#define to_fs(x) container_of(x, struct file_system, base)

struct file_handle {
	struct efi_file_handle base;
	struct file_system *fs;
	loff_t offset;       /* current file position/cursor */
	int isdir;

	/* for reading a directory: */
	struct fs_dir_stream *dirs;
	struct fs_dirent *dent;

	char path[0];
};
#define to_fh(x) container_of(x, struct file_handle, base)

static const struct efi_file_handle efi_file_handle_protocol;

static char *basename(struct file_handle *fh)
{
	char *s = strrchr(fh->path, '/');
	if (s)
		return s + 1;
	return fh->path;
}

static int set_blk_dev(struct file_handle *fh)
{
	return fs_set_blk_dev_with_part(fh->fs->desc, fh->fs->part);
}

/**
 * is_dir() - check if file handle points to directory
 *
 * We assume that set_blk_dev(fh) has been called already.
 *
 * @fh:		file handle
 * Return:	true if file handle points to a directory
 */
static int is_dir(struct file_handle *fh)
{
	struct fs_dir_stream *dirs;

	dirs = fs_opendir(fh->path);
	if (!dirs)
		return 0;

	fs_closedir(dirs);

	return 1;
}

/*
 * Normalize a path which may include either back or fwd slashes,
 * double slashes, . or .. entries in the path, etc.
 */
static int sanitize_path(char *path)
{
	char *p;

	/* backslash to slash: */
	p = path;
	while ((p = strchr(p, '\\')))
		*p++ = '/';

	/* handle double-slashes: */
	p = path;
	while ((p = strstr(p, "//"))) {
		char *src = p + 1;
		memmove(p, src, strlen(src) + 1);
	}

	/* handle extra /.'s */
	p = path;
	while ((p = strstr(p, "/."))) {
		/*
		 * You'd be tempted to do this *after* handling ".."s
		 * below to avoid having to check if "/." is start of
		 * a "/..", but that won't have the correct results..
		 * for example, "/foo/./../bar" would get resolved to
		 * "/foo/bar" if you did these two passes in the other
		 * order
		 */
		if (p[2] == '.') {
			p += 2;
			continue;
		}
		char *src = p + 2;
		memmove(p, src, strlen(src) + 1);
	}

	/* handle extra /..'s: */
	p = path;
	while ((p = strstr(p, "/.."))) {
		char *src = p + 3;

		p--;

		/* find beginning of previous path entry: */
		while (true) {
			if (p < path)
				return -1;
			if (*p == '/')
				break;
			p--;
		}

		memmove(p, src, strlen(src) + 1);
	}

	return 0;
}

/**
 * efi_create_file() - create file or directory
 *
 * @fh:			file handle
 * @attributes:		attributes for newly created file
 * Returns:		0 for success
 */
static int efi_create_file(struct file_handle *fh, u64 attributes)
{
	loff_t actwrite;
	void *buffer = &actwrite;

	if (attributes & EFI_FILE_DIRECTORY)
		return fs_mkdir(fh->path);
	else
		return fs_write(fh->path, map_to_sysmem(buffer), 0, 0,
				&actwrite);
}

/**
 * file_open() - open a file handle
 *
 * @fs:			file system
 * @parent:		directory relative to which the file is to be opened
 * @file_name:		path of the file to be opened. '\', '.', or '..' may
 *			be used as modifiers. A leading backslash indicates an
 *			absolute path.
 * @mode:		bit mask indicating the access mode (read, write,
 *			create)
 * @attributes:		attributes for newly created file
 * Returns:		handle to the opened file or NULL
 */
static struct efi_file_handle *file_open(struct file_system *fs,
		struct file_handle *parent, u16 *file_name, u64 mode,
		u64 attributes)
{
	struct file_handle *fh;
	char f0[MAX_UTF8_PER_UTF16] = {0};
	int plen = 0;
	int flen = 0;

	if (file_name) {
		utf16_to_utf8((u8 *)f0, file_name, 1);
		flen = u16_strlen(file_name);
	}

	/* we could have a parent, but also an absolute path: */
	if (f0[0] == '\\') {
		plen = 0;
	} else if (parent) {
		plen = strlen(parent->path) + 1;
	}

	/* +2 is for null and '/' */
	fh = calloc(1, sizeof(*fh) + plen + (flen * MAX_UTF8_PER_UTF16) + 2);

	fh->base = efi_file_handle_protocol;
	fh->fs = fs;

	if (parent) {
		char *p = fh->path;
		int exists;

		if (plen > 0) {
			strcpy(p, parent->path);
			p += plen - 1;
			*p++ = '/';
		}

		utf16_to_utf8((u8 *)p, file_name, flen);

		if (sanitize_path(fh->path))
			goto error;

		/* check if file exists: */
		if (set_blk_dev(fh))
			goto error;

		exists = fs_exists(fh->path);
		/* fs_exists() calls fs_close(), so open file system again */
		if (set_blk_dev(fh))
			goto error;

		if (!exists) {
			if (!(mode & EFI_FILE_MODE_CREATE) ||
			    efi_create_file(fh, attributes))
				goto error;
		}

		/* figure out if file is a directory: */
		fh->isdir = is_dir(fh);
	} else {
		fh->isdir = 1;
		strcpy(fh->path, "");
	}

	return &fh->base;

error:
	free(fh);
	return NULL;
}

static efi_status_t EFIAPI efi_file_open(struct efi_file_handle *file,
		struct efi_file_handle **new_handle,
		u16 *file_name, u64 open_mode, u64 attributes)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret;

	EFI_ENTRY("%p, %p, \"%ls\", %llx, %llu", file, new_handle,
		  file_name, open_mode, attributes);

	/* Check parameters */
	if (!file || !new_handle || !file_name) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	if (open_mode != EFI_FILE_MODE_READ &&
	    open_mode != (EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE) &&
	    open_mode != (EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE |
			 EFI_FILE_MODE_CREATE)) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	/*
	 * The UEFI spec requires that attributes are only set in create mode.
	 * The SCT does not care about this and sets EFI_FILE_DIRECTORY in
	 * read mode. EDK2 does not check that attributes are zero if not in
	 * create mode.
	 *
	 * So here we only check attributes in create mode and do not check
	 * that they are zero otherwise.
	 */
	if ((open_mode & EFI_FILE_MODE_CREATE) &&
	    (attributes & (EFI_FILE_READ_ONLY | ~EFI_FILE_VALID_ATTR))) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Open file */
	*new_handle = file_open(fh->fs, fh, file_name, open_mode, attributes);
	if (*new_handle) {
		EFI_PRINT("file handle %p\n", *new_handle);
		ret = EFI_SUCCESS;
	} else {
		ret = EFI_NOT_FOUND;
	}
out:
	return EFI_EXIT(ret);
}

static efi_status_t file_close(struct file_handle *fh)
{
	fs_closedir(fh->dirs);
	free(fh);
	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_file_close(struct efi_file_handle *file)
{
	struct file_handle *fh = to_fh(file);
	EFI_ENTRY("%p", file);
	return EFI_EXIT(file_close(fh));
}

static efi_status_t EFIAPI efi_file_delete(struct efi_file_handle *file)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p", file);

	if (set_blk_dev(fh) || fs_unlink(fh->path))
		ret = EFI_WARN_DELETE_FAILURE;

	file_close(fh);
	return EFI_EXIT(ret);
}

static efi_status_t file_read(struct file_handle *fh, u64 *buffer_size,
		void *buffer)
{
	loff_t actread;

	if (fs_read(fh->path, map_to_sysmem(buffer), fh->offset,
		    *buffer_size, &actread))
		return EFI_DEVICE_ERROR;

	*buffer_size = actread;
	fh->offset += actread;

	return EFI_SUCCESS;
}

static efi_status_t dir_read(struct file_handle *fh, u64 *buffer_size,
		void *buffer)
{
	struct efi_file_info *info = buffer;
	struct fs_dirent *dent;
	unsigned int required_size;

	if (!fh->dirs) {
		assert(fh->offset == 0);
		fh->dirs = fs_opendir(fh->path);
		if (!fh->dirs)
			return EFI_DEVICE_ERROR;
	}

	/*
	 * So this is a bit awkward.  Since fs layer is stateful and we
	 * can't rewind an entry, in the EFI_BUFFER_TOO_SMALL case below
	 * we might have to return without consuming the dent.. so we
	 * have to stash it for next call.
	 */
	if (fh->dent) {
		dent = fh->dent;
		fh->dent = NULL;
	} else {
		dent = fs_readdir(fh->dirs);
	}


	if (!dent) {
		/* no more files in directory: */
		/* workaround shim.efi bug/quirk.. as find_boot_csv()
		 * loops through directory contents, it initially calls
		 * read w/ zero length buffer to find out how much mem
		 * to allocate for the EFI_FILE_INFO, then allocates,
		 * and then calls a 2nd time.  If we return size of
		 * zero the first time, it happily passes that to
		 * AllocateZeroPool(), and when that returns NULL it
		 * thinks it is EFI_OUT_OF_RESOURCES.  So on first
		 * call return a non-zero size:
		 */
		if (*buffer_size == 0)
			*buffer_size = sizeof(*info);
		else
			*buffer_size = 0;
		return EFI_SUCCESS;
	}

	/* check buffer size: */
	required_size = sizeof(*info) + 2 * (strlen(dent->name) + 1);
	if (*buffer_size < required_size) {
		*buffer_size = required_size;
		fh->dent = dent;
		return EFI_BUFFER_TOO_SMALL;
	}

	*buffer_size = required_size;
	memset(info, 0, required_size);

	info->size = required_size;
	info->file_size = dent->size;
	info->physical_size = dent->size;

	if (dent->type == FS_DT_DIR)
		info->attribute |= EFI_FILE_DIRECTORY;

	ascii2unicode(info->file_name, dent->name);

	fh->offset++;

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_file_read(struct efi_file_handle *file,
					 efi_uintn_t *buffer_size, void *buffer)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_SUCCESS;
	u64 bs;

	EFI_ENTRY("%p, %p, %p", file, buffer_size, buffer);

	if (!buffer_size || !buffer) {
		ret = EFI_INVALID_PARAMETER;
		goto error;
	}

	if (set_blk_dev(fh)) {
		ret = EFI_DEVICE_ERROR;
		goto error;
	}

	bs = *buffer_size;
	if (fh->isdir)
		ret = dir_read(fh, &bs, buffer);
	else
		ret = file_read(fh, &bs, buffer);
	if (bs <= SIZE_MAX)
		*buffer_size = bs;
	else
		*buffer_size = SIZE_MAX;

error:
	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI efi_file_write(struct efi_file_handle *file,
					  efi_uintn_t *buffer_size,
					  void *buffer)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_SUCCESS;
	loff_t actwrite;

	EFI_ENTRY("%p, %p, %p", file, buffer_size, buffer);

	if (set_blk_dev(fh)) {
		ret = EFI_DEVICE_ERROR;
		goto error;
	}

	if (fs_write(fh->path, map_to_sysmem(buffer), fh->offset, *buffer_size,
		     &actwrite)) {
		ret = EFI_DEVICE_ERROR;
		goto error;
	}

	*buffer_size = actwrite;
	fh->offset += actwrite;

error:
	return EFI_EXIT(ret);
}

/**
 * efi_file_getpos() - get current position in file
 *
 * This function implements the GetPosition service of the EFI file protocol.
 * See the UEFI spec for details.
 *
 * @file:	file handle
 * @pos:	pointer to file position
 * Return:	status code
 */
static efi_status_t EFIAPI efi_file_getpos(struct efi_file_handle *file,
					   u64 *pos)
{
	efi_status_t ret = EFI_SUCCESS;
	struct file_handle *fh = to_fh(file);

	EFI_ENTRY("%p, %p", file, pos);

	if (fh->isdir) {
		ret = EFI_UNSUPPORTED;
		goto out;
	}

	*pos = fh->offset;
out:
	return EFI_EXIT(ret);
}

/**
 * efi_file_setpos() - set current position in file
 *
 * This function implements the SetPosition service of the EFI file protocol.
 * See the UEFI spec for details.
 *
 * @file:	file handle
 * @pos:	new file position
 * Return:	status code
 */
static efi_status_t EFIAPI efi_file_setpos(struct efi_file_handle *file,
					   u64 pos)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %llu", file, pos);

	if (fh->isdir) {
		if (pos != 0) {
			ret = EFI_UNSUPPORTED;
			goto error;
		}
		fs_closedir(fh->dirs);
		fh->dirs = NULL;
	}

	if (pos == ~0ULL) {
		loff_t file_size;

		if (set_blk_dev(fh)) {
			ret = EFI_DEVICE_ERROR;
			goto error;
		}

		if (fs_size(fh->path, &file_size)) {
			ret = EFI_DEVICE_ERROR;
			goto error;
		}

		pos = file_size;
	}

	fh->offset = pos;

error:
	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI efi_file_getinfo(struct efi_file_handle *file,
					    const efi_guid_t *info_type,
					    efi_uintn_t *buffer_size,
					    void *buffer)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %pUl, %p, %p", file, info_type, buffer_size, buffer);

	if (!guidcmp(info_type, &efi_file_info_guid)) {
		struct efi_file_info *info = buffer;
		char *filename = basename(fh);
		unsigned int required_size;
		loff_t file_size;

		/* check buffer size: */
		required_size = sizeof(*info) + 2 * (strlen(filename) + 1);
		if (*buffer_size < required_size) {
			*buffer_size = required_size;
			ret = EFI_BUFFER_TOO_SMALL;
			goto error;
		}

		if (set_blk_dev(fh)) {
			ret = EFI_DEVICE_ERROR;
			goto error;
		}

		if (fs_size(fh->path, &file_size)) {
			ret = EFI_DEVICE_ERROR;
			goto error;
		}

		memset(info, 0, required_size);

		info->size = required_size;
		info->file_size = file_size;
		info->physical_size = file_size;

		if (fh->isdir)
			info->attribute |= EFI_FILE_DIRECTORY;

		ascii2unicode(info->file_name, filename);
	} else if (!guidcmp(info_type, &efi_file_system_info_guid)) {
		struct efi_file_system_info *info = buffer;
		disk_partition_t part;
		efi_uintn_t required_size;
		int r;

		if (fh->fs->part >= 1)
			r = part_get_info(fh->fs->desc, fh->fs->part, &part);
		else
			r = part_get_info_whole_disk(fh->fs->desc, &part);
		if (r < 0) {
			ret = EFI_DEVICE_ERROR;
			goto error;
		}
		required_size = sizeof(info) + 2 *
				(strlen((const char *)part.name) + 1);
		if (*buffer_size < required_size) {
			*buffer_size = required_size;
			ret = EFI_BUFFER_TOO_SMALL;
			goto error;
		}

		memset(info, 0, required_size);

		info->size = required_size;
		info->read_only = true;
		info->volume_size = part.size * part.blksz;
		info->free_space = 0;
		info->block_size = part.blksz;
		/*
		 * TODO: The volume label is not available in U-Boot.
		 * Use the partition name as substitute.
		 */
		ascii2unicode((u16 *)info->volume_label,
			      (const char *)part.name);
	} else {
		ret = EFI_UNSUPPORTED;
	}

error:
	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI efi_file_setinfo(struct efi_file_handle *file,
					    const efi_guid_t *info_type,
					    efi_uintn_t buffer_size,
					    void *buffer)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_UNSUPPORTED;

	EFI_ENTRY("%p, %pUl, %zu, %p", file, info_type, buffer_size, buffer);

	if (!guidcmp(info_type, &efi_file_info_guid)) {
		struct efi_file_info *info = (struct efi_file_info *)buffer;
		char *filename = basename(fh);
		char *new_file_name, *pos;
		loff_t file_size;

		if (buffer_size < sizeof(struct efi_file_info)) {
			ret = EFI_BAD_BUFFER_SIZE;
			goto out;
		}
		/* We cannot change the directory attribute */
		if (!fh->isdir != !(info->attribute & EFI_FILE_DIRECTORY)) {
			ret = EFI_ACCESS_DENIED;
			goto out;
		}
		/* Check for renaming */
		new_file_name = malloc(utf16_utf8_strlen(info->file_name));
		if (!new_file_name) {
			ret = EFI_OUT_OF_RESOURCES;
			goto out;
		}
		pos = new_file_name;
		utf16_utf8_strcpy(&pos, info->file_name);
		if (strcmp(new_file_name, filename)) {
			/* TODO: we do not support renaming */
			EFI_PRINT("Renaming not supported\n");
			free(new_file_name);
			ret = EFI_ACCESS_DENIED;
			goto out;
		}
		free(new_file_name);
		/* Check for truncation */
		if (set_blk_dev(fh)) {
			ret = EFI_DEVICE_ERROR;
			goto out;
		}
		if (fs_size(fh->path, &file_size)) {
			ret = EFI_DEVICE_ERROR;
			goto out;
		}
		if (file_size != info->file_size) {
			/* TODO: we do not support truncation */
			EFI_PRINT("Truncation not supported\n");
			ret = EFI_ACCESS_DENIED;
			goto out;
		}
		/*
		 * We do not care for the other attributes
		 * TODO: Support read only
		 */
		ret = EFI_SUCCESS;
	} else if (!guidcmp(info_type, &efi_file_system_info_guid)) {
		if (buffer_size < sizeof(struct efi_file_system_info)) {
			ret = EFI_BAD_BUFFER_SIZE;
			goto out;
		}
	} else {
		ret = EFI_UNSUPPORTED;
	}
out:
	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI efi_file_flush(struct efi_file_handle *file)
{
	EFI_ENTRY("%p", file);
	return EFI_EXIT(EFI_SUCCESS);
}

static const struct efi_file_handle efi_file_handle_protocol = {
	/*
	 * TODO: We currently only support EFI file protocol revision 0x00010000
	 *	 while UEFI specs 2.4 - 2.7 prescribe revision 0x00020000.
	 */
	.rev = EFI_FILE_PROTOCOL_REVISION,
	.open = efi_file_open,
	.close = efi_file_close,
	.delete = efi_file_delete,
	.read = efi_file_read,
	.write = efi_file_write,
	.getpos = efi_file_getpos,
	.setpos = efi_file_setpos,
	.getinfo = efi_file_getinfo,
	.setinfo = efi_file_setinfo,
	.flush = efi_file_flush,
};

/**
 * efi_file_from_path() - open file via device path
 *
 * @fp:		device path
 * @return:	EFI_FILE_PROTOCOL for the file or NULL
 */
struct efi_file_handle *efi_file_from_path(struct efi_device_path *fp)
{
	struct efi_simple_file_system_protocol *v;
	struct efi_file_handle *f;
	efi_status_t ret;

	v = efi_fs_from_path(fp);
	if (!v)
		return NULL;

	EFI_CALL(ret = v->open_volume(v, &f));
	if (ret != EFI_SUCCESS)
		return NULL;

	/* Skip over device-path nodes before the file path. */
	while (fp && !EFI_DP_TYPE(fp, MEDIA_DEVICE, FILE_PATH))
		fp = efi_dp_next(fp);

	/*
	 * Step through the nodes of the directory path until the actual file
	 * node is reached which is the final node in the device path.
	 */
	while (fp) {
		struct efi_device_path_file_path *fdp =
			container_of(fp, struct efi_device_path_file_path, dp);
		struct efi_file_handle *f2;

		if (!EFI_DP_TYPE(fp, MEDIA_DEVICE, FILE_PATH)) {
			printf("bad file path!\n");
			f->close(f);
			return NULL;
		}

		EFI_CALL(ret = f->open(f, &f2, fdp->str,
				       EFI_FILE_MODE_READ, 0));
		if (ret != EFI_SUCCESS)
			return NULL;

		fp = efi_dp_next(fp);

		EFI_CALL(f->close(f));
		f = f2;
	}

	return f;
}

static efi_status_t EFIAPI
efi_open_volume(struct efi_simple_file_system_protocol *this,
		struct efi_file_handle **root)
{
	struct file_system *fs = to_fs(this);

	EFI_ENTRY("%p, %p", this, root);

	*root = file_open(fs, NULL, NULL, 0, 0);

	return EFI_EXIT(EFI_SUCCESS);
}

struct efi_simple_file_system_protocol *
efi_simple_file_system(struct blk_desc *desc, int part,
		       struct efi_device_path *dp)
{
	struct file_system *fs;

	fs = calloc(1, sizeof(*fs));
	fs->base.rev = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
	fs->base.open_volume = efi_open_volume;
	fs->desc = desc;
	fs->part = part;
	fs->dp = dp;

	return &fs->base;
}
