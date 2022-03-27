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
#include "config.h"
#endif

#include <stdint.h>
#include <string.h>

#include "gobex/gobex.h"
#include "gobex/gobex-header.h"

#include "util.h"

static uint8_t hdr_connid[] = { G_OBEX_HDR_CONNECTION, 1, 2, 3, 4 };
static uint8_t hdr_name_empty[] = { G_OBEX_HDR_NAME, 0x00, 0x03 };
static uint8_t hdr_name_ascii[] = { G_OBEX_HDR_NAME, 0x00, 0x0b,
				0x00, 'f', 0x00, 'o', 0x00, 'o',
				0x00, 0x00 };
static uint8_t hdr_name_umlaut[] = { G_OBEX_HDR_NAME, 0x00, 0x0b,
				0x00, 0xe5, 0x00, 0xe4, 0x00, 0xf6,
				0x00, 0x00 };
static uint8_t hdr_body[] = { G_OBEX_HDR_BODY, 0x00, 0x07, 1, 2, 3, 4 };
static uint8_t hdr_actionid[] = { G_OBEX_HDR_ACTION, 0xab };

static uint8_t hdr_uint32_nval[] = { G_OBEX_HDR_CONNECTION, 1, 2 };
static uint8_t hdr_unicode_nval_short[] = { G_OBEX_HDR_NAME, 0x12, 0x34,
						0x00, 'a', 0x00, 'b',
						0x00, 0x00 };
static uint8_t hdr_unicode_nval_data[] = { G_OBEX_HDR_NAME, 0x00, 0x01,
						0x00, 'a', 0x00, 'b' };
static uint8_t hdr_bytes_nval_short[] = { G_OBEX_HDR_BODY, 0xab, 0xcd,
						0x01, 0x02, 0x03 };
static uint8_t hdr_bytes_nval_data[] = { G_OBEX_HDR_BODY, 0xab };
static uint8_t hdr_bytes_nval_len[] = { G_OBEX_HDR_BODY, 0x00, 0x00 };
static uint8_t hdr_apparam[] = { G_OBEX_HDR_APPARAM, 0x00, 0x09, 0x00, 0x04,
						0x01, 0x02, 0x03, 0x04 };

static void test_header_name_empty(void)
{
	GObexHeader *header;
	uint8_t buf[1024];
	size_t len;

	header = g_obex_header_new_unicode(G_OBEX_HDR_NAME, "");

	g_assert(header != NULL);

	len = g_obex_header_encode(header, buf, sizeof(buf));

	assert_memequal(hdr_name_empty, sizeof(hdr_name_empty), buf, len);

	g_obex_header_free(header);
}

static void test_header_name_ascii(void)
{
	GObexHeader *header;
	uint8_t buf[1024];
	size_t len;

	header = g_obex_header_new_unicode(G_OBEX_HDR_NAME, "foo");

	g_assert(header != NULL);

	len = g_obex_header_encode(header, buf, sizeof(buf));

	assert_memequal(hdr_name_ascii, sizeof(hdr_name_ascii), buf, len);

	g_obex_header_free(header);
}

static void test_header_name_umlaut(void)
{
	GObexHeader *header;
	uint8_t buf[1024];
	size_t len;

	header = g_obex_header_new_unicode(G_OBEX_HDR_NAME, "åäö");

	g_assert(header != NULL);

	len = g_obex_header_encode(header, buf, sizeof(buf));

	assert_memequal(hdr_name_umlaut, sizeof(hdr_name_umlaut), buf, len);

	g_obex_header_free(header);
}

static void test_header_bytes(void)
{
	GObexHeader *header;
	uint8_t buf[1024], data[] = { 1, 2, 3, 4 };
	size_t len;

	header = g_obex_header_new_bytes(G_OBEX_HDR_BODY, data, sizeof(data));
	g_assert(header != NULL);

	len = g_obex_header_encode(header, buf, sizeof(buf));

	assert_memequal(hdr_body, sizeof(hdr_body), buf, len);

	g_obex_header_free(header);
}

static void test_header_apparam(void)
{
	GObexHeader *header;
	GObexApparam *apparam;
	uint8_t buf[1024];
	size_t len;

	apparam = g_obex_apparam_set_uint32(NULL, 0, 0x01020304);
	g_assert(apparam != NULL);

	header = g_obex_header_new_apparam(apparam);
	g_assert(header != NULL);

	len = g_obex_header_encode(header, buf, sizeof(buf));

	assert_memequal(hdr_apparam, sizeof(hdr_apparam), buf, len);

	g_obex_apparam_free(apparam);
	g_obex_header_free(header);
}

static void test_header_uint8(void)
{
	GObexHeader *header;
	uint8_t buf[1024];
	size_t len;

	header = g_obex_header_new_uint8(G_OBEX_HDR_ACTION, 0xab);

	g_assert(header != NULL);

	len = g_obex_header_encode(header, buf, sizeof(buf));

	assert_memequal(hdr_actionid, sizeof(hdr_actionid), buf, len);

	g_obex_header_free(header);
}

static void test_header_uint32(void)
{
	GObexHeader *header;
	uint8_t buf[1024];
	size_t len;

	header = g_obex_header_new_uint32(G_OBEX_HDR_CONNECTION, 0x01020304);
	len = g_obex_header_encode(header, buf, sizeof(buf));

	assert_memequal(hdr_connid, sizeof(hdr_connid), buf, len);

	g_obex_header_free(header);
}

static GObexHeader *parse_and_encode(uint8_t *buf, size_t buf_len)
{
	GObexHeader *header;
	uint8_t encoded[1024];
	size_t len;
	GError *err = NULL;

	header = g_obex_header_decode(buf, buf_len, G_OBEX_DATA_REF, &len,
									&err);
	g_assert_no_error(err);
	g_assert_cmpuint(len, ==, buf_len);

	len = g_obex_header_encode(header, encoded, sizeof(encoded));

	assert_memequal(buf, buf_len, encoded, len);

	return header;
}

static void test_header_encode_connid(void)
{
	GObexHeader *header;
	gboolean ret;
	guint32 val;

	header = parse_and_encode(hdr_connid, sizeof(hdr_connid));

	ret = g_obex_header_get_uint32(header, &val);

	g_assert(ret == TRUE);
	g_assert(val == 0x01020304);

	g_obex_header_free(header);
}

static void test_header_encode_name_ascii(void)
{
	GObexHeader *header;
	const char *str;
	gboolean ret;

	header = parse_and_encode(hdr_name_ascii, sizeof(hdr_name_ascii));

	ret = g_obex_header_get_unicode(header, &str);

	g_assert(ret == TRUE);
	g_assert_cmpstr(str, ==, "foo");

	g_obex_header_free(header);
}

static void test_header_encode_name_umlaut(void)
{
	GObexHeader *header;
	const char *str;
	gboolean ret;

	header = parse_and_encode(hdr_name_umlaut, sizeof(hdr_name_umlaut));

	ret = g_obex_header_get_unicode(header, &str);

	g_assert(ret == TRUE);
	g_assert_cmpstr(str, ==, "åäö");

	g_obex_header_free(header);
}

static void test_header_encode_name_empty(void)
{
	GObexHeader *header;
	const char *str;
	gboolean ret;

	header = parse_and_encode(hdr_name_empty, sizeof(hdr_name_empty));

	ret = g_obex_header_get_unicode(header, &str);

	g_assert(ret == TRUE);
	g_assert_cmpstr(str, ==, "");

	g_obex_header_free(header);
}

static void test_header_encode_body(void)
{
	GObexHeader *header;
	guint8 expected[] = { 1, 2, 3, 4};
	const guint8 *buf;
	size_t len;
	gboolean ret;

	header = parse_and_encode(hdr_body, sizeof(hdr_body));

	ret = g_obex_header_get_bytes(header, &buf, &len);

	g_assert(ret == TRUE);
	assert_memequal(expected, sizeof(expected), buf, len);

	g_obex_header_free(header);
}

static void test_header_encode_apparam(void)
{
	GObexHeader *header;
	GObexApparam *apparam;
	gboolean ret;
	guint32 data;

	header = parse_and_encode(hdr_apparam, sizeof(hdr_apparam));

	apparam = g_obex_header_get_apparam(header);
	g_assert(apparam != NULL);

	ret = g_obex_apparam_get_uint32(apparam, 0x00, &data);
	g_assert(ret == TRUE);
	g_assert(data == 0x01020304);

	g_obex_apparam_free(apparam);
	g_obex_header_free(header);
}

static void test_header_encode_actionid(void)
{
	GObexHeader *header;
	gboolean ret;
	guint8 val;

	header = parse_and_encode(hdr_actionid, sizeof(hdr_actionid));

	ret = g_obex_header_get_uint8(header, &val);

	g_assert(ret == TRUE);
	g_assert_cmpuint(val, ==, 0xab);

	g_obex_header_free(header);
}

static void test_decode_header_connid(void)
{
	GObexHeader *header;
	size_t parsed;
	GError *err = NULL;

	header = g_obex_header_decode(hdr_connid, sizeof(hdr_connid),
					G_OBEX_DATA_REF, &parsed, &err);
	g_assert_no_error(err);

	g_assert_cmpuint(parsed, ==, sizeof(hdr_connid));

	g_obex_header_free(header);
}

static void test_decode_header_name_ascii(void)
{
	GObexHeader *header;
	size_t parsed;
	GError *err = NULL;

	header = g_obex_header_decode(hdr_name_ascii, sizeof(hdr_name_ascii),
					G_OBEX_DATA_REF, &parsed, &err);
	g_assert_no_error(err);

	g_assert_cmpuint(parsed, ==, sizeof(hdr_name_ascii));

	g_obex_header_free(header);
}

static void test_decode_header_name_empty(void)
{
	GObexHeader *header;
	size_t parsed;
	GError *err = NULL;

	header = g_obex_header_decode(hdr_name_empty, sizeof(hdr_name_empty),
					G_OBEX_DATA_REF, &parsed, &err);
	g_assert_no_error(err);

	g_assert_cmpuint(parsed, ==, sizeof(hdr_name_empty));

	g_obex_header_free(header);
}

static void test_decode_header_name_umlaut(void)
{
	GObexHeader *header;
	size_t parsed;
	GError *err = NULL;

	header = g_obex_header_decode(hdr_name_umlaut, sizeof(hdr_name_umlaut),
					G_OBEX_DATA_REF, &parsed, &err);
	g_assert_no_error(err);

	g_assert_cmpuint(parsed, ==, sizeof(hdr_name_umlaut));

	g_obex_header_free(header);
}

static void test_decode_header_body(void)
{
	GObexHeader *header;
	size_t parsed;
	GError *err = NULL;

	header = g_obex_header_decode(hdr_body, sizeof(hdr_body),
					G_OBEX_DATA_COPY, &parsed, &err);
	g_assert_no_error(err);

	g_assert_cmpuint(parsed, ==, sizeof(hdr_body));

	g_obex_header_free(header);
}

static void test_decode_header_body_extdata(void)
{
	GObexHeader *header;
	size_t parsed;
	GError *err = NULL;

	header = g_obex_header_decode(hdr_body, sizeof(hdr_body),
					G_OBEX_DATA_REF, &parsed, &err);
	g_assert_no_error(err);

	g_assert_cmpuint(parsed, ==, sizeof(hdr_body));

	g_obex_header_free(header);
}

static void test_decode_header_actionid(void)
{
	GObexHeader *header;
	size_t parsed;
	GError *err = NULL;

	header = g_obex_header_decode(hdr_actionid, sizeof(hdr_actionid),
					G_OBEX_DATA_REF, &parsed, &err);
	g_assert_no_error(err);

	g_assert_cmpuint(parsed, ==, sizeof(hdr_actionid));

	g_obex_header_free(header);
}

static void decode_header_nval(uint8_t *buf, size_t len)
{
	GObexHeader *header;
	size_t parsed;
	GError *err = NULL;

	header = g_obex_header_decode(buf, len, G_OBEX_DATA_REF, &parsed,
									&err);
	g_assert_error(err, G_OBEX_ERROR, G_OBEX_ERROR_PARSE_ERROR);
	g_assert(header == NULL);
	g_error_free(err);
}

static void test_decode_header_uint32_nval(void)
{
	decode_header_nval(hdr_uint32_nval, sizeof(hdr_uint32_nval));
}

static void test_decode_header_unicode_nval_short(void)
{
	decode_header_nval(hdr_unicode_nval_short,
					sizeof(hdr_unicode_nval_short));
}

static void test_decode_header_unicode_nval_data(void)
{
	decode_header_nval(hdr_unicode_nval_data,
					sizeof(hdr_unicode_nval_data));
}

static void test_decode_header_bytes_nval_short(void)
{
	decode_header_nval(hdr_bytes_nval_short, sizeof(hdr_bytes_nval_short));
}

static void test_decode_header_bytes_nval_data(void)
{
	decode_header_nval(hdr_bytes_nval_data, sizeof(hdr_bytes_nval_data));
}

static void test_decode_header_bytes_nval_len(void)
{
	decode_header_nval(hdr_bytes_nval_len, sizeof(hdr_bytes_nval_len));
}

static void test_decode_header_multi(void)
{
	GObexHeader *header;
	GByteArray *buf;
	size_t parsed;
	GError *err = NULL;

	buf = g_byte_array_sized_new(sizeof(hdr_connid) +
					sizeof(hdr_name_ascii) +
					sizeof(hdr_actionid) +
					sizeof(hdr_body));

	g_byte_array_append(buf, hdr_connid, sizeof(hdr_connid));
	g_byte_array_append(buf, hdr_name_ascii, sizeof(hdr_name_ascii));
	g_byte_array_append(buf, hdr_actionid, sizeof(hdr_actionid));
	g_byte_array_append(buf, hdr_body, sizeof(hdr_body));

	header = g_obex_header_decode(buf->data, buf->len, G_OBEX_DATA_REF,
								&parsed, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(parsed, ==, sizeof(hdr_connid));
	g_byte_array_remove_range(buf, 0, parsed);
	g_obex_header_free(header);

	header = g_obex_header_decode(buf->data, buf->len, G_OBEX_DATA_REF,
								&parsed, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(parsed, ==, sizeof(hdr_name_ascii));
	g_byte_array_remove_range(buf, 0, parsed);
	g_obex_header_free(header);

	header = g_obex_header_decode(buf->data, buf->len, G_OBEX_DATA_REF,
								&parsed, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(parsed, ==, sizeof(hdr_actionid));
	g_byte_array_remove_range(buf, 0, parsed);
	g_obex_header_free(header);

	header = g_obex_header_decode(buf->data, buf->len, G_OBEX_DATA_REF,
								&parsed, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(parsed, ==, sizeof(hdr_body));
	g_byte_array_remove_range(buf, 0, parsed);
	g_obex_header_free(header);

	g_byte_array_unref(buf);
}

int main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/gobex/test_decode_header_connid",
						test_decode_header_connid);
	g_test_add_func("/gobex/test_decode_header_name_empty",
					test_decode_header_name_empty);
	g_test_add_func("/gobex/test_decode_header_name_ascii",
					test_decode_header_name_ascii);
	g_test_add_func("/gobex/test_decode_header_name_umlaut",
					test_decode_header_name_umlaut);
	g_test_add_func("/gobex/test_decode_header_body",
						test_decode_header_body);
	g_test_add_func("/gobex/test_decode_header_body_extdata",
					test_decode_header_body_extdata);
	g_test_add_func("/gobex/test_decode_header_actionid",
						test_decode_header_actionid);
	g_test_add_func("/gobex/test_decode_header_multi",
						test_decode_header_multi);

	g_test_add_func("/gobex/test_decode_header_uint32_nval",
					test_decode_header_uint32_nval);
	g_test_add_func("/gobex/test_decode_header_unicode_nval_short",
					test_decode_header_unicode_nval_short);
	g_test_add_func("/gobex/test_decode_header_unicode_nval_data",
					test_decode_header_unicode_nval_data);
	g_test_add_func("/gobex/test_decode_header_bytes_nval_short",
					test_decode_header_bytes_nval_short);
	g_test_add_func("/gobex/test_decode_header_bytes_nval_data",
					test_decode_header_bytes_nval_data);
	g_test_add_func("/gobex/test_decode_header_bytes_nval_len",
					test_decode_header_bytes_nval_len);

	g_test_add_func("/gobex/test_header_encode_connid",
						test_header_encode_connid);
	g_test_add_func("/gobex/test_header_encode_name_empty",
					test_header_encode_name_empty);
	g_test_add_func("/gobex/test_header_encode_name_ascii",
					test_header_encode_name_ascii);
	g_test_add_func("/gobex/test_header_encode_name_umlaut",
					test_header_encode_name_umlaut);
	g_test_add_func("/gobex/test_header_encode_body",
						test_header_encode_body);
	g_test_add_func("/gobex/test_header_encode_actionid",
						test_header_encode_actionid);
	g_test_add_func("/gobex/test_header_encode_apparam",
						test_header_encode_apparam);

	g_test_add_func("/gobex/test_header_name_empty",
						test_header_name_empty);
	g_test_add_func("/gobex/test_header_name_ascii",
						test_header_name_ascii);
	g_test_add_func("/gobex/test_header_name_umlaut",
						test_header_name_umlaut);
	g_test_add_func("/gobex/test_header_bytes", test_header_bytes);
	g_test_add_func("/gobex/test_header_uint8", test_header_uint8);
	g_test_add_func("/gobex/test_header_uint32", test_header_uint32);
	g_test_add_func("/gobex/test_header_apparam", test_header_apparam);

	return g_test_run();
}
