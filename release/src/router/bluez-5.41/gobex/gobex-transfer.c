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
#include <config.h>
#endif

#include <string.h>
#include <errno.h>

#include "gobex/gobex.h"
#include "gobex/gobex-debug.h"

#define FIRST_PACKET_TIMEOUT 60

static GSList *transfers = NULL;

static void transfer_response(GObex *obex, GError *err, GObexPacket *rsp,
							gpointer user_data);

struct transfer {
	guint id;
	guint8 opcode;

	GObex *obex;

	guint req_id;

	guint put_id;
	guint get_id;
	guint abort_id;

	GObexDataProducer data_producer;
	GObexDataConsumer data_consumer;
	GObexFunc complete_func;

	gpointer user_data;
};

static void transfer_free(struct transfer *transfer)
{
	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	transfers = g_slist_remove(transfers, transfer);

	if (transfer->req_id > 0)
		g_obex_cancel_req(transfer->obex, transfer->req_id, TRUE);

	if (transfer->put_id > 0)
		g_obex_remove_request_function(transfer->obex,
							transfer->put_id);

	if (transfer->get_id > 0)
		g_obex_remove_request_function(transfer->obex,
							transfer->get_id);

	if (transfer->abort_id > 0)
		g_obex_remove_request_function(transfer->obex,
							transfer->abort_id);

	g_obex_unref(transfer->obex);
	g_free(transfer);
}

static struct transfer *find_transfer(guint id)
{
	GSList *l;

	for (l = transfers; l != NULL; l = g_slist_next(l)) {
		struct transfer *t = l->data;
		if (t->id == id)
			return t;
	}

	return NULL;
}

static void transfer_complete(struct transfer *transfer, GError *err)
{
	guint id = transfer->id;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", id);

	transfer->complete_func(transfer->obex, err, transfer->user_data);
	/* Check if the complete_func removed the transfer */
	if (find_transfer(id) == NULL)
		return;

	transfer_free(transfer);
}

static void transfer_abort_response(GObex *obex, GError *err, GObexPacket *rsp,
							gpointer user_data)
{
	struct transfer *transfer = user_data;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	transfer->req_id = 0;

	/* Intentionally override error */
	err = g_error_new(G_OBEX_ERROR, G_OBEX_ERROR_CANCELLED,
						"Operation was aborted");
	g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", err->message);
	transfer_complete(transfer, err);
	g_error_free(err);
}


static gssize put_get_data(void *buf, gsize len, gpointer user_data)
{
	struct transfer *transfer = user_data;
	GObexPacket *req;
	GError *err = NULL;
	gssize ret;

	ret = transfer->data_producer(buf, len, transfer->user_data);
	if (ret == 0 || ret == -EAGAIN)
		return ret;

	if (ret > 0) {
		/* Check if SRM is active */
		if (!g_obex_srm_active(transfer->obex))
			return ret;

		/* Generate next packet */
		req = g_obex_packet_new(transfer->opcode, FALSE,
							G_OBEX_HDR_INVALID);
		g_obex_packet_add_body(req, put_get_data, transfer);
		transfer->req_id = g_obex_send_req(transfer->obex, req, -1,
						transfer_response, transfer,
						&err);
		goto done;
	}

	transfer->req_id = g_obex_abort(transfer->obex, transfer_abort_response,
								transfer, &err);
done:
	if (err != NULL) {
		transfer_complete(transfer, err);
		g_error_free(err);
	}

	return ret;
}

static gboolean handle_get_body(struct transfer *transfer, GObexPacket *rsp,
								GError **err)
{
	GObexHeader *body = g_obex_packet_get_body(rsp);
	gboolean ret;
	const guint8 *buf;
	gsize len;

	if (body == NULL)
		return TRUE;

	g_obex_header_get_bytes(body, &buf, &len);
	if (len == 0)
		return TRUE;

	ret = transfer->data_consumer(buf, len, transfer->user_data);
	if (ret == FALSE)
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_CANCELLED,
				"Data consumer callback failed");

	return ret;
}

static void transfer_response(GObex *obex, GError *err, GObexPacket *rsp,
							gpointer user_data)
{
	struct transfer *transfer = user_data;
	GObexPacket *req;
	gboolean rspcode, final;
	guint id;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	id = transfer->req_id;
	transfer->req_id = 0;

	if (err != NULL) {
		transfer_complete(transfer, err);
		return;
	}

	rspcode = g_obex_packet_get_operation(rsp, &final);
	if (rspcode != G_OBEX_RSP_SUCCESS && rspcode != G_OBEX_RSP_CONTINUE) {
		err = g_error_new(G_OBEX_ERROR, rspcode, "%s",
						g_obex_strerror(rspcode));
		goto failed;
	}

	if (transfer->opcode == G_OBEX_OP_GET) {
		handle_get_body(transfer, rsp, &err);
		if (err != NULL)
			goto failed;
	}

	if (rspcode == G_OBEX_RSP_SUCCESS) {
		transfer_complete(transfer, NULL);
		return;
	}

	if (transfer->opcode == G_OBEX_OP_PUT) {
		req = g_obex_packet_new(transfer->opcode, FALSE,
							G_OBEX_HDR_INVALID);
		g_obex_packet_add_body(req, put_get_data, transfer);
	} else if (!g_obex_srm_active(transfer->obex)) {
		req = g_obex_packet_new(transfer->opcode, TRUE,
							G_OBEX_HDR_INVALID);
	} else {
		/* Keep id since request still outstanting */
		transfer->req_id = id;
		return;
	}

	transfer->req_id = g_obex_send_req(obex, req, -1, transfer_response,
							transfer, &err);
failed:
	if (err != NULL) {
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", err->message);
		transfer_complete(transfer, err);
		g_error_free(err);
	}
}

static struct transfer *transfer_new(GObex *obex, guint8 opcode,
				GObexFunc complete_func, gpointer user_data)
{
	static guint next_id = 1;
	struct transfer *transfer;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "obex %p opcode %u", obex, opcode);

	transfer = g_new0(struct transfer, 1);

	transfer->id = next_id++;
	transfer->opcode = opcode;
	transfer->obex = g_obex_ref(obex);
	transfer->complete_func = complete_func;
	transfer->user_data = user_data;

	transfers = g_slist_append(transfers, transfer);

	return transfer;
}

guint g_obex_put_req_pkt(GObex *obex, GObexPacket *req,
			GObexDataProducer data_func, GObexFunc complete_func,
			gpointer user_data, GError **err)
{
	struct transfer *transfer;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "obex %p", obex);

	if (g_obex_packet_get_operation(req, NULL) != G_OBEX_OP_PUT)
		return 0;

	transfer = transfer_new(obex, G_OBEX_OP_PUT, complete_func, user_data);
	transfer->data_producer = data_func;

	g_obex_packet_add_body(req, put_get_data, transfer);

	transfer->req_id = g_obex_send_req(obex, req, FIRST_PACKET_TIMEOUT,
					transfer_response, transfer, err);
	if (transfer->req_id == 0) {
		transfer_free(transfer);
		return 0;
	}

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	return transfer->id;
}

guint g_obex_put_req(GObex *obex, GObexDataProducer data_func,
			GObexFunc complete_func, gpointer user_data,
			GError **err, guint8 first_hdr_id, ...)
{
	GObexPacket *req;
	va_list args;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "obex %p", obex);

	va_start(args, first_hdr_id);
	req = g_obex_packet_new_valist(G_OBEX_OP_PUT, FALSE,
							first_hdr_id, args);
	va_end(args);

	return g_obex_put_req_pkt(obex, req, data_func, complete_func,
							user_data, err);
}

static void transfer_abort_req(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct transfer *transfer = user_data;
	GObexPacket *rsp;
	GError *err;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	err = g_error_new(G_OBEX_ERROR, G_OBEX_ERROR_CANCELLED,
						"Request was aborted");
	rsp = g_obex_packet_new(G_OBEX_RSP_SUCCESS, TRUE, G_OBEX_HDR_INVALID);
	g_obex_send(obex, rsp, NULL);

	transfer_complete(transfer, err);
	g_error_free(err);
}

static guint8 put_get_bytes(struct transfer *transfer, GObexPacket *req)
{
	GObexHeader *body;
	gboolean final;
	guint8 rsp;
	const guint8 *buf;
	gsize len;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	g_obex_packet_get_operation(req, &final);
	if (final)
		rsp = G_OBEX_RSP_SUCCESS;
	else
		rsp = G_OBEX_RSP_CONTINUE;

	body = g_obex_packet_get_body(req);
	if (body == NULL)
		return rsp;

	g_obex_header_get_bytes(body, &buf, &len);
	if (len == 0)
		return rsp;

	if (transfer->data_consumer(buf, len, transfer->user_data) == FALSE)
		rsp = G_OBEX_RSP_FORBIDDEN;

	return rsp;
}

static void transfer_put_req_first(struct transfer *transfer, GObexPacket *req,
					guint8 first_hdr_id, va_list args)
{
	GError *err = NULL;
	GObexPacket *rsp;
	guint8 rspcode;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	rspcode = put_get_bytes(transfer, req);

	rsp = g_obex_packet_new_valist(rspcode, TRUE, first_hdr_id, args);

	if (!g_obex_send(transfer->obex, rsp, &err)) {
		transfer_complete(transfer, err);
		g_error_free(err);
		return;
	}

	if (rspcode != G_OBEX_RSP_CONTINUE)
		transfer_complete(transfer, NULL);
}

static void transfer_put_req(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct transfer *transfer = user_data;
	GError *err = NULL;
	GObexPacket *rsp;
	guint8 rspcode;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	rspcode = put_get_bytes(transfer, req);

	/* Don't send continue while SRM is active */
	if (g_obex_srm_active(transfer->obex) &&
				rspcode == G_OBEX_RSP_CONTINUE)
		goto done;

	rsp = g_obex_packet_new(rspcode, TRUE, G_OBEX_HDR_INVALID);

	if (!g_obex_send(obex, rsp, &err)) {
		transfer_complete(transfer, err);
		g_error_free(err);
		return;
	}

done:
	if (rspcode != G_OBEX_RSP_CONTINUE)
		transfer_complete(transfer, NULL);
}

guint g_obex_put_rsp(GObex *obex, GObexPacket *req,
			GObexDataConsumer data_func, GObexFunc complete_func,
			gpointer user_data, GError **err,
			guint8 first_hdr_id, ...)
{
	struct transfer *transfer;
	va_list args;
	guint id;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "obex %p", obex);

	transfer = transfer_new(obex, G_OBEX_OP_PUT, complete_func, user_data);
	transfer->data_consumer = data_func;

	va_start(args, first_hdr_id);
	transfer_put_req_first(transfer, req, first_hdr_id, args);
	va_end(args);
	if (!g_slist_find(transfers, transfer))
		return 0;

	id = g_obex_add_request_function(obex, G_OBEX_OP_PUT, transfer_put_req,
								transfer);
	transfer->put_id = id;

	id = g_obex_add_request_function(obex, G_OBEX_OP_ABORT,
						transfer_abort_req, transfer);
	transfer->abort_id = id;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	return transfer->id;
}

guint g_obex_get_req_pkt(GObex *obex, GObexPacket *req,
			GObexDataConsumer data_func, GObexFunc complete_func,
			gpointer user_data, GError **err)
{
	struct transfer *transfer;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "obex %p", obex);

	if (g_obex_packet_get_operation(req, NULL) != G_OBEX_OP_GET)
		return 0;

	transfer = transfer_new(obex, G_OBEX_OP_GET, complete_func, user_data);
	transfer->data_consumer = data_func;
	transfer->req_id = g_obex_send_req(obex, req, FIRST_PACKET_TIMEOUT,
					transfer_response, transfer, err);
	if (transfer->req_id == 0) {
		transfer_free(transfer);
		return 0;
	}

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	return transfer->id;
}

guint g_obex_get_req(GObex *obex, GObexDataConsumer data_func,
			GObexFunc complete_func, gpointer user_data,
			GError **err, guint8 first_hdr_id, ...)
{
	struct transfer *transfer;
	GObexPacket *req;
	va_list args;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "obex %p", obex);

	transfer = transfer_new(obex, G_OBEX_OP_GET, complete_func, user_data);
	transfer->data_consumer = data_func;

	va_start(args, first_hdr_id);
	req = g_obex_packet_new_valist(G_OBEX_OP_GET, TRUE,
							first_hdr_id, args);
	va_end(args);

	transfer->req_id = g_obex_send_req(obex, req, FIRST_PACKET_TIMEOUT,
					transfer_response, transfer, err);
	if (transfer->req_id == 0) {
		transfer_free(transfer);
		return 0;
	}

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	return transfer->id;
}

static gssize get_get_data(void *buf, gsize len, gpointer user_data)
{
	struct transfer *transfer = user_data;
	GObexPacket *req, *rsp;
	GError *err = NULL;
	gssize ret;
	guint8 op;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	ret = transfer->data_producer(buf, len, transfer->user_data);
	if (ret > 0) {
		if (!g_obex_srm_active(transfer->obex))
			return ret;

		/* Generate next response */
		rsp = g_obex_packet_new(G_OBEX_RSP_CONTINUE, TRUE,
							G_OBEX_HDR_INVALID);
		g_obex_packet_add_body(rsp, get_get_data, transfer);

		if (!g_obex_send(transfer->obex, rsp, &err)) {
			transfer_complete(transfer, err);
			g_error_free(err);
		}

		return ret;
	}

	if (ret == -EAGAIN)
		return ret;

	if (ret == 0) {
		transfer_complete(transfer, NULL);
		return ret;
	}

	op = g_obex_errno_to_rsp(ret);

	req = g_obex_packet_new(op, TRUE, G_OBEX_HDR_INVALID);
	g_obex_send(transfer->obex, req, NULL);

	err = g_error_new(G_OBEX_ERROR, G_OBEX_ERROR_CANCELLED,
				"Data producer function failed");
	g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", err->message);
	transfer_complete(transfer, err);
	g_error_free(err);

	return ret;
}

static gboolean transfer_get_req_first(struct transfer *transfer,
							GObexPacket *rsp)
{
	GError *err = NULL;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	g_obex_packet_add_body(rsp, get_get_data, transfer);

	if (!g_obex_send(transfer->obex, rsp, &err)) {
		transfer_complete(transfer, err);
		g_error_free(err);
		return FALSE;
	}

	return TRUE;
}

static void transfer_get_req(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct transfer *transfer = user_data;
	GError *err = NULL;
	GObexPacket *rsp;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	rsp = g_obex_packet_new(G_OBEX_RSP_CONTINUE, TRUE, G_OBEX_HDR_INVALID);
	g_obex_packet_add_body(rsp, get_get_data, transfer);

	if (!g_obex_send(obex, rsp, &err)) {
		transfer_complete(transfer, err);
		g_error_free(err);
	}
}

guint g_obex_get_rsp_pkt(GObex *obex, GObexPacket *rsp,
			GObexDataProducer data_func, GObexFunc complete_func,
			gpointer user_data, GError **err)
{
	struct transfer *transfer;
	guint id;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "obex %p", obex);

	transfer = transfer_new(obex, G_OBEX_OP_GET, complete_func, user_data);
	transfer->data_producer = data_func;

	if (!transfer_get_req_first(transfer, rsp))
		return 0;

	if (!g_slist_find(transfers, transfer))
		return 0;

	id = g_obex_add_request_function(obex, G_OBEX_OP_GET, transfer_get_req,
								transfer);
	transfer->get_id = id;

	id = g_obex_add_request_function(obex, G_OBEX_OP_ABORT,
						transfer_abort_req, transfer);
	transfer->abort_id = id;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", transfer->id);

	return transfer->id;
}

guint g_obex_get_rsp(GObex *obex, GObexDataProducer data_func,
			GObexFunc complete_func, gpointer user_data,
			GError **err, guint8 first_hdr_id, ...)
{
	GObexPacket *rsp;
	va_list args;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "obex %p", obex);

	va_start(args, first_hdr_id);
	rsp = g_obex_packet_new_valist(G_OBEX_RSP_CONTINUE, TRUE,
							first_hdr_id, args);
	va_end(args);

	return g_obex_get_rsp_pkt(obex, rsp, data_func, complete_func,
							user_data, err);
}

gboolean g_obex_cancel_transfer(guint id, GObexFunc complete_func,
			gpointer user_data)
{
	struct transfer *transfer = NULL;
	gboolean ret = TRUE;

	g_obex_debug(G_OBEX_DEBUG_TRANSFER, "transfer %u", id);

	transfer = find_transfer(id);

	if (transfer == NULL)
		return FALSE;

	if (complete_func == NULL)
		goto done;

	transfer->complete_func = complete_func;
	transfer->user_data = user_data;

	if (!transfer->req_id) {
		transfer->req_id = g_obex_abort(transfer->obex,
						transfer_abort_response,
						transfer, NULL);
		if (transfer->req_id)
			return TRUE;
	}

	ret = g_obex_cancel_req(transfer->obex, transfer->req_id, FALSE);
	if (ret)
		return TRUE;

done:
	transfer_free(transfer);
	return ret;
}
