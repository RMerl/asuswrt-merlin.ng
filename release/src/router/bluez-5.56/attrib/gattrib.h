/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010  Nokia Corporation
 *  Copyright (C) 2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */
#ifndef __GATTRIB_H
#define __GATTRIB_H

#ifdef __cplusplus
extern "C" {
#endif

#define GATTRIB_ALL_REQS 0xFE
#define GATTRIB_ALL_HANDLES 0x0000

struct bt_att;  /* Forward declaration for compatibility */
struct _GAttrib;
typedef struct _GAttrib GAttrib;

typedef void (*GAttribResultFunc) (guint8 status, const guint8 *pdu,
					guint16 len, gpointer user_data);
typedef void (*GAttribDisconnectFunc)(gpointer user_data);
typedef void (*GAttribDebugFunc)(const char *str, gpointer user_data);
typedef void (*GAttribNotifyFunc)(const guint8 *pdu, guint16 len,
							gpointer user_data);

GAttrib *g_attrib_new(GIOChannel *io, guint16 mtu, bool ext_signed);
GAttrib *g_attrib_ref(GAttrib *attrib);
void g_attrib_unref(GAttrib *attrib);

GIOChannel *g_attrib_get_channel(GAttrib *attrib);

struct bt_att *g_attrib_get_att(GAttrib *attrib);

gboolean g_attrib_set_destroy_function(GAttrib *attrib,
		GDestroyNotify destroy, gpointer user_data);

guint g_attrib_send(GAttrib *attrib, guint id, const guint8 *pdu, guint16 len,
			GAttribResultFunc func, gpointer user_data,
			GDestroyNotify notify);

gboolean g_attrib_cancel(GAttrib *attrib, guint id);
gboolean g_attrib_cancel_all(GAttrib *attrib);

guint g_attrib_register(GAttrib *attrib, guint8 opcode, guint16 handle,
				GAttribNotifyFunc func, gpointer user_data,
				GDestroyNotify notify);

uint8_t *g_attrib_get_buffer(GAttrib *attrib, size_t *len);
gboolean g_attrib_set_mtu(GAttrib *attrib, int mtu);

gboolean g_attrib_unregister(GAttrib *attrib, guint id);
gboolean g_attrib_unregister_all(GAttrib *attrib);

#ifdef __cplusplus
}
#endif
#endif
