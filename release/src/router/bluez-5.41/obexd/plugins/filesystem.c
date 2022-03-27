/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2009-2010  Intel Corporation
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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <inttypes.h>

#include <glib.h>

#include "obexd/src/obexd.h"
#include "obexd/src/plugin.h"
#include "obexd/src/log.h"
#include "obexd/src/mimetype.h"
#include "filesystem.h"

#define EOL_CHARS "\n"

#define FL_VERSION "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" EOL_CHARS

#define FL_TYPE "<!DOCTYPE folder-listing SYSTEM \"obex-folder-listing.dtd\">" EOL_CHARS

#define FL_TYPE_PCSUITE "<!DOCTYPE folder-listing SYSTEM \"obex-folder-listing.dtd\"" EOL_CHARS \
			"  [ <!ATTLIST folder mem-type CDATA #IMPLIED> ]>" EOL_CHARS

#define FL_BODY_BEGIN "<folder-listing version=\"1.0\">" EOL_CHARS

#define FL_BODY_END "</folder-listing>" EOL_CHARS

#define FL_PARENT_FOLDER_ELEMENT "<parent-folder/>" EOL_CHARS

#define FL_FILE_ELEMENT "<file name=\"%s\" size=\"%" PRIu64 "\"" \
			" %s accessed=\"%s\" " \
			"modified=\"%s\" created=\"%s\"/>" EOL_CHARS

#define FL_FOLDER_ELEMENT "<folder name=\"%s\" %s accessed=\"%s\" " \
			"modified=\"%s\" created=\"%s\"/>" EOL_CHARS

#define FL_FOLDER_ELEMENT_PCSUITE "<folder name=\"%s\" %s accessed=\"%s\"" \
			" modified=\"%s\" mem-type=\"DEV\"" \
			" created=\"%s\"/>" EOL_CHARS

#define FTP_TARGET_SIZE 16

static const uint8_t FTP_TARGET[FTP_TARGET_SIZE] = {
			0xF9, 0xEC, 0x7B, 0xC4,  0x95, 0x3C, 0x11, 0xD2,
			0x98, 0x4E, 0x52, 0x54,  0x00, 0xDC, 0x9E, 0x09  };

#define PCSUITE_WHO_SIZE 8

static const uint8_t PCSUITE_WHO[PCSUITE_WHO_SIZE] = {
			'P', 'C', ' ', 'S', 'u', 'i', 't', 'e' };

gboolean is_filename(const char *name)
{
	if (strchr(name, '/'))
		return FALSE;

	if (strcmp(name, ".") == 0)
		return FALSE;

	if (strcmp(name, "..") == 0)
		return FALSE;

	return TRUE;
}

int verify_path(const char *path)
{
	char *t;
	int ret = 0;

	if (obex_option_symlinks())
		return 0;

	t = realpath(path, NULL);
	if (t == NULL)
		return -errno;

	if (!g_str_has_prefix(t, obex_option_root_folder()))
		ret = -EPERM;

	free(t);

	return ret;
}

static char *file_stat_line(char *filename, struct stat *fstat,
					struct stat *dstat, gboolean root,
					gboolean pcsuite)
{
	char perm[51], atime[18], ctime[18], mtime[18];
	char *escaped, *ret = NULL;

	snprintf(perm, 50, "user-perm=\"%s%s%s\" group-perm=\"%s%s%s\" "
			"other-perm=\"%s%s%s\"",
			(fstat->st_mode & S_IRUSR ? "R" : ""),
			(fstat->st_mode & S_IWUSR ? "W" : ""),
			(dstat->st_mode & S_IWUSR ? "D" : ""),
			(fstat->st_mode & S_IRGRP ? "R" : ""),
			(fstat->st_mode & S_IWGRP ? "W" : ""),
			(dstat->st_mode & S_IWGRP ? "D" : ""),
			(fstat->st_mode & S_IROTH ? "R" : ""),
			(fstat->st_mode & S_IWOTH ? "W" : ""),
			(dstat->st_mode & S_IWOTH ? "D" : ""));

	strftime(atime, 17, "%Y%m%dT%H%M%SZ", gmtime(&fstat->st_atime));
	strftime(ctime, 17, "%Y%m%dT%H%M%SZ", gmtime(&fstat->st_ctime));
	strftime(mtime, 17, "%Y%m%dT%H%M%SZ", gmtime(&fstat->st_mtime));

	escaped = g_markup_escape_text(filename, -1);

	if (S_ISDIR(fstat->st_mode)) {
		if (pcsuite && root && g_str_equal(filename, "Data"))
			ret = g_strdup_printf(FL_FOLDER_ELEMENT_PCSUITE,
						escaped, perm, atime,
						mtime, ctime);
		else
			ret = g_strdup_printf(FL_FOLDER_ELEMENT, escaped, perm,
							atime, mtime, ctime);
	} else if (S_ISREG(fstat->st_mode))
		ret = g_strdup_printf(FL_FILE_ELEMENT, escaped,
					(uint64_t) fstat->st_size,
					perm, atime, mtime, ctime);

	g_free(escaped);

	return ret;
}

static void *filesystem_open(const char *name, int oflag, mode_t mode,
					void *context, size_t *size, int *err)
{
	struct stat stats;
	struct statvfs buf;
	int fd, ret;
	uint64_t avail;

	fd = open(name, oflag, mode);
	if (fd < 0) {
		if (err)
			*err = -errno;
		return NULL;
	}

	if (fstat(fd, &stats) < 0) {
		if (err)
			*err = -errno;
		goto failed;
	}

	ret = verify_path(name);
	if (ret < 0) {
		if (err)
			*err = ret;
		goto failed;
	}

	if (oflag == O_RDONLY) {
		if (size)
			*size = stats.st_size;
		goto done;
	}

	if (fstatvfs(fd, &buf) < 0) {
		if (err)
			*err = -errno;
		goto failed;
	}

	if (size == NULL)
		goto done;

	avail = (uint64_t) buf.f_bsize * buf.f_bavail;
	if (avail < *size) {
		if (err)
			*err = -ENOSPC;
		goto failed;
	}

done:
	if (err)
		*err = 0;

	return GINT_TO_POINTER(fd);

failed:
	close(fd);
	return NULL;
}

static int filesystem_close(void *object)
{
	if (close(GPOINTER_TO_INT(object)) < 0)
		return -errno;

	return 0;
}

static ssize_t filesystem_read(void *object, void *buf, size_t count)
{
	ssize_t ret;

	ret = read(GPOINTER_TO_INT(object), buf, count);
	if (ret < 0)
		return -errno;

	return ret;
}

static ssize_t filesystem_write(void *object, const void *buf, size_t count)
{
	ssize_t ret;

	ret = write(GPOINTER_TO_INT(object), buf, count);
	if (ret < 0)
		return -errno;

	return ret;
}

static int filesystem_rename(const char *name, const char *destname)
{
	int ret;

	ret = rename(name, destname);
	if (ret < 0) {
		error("rename(%s, %s): %s (%d)", name, destname,
						strerror(errno), errno);
		return -errno;
	}

	return ret;
}

static int sendfile_async(int out_fd, int in_fd, off_t *offset, size_t count)
{
	int pid;

	/* Run sendfile on child process */
	pid = fork();
	switch (pid) {
		case 0:
			break;
		case -1:
			error("fork() %s (%d)", strerror(errno), errno);
			return -errno;
		default:
			DBG("child %d forked", pid);
			return pid;
	}

	/* At child */
	if (sendfile(out_fd, in_fd, offset, count) < 0)
		error("sendfile(): %s (%d)", strerror(errno), errno);

	close(in_fd);
	close(out_fd);

	exit(errno);
}

static int filesystem_copy(const char *name, const char *destname)
{
	void *in, *out;
	ssize_t ret;
	size_t size;
	struct stat st;
	int in_fd, out_fd, err;

	in = filesystem_open(name, O_RDONLY, 0, NULL, &size, &err);
	if (in == NULL) {
		error("open(%s): %s (%d)", name, strerror(-err), -err);
		return -err;
	}

	in_fd = GPOINTER_TO_INT(in);
	ret = fstat(in_fd, &st);
	if (ret < 0) {
		error("stat(%s): %s (%d)", name, strerror(errno), errno);
		return -errno;
	}

	out = filesystem_open(destname, O_WRONLY | O_CREAT | O_TRUNC,
					st.st_mode, NULL, &size, &err);
	if (out == NULL) {
		error("open(%s): %s (%d)", destname, strerror(-err), -err);
		filesystem_close(in);
		return -errno;
	}

	out_fd = GPOINTER_TO_INT(out);

	/* Check if sendfile is supported */
	ret = sendfile(out_fd, in_fd, NULL, 0);
	if (ret < 0) {
		ret = -errno;
		error("sendfile: %s (%zd)", strerror(-ret), -ret);
		goto done;
	}

	ret = sendfile_async(out_fd, in_fd, NULL, st.st_size);
	if (ret < 0)
		goto done;

	return 0;

done:
	filesystem_close(in);
	filesystem_close(out);

	return ret;
}

struct capability_object {
	int pid;
	int output;
	int err;
	gboolean aborted;
	GString *buffer;
};

static void script_exited(GPid pid, int status, void *data)
{
	struct capability_object *object = data;
	char buf[128];

	object->pid = -1;

	DBG("pid: %d status: %d", pid, status);

	g_spawn_close_pid(pid);

	/* free the object if aborted */
	if (object->aborted) {
		if (object->buffer != NULL)
			g_string_free(object->buffer, TRUE);

		g_free(object);
		return;
	}

	if (WEXITSTATUS(status) != EXIT_SUCCESS) {
		memset(buf, 0, sizeof(buf));
		if (read(object->err, buf, sizeof(buf)) > 0)
			error("%s", buf);
		obex_object_set_io_flags(data, G_IO_ERR, -EPERM);
	} else
		obex_object_set_io_flags(data, G_IO_IN, 0);
}

static int capability_exec(const char **argv, int *output, int *err)
{
	GError *gerr = NULL;
	int pid;
	GSpawnFlags flags = G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH;

	if (!g_spawn_async_with_pipes(NULL, (char **) argv, NULL, flags, NULL,
				NULL, &pid, NULL, output, err, &gerr)) {
		error("%s", gerr->message);
		g_error_free(gerr);
		return -EPERM;
	}

	DBG("executing %s pid %d", argv[0], pid);

	return pid;
}

static void *capability_open(const char *name, int oflag, mode_t mode,
					void *context, size_t *size, int *err)
{
	struct capability_object *object = NULL;
	char *buf;
	const char *argv[2];

	if (oflag != O_RDONLY)
		goto fail;

	object = g_new0(struct capability_object, 1);
	object->pid = -1;
	object->output = -1;
	object->err = -1;

	if (name[0] != '!') {
		GError *gerr = NULL;
		gboolean ret;

		ret = g_file_get_contents(name, &buf, NULL, &gerr);
		if (ret == FALSE) {
			error("%s", gerr->message);
			g_error_free(gerr);
			goto fail;
		}

		object->buffer = g_string_new(buf);

		if (size)
			*size = object->buffer->len;

		goto done;
	}

	argv[0] = &name[1];
	argv[1] = NULL;

	object->pid = capability_exec(argv, &object->output, &object->err);
	if (object->pid < 0)
		goto fail;

	/* Watch cannot be removed while the process is still running */
	g_child_watch_add(object->pid, script_exited, object);

done:
	if (err)
		*err = 0;

	return object;

fail:
	if (err)
		*err = -EPERM;

	g_free(object);
	return NULL;
}

static GString *append_pcsuite_preamble(GString *object)
{
	return g_string_append(object, FL_TYPE_PCSUITE);
}

static GString *append_folder_preamble(GString *object)
{
	return g_string_append(object, FL_TYPE);
}

static GString *append_listing(GString *object, const char *name,
				gboolean pcsuite, size_t *size, int *err)
{
	struct stat fstat, dstat;
	struct dirent *ep;
	DIR *dp;
	gboolean root;
	int ret;

	root = g_str_equal(name, obex_option_root_folder());

	dp = opendir(name);
	if (dp == NULL) {
		if (err)
			*err = -ENOENT;
		goto failed;
	}

	if (!root)
		object = g_string_append(object, FL_PARENT_FOLDER_ELEMENT);

	ret = verify_path(name);
	if (ret < 0) {
		*err = ret;
		goto failed;
	}

	ret = stat(name, &dstat);
	if (ret < 0) {
		if (err)
			*err = -errno;
		goto failed;
	}

	while ((ep = readdir(dp))) {
		char *filename;
		char *fullname;
		char *line;

		if (ep->d_name[0] == '.')
			continue;

		filename = g_filename_to_utf8(ep->d_name, -1, NULL, NULL, NULL);
		if (filename == NULL) {
			error("g_filename_to_utf8: invalid filename");
			continue;
		}

		fullname = g_build_filename(name, ep->d_name, NULL);

		ret = stat(fullname, &fstat);
		if (ret < 0) {
			DBG("stat: %s(%d)", strerror(errno), errno);
			g_free(filename);
			g_free(fullname);
			continue;
		}

		g_free(fullname);

		line = file_stat_line(filename, &fstat, &dstat, root, FALSE);
		if (line == NULL) {
			g_free(filename);
			continue;
		}

		g_free(filename);

		object = g_string_append(object, line);
		g_free(line);
	}

	closedir(dp);

	object = g_string_append(object, FL_BODY_END);
	if (size)
		*size = object->len;

	if (err)
		*err = 0;

	return object;

failed:
	if (dp)
		closedir(dp);

	g_string_free(object, TRUE);
	return NULL;
}

static void *folder_open(const char *name, int oflag, mode_t mode,
					void *context, size_t *size, int *err)
{
	GString *object;

	object = g_string_new(FL_VERSION);
	object = append_folder_preamble(object);
	object = g_string_append(object, FL_BODY_BEGIN);

	return append_listing(object, name, FALSE, size, err);
}

static void *pcsuite_open(const char *name, int oflag, mode_t mode,
					void *context, size_t *size, int *err)
{
	GString *object;

	object = g_string_new(FL_VERSION);
	object = append_pcsuite_preamble(object);
	object = g_string_append(object, FL_BODY_BEGIN);

	return append_listing(object, name, TRUE, size, err);
}

static int string_free(void *object)
{
	GString *string = object;

	g_string_free(string, TRUE);

	return 0;
}

ssize_t string_read(void *object, void *buf, size_t count)
{
	GString *string = object;
	ssize_t len;

	if (string->len == 0)
		return 0;

	len = MIN(string->len, count);
	memcpy(buf, string->str, len);
	g_string_erase(string, 0, len);

	return len;
}

static ssize_t folder_read(void *object, void *buf, size_t count)
{
	return string_read(object, buf, count);
}

static ssize_t capability_read(void *object, void *buf, size_t count)
{
	struct capability_object *obj = object;

	if (obj->buffer)
		return string_read(obj->buffer, buf, count);

	if (obj->pid >= 0)
		return -EAGAIN;

	return read(obj->output, buf, count);
}

static int capability_close(void *object)
{
	struct capability_object *obj = object;
	int err = 0;

	if (obj->pid < 0)
		goto done;

	DBG("kill: pid %d", obj->pid);
	err = kill(obj->pid, SIGTERM);
	if (err < 0) {
		err = -errno;
		error("kill: %s (%d)", strerror(-err), -err);
		goto done;
	}

	obj->aborted = TRUE;
	return 0;

done:
	if (obj->buffer != NULL)
		g_string_free(obj->buffer, TRUE);

	g_free(obj);

	return err;
}

static struct obex_mime_type_driver file = {
	.open = filesystem_open,
	.close = filesystem_close,
	.read = filesystem_read,
	.write = filesystem_write,
	.remove = remove,
	.move = filesystem_rename,
	.copy = filesystem_copy,
};

static struct obex_mime_type_driver capability = {
	.target = FTP_TARGET,
	.target_size = FTP_TARGET_SIZE,
	.mimetype = "x-obex/capability",
	.open = capability_open,
	.close = capability_close,
	.read = capability_read,
};

static struct obex_mime_type_driver folder = {
	.target = FTP_TARGET,
	.target_size = FTP_TARGET_SIZE,
	.mimetype = "x-obex/folder-listing",
	.open = folder_open,
	.close = string_free,
	.read = folder_read,
};

static struct obex_mime_type_driver pcsuite = {
	.target = FTP_TARGET,
	.target_size = FTP_TARGET_SIZE,
	.who = PCSUITE_WHO,
	.who_size = PCSUITE_WHO_SIZE,
	.mimetype = "x-obex/folder-listing",
	.open = pcsuite_open,
	.close = string_free,
	.read = folder_read,
};

static int filesystem_init(void)
{
	int err;

	err = obex_mime_type_driver_register(&folder);
	if (err < 0)
		return err;

	err = obex_mime_type_driver_register(&capability);
	if (err < 0)
		return err;

	err = obex_mime_type_driver_register(&pcsuite);
	if (err < 0)
		return err;

	return obex_mime_type_driver_register(&file);
}

static void filesystem_exit(void)
{
	obex_mime_type_driver_unregister(&folder);
	obex_mime_type_driver_unregister(&capability);
	obex_mime_type_driver_unregister(&file);
}

OBEX_PLUGIN_DEFINE(filesystem, filesystem_init, filesystem_exit)
