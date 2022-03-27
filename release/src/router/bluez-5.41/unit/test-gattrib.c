/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Google, Inc.
 *
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

#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <glib.h>

#include "src/shared/util.h"
#include "lib/bluetooth.h"
#include "lib/uuid.h"
#include "attrib/att.h"
#include "attrib/gattrib.h"
#include "src/log.h"

#define DEFAULT_MTU 23

#define data(args...) ((const unsigned char[]) { args })

struct test_pdu {
	bool valid;
	bool sent;
	bool received;
	const uint8_t *data;
	size_t size;
};

#define pdu(args...)				\
	{					\
		.valid = true,			\
		.sent = false,			\
		.received = false,		\
		.data = data(args),		\
		.size = sizeof(data(args)),	\
	}

struct context {
	GMainLoop *main_loop;
	GIOChannel *att_io;
	GIOChannel *server_io;
	GAttrib *att;
};

static void setup_context(struct context *cxt, gconstpointer data)
{
	int err, sv[2];

	cxt->main_loop = g_main_loop_new(NULL, FALSE);
	g_assert(cxt->main_loop != NULL);

	err = socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sv);
	g_assert(err == 0);

	cxt->att_io = g_io_channel_unix_new(sv[0]);
	g_assert(cxt->att_io != NULL);

	g_io_channel_set_close_on_unref(cxt->att_io, TRUE);

	cxt->server_io = g_io_channel_unix_new(sv[1]);
	g_assert(cxt->server_io != NULL);

	g_io_channel_set_close_on_unref(cxt->server_io, TRUE);
	g_io_channel_set_encoding(cxt->server_io, NULL, NULL);
	g_io_channel_set_buffered(cxt->server_io, FALSE);

	cxt->att = g_attrib_new(cxt->att_io, DEFAULT_MTU, false);
	g_assert(cxt->att != NULL);
}

static void teardown_context(struct context *cxt, gconstpointer data)
{
	if (cxt->att)
		g_attrib_unref(cxt->att);

	g_io_channel_unref(cxt->server_io);

	g_io_channel_unref(cxt->att_io);

	g_main_loop_unref(cxt->main_loop);
}


static void test_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	g_print("%s%s\n", prefix, str);
}

static void destroy_canary_increment(gpointer data)
{
	int *canary = data;
	(*canary)++;
}

static void test_refcount(struct context *cxt, gconstpointer unused)
{
	GAttrib *extra_ref;
	int destroy_canary = 0;

	g_attrib_set_destroy_function(cxt->att, destroy_canary_increment,
							       &destroy_canary);

	extra_ref = g_attrib_ref(cxt->att);

	g_assert(extra_ref == cxt->att);

	g_assert(destroy_canary == 0);

	g_attrib_unref(extra_ref);

	g_assert(destroy_canary == 0);

	g_attrib_unref(cxt->att);

	g_assert(destroy_canary == 1);

	/* Avoid a double-free from the teardown function */
	cxt->att = NULL;
}

static void test_get_channel(struct context *cxt, gconstpointer unused)
{
	GIOChannel *chan;

	chan = g_attrib_get_channel(cxt->att);

	g_assert(chan == cxt->att_io);
}

struct expect_response {
	struct test_pdu expect;
	struct test_pdu respond;
	GSourceFunc receive_cb;
	gpointer user_data;
};

static gboolean test_client(GIOChannel *channel, GIOCondition cond,
								  gpointer data)
{
	struct expect_response *cr = data;
	int fd;
	uint8_t buf[256];
	ssize_t len;
	int cmp;

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP))
		return FALSE;

	fd = g_io_channel_unix_get_fd(channel);

	len = read(fd, buf, sizeof(buf));

	g_assert(len > 0);
	g_assert_cmpint(len, ==, cr->expect.size);

	if (g_test_verbose())
		util_hexdump('?', cr->expect.data,  cr->expect.size,
						   test_debug, "test_client: ");

	cmp = memcmp(cr->expect.data, buf, len);

	g_assert(cmp == 0);

	cr->expect.received = true;

	if (cr->receive_cb != NULL)
		cr->receive_cb(cr->user_data);

	if (cr->respond.valid) {
		if (g_test_verbose())
			util_hexdump('<', cr->respond.data, cr->respond.size,
						   test_debug, "test_client: ");
		len = write(fd, cr->respond.data, cr->respond.size);

		g_assert_cmpint(len, ==, cr->respond.size);

		cr->respond.sent = true;
	}

	return TRUE;
}

struct result_data {
	guint8 status;
	guint8 *pdu;
	guint16 len;
	GSourceFunc complete_cb;
	gpointer user_data;
};

static void result_canary(guint8 status, const guint8 *pdu, guint16 len,
								gpointer data)
{
	struct result_data *result = data;

	result->status = status;
	result->pdu = g_malloc0(len);
	memcpy(result->pdu, pdu, len);
	result->len = len;

	if (g_test_verbose())
		util_hexdump('<', pdu, len, test_debug, "result_canary: ");

	if (result->complete_cb != NULL)
		result->complete_cb(result->user_data);
}

static gboolean context_stop_main_loop(gpointer user_data)
{
	struct context *cxt = user_data;

	g_main_loop_quit(cxt->main_loop);
	return FALSE;
}

static void test_send(struct context *cxt, gconstpointer unused)
{
	int cmp;
	struct result_data results;
	struct expect_response data = {
		.expect = pdu(0x02, 0x00, 0x02),
		.respond = pdu(0x03, 0x02, 0x03, 0x04),
		.receive_cb = NULL,
		.user_data = NULL,
	};

	g_io_add_watch(cxt->server_io, G_IO_IN | G_IO_HUP | G_IO_ERR |
						G_IO_NVAL, test_client, &data);

	results.complete_cb = context_stop_main_loop;
	results.user_data = cxt;

	g_attrib_send(cxt->att, 0, data.expect.data, data.expect.size,
				      result_canary, (gpointer) &results, NULL);

	g_main_loop_run(cxt->main_loop);

	g_assert(results.pdu != NULL);

	g_assert_cmpint(results.len, ==, data.respond.size);

	cmp = memcmp(results.pdu, data.respond.data, results.len);

	g_assert(cmp == 0);

	g_free(results.pdu);
}

struct event_info {
	struct context *context;
	int event_id;
};

static gboolean cancel_existing_attrib_event(gpointer user_data)
{
	struct event_info *info = user_data;
	gboolean canceled;

	canceled = g_attrib_cancel(info->context->att, info->event_id);

	g_assert(canceled);

	g_idle_add(context_stop_main_loop, info->context);

	return FALSE;
}

static void test_cancel(struct context *cxt, gconstpointer unused)
{
	gboolean canceled;
	struct result_data results;
	struct event_info info;
	struct expect_response data = {
		.expect = pdu(0x02, 0x00, 0x02),
		.respond = pdu(0x03, 0x02, 0x03, 0x04),
	};

	g_io_add_watch(cxt->server_io,
				      G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
							    test_client, &data);

	results.pdu = NULL;

	info.context = cxt;
	info.event_id = g_attrib_send(cxt->att, 0, data.expect.data,
						data.expect.size, result_canary,
								&results, NULL);

	data.receive_cb = cancel_existing_attrib_event;
	data.user_data = &info;

	g_main_loop_run(cxt->main_loop);

	g_assert(results.pdu == NULL);

	results.pdu = NULL;
	data.expect.received = false;
	data.respond.sent = false;

	info.event_id = g_attrib_send(cxt->att, 0, data.expect.data,
						data.expect.size, result_canary,
								&results, NULL);

	canceled = g_attrib_cancel(cxt->att, info.event_id);
	g_assert(canceled);

	g_idle_add(context_stop_main_loop, info.context);

	g_main_loop_run(cxt->main_loop);

	g_assert(!data.expect.received);
	g_assert(!data.respond.sent);
	g_assert(results.pdu == NULL);

	/* Invalid ID */
	canceled = g_attrib_cancel(cxt->att, 42);
	g_assert(!canceled);
}

static void send_test_pdus(gpointer context, struct test_pdu *pdus)
{
	struct context *cxt = context;
	size_t len;
	int fd;
	struct test_pdu *cur_pdu;

	fd = g_io_channel_unix_get_fd(cxt->server_io);

	for (cur_pdu = pdus; cur_pdu->valid; cur_pdu++)
		cur_pdu->sent = false;

	for (cur_pdu = pdus; cur_pdu->valid; cur_pdu++) {
		if (g_test_verbose())
			util_hexdump('>', cur_pdu->data, cur_pdu->size,
						test_debug, "send_test_pdus: ");
		len = write(fd, cur_pdu->data, cur_pdu->size);
		g_assert_cmpint(len, ==, cur_pdu->size);
		cur_pdu->sent = true;
	}

	g_idle_add(context_stop_main_loop, cxt);
	g_main_loop_run(cxt->main_loop);
}

#define PDU_MTU_RESP pdu(ATT_OP_MTU_RESP, 0x17)
#define PDU_FIND_INFO_REQ pdu(ATT_OP_FIND_INFO_REQ, 0x01, 0x00, 0xFF, 0xFF)
#define PDU_NO_ATT_ERR pdu(ATT_OP_ERROR, ATT_OP_FIND_INFO_REQ, 0x00, 0x00, 0x0A)
#define PDU_IND_NODATA pdu(ATT_OP_HANDLE_IND, 0x01, 0x00)
#define PDU_INVALID_IND pdu(ATT_OP_HANDLE_IND, 0x14)
#define PDU_IND_DATA pdu(ATT_OP_HANDLE_IND, 0x14, 0x00, 0x01)

struct expect_test_data {
	struct test_pdu *expected;
	GAttrib *att;
};

static void notify_canary_expect(const guint8 *pdu, guint16 len, gpointer data)
{
	struct expect_test_data *expect = data;
	struct test_pdu *expected = expect->expected;
	int cmp;

	if (g_test_verbose())
		util_hexdump('<', pdu, len, test_debug,
						      "notify_canary_expect: ");

	while (expected->valid && expected->received)
		expected++;

	g_assert(expected->valid);

	if (g_test_verbose())
		util_hexdump('?', expected->data, expected->size, test_debug,
						      "notify_canary_expect: ");

	g_assert_cmpint(expected->size, ==, len);

	cmp = memcmp(pdu, expected->data, expected->size);

	g_assert(cmp == 0);

	expected->received = true;

	if (pdu[0] == ATT_OP_FIND_INFO_REQ) {
		struct test_pdu no_attributes = PDU_NO_ATT_ERR;
		int reqid;

		reqid = g_attrib_send(expect->att, 0, no_attributes.data,
					  no_attributes.size, NULL, NULL, NULL);
		g_assert(reqid != 0);
	}
}

static void test_register(struct context *cxt, gconstpointer user_data)
{
	guint reg_id;
	gboolean canceled;
	struct test_pdu pdus[] = {
		/*
		 * Unmatched PDU opcode
		 * Unmatched handle (GATTRIB_ALL_REQS) */
		PDU_FIND_INFO_REQ,
		/*
		 * Matched PDU opcode
		 * Unmatched handle (GATTRIB_ALL_HANDLES) */
		PDU_IND_NODATA,
		/*
		 * Matched PDU opcode
		 * Invalid length? */
		PDU_INVALID_IND,
		/*
		 * Matched PDU opcode
		 * Matched handle */
		PDU_IND_DATA,
		{ },
	};
	struct test_pdu req_pdus[] = { PDU_FIND_INFO_REQ, { } };
	struct test_pdu all_ind_pdus[] = {
		PDU_IND_NODATA,
		PDU_INVALID_IND,
		PDU_IND_DATA,
		{ },
	};
	struct test_pdu followed_ind_pdus[] = { PDU_IND_DATA, { } };
	struct test_pdu *current_pdu;
	struct expect_test_data expect;

	expect.att = cxt->att;

	/*
	 * Without registering anything, should be able to ignore everything but
	 * an unexpected response. */
	send_test_pdus(cxt, pdus);

	if (g_test_verbose())
		g_print("ALL_REQS, ALL_HANDLES\r\n");

	expect.expected = req_pdus;
	reg_id = g_attrib_register(cxt->att, GATTRIB_ALL_REQS,
				      GATTRIB_ALL_HANDLES, notify_canary_expect,
								 &expect, NULL);

	send_test_pdus(cxt, pdus);

	canceled = g_attrib_unregister(cxt->att, reg_id);

	g_assert(canceled);

	for (current_pdu = req_pdus; current_pdu->valid; current_pdu++)
		g_assert(current_pdu->received);

	if (g_test_verbose())
		g_print("IND, ALL_HANDLES\r\n");

	expect.expected = all_ind_pdus;
	reg_id = g_attrib_register(cxt->att, ATT_OP_HANDLE_IND,
				      GATTRIB_ALL_HANDLES, notify_canary_expect,
								 &expect, NULL);

	send_test_pdus(cxt, pdus);

	canceled = g_attrib_unregister(cxt->att, reg_id);

	g_assert(canceled);

	for (current_pdu = all_ind_pdus; current_pdu->valid; current_pdu++)
		g_assert(current_pdu->received);

	if (g_test_verbose())
		g_print("IND, 0x0014\r\n");

	expect.expected = followed_ind_pdus;
	reg_id = g_attrib_register(cxt->att, ATT_OP_HANDLE_IND, 0x0014,
					notify_canary_expect, &expect, NULL);

	send_test_pdus(cxt, pdus);

	canceled = g_attrib_unregister(cxt->att, reg_id);

	g_assert(canceled);

	for (current_pdu = followed_ind_pdus; current_pdu->valid; current_pdu++)
		g_assert(current_pdu->received);

	canceled = g_attrib_unregister(cxt->att, reg_id);

	g_assert(!canceled);
}

static void test_buffers(struct context *cxt, gconstpointer unused)
{
	size_t buflen;
	uint8_t *buf;
	gboolean success;

	buf = g_attrib_get_buffer(cxt->att, &buflen);
	g_assert(buf != 0);
	g_assert_cmpint(buflen, ==, DEFAULT_MTU);

	success = g_attrib_set_mtu(cxt->att, 5);
	g_assert(!success);

	success = g_attrib_set_mtu(cxt->att, 255);
	g_assert(success);

	buf = g_attrib_get_buffer(cxt->att, &buflen);
	g_assert(buf != 0);
	g_assert_cmpint(buflen, ==, 255);
}

int main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);

	if (g_test_verbose())
		__btd_log_init("*", 0);

	/*
	 * Test the GAttrib API behavior
	 */
	g_test_add("/gattrib/refcount", struct context, NULL, setup_context,
					      test_refcount, teardown_context);
	g_test_add("/gattrib/get_channel", struct context, NULL, setup_context,
					    test_get_channel, teardown_context);
	g_test_add("/gattrib/send", struct context, NULL, setup_context,
						   test_send, teardown_context);
	g_test_add("/gattrib/cancel", struct context, NULL, setup_context,
						 test_cancel, teardown_context);
	g_test_add("/gattrib/register", struct context, NULL, setup_context,
					       test_register, teardown_context);
	g_test_add("/gattrib/buffers", struct context, NULL, setup_context,
						test_buffers, teardown_context);

	return g_test_run();
}
