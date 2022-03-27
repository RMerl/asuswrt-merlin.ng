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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <inttypes.h>

#include <glib.h>

#include "gobex/gobex.h"

#include "btio/btio.h"
#include "obexd.h"
#include "log.h"
#include "obex.h"
#include "obex-priv.h"
#include "server.h"
#include "manager.h"
#include "mimetype.h"
#include "service.h"
#include "transport.h"

static GSList *sessions = NULL;

typedef struct {
	uint8_t  version;
	uint8_t  flags;
	uint16_t mtu;
} __attribute__ ((packed)) obex_connect_hdr_t;

struct auth_header {
	uint8_t tag;
	uint8_t len;
	uint8_t val[0];
} __attribute__ ((packed));

/* Possible commands */
static struct {
	int cmd;
	const char *name;
} obex_command[] = {
	{ G_OBEX_OP_CONNECT,	"CONNECT"	},
	{ G_OBEX_OP_DISCONNECT,	"DISCONNECT"	},
	{ G_OBEX_OP_PUT,	"PUT"		},
	{ G_OBEX_OP_GET,	"GET"		},
	{ G_OBEX_OP_SETPATH,	"SETPATH"	},
	{ G_OBEX_OP_SESSION,	"SESSION"	},
	{ G_OBEX_OP_ABORT,	"ABORT"		},
	{ G_OBEX_OP_ACTION,	"ACTION"	},
	{ 0xFF,			NULL		},
};

/* Possible Response */
static struct {
	int rsp;
	const char *name;
} obex_response[] = {
	{ G_OBEX_RSP_CONTINUE,			"CONTINUE"		},
	{ G_OBEX_RSP_SUCCESS,			"SUCCESS"		},
	{ G_OBEX_RSP_CREATED,			"CREATED"		},
	{ G_OBEX_RSP_ACCEPTED,			"ACCEPTED"		},
	{ G_OBEX_RSP_NON_AUTHORITATIVE,		"NON_AUTHORITATIVE"	},
	{ G_OBEX_RSP_NO_CONTENT,		"NO_CONTENT"		},
	{ G_OBEX_RSP_RESET_CONTENT,		"RESET_CONTENT"		},
	{ G_OBEX_RSP_PARTIAL_CONTENT,		"PARTIAL_CONTENT"	},
	{ G_OBEX_RSP_MULTIPLE_CHOICES,		"MULTIPLE_CHOICES"	},
	{ G_OBEX_RSP_MOVED_PERMANENTLY,		"MOVED_PERMANENTLY"	},
	{ G_OBEX_RSP_MOVED_TEMPORARILY,		"MOVED_TEMPORARILY"	},
	{ G_OBEX_RSP_SEE_OTHER,			"SEE_OTHER"		},
	{ G_OBEX_RSP_NOT_MODIFIED,		"NOT_MODIFIED"		},
	{ G_OBEX_RSP_USE_PROXY,			"USE_PROXY"		},
	{ G_OBEX_RSP_BAD_REQUEST,		"BAD_REQUEST"		},
	{ G_OBEX_RSP_UNAUTHORIZED,		"UNAUTHORIZED"		},
	{ G_OBEX_RSP_PAYMENT_REQUIRED,		"PAYMENT_REQUIRED"	},
	{ G_OBEX_RSP_FORBIDDEN,			"FORBIDDEN"		},
	{ G_OBEX_RSP_NOT_FOUND,			"NOT_FOUND"		},
	{ G_OBEX_RSP_METHOD_NOT_ALLOWED,	"METHOD_NOT_ALLOWED"	},
	{ G_OBEX_RSP_NOT_ACCEPTABLE,		"NOT_ACCEPTABLE"	},
	{ G_OBEX_RSP_PROXY_AUTH_REQUIRED,	"PROXY_AUTH_REQUIRED"	},
	{ G_OBEX_RSP_REQUEST_TIME_OUT,		"REQUEST_TIME_OUT"	},
	{ G_OBEX_RSP_CONFLICT,			"CONFLICT"		},
	{ G_OBEX_RSP_GONE,			"GONE"			},
	{ G_OBEX_RSP_LENGTH_REQUIRED,		"LENGTH_REQUIRED"	},
	{ G_OBEX_RSP_PRECONDITION_FAILED,	"PRECONDITION_FAILED"	},
	{ G_OBEX_RSP_REQ_ENTITY_TOO_LARGE,	"REQ_ENTITY_TOO_LARGE"	},
	{ G_OBEX_RSP_REQ_URL_TOO_LARGE,		"REQ_URL_TOO_LARGE"	},
	{ G_OBEX_RSP_UNSUPPORTED_MEDIA_TYPE,	"UNSUPPORTED_MEDIA_TYPE"},
	{ G_OBEX_RSP_INTERNAL_SERVER_ERROR,	"INTERNAL_SERVER_ERROR"	},
	{ G_OBEX_RSP_NOT_IMPLEMENTED,		"NOT_IMPLEMENTED"	},
	{ G_OBEX_RSP_BAD_GATEWAY,		"BAD_GATEWAY"		},
	{ G_OBEX_RSP_SERVICE_UNAVAILABLE,	"SERVICE_UNAVAILABLE"	},
	{ G_OBEX_RSP_GATEWAY_TIMEOUT,		"GATEWAY_TIMEOUT"	},
	{ G_OBEX_RSP_VERSION_NOT_SUPPORTED,	"VERSION_NOT_SUPPORTED"	},
	{ G_OBEX_RSP_DATABASE_FULL,		"DATABASE_FULL"		},
	{ G_OBEX_RSP_DATABASE_LOCKED,		"DATABASE_LOCKED"	},
	{ 0xFF,					NULL			},
};

static gboolean handle_async_io(void *object, int flags, int err,
						void *user_data);

static void print_event(int cmd, int rsp)
{
	const char *cmdstr = NULL, *rspstr = NULL;
	int i;
	static int lastcmd;

	if (cmd < 0)
		cmd = lastcmd;
	else
		lastcmd = cmd;

	for (i = 0; obex_command[i].cmd != 0xFF; i++) {
		if (obex_command[i].cmd != cmd)
			continue;
		cmdstr = obex_command[i].name;
	}

	for (i = 0; obex_response[i].rsp != 0xFF; i++) {
		if (obex_response[i].rsp != rsp)
			continue;
		rspstr = obex_response[i].name;
	}

	obex_debug("%s(0x%x), %s(0x%x)", cmdstr, cmd, rspstr, rsp);
}

static void os_set_response(struct obex_session *os, int err)
{
	uint8_t rsp;

	rsp = g_obex_errno_to_rsp(err);

	print_event(-1, rsp);

	g_obex_send_rsp(os->obex, rsp, NULL, G_OBEX_HDR_INVALID);
}

static void os_session_mark_aborted(struct obex_session *os)
{
	/* the session was already cancelled/aborted or size in unknown */
	if (os->aborted || os->size == OBJECT_SIZE_UNKNOWN)
		return;

	os->aborted = (os->size != os->offset);
}

static void os_reset_session(struct obex_session *os)
{
	os_session_mark_aborted(os);

	if (os->object) {
		os->driver->set_io_watch(os->object, NULL, NULL);
		os->driver->close(os->object);
		if (os->aborted && os->cmd == G_OBEX_OP_PUT && os->path &&
				os->driver->remove)
			os->driver->remove(os->path);
	}

	if (os->service && os->service->reset)
		os->service->reset(os, os->service_data);

	if (os->name) {
		g_free(os->name);
		os->name = NULL;
	}
	if (os->type) {
		g_free(os->type);
		os->type = NULL;
	}
	if (os->buf) {
		g_free(os->buf);
		os->buf = NULL;
	}
	if (os->path) {
		g_free(os->path);
		os->path = NULL;
	}
	if (os->apparam) {
		g_free(os->apparam);
		os->apparam = NULL;
		os->apparam_len = 0;
	}

	if (os->get_rsp > 0) {
		g_obex_remove_request_function(os->obex, os->get_rsp);
		os->get_rsp = 0;
	}

	os->object = NULL;
	os->driver = NULL;
	os->aborted = FALSE;
	os->pending = 0;
	os->offset = 0;
	os->size = OBJECT_SIZE_DELETE;
	os->headers_sent = FALSE;
	os->checked = FALSE;
}

static void obex_session_free(struct obex_session *os)
{
	sessions = g_slist_remove(sessions, os);

	if (os->io)
		g_io_channel_unref(os->io);

	if (os->obex)
		g_obex_unref(os->obex);

	g_free(os->src);
	g_free(os->dst);

	g_free(os);
}

/* From Imendio's GnomeVFS OBEX module (om-utils.c) */
static time_t parse_iso8610(const char *val, int size)
{
	time_t time, tz_offset = 0;
	struct tm tm;
	char *date;
	char tz;
	int nr;

	memset(&tm, 0, sizeof(tm));
	/* According to spec the time doesn't have to be null terminated */
	date = g_strndup(val, size);
	nr = sscanf(date, "%04u%02u%02uT%02u%02u%02u%c",
			&tm.tm_year, &tm.tm_mon, &tm.tm_mday,
			&tm.tm_hour, &tm.tm_min, &tm.tm_sec,
			&tz);
	g_free(date);
	if (nr < 6) {
		/* Invalid time format */
		return -1;
	}

	tm.tm_year -= 1900;	/* Year since 1900 */
	tm.tm_mon--;		/* Months since January, values 0-11 */
	tm.tm_isdst = -1;	/* Daylight savings information not avail */

#if defined(HAVE_TM_GMTOFF)
	tz_offset = tm.tm_gmtoff;
#elif defined(HAVE_TIMEZONE)
	tz_offset = -timezone;
	if (tm.tm_isdst > 0)
		tz_offset += 3600;
#endif

	time = mktime(&tm);
	if (nr == 7) {
		/*
		 * Date/Time was in localtime (to remote device)
		 * already. Since we don't know anything about the
		 * timezone on that one we won't try to apply UTC offset
		 */
		time += tz_offset;
	}

	return time;
}

static void parse_service(struct obex_session *os, GObexPacket *req)
{
	GObexHeader *hdr;
	const guint8 *target = NULL, *who = NULL;
	gsize target_size = 0, who_size = 0;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_WHO);
	if (hdr == NULL)
		goto target;

	g_obex_header_get_bytes(hdr, &who, &who_size);

target:
	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_TARGET);
	if (hdr == NULL)
		goto probe;

	g_obex_header_get_bytes(hdr, &target, &target_size);

probe:
	os->service = obex_service_driver_find(os->server->drivers,
						target, target_size,
						who, who_size);
}

static void cmd_connect(GObex *obex, GObexPacket *req, void *user_data)
{
	struct obex_session *os = user_data;
	GObexPacket *rsp;
	GObexHeader *hdr;
	int err;

	DBG("");

	print_event(G_OBEX_OP_CONNECT, -1);

	parse_service(os, req);

	if (os->service == NULL || os->service->connect == NULL) {
		error("Connect attempt to a non-supported target");
		os_set_response(os, -EPERM);
		return;
	}

	DBG("Selected driver: %s", os->service->name);

	os->service_data = os->service->connect(os, &err);
	if (err < 0) {
		os_set_response(os, err);
		return;
	}

	os->cmd = G_OBEX_OP_CONNECT;

	rsp = g_obex_packet_new(G_OBEX_RSP_SUCCESS, TRUE, G_OBEX_HDR_INVALID);

	if (os->service->target) {
		hdr = g_obex_header_new_bytes(G_OBEX_HDR_WHO,
						os->service->target,
						os->service->target_size);
		g_obex_packet_add_header(rsp, hdr);
	}

	g_obex_send(obex, rsp, NULL);

	print_event(-1, 0);
}

static void cmd_disconnect(GObex *obex, GObexPacket *req, void *user_data)
{
	struct obex_session *os = user_data;

	DBG("session %p", os);

	print_event(G_OBEX_OP_DISCONNECT, -1);

	os->cmd = G_OBEX_OP_DISCONNECT;

	os_set_response(os, 0);
}

static ssize_t driver_write(struct obex_session *os)
{
	ssize_t len = 0;

	while (os->pending > 0) {
		ssize_t w;

		w = os->driver->write(os->object, os->buf + len, os->pending);
		if (w < 0) {
			error("write(): %s (%zd)", strerror(-w), -w);
			if (w == -EINTR)
				continue;
			else if (w == -EINVAL)
				memmove(os->buf, os->buf + len, os->pending);

			return w;
		}

		len += w;
		os->offset += w;
		os->pending -= w;
	}

	DBG("%zd written", len);

	if (os->service->progress != NULL)
		os->service->progress(os, os->service_data);

	return len;
}

static gssize driver_read(struct obex_session *os, void *buf, gsize size)
{
	gssize len;

	if (os->object == NULL)
		return -EIO;

	if (os->service->progress != NULL)
		os->service->progress(os, os->service_data);

	len = os->driver->read(os->object, buf, size);
	if (len < 0) {
		error("read(): %s (%zd)", strerror(-len), -len);
		if (len == -ENOSTR)
			return 0;
		if (len == -EAGAIN)
			os->driver->set_io_watch(os->object, handle_async_io,
									os);
	}

	os->offset += len;

	DBG("%zd read", len);

	return len;
}

static gssize send_data(void *buf, gsize size, gpointer user_data)
{
	struct obex_session *os = user_data;

	DBG("name=%s type=%s file=%p size=%zu", os->name, os->type, os->object,
									size);

	if (os->aborted)
		return os->err < 0 ? os->err : -EPERM;

	return driver_read(os, buf, size);
}

static void transfer_complete(GObex *obex, GError *err, gpointer user_data)
{
	struct obex_session *os = user_data;

	DBG("");

	if (err != NULL) {
		error("transfer failed: %s\n", err->message);
		goto reset;
	}

	if (os->object && os->driver && os->driver->flush) {
		if (os->driver->flush(os->object) == -EAGAIN) {
			g_obex_suspend(os->obex);
			os->driver->set_io_watch(os->object, handle_async_io,
									os);
			return;
		}
	}

reset:
	os_reset_session(os);
}

static int driver_get_headers(struct obex_session *os)
{
	GObexPacket *rsp;
	gssize len;
	guint8 data[255];
	guint8 id;
	GObexHeader *hdr;

	DBG("name=%s type=%s object=%p", os->name, os->type, os->object);

	if (os->aborted)
		return os->err < 0 ? os->err : -EPERM;

	if (os->object == NULL)
		return -EIO;

	if (os->headers_sent)
		return 0;

	rsp = g_obex_packet_new(G_OBEX_RSP_CONTINUE, TRUE, G_OBEX_HDR_INVALID);

	if (os->driver->get_next_header == NULL)
		goto done;

	while ((len = os->driver->get_next_header(os->object, &data,
							sizeof(data), &id))) {
		if (len < 0) {
			error("get_next_header(): %s (%zd)", strerror(-len),
								-len);

			g_obex_packet_free(rsp);

			if (len == -EAGAIN)
				return len;

			g_free(os->buf);
			os->buf = NULL;

			return len;
		}

		hdr = g_obex_header_new_bytes(id, data, len);
		g_obex_packet_add_header(rsp, hdr);
	}

done:
	if (os->size != OBJECT_SIZE_UNKNOWN && os->size < UINT32_MAX) {
		hdr = g_obex_header_new_uint32(G_OBEX_HDR_LENGTH, os->size);
		g_obex_packet_add_header(rsp, hdr);
	}

	g_obex_get_rsp_pkt(os->obex, rsp, send_data, transfer_complete, os,
									NULL);

	os->headers_sent = TRUE;

	print_event(-1, G_OBEX_RSP_CONTINUE);

	return 0;
}

static gboolean handle_async_io(void *object, int flags, int err,
						void *user_data)
{
	struct obex_session *os = user_data;

	if (err < 0)
		goto done;

	if (flags & G_IO_OUT)
		err = driver_write(os);
	if ((flags & G_IO_IN) && !os->headers_sent)
		err = driver_get_headers(os);

	if (err == -EAGAIN)
		return TRUE;

done:
	if (err < 0) {
		os->err = err;
		os->aborted = TRUE;
	}

	g_obex_resume(os->obex);

	return FALSE;
}

static gboolean recv_data(const void *buf, gsize size, gpointer user_data)
{
	struct obex_session *os = user_data;
	ssize_t ret;

	DBG("name=%s type=%s file=%p size=%zu", os->name, os->type, os->object,
									size);

	if (os->aborted)
		return FALSE;

	/* workaround: client didn't send the object length */
	if (os->size == OBJECT_SIZE_DELETE)
		os->size = OBJECT_SIZE_UNKNOWN;

	os->buf = g_realloc(os->buf, os->pending + size);
	memcpy(os->buf + os->pending, buf, size);
	os->pending += size;

	/* only write if both object and driver are valid */
	if (os->object == NULL || os->driver == NULL) {
		DBG("Stored %" PRIu64 " bytes into temporary buffer",
								os->pending);
		return TRUE;
	}

	ret = driver_write(os);
	if (ret >= 0)
		return TRUE;

	if (ret == -EAGAIN) {
		g_obex_suspend(os->obex);
		os->driver->set_io_watch(os->object, handle_async_io, os);
		return TRUE;
	}

	return FALSE;
}

static void parse_type(struct obex_session *os, GObexPacket *req)
{
	GObexHeader *hdr;
	const guint8 *type;
	gsize len;

	g_free(os->type);
	os->type = NULL;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_TYPE);
	if (hdr == NULL)
		goto probe;

	if (!g_obex_header_get_bytes(hdr, &type, &len))
		goto probe;

	/* Ensure null termination */
	if (type[len - 1] != '\0')
		goto probe;

	os->type = g_strndup((const char *) type, len);
	DBG("TYPE: %s", os->type);

probe:
	os->driver = obex_mime_type_driver_find(os->service->target,
						os->service->target_size,
						os->type,
						os->service->who,
						os->service->who_size);
}

static void parse_name(struct obex_session *os, GObexPacket *req)
{
	GObexHeader *hdr;
	const char *name;

	g_free(os->name);
	os->name = NULL;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_NAME);
	if (hdr == NULL)
		return;

	if (!g_obex_header_get_unicode(hdr, &name))
		return;

	os->name = g_strdup(name);
	DBG("NAME: %s", os->name);
}

static void parse_apparam(struct obex_session *os, GObexPacket *req)
{
	GObexHeader *hdr;
	const guint8 *apparam;
	gsize len;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_APPARAM);
	if (hdr == NULL)
		return;

	if (!g_obex_header_get_bytes(hdr, &apparam, &len))
		return;

	os->apparam = g_memdup(apparam, len);
	os->apparam_len = len;
	DBG("APPARAM");
}

static void cmd_get(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct obex_session *os = user_data;
	int err;

	DBG("session %p", os);

	print_event(G_OBEX_OP_GET, -1);

	if (os->service == NULL) {
		os_set_response(os, -EPERM);
		return;
	}

	if (os->service->get == NULL) {
		os_set_response(os, -ENOSYS);
		return;
	}

	os->headers_sent = FALSE;

	if (os->type) {
		g_free(os->type);
		os->type = NULL;
	}

	parse_type(os, req);

	if (!os->driver) {
		error("No driver found");
		os_set_response(os, -ENOSYS);
		return;
	}

	os->cmd = G_OBEX_OP_GET;

	parse_name(os, req);

	parse_apparam(os, req);

	err = os->service->get(os, os->service_data);
	if (err == 0)
		return;

	os_set_response(os, err);
}

static void cmd_setpath(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct obex_session *os = user_data;
	int err;

	DBG("");

	print_event(G_OBEX_OP_SETPATH, -1);

	if (os->service == NULL) {
		err = -EPERM;
		goto done;
	}

	if (os->service->setpath == NULL) {
		err = -ENOSYS;
		goto done;
	}

	os->cmd = G_OBEX_OP_SETPATH;

	parse_name(os, req);

	os->nonhdr = g_obex_packet_get_data(req, &os->nonhdr_len);

	err = os->service->setpath(os, os->service_data);
done:
	os_set_response(os, err);
}

int obex_get_stream_start(struct obex_session *os, const char *filename)
{
	int err;
	void *object;
	size_t size = OBJECT_SIZE_UNKNOWN;

	object = os->driver->open(filename, O_RDONLY, 0, os->service_data,
								&size, &err);
	if (object == NULL) {
		error("open(%s): %s (%d)", filename, strerror(-err), -err);
		return err;
	}

	os->object = object;
	os->offset = 0;
	os->size = size;

	err = driver_get_headers(os);
	if (err != -EAGAIN)
		return err;

	g_obex_suspend(os->obex);
	os->driver->set_io_watch(os->object, handle_async_io, os);
	return 0;
}

int obex_put_stream_start(struct obex_session *os, const char *filename)
{
	int err;

	os->object = os->driver->open(filename, O_WRONLY | O_CREAT | O_TRUNC,
					0600, os->service_data,
					os->size != OBJECT_SIZE_UNKNOWN ?
					(size_t *) &os->size : NULL, &err);
	if (os->object == NULL) {
		error("open(%s): %s (%d)", filename, strerror(-err), -err);
		return err;
	}

	os->path = g_strdup(filename);

	return 0;
}

static void parse_length(struct obex_session *os, GObexPacket *req)
{
	GObexHeader *hdr;
	guint32 size;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_LENGTH);
	if (hdr == NULL)
		return;

	if (!g_obex_header_get_uint32(hdr, &size))
		return;

	os->size = size;
	DBG("LENGTH: %" PRIu64, os->size);
}

static void parse_time(struct obex_session *os, GObexPacket *req)
{
	GObexHeader *hdr;
	const guint8 *time;
	gsize len;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_TIME);
	if (hdr == NULL)
		return;


	if (!g_obex_header_get_bytes(hdr, &time, &len))
		return;

	os->time = parse_iso8610((const char *) time, len);
	DBG("TIME: %s", ctime(&os->time));
}

static gboolean check_put(GObex *obex, GObexPacket *req, void *user_data)
{
	struct obex_session *os = user_data;
	int ret;

	if (os->service->chkput == NULL)
		goto done;

	ret = os->service->chkput(os, os->service_data);
	switch (ret) {
	case 0:
		break;
	case -EAGAIN:
		g_obex_suspend(os->obex);
		os->driver->set_io_watch(os->object, handle_async_io, os);
		return TRUE;
	default:
		os_set_response(os, ret);
		return FALSE;
	}

	if (os->size == OBJECT_SIZE_DELETE || os->size == OBJECT_SIZE_UNKNOWN)
		DBG("Got a PUT without a Length");

done:
	os->checked = TRUE;

	return TRUE;
}

static void cmd_put(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct obex_session *os = user_data;
	int err;

	DBG("");

	print_event(G_OBEX_OP_PUT, -1);

	if (os->service == NULL) {
		os_set_response(os, -EPERM);
		return;
	}

	parse_type(os, req);

	if (os->driver == NULL) {
		error("No driver found");
		os_set_response(os, -ENOSYS);
		return;
	}

	os->cmd = G_OBEX_OP_PUT;

	/* Set size to unknown if a body header exists */
	if (g_obex_packet_get_body(req))
		os->size = OBJECT_SIZE_UNKNOWN;

	parse_name(os, req);
	parse_length(os, req);
	parse_time(os, req);
	parse_apparam(os, req);

	if (!os->checked) {
		if (!check_put(obex, req, user_data))
			return;
	}

	if (os->service->put == NULL) {
		os_set_response(os, -ENOSYS);
		return;
	}

	err = os->service->put(os, os->service_data);
	if (err == 0) {
		g_obex_put_rsp(obex, req, recv_data, transfer_complete, os,
						NULL, G_OBEX_HDR_INVALID);
		print_event(G_OBEX_OP_PUT, G_OBEX_RSP_CONTINUE);
		return;
	}

	os_set_response(os, err);
}

static void parse_destname(struct obex_session *os, GObexPacket *req)
{
	GObexHeader *hdr;
	const char *destname;

	g_free(os->destname);
	os->destname = NULL;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_DESTNAME);
	if (hdr == NULL)
		return;

	if (!g_obex_header_get_unicode(hdr, &destname))
		return;

	os->destname = g_strdup(destname);
	DBG("DESTNAME: %s", os->destname);
}

static void parse_action(struct obex_session *os, GObexPacket *req)
{
	GObexHeader *hdr;
	guint8 id;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_ACTION);
	if (hdr == NULL)
		return;

	if (!g_obex_header_get_uint8(hdr, &id))
		return;

	os->action_id = id;
	DBG("ACTION: 0x%02x", os->action_id);
}

static void cmd_action(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct obex_session *os = user_data;
	int err;

	DBG("");

	print_event(G_OBEX_OP_ACTION, -1);

	if (os->service == NULL) {
		err = -EPERM;
		goto done;
	}

	if (os->service->action == NULL) {
		err = -ENOSYS;
		goto done;
	}

	os->cmd = G_OBEX_OP_ACTION;

	parse_name(os, req);
	parse_destname(os, req);
	parse_action(os, req);

	os->driver = obex_mime_type_driver_find(os->service->target,
						os->service->target_size,
						NULL,
						os->service->who,
						os->service->who_size);
	if (os->driver == NULL) {
		err = -ENOSYS;
		goto done;
	}

	err = os->service->action(os, os->service_data);
done:
	os_set_response(os, err);
}

static void cmd_abort(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct obex_session *os = user_data;

	DBG("");

	print_event(G_OBEX_OP_ABORT, -1);

	os_reset_session(os);

	os_set_response(os, 0);
}

static void obex_session_destroy(struct obex_session *os)
{
	DBG("");

	os_reset_session(os);

	if (os->service && os->service->disconnect)
		os->service->disconnect(os, os->service_data);

	obex_session_free(os);
}

static void disconn_func(GObex *obex, GError *err, gpointer user_data)
{
	struct obex_session *os = user_data;

	error("disconnected: %s\n", err ? err->message : "<no err>");
	obex_session_destroy(os);
}

int obex_session_start(GIOChannel *io, uint16_t tx_mtu, uint16_t rx_mtu,
				gboolean stream, struct obex_server *server)
{
	struct obex_session *os;
	GObex *obex;
	GObexTransportType type;
	static uint32_t id = 0;

	DBG("");

	os = g_new0(struct obex_session, 1);
	os->id = ++id;

	os->service = obex_service_driver_find(server->drivers, NULL,
							0, NULL, 0);
	os->server = server;
	os->size = OBJECT_SIZE_DELETE;

	type = stream ? G_OBEX_TRANSPORT_STREAM : G_OBEX_TRANSPORT_PACKET;

	obex = g_obex_new(io, type, rx_mtu, tx_mtu);
	if (!obex) {
		obex_session_free(os);
		return -EIO;
	}

	g_obex_set_disconnect_function(obex, disconn_func, os);
	g_obex_add_request_function(obex, G_OBEX_OP_CONNECT, cmd_connect, os);
	g_obex_add_request_function(obex, G_OBEX_OP_DISCONNECT, cmd_disconnect,
									os);
	g_obex_add_request_function(obex, G_OBEX_OP_PUT, cmd_put, os);
	g_obex_add_request_function(obex, G_OBEX_OP_GET, cmd_get, os);
	g_obex_add_request_function(obex, G_OBEX_OP_SETPATH, cmd_setpath, os);
	g_obex_add_request_function(obex, G_OBEX_OP_ACTION, cmd_action, os);
	g_obex_add_request_function(obex, G_OBEX_OP_ABORT, cmd_abort, os);

	os->obex = obex;
	os->io = g_io_channel_ref(io);

	obex_getsockname(os, &os->src);
	obex_getpeername(os, &os->dst);

	sessions = g_slist_prepend(sessions, os);

	return 0;
}

const char *obex_get_name(struct obex_session *os)
{
	return os->name;
}

const char *obex_get_destname(struct obex_session *os)
{
	return os->destname;
}

void obex_set_name(struct obex_session *os, const char *name)
{
	g_free(os->name);
	os->name = g_strdup(name);
	DBG("Name changed: %s", os->name);
}

ssize_t obex_get_size(struct obex_session *os)
{
	return os->size;
}

const char *obex_get_type(struct obex_session *os)
{
	return os->type;
}

int obex_remove(struct obex_session *os, const char *path)
{
	if (os->driver == NULL)
		return -ENOSYS;

	return os->driver->remove(path);
}

int obex_copy(struct obex_session *os, const char *source,
						const char *destination)
{
	if (os->driver == NULL || os->driver->copy == NULL)
		return -ENOSYS;

	DBG("%s %s", source, destination);

	return os->driver->copy(source, destination);
}

int obex_move(struct obex_session *os, const char *source,
						const char *destination)
{
	if (os->driver == NULL || os->driver->move == NULL)
		return -ENOSYS;

	DBG("%s %s", source, destination);

	return os->driver->move(source, destination);
}

uint8_t obex_get_action_id(struct obex_session *os)
{
	return os->action_id;
}

ssize_t obex_get_apparam(struct obex_session *os, const uint8_t **buffer)
{
	*buffer = os->apparam;

	return os->apparam_len;
}

ssize_t obex_get_non_header_data(struct obex_session *os,
							const uint8_t **data)
{
	*data = os->nonhdr;

	return os->nonhdr_len;
}

int obex_getpeername(struct obex_session *os, char **name)
{
	struct obex_transport_driver *transport = os->server->transport;

	if (transport == NULL || transport->getpeername == NULL)
		return -ENOTSUP;

	return transport->getpeername(os->io, name);
}

int obex_getsockname(struct obex_session *os, char **name)
{
	struct obex_transport_driver *transport = os->server->transport;

	if (transport == NULL || transport->getsockname == NULL)
		return -ENOTSUP;

	return transport->getsockname(os->io, name);
}

int memncmp0(const void *a, size_t na, const void *b, size_t nb)
{
	if (na != nb)
		return na - nb;

	if (a == NULL)
		return -(a != b);

	if (b == NULL)
		return a != b;

	return memcmp(a, b, na);
}
