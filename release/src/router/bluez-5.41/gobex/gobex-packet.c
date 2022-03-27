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

#include "gobex-defs.h"
#include "gobex-packet.h"
#include "gobex-debug.h"

#define FINAL_BIT 0x80

struct _GObexPacket {
	guint8 opcode;
	gboolean final;

	GObexDataPolicy data_policy;

	union {
		void *buf;		/* Non-header data */
		const void *buf_ref;	/* Reference to non-header data */
	} data;
	gsize data_len;

	gsize hlen;		/* Length of all encoded headers */
	GSList *headers;

	GObexDataProducer get_body;
	gpointer get_body_data;
};

GObexHeader *g_obex_packet_get_header(GObexPacket *pkt, guint8 id)
{
	GSList *l;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	for (l = pkt->headers; l != NULL; l = g_slist_next(l)) {
		GObexHeader *hdr = l->data;

		if (g_obex_header_get_id(hdr) == id)
			return hdr;
	}

	return NULL;
}

GObexHeader *g_obex_packet_get_body(GObexPacket *pkt)
{
	GObexHeader *body;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	body = g_obex_packet_get_header(pkt, G_OBEX_HDR_BODY);
	if (body != NULL)
		return body;

	return g_obex_packet_get_header(pkt, G_OBEX_HDR_BODY_END);
}

guint8 g_obex_packet_get_operation(GObexPacket *pkt, gboolean *final)
{
	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	if (final)
		*final = pkt->final;

	return pkt->opcode;
}

gboolean g_obex_packet_prepend_header(GObexPacket *pkt, GObexHeader *header)
{
	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	pkt->headers = g_slist_prepend(pkt->headers, header);
	pkt->hlen += g_obex_header_get_length(header);

	return TRUE;
}

gboolean g_obex_packet_add_header(GObexPacket *pkt, GObexHeader *header)
{
	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	pkt->headers = g_slist_append(pkt->headers, header);
	pkt->hlen += g_obex_header_get_length(header);

	return TRUE;
}

gboolean g_obex_packet_add_body(GObexPacket *pkt, GObexDataProducer func,
							gpointer user_data)
{
	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	if (pkt->get_body != NULL)
		return FALSE;

	pkt->get_body = func;
	pkt->get_body_data = user_data;

	return TRUE;
}

gboolean g_obex_packet_add_unicode(GObexPacket *pkt, guint8 id,
							const char *str)
{
	GObexHeader *hdr;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	hdr = g_obex_header_new_unicode(id, str);
	if (hdr == NULL)
		return FALSE;

	return g_obex_packet_add_header(pkt, hdr);
}

gboolean g_obex_packet_add_bytes(GObexPacket *pkt, guint8 id,
						const void *data, gsize len)
{
	GObexHeader *hdr;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	hdr = g_obex_header_new_bytes(id, data, len);
	if (hdr == NULL)
		return FALSE;

	return g_obex_packet_add_header(pkt, hdr);
}

gboolean g_obex_packet_add_uint8(GObexPacket *pkt, guint8 id, guint8 val)
{
	GObexHeader *hdr;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	hdr = g_obex_header_new_uint8(id, val);
	if (hdr == NULL)
		return FALSE;

	return g_obex_packet_add_header(pkt, hdr);
}

gboolean g_obex_packet_add_uint32(GObexPacket *pkt, guint8 id, guint32 val)
{
	GObexHeader *hdr;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	hdr = g_obex_header_new_uint32(id, val);
	if (hdr == NULL)
		return FALSE;

	return g_obex_packet_add_header(pkt, hdr);
}

const void *g_obex_packet_get_data(GObexPacket *pkt, gsize *len)
{
	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	if (pkt->data_len == 0) {
		*len = 0;
		return NULL;
	}

	*len = pkt->data_len;

	switch (pkt->data_policy) {
	case G_OBEX_DATA_INHERIT:
	case G_OBEX_DATA_COPY:
		return pkt->data.buf;
	case G_OBEX_DATA_REF:
		return pkt->data.buf_ref;
	}

	g_assert_not_reached();
}

gboolean g_obex_packet_set_data(GObexPacket *pkt, const void *data, gsize len,
						GObexDataPolicy data_policy)
{
	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	if (pkt->data.buf || pkt->data.buf_ref)
		return FALSE;

	pkt->data_policy = data_policy;
	pkt->data_len = len;

	switch (data_policy) {
	case G_OBEX_DATA_COPY:
		pkt->data.buf = g_memdup(data, len);
		break;
	case G_OBEX_DATA_REF:
		pkt->data.buf_ref = data;
		break;
	case G_OBEX_DATA_INHERIT:
		pkt->data.buf = (void *) data;
		break;
	}

	return TRUE;
}

GObexPacket *g_obex_packet_new_valist(guint8 opcode, gboolean final,
					guint8 first_hdr_id, va_list args)
{
	GObexPacket *pkt;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", opcode);

	pkt = g_new0(GObexPacket, 1);

	pkt->opcode = opcode;
	pkt->final = final;
	pkt->headers = g_obex_header_create_list(first_hdr_id, args,
								&pkt->hlen);
	pkt->data_policy = G_OBEX_DATA_COPY;

	return pkt;
}

GObexPacket *g_obex_packet_new(guint8 opcode, gboolean final,
						guint8 first_hdr_id, ...)
{
	GObexPacket *pkt;
	va_list args;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", opcode);

	va_start(args, first_hdr_id);
	pkt = g_obex_packet_new_valist(opcode, final, first_hdr_id, args);
	va_end(args);

	return pkt;
}

void g_obex_packet_free(GObexPacket *pkt)
{
	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	switch (pkt->data_policy) {
	case G_OBEX_DATA_INHERIT:
	case G_OBEX_DATA_COPY:
		g_free(pkt->data.buf);
		break;
	case G_OBEX_DATA_REF:
		break;
	}

	g_slist_foreach(pkt->headers, (GFunc) g_obex_header_free, NULL);
	g_slist_free(pkt->headers);
	g_free(pkt);
}

static gboolean parse_headers(GObexPacket *pkt, const void *data, gsize len,
						GObexDataPolicy data_policy,
						GError **err)
{
	const guint8 *buf = data;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	while (len > 0) {
		GObexHeader *header;
		gsize parsed;

		header = g_obex_header_decode(buf, len, data_policy, &parsed,
									err);
		if (header == NULL)
			return FALSE;

		pkt->headers = g_slist_append(pkt->headers, header);
		pkt->hlen += parsed;

		len -= parsed;
		buf += parsed;
	}

	return TRUE;
}

static const guint8 *get_bytes(void *to, const guint8 *from, gsize count)
{
	memcpy(to, from, count);
	return (from + count);
}

GObexPacket *g_obex_packet_decode(const void *data, gsize len,
						gsize header_offset,
						GObexDataPolicy data_policy,
						GError **err)
{
	const guint8 *buf = data;
	guint16 packet_len;
	guint8 opcode;
	GObexPacket *pkt;
	gboolean final;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "");

	if (data_policy == G_OBEX_DATA_INHERIT) {
		if (!err)
			return NULL;
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_INVALID_ARGS,
							"Invalid data policy");
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", (*err)->message);
		return NULL;
	}

	if (len < 3 + header_offset) {
		if (!err)
			return NULL;
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_PARSE_ERROR,
					"Not enough data to decode packet");
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", (*err)->message);
		return NULL;
	}

	buf = get_bytes(&opcode, buf, sizeof(opcode));
	buf = get_bytes(&packet_len, buf, sizeof(packet_len));

	packet_len = g_ntohs(packet_len);
	if (packet_len != len) {
		if (!err)
			return NULL;
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_PARSE_ERROR,
				"Incorrect packet length (%u != %zu)",
				packet_len, len);
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", (*err)->message);
		return NULL;
	}

	final = (opcode & FINAL_BIT) ? TRUE : FALSE;
	opcode &= ~FINAL_BIT;

	pkt = g_obex_packet_new(opcode, final, G_OBEX_HDR_INVALID);

	if (header_offset == 0)
		goto headers;

	g_obex_packet_set_data(pkt, buf, header_offset, data_policy);
	buf += header_offset;

headers:
	if (!parse_headers(pkt, buf, len - (3 + header_offset),
							data_policy, err))
		goto failed;

	return pkt;

failed:
	g_obex_packet_free(pkt);
	return NULL;
}

static gssize get_body(GObexPacket *pkt, guint8 *buf, gsize len)
{
	guint16 u16;
	gssize ret;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	if (len < 3)
		return -ENOBUFS;

	ret = pkt->get_body(buf + 3, len - 3, pkt->get_body_data);
	if (ret < 0)
		return ret;

	if (ret > 0)
		buf[0] = G_OBEX_HDR_BODY;
	else
		buf[0] = G_OBEX_HDR_BODY_END;

	u16 = g_htons(ret + 3);
	memcpy(&buf[1], &u16, sizeof(u16));

	return ret;
}

gssize g_obex_packet_encode(GObexPacket *pkt, guint8 *buf, gsize len)
{
	gssize ret;
	gsize count;
	guint16 u16;
	GSList *l;

	g_obex_debug(G_OBEX_DEBUG_PACKET, "opcode 0x%02x", pkt->opcode);

	if (3 + pkt->data_len + pkt->hlen > len)
		return -ENOBUFS;

	buf[0] = pkt->opcode;
	if (pkt->final)
		buf[0] |= FINAL_BIT;

	if (pkt->data_len > 0) {
		if (pkt->data_policy == G_OBEX_DATA_REF)
			memcpy(&buf[3], pkt->data.buf_ref, pkt->data_len);
		else
			memcpy(&buf[3], pkt->data.buf, pkt->data_len);
	}

	count = 3 + pkt->data_len;

	for (l = pkt->headers; l != NULL; l = g_slist_next(l)) {
		GObexHeader *hdr = l->data;

		if (count >= len)
			return -ENOBUFS;

		ret = g_obex_header_encode(hdr, buf + count, len - count);
		if (ret < 0)
			return ret;

		count += ret;
	}

	if (pkt->get_body) {
		ret = get_body(pkt, buf + count, len - count);
		if (ret < 0)
			return ret;
		if (ret == 0) {
			if (pkt->opcode == G_OBEX_RSP_CONTINUE)
				buf[0] = G_OBEX_RSP_SUCCESS;
			buf[0] |= FINAL_BIT;
		}

		count += ret + 3;
	}

	u16 = g_htons(count);
	memcpy(&buf[1], &u16, sizeof(u16));

	return count;
}
