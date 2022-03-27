/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Nokia Corporation
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <inttypes.h>

#include <glib.h>

#include "obexd/src/obexd.h"
#include "obexd/src/plugin.h"
#include "obexd/src/log.h"
#include "obexd/src/obex.h"
#include "obexd/src/manager.h"
#include "obexd/src/mimetype.h"
#include "obexd/src/service.h"
#include "ftp.h"
#include "filesystem.h"

#define LST_TYPE "x-obex/folder-listing"
#define CAP_TYPE "x-obex/capability"

static const uint8_t FTP_TARGET[TARGET_SIZE] = {
			0xF9, 0xEC, 0x7B, 0xC4, 0x95, 0x3C, 0x11, 0xD2,
			0x98, 0x4E, 0x52, 0x54, 0x00, 0xDC, 0x9E, 0x09 };

struct ftp_session {
	struct obex_session *os;
	struct obex_transfer *transfer;
	char *folder;
};

static void set_folder(struct ftp_session *ftp, const char *new_folder)
{
	DBG("%p folder %s", ftp, new_folder);

	g_free(ftp->folder);

	ftp->folder = new_folder ? g_strdup(new_folder) : NULL;
}

static int get_by_type(struct ftp_session *ftp, const char *type)
{
	struct obex_session *os = ftp->os;
	const char *capability = obex_option_capability();
	const char *name = obex_get_name(os);
	char *path;
	int err;

	DBG("%p name %s type %s", ftp, name, type);

	if (type == NULL && name == NULL)
		return -EBADR;

	if (type != NULL && g_ascii_strcasecmp(type, CAP_TYPE) == 0)
		return obex_get_stream_start(os, capability);

	if (name != NULL && !is_filename(name))
		return -EBADR;

	path = g_build_filename(ftp->folder, name, NULL);
	err = obex_get_stream_start(os, path);

	g_free(path);

	return err;
}

void *ftp_connect(struct obex_session *os, int *err)
{
	struct ftp_session *ftp;
	const char *root_folder;

	DBG("");

	root_folder = obex_option_root_folder();

	manager_register_session(os);

	ftp = g_new0(struct ftp_session, 1);
	set_folder(ftp, root_folder);
	ftp->os = os;

	if (err)
		*err = 0;

	ftp->transfer = manager_register_transfer(os);

	DBG("session %p created", ftp);

	return ftp;
}

int ftp_get(struct obex_session *os, void *user_data)
{
	struct ftp_session *ftp = user_data;
	const char *type = obex_get_type(os);
	int ret;

	DBG("%p", ftp);

	if (ftp->folder == NULL)
		return -ENOENT;

	ret = get_by_type(ftp, type);
	if (ret < 0)
		return ret;

	/* Only track progress of file transfer */
	if (type == NULL)
		manager_emit_transfer_started(ftp->transfer);

	return 0;
}

static int ftp_delete(struct ftp_session *ftp, const char *name)
{
	char *path;
	int ret = 0;

	DBG("%p name %s", ftp, name);

	if (!(ftp->folder && name))
		return -EINVAL;

	path = g_build_filename(ftp->folder, name, NULL);

	if (obex_remove(ftp->os, path) < 0)
		ret = -errno;

	g_free(path);

	return ret;
}

int ftp_chkput(struct obex_session *os, void *user_data)
{
	struct ftp_session *ftp = user_data;
	const char *name = obex_get_name(os);
	char *path;
	int ret;

	DBG("%p name %s", ftp, name);

	if (name == NULL)
		return -EBADR;

	if (!is_filename(name))
		return -EBADR;

	if (obex_get_size(os) == OBJECT_SIZE_DELETE)
		return 0;

	path = g_build_filename(ftp->folder, name, NULL);

	ret = obex_put_stream_start(os, path);

	if (ret == 0)
		manager_emit_transfer_started(ftp->transfer);

	g_free(path);

	return ret;
}

int ftp_put(struct obex_session *os, void *user_data)
{
	struct ftp_session *ftp = user_data;
	const char *name = obex_get_name(os);
	ssize_t size = obex_get_size(os);

	DBG("%p name %s size %zd", ftp, name, size);

	if (ftp->folder == NULL)
		return -EPERM;

	if (name == NULL)
		return -EBADR;

	if (!is_filename(name))
		return -EBADR;

	if (size == OBJECT_SIZE_DELETE)
		return ftp_delete(ftp, name);

	return 0;
}

int ftp_setpath(struct obex_session *os, void *user_data)
{
	struct ftp_session *ftp = user_data;
	const char *root_folder, *name;
	const uint8_t *nonhdr;
	char *fullname;
	struct stat dstat;
	gboolean root;
	int err;

	if (obex_get_non_header_data(os, &nonhdr) != 2) {
		error("Set path failed: flag and constants not found!");
		return -EBADMSG;
	}

	name = obex_get_name(os);
	root_folder = obex_option_root_folder();
	root = g_str_equal(root_folder, ftp->folder);

	DBG("%p name %s", ftp, name);

	/* Check flag "Backup" */
	if ((nonhdr[0] & 0x01) == 0x01) {
		DBG("Set to parent path");

		if (root)
			return -EPERM;

		fullname = g_path_get_dirname(ftp->folder);
		set_folder(ftp, fullname);
		g_free(fullname);

		DBG("Set to parent path: %s", ftp->folder);

		return 0;
	}

	if (!name) {
		DBG("Set path failed: name missing!");
		return -EINVAL;
	}

	if (strlen(name) == 0) {
		DBG("Set to root");
		set_folder(ftp, root_folder);
		return 0;
	}

	/* Check and set to name path */
	if (!is_filename(name)) {
		error("Set path failed: name incorrect!");
		return -EPERM;
	}

	fullname = g_build_filename(ftp->folder, name, NULL);

	DBG("Fullname: %s", fullname);

	err = verify_path(fullname);
	if (err == -ENOENT)
		goto not_found;

	if (err < 0)
		goto done;

	err = stat(fullname, &dstat);

	if (err < 0) {
		err = -errno;

		if (err == -ENOENT)
			goto not_found;

		DBG("stat: %s(%d)", strerror(-err), -err);

		goto done;
	}

	if (S_ISDIR(dstat.st_mode) && (dstat.st_mode & S_IRUSR) &&
						(dstat.st_mode & S_IXUSR)) {
		set_folder(ftp, fullname);
		goto done;
	}

	err = -EPERM;
	goto done;

not_found:
	if (nonhdr[0] != 0) {
		err = -ENOENT;
		goto done;
	}

	if (mkdir(fullname, 0755) <  0) {
		err = -errno;
		DBG("mkdir: %s(%d)", strerror(-err), -err);
		goto done;
	}

	err = 0;
	set_folder(ftp, fullname);

done:
	g_free(fullname);
	return err;
}

static gboolean is_valid_path(const char *path)
{
	char **elements, **cur;
	int depth = 0;

	elements = g_strsplit(path, "/", 0);

	for (cur = elements; *cur != NULL; cur++) {
		if (**cur == '\0' || strcmp(*cur, ".") == 0)
			continue;

		if (strcmp(*cur, "..") == 0) {
			depth--;
			if (depth < 0)
				break;
			continue;
		}

		depth++;
	}

	g_strfreev(elements);

	if (depth < 0)
		return FALSE;

	return TRUE;
}

static char *ftp_build_filename(struct ftp_session *ftp, const char *destname)
{
	char *filename;

	/* DestName can either be relative or absolute (FTP style) */
	if (destname[0] == '/')
		filename = g_build_filename(obex_option_root_folder(),
                                                                destname, NULL);
	else
		filename = g_build_filename(ftp->folder, destname, NULL);

	if (is_valid_path(filename + strlen(obex_option_root_folder())))
		return filename;

	g_free(filename);

	return NULL;
}

static int ftp_copy(struct ftp_session *ftp, const char *name,
							const char *destname)
{
	char *source, *destination, *destdir;
	int ret;

	DBG("%p name %s destination %s", ftp, name, destname);

	if (ftp->folder == NULL) {
		error("No folder set");
		return -ENOENT;
	}

	if (name == NULL || destname == NULL)
		return -EINVAL;

	destination = ftp_build_filename(ftp, destname);

	if (destination == NULL)
		return -EBADR;

	destdir = g_path_get_dirname(destination);
	ret = verify_path(destdir);
	g_free(destdir);

	if (ret < 0)
		return ret;

	source = g_build_filename(ftp->folder, name, NULL);

	ret = obex_copy(ftp->os, source, destination);

	g_free(source);
	g_free(destination);

	return ret;
}

static int ftp_move(struct ftp_session *ftp, const char *name,
							const char *destname)
{
	char *source, *destination, *destdir;
	int ret;

	DBG("%p name %s destname %s", ftp, name, destname);

	if (ftp->folder == NULL) {
		error("No folder set");
		return -ENOENT;
	}

	if (name == NULL || destname == NULL)
		return -EINVAL;

	destination = ftp_build_filename(ftp, destname);

	if (destination == NULL)
		return -EBADR;

	destdir = g_path_get_dirname(destination);
	ret = verify_path(destdir);
	g_free(destdir);

	if (ret < 0)
		return ret;

	source = g_build_filename(ftp->folder, name, NULL);

	ret = obex_move(ftp->os, source, destination);

	g_free(source);
	g_free(destination);

	return ret;
}

int ftp_action(struct obex_session *os, void *user_data)
{
	struct ftp_session *ftp = user_data;
	const char *name, *destname;
	uint8_t action_id;

	name = obex_get_name(os);
	if (name == NULL || !is_filename(name))
		return -EBADR;

	destname = obex_get_destname(os);
	action_id = obex_get_action_id(os);

	DBG("%p action 0x%x", ftp, action_id);

	switch (action_id) {
	case 0x00: /* Copy Object */
		return ftp_copy(ftp, name, destname);
	case 0x01: /* Move/Rename Object */
		return ftp_move(ftp, name, destname);
	default:
		return -ENOSYS;
	}
}

void ftp_disconnect(struct obex_session *os, void *user_data)
{
	struct ftp_session *ftp = user_data;

	DBG("%p", ftp);

	manager_unregister_session(os);

	manager_unregister_transfer(ftp->transfer);

	g_free(ftp->folder);
	g_free(ftp);
}

static void ftp_progress(struct obex_session *os, void *user_data)
{
	struct ftp_session *ftp = user_data;

	manager_emit_transfer_progress(ftp->transfer);
}

static void ftp_reset(struct obex_session *os, void *user_data)
{
	struct ftp_session *ftp = user_data;

	manager_emit_transfer_completed(ftp->transfer);
}

static struct obex_service_driver ftp = {
	.name = "File Transfer server",
	.service = OBEX_FTP,
	.target = FTP_TARGET,
	.target_size = TARGET_SIZE,
	.connect = ftp_connect,
	.progress = ftp_progress,
	.get = ftp_get,
	.put = ftp_put,
	.chkput = ftp_chkput,
	.setpath = ftp_setpath,
	.action = ftp_action,
	.disconnect = ftp_disconnect,
	.reset = ftp_reset
};

static int ftp_init(void)
{
	return obex_service_driver_register(&ftp);
}

static void ftp_exit(void)
{
	obex_service_driver_unregister(&ftp);
}

OBEX_PLUGIN_DEFINE(ftp, ftp_init, ftp_exit)
