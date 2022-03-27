/*
 *
 *  OBEX library with GLib integration
 *
 *  Copyright (C) 2011  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
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

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <glib.h>

#include "gobex/gobex.h"

#include "util.h"

GQuark test_error_quark(void)
{
	return g_quark_from_static_string("test-error-quark");
}

static void dump_bytes(const uint8_t *buf, size_t buf_len)
{
	size_t i;

	for (i = 0; i < buf_len; i++)
		g_printerr("%02x ", buf[i]);

	g_printerr("\n");
}

void dump_bufs(const void *mem1, size_t len1, const void *mem2, size_t len2)
{
	g_printerr("\nExpected: ");
	dump_bytes(mem1, len1);
	g_printerr("Got:      ");
	dump_bytes(mem2, len2);
}

void assert_memequal(const void *mem1, size_t len1,
						const void *mem2, size_t len2)
{
	if (len1 == len2 && memcmp(mem1, mem2, len1) == 0)
		return;

	dump_bufs(mem1, len1, mem2, len2);

	g_assert(0);
}

GObex *create_gobex(int fd, GObexTransportType transport_type,
						gboolean close_on_unref)
{
	GIOChannel *io;
	GObex *obex;

	io = g_io_channel_unix_new(fd);
	g_assert(io != NULL);

	g_io_channel_set_close_on_unref(io, close_on_unref);

	obex = g_obex_new(io, transport_type, -1, -1);
	g_io_channel_unref(io);

	return obex;
}

void create_endpoints(GObex **obex, GIOChannel **io, int sock_type)
{
	GObexTransportType transport_type;
	int sv[2];

	if (socketpair(AF_UNIX, sock_type | SOCK_NONBLOCK, 0, sv) < 0) {
		g_printerr("socketpair: %s", strerror(errno));
		abort();
	}

	if (sock_type == SOCK_STREAM)
		transport_type = G_OBEX_TRANSPORT_STREAM;
	else
		transport_type = G_OBEX_TRANSPORT_PACKET;

	*obex = create_gobex(sv[0], transport_type, TRUE);
	g_assert(*obex != NULL);

	if (io == NULL) {
		close(sv[1]);
		return;
	}

	*io = g_io_channel_unix_new(sv[1]);
	g_assert(*io != NULL);

	g_io_channel_set_encoding(*io, NULL, NULL);
	g_io_channel_set_buffered(*io, FALSE);
	g_io_channel_set_close_on_unref(*io, TRUE);
}

gboolean test_timeout(gpointer user_data)
{
	struct test_data *d = user_data;

	if (!g_main_loop_is_running(d->mainloop))
		return FALSE;

	d->err = g_error_new(TEST_ERROR, TEST_ERROR_TIMEOUT, "Timed out");

	g_main_loop_quit(d->mainloop);

	return FALSE;
}

gboolean test_io_cb(GIOChannel *io, GIOCondition cond, gpointer user_data)
{
	struct test_data *d = user_data;
	GIOStatus status;
	gsize bytes_written, rbytes, send_buf_len, expect_len;
	char buf[65535];
	const char *send_buf, *expect;

	expect = d->recv[d->count].data;
	expect_len = d->recv[d->count].len;
	send_buf = d->send[d->count].data;
	send_buf_len = d->send[d->count].len;

	d->count++;

	if (!(cond & G_IO_IN))
		goto send;

	status = g_io_channel_read_chars(io, buf, sizeof(buf), &rbytes, NULL);
	if (status != G_IO_STATUS_NORMAL) {
		g_print("io_cb count %u\n", d->count);
		g_set_error(&d->err, TEST_ERROR, TEST_ERROR_UNEXPECTED,
				"Reading data failed with status %d", status);
		goto failed;
	}

	if (rbytes < expect_len) {
		g_print("io_cb count %u\n", d->count);
		dump_bufs(expect, expect_len, buf, rbytes);
		g_set_error(&d->err, TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Not enough data from socket");
		goto failed;
	}

	if (memcmp(buf, expect, expect_len) != 0) {
		g_print("io_cb count %u\n", d->count);
		dump_bufs(expect, expect_len, buf, rbytes);
		g_set_error(&d->err, TEST_ERROR, TEST_ERROR_UNEXPECTED,
					"Received data is not correct");
		goto failed;
	}

send:
	if ((gssize) send_buf_len < 0)
		goto failed;

	g_io_channel_write_chars(io, send_buf, send_buf_len, &bytes_written,
									NULL);
	if (bytes_written != send_buf_len) {
		g_print("io_cb count %u\n", d->count);
		g_set_error(&d->err, TEST_ERROR, TEST_ERROR_UNEXPECTED,
						"Unable to write to socket");
		goto failed;
	}

	if (d->recv[d->count].len < 0 || (gssize) expect_len < 0)
		return test_io_cb(io, G_IO_OUT, user_data);

	return TRUE;

failed:
	g_main_loop_quit(d->mainloop);
	d->io_completed = TRUE;
	return FALSE;
}
