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
#include <errno.h>

#include "gobex/gobex.h"
#include "gobex/gobex-packet.h"

#include "util.h"

static uint8_t pkt_connect[] = { G_OBEX_OP_CONNECT, 0x00, 0x0c,
					0x10, 0x00, 0x10, 0x00,
					G_OBEX_HDR_TARGET,
						0x00, 0x05, 0xab, 0xcd };
static uint8_t pkt_put_action[] = { G_OBEX_OP_PUT, 0x00, 0x05,
					G_OBEX_HDR_ACTION, 0xab };
static uint8_t pkt_put_body[] = { G_OBEX_OP_PUT, 0x00, 0x0a,
					G_OBEX_HDR_BODY, 0x00, 0x07,
					1, 2, 3, 4 };
static uint8_t pkt_put[] = { G_OBEX_OP_PUT, 0x00, 0x03 };

static uint8_t pkt_nval_len[] = { G_OBEX_OP_PUT, 0xab, 0xcd, 0x12 };

static guint8 pkt_put_long[] = { G_OBEX_OP_PUT, 0x00, 0x32,
	G_OBEX_HDR_CONNECTION, 0x01, 0x02, 0x03, 0x04,
	G_OBEX_HDR_TYPE, 0x00, 0x0b,
	'f', 'o', 'o', '/', 'b', 'a', 'r', '\0',
	G_OBEX_HDR_NAME, 0x00, 0x15,
	0, 'f', 0, 'i', 0, 'l', 0, 'e', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0,
	G_OBEX_HDR_ACTION, 0xab,
	G_OBEX_HDR_BODY, 0x00, 0x08,
	0, 1, 2, 3, 4 };

static void test_pkt(void)
{
	GObexPacket *pkt;

	pkt = g_obex_packet_new(G_OBEX_OP_PUT, TRUE, G_OBEX_HDR_INVALID);

	g_assert(pkt != NULL);

	g_obex_packet_free(pkt);
}

static void test_decode_pkt(void)
{
	GObexPacket *pkt;
	GError *err = NULL;

	pkt = g_obex_packet_decode(pkt_put, sizeof(pkt_put), 0,
						G_OBEX_DATA_REF, &err);
	g_assert_no_error(err);

	g_obex_packet_free(pkt);
}

static void test_decode_pkt_header(void)
{
	GObexPacket *pkt;
	GObexHeader *header;
	GError *err = NULL;
	gboolean ret;
	guint8 val;

	pkt = g_obex_packet_decode(pkt_put_action, sizeof(pkt_put_action),
						0, G_OBEX_DATA_REF, &err);
	g_assert_no_error(err);

	header = g_obex_packet_get_header(pkt, G_OBEX_HDR_ACTION);
	g_assert(header != NULL);

	ret = g_obex_header_get_uint8(header, &val);
	g_assert(ret == TRUE);
	g_assert(val == 0xab);

	g_obex_packet_free(pkt);
}

static void test_decode_connect(void)
{
	GObexPacket *pkt;
	GObexHeader *header;
	GError *err = NULL;
	gboolean ret;
	const guint8 *buf;
	guint8 target[] = { 0xab, 0xcd };
	gsize len;

	pkt = g_obex_packet_decode(pkt_connect, sizeof(pkt_connect),
						4, G_OBEX_DATA_REF, &err);
	g_assert_no_error(err);
	g_assert(pkt != NULL);

	header = g_obex_packet_get_header(pkt, G_OBEX_HDR_TARGET);
	g_assert(header != NULL);

	ret = g_obex_header_get_bytes(header, &buf, &len);
	g_assert(ret == TRUE);
	assert_memequal(target, sizeof(target), buf, len);

	g_obex_packet_free(pkt);
}

static void test_decode_nval(void)
{
	GObexPacket *pkt;
	GError *err = NULL;

	pkt = g_obex_packet_decode(pkt_nval_len, sizeof(pkt_nval_len), 0,
						G_OBEX_DATA_REF, &err);
	g_assert_error(err, G_OBEX_ERROR, G_OBEX_ERROR_PARSE_ERROR);
	g_assert(pkt == NULL);

	g_error_free(err);
}

static void test_decode_encode(void)
{
	GObexPacket *pkt;
	GError *err = NULL;
	uint8_t buf[255];
	gssize len;

	pkt = g_obex_packet_decode(pkt_put_action, sizeof(pkt_put_action),
						0, G_OBEX_DATA_REF, &err);
	g_assert_no_error(err);

	len = g_obex_packet_encode(pkt, buf, sizeof(buf));
	if (len < 0) {
		g_printerr("Encoding failed: %s\n", g_strerror(-len));
		g_assert_not_reached();
	}

	assert_memequal(pkt_put_action, sizeof(pkt_put_action), buf, len);

	g_obex_packet_free(pkt);
}

static gssize get_body_data(void *buf, gsize len, gpointer user_data)
{
	uint8_t data[] = { 1, 2, 3, 4 };

	memcpy(buf, data, sizeof(data));

	return sizeof(data);
}

static void test_encode_on_demand(void)
{
	GObexPacket *pkt;
	uint8_t buf[255];
	gssize len;

	pkt = g_obex_packet_new(G_OBEX_OP_PUT, FALSE, G_OBEX_HDR_INVALID);
	g_obex_packet_add_body(pkt, get_body_data, NULL);

	len = g_obex_packet_encode(pkt, buf, sizeof(buf));
	if (len < 0) {
		g_printerr("Encoding failed: %s\n", g_strerror(-len));
		g_assert_not_reached();
	}

	assert_memequal(pkt_put_body, sizeof(pkt_put_body), buf, len);

	g_obex_packet_free(pkt);
}

static gssize get_body_data_fail(void *buf, gsize len, gpointer user_data)
{
	return -EIO;
}

static void test_encode_on_demand_fail(void)
{
	GObexPacket *pkt;
	uint8_t buf[255];
	gssize len;

	pkt = g_obex_packet_new(G_OBEX_OP_PUT, FALSE, G_OBEX_HDR_INVALID);
	g_obex_packet_add_body(pkt, get_body_data_fail, NULL);

	len = g_obex_packet_encode(pkt, buf, sizeof(buf));

	g_assert_cmpint(len, ==, -EIO);

	g_obex_packet_free(pkt);
}

static void test_create_args(void)
{
	GObexPacket *pkt;
	guint8 buf[255], body[] = { 0x00, 0x01, 0x02, 0x03, 0x04 };
	gssize len;

	pkt = g_obex_packet_new(G_OBEX_OP_PUT, FALSE,
			G_OBEX_HDR_CONNECTION, 0x01020304,
			G_OBEX_HDR_TYPE, "foo/bar", strlen("foo/bar") + 1,
			G_OBEX_HDR_NAME, "file.txt",
			G_OBEX_HDR_ACTION, 0xab,
			G_OBEX_HDR_BODY, body, sizeof(body),
			G_OBEX_HDR_INVALID);

	g_assert(pkt != NULL);

	len = g_obex_packet_encode(pkt, buf, sizeof(buf));
	g_assert(len > 0);

	assert_memequal(pkt_put_long, sizeof(pkt_put_long), buf, len);

	g_obex_packet_free(pkt);
}

int main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/gobex/test_pkt", test_pkt);
	g_test_add_func("/gobex/test_decode_pkt", test_decode_pkt);
	g_test_add_func("/gobex/test_decode_pkt_header",
						test_decode_pkt_header);
	g_test_add_func("/gobex/test_decode_connect",
						test_decode_connect);

	g_test_add_func("/gobex/test_decode_nval", test_decode_nval);

	g_test_add_func("/gobex/test_encode_pkt", test_decode_encode);

	g_test_add_func("/gobex/test_encode_on_demand", test_encode_on_demand);
	g_test_add_func("/gobex/test_encode_on_demand_fail",
						test_encode_on_demand_fail);

	g_test_add_func("/gobex/test_create_args", test_create_args);

	return g_test_run();
}
