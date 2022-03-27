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

enum {
	TEST_ERROR_TIMEOUT,
	TEST_ERROR_UNEXPECTED,
};

struct test_buf {
	const void *data;
	gssize len;
};

struct test_data {
	guint count;
	GError *err;
	struct test_buf recv[4];
	struct test_buf send[4];
	guint provide_delay;
	GObex *obex;
	guint id;
	gsize total;
	GMainLoop *mainloop;
	gboolean io_completed;
};

#define TEST_ERROR test_error_quark()
GQuark test_error_quark(void);

void dump_bufs(const void *mem1, size_t len1, const void *mem2, size_t len2);
void assert_memequal(const void *mem1, size_t len1,
						const void *mem2, size_t len2);

GObex *create_gobex(int fd, GObexTransportType transport_type,
						gboolean close_on_unref);
void create_endpoints(GObex **obex, GIOChannel **io, int sock_type);

gboolean test_io_cb(GIOChannel *io, GIOCondition cond, gpointer user_data);
gboolean test_timeout(gpointer user_data);
