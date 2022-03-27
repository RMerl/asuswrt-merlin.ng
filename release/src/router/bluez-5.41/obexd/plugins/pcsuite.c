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

#include "gdbus/gdbus.h"

#include "obexd/src/obexd.h"
#include "obexd/src/plugin.h"
#include "obexd/src/log.h"
#include "obexd/src/obex.h"
#include "obexd/src/mimetype.h"
#include "obexd/src/service.h"
#include "ftp.h"

#define PCSUITE_CHANNEL 24
#define PCSUITE_WHO_SIZE 8

#define PCSUITE_RECORD "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>	\
<record>								\
  <attribute id=\"0x0001\">						\
    <sequence>								\
      <uuid value=\"00005005-0000-1000-8000-0002ee000001\"/>		\
    </sequence>								\
  </attribute>								\
									\
  <attribute id=\"0x0004\">						\
    <sequence>								\
      <sequence>							\
        <uuid value=\"0x0100\"/>					\
      </sequence>							\
      <sequence>							\
        <uuid value=\"0x0003\"/>					\
        <uint8 value=\"%u\" name=\"channel\"/>				\
      </sequence>							\
      <sequence>							\
        <uuid value=\"0x0008\"/>					\
      </sequence>							\
    </sequence>								\
  </attribute>								\
									\
  <attribute id=\"0x0005\">						\
    <sequence>								\
      <uuid value=\"0x1002\"/>						\
    </sequence>								\
  </attribute>								\
									\
  <attribute id=\"0x0009\">						\
    <sequence>								\
      <sequence>							\
        <uuid value=\"00005005-0000-1000-8000-0002ee000001\"/>		\
        <uint16 value=\"0x0100\" name=\"version\"/>			\
      </sequence>							\
    </sequence>								\
  </attribute>								\
									\
  <attribute id=\"0x0100\">						\
    <text value=\"%s\" name=\"name\"/>					\
  </attribute>								\
</record>"

#define BACKUP_BUS_NAME		"com.nokia.backup.plugin"
#define BACKUP_PATH		"/com/nokia/backup"
#define BACKUP_PLUGIN_INTERFACE	"com.nokia.backup.plugin"
#define BACKUP_DBUS_TIMEOUT	(1000 * 60 * 15)

static const uint8_t FTP_TARGET[TARGET_SIZE] = {
			0xF9, 0xEC, 0x7B, 0xC4, 0x95, 0x3C, 0x11, 0xD2,
			0x98, 0x4E, 0x52, 0x54, 0x00, 0xDC, 0x9E, 0x09 };

static const uint8_t PCSUITE_WHO[PCSUITE_WHO_SIZE] = {
			'P', 'C', ' ', 'S', 'u', 'i', 't', 'e' };

struct pcsuite_session {
	struct ftp_session *ftp;
	char *lock_file;
	int fd;
};

static void *pcsuite_connect(struct obex_session *os, int *err)
{
	struct pcsuite_session *pcsuite;
	struct ftp_session *ftp;
	int fd;
	char *filename;

	DBG("");

	ftp = ftp_connect(os, err);
	if (ftp == NULL)
		return NULL;

	filename = g_build_filename(g_get_home_dir(), ".pcsuite", NULL);

	fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (fd < 0 && errno != EEXIST) {
		error("open(%s): %s(%d)", filename, strerror(errno), errno);
		goto fail;
	}

	/* Try to remove the file before retrying since it could be
	   that some process left/crash without removing it */
	if (fd < 0) {
		if (remove(filename) < 0) {
			error("remove(%s): %s(%d)", filename, strerror(errno),
									errno);
			goto fail;
		}

		fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0644);
		if (fd < 0) {
			error("open(%s): %s(%d)", filename, strerror(errno),
									errno);
			goto fail;
		}
	}

	DBG("%s created", filename);

	pcsuite = g_new0(struct pcsuite_session, 1);
	pcsuite->ftp = ftp;
	pcsuite->lock_file = filename;
	pcsuite->fd = fd;

	DBG("session %p created", pcsuite);

	if (err)
		*err = 0;

	return pcsuite;

fail:
	if (ftp)
		ftp_disconnect(os, ftp);
	if (err)
		*err = -errno;

	g_free(filename);

	return NULL;
}

static int pcsuite_get(struct obex_session *os, void *user_data)
{
	struct pcsuite_session *pcsuite = user_data;

	DBG("%p", pcsuite);

	return ftp_get(os, pcsuite->ftp);
}

static int pcsuite_chkput(struct obex_session *os, void *user_data)
{
	struct pcsuite_session *pcsuite = user_data;

	DBG("%p", pcsuite);

	return ftp_chkput(os, pcsuite->ftp);
}

static int pcsuite_put(struct obex_session *os, void *user_data)
{
	struct pcsuite_session *pcsuite = user_data;

	DBG("%p", pcsuite);

	return ftp_put(os, pcsuite->ftp);
}

static int pcsuite_setpath(struct obex_session *os, void *user_data)
{
	struct pcsuite_session *pcsuite = user_data;

	DBG("%p", pcsuite);

	return ftp_setpath(os, pcsuite->ftp);
}

static int pcsuite_action(struct obex_session *os, void *user_data)
{
	struct pcsuite_session *pcsuite = user_data;

	DBG("%p", pcsuite);

	return ftp_action(os, pcsuite->ftp);
}

static void pcsuite_disconnect(struct obex_session *os, void *user_data)
{
	struct pcsuite_session *pcsuite = user_data;

	DBG("%p", pcsuite);

	if (pcsuite->fd >= 0)
		close(pcsuite->fd);

	if (pcsuite->lock_file) {
		remove(pcsuite->lock_file);
		g_free(pcsuite->lock_file);
	}

	if (pcsuite->ftp)
		ftp_disconnect(os, pcsuite->ftp);

	g_free(pcsuite);
}

static struct obex_service_driver pcsuite = {
	.name = "Nokia OBEX PC Suite Services",
	.service = OBEX_PCSUITE,
	.channel = PCSUITE_CHANNEL,
	.secure = TRUE,
	.record = PCSUITE_RECORD,
	.target = FTP_TARGET,
	.target_size = TARGET_SIZE,
	.who = PCSUITE_WHO,
	.who_size = PCSUITE_WHO_SIZE,
	.connect = pcsuite_connect,
	.get = pcsuite_get,
	.put = pcsuite_put,
	.chkput = pcsuite_chkput,
	.setpath = pcsuite_setpath,
	.action = pcsuite_action,
	.disconnect = pcsuite_disconnect
};

struct backup_object {
	char *cmd;
	int fd;
	int oflag;
	int error_code;
	mode_t mode;
	DBusPendingCall *pending_call;
	DBusConnection *conn;
};

static void on_backup_dbus_notify(DBusPendingCall *pending_call,
					void *user_data)
{
	struct backup_object *obj = user_data;
	DBusMessage *reply;
	const char *filename;
	int error_code;

	DBG("Notification received for pending call - %s", obj->cmd);

	reply = dbus_pending_call_steal_reply(pending_call);

	if (reply && dbus_message_get_args(reply, NULL, DBUS_TYPE_INT32,
					&error_code, DBUS_TYPE_STRING,
					&filename, DBUS_TYPE_INVALID)) {

		obj->error_code = error_code;

		if (filename) {
			DBG("Notification - file path = %s, error_code = %d",
					filename, error_code);
			if (error_code == 0)
				obj->fd = open(filename,obj->oflag,obj->mode);
		}

	} else
		DBG("Notification timed out or connection got closed");

	if (reply)
		dbus_message_unref(reply);

	dbus_pending_call_unref(pending_call);
	obj->pending_call = NULL;
	dbus_connection_unref(obj->conn);
	obj->conn = NULL;

	if (obj->fd >= 0) {
		DBG("File opened, setting io flags, cmd = %s",
				obj->cmd);
		if (obj->oflag == O_RDONLY)
			obex_object_set_io_flags(user_data, G_IO_IN, 0);
		else
			obex_object_set_io_flags(user_data, G_IO_OUT, 0);
	} else {
		DBG("File open error, setting io error, cmd = %s",
				obj->cmd);
		obex_object_set_io_flags(user_data, G_IO_ERR, -EPERM);
	}
}

static gboolean send_backup_dbus_message(const char *oper,
					struct backup_object *obj,
					size_t *size)
{
	DBusConnection *conn;
	DBusMessage *msg;
	DBusPendingCall *pending_call;
	gboolean ret = FALSE;
	dbus_uint32_t file_size;

	file_size = size ? *size : 0;

	conn = g_dbus_setup_bus(DBUS_BUS_SESSION, NULL, NULL);

	if (conn == NULL)
		return FALSE;

	msg = dbus_message_new_method_call(BACKUP_BUS_NAME, BACKUP_PATH,
						BACKUP_PLUGIN_INTERFACE,
						"request");
	if (msg == NULL) {
		dbus_connection_unref(conn);
		return FALSE;
	}

	dbus_message_append_args(msg, DBUS_TYPE_STRING, &oper,
					DBUS_TYPE_STRING, &obj->cmd,
					DBUS_TYPE_INT32, &file_size,
					DBUS_TYPE_INVALID);

	if (strcmp(oper, "open") == 0) {
		ret = g_dbus_send_message_with_reply(conn, msg, &pending_call,
							BACKUP_DBUS_TIMEOUT);
		dbus_message_unref(msg);
		if (ret) {
			obj->conn = conn;
			obj->pending_call = pending_call;
			ret = dbus_pending_call_set_notify(pending_call,
							on_backup_dbus_notify,
							obj, NULL);
		} else
			dbus_connection_unref(conn);
	} else {
		g_dbus_send_message(conn, msg);
		dbus_connection_unref(conn);
	}

	return ret;
}

static void *backup_open(const char *name, int oflag, mode_t mode,
				void *context, size_t *size, int *err)
{
	struct backup_object *obj = g_new0(struct backup_object, 1);

	DBG("cmd = %s", name);

	obj->cmd = g_path_get_basename(name);
	obj->oflag = oflag;
	obj->mode = mode;
	obj->fd = -1;
	obj->pending_call = NULL;
	obj->conn = NULL;
	obj->error_code = 0;

	if (send_backup_dbus_message("open", obj, size) == FALSE) {
		g_free(obj);
		obj = NULL;
	}

	if (err)
		*err = 0;

	return obj;
}

static int backup_close(void *object)
{
	struct backup_object *obj = object;
	size_t size = 0;

	DBG("cmd = %s", obj->cmd);

	if (obj->fd != -1)
		close(obj->fd);

	if (obj->pending_call) {
		dbus_pending_call_cancel(obj->pending_call);
		dbus_pending_call_unref(obj->pending_call);
		dbus_connection_unref(obj->conn);
	}

	send_backup_dbus_message("close", obj, &size);

	g_free(obj->cmd);
	g_free(obj);

	return 0;
}

static ssize_t backup_read(void *object, void *buf, size_t count)
{
	struct backup_object *obj = object;
	ssize_t ret = 0;

	if (obj->pending_call) {
		DBG("cmd = %s, IN WAITING STAGE", obj->cmd);
		return -EAGAIN;
	}

	if (obj->fd != -1) {
		DBG("cmd = %s, READING DATA", obj->cmd);
		ret = read(obj->fd, buf, count);
		if (ret < 0)
			ret = -errno;
	} else {
		DBG("cmd = %s, PERMANENT FAILURE", obj->cmd);
		ret = obj->error_code ? -obj->error_code : -ENOENT;
	}

	return ret;
}

static ssize_t backup_write(void *object, const void *buf, size_t count)
{
	struct backup_object *obj = object;
	ssize_t ret = 0;

	if (obj->pending_call) {
		DBG("cmd = %s, IN WAITING STAGE", obj->cmd);
		return -EAGAIN;
	}

	if (obj->fd != -1) {
		ret = write(obj->fd, buf, count);

		DBG("cmd = %s, WRITTING", obj->cmd);

		if (ret < 0) {
			error("backup: cmd = %s", obj->cmd);
			ret = -errno;
		}
	} else {
		error("backup: cmd = %s", obj->cmd);
		ret = obj->error_code ? -obj->error_code : -ENOENT;
	}

	return ret;
}

static int backup_flush(void *object)
{
	DBG("%p", object);

	return 0;
}

static struct obex_mime_type_driver backup = {
	.target = FTP_TARGET,
	.target_size = TARGET_SIZE,
	.mimetype = "application/vnd.nokia-backup",
	.open = backup_open,
	.close = backup_close,
	.read = backup_read,
	.write = backup_write,
	.flush = backup_flush,
};

static int pcsuite_init(void)
{
	int err;

	err = obex_service_driver_register(&pcsuite);
	if (err < 0)
		return err;

	err = obex_mime_type_driver_register(&backup);
	if (err < 0)
		obex_service_driver_unregister(&pcsuite);

	return err;
}

static void pcsuite_exit(void)
{
	obex_mime_type_driver_unregister(&backup);
	obex_service_driver_unregister(&pcsuite);
}

OBEX_PLUGIN_DEFINE(pcsuite, pcsuite_init, pcsuite_exit)
