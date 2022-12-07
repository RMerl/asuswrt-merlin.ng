/* SPDX-License-Identifier: GPL-2.0-only */
/*
 *
 *  OBEX library with GLib integration
 *
 *  Copyright (C) 2011  Intel Corporation. All rights reserved.
 *
 */

#define TEST_BUF_MAX 5

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
	struct test_buf recv[TEST_BUF_MAX];
	struct test_buf send[TEST_BUF_MAX];
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
