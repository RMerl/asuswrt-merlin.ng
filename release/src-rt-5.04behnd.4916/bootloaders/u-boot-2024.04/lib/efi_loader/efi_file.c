// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI_FILE_PROTOCOL
 *
 * Copyright (c) 2017 Rob Clark
 */

#include <charset.h>
#include <efi_loader.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <fs.h>
#include <part.h>

/* GUID for file system information */
const efi_guid_t efi_file_system_info_guid = EFI_FILE_SYSTEM_INFO_GUID;

/* GUID to obtain the volume label */
const efi_guid_t efi_system_volume_label_id = EFI_FILE_SYSTEM_VOLUME_LABEL_ID;

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
	u64 open_mode;

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
 * @open_mode:		bit mask indicating the access mode (read, write,
 *			create)
 * @attributes:		attributes for newly created file
 * Returns:		handle to the opened file or NULL
 */
static struct efi_file_handle *file_open(struct file_system *fs,
		struct file_handle *parent, u16 *file_name, u64 open_mode,
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
	if (!fh)
		return NULL;

	fh->open_mode = open_mode;
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
			if (!(open_mode & EFI_FILE_MODE_CREATE) ||
			    efi_create_file(fh, attributes))
				goto error;
			if (set_blk_dev(fh))
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

efi_status_t efi_file_open_int(struct efi_file_handle *this,
			       struct efi_file_handle **new_handle,
			       u16 *file_name, u64 open_mode,
			       u64 attributes)
{
	struct file_handle *fh = to_fh(this);
	efi_status_t ret;

	/* Check parameters */
	if (!this || !new_handle || !file_name) {
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
	return ret;
}

/**
 * efi_file_open_()
 *
 * This function implements the Open service of the File Protocol.
 * See the UEFI spec for details.
 *
 * @this:	EFI_FILE_PROTOCOL instance
 * @new_handle:	on return pointer to file handle
 * @file_name:	file name
 * @open_mode:	mode to open the file (read, read/write, create/read/write)
 * @attributes:	attributes for newly created file
 */
static efi_status_t EFIAPI efi_file_open(struct efi_file_handle *this,
					 struct efi_file_handle **new_handle,
					 u16 *file_name, u64 open_mode,
					 u64 attributes)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %p, \"%ls\", %llx, %llu", this, new_handle,
		  file_name, open_mode, attributes);

	ret = efi_file_open_int(this, new_handle, file_name, open_mode,
				attributes);

	return EFI_EXIT(ret);
}

/**
 * efi_file_open_ex() - open file asynchronously
 *
 * This function implements the OpenEx service of the File Protocol.
 * See the UEFI spec for details.
 *
 * @this:	EFI_FILE_PROTOCOL instance
 * @new_handle:	on return pointer to file handle
 * @file_name:	file name
 * @open_mode:	mode to open the file (read, read/write, create/read/write)
 * @attributes:	attributes for newly created file
 * @token:	transaction token
 */
static efi_status_t EFIAPI efi_file_open_ex(struct efi_file_handle *this,
					    struct efi_file_handle **new_handle,
					    u16 *file_name, u64 open_mode,
					    u64 attributes,
					    struct efi_file_io_token *token)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %p, \"%ls\", %llx, %llu, %p", this, new_handle,
		  file_name, open_mode, attributes, token);

	if (!token) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = efi_file_open_int(this, new_handle, file_name, open_mode,
				attributes);

	if (ret == EFI_SUCCESS && token->event) {
		token->status = EFI_SUCCESS;
		efi_signal_event(token->event);
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

efi_status_t efi_file_close_int(struct efi_file_handle *file)
{
	struct file_handle *fh = to_fh(file);

	return file_close(fh);
}

static efi_status_t EFIAPI efi_file_close(struct efi_file_handle *file)
{
	EFI_ENTRY("%p", file);
	return EFI_EXIT(efi_file_close_int(file));
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

/**
 * efi_get_file_size() - determine the size of a file
 *
 * @fh:		file handle
 * @file_size:	pointer to receive file size
 * Return:	status code
 */
static efi_status_t efi_get_file_size(struct file_handle *fh,
				      loff_t *file_size)
{
	if (set_blk_dev(fh))
		return EFI_DEVICE_ERROR;

	if (fs_size(fh->path, file_size))
		return EFI_DEVICE_ERROR;

	return EFI_SUCCESS;
}

/**
 * efi_file_size() - Get the size of a file using an EFI file handle
 *
 * @fh:		EFI file handle
 * @size:	buffer to fill in the discovered size
 *
 * Return:	size of the file
 */
efi_status_t efi_file_size(struct efi_file_handle *fh, efi_uintn_t *size)
{
	struct efi_file_info *info = NULL;
	efi_uintn_t bs = 0;
	efi_status_t ret;

	*size = 0;
	ret = EFI_CALL(fh->getinfo(fh, (efi_guid_t *)&efi_file_info_guid, &bs,
				   info));
	if (ret != EFI_BUFFER_TOO_SMALL) {
		ret = EFI_DEVICE_ERROR;
		goto out;
	}

	info = malloc(bs);
	if (!info) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	ret = EFI_CALL(fh->getinfo(fh, (efi_guid_t *)&efi_file_info_guid, &bs,
				   info));
	if (ret != EFI_SUCCESS)
		goto out;

	*size = info->file_size;

out:
	free(info);
	return ret;
}

static efi_status_t file_read(struct file_handle *fh, u64 *buffer_size,
		void *buffer)
{
	loff_t actread;
	efi_status_t ret;
	loff_t file_size;

	if (!buffer) {
		ret = EFI_INVALID_PARAMETER;
		return ret;
	}

	ret = efi_get_file_size(fh, &file_size);
	if (ret != EFI_SUCCESS)
		return ret;
	if (file_size < fh->offset) {
		ret = EFI_DEVICE_ERROR;
		return ret;
	}

	if (set_blk_dev(fh))
		return EFI_DEVICE_ERROR;
	if (fs_read(fh->path, map_to_sysmem(buffer), fh->offset,
		    *buffer_size, &actread))
		return EFI_DEVICE_ERROR;

	*buffer_size = actread;
	fh->offset += actread;

	return EFI_SUCCESS;
}

static void rtc2efi(struct efi_time *time, struct rtc_time *tm)
{
	memset(time, 0, sizeof(struct efi_time));
	time->year = tm->tm_year;
	time->month = tm->tm_mon;
	time->day = tm->tm_mday;
	time->hour = tm->tm_hour;
	time->minute = tm->tm_min;
	time->second = tm->tm_sec;
}

static efi_status_t dir_read(struct file_handle *fh, u64 *buffer_size,
		void *buffer)
{
	struct efi_file_info *info = buffer;
	struct fs_dirent *dent;
	u64 required_size;
	u16 *dst;

	if (set_blk_dev(fh))
		return EFI_DEVICE_ERROR;

	if (!fh->dirs) {
		assert(fh->offset == 0);
		fh->dirs = fs_opendir(fh->path);
		if (!fh->dirs)
			return EFI_DEVICE_ERROR;
		fh->dent = NULL;
	}

	/*
	 * So this is a bit awkward.  Since fs layer is stateful and we
	 * can't rewind an entry, in the EFI_BUFFER_TOO_SMALL case below
	 * we might have to return without consuming the dent.. so we
	 * have to stash it for next call.
	 */
	if (fh->dent) {
		dent = fh->dent;
	} else {
		dent = fs_readdir(fh->dirs);
	}

	if (!dent) {
		/* no more files in directory */
		*buffer_size = 0;
		return EFI_SUCCESS;
	}

	/* check buffer size: */
	required_size = sizeof(*info) +
			2 * (utf8_utf16_strlen(dent->name) + 1);
	if (*buffer_size < required_size) {
		*buffer_size = required_size;
		fh->dent = dent;
		return EFI_BUFFER_TOO_SMALL;
	}
	if (!buffer)
		return EFI_INVALID_PARAMETER;
	fh->dent = NULL;

	*buffer_size = required_size;
	memset(info, 0, required_size);

	info->size = required_size;
	info->file_size = dent->size;
	info->physical_size = dent->size;
	info->attribute = dent->attr;
	rtc2efi(&info->create_time, &dent->create_time);
	rtc2efi(&info->modification_time, &dent->change_time);
	rtc2efi(&info->last_access_time, &dent->access_time);

	if (dent->type == FS_DT_DIR)
		info->attribute |= EFI_FILE_DIRECTORY;

	dst = info->file_name;
	utf8_utf16_strcpy(&dst, dent->name);

	fh->offset++;

	return EFI_SUCCESS;
}

efi_status_t efi_file_read_int(struct efi_file_handle *this,
			       efi_uintn_t *buffer_size, void *buffer)
{
	struct file_handle *fh = to_fh(this);
	efi_status_t ret = EFI_SUCCESS;
	u64 bs;

	if (!this || !buffer_size)
		return EFI_INVALID_PARAMETER;

	bs = *buffer_size;
	if (fh->isdir)
		ret = dir_read(fh, &bs, buffer);
	else
		ret = file_read(fh, &bs, buffer);
	if (bs <= SIZE_MAX)
		*buffer_size = bs;
	else
		*buffer_size = SIZE_MAX;

	return ret;
}

/**
 * efi_file_read() - read file
 *
 * This function implements the Read() service of the EFI_FILE_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:		file protocol instance
 * @buffer_size:	number of bytes to read
 * @buffer:		read buffer
 * Return:		status code
 */
static efi_status_t EFIAPI efi_file_read(struct efi_file_handle *this,
					 efi_uintn_t *buffer_size, void *buffer)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %p, %p", this, buffer_size, buffer);

	ret = efi_file_read_int(this, buffer_size, buffer);

	return EFI_EXIT(ret);
}

/**
 * efi_file_read_ex() - read file asynchonously
 *
 * This function implements the ReadEx() service of the EFI_FILE_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:		file protocol instance
 * @token:		transaction token
 * Return:		status code
 */
static efi_status_t EFIAPI efi_file_read_ex(struct efi_file_handle *this,
					    struct efi_file_io_token *token)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %p", this, token);

	if (!token) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = efi_file_read_int(this, &token->buffer_size, token->buffer);

	if (ret == EFI_SUCCESS && token->event) {
		token->status = EFI_SUCCESS;
		efi_signal_event(token->event);
	}

out:
	return EFI_EXIT(ret);
}

static efi_status_t efi_file_write_int(struct efi_file_handle *this,
				       efi_uintn_t *buffer_size, void *buffer)
{
	struct file_handle *fh = to_fh(this);
	efi_status_t ret = EFI_SUCCESS;
	loff_t actwrite;

	if (!this || !buffer_size || !buffer) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	if (fh->isdir) {
		ret = EFI_UNSUPPORTED;
		goto out;
	}
	if (!(fh->open_mode & EFI_FILE_MODE_WRITE)) {
		ret = EFI_ACCESS_DENIED;
		goto out;
	}

	if (!*buffer_size)
		goto out;

	if (set_blk_dev(fh)) {
		ret = EFI_DEVICE_ERROR;
		goto out;
	}
	if (fs_write(fh->path, map_to_sysmem(buffer), fh->offset, *buffer_size,
		     &actwrite)) {
		ret = EFI_DEVICE_ERROR;
		goto out;
	}
	*buffer_size = actwrite;
	fh->offset += actwrite;

out:
	return ret;
}

/**
 * efi_file_write() - write to file
 *
 * This function implements the Write() service of the EFI_FILE_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:		file protocol instance
 * @buffer_size:	number of bytes to write
 * @buffer:		buffer with the bytes to write
 * Return:		status code
 */
static efi_status_t EFIAPI efi_file_write(struct efi_file_handle *this,
					  efi_uintn_t *buffer_size,
					  void *buffer)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %p, %p", this, buffer_size, buffer);

	ret = efi_file_write_int(this, buffer_size, buffer);

	return EFI_EXIT(ret);
}

/**
 * efi_file_write_ex() - write to file
 *
 * This function implements the WriteEx() service of the EFI_FILE_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:		file protocol instance
 * @token:		transaction token
 * Return:		status code
 */
static efi_status_t EFIAPI efi_file_write_ex(struct efi_file_handle *this,
					     struct efi_file_io_token *token)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %p", this, token);

	if (!token) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = efi_file_write_int(this, &token->buffer_size, token->buffer);

	if (ret == EFI_SUCCESS && token->event) {
		token->status = EFI_SUCCESS;
		efi_signal_event(token->event);
	}

out:
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

efi_status_t efi_file_setpos_int(struct efi_file_handle *file, u64 pos)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_SUCCESS;

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

		ret = efi_get_file_size(fh, &file_size);
		if (ret != EFI_SUCCESS)
			goto error;
		pos = file_size;
	}

	fh->offset = pos;

error:
	return ret;
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
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %llu", file, pos);

	ret = efi_file_setpos_int(file, pos);

	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI efi_file_getinfo(struct efi_file_handle *file,
					    const efi_guid_t *info_type,
					    efi_uintn_t *buffer_size,
					    void *buffer)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_SUCCESS;
	u16 *dst;

	EFI_ENTRY("%p, %pUs, %p, %p", file, info_type, buffer_size, buffer);

	if (!file || !info_type || !buffer_size ||
	    (*buffer_size && !buffer)) {
		ret = EFI_INVALID_PARAMETER;
		goto error;
	}

	if (!guidcmp(info_type, &efi_file_info_guid)) {
		struct efi_file_info *info = buffer;
		char *filename = basename(fh);
		unsigned int required_size;
		loff_t file_size;

		/* check buffer size: */
		required_size = sizeof(*info) +
				2 * (utf8_utf16_strlen(filename) + 1);
		if (*buffer_size < required_size) {
			*buffer_size = required_size;
			ret = EFI_BUFFER_TOO_SMALL;
			goto error;
		}

		ret = efi_get_file_size(fh, &file_size);
		if (ret != EFI_SUCCESS)
			goto error;

		memset(info, 0, required_size);

		info->size = required_size;
		info->file_size = file_size;
		info->physical_size = file_size;

		if (fh->isdir)
			info->attribute |= EFI_FILE_DIRECTORY;

		dst = info->file_name;
		utf8_utf16_strcpy(&dst, filename);
	} else if (!guidcmp(info_type, &efi_file_system_info_guid)) {
		struct efi_file_system_info *info = buffer;
		struct disk_partition part;
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
		required_size = sizeof(*info) + 2;
		if (*buffer_size < required_size) {
			*buffer_size = required_size;
			ret = EFI_BUFFER_TOO_SMALL;
			goto error;
		}

		memset(info, 0, required_size);

		info->size = required_size;
		/*
		 * TODO: We cannot determine if the volume can be written to.
		 */
		info->read_only = false;
		info->volume_size = part.size * part.blksz;
		/*
		 * TODO: We currently have no function to determine the free
		 * space. The volume size is the best upper bound we have.
		 */
		info->free_space = info->volume_size;
		info->block_size = part.blksz;
		/*
		 * TODO: The volume label is not available in U-Boot.
		 */
		info->volume_label[0] = 0;
	} else if (!guidcmp(info_type, &efi_system_volume_label_id)) {
		if (*buffer_size < 2) {
			*buffer_size = 2;
			ret = EFI_BUFFER_TOO_SMALL;
			goto error;
		}
		*(u16 *)buffer = 0;
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

	EFI_ENTRY("%p, %pUs, %zu, %p", file, info_type, buffer_size, buffer);

	if (!guidcmp(info_type, &efi_file_info_guid)) {
		struct efi_file_info *info = (struct efi_file_info *)buffer;
		char *filename = basename(fh);
		char *new_file_name, *pos;
		loff_t file_size;

		/* The buffer will always contain a file name. */
		if (buffer_size < sizeof(struct efi_file_info) + 2 ||
		    buffer_size < info->size) {
			ret = EFI_BAD_BUFFER_SIZE;
			goto out;
		}
		/* We cannot change the directory attribute */
		if (!fh->isdir != !(info->attribute & EFI_FILE_DIRECTORY)) {
			ret = EFI_ACCESS_DENIED;
			goto out;
		}
		/* Check for renaming */
		new_file_name = malloc(utf16_utf8_strlen(info->file_name) + 1);
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
		ret = efi_get_file_size(fh, &file_size);
		if (ret != EFI_SUCCESS)
			goto out;
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
	} else {
		/* TODO: We do not support changing the volume label */
		ret = EFI_UNSUPPORTED;
	}
out:
	return EFI_EXIT(ret);
}

/**
 * efi_file_flush_int() - flush file
 *
 * This is the internal implementation of the Flush() and FlushEx() services of
 * the EFI_FILE_PROTOCOL.
 *
 * @this:	file protocol instance
 * Return:	status code
 */
static efi_status_t efi_file_flush_int(struct efi_file_handle *this)
{
	struct file_handle *fh = to_fh(this);

	if (!this)
		return EFI_INVALID_PARAMETER;

	if (!(fh->open_mode & EFI_FILE_MODE_WRITE))
		return EFI_ACCESS_DENIED;

	/* TODO: flush for file position after end of file */
	return EFI_SUCCESS;
}

/**
 * efi_file_flush() - flush file
 *
 * This function implements the Flush() service of the EFI_FILE_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:	file protocol instance
 * Return:	status code
 */
static efi_status_t EFIAPI efi_file_flush(struct efi_file_handle *this)
{
	efi_status_t ret;

	EFI_ENTRY("%p", this);

	ret = efi_file_flush_int(this);

	return EFI_EXIT(ret);
}

/**
 * efi_file_flush_ex() - flush file
 *
 * This function implements the FlushEx() service of the EFI_FILE_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:	file protocol instance
 * @token:	transaction token
 * Return:	status code
 */
static efi_status_t EFIAPI efi_file_flush_ex(struct efi_file_handle *this,
					     struct efi_file_io_token *token)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %p", this, token);

	if (!token) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = efi_file_flush_int(this);

	if (ret == EFI_SUCCESS && token->event) {
		token->status = EFI_SUCCESS;
		efi_signal_event(token->event);
	}

out:
	return EFI_EXIT(ret);
}

static const struct efi_file_handle efi_file_handle_protocol = {
	.rev = EFI_FILE_PROTOCOL_REVISION2,
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
	.open_ex = efi_file_open_ex,
	.read_ex = efi_file_read_ex,
	.write_ex = efi_file_write_ex,
	.flush_ex = efi_file_flush_ex,
};

/**
 * efi_file_from_path() - open file via device path
 *
 * The device path @fp consists of the device path of the handle with the
 * simple file system protocol and one or more file path device path nodes.
 * The concatenation of all file path names provides the total file path.
 *
 * The code starts at the first file path node and tries to open that file or
 * directory. If there is a succeding file path node, the code opens it relative
 * to this directory and continues iterating until reaching the last file path
 * node.
 *
 * @fp:		device path
 * Return:	EFI_FILE_PROTOCOL for the file or NULL
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
		u16 *filename;
		size_t filename_sz;

		if (!EFI_DP_TYPE(fp, MEDIA_DEVICE, FILE_PATH)) {
			printf("bad file path!\n");
			EFI_CALL(f->close(f));
			return NULL;
		}

		/*
		 * UEFI specification requires pointers that are passed to
		 * protocol member functions to be aligned.  So memcpy it
		 * unconditionally
		 */
		if (fdp->dp.length <= offsetof(struct efi_device_path_file_path, str))
			return NULL;
		filename_sz = fdp->dp.length -
			offsetof(struct efi_device_path_file_path, str);
		filename = malloc(filename_sz);
		if (!filename)
			return NULL;
		memcpy(filename, fdp->str, filename_sz);
		EFI_CALL(ret = f->open(f, &f2, filename,
				       EFI_FILE_MODE_READ, 0));
		free(filename);
		if (ret != EFI_SUCCESS)
			return NULL;

		fp = efi_dp_next(fp);

		EFI_CALL(f->close(f));
		f = f2;
	}

	return f;
}

efi_status_t efi_open_volume_int(struct efi_simple_file_system_protocol *this,
				 struct efi_file_handle **root)
{
	struct file_system *fs = to_fs(this);

	*root = file_open(fs, NULL, NULL, 0, 0);

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI
efi_open_volume(struct efi_simple_file_system_protocol *this,
		struct efi_file_handle **root)
{
	EFI_ENTRY("%p, %p", this, root);

	return EFI_EXIT(efi_open_volume_int(this, root));
}

efi_status_t
efi_create_simple_file_system(struct blk_desc *desc, int part,
			      struct efi_device_path *dp,
			      struct efi_simple_file_system_protocol **fsp)
{
	struct file_system *fs;

	fs = calloc(1, sizeof(*fs));
	if (!fs)
		return EFI_OUT_OF_RESOURCES;
	fs->base.rev = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
	fs->base.open_volume = efi_open_volume;
	fs->desc = desc;
	fs->part = part;
	fs->dp = dp;
	*fsp = &fs->base;

	return EFI_SUCCESS;
}
