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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

#include "gobex/gobex.h"

#include "util.h"

#define FINAL_BIT 0x80
#define RANDOM_PACKETS 4

static guint8 put_req_first[] = { G_OBEX_OP_PUT, 0x00, 0x30,
	G_OBEX_HDR_TYPE, 0x00, 0x0b,
	'f', 'o', 'o', '/', 'b', 'a', 'r', '\0',
	G_OBEX_HDR_NAME, 0x00, 0x15,
	0, 'f', 0, 'i', 0, 'l', 0, 'e', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0,
	G_OBEX_HDR_BODY, 0x00, 0x0d,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

static guint8 put_req_first_srm[] = { G_OBEX_OP_PUT, 0x00, 0x32,
	G_OBEX_HDR_TYPE, 0x00, 0x0b,
	'f', 'o', 'o', '/', 'b', 'a', 'r', '\0',
	G_OBEX_HDR_NAME, 0x00, 0x15,
	0, 'f', 0, 'i', 0, 'l', 0, 'e', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0,
	G_OBEX_HDR_SRM, G_OBEX_SRM_ENABLE,
	G_OBEX_HDR_BODY, 0x00, 0x0d,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

static guint8 put_req_zero[255] = { G_OBEX_OP_PUT, 0x00, 0xff,
	G_OBEX_HDR_BODY, 0x00, 0xfc };

static guint8 put_req_last[] = { G_OBEX_OP_PUT | FINAL_BIT, 0x00, 0x06,
					G_OBEX_HDR_BODY_END, 0x00, 0x03 };

static guint8 abort_req[] = { G_OBEX_OP_ABORT | FINAL_BIT, 0x00, 0x03 };

static guint8 put_rsp_first[] = { G_OBEX_RSP_CONTINUE | FINAL_BIT,
								0x00, 0x03 };
static guint8 put_rsp_first_srm[] = { G_OBEX_RSP_CONTINUE | FINAL_BIT,
							0x00, 0x05,
							G_OBEX_HDR_SRM, 0x01 };
static guint8 put_rsp_first_srm_wait[] = { G_OBEX_RSP_CONTINUE | FINAL_BIT,
							0x00, 0x07,
							G_OBEX_HDR_SRM, 0x01,
							G_OBEX_HDR_SRMP, 0x01 };
static guint8 put_rsp_last[] = { G_OBEX_RSP_SUCCESS | FINAL_BIT, 0x00, 0x03 };

static guint8 get_req_first[] = { G_OBEX_OP_GET | FINAL_BIT, 0x00, 0x23,
	G_OBEX_HDR_TYPE, 0x00, 0x0b,
	'f', 'o', 'o', '/', 'b', 'a', 'r', '\0',
	G_OBEX_HDR_NAME, 0x00, 0x15,
	0, 'f', 0, 'i', 0, 'l', 0, 'e', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0 };

static guint8 get_req_first_app[] = { G_OBEX_OP_GET | FINAL_BIT, 0x00, 0x2a,
	G_OBEX_HDR_TYPE, 0x00, 0x0b,
	'f', 'o', 'o', '/', 'b', 'a', 'r', '\0',
	G_OBEX_HDR_NAME, 0x00, 0x15,
	0, 'f', 0, 'i', 0, 'l', 0, 'e', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0,
	G_OBEX_HDR_APPARAM, 0x00, 0x07,
	0, 1, 2, 3  };

static guint8 get_req_first_srm[] = { G_OBEX_OP_GET | FINAL_BIT, 0x00, 0x25,
	G_OBEX_HDR_SRM, 0x01,
	G_OBEX_HDR_TYPE, 0x00, 0x0b,
	'f', 'o', 'o', '/', 'b', 'a', 'r', '\0',
	G_OBEX_HDR_NAME, 0x00, 0x15,
	0, 'f', 0, 'i', 0, 'l', 0, 'e', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0 };

static guint8 get_req_first_srm_wait[] = { G_OBEX_OP_GET | FINAL_BIT, 0x00, 0x27,
	G_OBEX_HDR_SRM, 0x01,
	G_OBEX_HDR_TYPE, 0x00, 0x0b,
	'f', 'o', 'o', '/', 'b', 'a', 'r', '\0',
	G_OBEX_HDR_NAME, 0x00, 0x15,
	0, 'f', 0, 'i', 0, 'l', 0, 'e', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0,
	G_OBEX_HDR_SRMP, 0x01 };

static guint8 get_req_srm_wait[] = { G_OBEX_OP_GET | FINAL_BIT, 0x00, 0x05,
	G_OBEX_HDR_SRMP, 0x01 };

static guint8 get_req_last[] = { G_OBEX_OP_GET | FINAL_BIT, 0x00, 0x03, };

static guint8 get_rsp_first_app[] = { G_OBEX_RSP_CONTINUE | FINAL_BIT, 0x00, 0x0A,
					G_OBEX_HDR_APPARAM, 0x00, 0x07,
					0, 1, 2, 3 };
static guint8 get_rsp_first[] = { G_OBEX_RSP_CONTINUE | FINAL_BIT, 0x00, 0x10,
					G_OBEX_HDR_BODY, 0x00, 0x0d,
					0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
static guint8 get_rsp_first_srm[] = { G_OBEX_RSP_CONTINUE | FINAL_BIT, 0x00, 0x12,
					G_OBEX_HDR_SRM, 0x01,
					G_OBEX_HDR_BODY, 0x00, 0x0d,
					0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
static guint8 get_rsp_first_srm_wait_next[] = { G_OBEX_RSP_CONTINUE | FINAL_BIT,
					0x00, 0x14,
					G_OBEX_HDR_SRM, 0x01,
					G_OBEX_HDR_SRMP, 0x02,
					G_OBEX_HDR_BODY, 0x00, 0x0d,
					0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
static guint8 get_rsp_srm_wait[] = { G_OBEX_RSP_CONTINUE | FINAL_BIT,
					0x00, 0x03 };
static guint8 get_rsp_zero[255] = { G_OBEX_RSP_CONTINUE | FINAL_BIT, 0x00, 0xff,
					G_OBEX_HDR_BODY, 0x00, 0xfc };
static guint8 get_rsp_zero_wait_next[255] = { G_OBEX_RSP_CONTINUE | FINAL_BIT,
					0x00, 0xff,
					G_OBEX_HDR_SRMP, 0x02,
					G_OBEX_HDR_BODY, 0x00, 0xfa };
static guint8 get_rsp_last[] = { G_OBEX_RSP_SUCCESS | FINAL_BIT, 0x00, 0x06,
					G_OBEX_HDR_BODY_END, 0x00, 0x03 };

static guint8 conn_req[] = { G_OBEX_OP_CONNECT | FINAL_BIT, 0x00, 0x07,
					0x10, 0x00, 0x10, 0x00 };
static guint8 conn_rsp[] = { G_OBEX_RSP_SUCCESS | FINAL_BIT, 0x00, 0x0c,
					0x10, 0x00, 0x10, 0x00,
					G_OBEX_HDR_CONNECTION, 0x00, 0x00,
					0x00, 0x01 };

static guint8 conn_req_srm[] = { G_OBEX_OP_CONNECT | FINAL_BIT, 0x00, 0x09,
					0x10, 0x00, 0x10, 0x00,
					G_OBEX_HDR_SRM, 0x02 };
static guint8 conn_rsp_srm[] = { G_OBEX_RSP_SUCCESS | FINAL_BIT, 0x00, 0x0e,
					0x10, 0x00, 0x10, 0x00,
					G_OBEX_HDR_CONNECTION, 0x00, 0x00,
					0x00, 0x01,
					G_OBEX_HDR_SRM, 0x01 };

static guint8 unavailable_rsp[] = { G_OBEX_RSP_SERVICE_UNAVAILABLE | FINAL_BIT,
					0x00, 0x03 };

static guint8 conn_get_req_first[] = { G_OBEX_OP_GET | FINAL_BIT, 0x00, 0x28,
	G_OBEX_HDR_CONNECTION, 0x00, 0x00, 0x00, 0x01,
	G_OBEX_HDR_TYPE, 0x00, 0x0b,
	'f', 'o', 'o', '/', 'b', 'a', 'r', '\0',
	G_OBEX_HDR_NAME, 0x00, 0x15,
	0, 'f', 0, 'i', 0, 'l', 0, 'e', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0 };

static guint8 conn_get_req_wrg[] = { G_OBEX_OP_GET | FINAL_BIT, 0x00, 0x28,
	G_OBEX_HDR_CONNECTION, 0x00, 0x00, 0x00, 0xFF,
	G_OBEX_HDR_TYPE, 0x00, 0x0b,
	'f', 'o', 'o', '/', 'b', 'a', 'r', '\0',
	G_OBEX_HDR_NAME, 0x00, 0x15,
	0, 'f', 0, 'i', 0, 'l', 0, 'e', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0 };

static guint8 conn_put_req_first[] = { G_OBEX_OP_PUT, 0x00, 0x35,
	G_OBEX_HDR_CONNECTION, 0x00, 0x00, 0x00, 0x01,
	G_OBEX_HDR_TYPE, 0x00, 0x0b,
	'f', 'o', 'o', '/', 'b', 'a', 'r', '\0',
	G_OBEX_HDR_NAME, 0x00, 0x15,
	0, 'f', 0, 'i', 0, 'l', 0, 'e', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0,
	G_OBEX_HDR_BODY, 0x00, 0x0d,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

static guint8 hdr_type[] = "foo/bar";
static guint8 hdr_app[] = { 0, 1, 2, 3 };
static guint8 body_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

static void transfer_complete(GObex *obex, GError *err, gpointer user_data)
{
	struct test_data *d = user_data;

	if (err != NULL)
		d->err = g_error_copy(err);

	g_main_loop_quit(d->mainloop);
}

static gboolean resume_obex(gpointer user_data)
{
	struct test_data *d = user_data;

	if (!g_main_loop_is_running(d->mainloop))
		return FALSE;

	g_obex_resume(d->obex);

	return FALSE;
}

static gssize provide_seq(void *buf, gsize len, gpointer user_data)
{
	struct test_data *d = user_data;
	int fd;
	gssize ret;

	if (d->count == RANDOM_PACKETS - 1)
		return 0;

	fd = open("/dev/urandom", O_RDONLY | O_NOCTTY, 0);
	if (fd < 0) {
		g_set_error(&d->err, TEST_ERROR, TEST_ERROR_UNEXPECTED,
				"open(/dev/urandom): %s", strerror(errno));
		g_main_loop_quit(d->mainloop);
		return -1;
	}

	ret = read(fd, buf, len);
	close(fd);
	return ret;
}

static gssize provide_eagain(void *buf, gsize len, gpointer user_data)
{
	struct test_data *d = user_data;

	if (d->count > 0)
		return 0;

	if (len < sizeof(body_data)) {
		g_set_error(&d->err, TEST_ERROR, TEST_ERROR_UNEXPECTED,
				"Got data request for only %zu bytes", len);
		g_main_loop_quit(d->mainloop);
		return -1;
	}

	if (d->provide_delay > 0) {
		g_timeout_add(d->provide_delay, resume_obex, d);
		d->provide_delay = 0;
		return -EAGAIN;
	}

	memcpy(buf, body_data, sizeof(body_data));

	return sizeof(body_data);
}

static gssize provide_data(void *buf, gsize len, gpointer user_data)
{
	struct test_data *d = user_data;

	if (d->total > 0)
		return 0;

	if (len < sizeof(body_data)) {
		g_set_error(&d->err, TEST_ERROR, TEST_ERROR_UNEXPECTED,
				"Got data request for only %zu bytes", len);
		g_main_loop_quit(d->mainloop);
		return -1;
	}

	memcpy(buf, body_data, sizeof(body_data));

	if (d->provide_delay > 0) {
		g_obex_suspend(d->obex);
		g_timeout_add(d->provide_delay, resume_obex, d);
	}

	d->total += sizeof(body_data);

	return sizeof(body_data);
}

static void test_put_req(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ put_req_first, sizeof(put_req_first) },
				{ put_req_last, sizeof(put_req_last) } }, {
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_last, sizeof(put_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_put_req(obex, provide_data, transfer_complete, &d, &d.err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 2);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static gboolean rcv_data(const void *buf, gsize len, gpointer user_data)
{
	struct test_data *d = user_data;

	if (len != sizeof(body_data))
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected byte count %zu", len);

	if (memcmp(buf, body_data, sizeof(body_data)) != 0) {
		dump_bufs(body_data, sizeof(body_data), buf, len);
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected byte count %zu", len);
	}

	return TRUE;
}

static void handle_put(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct test_data *d = user_data;
	guint8 op = g_obex_packet_get_operation(req, NULL);
	guint id;

	if (op != G_OBEX_OP_PUT) {
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected opcode 0x%02x", op);
		g_main_loop_quit(d->mainloop);
		return;
	}

	id = g_obex_put_rsp(obex, req, rcv_data, transfer_complete, d, &d->err,
							G_OBEX_HDR_INVALID);
	if (id == 0)
		g_main_loop_quit(d->mainloop);
}

static void test_put_rsp(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_last, sizeof(put_rsp_last) } }, {
				{ put_req_last, sizeof(put_req_last) },
				{ NULL, 0 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_PUT, handle_put, &d);

	g_io_channel_write_chars(io, (char *) put_req_first,
					sizeof(put_req_first), NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 1);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static gboolean rcv_seq(const void *buf, gsize len, gpointer user_data)
{
	return TRUE;
}

static void handle_put_seq(GObex *obex, GObexPacket *req,
							gpointer user_data)
{
	struct test_data *d = user_data;
	guint8 op = g_obex_packet_get_operation(req, NULL);
	guint id;

	if (op != G_OBEX_OP_PUT) {
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected opcode 0x%02x", op);
		g_main_loop_quit(d->mainloop);
		return;
	}

	id = g_obex_put_rsp(obex, req, rcv_seq, transfer_complete, d,
						&d->err, G_OBEX_HDR_INVALID);
	if (id == 0)
		g_main_loop_quit(d->mainloop);
}

static void test_stream_put_rsp(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_last, sizeof(put_rsp_last) } }, {
				{ put_req_zero, sizeof(put_req_zero) },
				{ put_req_zero, sizeof(put_req_zero) },
				{ put_req_last, sizeof(put_req_last) },
				{ NULL, -1 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_PUT, handle_put_seq,
									&d);

	g_io_channel_write_chars(io, (char *) put_req_first,
					sizeof(put_req_first), NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS - 1);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static gboolean cancel_transfer(gpointer user_data)
{
	struct test_data *d = user_data;

	if (d->id > 0)
		g_obex_cancel_transfer(d->id, transfer_complete, user_data);

	return FALSE;
}

static gssize abort_data(void *buf, gsize len, gpointer user_data)
{
	g_idle_add_full(G_PRIORITY_HIGH, cancel_transfer, user_data, NULL);
	return provide_data(buf, len, user_data);
}

static void test_stream_put_req_abort(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ put_req_first, sizeof(put_req_first) },
				{ abort_req, sizeof(abort_req) } }, {
				{ put_rsp_last, sizeof(put_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	d.id = g_obex_put_req(obex, abort_data, transfer_complete, &d, &d.err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 2);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_error(d.err, G_OBEX_ERROR, G_OBEX_ERROR_CANCELLED);
	g_error_free(d.err);
}

static void test_stream_put_rsp_abort(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_last, sizeof(put_rsp_last) } }, {
				{ put_req_zero, sizeof(put_req_zero) },
				{ abort_req, sizeof(abort_req) },
				{ NULL, -1 },
				{ NULL, -1 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_PUT, handle_put_seq, &d);

	g_io_channel_write_chars(io, (char *) put_req_first,
					sizeof(put_req_first), NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS - 2);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_error(d.err, G_OBEX_ERROR, G_OBEX_ERROR_CANCELLED);
	g_error_free(d.err);
}

static void handle_put_seq_wait(GObex *obex, GObexPacket *req,
							gpointer user_data)
{
	struct test_data *d = user_data;
	guint8 op = g_obex_packet_get_operation(req, NULL);
	guint id;

	if (op != G_OBEX_OP_PUT) {
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected opcode 0x%02x", op);
		g_main_loop_quit(d->mainloop);
		return;
	}

	id = g_obex_put_rsp(obex, req, rcv_seq, transfer_complete, d,
					&d->err,
					G_OBEX_HDR_SRMP, G_OBEX_SRMP_WAIT,
					G_OBEX_HDR_INVALID);
	if (id == 0)
		g_main_loop_quit(d->mainloop);
}

static void test_packet_put_rsp_wait(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
		{ put_rsp_first_srm_wait, sizeof(put_rsp_first_srm_wait) },
		{ put_rsp_first, sizeof(put_rsp_first) },
		{ NULL, -1 },
		{ put_rsp_last, sizeof(put_rsp_last) } }, {
		{ put_req_zero, sizeof(put_req_zero) },
		{ put_req_zero, sizeof(put_req_zero) },
		{ put_req_last, sizeof(put_req_last) },
		{ NULL, 0 } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_PUT,
						handle_put_seq_wait, &d);

	g_io_channel_write_chars(io, (char *) put_req_first_srm,
					sizeof(put_req_first_srm), NULL,
					&d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_packet_put_rsp(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ put_rsp_first_srm, sizeof(put_rsp_first_srm) },
			{ NULL, -1 },
			{ NULL, -1 },
			{ put_rsp_last, sizeof(put_rsp_last) } }, {
			{ put_req_zero, sizeof(put_req_zero) },
			{ put_req_zero, sizeof(put_req_zero) },
			{ put_req_last, sizeof(put_req_last) },
			{ NULL, 0 } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_PUT, handle_put_seq, &d);

	g_io_channel_write_chars(io, (char *) put_req_first_srm,
					sizeof(put_req_first_srm), NULL,
					&d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_get_req(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ get_req_first, sizeof(get_req_first) },
				{ get_req_last, sizeof(get_req_last) } }, {
				{ get_rsp_first, sizeof(get_rsp_first) },
				{ get_rsp_last, sizeof(get_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_get_req(obex, rcv_data, transfer_complete, &d, &d.err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 2);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_stream_get_req(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ get_req_first, sizeof(get_req_first) },
				{ NULL, 0 },
				{ NULL, 0 },
				{ get_req_last, sizeof(get_req_last) } }, {
				{ get_rsp_first, sizeof(get_rsp_first) },
				{ get_rsp_zero, sizeof(get_rsp_zero) },
				{ get_rsp_zero, sizeof(get_rsp_zero) },
				{ get_rsp_last, sizeof(get_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_get_req(obex, rcv_seq, transfer_complete, &d, &d.err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_packet_get_req(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ get_req_first_srm, sizeof(get_req_first_srm) },
			{ NULL, -1 },
			{ NULL, -1 },
			{ get_req_last, sizeof(get_req_last) } }, {
			{ get_rsp_first_srm, sizeof(get_rsp_first_srm) },
			{ get_rsp_zero, sizeof(get_rsp_zero) },
			{ get_rsp_zero, sizeof(get_rsp_zero) },
			{ get_rsp_last, sizeof(get_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_get_req(obex, rcv_seq, transfer_complete, &d, &d.err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_packet_get_req_wait(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
		{ get_req_first_srm_wait, sizeof(get_req_first_srm_wait) },
		{ get_req_last, sizeof(get_req_last) },
		{ NULL, -1 },
		{ get_req_last, sizeof(get_req_last) } }, {
		{ get_rsp_first_srm, sizeof(get_rsp_first_srm) },
		{ get_rsp_zero, sizeof(get_rsp_zero) },
		{ get_rsp_zero, sizeof(get_rsp_zero) },
		{ get_rsp_last, sizeof(get_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_get_req(obex, rcv_seq, transfer_complete, &d, &d.err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_SRMP, G_OBEX_SRMP_WAIT,
				G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static gboolean rcv_seq_delay(const void *buf, gsize len, gpointer user_data)
{
	struct test_data *d = user_data;

	if (d->provide_delay > 0) {
		g_obex_suspend(d->obex);
		g_timeout_add_seconds(d->provide_delay, resume_obex, d);
		d->provide_delay = 0;
	}

	return TRUE;
}

static void test_packet_get_req_suspend_resume(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
		{ get_req_first_srm, sizeof(get_req_first_srm) },
		{ get_req_srm_wait, sizeof(get_req_srm_wait) },
		{ get_req_srm_wait, sizeof(get_req_srm_wait) },
		{ get_req_last, sizeof(get_req_last) } }, {
		{ get_rsp_first_srm, sizeof(get_rsp_first_srm) },
		{ get_rsp_srm_wait, sizeof(get_rsp_srm_wait) },
		{ get_rsp_srm_wait, sizeof(get_rsp_srm_wait) },
		{ get_rsp_last, sizeof(get_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);
	d.obex = obex;
	d.provide_delay = 1;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_get_req(obex, rcv_seq_delay, transfer_complete, &d, &d.err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 4);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_packet_get_req_wait_next(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
		{ get_req_first_srm, sizeof(get_req_first_srm) },
		{ get_req_last, sizeof(get_req_last) },
		{ get_req_last, sizeof(get_req_last) },
		{ get_req_last, sizeof(get_req_last) } }, {
		{ get_rsp_first_srm_wait_next,
		sizeof(get_rsp_first_srm_wait_next) },
		{ get_rsp_zero_wait_next, sizeof(get_rsp_zero_wait_next) },
		{ get_rsp_zero_wait_next, sizeof(get_rsp_zero_wait_next) },
		{ get_rsp_last, sizeof(get_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_get_req(obex, rcv_seq, transfer_complete, &d, &d.err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_get_req_app(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ get_req_first_app, sizeof(get_req_first_app) },
			{ get_req_last, sizeof(get_req_last) },
			{ get_req_last, sizeof(get_req_last) } }, {
			{ get_rsp_first_app, sizeof(get_rsp_first_app) },
			{ get_rsp_first, sizeof(get_rsp_first) },
			{ get_rsp_last, sizeof(get_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_get_req(obex, rcv_data, transfer_complete, &d, &d.err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_APPARAM, hdr_app, sizeof(hdr_app),
				G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 3);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void handle_get_eagain(GObex *obex, GObexPacket *req,
						gpointer user_data)
{
	struct test_data *d = user_data;
	guint8 op = g_obex_packet_get_operation(req, NULL);
	guint id;

	if (op != G_OBEX_OP_GET) {
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected opcode 0x%02x", op);
		g_main_loop_quit(d->mainloop);
		return;
	}

	id = g_obex_get_rsp(obex, provide_eagain, transfer_complete, d,
						&d->err, G_OBEX_HDR_INVALID);
	if (id == 0)
		g_main_loop_quit(d->mainloop);
}

static void handle_get(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct test_data *d = user_data;
	guint8 op = g_obex_packet_get_operation(req, NULL);
	guint id;

	if (op != G_OBEX_OP_GET) {
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected opcode 0x%02x", op);
		g_main_loop_quit(d->mainloop);
		return;
	}

	id = g_obex_get_rsp(obex, provide_data, transfer_complete, d, &d->err,
							G_OBEX_HDR_INVALID);
	if (id == 0)
		g_main_loop_quit(d->mainloop);
}

static void test_stream_put_req(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ NULL, 0 },
				{ NULL, 0 },
				{ NULL, 0 },
				{ put_req_last, sizeof(put_req_last) } }, {
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_last, sizeof(put_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_put_req(obex, provide_seq, transfer_complete, &d, &d.err,
					G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
					G_OBEX_HDR_NAME, "random.bin",
					G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static gssize provide_seq_delay(void *buf, gsize len, gpointer user_data)
{
	struct test_data *d = user_data;
	int fd;
	gssize ret;

	if (d->provide_delay > 0) {
		g_obex_suspend(d->obex);
		g_timeout_add_seconds(d->provide_delay, resume_obex, d);
		d->provide_delay = 0;
	}

	if (d->count == RANDOM_PACKETS - 1)
		return 0;

	fd = open("/dev/urandom", O_RDONLY | O_NOCTTY, 0);
	if (fd < 0) {
		g_set_error(&d->err, TEST_ERROR, TEST_ERROR_UNEXPECTED,
				"open(/dev/urandom): %s", strerror(errno));
		g_main_loop_quit(d->mainloop);
		return -1;
	}

	ret = read(fd, buf, len);
	close(fd);
	return ret;
}

static void test_packet_put_req_suspend_resume(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ NULL, 0 },
			{ NULL, 0 },
			{ NULL, 0 },
			{ put_req_last, sizeof(put_req_last) } }, {
			{ put_rsp_first_srm, sizeof(put_rsp_first_srm) },
			{ NULL, 0 },
			{ NULL, 0 },
			{ put_rsp_last, sizeof(put_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);
	d.obex = obex;
	d.provide_delay = 1;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	g_obex_put_req(obex, provide_seq_delay, transfer_complete, &d, &d.err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "random.bin",
				G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_packet_put_req_wait(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
		{ NULL, 0 },
		{ NULL, 0 },
		{ NULL, 0 },
		{ put_req_last, sizeof(put_req_last) } }, {
		{ put_rsp_first_srm_wait, sizeof(put_rsp_first_srm_wait) },
		{ put_rsp_first, sizeof(put_rsp_first) },
		{ NULL, 0 },
		{ put_rsp_last, sizeof(put_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_put_req(obex, provide_seq, transfer_complete, &d, &d.err,
					G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
					G_OBEX_HDR_NAME, "random.bin",
					G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_packet_put_req(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ NULL, 0 },
			{ NULL, 0 },
			{ NULL, 0 },
			{ put_req_last, sizeof(put_req_last) } }, {
			{ put_rsp_first_srm, sizeof(put_rsp_first_srm) },
			{ NULL, 0 },
			{ NULL, 0 },
			{ put_rsp_last, sizeof(put_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_put_req(obex, provide_seq, transfer_complete, &d, &d.err,
					G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
					G_OBEX_HDR_NAME, "random.bin",
					G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_put_req_eagain(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ put_req_first, sizeof(put_req_first) },
				{ put_req_last, sizeof(put_req_last) } }, {
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_last, sizeof(put_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;
	d.provide_delay = 200;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_put_req(obex, provide_eagain, transfer_complete, &d, &d.err,
					G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
					G_OBEX_HDR_NAME, "file.txt",
					G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 2);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_get_rsp(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ get_rsp_first, sizeof(get_rsp_first) },
				{ get_rsp_last, sizeof(get_rsp_last) } }, {
				{ get_req_last, sizeof(get_req_last) },
				{ NULL, 0 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_GET, handle_get, &d);

	g_io_channel_write_chars(io, (char *) get_req_first,
					sizeof(get_req_first), NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 1);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void handle_get_seq(GObex *obex, GObexPacket *req,
							gpointer user_data)
{
	struct test_data *d = user_data;
	guint8 op = g_obex_packet_get_operation(req, NULL);
	guint id;

	if (op != G_OBEX_OP_GET) {
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected opcode 0x%02x", op);
		g_main_loop_quit(d->mainloop);
		return;
	}

	id = g_obex_get_rsp(obex, provide_seq, transfer_complete, d,
						&d->err, G_OBEX_HDR_INVALID);
	if (id == 0)
		g_main_loop_quit(d->mainloop);
}

static void test_stream_get_rsp(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ NULL, 0 },
				{ NULL, 0 },
				{ NULL, 0 },
				{ get_rsp_last, sizeof(get_rsp_last) } }, {
				{ get_req_last, sizeof(get_req_last) },
				{ get_req_last, sizeof(get_req_last) },
				{ get_req_last, sizeof(get_req_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_GET, handle_get_seq, &d);

	g_io_channel_write_chars(io, (char *) get_req_first,
					sizeof(get_req_first), NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS - 1);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_packet_get_rsp(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ NULL, 0 },
				{ NULL, 0 },
				{ NULL, 0 },
				{ get_rsp_last, sizeof(get_rsp_last) } }, {
				{ NULL, 0 },
				{ NULL, 0 },
				{ get_req_last, sizeof(get_req_last) } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_GET, handle_get_seq, &d);

	g_io_channel_write_chars(io, (char *) get_req_first_srm,
					sizeof(get_req_first_srm), NULL,
					&d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS - 1);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void handle_get_seq_srm_wait(GObex *obex, GObexPacket *req,
							gpointer user_data)
{
	struct test_data *d = user_data;
	guint8 op = g_obex_packet_get_operation(req, NULL);
	guint id;

	if (op != G_OBEX_OP_GET) {
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected opcode 0x%02x", op);
		g_main_loop_quit(d->mainloop);
		return;
	}

	id = g_obex_get_rsp(obex, provide_seq, transfer_complete, d,
					&d->err,
					G_OBEX_HDR_SRMP, G_OBEX_SRMP_WAIT,
					G_OBEX_HDR_INVALID);
	if (id == 0)
		g_main_loop_quit(d->mainloop);
}

static void test_packet_get_rsp_wait(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ NULL, 0 },
				{ NULL, 0 },
				{ NULL, 0 },
				{ get_rsp_last, sizeof(get_rsp_last) } }, {
				{ get_req_last, sizeof(get_req_last) },
				{ NULL, 0 },
				{ get_req_last, sizeof(get_req_last) } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_GET,
					handle_get_seq_srm_wait, &d);

	g_io_channel_write_chars(io, (char *) get_req_first_srm,
					sizeof(get_req_first_srm), NULL,
					&d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS - 1);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void handle_get_app(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct test_data *d = user_data;
	guint8 op = g_obex_packet_get_operation(req, NULL);
	GObexPacket *rsp;

	if (op != G_OBEX_OP_GET) {
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected opcode 0x%02x", op);
		g_main_loop_quit(d->mainloop);
		return;
	}

	g_obex_add_request_function(d->obex, G_OBEX_OP_GET, handle_get, d);

	rsp = g_obex_packet_new(G_OBEX_RSP_CONTINUE, TRUE,
				G_OBEX_HDR_APPARAM, hdr_app, sizeof(hdr_app),
				G_OBEX_HDR_INVALID);

	if (g_obex_send(d->obex, rsp, NULL) == FALSE)
		g_main_loop_quit(d->mainloop);
}

static void test_get_rsp_app(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ get_rsp_first_app, sizeof(get_rsp_first_app) },
			{ get_rsp_first, sizeof(get_rsp_first) },
			{ get_rsp_last, sizeof(get_rsp_last) } }, {
			{ get_req_first, sizeof(get_req_first) },
			{ get_req_last, sizeof(get_req_last) },
			{ NULL, 0 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_GET, handle_get_app, &d);

	g_io_channel_write_chars(io, (char *) get_req_first_app,
					sizeof(get_req_first_app), NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 2);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_put_req_delay(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ put_req_first, sizeof(put_req_first) },
				{ put_req_last, sizeof(put_req_last) } }, {
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_last, sizeof(put_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;
	d.provide_delay = 200;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_put_req(obex, provide_data, transfer_complete, &d, &d.err,
					G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
					G_OBEX_HDR_NAME, "file.txt",
					G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 2);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_get_rsp_delay(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ get_rsp_first, sizeof(get_rsp_first) },
				{ get_rsp_last, sizeof(get_rsp_last) } }, {
				{ get_req_last, sizeof(get_req_last) },
				{ NULL, 0 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;
	d.provide_delay = 200;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_GET, handle_get, &d);

	g_io_channel_write_chars(io, (char *) get_req_first,
					sizeof(get_req_first), NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 1);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static gboolean rcv_data_delay(const void *buf, gsize len, gpointer user_data)
{
	struct test_data *d = user_data;

	if (len != sizeof(body_data))
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected byte count %zu", len);

	if (memcmp(buf, body_data, sizeof(body_data)) != 0) {
		dump_bufs(body_data, sizeof(body_data), buf, len);
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected byte count %zu", len);
	}

	if (d->provide_delay > 0) {
		g_obex_suspend(d->obex);
		g_timeout_add(d->provide_delay, resume_obex, d);
	}

	return TRUE;
}

static void handle_put_delay(GObex *obex, GObexPacket *req, gpointer user_data)
{
	struct test_data *d = user_data;
	guint8 op = g_obex_packet_get_operation(req, NULL);
	guint id;

	if (op != G_OBEX_OP_PUT) {
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected opcode 0x%02x", op);
		g_main_loop_quit(d->mainloop);
		return;
	}

	id = g_obex_put_rsp(obex, req, rcv_data_delay, transfer_complete, d,
						&d->err, G_OBEX_HDR_INVALID);
	if (id == 0)
		g_main_loop_quit(d->mainloop);
}

static void test_put_rsp_delay(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_last, sizeof(put_rsp_last) } }, {
				{ put_req_last, sizeof(put_req_last) },
				{ NULL, 0 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;
	d.provide_delay = 200;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_PUT, handle_put_delay, &d);

	g_io_channel_write_chars(io, (char *) put_req_first,
					sizeof(put_req_first), NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 1);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_get_req_delay(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ get_req_first, sizeof(get_req_first) },
				{ get_req_last, sizeof(get_req_last) } }, {
				{ get_rsp_first, sizeof(get_rsp_first) },
				{ get_rsp_last, sizeof(get_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;
	d.provide_delay = 200;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_get_req(obex, rcv_data_delay, transfer_complete, &d, &d.err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 2);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_get_rsp_eagain(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ get_rsp_first, sizeof(get_rsp_first) },
				{ get_rsp_last, sizeof(get_rsp_last) } }, {
				{ get_req_last, sizeof(get_req_last) },
				{ NULL, 0 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;
	d.provide_delay = 200;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_GET, handle_get_eagain,
									&d);

	g_io_channel_write_chars(io, (char *) get_req_first,
					sizeof(get_req_first), NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 1);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void conn_complete(GObex *obex, GError *err, GObexPacket *rsp,
							gpointer user_data)
{
	struct test_data *d = user_data;

	if (err != NULL)
		d->err = g_error_copy(err);

	g_main_loop_quit(d->mainloop);
}

static void test_conn_req(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ conn_req, sizeof(conn_req) } }, {
				{ conn_rsp, sizeof(conn_rsp) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_connect(obex, conn_complete, &d, &d.err, G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 1);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void handle_conn_rsp(GObex *obex, GObexPacket *req,
						gpointer user_data)
{
	struct test_data *d = user_data;
	guint8 op = g_obex_packet_get_operation(req, NULL);
	GObexPacket *rsp;

	if (op != G_OBEX_OP_CONNECT) {
		d->err = g_error_new(TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Unexpected opcode 0x%02x", op);
		g_main_loop_quit(d->mainloop);
		return;
	}

	rsp = g_obex_packet_new(G_OBEX_RSP_SUCCESS, TRUE,
						G_OBEX_HDR_CONNECTION, 1,
						G_OBEX_HDR_INVALID);
	g_obex_send(obex, rsp, &d->err);
}

static void test_conn_rsp(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ conn_rsp, sizeof(conn_rsp) } }, {
			{ NULL, -1 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_CONNECT,
						handle_conn_rsp, &d);

	g_io_channel_write_chars(io, (char *) conn_req, sizeof(conn_req),
								NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 1);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	if (!d.io_completed)
		g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void conn_complete_get_req(GObex *obex, GError *err, GObexPacket *rsp,
							gpointer user_data)
{
	struct test_data *d = user_data;

	if (err != NULL) {
		d->err = g_error_copy(err);
		g_main_loop_quit(d->mainloop);
	}

	g_obex_get_req(obex, rcv_data, transfer_complete, d, &d->err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_INVALID);
}

static void test_conn_get_req(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ conn_req, sizeof(conn_req) },
			{ conn_get_req_first, sizeof(conn_get_req_first) },
			{ get_req_last, sizeof(get_req_last) }}, {
			{ conn_rsp, sizeof(conn_rsp) } ,
			{ get_rsp_first, sizeof(get_rsp_first) },
			{ get_rsp_last, sizeof(get_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_connect(obex, conn_complete_get_req, &d, &d.err,
							G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 3);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_conn_get_rsp(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ conn_rsp, sizeof(conn_rsp) },
			{ get_rsp_first, sizeof(get_rsp_first) },
			{ get_rsp_last, sizeof(get_rsp_last) } }, {
			{ conn_get_req_first, sizeof(conn_get_req_first) },
			{ get_req_last, sizeof(get_req_last) },
			{ NULL, 0 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_CONNECT,
						handle_conn_rsp, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_GET,
						handle_get, &d);

	g_io_channel_write_chars(io, (char *) conn_req, sizeof(conn_req),
								NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 2);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void conn_complete_put_req(GObex *obex, GError *err, GObexPacket *rsp,
							gpointer user_data)
{
	struct test_data *d = user_data;

	if (err != NULL) {
		d->err = g_error_copy(err);
		g_main_loop_quit(d->mainloop);
	}

	g_obex_put_req(obex, provide_data, transfer_complete, d, &d->err,
				G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
				G_OBEX_HDR_NAME, "file.txt",
				G_OBEX_HDR_INVALID);
}

static void test_conn_put_req(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ conn_req, sizeof(conn_req) },
			{ conn_put_req_first, sizeof(conn_put_req_first) },
			{ put_req_last, sizeof(put_req_last) }}, {
			{ conn_rsp, sizeof(conn_rsp) } ,
			{ put_rsp_first, sizeof(put_rsp_first) },
			{ put_rsp_last, sizeof(put_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_connect(obex, conn_complete_put_req, &d, &d.err,
							G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 3);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_conn_put_rsp(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ conn_rsp, sizeof(conn_rsp) },
			{ put_rsp_first, sizeof(put_rsp_first) },
			{ put_rsp_last, sizeof(put_rsp_last) } }, {
			{ conn_put_req_first, sizeof(conn_put_req_first) },
			{ put_req_last, sizeof(put_req_last) },
			{ NULL, 0 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_CONNECT,
						handle_conn_rsp, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_PUT,
						handle_put, &d);

	g_io_channel_write_chars(io, (char *) conn_req, sizeof(conn_req),
								NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 2);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void test_conn_get_wrg_rsp(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
			{ conn_rsp, sizeof(conn_rsp) },
			{ unavailable_rsp, sizeof(unavailable_rsp) } }, {
			{ conn_get_req_wrg, sizeof(conn_get_req_wrg) },
			{ NULL, -1 } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_add_request_function(obex, G_OBEX_OP_CONNECT,
						handle_conn_rsp, &d);

	g_io_channel_write_chars(io, (char *) conn_req, sizeof(conn_req),
								NULL, &d.err);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, 2);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	if (!d.io_completed)
		g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void conn_complete_put_req_seq(GObex *obex, GError *err,
					GObexPacket *rsp, gpointer user_data)
{
	struct test_data *d = user_data;

	if (err != NULL) {
		d->err = g_error_copy(err);
		g_main_loop_quit(d->mainloop);
	}

	g_obex_put_req(obex, provide_seq, transfer_complete, d, &d->err,
					G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
					G_OBEX_HDR_NAME, "random.bin",
					G_OBEX_HDR_INVALID);
}

static void test_conn_put_req_seq(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ conn_req, sizeof(conn_req) } ,
				{ NULL, 0 },
				{ NULL, 0 },
				{ put_req_last, sizeof(put_req_last) } }, {
				{ conn_rsp, sizeof(conn_rsp) } ,
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_first, sizeof(put_rsp_first) },
				{ put_rsp_last, sizeof(put_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_STREAM);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_connect(obex, conn_complete_put_req_seq, &d, &d.err,
							G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

static void conn_complete_put_req_seq_srm(GObex *obex, GError *err,
					GObexPacket *rsp, gpointer user_data)
{
	struct test_data *d = user_data;

	if (err != NULL) {
		d->err = g_error_copy(err);
		g_main_loop_quit(d->mainloop);
	}

	g_obex_put_req(obex, provide_seq, transfer_complete, d, &d->err,
					G_OBEX_HDR_TYPE, hdr_type, sizeof(hdr_type),
					G_OBEX_HDR_NAME, "random.bin",
					G_OBEX_HDR_INVALID);
}

static void test_conn_put_req_seq_srm(void)
{
	GIOChannel *io;
	GIOCondition cond;
	guint io_id, timer_id;
	GObex *obex;
	struct test_data d = { 0, NULL, {
				{ conn_req_srm, sizeof(conn_req_srm) } ,
				{ NULL, 0 },
				{ NULL, 0 },
				{ put_req_last, sizeof(put_req_last) } }, {
				{ conn_rsp_srm, sizeof(conn_rsp_srm) } ,
				{ NULL, 0 },
				{ NULL, 0 },
				{ put_rsp_last, sizeof(put_rsp_last) } } };

	create_endpoints(&obex, &io, SOCK_SEQPACKET);
	d.obex = obex;

	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	io_id = g_io_add_watch(io, cond, test_io_cb, &d);

	d.mainloop = g_main_loop_new(NULL, FALSE);

	timer_id = g_timeout_add_seconds(1, test_timeout, &d);

	g_obex_connect(obex, conn_complete_put_req_seq_srm, &d, &d.err,
					G_OBEX_HDR_SRM, G_OBEX_SRM_INDICATE,
					G_OBEX_HDR_INVALID);
	g_assert_no_error(d.err);

	g_main_loop_run(d.mainloop);

	g_assert_cmpuint(d.count, ==, RANDOM_PACKETS);

	g_main_loop_unref(d.mainloop);

	g_source_remove(timer_id);
	g_io_channel_unref(io);
	g_source_remove(io_id);
	g_obex_unref(obex);

	g_assert_no_error(d.err);
}

int main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/gobex/test_conn_req", test_conn_req);
	g_test_add_func("/gobex/test_conn_rsp", test_conn_rsp);

	g_test_add_func("/gobex/test_put_req", test_put_req);
	g_test_add_func("/gobex/test_put_rsp", test_put_rsp);

	g_test_add_func("/gobex/test_get_req", test_get_req);
	g_test_add_func("/gobex/test_get_rsp", test_get_rsp);

	g_test_add_func("/gobex/test_get_req_app", test_get_req_app);
	g_test_add_func("/gobex/test_get_rsp_app", test_get_rsp_app);

	g_test_add_func("/gobex/test_put_req_delay", test_put_req_delay);
	g_test_add_func("/gobex/test_put_rsp_delay", test_put_rsp_delay);

	g_test_add_func("/gobex/test_get_req_delay", test_get_req_delay);
	g_test_add_func("/gobex/test_get_rsp_delay", test_get_rsp_delay);

	g_test_add_func("/gobex/test_put_req_eagain", test_put_req_eagain);
	g_test_add_func("/gobex/test_get_req_eagain", test_get_rsp_eagain);

	g_test_add_func("/gobex/test_stream_put_req", test_stream_put_req);
	g_test_add_func("/gobex/test_stream_put_rsp", test_stream_put_rsp);

	g_test_add_func("/gobex/test_stream_put_req_abort",
						test_stream_put_req_abort);
	g_test_add_func("/gobex/test_stream_put_rsp_abort",
						test_stream_put_rsp_abort);

	g_test_add_func("/gobex/test_stream_get_req", test_stream_get_req);
	g_test_add_func("/gobex/test_stream_get_rsp", test_stream_get_rsp);

	g_test_add_func("/gobex/test_conn_get_req", test_conn_get_req);
	g_test_add_func("/gobex/test_conn_get_rsp", test_conn_get_rsp);

	g_test_add_func("/gobex/test_conn_put_req", test_conn_put_req);
	g_test_add_func("/gobex/test_conn_put_rsp", test_conn_put_rsp);

	g_test_add_func("/gobex/test_conn_get_wrg_rsp", test_conn_get_wrg_rsp);

	g_test_add_func("/gobex/test_conn_put_req_seq",
						test_conn_put_req_seq);

	g_test_add_func("/gobex/test_packet_put_req", test_packet_put_req);
	g_test_add_func("/gobex/test_packet_put_req_wait",
						test_packet_put_req_wait);
	g_test_add_func("/gobex/test_packet_put_req_suspend_resume",
					test_packet_put_req_suspend_resume);

	g_test_add_func("/gobex/test_packet_put_rsp", test_packet_put_rsp);
	g_test_add_func("/gobex/test_packet_put_rsp_wait",
						test_packet_put_rsp_wait);

	g_test_add_func("/gobex/test_packet_get_rsp", test_packet_get_rsp);
	g_test_add_func("/gobex/test_packet_get_rsp_wait",
						test_packet_get_rsp_wait);

	g_test_add_func("/gobex/test_packet_get_req", test_packet_get_req);
	g_test_add_func("/gobex/test_packet_get_req_wait",
						test_packet_get_req_wait);
	g_test_add_func("/gobex/test_packet_get_req_suspend_resume",
					test_packet_get_req_suspend_resume);

	g_test_add_func("/gobex/test_packet_get_req_wait_next",
						test_packet_get_req_wait_next);

	g_test_add_func("/gobex/test_conn_put_req_seq_srm",
						test_conn_put_req_seq_srm);

	return g_test_run();
}
