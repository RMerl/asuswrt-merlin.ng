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

#ifndef __GOBEX_PACKET_H
#define __GOBEX_PACKET_H

#include <stdarg.h>
#include <glib.h>

#include "gobex/gobex-defs.h"
#include "gobex/gobex-header.h"

/* Request opcodes */
#define G_OBEX_OP_CONNECT			0x00
#define G_OBEX_OP_DISCONNECT			0x01
#define G_OBEX_OP_PUT				0x02
#define G_OBEX_OP_GET				0x03
#define G_OBEX_OP_SETPATH			0x05
#define G_OBEX_OP_ACTION			0x06
#define G_OBEX_OP_SESSION			0x07
#define G_OBEX_OP_ABORT				0x7f

/* Response codes */
#define G_OBEX_RSP_CONTINUE			0x10
#define G_OBEX_RSP_SUCCESS			0x20
#define G_OBEX_RSP_CREATED			0x21
#define G_OBEX_RSP_ACCEPTED			0x22
#define G_OBEX_RSP_NON_AUTHORITATIVE		0x23
#define G_OBEX_RSP_NO_CONTENT			0x24
#define G_OBEX_RSP_RESET_CONTENT		0x25
#define G_OBEX_RSP_PARTIAL_CONTENT		0x26
#define G_OBEX_RSP_MULTIPLE_CHOICES		0x30
#define G_OBEX_RSP_MOVED_PERMANENTLY		0x31
#define G_OBEX_RSP_MOVED_TEMPORARILY		0x32
#define G_OBEX_RSP_SEE_OTHER			0x33
#define G_OBEX_RSP_NOT_MODIFIED			0x34
#define G_OBEX_RSP_USE_PROXY			0x35
#define G_OBEX_RSP_BAD_REQUEST			0x40
#define G_OBEX_RSP_UNAUTHORIZED			0x41
#define G_OBEX_RSP_PAYMENT_REQUIRED		0x42
#define G_OBEX_RSP_FORBIDDEN			0x43
#define G_OBEX_RSP_NOT_FOUND			0x44
#define G_OBEX_RSP_METHOD_NOT_ALLOWED		0x45
#define G_OBEX_RSP_NOT_ACCEPTABLE		0x46
#define G_OBEX_RSP_PROXY_AUTH_REQUIRED		0x47
#define G_OBEX_RSP_REQUEST_TIME_OUT		0x48
#define G_OBEX_RSP_CONFLICT			0x49
#define G_OBEX_RSP_GONE				0x4a
#define G_OBEX_RSP_LENGTH_REQUIRED		0x4b
#define G_OBEX_RSP_PRECONDITION_FAILED		0x4c
#define G_OBEX_RSP_REQ_ENTITY_TOO_LARGE		0x4d
#define G_OBEX_RSP_REQ_URL_TOO_LARGE		0x4e
#define G_OBEX_RSP_UNSUPPORTED_MEDIA_TYPE	0x4f
#define G_OBEX_RSP_INTERNAL_SERVER_ERROR	0x50
#define G_OBEX_RSP_NOT_IMPLEMENTED		0x51
#define G_OBEX_RSP_BAD_GATEWAY			0x52
#define G_OBEX_RSP_SERVICE_UNAVAILABLE		0x53
#define G_OBEX_RSP_GATEWAY_TIMEOUT		0x54
#define G_OBEX_RSP_VERSION_NOT_SUPPORTED	0x55
#define G_OBEX_RSP_DATABASE_FULL		0x60
#define G_OBEX_RSP_DATABASE_LOCKED		0x61

typedef struct _GObexPacket GObexPacket;

GObexHeader *g_obex_packet_get_header(GObexPacket *pkt, guint8 id);
GObexHeader *g_obex_packet_get_body(GObexPacket *pkt);
guint8 g_obex_packet_get_operation(GObexPacket *pkt, gboolean *final);
gboolean g_obex_packet_prepend_header(GObexPacket *pkt, GObexHeader *header);
gboolean g_obex_packet_add_header(GObexPacket *pkt, GObexHeader *header);
gboolean g_obex_packet_add_body(GObexPacket *pkt, GObexDataProducer func,
							gpointer user_data);
gboolean g_obex_packet_add_unicode(GObexPacket *pkt, guint8 id,
							const char *str);
gboolean g_obex_packet_add_bytes(GObexPacket *pkt, guint8 id,
						const void *data, gsize len);
gboolean g_obex_packet_add_uint8(GObexPacket *pkt, guint8 id, guint8 val);
gboolean g_obex_packet_add_uint32(GObexPacket *pkt, guint8 id, guint32 val);
gboolean g_obex_packet_set_data(GObexPacket *pkt, const void *data, gsize len,
						GObexDataPolicy data_policy);
const void *g_obex_packet_get_data(GObexPacket *pkt, gsize *len);
GObexPacket *g_obex_packet_new(guint8 opcode, gboolean final,
						guint8 first_hdr_id, ...);
GObexPacket *g_obex_packet_new_valist(guint8 opcode, gboolean final,
					guint8 first_hdr_id, va_list args);
void g_obex_packet_free(GObexPacket *pkt);

GObexPacket *g_obex_packet_decode(const void *data, gsize len,
						gsize header_offset,
						GObexDataPolicy data_policy,
						GError **err);
gssize g_obex_packet_encode(GObexPacket *pkt, guint8 *buf, gsize len);

#endif /* __GOBEX_PACKET_H */
