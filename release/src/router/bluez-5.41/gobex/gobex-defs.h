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

#ifndef __GOBEX_DEFS_H
#define __GOBEX_DEFS_H

#include <glib.h>

typedef enum {
	G_OBEX_DATA_INHERIT,
	G_OBEX_DATA_COPY,
	G_OBEX_DATA_REF,
} GObexDataPolicy;

#define G_OBEX_ERROR_FIRST (0xff + 1)
#define G_OBEX_PROTO_ERROR(code) ((code) < G_OBEX_ERROR_FIRST)

typedef enum {
	G_OBEX_ERROR_PARSE_ERROR = G_OBEX_ERROR_FIRST,
	G_OBEX_ERROR_INVALID_ARGS,
	G_OBEX_ERROR_DISCONNECTED,
	G_OBEX_ERROR_TIMEOUT,
	G_OBEX_ERROR_CANCELLED,
	G_OBEX_ERROR_FAILED,
} GObexError;

typedef gssize (*GObexDataProducer) (void *buf, gsize len, gpointer user_data);
typedef gboolean (*GObexDataConsumer) (const void *buf, gsize len,
							gpointer user_data);

#define G_OBEX_ERROR g_obex_error_quark()
GQuark g_obex_error_quark(void);

#endif /* __GOBEX_DEFS_H */
