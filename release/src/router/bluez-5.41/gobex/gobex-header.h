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

#ifndef __GOBEX_HEADER_H
#define __GOBEX_HEADER_H

#include <glib.h>

#include "gobex/gobex-defs.h"
#include "gobex/gobex-apparam.h"

/* Header ID's */
#define G_OBEX_HDR_INVALID	0x00

#define G_OBEX_HDR_COUNT		0xc0
#define G_OBEX_HDR_NAME			0x01
#define G_OBEX_HDR_TYPE			0x42
#define G_OBEX_HDR_LENGTH		0xc3
#define G_OBEX_HDR_TIME			0x44
#define G_OBEX_HDR_DESCRIPTION		0x05
#define G_OBEX_HDR_TARGET		0x46
#define G_OBEX_HDR_HTTP			0x47
#define G_OBEX_HDR_BODY			0x48
#define G_OBEX_HDR_BODY_END		0x49
#define G_OBEX_HDR_WHO			0x4a
#define G_OBEX_HDR_CONNECTION		0xcb
#define G_OBEX_HDR_APPARAM		0x4c
#define G_OBEX_HDR_AUTHCHAL		0x4d
#define G_OBEX_HDR_AUTHRESP		0x4e
#define G_OBEX_HDR_CREATOR		0xcf
#define G_OBEX_HDR_WANUUID		0x50
#define G_OBEX_HDR_OBJECTCLASS		0x51
#define G_OBEX_HDR_SESSIONPARAM		0x52
#define G_OBEX_HDR_SESSIONSEQ		0x93
#define G_OBEX_HDR_ACTION		0x94
#define G_OBEX_HDR_DESTNAME		0x15
#define G_OBEX_HDR_PERMISSIONS		0xd6
#define G_OBEX_HDR_SRM			0x97
#define G_OBEX_HDR_SRMP			0x98

/* Action header values */
#define G_OBEX_ACTION_COPY		0x00
#define G_OBEX_ACTION_MOVE		0x01
#define G_OBEX_ACTION_SETPERM		0x02

/* SRM header values */
#define G_OBEX_SRM_DISABLE		0x00
#define G_OBEX_SRM_ENABLE		0x01
#define G_OBEX_SRM_INDICATE		0x02

/* SRMP header values */
#define G_OBEX_SRMP_NEXT		0x00
#define G_OBEX_SRMP_WAIT		0x01
#define G_OBEX_SRMP_NEXT_WAIT		0x02

typedef struct _GObexHeader GObexHeader;

gboolean g_obex_header_get_unicode(GObexHeader *header, const char **str);
gboolean g_obex_header_get_bytes(GObexHeader *header, const guint8 **val,
								gsize *len);
gboolean g_obex_header_get_uint8(GObexHeader *header, guint8 *val);
gboolean g_obex_header_get_uint32(GObexHeader *header, guint32 *val);
GObexApparam *g_obex_header_get_apparam(GObexHeader *header);

GObexHeader *g_obex_header_new_unicode(guint8 id, const char *str);
GObexHeader *g_obex_header_new_bytes(guint8 id, const void *data, gsize len);
GObexHeader *g_obex_header_new_uint8(guint8 id, guint8 val);
GObexHeader *g_obex_header_new_uint32(guint8 id, guint32 val);
GObexHeader *g_obex_header_new_tag(guint8 id, GObexApparam *apparam);
GObexHeader *g_obex_header_new_apparam(GObexApparam *apparam);

GSList *g_obex_header_create_list(guint8 first_hdr_id, va_list args,
							gsize *total_len);

guint8 g_obex_header_get_id(GObexHeader *header);
guint16 g_obex_header_get_length(GObexHeader *header);

gssize g_obex_header_encode(GObexHeader *header, void *buf, gsize buf_len);
GObexHeader *g_obex_header_decode(const void *data, gsize len,
				GObexDataPolicy data_policy, gsize *parsed,
				GError **err);
void g_obex_header_free(GObexHeader *header);

#endif /* __GOBEX_HEADER_H */
