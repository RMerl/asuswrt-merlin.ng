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

#include "gobex-header.h"
#include "gobex-debug.h"

/* Header types */
#define G_OBEX_HDR_ENC_UNICODE	(0 << 6)
#define G_OBEX_HDR_ENC_BYTES	(1 << 6)
#define G_OBEX_HDR_ENC_UINT8	(2 << 6)
#define G_OBEX_HDR_ENC_UINT32	(3 << 6)

#define G_OBEX_HDR_ENC(id)	((id) & 0xc0)

struct _GObexHeader {
	guint8 id;
	gboolean extdata;
	gsize vlen;			/* Length of value */
	gsize hlen;			/* Length of full encoded header */
	union {
		char *string;		/* UTF-8 converted from UTF-16 */
		guint8 *data;		/* Own buffer */
		const guint8 *extdata;	/* Reference to external buffer */
		guint8 u8;
		guint32 u32;
	} v;
};

static glong utf8_to_utf16(gunichar2 **utf16, const char *utf8) {
	glong utf16_len;
	int i;

	if (*utf8 == '\0') {
		*utf16 = NULL;
		return 0;
	}

	*utf16 = g_utf8_to_utf16(utf8, -1, NULL, &utf16_len, NULL);
	if (*utf16 == NULL)
		return -1;

	/* g_utf8_to_utf16 produces host-byteorder UTF-16,
	 * but OBEX requires network byteorder (big endian) */
	for (i = 0; i < utf16_len; i++)
		(*utf16)[i] = g_htons((*utf16)[i]);

	utf16_len = (utf16_len + 1) << 1;

	return utf16_len;
}

static guint8 *put_bytes(guint8 *to, const void *from, gsize count)
{
	memcpy(to, from, count);
	return (to + count);
}

static const guint8 *get_bytes(void *to, const guint8 *from, gsize count)
{
	memcpy(to, from, count);
	return (from + count);
}

gssize g_obex_header_encode(GObexHeader *header, void *buf, gsize buf_len)
{
	guint8 *ptr = buf;
	guint16 u16;
	guint32 u32;
	gunichar2 *utf16;
	glong utf16_len;

	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x",
						G_OBEX_HDR_ENC(header->id));

	if (buf_len < header->hlen)
		return -1;

	ptr = put_bytes(ptr, &header->id, sizeof(header->id));

	switch (G_OBEX_HDR_ENC(header->id)) {
	case G_OBEX_HDR_ENC_UNICODE:
		utf16_len = utf8_to_utf16(&utf16, header->v.string);
		if (utf16_len < 0 || (guint16) utf16_len > buf_len)
			return -1;
		g_assert_cmpuint(utf16_len + 3, ==, header->hlen);
		u16 = g_htons(utf16_len + 3);
		ptr = put_bytes(ptr, &u16, sizeof(u16));
		put_bytes(ptr, utf16, utf16_len);
		g_free(utf16);
		break;
	case G_OBEX_HDR_ENC_BYTES:
		u16 = g_htons(header->hlen);
		ptr = put_bytes(ptr, &u16, sizeof(u16));
		if (header->extdata)
			put_bytes(ptr, header->v.extdata, header->vlen);
		else
			put_bytes(ptr, header->v.data, header->vlen);
		break;
	case G_OBEX_HDR_ENC_UINT8:
		*ptr = header->v.u8;
		break;
	case G_OBEX_HDR_ENC_UINT32:
		u32 = g_htonl(header->v.u32);
		put_bytes(ptr, &u32, sizeof(u32));
		break;
	default:
		g_assert_not_reached();
	}

	return header->hlen;
}

GObexHeader *g_obex_header_decode(const void *data, gsize len,
				GObexDataPolicy data_policy, gsize *parsed,
				GError **err)
{
	GObexHeader *header;
	const guint8 *ptr = data;
	guint16 hdr_len;
	gsize str_len;
	GError *conv_err = NULL;

	if (len < 2) {
		if (!err)
			return NULL;
		g_set_error(err, G_OBEX_ERROR, G_OBEX_ERROR_PARSE_ERROR,
						"Too short header in packet");
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", (*err)->message);
		return NULL;
	}

	header = g_new0(GObexHeader, 1);

	ptr = get_bytes(&header->id, ptr, sizeof(header->id));

	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x",
						G_OBEX_HDR_ENC(header->id));

	switch (G_OBEX_HDR_ENC(header->id)) {
	case G_OBEX_HDR_ENC_UNICODE:
		if (len < 3) {
			g_set_error(err, G_OBEX_ERROR,
				G_OBEX_ERROR_PARSE_ERROR,
				"Not enough data for unicode header (0x%02x)",
				header->id);
			goto failed;
		}
		ptr = get_bytes(&hdr_len, ptr, sizeof(hdr_len));
		hdr_len = g_ntohs(hdr_len);

		if (hdr_len == 3) {
			header->v.string = g_strdup("");
			header->vlen = 0;
			header->hlen = hdr_len;
			*parsed = hdr_len;
			break;
		}

		if (hdr_len > len || hdr_len < 5) {
			g_set_error(err, G_OBEX_ERROR,
				G_OBEX_ERROR_PARSE_ERROR,
				"Invalid unicode header (0x%02x) length (%u)",
				header->id, hdr_len);
			goto failed;
		}

		header->v.string = g_convert((const char *) ptr, hdr_len - 5,
						"UTF-8", "UTF-16BE",
						NULL, &str_len, &conv_err);
		if (header->v.string == NULL) {
			g_set_error(err, G_OBEX_ERROR,
					G_OBEX_ERROR_PARSE_ERROR,
					"Unicode conversion failed: %s",
					conv_err->message);
			g_error_free(conv_err);
			goto failed;
		}

		header->vlen = (gsize) str_len;
		header->hlen = hdr_len;

		*parsed = hdr_len;

		break;
	case G_OBEX_HDR_ENC_BYTES:
		if (len < 3) {
			g_set_error(err, G_OBEX_ERROR,
					G_OBEX_ERROR_PARSE_ERROR,
					"Too short byte array header");
			goto failed;
		}
		ptr = get_bytes(&hdr_len, ptr, sizeof(hdr_len));
		hdr_len = g_ntohs(hdr_len);
		if (hdr_len > len) {
			g_set_error(err, G_OBEX_ERROR,
					G_OBEX_ERROR_PARSE_ERROR,
					"Too long byte array header");
			goto failed;
		}

		if (hdr_len < 3) {
			g_set_error(err, G_OBEX_ERROR,
					G_OBEX_ERROR_PARSE_ERROR,
					"Too small byte array length");
			goto failed;
		}

		header->vlen = hdr_len - 3;
		header->hlen = hdr_len;

		switch (data_policy) {
		case G_OBEX_DATA_COPY:
			header->v.data = g_memdup(ptr, header->vlen);
			break;
		case G_OBEX_DATA_REF:
			header->extdata = TRUE;
			header->v.extdata = ptr;
			break;
		case G_OBEX_DATA_INHERIT:
		default:
			g_set_error(err, G_OBEX_ERROR,
					G_OBEX_ERROR_INVALID_ARGS,
					"Invalid data policy");
			goto failed;
		}

		*parsed = hdr_len;

		break;
	case G_OBEX_HDR_ENC_UINT8:
		header->vlen = 1;
		header->hlen = 2;
		header->v.u8 = *ptr;
		*parsed = 2;
		break;
	case G_OBEX_HDR_ENC_UINT32:
		if (len < 5) {
			g_set_error(err, G_OBEX_ERROR,
					G_OBEX_ERROR_PARSE_ERROR,
					"Too short uint32 header");
			goto failed;
		}
		header->vlen = 4;
		header->hlen = 5;
		get_bytes(&header->v.u32, ptr, sizeof(header->v.u32));
		header->v.u32 = g_ntohl(header->v.u32);
		*parsed = 5;
		break;
	default:
		g_assert_not_reached();
	}

	return header;

failed:
	if (*err)
		g_obex_debug(G_OBEX_DEBUG_ERROR, "%s", (*err)->message);
	g_obex_header_free(header);
	return NULL;
}

void g_obex_header_free(GObexHeader *header)
{
	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x",
						G_OBEX_HDR_ENC(header->id));

	switch (G_OBEX_HDR_ENC(header->id)) {
	case G_OBEX_HDR_ENC_UNICODE:
		g_free(header->v.string);
		break;
	case G_OBEX_HDR_ENC_BYTES:
		if (!header->extdata)
			g_free(header->v.data);
		break;
	case G_OBEX_HDR_ENC_UINT8:
	case G_OBEX_HDR_ENC_UINT32:
		break;
	default:
		g_assert_not_reached();
	}

	g_free(header);
}

gboolean g_obex_header_get_unicode(GObexHeader *header, const char **str)
{
	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x",
						G_OBEX_HDR_ENC(header->id));

	if (G_OBEX_HDR_ENC(header->id) != G_OBEX_HDR_ENC_UNICODE)
		return FALSE;

	*str = header->v.string;

	g_obex_debug(G_OBEX_DEBUG_HEADER, "%s", *str);

	return TRUE;
}

gboolean g_obex_header_get_bytes(GObexHeader *header, const guint8 **val,
								gsize *len)
{
	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x",
						G_OBEX_HDR_ENC(header->id));

	if (G_OBEX_HDR_ENC(header->id) != G_OBEX_HDR_ENC_BYTES)
		return FALSE;

	*len = header->vlen;

	if (header->extdata)
		*val = header->v.extdata;
	else
		*val = header->v.data;

	return TRUE;
}

GObexApparam *g_obex_header_get_apparam(GObexHeader *header)
{
	gboolean ret;
	const guint8 *val;
	gsize len;

	ret = g_obex_header_get_bytes(header, &val, &len);
	if (!ret)
		return NULL;

	return g_obex_apparam_decode(val, len);
}

gboolean g_obex_header_get_uint8(GObexHeader *header, guint8 *val)
{
	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x",
						G_OBEX_HDR_ENC(header->id));

	if (G_OBEX_HDR_ENC(header->id) != G_OBEX_HDR_ENC_UINT8)
		return FALSE;

	*val = header->v.u8;

	g_obex_debug(G_OBEX_DEBUG_HEADER, "%u", *val);

	return TRUE;
}

gboolean g_obex_header_get_uint32(GObexHeader *header, guint32 *val)
{
	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x",
						G_OBEX_HDR_ENC(header->id));

	if (G_OBEX_HDR_ENC(header->id) != G_OBEX_HDR_ENC_UINT32)
		return FALSE;

	*val = header->v.u32;

	g_obex_debug(G_OBEX_DEBUG_HEADER, "%u", *val);

	return TRUE;
}

GObexHeader *g_obex_header_new_unicode(guint8 id, const char *str)
{
	GObexHeader *header;
	gsize len;

	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x", G_OBEX_HDR_ENC(id));

	if (G_OBEX_HDR_ENC(id) != G_OBEX_HDR_ENC_UNICODE)
		return NULL;

	header = g_new0(GObexHeader, 1);

	header->id = id;

	len = g_utf8_strlen(str, -1);

	header->vlen = len;
	header->hlen = len == 0 ? 3 : 3 + ((len + 1) * 2);
	header->v.string = g_strdup(str);

	g_obex_debug(G_OBEX_DEBUG_HEADER, "%s", header->v.string);

	return header;
}

GObexHeader *g_obex_header_new_bytes(guint8 id, const void *data, gsize len)
{
	GObexHeader *header;

	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x", G_OBEX_HDR_ENC(id));

	if (G_OBEX_HDR_ENC(id) != G_OBEX_HDR_ENC_BYTES)
		return NULL;

	header = g_new0(GObexHeader, 1);

	header->id = id;
	header->vlen = len;
	header->hlen = len + 3;
	header->v.data = g_memdup(data, len);

	return header;
}

GObexHeader *g_obex_header_new_tag(guint8 id, GObexApparam *apparam)
{
	guint8 buf[1024];
	gssize len;

	len = g_obex_apparam_encode(apparam, buf, sizeof(buf));
	if (len < 0)
		return NULL;

	return g_obex_header_new_bytes(id, buf, len);
}

GObexHeader *g_obex_header_new_apparam(GObexApparam *apparam)
{
	return g_obex_header_new_tag(G_OBEX_HDR_APPARAM, apparam);
}

GObexHeader *g_obex_header_new_uint8(guint8 id, guint8 val)
{
	GObexHeader *header;

	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x", G_OBEX_HDR_ENC(id));

	if (G_OBEX_HDR_ENC(id) != G_OBEX_HDR_ENC_UINT8)
		return NULL;

	header = g_new0(GObexHeader, 1);

	header->id = id;
	header->vlen = 1;
	header->hlen = 2;
	header->v.u8 = val;

	g_obex_debug(G_OBEX_DEBUG_HEADER, "%u", header->v.u8);

	return header;
}

GObexHeader *g_obex_header_new_uint32(guint8 id, guint32 val)
{
	GObexHeader *header;

	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x", G_OBEX_HDR_ENC(id));

	if (G_OBEX_HDR_ENC(id) != G_OBEX_HDR_ENC_UINT32)
		return NULL;

	header = g_new0(GObexHeader, 1);

	header->id = id;
	header->vlen = 4;
	header->hlen = 5;
	header->v.u32 = val;

	g_obex_debug(G_OBEX_DEBUG_HEADER, "%u", header->v.u32);

	return header;
}

guint8 g_obex_header_get_id(GObexHeader *header)
{
	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x id 0x%02x",
				G_OBEX_HDR_ENC(header->id), header->id);

	return header->id;
}

guint16 g_obex_header_get_length(GObexHeader *header)
{
	g_obex_debug(G_OBEX_DEBUG_HEADER, "header 0x%02x length %zu",
				G_OBEX_HDR_ENC(header->id), header->hlen);

	return header->hlen;
}

GSList *g_obex_header_create_list(guint8 first_hdr_id, va_list args,
							gsize *total_len)
{
	unsigned int id = first_hdr_id;
	GSList *l = NULL;

	g_obex_debug(G_OBEX_DEBUG_HEADER, "");

	*total_len = 0;

	while (id != G_OBEX_HDR_INVALID) {
		GObexHeader *hdr;
		const char *str;
		const void *bytes;
		unsigned int val;
		gsize len;

		switch (G_OBEX_HDR_ENC(id)) {
		case G_OBEX_HDR_ENC_UNICODE:
			str = va_arg(args, const char *);
			hdr = g_obex_header_new_unicode(id, str);
			break;
		case G_OBEX_HDR_ENC_BYTES:
			bytes = va_arg(args, void *);
			len = va_arg(args, gsize);
			hdr = g_obex_header_new_bytes(id, bytes, len);
			break;
		case G_OBEX_HDR_ENC_UINT8:
			val = va_arg(args, unsigned int);
			hdr = g_obex_header_new_uint8(id, val);
			break;
		case G_OBEX_HDR_ENC_UINT32:
			val = va_arg(args, unsigned int);
			hdr = g_obex_header_new_uint32(id, val);
			break;
		default:
			g_assert_not_reached();
		}

		l = g_slist_append(l, hdr);
		*total_len += hdr->hlen;
		id = va_arg(args, int);
	}

	return l;
}
