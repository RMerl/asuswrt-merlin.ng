/*
 *
 *  OBEX library with GLib integration
 *
 *  Copyright (C) 2011  Intel Corporation. All rights reserved.
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
# include "config.h"
#endif

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "gobex.h"
#include "gobex-debug.h"

#define G_OBEX_DEFAULT_MTU	4096
#define G_OBEX_MINIMUM_MTU	255
#define G_OBEX_MAXIMUM_MTU	65535

#define G_OBEX_DEFAULT_TIMEOUT	10
#define G_OBEX_ABORT_TIMEOUT	5

#define G_OBEX_OP_NONE		0xff

#define FINAL_BIT		0x80

#define CONNID_INVALID		0xffffffff

/* Challenge request */
#define NONCE_TAG		0x00
#define NONCE_LEN		16

/* Challenge response */
#define DIGEST_TAG		0x00

guint gobex_debug = 0;

struct srm_config {
	guint8 op;
	gboolean enabled;
	guint8 srm;
	guint8 srmp;
	gboolean outgoing;
};

struct _GObex {
	int ref_count;
	GIOChannel *io;
	guint io_source;

	gboolean (*read) (GObex *obex, GError **err);
	gboolean (*write) (GObex *obex, GError **err);

	guint8 *rx_buf;
	size_t rx_data;
	guint16 rx_pkt_len;
	guint8 rx_last_op;

	guint8 *tx_buf;
	size_t tx_data;
	size_t tx_sent;

	gboolean suspended;
	gboolean use_srm;

	struct srm_config *srm;

	guint write_source;

	gssize io_rx_mtu;
	gssize io_tx_mtu;

	guint16 rx_mtu;
	guint16 tx_mtu;

	guint32 conn_id;
	GObexApparam *authchal;

	GQueue *tx_queue;

	GSList *req_handlers;

	GObexFunc disconn_func;
	gpointer disconn_func_data;

	struct pending_pkt *pending_req;
};

struct pending_pkt {
	guint id;
	GObex *obex;
	GObexPacket *pkt;
	guint timeout;
	guint timeout_id;
	GObexResponseFunc rsp_func;
	gpointer rsp_data;
	gboolean cancelled;
	gboolean suspended;
	gboolean authenticating;
};

struct req_handler {
	guint id;
	guint8 opcode;
	GObexRequestFunc func;
	gpointer user_data;
};

struct connect_data {
	guint8 version;
	guint8 flags;
	guint16 mtu;
} __attribute__ ((packed));

struct setpath_data {
	guint8 flags;
	guint8 constants;
} __attribute__ ((packed));

static struct error_code {
	guint8 code;
	const char *name;
} obex_errors[] = {
	{ G_OBEX_RSP_CONTINUE,			"Continue" },
	{ G_OBEX_RSP_SUCCESS,			"Success" },
	{ G_OBEX_RSP_CREATED,			"Created" },
	{ G_OBEX_RSP_ACCEPTED,			"Accepted" },
	{ G_OBEX_RSP_NON_AUTHORITATIVE,		"Non Authoritative" },
	{ G_OBEX_RSP_NO_CONTENT,		"No Content" },
	{ G_OBEX_RSP_RESET_CONTENT,		"Reset Content" },
	{ G_OBEX_RSP_PARTIAL_CONTENT,		"Partial Content" },
	{ G_OBEX_RSP_MULTIPLE_CHOICES,		"Multiple Choices" },
	{ G_OBEX_RSP_MOVED_PERMANENTLY,		"Moved Permanently" },
	{ G_OBEX_RSP_MOVED_TEMPORARILY,		"Moved Temporarily" },
	{ G_OBEX_RSP_SEE_OTHER,			"See Other" },
	{ G_OBEX_RSP_NOT_MODIFIED,		"Not Modified" },
	{ G_OBEX_RSP_USE_PROXY,			"Use Proxy" },
	{ G_OBEX_RSP_BAD_REQUEST,		"Bad Request" },
	{ G_OBEX_RSP_UNAUTHORIZED,		"Unauthorized" },
	{ G_OBEX_RSP_PAYMENT_REQUIRED,		"Payment Required" },
	{ G_OBEX_RSP_FORBIDDEN,			"Forbidden" },
	{ G_OBEX_RSP_NOT_FOUND,			"Not Found" },
	{ G_OBEX_RSP_METHOD_NOT_ALLOWED,	"Method Not Allowed" },
	{ G_OBEX_RSP_NOT_ACCEPTABLE,		"Not Acceptable" },
	{ G_OBEX_RSP_PROXY_AUTH_REQUIRED,	"Proxy Authentication Required" },
	{ G_OBEX_RSP_REQUEST_TIME_OUT,		"Request Time Out" },
	{ G_OBEX_RSP_CONFLICT,			"Conflict" },
	{ G_OBEX_RSP_GONE,			"Gone" },
	{ G_OBEX_RSP_LENGTH_REQUIRED,		"Length Required" },
	{ G_OBEX_RSP_PRECONDITION_FAILED,	"Precondition Failed" },
	{ G_OBEX_RSP_REQ_ENTITY_TOO_LARGE,	"Request Entity Too Large" },
	{ G_OBEX_RSP_REQ_URL_TOO_LARGE,		"Request URL Too Large" },
	{ G_OBEX_RSP_UNSUPPORTED_MEDIA_TYPE,	"Unsupported Media Type" },
	{ G_OBEX_RSP_INTERNAL_SERVER_ERROR,	"Internal Server Error" },
	{ G_OBEX_RSP_NOT_IMPLEMENTED,		"Not Implemented" },
	{ G_OBEX_RSP_BAD_GATEWAY,		"Bad Gateway" },
	{ G_OBEX_RSP_SERVICE_UNAVAILABLE,	"Service Unavailable" },
	{ G_OBEX_RSP_GATEWAY_TIMEOUT,		"Gateway Timeout" },
	{ G_OBEX_RSP_VERSION_NOT_SUPPORTED,	"Version Not Supported" },
	{ G_OBEX_RSP_DATABASE_FULL,		"Database Full" },
	{ G_OBEX_RSP_DATABASE_LOCKED,		"Database Locked" },
	{ 0x00,					NULL }
};

const char *g_obex_strerror(guint8 err_code)
{
	struct error_code *error;

	for (error = obex_errors; error->name != NULL; error++) {
		if (error->code == err_code)
			return error->name;
	}

	return "<unknown>";
}

static ssize_t req_header_offset(guint8 opcode)
{
	switch (opcode) {
	case G_OBEX_OP_CONNECT:
		return sizeof(struct connect_data);
	case G_OBEX_OP_SETPATH:
		return sizeof(struct setpath_data);
	case G_OBEX_OP_DISCONNECT:
	case G_OBEX_OP_PUT:
	case G_OBEX_OP_GET:
	case G_OBEX_OP_SESSION:
	case G_OBEX_OP_ABORT:
	case G_OBEX_OP_ACTION:
		return 0;
	default:
		return -1;
	}
}

static ssize_t rsp_header_offset(guint8 opcode)
{
	switch (opcode) {
	case G_OBEX_OP_CONNECT:
		return sizeof(struct connect_data);
	case G_OBEX_OP_SETPATH:
	case G_OBEX_OP_DISCONNECT:
	case G_OBEX_OP_PUT:
	case G_OBEX_OP_GET:
	case G_OBEX_OP_SESSION:
	case G_OBEX_OP_ABORT:
	case G_OBEX_OP_ACTION:
		return 0;
	default:
		return -1;
	}
}

static void pending_pkt_free(struct pending_pkt *p)
{
	if (p->obex != NULL)
		g_obex_unref(p->obex);

	if (p->timeout_id > 0)
		g_source_remove(p->timeout_id);

	g_obex_packet_free(p->pkt);

	g_free(p);
}

static gboolean req_timeout(gpointer user_data)
{
	GObex *obex = user_data;
	struct pending_pkt *p = obex->pending_req;
	GError *err;

	g_assert(p != NULL);

	p->timeout_id = 0;
	obex->pending_req = NULL;

	err = g_error_new(G_OBEX_ERROR, G_OBEX_ERROR_TIMEOUT,
					"Timed out waiting for response");

	g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", err->message);

	if (p->rsp_func)
		p->rsp_func(obex, err, NULL, p->rsp_data);

	g_error_free(err);
	pending_pkt_free(p);

	return FALSE;
}

static gboolean write_stream(GObex *obex, GError **err)
{
	GIOStatus status;
	gsize bytes_written;
	char *buf;

	buf = (char *) &obex->tx_buf[obex->tx_sent];
	status = g_io_channel_write_chars(obex->io, buf, obex->tx_data,
							&bytes_written, err);
	if (status != G_IO_STATUS_NORMAL)
		return FALSE;

	g_obex_dump(G_OBEX_DEBUG_DATA, "<", buf, bytes_written);

	obex->tx_sent += bytes_written;
	obex->tx_data -= bytes_written;

	return TRUE;
}

static gboolean write_packet(GObex *obex, GError **err)
{
	GIOStatus status;
	gsize bytes_written;
	char *buf;

	buf = (char *) &obex->tx_buf[obex->tx_sent];
	status = g_io_channel_write_chars(obex->io, buf, obex->tx_data,
							&bytes_written, err);
	if (status != G_IO_STATUS_NORMAL)
		return FALSE;

	if (bytes_written != obex->tx_data)
		return FALSE;

	g_obex_dump(G_OBEX_DEBUG_DATA, "<", buf, bytes_written);

	obex->tx_sent += bytes_written;
	obex->tx_data -= bytes_written;

	return TRUE;
}

static void set_srmp(GObex *obex, guint8 srmp, gboolean outgoing)
{
	struct srm_config *config = obex->srm;

	if (config == NULL)
		return;

	/* Dont't reset if direction doesn't match */
	if (srmp > G_OBEX_SRMP_NEXT_WAIT && config->outgoing != outgoing)
		return;

	config->srmp = srmp;
	config->outgoing = outgoing;
}

static void set_srm(GObex *obex, guint8 op, guint8 srm)
{
	struct srm_config *config = obex->srm;
	gboolean enable;

	if (config == NULL) {
		if (srm == G_OBEX_SRM_DISABLE)
			return;

		config = g_new0(struct srm_config, 1);
		config->op = op;
		config->srm = srm;
		obex->srm = config;
		return;
	}

	/* Indicate response, treat it as request */
	if (config->srm == G_OBEX_SRM_INDICATE) {
		if (srm != G_OBEX_SRM_ENABLE)
			goto done;
		config->srm = srm;
		return;
	}

	enable = (srm == G_OBEX_SRM_ENABLE);
	if (config->enabled == enable)
		goto done;

	config->enabled = enable;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "SRM %s", config->enabled ?
						"Enabled" : "Disabled");

done:
	if (config->enabled)
		return;

	g_free(obex->srm);
	obex->srm = NULL;
}

static gboolean g_obex_srm_enabled(GObex *obex)
{
	if (!obex->use_srm)
		return FALSE;

	if (obex->srm == NULL)
		return FALSE;

	return obex->srm->enabled;
}

static void check_srm_final(GObex *obex, guint8 op)
{
	if (!g_obex_srm_enabled(obex))
		return;

	switch (obex->srm->op) {
	case G_OBEX_OP_CONNECT:
		return;
	default:
		if (op <= G_OBEX_RSP_CONTINUE)
			return;
	}

	set_srm(obex, op, G_OBEX_SRM_DISABLE);
}

static void setup_srm(GObex *obex, GObexPacket *pkt, gboolean outgoing)
{
	GObexHeader *hdr;
	guint8 op;
	gboolean final;

	if (!obex->use_srm)
		return;

	op = g_obex_packet_get_operation(pkt, &final);

	hdr = g_obex_packet_get_header(pkt, G_OBEX_HDR_SRM);
	if (hdr != NULL) {
		guint8 srm;
		g_obex_header_get_uint8(hdr, &srm);
		g_obex_debug(G_OBEX_DEBUG_COMMAND, "srm 0x%02x", srm);
		set_srm(obex, op, srm);
	} else if (!g_obex_srm_enabled(obex))
		set_srm(obex, op, G_OBEX_SRM_DISABLE);

	hdr = g_obex_packet_get_header(pkt, G_OBEX_HDR_SRMP);
	if (hdr != NULL) {
		guint8 srmp;
		g_obex_header_get_uint8(hdr, &srmp);
		g_obex_debug(G_OBEX_DEBUG_COMMAND, "srmp 0x%02x", srmp);
		set_srmp(obex, srmp, outgoing);
	} else if (obex->pending_req && obex->pending_req->suspended)
		g_obex_packet_add_uint8(pkt, G_OBEX_HDR_SRMP, G_OBEX_SRMP_WAIT);
	else
		set_srmp(obex, -1, outgoing);

	if (final)
		check_srm_final(obex, op);
}

static gboolean write_data(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	GObex *obex = user_data;

	if (cond & G_IO_NVAL)
		return FALSE;

	if (cond & (G_IO_HUP | G_IO_ERR))
		goto stop_tx;

	if (obex->tx_data == 0) {
		struct pending_pkt *p = g_queue_pop_head(obex->tx_queue);
		ssize_t len;

		if (p == NULL)
			goto stop_tx;

		setup_srm(obex, p->pkt, TRUE);

		if (g_obex_srm_enabled(obex))
			goto encode;

		/* Can't send a request while there's a pending one */
		if (obex->pending_req && p->id > 0) {
			g_queue_push_head(obex->tx_queue, p);
			goto stop_tx;
		}

encode:
		len = g_obex_packet_encode(p->pkt, obex->tx_buf, obex->tx_mtu);
		if (len == -EAGAIN) {
			g_queue_push_head(obex->tx_queue, p);
			g_obex_suspend(obex);
			goto stop_tx;
		}

		if (len < 0) {
			pending_pkt_free(p);
			goto done;
		}

		if (p->id > 0) {
			if (obex->pending_req != NULL)
				pending_pkt_free(obex->pending_req);
			obex->pending_req = p;
			p->timeout_id = g_timeout_add_seconds(p->timeout,
							req_timeout, obex);
		} else {
			/* During packet encode final bit can be set */
			if (obex->tx_buf[0] & FINAL_BIT)
				check_srm_final(obex,
						obex->tx_buf[0] & ~FINAL_BIT);
			pending_pkt_free(p);
		}

		obex->tx_data = len;
		obex->tx_sent = 0;
	}

	if (obex->suspended) {
		obex->write_source = 0;
		return FALSE;
	}

	if (!obex->write(obex, NULL))
		goto stop_tx;

done:
	if (obex->tx_data > 0 || g_queue_get_length(obex->tx_queue) > 0)
		return TRUE;

stop_tx:
	obex->rx_last_op = G_OBEX_OP_NONE;
	obex->tx_data = 0;
	obex->write_source = 0;
	return FALSE;
}

static void enable_tx(GObex *obex)
{
	GIOCondition cond;

	if (obex->suspended)
		return;

	if (!obex->io || obex->write_source > 0)
		return;

	cond = G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	obex->write_source = g_io_add_watch(obex->io, cond, write_data, obex);
}

static gboolean g_obex_send_internal(GObex *obex, struct pending_pkt *p,
								GError **err)
{

	if (obex->io == NULL) {
		if (!err)
			return FALSE;
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_DISCONNECTED,
					"The transport is not connected");
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", (*err)->message);
		return FALSE;
	}

	if (g_obex_packet_get_operation(p->pkt, NULL) == G_OBEX_OP_ABORT)
		g_queue_push_head(obex->tx_queue, p);
	else
		g_queue_push_tail(obex->tx_queue, p);

	if (obex->pending_req == NULL || p->id == 0)
		enable_tx(obex);

	return TRUE;
}

static void init_connect_data(GObex *obex, struct connect_data *data)
{
	guint16 u16;

	memset(data, 0, sizeof(*data));

	data->version = 0x10;
	data->flags = 0;

	u16 = g_htons(obex->rx_mtu);
	memcpy(&data->mtu, &u16, sizeof(u16));
}

static guint8 *digest_response(const guint8 *nonce)
{
	GChecksum *md5;
	guint8 *result;
	gsize size;

	result = g_new0(guint8, NONCE_LEN);

	md5 = g_checksum_new(G_CHECKSUM_MD5);
	if (md5 == NULL)
		return result;

	g_checksum_update(md5, nonce, NONCE_LEN);
	g_checksum_update(md5, (guint8 *) ":BlueZ", 6);

	size = NONCE_LEN;
	g_checksum_get_digest(md5, result, &size);

	g_checksum_free(md5);

	return result;
}

static void prepare_auth_rsp(GObex *obex, GObexPacket *rsp)
{
	GObexHeader *hdr;
	GObexApparam *authrsp;
	const guint8 *nonce;
	guint8 *result;
	gsize len;

	/* Check if client is already responding to authentication challenge */
	hdr = g_obex_packet_get_header(rsp, G_OBEX_HDR_AUTHRESP);
	if (hdr)
		goto done;

	if (!g_obex_apparam_get_bytes(obex->authchal, NONCE_TAG, &nonce, &len))
		goto done;

	if (len != NONCE_LEN)
		goto done;

	result = digest_response(nonce);
	authrsp = g_obex_apparam_set_bytes(NULL, DIGEST_TAG, result, NONCE_LEN);

	hdr = g_obex_header_new_tag(G_OBEX_HDR_AUTHRESP, authrsp);
	g_obex_packet_add_header(rsp, hdr);

	g_obex_apparam_free(authrsp);
	g_free(result);

done:
	g_obex_apparam_free(obex->authchal);
	obex->authchal = NULL;
}

static void prepare_connect_rsp(GObex *obex, GObexPacket *rsp)
{
	GObexHeader *hdr;
	struct connect_data data;
	static guint32 next_connid = 1;

	init_connect_data(obex, &data);
	g_obex_packet_set_data(rsp, &data, sizeof(data), G_OBEX_DATA_COPY);

	hdr = g_obex_packet_get_header(rsp, G_OBEX_HDR_CONNECTION);
	if (hdr) {
		g_obex_header_get_uint32(hdr, &obex->conn_id);
		goto done;
	}

	obex->conn_id = next_connid++;

	hdr = g_obex_header_new_uint32(G_OBEX_HDR_CONNECTION, obex->conn_id);
	g_obex_packet_prepend_header(rsp, hdr);

done:
	if (obex->authchal)
		prepare_auth_rsp(obex, rsp);
}

static void prepare_srm_rsp(GObex *obex, GObexPacket *pkt)
{
	GObexHeader *hdr;

	if (!obex->use_srm || obex->srm == NULL)
		return;

	if (obex->srm->enabled)
		return;

	hdr = g_obex_packet_get_header(pkt, G_OBEX_HDR_SRM);
	if (hdr != NULL)
		return;

	hdr = g_obex_header_new_uint8(G_OBEX_HDR_SRM, G_OBEX_SRM_ENABLE);
	g_obex_packet_prepend_header(pkt, hdr);
}

gboolean g_obex_send(GObex *obex, GObexPacket *pkt, GError **err)
{
	struct pending_pkt *p;
	gboolean ret;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "conn %u", obex->conn_id);

	if (obex == NULL || pkt == NULL) {
		if (!err)
			return FALSE;
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_INVALID_ARGS,
				"Invalid arguments");
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", (*err)->message);
		return FALSE;
	}

	switch (obex->rx_last_op) {
	case G_OBEX_OP_CONNECT:
		prepare_connect_rsp(obex, pkt);
		break;
	case G_OBEX_OP_GET:
	case G_OBEX_OP_PUT:
		prepare_srm_rsp(obex, pkt);
		break;
	}

	p = g_new0(struct pending_pkt, 1);
	p->pkt = pkt;

	ret = g_obex_send_internal(obex, p, err);
	if (ret == FALSE)
		pending_pkt_free(p);

	return ret;
}

static void prepare_srm_req(GObex *obex, GObexPacket *pkt)
{
	GObexHeader *hdr;

	if (!obex->use_srm)
		return;

	if (obex->srm != NULL && obex->srm->enabled)
		return;

	hdr = g_obex_packet_get_header(pkt, G_OBEX_HDR_SRM);
	if (hdr != NULL)
		return;

	hdr = g_obex_header_new_uint8(G_OBEX_HDR_SRM, G_OBEX_SRM_ENABLE);
	g_obex_packet_prepend_header(pkt, hdr);
}

guint g_obex_send_req(GObex *obex, GObexPacket *req, int timeout,
			GObexResponseFunc func, gpointer user_data,
			GError **err)
{
	GObexHeader *hdr;
	struct pending_pkt *p;
	static guint id = 1;
	guint8 op;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "conn %u", obex->conn_id);

	op = g_obex_packet_get_operation(req, NULL);
	if (op == G_OBEX_OP_PUT || op == G_OBEX_OP_GET) {
		/* Only enable SRM automatically for GET and PUT */
		prepare_srm_req(obex, req);
	}

	if (obex->conn_id == CONNID_INVALID)
		goto create_pending;

	if (obex->rx_last_op == G_OBEX_RSP_CONTINUE)
		goto create_pending;

	if (g_obex_srm_enabled(obex) && obex->pending_req != NULL)
		goto create_pending;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_CONNECTION);
	if (hdr != NULL)
		goto create_pending;

	hdr = g_obex_header_new_uint32(G_OBEX_HDR_CONNECTION, obex->conn_id);
	g_obex_packet_prepend_header(req, hdr);

create_pending:
	p = g_new0(struct pending_pkt, 1);

	p->pkt = req;
	p->id = id++;
	p->rsp_func = func;
	p->rsp_data = user_data;

	if (timeout < 0)
		p->timeout = G_OBEX_DEFAULT_TIMEOUT;
	else
		p->timeout = timeout;

	if (!g_obex_send_internal(obex, p, err)) {
		pending_pkt_free(p);
		return 0;
	}

	return p->id;
}

static int pending_pkt_cmp(gconstpointer a, gconstpointer b)
{
	const struct pending_pkt *p = a;
	guint id = GPOINTER_TO_UINT(b);

	return (p->id - id);
}

static gboolean pending_req_abort(GObex *obex, GError **err)
{
	struct pending_pkt *p = obex->pending_req;
	GObexPacket *req;

	if (p->cancelled)
		return TRUE;

	p->cancelled = TRUE;

	if (p->timeout_id > 0)
		g_source_remove(p->timeout_id);

	p->timeout = G_OBEX_ABORT_TIMEOUT;
	p->timeout_id = g_timeout_add_seconds(p->timeout, req_timeout, obex);

	req = g_obex_packet_new(G_OBEX_OP_ABORT, TRUE, G_OBEX_HDR_INVALID);

	return g_obex_send(obex, req, err);
}

static gboolean cancel_complete(gpointer user_data)
{
	struct pending_pkt *p = user_data;
	GObex *obex = p->obex;
	GError *err;

	g_assert(p->rsp_func != NULL);

	err = g_error_new(G_OBEX_ERROR, G_OBEX_ERROR_CANCELLED,
					"The request was cancelled");
	p->rsp_func(obex, err, NULL, p->rsp_data);

	g_error_free(err);

	pending_pkt_free(p);

	return FALSE;
}

gboolean g_obex_cancel_req(GObex *obex, guint req_id, gboolean remove_callback)
{
	GList *match;
	struct pending_pkt *p;

	if (obex->pending_req && obex->pending_req->id == req_id) {
		if (!pending_req_abort(obex, NULL)) {
			p = obex->pending_req;
			obex->pending_req = NULL;
			goto immediate_completion;
		}

		if (remove_callback)
			obex->pending_req->rsp_func = NULL;

		return TRUE;
	}

	match = g_queue_find_custom(obex->tx_queue, GUINT_TO_POINTER(req_id),
							pending_pkt_cmp);
	if (match == NULL)
		return FALSE;

	p = match->data;

	g_queue_delete_link(obex->tx_queue, match);

immediate_completion:
	p->cancelled = TRUE;
	p->obex = g_obex_ref(obex);

	if (remove_callback || p->rsp_func == NULL)
		pending_pkt_free(p);
	else
		g_idle_add(cancel_complete, p);

	return TRUE;
}

gboolean g_obex_send_rsp(GObex *obex, guint8 rspcode, GError **err,
						guint8 first_hdr_type, ...)
{
	GObexPacket *rsp;
	va_list args;

	va_start(args, first_hdr_type);
	rsp = g_obex_packet_new_valist(rspcode, TRUE, first_hdr_type, args);
	va_end(args);

	return g_obex_send(obex, rsp, err);
}

void g_obex_set_disconnect_function(GObex *obex, GObexFunc func,
							gpointer user_data)
{
	obex->disconn_func = func;
	obex->disconn_func_data = user_data;
}

static int req_handler_cmpop(gconstpointer a, gconstpointer b)
{
	const struct req_handler *handler = a;
	guint opcode = GPOINTER_TO_UINT(b);

	return (int) handler->opcode - (int) opcode;
}

static int req_handler_cmpid(gconstpointer a, gconstpointer b)
{
	const struct req_handler *handler = a;
	guint id = GPOINTER_TO_UINT(b);

	return (int) handler->id - (int) id;
}

guint g_obex_add_request_function(GObex *obex, guint8 opcode,
						GObexRequestFunc func,
						gpointer user_data)
{
	struct req_handler *handler;
	static guint next_id = 1;

	handler = g_new0(struct req_handler, 1);
	handler->id = next_id++;
	handler->opcode = opcode;
	handler->func = func;
	handler->user_data = user_data;

	obex->req_handlers = g_slist_prepend(obex->req_handlers, handler);

	return handler->id;
}

gboolean g_obex_remove_request_function(GObex *obex, guint id)
{
	struct req_handler *handler;
	GSList *match;

	match = g_slist_find_custom(obex->req_handlers, GUINT_TO_POINTER(id),
							req_handler_cmpid);
	if (match == NULL)
		return FALSE;

	handler = match->data;

	obex->req_handlers = g_slist_delete_link(obex->req_handlers, match);
	g_free(handler);

	return TRUE;
}

static void g_obex_srm_suspend(GObex *obex)
{
	struct pending_pkt *p = obex->pending_req;
	GObexPacket *req;

	if (p->timeout_id > 0) {
		g_source_remove(p->timeout_id);
		p->timeout_id = 0;
	}

	p->suspended = TRUE;

	req = g_obex_packet_new(G_OBEX_OP_GET, TRUE,
					G_OBEX_HDR_SRMP, G_OBEX_SRMP_WAIT,
					G_OBEX_HDR_INVALID);

	g_obex_send(obex, req, NULL);
}

void g_obex_suspend(GObex *obex)
{
	struct pending_pkt *req = obex->pending_req;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "conn %u", obex->conn_id);

	if (!g_obex_srm_active(obex) || !req)
		goto done;

	/* Send SRMP wait in case of GET */
	if (g_obex_packet_get_operation(req->pkt, NULL) == G_OBEX_OP_GET) {
		g_obex_srm_suspend(obex);
		return;
	}

done:
	obex->suspended = TRUE;

	if (obex->write_source > 0) {
		g_source_remove(obex->write_source);
		obex->write_source = 0;
	}
}

static void g_obex_srm_resume(GObex *obex)
{
	struct pending_pkt *p = obex->pending_req;
	GObexPacket *req;

	p->timeout_id = g_timeout_add_seconds(p->timeout, req_timeout, obex);
	p->suspended = FALSE;

	req = g_obex_packet_new(G_OBEX_OP_GET, TRUE, G_OBEX_HDR_INVALID);

	g_obex_send(obex, req, NULL);
}

void g_obex_resume(GObex *obex)
{
	struct pending_pkt *req = obex->pending_req;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "conn %u", obex->conn_id);

	obex->suspended = FALSE;

	if (g_obex_srm_active(obex) || !req)
		goto done;

	if (g_obex_packet_get_operation(req->pkt, NULL) == G_OBEX_OP_GET)
		g_obex_srm_resume(obex);

done:
	if (g_queue_get_length(obex->tx_queue) > 0 || obex->tx_data > 0)
		enable_tx(obex);
}

gboolean g_obex_srm_active(GObex *obex)
{
	gboolean ret = FALSE;

	if (!g_obex_srm_enabled(obex))
		goto done;

	if (obex->srm->srmp <= G_OBEX_SRMP_NEXT_WAIT)
		goto done;

	ret = TRUE;
done:
	g_obex_debug(G_OBEX_DEBUG_COMMAND, "%s", ret ? "yes" : "no");
	return ret;
}

static void auth_challenge(GObex *obex)
{
	struct pending_pkt *p = obex->pending_req;

	if (p->authenticating)
		return;

	p->authenticating = TRUE;

	prepare_auth_rsp(obex, p->pkt);

	/* Remove it as pending and add it back to the queue so it gets sent
	 * again */
	if (p->timeout_id > 0) {
		g_source_remove(p->timeout_id);
		p->timeout_id = 0;
	}
	obex->pending_req = NULL;
	g_obex_send_internal(obex, p, NULL);
}

static void parse_connect_data(GObex *obex, GObexPacket *pkt)
{
	const struct connect_data *data;
	GObexHeader *hdr;
	guint16 u16;
	size_t data_len;

	data = g_obex_packet_get_data(pkt, &data_len);
	if (data == NULL || data_len != sizeof(*data))
		return;

	memcpy(&u16, &data->mtu, sizeof(u16));

	obex->tx_mtu = g_ntohs(u16);
	if (obex->io_tx_mtu > 0 && obex->tx_mtu > obex->io_tx_mtu)
		obex->tx_mtu = obex->io_tx_mtu;
	obex->tx_buf = g_realloc(obex->tx_buf, obex->tx_mtu);

	hdr = g_obex_packet_get_header(pkt, G_OBEX_HDR_CONNECTION);
	if (hdr)
		g_obex_header_get_uint32(hdr, &obex->conn_id);

	hdr = g_obex_packet_get_header(pkt, G_OBEX_HDR_AUTHCHAL);
	if (hdr)
		obex->authchal = g_obex_header_get_apparam(hdr);
}

static gboolean parse_response(GObex *obex, GObexPacket *rsp)
{
	struct pending_pkt *p = obex->pending_req;
	guint8 opcode, rspcode;
	gboolean final;

	rspcode = g_obex_packet_get_operation(rsp, &final);

	opcode = g_obex_packet_get_operation(p->pkt, NULL);
	if (opcode == G_OBEX_OP_CONNECT) {
		parse_connect_data(obex, rsp);
		if (rspcode == G_OBEX_RSP_UNAUTHORIZED && obex->authchal)
			auth_challenge(obex);
	}

	setup_srm(obex, rsp, FALSE);

	if (!g_obex_srm_enabled(obex))
		return final;

	/*
	 * Resposes have final bit set but in case of GET with SRM
	 * we should not remove the request since the remote side will
	 * continue sending responses until the transfer is finished
	 */
	if (opcode == G_OBEX_OP_GET && rspcode == G_OBEX_RSP_CONTINUE) {
		if (p->timeout_id > 0)
			g_source_remove(p->timeout_id);

		p->timeout_id = g_timeout_add_seconds(p->timeout, req_timeout,
									obex);
		return FALSE;
	}

	return final;
}

static void handle_response(GObex *obex, GError *err, GObexPacket *rsp)
{
	struct pending_pkt *p;
	gboolean disconn = err ? TRUE : FALSE, final_rsp = TRUE;

	if (rsp != NULL)
		final_rsp = parse_response(obex, rsp);

	if (!obex->pending_req)
		return;

	p = obex->pending_req;

	/* Reset if final so it can no longer be cancelled */
	if (final_rsp)
		obex->pending_req = NULL;

	if (p->cancelled)
		err = g_error_new(G_OBEX_ERROR, G_OBEX_ERROR_CANCELLED,
					"The operation was cancelled");

	if (err)
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", err->message);

	if (p->rsp_func) {
		p->rsp_func(obex, err, rsp, p->rsp_data);

		/* Check if user callback removed the request */
		if (!final_rsp && p != obex->pending_req)
			return;
	}

	if (p->cancelled)
		g_error_free(err);

	if (final_rsp)
		pending_pkt_free(p);

	if (!disconn && g_queue_get_length(obex->tx_queue) > 0)
		enable_tx(obex);
}

static gboolean check_connid(GObex *obex, GObexPacket *pkt)
{
	GObexHeader *hdr;
	guint32 id;

	if (obex->conn_id == CONNID_INVALID)
		return TRUE;

	hdr = g_obex_packet_get_header(pkt, G_OBEX_HDR_CONNECTION);
	if (hdr == NULL)
		return TRUE;

	g_obex_header_get_uint32(hdr, &id);

	return obex->conn_id == id;
}

static int parse_request(GObex *obex, GObexPacket *req)
{
	guint8 op;
	gboolean final;

	op = g_obex_packet_get_operation(req, &final);
	switch (op) {
	case G_OBEX_OP_CONNECT:
		parse_connect_data(obex, req);
		break;
	case G_OBEX_OP_ABORT:
		break;
	default:
		if (check_connid(obex, req))
			break;

		return -G_OBEX_RSP_SERVICE_UNAVAILABLE;
	}

	setup_srm(obex, req, FALSE);

	return op;
}

static void handle_request(GObex *obex, GObexPacket *req)
{
	GSList *match;
	int op;

	op = parse_request(obex, req);
	if (op < 0)
		goto fail;

	match = g_slist_find_custom(obex->req_handlers, GUINT_TO_POINTER(op),
							req_handler_cmpop);
	if (match) {
		struct req_handler *handler = match->data;
		handler->func(obex, req, handler->user_data);
		return;
	}

	op = -G_OBEX_RSP_NOT_IMPLEMENTED;

fail:
	g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", g_obex_strerror(-op));
	g_obex_send_rsp(obex, -op, NULL, G_OBEX_HDR_INVALID);
}

static gboolean read_stream(GObex *obex, GError **err)
{
	GIOChannel *io = obex->io;
	GIOStatus status;
	gsize rbytes, toread;
	guint16 u16;
	char *buf;

	if (obex->rx_data >= 3)
		goto read_body;

	rbytes = 0;
	toread = 3 - obex->rx_data;
	buf = (char *) &obex->rx_buf[obex->rx_data];

	status = g_io_channel_read_chars(io, buf, toread, &rbytes, NULL);
	if (status != G_IO_STATUS_NORMAL)
		return TRUE;

	obex->rx_data += rbytes;
	if (obex->rx_data < 3)
		goto done;

	memcpy(&u16, &buf[1], sizeof(u16));
	obex->rx_pkt_len = g_ntohs(u16);

	if (obex->rx_pkt_len > obex->rx_mtu) {
		if (!err)
			return FALSE;
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_PARSE_ERROR,
				"Too big incoming packet");
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", (*err)->message);
		return FALSE;
	}

read_body:
	if (obex->rx_data >= obex->rx_pkt_len)
		goto done;

	do {
		toread = obex->rx_pkt_len - obex->rx_data;
		buf = (char *) &obex->rx_buf[obex->rx_data];

		status = g_io_channel_read_chars(io, buf, toread, &rbytes, NULL);
		if (status != G_IO_STATUS_NORMAL)
			goto done;

		obex->rx_data += rbytes;
	} while (rbytes > 0 && obex->rx_data < obex->rx_pkt_len);

done:
	g_obex_dump(G_OBEX_DEBUG_DATA, ">", obex->rx_buf, obex->rx_data);

	return TRUE;
}

static gboolean read_packet(GObex *obex, GError **err)
{
	GIOChannel *io = obex->io;
	GError *read_err = NULL;
	GIOStatus status;
	gsize rbytes;
	guint16 u16;

	if (obex->rx_data > 0) {
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_PARSE_ERROR,
				"RX buffer not empty before reading packet");
		goto fail;
	}

	status = g_io_channel_read_chars(io, (char *) obex->rx_buf,
					obex->rx_mtu, &rbytes, &read_err);
	if (status != G_IO_STATUS_NORMAL) {
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_PARSE_ERROR,
				"Unable to read data: %s", read_err->message);
		g_error_free(read_err);
		goto fail;
	}

	obex->rx_data += rbytes;

	if (rbytes < 3) {
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_PARSE_ERROR,
				"Incomplete packet received");
		goto fail;
	}

	memcpy(&u16, &obex->rx_buf[1], sizeof(u16));
	obex->rx_pkt_len = g_ntohs(u16);

	if (obex->rx_pkt_len != rbytes) {
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_PARSE_ERROR,
			"Data size doesn't match packet size (%zu != %u)",
			rbytes, obex->rx_pkt_len);
		return FALSE;
	}

	g_obex_dump(G_OBEX_DEBUG_DATA, ">", obex->rx_buf, obex->rx_data);

	return TRUE;
fail:
	if (err)
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", (*err)->message);

	return FALSE;
}

static gboolean incoming_data(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	GObex *obex = user_data;
	GObexPacket *pkt;
	ssize_t header_offset;
	GError *err = NULL;
	guint8 opcode;

	if (cond & G_IO_NVAL)
		return FALSE;

	if (cond & (G_IO_HUP | G_IO_ERR)) {
		err = g_error_new(G_OBEX_ERROR, G_OBEX_ERROR_DISCONNECTED,
					"Transport got disconnected");
		goto failed;
	}

	if (!obex->read(obex, &err))
		goto failed;

	if (obex->rx_data < 3 || obex->rx_data < obex->rx_pkt_len)
		return TRUE;

	obex->rx_last_op = obex->rx_buf[0] & ~FINAL_BIT;

	if (obex->pending_req) {
		struct pending_pkt *p = obex->pending_req;
		opcode = g_obex_packet_get_operation(p->pkt, NULL);
		header_offset = rsp_header_offset(opcode);
	} else {
		opcode = obex->rx_last_op;
		/* Unexpected response -- fail silently */
		if (opcode > 0x1f && opcode != G_OBEX_OP_ABORT) {
			obex->rx_data = 0;
			return TRUE;
		}
		header_offset = req_header_offset(opcode);
	}

	if (header_offset < 0) {
		err = g_error_new(G_OBEX_ERROR, G_OBEX_ERROR_PARSE_ERROR,
				"Unknown header offset for opcode 0x%02x",
				opcode);
		goto failed;
	}

	pkt = g_obex_packet_decode(obex->rx_buf, obex->rx_data, header_offset,
							G_OBEX_DATA_REF, &err);
	if (pkt == NULL)
		goto failed;

	/* Protect against user callback freeing the object */
	g_obex_ref(obex);

	if (obex->pending_req)
		handle_response(obex, NULL, pkt);
	else
		handle_request(obex, pkt);

	obex->rx_data = 0;

	g_obex_unref(obex);

	if (err != NULL)
		g_error_free(err);

	if (pkt != NULL)
		g_obex_packet_free(pkt);

	return TRUE;

failed:
	if (err)
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", err->message);

	g_io_channel_unref(obex->io);
	obex->io = NULL;
	obex->io_source = 0;
	obex->rx_data = 0;

	/* Protect against user callback freeing the object */
	g_obex_ref(obex);

	if (obex->pending_req)
		handle_response(obex, err, NULL);

	if (obex->disconn_func)
		obex->disconn_func(obex, err, obex->disconn_func_data);

	g_obex_unref(obex);

	g_error_free(err);

	return FALSE;
}

static GDebugKey keys[] = {
	{ "error",	G_OBEX_DEBUG_ERROR },
	{ "command",	G_OBEX_DEBUG_COMMAND },
	{ "transfer",	G_OBEX_DEBUG_TRANSFER },
	{ "header",	G_OBEX_DEBUG_HEADER },
	{ "packet",	G_OBEX_DEBUG_PACKET },
	{ "data",	G_OBEX_DEBUG_DATA },
	{ "apparam",	G_OBEX_DEBUG_APPARAM },
};

GObex *g_obex_new(GIOChannel *io, GObexTransportType transport_type,
					gssize io_rx_mtu, gssize io_tx_mtu)
{
	GObex *obex;
	GIOCondition cond;

	if (gobex_debug == 0) {
		const char *env = g_getenv("GOBEX_DEBUG");

		if (env) {
			gobex_debug = g_parse_debug_string(env, keys, 7);
			g_setenv("G_MESSAGES_DEBUG", "gobex", FALSE);
		} else
			gobex_debug = G_OBEX_DEBUG_NONE;
	}

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "");

	if (io == NULL)
		return NULL;

	if (io_rx_mtu >= 0 && io_rx_mtu < G_OBEX_MINIMUM_MTU)
		return NULL;

	if (io_tx_mtu >= 0 && io_tx_mtu < G_OBEX_MINIMUM_MTU)
		return NULL;

	obex = g_new0(GObex, 1);

	obex->io = g_io_channel_ref(io);
	obex->ref_count = 1;
	obex->conn_id = CONNID_INVALID;
	obex->rx_last_op = G_OBEX_OP_NONE;

	obex->io_rx_mtu = io_rx_mtu;
	obex->io_tx_mtu = io_tx_mtu;

	if (io_rx_mtu > G_OBEX_MAXIMUM_MTU)
		obex->rx_mtu = G_OBEX_MAXIMUM_MTU;
	else if (io_rx_mtu < G_OBEX_MINIMUM_MTU)
		obex->rx_mtu = G_OBEX_DEFAULT_MTU;
	else
		obex->rx_mtu = io_rx_mtu;

	obex->tx_mtu = G_OBEX_MINIMUM_MTU;

	obex->tx_queue = g_queue_new();
	obex->rx_buf = g_malloc(obex->rx_mtu);
	obex->tx_buf = g_malloc(obex->tx_mtu);

	switch (transport_type) {
	case G_OBEX_TRANSPORT_STREAM:
		obex->read = read_stream;
		obex->write = write_stream;
		break;
	case G_OBEX_TRANSPORT_PACKET:
		obex->use_srm = TRUE;
		obex->read = read_packet;
		obex->write = write_packet;
		break;
	default:
		g_obex_unref(obex);
		return NULL;
	}

	g_io_channel_set_encoding(io, NULL, NULL);
	g_io_channel_set_buffered(io, FALSE);
	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	obex->io_source = g_io_add_watch(io, cond, incoming_data, obex);

	return obex;
}

GObex *g_obex_ref(GObex *obex)
{
	int refs;

	if (obex == NULL)
		return NULL;

	refs = __sync_add_and_fetch(&obex->ref_count, 1);

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "ref %u", refs);

	return obex;
}

void g_obex_unref(GObex *obex)
{
	int refs;

	refs = __sync_sub_and_fetch(&obex->ref_count, 1);

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "ref %u", refs);

	if (refs > 0)
		return;

	g_slist_free_full(obex->req_handlers, g_free);

	g_queue_foreach(obex->tx_queue, (GFunc) pending_pkt_free, NULL);
	g_queue_free(obex->tx_queue);

	if (obex->io != NULL)
		g_io_channel_unref(obex->io);

	if (obex->io_source > 0)
		g_source_remove(obex->io_source);

	if (obex->write_source > 0)
		g_source_remove(obex->write_source);

	g_free(obex->rx_buf);
	g_free(obex->tx_buf);
	g_free(obex->srm);

	if (obex->pending_req)
		pending_pkt_free(obex->pending_req);

	if (obex->authchal)
		g_obex_apparam_free(obex->authchal);

	g_free(obex);
}

/* Higher level functions */

guint g_obex_connect(GObex *obex, GObexResponseFunc func, gpointer user_data,
					GError **err, guint8 first_hdr_id, ...)
{
	GObexPacket *req;
	struct connect_data data;
	va_list args;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "");

	va_start(args, first_hdr_id);
	req = g_obex_packet_new_valist(G_OBEX_OP_CONNECT, TRUE,
							first_hdr_id, args);
	va_end(args);

	init_connect_data(obex, &data);
	g_obex_packet_set_data(req, &data, sizeof(data), G_OBEX_DATA_COPY);

	return g_obex_send_req(obex, req, -1, func, user_data, err);
}

guint g_obex_disconnect(GObex *obex, GObexResponseFunc func, gpointer user_data,
								GError **err)
{
	GObexPacket *req;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "");

	req = g_obex_packet_new(G_OBEX_OP_DISCONNECT, TRUE, G_OBEX_HDR_INVALID);

	return g_obex_send_req(obex, req, -1, func, user_data, err);
}

guint g_obex_setpath(GObex *obex, const char *path, GObexResponseFunc func,
					gpointer user_data, GError **err)
{
	GObexPacket *req;
	struct setpath_data data;
	const char *folder;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "conn %u", obex->conn_id);

	req = g_obex_packet_new(G_OBEX_OP_SETPATH, TRUE, G_OBEX_HDR_INVALID);

	memset(&data, 0, sizeof(data));

	if (path != NULL && strncmp("..", path, 2) == 0) {
		data.flags = 0x03;
		folder = (path[2] == '/') ? &path[3] : NULL;
	} else {
		data.flags = 0x02;
		folder = path;
	}

	if (folder != NULL) {
		GObexHeader *hdr;
		hdr = g_obex_header_new_unicode(G_OBEX_HDR_NAME, folder);
		g_obex_packet_add_header(req, hdr);
	}

	g_obex_packet_set_data(req, &data, sizeof(data), G_OBEX_DATA_COPY);

	return g_obex_send_req(obex, req, -1, func, user_data, err);
}

guint g_obex_mkdir(GObex *obex, const char *path, GObexResponseFunc func,
					gpointer user_data, GError **err)
{
	GObexPacket *req;
	struct setpath_data data;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "conn %u", obex->conn_id);

	req = g_obex_packet_new(G_OBEX_OP_SETPATH, TRUE, G_OBEX_HDR_NAME, path,
							G_OBEX_HDR_INVALID);

	memset(&data, 0, sizeof(data));
	g_obex_packet_set_data(req, &data, sizeof(data), G_OBEX_DATA_COPY);

	return g_obex_send_req(obex, req, -1, func, user_data, err);
}

guint g_obex_delete(GObex *obex, const char *name, GObexResponseFunc func,
					gpointer user_data, GError **err)
{
	GObexPacket *req;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "conn %u", obex->conn_id);

	req = g_obex_packet_new(G_OBEX_OP_PUT, TRUE, G_OBEX_HDR_NAME, name,
							G_OBEX_HDR_INVALID);

	return g_obex_send_req(obex, req, -1, func, user_data, err);
}

guint g_obex_copy(GObex *obex, const char *name, const char *dest,
			GObexResponseFunc func, gpointer user_data,
			GError **err)
{
	GObexPacket *req;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "conn %u", obex->conn_id);

	req = g_obex_packet_new(G_OBEX_OP_ACTION, TRUE,
					G_OBEX_HDR_ACTION, G_OBEX_ACTION_COPY,
					G_OBEX_HDR_NAME, name,
					G_OBEX_HDR_DESTNAME, dest,
					G_OBEX_HDR_INVALID);

	return g_obex_send_req(obex, req, -1, func, user_data, err);
}

guint g_obex_move(GObex *obex, const char *name, const char *dest,
			GObexResponseFunc func, gpointer user_data,
			GError **err)
{
	GObexPacket *req;

	g_obex_debug(G_OBEX_DEBUG_COMMAND, "conn %u", obex->conn_id);

	req = g_obex_packet_new(G_OBEX_OP_ACTION, TRUE,
					G_OBEX_HDR_ACTION, G_OBEX_ACTION_MOVE,
					G_OBEX_HDR_NAME, name,
					G_OBEX_HDR_DESTNAME, dest,
					G_OBEX_HDR_INVALID);

	return g_obex_send_req(obex, req, -1, func, user_data, err);
}

guint g_obex_abort(GObex *obex, GObexResponseFunc func, gpointer user_data,
								GError **err)
{
	GObexPacket *req;

	req = g_obex_packet_new(G_OBEX_OP_ABORT, TRUE, G_OBEX_HDR_INVALID);

	return g_obex_send_req(obex, req, -1, func, user_data, err);
}

guint8 g_obex_errno_to_rsp(int err)
{
	switch (err) {
	case 0:
		return G_OBEX_RSP_SUCCESS;
	case -EPERM:
	case -EACCES:
		return G_OBEX_RSP_FORBIDDEN;
	case -ENOENT:
		return G_OBEX_RSP_NOT_FOUND;
	case -EINVAL:
	case -EBADR:
		return G_OBEX_RSP_BAD_REQUEST;
	case -EFAULT:
		return G_OBEX_RSP_SERVICE_UNAVAILABLE;
	case -ENOSYS:
		return G_OBEX_RSP_NOT_IMPLEMENTED;
	case -ENOTEMPTY:
	case -EEXIST:
		return G_OBEX_RSP_PRECONDITION_FAILED;
	default:
		return G_OBEX_RSP_INTERNAL_SERVER_ERROR;
	}
}
