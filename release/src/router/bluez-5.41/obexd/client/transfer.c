/*
 *
 *  OBEX Client
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2011-2012  BMW Car IT GmbH. All rights reserved.
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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>

#include <glib.h>

#include "gdbus/gdbus.h"
#include "gobex/gobex.h"

#include "obexd/src/log.h"
#include "dbus.h"
#include "transfer.h"

#define TRANSFER_INTERFACE "org.bluez.obex.Transfer1"
#define ERROR_INTERFACE "org.bluez.obex.Error"

#define OBC_TRANSFER_ERROR obc_transfer_error_quark()

#define FIRST_PACKET_TIMEOUT 60

static guint64 counter = 0;

struct transfer_callback {
	transfer_callback_t func;
	void *data;
};

enum {
	TRANSFER_STATUS_QUEUED = 0,
	TRANSFER_STATUS_ACTIVE,
	TRANSFER_STATUS_SUSPENDED,
	TRANSFER_STATUS_COMPLETE,
	TRANSFER_STATUS_SUSPENDED_QUEUED,
	TRANSFER_STATUS_ERROR
};

struct obc_transfer {
	GObex *obex;
	uint8_t status;
	GObexApparam *apparam;
	guint8 op;
	struct transfer_callback *callback;
	DBusConnection *conn;
	DBusMessage *msg;
	char *session;		/* Session path */
	char *owner;		/* Transfer initiator */
	char *path;		/* Transfer path */
	char *filename;		/* Transfer file location */
	char *name;		/* Transfer object name */
	char *type;		/* Transfer object type */
	int fd;
	guint req;
	guint xfer;
	gint64 size;
	gint64 transferred;
	gint64 progress;
	guint progress_id;
};

static GQuark obc_transfer_error_quark(void)
{
	return g_quark_from_static_string("obc-transfer-error-quark");
}

DBusMessage *obc_transfer_create_dbus_reply(struct obc_transfer *transfer,
							DBusMessage *message)
{
	DBusMessage *reply;
	DBusMessageIter iter;

	reply = dbus_message_new_method_return(message);
	if (reply == NULL)
		return NULL;

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH,
							&transfer->path);
	g_dbus_get_properties(transfer->conn, transfer->path,
						TRANSFER_INTERFACE, &iter);

	return reply;
}

static void abort_complete(GObex *obex, GError *err, gpointer user_data)
{
	struct obc_transfer *transfer = user_data;
	struct transfer_callback *callback = transfer->callback;
	DBusMessage *reply;

	transfer->xfer = 0;

	reply = dbus_message_new_method_return(transfer->msg);
	if (reply)
		g_dbus_send_message(transfer->conn, reply);

	dbus_message_unref(transfer->msg);
	transfer->msg = NULL;

	if (callback == NULL)
		return;

	if (err) {
		callback->func(transfer, err, callback->data);
	} else {
		GError *abort_err;

		abort_err = g_error_new(OBC_TRANSFER_ERROR, -ECANCELED, "%s",
						"Transfer cancelled by user");
		callback->func(transfer, abort_err, callback->data);
		g_error_free(abort_err);
	}
}

static DBusMessage *obc_transfer_cancel(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	struct obc_transfer *transfer = user_data;
	const char *sender;

	sender = dbus_message_get_sender(message);
	if (g_strcmp0(transfer->owner, sender) != 0)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".NotAuthorized",
				"Not Authorized");

	if (transfer->msg != NULL)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InProgress",
				"Cancellation already in progress");

	if (transfer->status == TRANSFER_STATUS_SUSPENDED)
		g_obex_resume(transfer->obex);

	if (transfer->req > 0) {
		if (!g_obex_cancel_req(transfer->obex, transfer->req, TRUE))
			return g_dbus_create_error(message,
						ERROR_INTERFACE ".Failed",
						"Failed");
		transfer->req = 0;
	}

	if (transfer->xfer == 0) {
		struct transfer_callback *callback = transfer->callback;

		if (callback != NULL) {
			GError *err;

			err = g_error_new(OBC_TRANSFER_ERROR, -ECANCELED, "%s",
						"Transfer cancelled by user");
			callback->func(transfer, err, callback->data);
			g_error_free(err);
		}

		return dbus_message_new_method_return(message);
	}

	if (transfer->progress_id != 0) {
		g_source_remove(transfer->progress_id);
		transfer->progress_id = 0;
	}

	if (!g_obex_cancel_transfer(transfer->xfer, abort_complete, transfer))
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".Failed",
				"Failed");

	transfer->msg = dbus_message_ref(message);

	return NULL;
}

static void transfer_set_status(struct obc_transfer *transfer, uint8_t status)
{
	if (transfer->status == status)
		return;

	transfer->status = status;

	g_dbus_emit_property_changed(transfer->conn, transfer->path,
					TRANSFER_INTERFACE, "Status");
}

static DBusMessage *obc_transfer_suspend(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	struct obc_transfer *transfer = user_data;
	const char *sender;
	uint8_t status;

	sender = dbus_message_get_sender(message);
	if (g_strcmp0(transfer->owner, sender) != 0)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".NotAuthorized",
				"Not Authorized");

	switch (transfer->status) {
	case TRANSFER_STATUS_QUEUED:
		status = TRANSFER_STATUS_SUSPENDED_QUEUED;
		break;
	case TRANSFER_STATUS_ACTIVE:
		if (transfer->xfer)
			g_obex_suspend(transfer->obex);
		status = TRANSFER_STATUS_SUSPENDED;
		break;
	default:
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".NotInProgress",
				"Not in progress");
	}

	transfer_set_status(transfer, status);

	return g_dbus_create_reply(message, DBUS_TYPE_INVALID);
}

static DBusMessage *obc_transfer_resume(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	struct obc_transfer *transfer = user_data;
	const char *sender;
	uint8_t status;

	sender = dbus_message_get_sender(message);
	if (g_strcmp0(transfer->owner, sender) != 0)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".NotAuthorized",
				"Not Authorized");

	switch (transfer->status) {
	case TRANSFER_STATUS_SUSPENDED_QUEUED:
		status = TRANSFER_STATUS_QUEUED;
		break;
	case TRANSFER_STATUS_SUSPENDED:
		if (transfer->xfer)
			g_obex_resume(transfer->obex);
		else
			obc_transfer_start(transfer, NULL, NULL);
		status = TRANSFER_STATUS_ACTIVE;
		break;
	default:
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".NotInProgress",
				"Not in progress");
	}

	transfer_set_status(transfer, status);

	return g_dbus_create_reply(message, DBUS_TYPE_INVALID);
}

static gboolean name_exists(const GDBusPropertyTable *property, void *data)
{
	struct obc_transfer *transfer = data;

	return transfer->name != NULL;
}

static gboolean get_name(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obc_transfer *transfer = data;

	if (transfer->name == NULL)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
							&transfer->name);

	return TRUE;
}

static gboolean get_size(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obc_transfer *transfer = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64,
							&transfer->size);

	return TRUE;
}

static gboolean filename_exists(const GDBusPropertyTable *property, void *data)
{
	struct obc_transfer *transfer = data;

	return transfer->filename != NULL;
}

static gboolean get_filename(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obc_transfer *transfer = data;

	if (transfer->filename == NULL)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
							&transfer->filename);

	return TRUE;
}

static gboolean transferred_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct obc_transfer *transfer = data;

	return transfer->obex != NULL;
}

static gboolean get_transferred(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obc_transfer *transfer = data;

	if (transfer->obex == NULL)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64,
							&transfer->progress);

	return TRUE;
}

static const char *status2str(uint8_t status)
{
	switch (status) {
	case TRANSFER_STATUS_QUEUED:
		return "queued";
	case TRANSFER_STATUS_ACTIVE:
		return "active";
	case TRANSFER_STATUS_SUSPENDED_QUEUED:
	case TRANSFER_STATUS_SUSPENDED:
		return "suspended";
	case TRANSFER_STATUS_COMPLETE:
		return "complete";
	case TRANSFER_STATUS_ERROR:
	default:
		return "error";
	}
}

static gboolean get_status(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obc_transfer *transfer = data;
	const char *status = status2str(transfer->status);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &status);

	return TRUE;
}

static gboolean get_session(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obc_transfer *transfer = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH,
							&transfer->session);

	return TRUE;
}

static const GDBusMethodTable obc_transfer_methods[] = {
	{ GDBUS_METHOD("Suspend", NULL, NULL, obc_transfer_suspend) },
	{ GDBUS_METHOD("Resume", NULL, NULL, obc_transfer_resume) },
	{ GDBUS_ASYNC_METHOD("Cancel", NULL, NULL,
				obc_transfer_cancel) },
	{ }
};

static const GDBusPropertyTable obc_transfer_properties[] = {
	{ "Status", "s", get_status },
	{ "Name", "s", get_name, NULL, name_exists },
	{ "Size", "t", get_size },
	{ "Filename", "s", get_filename, NULL, filename_exists },
	{ "Transferred", "t", get_transferred, NULL, transferred_exists },
	{ "Session", "o", get_session },
	{ }
};

static void obc_transfer_free(struct obc_transfer *transfer)
{
	DBG("%p", transfer);

	if (transfer->status == TRANSFER_STATUS_SUSPENDED)
		g_obex_resume(transfer->obex);

	if (transfer->req > 0)
		g_obex_cancel_req(transfer->obex, transfer->req, TRUE);

	if (transfer->xfer)
		g_obex_cancel_transfer(transfer->xfer, NULL, NULL);

	if (transfer->progress_id != 0) {
		g_source_remove(transfer->progress_id);
		transfer->progress_id = 0;
	}

	if (transfer->op == G_OBEX_OP_GET &&
				transfer->status != TRANSFER_STATUS_COMPLETE &&
				transfer->filename)
		remove(transfer->filename);

	if (transfer->fd > 0)
		close(transfer->fd);

	if (transfer->apparam != NULL)
		g_obex_apparam_free(transfer->apparam);

	if (transfer->conn)
		dbus_connection_unref(transfer->conn);

	if (transfer->msg)
		dbus_message_unref(transfer->msg);

	if (transfer->obex)
		g_obex_unref(transfer->obex);

	g_free(transfer->callback);
	g_free(transfer->owner);
	g_free(transfer->filename);
	g_free(transfer->name);
	g_free(transfer->type);
	g_free(transfer->session);
	g_free(transfer->path);
	g_free(transfer);
}

static struct obc_transfer *obc_transfer_create(guint8 op,
						const char *filename,
						const char *name,
						const char *type)
{
	struct obc_transfer *transfer;

	transfer = g_new0(struct obc_transfer, 1);
	transfer->op = op;
	transfer->filename = g_strdup(filename);
	transfer->name = g_strdup(name);
	transfer->type = g_strdup(type);

	return transfer;
}

gboolean obc_transfer_register(struct obc_transfer *transfer,
						DBusConnection *conn,
						const char *session,
						const char *owner,
						GError **err)
{
	transfer->owner = g_strdup(owner);

	transfer->session = g_strdup(session);
	transfer->path = g_strdup_printf("%s/transfer%ju", session, counter++);

	transfer->conn = dbus_connection_ref(conn);
	if (transfer->conn == NULL) {
		g_set_error(err, OBC_TRANSFER_ERROR, -EFAULT,
						"Unable to connect to D-Bus");
		return FALSE;
	}

	if (g_dbus_register_interface(transfer->conn, transfer->path,
				TRANSFER_INTERFACE,
				obc_transfer_methods, NULL,
				obc_transfer_properties, transfer,
				NULL) == FALSE) {
		g_set_error(err, OBC_TRANSFER_ERROR, -EFAULT,
						"Unable to register to D-Bus");
		return FALSE;
	}

	DBG("%p registered %s", transfer, transfer->path);

	return TRUE;
}

static gboolean transfer_open(struct obc_transfer *transfer, int flags,
						mode_t mode, GError **err)
{
	int fd;
	char *filename;

	if (transfer->filename != NULL && strcmp(transfer->filename, "") != 0) {
		fd = open(transfer->filename, flags, mode);
		if (fd < 0) {
			error("open(): %s(%d)", strerror(errno), errno);
			g_set_error(err, OBC_TRANSFER_ERROR, -errno,
							"Unable to open file");
			return FALSE;
		}
		goto done;
	}

	fd = g_file_open_tmp("obex-clientXXXXXX", &filename, err);
	if (fd < 0) {
		error("g_file_open_tmp(): %s", (*err)->message);
		return FALSE;
	}

	if (transfer->filename == NULL) {
		remove(filename); /* remove always only if NULL was given */
		g_free(filename);
	} else {
		g_free(transfer->filename);
		transfer->filename = filename;
	}

done:
	transfer->fd = fd;
	return TRUE;
}

struct obc_transfer *obc_transfer_get(const char *type, const char *name,
					const char *filename, GError **err)
{
	struct obc_transfer *transfer;
	int perr;

	transfer = obc_transfer_create(G_OBEX_OP_GET, filename, name, type);

	perr = transfer_open(transfer, O_WRONLY | O_CREAT | O_TRUNC, 0600, err);
	if (perr < 0) {
		obc_transfer_free(transfer);
		return NULL;
	}

	return transfer;
}

struct obc_transfer *obc_transfer_put(const char *type, const char *name,
					const char *filename,
					const void *contents, size_t size,
					GError **err)
{
	struct obc_transfer *transfer;
	struct stat st;
	int perr;

	if ((filename == NULL || strcmp(filename, "") == 0) &&
							contents == NULL) {
		g_set_error(err, OBC_TRANSFER_ERROR, -EINVAL,
						"Invalid filename given");
		return NULL;
	}

	transfer = obc_transfer_create(G_OBEX_OP_PUT, filename, name, type);

	if (contents != NULL) {
		ssize_t w;

		if (!transfer_open(transfer, O_RDWR, 0, err))
			goto fail;

		w = write(transfer->fd, contents, size);
		if (w < 0) {
			perr = errno;
			error("write(): %s(%d)", strerror(perr), perr);
			g_set_error(err, OBC_TRANSFER_ERROR, -perr,
						"Writing to file failed");
			goto fail;
		} else if ((size_t) w != size) {
			error("Unable to write all contents to file");
			g_set_error(err, OBC_TRANSFER_ERROR, -EFAULT,
					"Writing all contents to file failed");
			goto fail;
		}
		lseek(transfer->fd, 0, SEEK_SET);
	} else {
		if (!transfer_open(transfer, O_RDONLY, 0, err))
			goto fail;
	}

	if (fstat(transfer->fd, &st) < 0) {
		perr = errno;
		error("fstat(): %s(%d)", strerror(perr), perr);
		g_set_error(err, OBC_TRANSFER_ERROR, -perr,
						"Unable to get file status");
		goto fail;
	}

	transfer->size = st.st_size;

	return transfer;

fail:
	obc_transfer_free(transfer);
	return NULL;
}

void obc_transfer_unregister(struct obc_transfer *transfer)
{
	if (transfer->path) {
		g_dbus_unregister_interface(transfer->conn,
			transfer->path, TRANSFER_INTERFACE);
	}

	DBG("%p unregistered %s", transfer, transfer->path);

	obc_transfer_free(transfer);
}

static gboolean get_xfer_progress(const void *buf, gsize len,
							gpointer user_data)
{
	struct obc_transfer *transfer = user_data;

	if (transfer->fd > 0) {
		int w;

		w = write(transfer->fd, buf, len);
		if (w < 0)
			return FALSE;

		transfer->transferred += w;
	}

	return TRUE;
}

static void xfer_complete(GObex *obex, GError *err, gpointer user_data)
{
	struct obc_transfer *transfer = user_data;
	struct transfer_callback *callback = transfer->callback;

	transfer->xfer = 0;

	if (transfer->progress_id != 0) {
		g_source_remove(transfer->progress_id);
		transfer->progress_id = 0;
	}

	if (transfer->status == TRANSFER_STATUS_SUSPENDED)
		g_obex_resume(transfer->obex);

	if (err)
		transfer_set_status(transfer, TRANSFER_STATUS_ERROR);
	else
		transfer_set_status(transfer, TRANSFER_STATUS_COMPLETE);

	if (callback)
		callback->func(transfer, err, callback->data);
}

static void get_xfer_progress_first(GObex *obex, GError *err, GObexPacket *rsp,
							gpointer user_data)
{
	struct obc_transfer *transfer = user_data;
	GObexPacket *req;
	GObexHeader *hdr;
	GObexApparam *apparam;
	const guint8 *buf;
	gsize len;
	guint8 rspcode;
	gboolean final;

	if (err != NULL) {
		xfer_complete(obex, err, transfer);
		return;
	}

	rspcode = g_obex_packet_get_operation(rsp, &final);
	if (rspcode != G_OBEX_RSP_SUCCESS && rspcode != G_OBEX_RSP_CONTINUE) {
		err = g_error_new(OBC_TRANSFER_ERROR, rspcode, "%s",
						g_obex_strerror(rspcode));
		xfer_complete(obex, err, transfer);
		g_error_free(err);
		return;
	}

	hdr = g_obex_packet_get_header(rsp, G_OBEX_HDR_LENGTH);
	if (hdr) {
		uint32_t len;
		if (g_obex_header_get_uint32(hdr, &len)) {
			transfer->size = len;
			g_dbus_emit_property_changed(transfer->conn,
						transfer->path,
						TRANSFER_INTERFACE, "Size");
		}
	}

	hdr = g_obex_packet_get_header(rsp, G_OBEX_HDR_APPARAM);
	if (hdr) {
		apparam = g_obex_header_get_apparam(hdr);
		if (apparam != NULL)
			obc_transfer_set_apparam(transfer, apparam);
	}

	hdr = g_obex_packet_get_body(rsp);
	if (hdr) {
		g_obex_header_get_bytes(hdr, &buf, &len);
		if (len != 0)
			get_xfer_progress(buf, len, transfer);
	}

	if (rspcode == G_OBEX_RSP_SUCCESS) {
		transfer->req = 0;
		xfer_complete(obex, err, transfer);
		return;
	}

	if (g_obex_srm_active(obex) ||
				transfer->status == TRANSFER_STATUS_SUSPENDED)
		return;

	transfer->req = 0;

	req = g_obex_packet_new(G_OBEX_OP_GET, TRUE, G_OBEX_HDR_INVALID);

	transfer->xfer = g_obex_get_req_pkt(obex, req, get_xfer_progress,
						xfer_complete, transfer,
						&err);
}

static gssize put_xfer_progress(void *buf, gsize len, gpointer user_data)
{
	struct obc_transfer *transfer = user_data;
	gssize size;

	size = read(transfer->fd, buf, len);
	if (size <= 0)
		return size;

	transfer->transferred += size;

	return size;
}

gboolean obc_transfer_set_callback(struct obc_transfer *transfer,
					transfer_callback_t func,
					void *user_data)
{
	struct transfer_callback *callback;

	if (transfer->callback != NULL)
		return FALSE;

	callback = g_new0(struct transfer_callback, 1);
	callback->func = func;
	callback->data = user_data;

	transfer->callback = callback;

	return TRUE;
}

static gboolean report_progress(gpointer data)
{
	struct obc_transfer *transfer = data;

	if (transfer->transferred == transfer->progress)
		return TRUE;

	transfer->progress = transfer->transferred;

	if (transfer->transferred == transfer->size) {
		transfer->progress_id = 0;
		return FALSE;
	}

	if (transfer->status != TRANSFER_STATUS_ACTIVE &&
				transfer->status != TRANSFER_STATUS_SUSPENDED)
		transfer_set_status(transfer, TRANSFER_STATUS_ACTIVE);

	g_dbus_emit_property_changed(transfer->conn, transfer->path,
					TRANSFER_INTERFACE, "Transferred");

	return TRUE;
}

static gboolean transfer_start_get(struct obc_transfer *transfer, GError **err)
{
	GObexPacket *req;
	GObexHeader *hdr;

	if (transfer->xfer > 0) {
		g_set_error(err, OBC_TRANSFER_ERROR, -EALREADY,
						"Transfer already started");
		return FALSE;
	}

	req = g_obex_packet_new(G_OBEX_OP_GET, TRUE, G_OBEX_HDR_INVALID);

	if (transfer->name != NULL)
		g_obex_packet_add_unicode(req, G_OBEX_HDR_NAME,
							transfer->name);

	if (transfer->type != NULL)
		g_obex_packet_add_bytes(req, G_OBEX_HDR_TYPE, transfer->type,
						strlen(transfer->type) + 1);

	if (transfer->apparam != NULL) {
		hdr = g_obex_header_new_apparam(transfer->apparam);
		g_obex_packet_add_header(req, hdr);
	}

	transfer->req = g_obex_send_req(transfer->obex, req,
						FIRST_PACKET_TIMEOUT,
						get_xfer_progress_first,
						transfer, err);
	if (transfer->req == 0)
		return FALSE;

	if (transfer->path == NULL)
		return TRUE;

	transfer->progress_id = g_timeout_add_seconds(1, report_progress,
								transfer);

	return TRUE;
}

static gboolean transfer_start_put(struct obc_transfer *transfer, GError **err)
{
	GObexPacket *req;
	GObexHeader *hdr;

	if (transfer->xfer > 0) {
		g_set_error(err, OBC_TRANSFER_ERROR, -EALREADY,
						"Transfer already started");
		return FALSE;
	}

	req = g_obex_packet_new(G_OBEX_OP_PUT, FALSE, G_OBEX_HDR_INVALID);

	if (transfer->name != NULL)
		g_obex_packet_add_unicode(req, G_OBEX_HDR_NAME,
							transfer->name);

	if (transfer->type != NULL)
		g_obex_packet_add_bytes(req, G_OBEX_HDR_TYPE, transfer->type,
						strlen(transfer->type) + 1);

	if (transfer->size < UINT32_MAX)
		g_obex_packet_add_uint32(req, G_OBEX_HDR_LENGTH, transfer->size);

	if (transfer->apparam != NULL) {
		hdr = g_obex_header_new_apparam(transfer->apparam);
		g_obex_packet_add_header(req, hdr);
	}

	transfer->xfer = g_obex_put_req_pkt(transfer->obex, req,
					put_xfer_progress, xfer_complete,
					transfer, err);
	if (transfer->xfer == 0)
		return FALSE;

	if (transfer->path == NULL)
		return TRUE;

	transfer->progress_id = g_timeout_add_seconds(1, report_progress,
								transfer);

	return TRUE;
}

gboolean obc_transfer_start(struct obc_transfer *transfer, void *obex,
								GError **err)
{
	if (!transfer->obex)
		transfer->obex = g_obex_ref(obex);

	if (transfer->status == TRANSFER_STATUS_SUSPENDED_QUEUED) {
		/* Reset status so the transfer can be resumed */
		transfer->status = TRANSFER_STATUS_SUSPENDED;
		return TRUE;
	}

	switch (transfer->op) {
	case G_OBEX_OP_GET:
		return transfer_start_get(transfer, err);
	case G_OBEX_OP_PUT:
		return transfer_start_put(transfer, err);
	}

	g_set_error(err, OBC_TRANSFER_ERROR, -ENOTSUP, "Not supported");
	return FALSE;
}

guint8 obc_transfer_get_operation(struct obc_transfer *transfer)
{
	return transfer->op;
}

void obc_transfer_set_apparam(struct obc_transfer *transfer, void *data)
{
	if (transfer->apparam != NULL)
		g_obex_apparam_free(transfer->apparam);

	if (data == NULL)
		return;

	transfer->apparam = data;
}

void *obc_transfer_get_apparam(struct obc_transfer *transfer)
{
	return transfer->apparam;
}

int obc_transfer_get_contents(struct obc_transfer *transfer, char **contents,
								size_t *size)
{
	struct stat st;
	ssize_t ret;

	if (contents == NULL)
		return -EINVAL;

	if (fstat(transfer->fd, &st) < 0) {
		error("fstat(): %s(%d)", strerror(errno), errno);
		return -errno;
	}

	if (lseek(transfer->fd, 0, SEEK_SET) < 0) {
		error("lseek(): %s(%d)", strerror(errno), errno);
		return -errno;
	}

	*contents = g_malloc(st.st_size + 1);

	ret = read(transfer->fd, *contents, st.st_size);
	if (ret < 0) {
		error("read(): %s(%d)", strerror(errno), errno);
		g_free(*contents);
		return -errno;
	}

	(*contents)[ret] = '\0';

	if (size)
		*size = ret;

	return 0;
}

const char *obc_transfer_get_path(struct obc_transfer *transfer)
{
	return transfer->path;
}

gint64 obc_transfer_get_size(struct obc_transfer *transfer)
{
	return transfer->size;
}
