/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX library with GLib integration
 *
 *  Copyright (C) 2012  Intel Corporation.
 *
 */

#ifndef __GOBEX_APPARAM_H
#define __GOBEX_APPARAM_H

#include <glib.h>

typedef struct _GObexApparam GObexApparam;

GObexApparam *g_obex_apparam_decode(const void *data, gsize size);
gssize g_obex_apparam_encode(GObexApparam *apparam, void *buf, gsize size);

GObexApparam *g_obex_apparam_set_bytes(GObexApparam *apparam, guint8 id,
						const void *value, gsize size);
GObexApparam *g_obex_apparam_set_uint8(GObexApparam *apparam, guint8 id,
							guint8 value);
GObexApparam *g_obex_apparam_set_uint16(GObexApparam *apparam, guint8 id,
							guint16 value);
GObexApparam *g_obex_apparam_set_uint32(GObexApparam *apparam, guint8 id,
							guint32 value);
GObexApparam *g_obex_apparam_set_uint64(GObexApparam *apparam, guint8 id,
							guint64 value);
GObexApparam *g_obex_apparam_set_string(GObexApparam *apparam, guint8 id,
							const char *value);

gboolean g_obex_apparam_get_bytes(GObexApparam *apparam, guint8 id,
					const guint8 **val, gsize *len);
gboolean g_obex_apparam_get_uint8(GObexApparam *apparam, guint8 id,
							guint8 *value);
gboolean g_obex_apparam_get_uint16(GObexApparam *apparam, guint8 id,
							guint16 *value);
gboolean g_obex_apparam_get_uint32(GObexApparam *apparam, guint8 id,
							guint32 *value);
gboolean g_obex_apparam_get_uint64(GObexApparam *apparam, guint8 id,
							guint64 *value);
char *g_obex_apparam_get_string(GObexApparam *apparam, guint8 id);

void g_obex_apparam_free(GObexApparam *apparam);

#endif /* __GOBEX_APPARAM_H */
