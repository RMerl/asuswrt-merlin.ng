/*
 *
 *  OBEX library with GLib integration
 *
 *  Copyright (C) 2012  Intel Corporation.
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
#include <unistd.h>
#include <errno.h>

#include "gobex-apparam.h"
#include "gobex-debug.h"

struct _GObexApparam {
	GHashTable *tags;
};

struct apparam_tag {
	guint8 id;
	guint8 len;
	union {
		/* Data is stored in network order */
		char string[0];
		guint8 data[0];
		guint8 u8;
		guint16 u16;
		guint32 u32;
		guint64 u64;
	} value;
} __attribute__ ((packed));

static struct apparam_tag *tag_new(guint8 id, guint8 len, const void *data)
{
	struct apparam_tag *tag;

	tag = g_malloc0(2 + len);
	tag->id = id;
	tag->len = len;
	memcpy(tag->value.data, data, len);

	return tag;
}

static GObexApparam *g_obex_apparam_new(void)
{
	GObexApparam *apparam;

	apparam = g_new0(GObexApparam, 1);
	apparam->tags = g_hash_table_new_full(g_direct_hash, g_direct_equal,
							NULL, g_free);

	return apparam;
}

static struct apparam_tag *apparam_tag_decode(const void *data, gsize size,
							gsize *parsed)
{
	struct apparam_tag *tag;
	const guint8 *ptr = data;
	guint8 id;
	guint8 len;

	if (size < 2)
		return NULL;

	id = ptr[0];
	len = ptr[1];

	if (len > size - 2)
		return NULL;

	tag = tag_new(id, len, ptr + 2);
	if (tag == NULL)
		return NULL;

	*parsed = 2 + tag->len;

	return tag;
}

GObexApparam *g_obex_apparam_decode(const void *data, gsize size)
{
	GObexApparam *apparam;
	GHashTable *tags;
	gsize count = 0;

	if (size < 2)
		return NULL;

	apparam = g_obex_apparam_new();

	tags = apparam->tags;
	while (count < size) {
		struct apparam_tag *tag;
		gsize parsed;
		guint id;

		tag = apparam_tag_decode(data + count, size - count, &parsed);
		if (tag == NULL)
			break;

		id = tag->id;
		g_hash_table_insert(tags, GUINT_TO_POINTER(id), tag);

		count += parsed;
	}

	if (count != size) {
		g_obex_apparam_free(apparam);
		return NULL;
	}

	return apparam;
}

static gssize tag_encode(struct apparam_tag *tag, void *buf, gsize len)
{
	gsize count = 2 + tag->len;

	if (len < count)
		return -ENOBUFS;

	memcpy(buf, tag, count);

	return count;
}

gssize g_obex_apparam_encode(GObexApparam *apparam, void *buf, gsize len)
{
	gsize count = 0;
	gssize ret;
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init(&iter, apparam->tags);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		struct apparam_tag *tag = value;

		ret = tag_encode(tag, buf + count, len - count);
		if (ret < 0)
			return ret;

		count += ret;
	}

	return count;
}

GObexApparam *g_obex_apparam_set_bytes(GObexApparam *apparam, guint8 id,
						const void *value, gsize len)
{
	struct apparam_tag *tag;
	guint uid = id;

	if (apparam == NULL)
		apparam = g_obex_apparam_new();

	tag = tag_new(id, len, value);
	g_hash_table_replace(apparam->tags, GUINT_TO_POINTER(uid), tag);

	return apparam;
}

GObexApparam *g_obex_apparam_set_uint8(GObexApparam *apparam, guint8 id,
							guint8 value)
{
	g_obex_debug(G_OBEX_DEBUG_APPARAM, "tag 0x%02x value %u", id, value);

	return g_obex_apparam_set_bytes(apparam, id, &value, 1);
}

GObexApparam *g_obex_apparam_set_uint16(GObexApparam *apparam, guint8 id,
							guint16 value)
{
	guint16 num = g_htons(value);

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "tag 0x%02x value %u", id, value);

	return g_obex_apparam_set_bytes(apparam, id, &num, 2);
}

GObexApparam *g_obex_apparam_set_uint32(GObexApparam *apparam, guint8 id,
							guint32 value)
{
	guint32 num = g_htonl(value);

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "tag 0x%02x value %u", id, value);

	return g_obex_apparam_set_bytes(apparam, id, &num, 4);
}

GObexApparam *g_obex_apparam_set_uint64(GObexApparam *apparam, guint8 id,
							guint64 value)
{
	guint64 num = GUINT64_TO_BE(value);

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "tag 0x%02x value %"
						G_GUINT64_FORMAT, id, value);

	return g_obex_apparam_set_bytes(apparam, id, &num, 8);
}

GObexApparam *g_obex_apparam_set_string(GObexApparam *apparam, guint8 id,
							const char *value)
{
	gsize len;

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "tag 0x%02x value %s", id, value);

	len = strlen(value) + 1;
	if (len > G_MAXUINT8) {
		((char *) value)[G_MAXUINT8 - 1] = '\0';
		len = G_MAXUINT8;
	}

	return g_obex_apparam_set_bytes(apparam, id, value, len);
}

static struct apparam_tag *g_obex_apparam_find_tag(GObexApparam *apparam,
								guint id)
{
	return g_hash_table_lookup(apparam->tags, GUINT_TO_POINTER(id));
}

gboolean g_obex_apparam_get_uint8(GObexApparam *apparam, guint8 id,
							guint8 *dest)
{
	struct apparam_tag *tag;

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "tag 0x%02x", id);

	tag = g_obex_apparam_find_tag(apparam, id);
	if (tag == NULL)
		return FALSE;

	*dest = tag->value.u8;

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "%u", *dest);

	return TRUE;
}

gboolean g_obex_apparam_get_uint16(GObexApparam *apparam, guint8 id,
							guint16 *dest)
{
	struct apparam_tag *tag;

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "tag 0x%02x", id);

	tag = g_obex_apparam_find_tag(apparam, id);
	if (tag == NULL)
		return FALSE;

	if (tag->len < sizeof(*dest))
		return FALSE;

	*dest = g_ntohs(tag->value.u16);

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "%u", *dest);

	return TRUE;
}

gboolean g_obex_apparam_get_uint32(GObexApparam *apparam, guint8 id,
							guint32 *dest)
{
	struct apparam_tag *tag;

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "tag 0x%02x", id);

	tag = g_obex_apparam_find_tag(apparam, id);
	if (tag == NULL)
		return FALSE;

	if (tag->len < sizeof(*dest))
		return FALSE;

	*dest = g_ntohl(tag->value.u32);

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "%u", *dest);

	return TRUE;
}

gboolean g_obex_apparam_get_uint64(GObexApparam *apparam, guint8 id,
							guint64 *dest)
{
	struct apparam_tag *tag;

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "tag 0x%02x", id);

	tag = g_obex_apparam_find_tag(apparam, id);
	if (tag == NULL)
		return FALSE;

	if (tag->len < sizeof(*dest))
		return FALSE;

	*dest = GUINT64_FROM_BE(tag->value.u64);

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "%" G_GUINT64_FORMAT, *dest);

	return TRUE;
}

char *g_obex_apparam_get_string(GObexApparam *apparam, guint8 id)
{
	struct apparam_tag *tag;
	char *string;

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "tag 0x%02x", id);

	tag = g_obex_apparam_find_tag(apparam, id);
	if (tag == NULL)
		return NULL;

	string = g_strndup(tag->value.string, tag->len);

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "%s", string);

	return string;
}

gboolean g_obex_apparam_get_bytes(GObexApparam *apparam, guint8 id,
					const guint8 **val, gsize *len)
{
	struct apparam_tag *tag;

	g_obex_debug(G_OBEX_DEBUG_APPARAM, "tag 0x%02x", id);

	tag = g_obex_apparam_find_tag(apparam, id);
	if (tag == NULL)
		return FALSE;

	*len = tag->len;
	*val = tag->value.data;

	return TRUE;
}

void g_obex_apparam_free(GObexApparam *apparam)
{
	g_hash_table_unref(apparam->tags);
	g_free(apparam);
}
