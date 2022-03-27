/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <glib.h>
#include "src/shared/util.h"
#include "src/log.h"
#include "android/ipc-common.h"
#include "android/ipc.h"

static const char HAL_SK_PATH[] = "\0test_hal_socket";

#define SERVICE_ID_MAX 10

struct test_data {
	bool disconnect;
	const void *cmd;
	uint16_t cmd_size;
	uint8_t service;
	const struct ipc_handler *handlers;
	uint8_t handlers_size;
};

struct context {
	GMainLoop *main_loop;

	int sk;

	guint source;
	guint cmd_source;
	guint notif_source;

	GIOChannel *cmd_io;
	GIOChannel *notif_io;

	const struct test_data *data;
};


static struct ipc *ipc = NULL;

static void context_quit(struct context *context)
{
	g_main_loop_quit(context->main_loop);
}

static gboolean cmd_watch(GIOChannel *io, GIOCondition cond,
						gpointer user_data)
{
	struct context *context = user_data;
	const struct test_data *test_data = context->data;
	const struct ipc_hdr *sent_msg = test_data->cmd;
	uint8_t buf[128];
	int sk;

	struct ipc_hdr success_resp = {
		.service_id = sent_msg->service_id,
		.opcode = sent_msg->opcode,
		.len = 0,
	};

	if (cond & (G_IO_HUP | G_IO_ERR | G_IO_NVAL)) {
		g_assert(test_data->disconnect);
		return FALSE;
	}

	g_assert(!test_data->disconnect);

	sk = g_io_channel_unix_get_fd(io);

	g_assert(read(sk, buf, sizeof(buf)) == sizeof(struct ipc_hdr));
	g_assert(!memcmp(&success_resp, buf, sizeof(struct ipc_hdr)));

	context_quit(context);

	return TRUE;
}

static gboolean notif_watch(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct context *context = user_data;
	const struct test_data *test_data = context->data;

	if (cond & (G_IO_HUP | G_IO_ERR | G_IO_NVAL)) {
		g_assert(test_data->disconnect);
		return FALSE;
	}

	g_assert(!test_data->disconnect);

	return TRUE;
}

static gboolean connect_handler(GIOChannel *io, GIOCondition cond,
						gpointer user_data)
{
	struct context *context = user_data;
	const struct test_data *test_data = context->data;
	GIOChannel *new_io;
	GIOCondition watch_cond;
	int sk;

	if (cond & (G_IO_HUP | G_IO_ERR | G_IO_NVAL)) {
		g_assert(FALSE);
		return FALSE;
	}

	g_assert(!context->cmd_source || !context->notif_source);

	sk = accept(context->sk, NULL, NULL);
	g_assert(sk >= 0);

	new_io = g_io_channel_unix_new(sk);

	watch_cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;

	if (context->cmd_source && !context->notif_source) {
		context->notif_source = g_io_add_watch(new_io, watch_cond,
							notif_watch, context);
		g_assert(context->notif_source > 0);
		context->notif_io = new_io;
	}

	if (!context->cmd_source) {
		context->cmd_source = g_io_add_watch(new_io, watch_cond,
							cmd_watch, context);
		context->cmd_io = new_io;
	}

	if (context->cmd_source && context->notif_source && !test_data->cmd)
		context_quit(context);

	return TRUE;
}

static struct context *create_context(gconstpointer data)
{
	struct context *context = g_new0(struct context, 1);
	struct sockaddr_un addr;
	GIOChannel *io;
	int ret, sk;

	context->main_loop = g_main_loop_new(NULL, FALSE);
	g_assert(context->main_loop);

	sk = socket(AF_LOCAL, SOCK_SEQPACKET, 0);
	g_assert(sk >= 0);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	memcpy(addr.sun_path, HAL_SK_PATH, sizeof(HAL_SK_PATH));

	ret = bind(sk, (struct sockaddr *) &addr, sizeof(addr));
	g_assert(ret == 0);

	ret = listen(sk, 5);
	g_assert(ret == 0);

	io = g_io_channel_unix_new(sk);

	g_io_channel_set_close_on_unref(io, TRUE);

	context->source = g_io_add_watch(io,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				connect_handler, context);
	g_assert(context->source > 0);

	g_io_channel_unref(io);

	context->sk = sk;
	context->data = data;

	return context;
}

static void execute_context(struct context *context)
{
	g_main_loop_run(context->main_loop);

	g_io_channel_shutdown(context->notif_io, TRUE, NULL);
	g_io_channel_shutdown(context->cmd_io, TRUE, NULL);
	g_io_channel_unref(context->cmd_io);
	g_io_channel_unref(context->notif_io);

	g_source_remove(context->notif_source);
	g_source_remove(context->cmd_source);
	g_source_remove(context->source);

	g_main_loop_unref(context->main_loop);

	g_free(context);
}

static void disconnected(void *data)
{
	struct context *context = data;

	g_assert(context->data->disconnect);

	context_quit(context);
}

static void test_init(gconstpointer data)
{
	struct context *context = create_context(data);

	ipc = ipc_init(HAL_SK_PATH, sizeof(HAL_SK_PATH), SERVICE_ID_MAX,
						true, NULL, NULL);

	g_assert(ipc);

	execute_context(context);

	ipc_cleanup(ipc);
	ipc = NULL;
}

static gboolean send_cmd(gpointer user_data)
{
	struct context *context = user_data;
	const struct test_data *test_data = context->data;
	int sk;

	sk = g_io_channel_unix_get_fd(context->cmd_io);
	g_assert(sk >= 0);

	g_assert(write(sk, test_data->cmd, test_data->cmd_size) ==
						test_data->cmd_size);

	return FALSE;
}

static gboolean register_service(gpointer user_data)
{
	struct context *context = user_data;
	const struct test_data *test_data = context->data;

	ipc_register(ipc, test_data->service, test_data->handlers,
						test_data->handlers_size);

	return FALSE;
}

static gboolean unregister_service(gpointer user_data)
{
	struct context *context = user_data;
	const struct test_data *test_data = context->data;

	ipc_unregister(ipc, test_data->service);

	return FALSE;
}

static void test_cmd(gconstpointer data)
{
	struct context *context = create_context(data);

	ipc = ipc_init(HAL_SK_PATH, sizeof(HAL_SK_PATH), SERVICE_ID_MAX,
					true, disconnected, context);

	g_assert(ipc);

	g_idle_add(send_cmd, context);

	execute_context(context);

	ipc_cleanup(ipc);
	ipc = NULL;
}

static void test_cmd_reg(gconstpointer data)
{
	struct context *context = create_context(data);
	const struct test_data *test_data = context->data;

	ipc = ipc_init(HAL_SK_PATH, sizeof(HAL_SK_PATH), SERVICE_ID_MAX,
					true, disconnected, context);

	g_assert(ipc);

	g_idle_add(register_service, context);
	g_idle_add(send_cmd, context);

	execute_context(context);

	ipc_unregister(ipc, test_data->service);

	ipc_cleanup(ipc);
	ipc = NULL;
}

static void test_cmd_reg_1(gconstpointer data)
{
	struct context *context = create_context(data);

	ipc = ipc_init(HAL_SK_PATH, sizeof(HAL_SK_PATH), SERVICE_ID_MAX,
					true, disconnected, context);

	g_assert(ipc);

	g_idle_add(register_service, context);
	g_idle_add(unregister_service, context);
	g_idle_add(send_cmd, context);

	execute_context(context);

	ipc_cleanup(ipc);
	ipc = NULL;
}

static void test_cmd_handler_1(const void *buf, uint16_t len)
{
	ipc_send_rsp(ipc, 0, 1, 0);
}

static void test_cmd_handler_2(const void *buf, uint16_t len)
{
	ipc_send_rsp(ipc, 0, 2, 0);
}

static void test_cmd_handler_invalid(const void *buf, uint16_t len)
{
	g_assert(false);
}

static const struct test_data test_init_1 = {};

static const struct ipc_hdr test_cmd_1_hdr = {
	.service_id = 0,
	.opcode = 1,
	.len = 0
};

static const struct ipc_hdr test_cmd_2_hdr = {
	.service_id = 0,
	.opcode = 2,
	.len = 0
};

static const struct test_data test_cmd_service_invalid_1 = {
	.cmd = &test_cmd_1_hdr,
	.cmd_size = sizeof(test_cmd_1_hdr),
	.disconnect = true,
};

static const struct ipc_handler cmd_handlers[] = {
	{ test_cmd_handler_1, false, 0 }
};

static const struct test_data test_cmd_service_valid_1 = {
	.cmd = &test_cmd_1_hdr,
	.cmd_size = sizeof(test_cmd_1_hdr),
	.service = 0,
	.handlers = cmd_handlers,
	.handlers_size = 1
};

static const struct test_data test_cmd_service_invalid_2 = {
	.cmd = &test_cmd_1_hdr,
	.cmd_size = sizeof(test_cmd_1_hdr),
	.service = 0,
	.handlers = cmd_handlers,
	.handlers_size = 1,
	.disconnect = true,
};

static const struct ipc_handler cmd_handlers_invalid_2[] = {
	{ test_cmd_handler_1, false, 0 },
	{ test_cmd_handler_invalid, false, 0 }
};

static const struct ipc_handler cmd_handlers_invalid_1[] = {
	{ test_cmd_handler_invalid, false, 0 },
	{ test_cmd_handler_2, false, 0 },
};

static const struct test_data test_cmd_opcode_valid_1 = {
	.cmd = &test_cmd_1_hdr,
	.cmd_size = sizeof(test_cmd_1_hdr),
	.service = 0,
	.handlers = cmd_handlers_invalid_2,
	.handlers_size = 2,
};

static const struct test_data test_cmd_opcode_valid_2 = {
	.cmd = &test_cmd_2_hdr,
	.cmd_size = sizeof(test_cmd_2_hdr),
	.service = 0,
	.handlers = cmd_handlers_invalid_1,
	.handlers_size = 2,
};

static const struct test_data test_cmd_opcode_invalid_1 = {
	.cmd = &test_cmd_2_hdr,
	.cmd_size = sizeof(test_cmd_2_hdr),
	.service = 0,
	.handlers = cmd_handlers,
	.handlers_size = 1,
	.disconnect = true,
};

static const struct test_data test_cmd_hdr_invalid = {
	.cmd = &test_cmd_1_hdr,
	.cmd_size = sizeof(test_cmd_1_hdr) - 1,
	.service = 0,
	.handlers = cmd_handlers,
	.handlers_size = 1,
	.disconnect = true,
};

#define VARDATA_EX1 "some data example"

struct vardata {
	struct ipc_hdr hdr;
	uint8_t data[IPC_MTU - sizeof(struct ipc_hdr)];
} __attribute__((packed));

static const struct vardata test_cmd_vardata = {
	.hdr.service_id = 0,
	.hdr.opcode = 1,
	.hdr.len = sizeof(VARDATA_EX1),
	.data = VARDATA_EX1,
};

static const struct ipc_handler cmd_vardata_handlers[] = {
	{ test_cmd_handler_1, true, sizeof(VARDATA_EX1) }
};

static const struct test_data test_cmd_vardata_valid = {
	.cmd = &test_cmd_vardata,
	.cmd_size = sizeof(struct ipc_hdr) + sizeof(VARDATA_EX1),
	.service = 0,
	.handlers = cmd_vardata_handlers,
	.handlers_size = 1,
};

static const struct ipc_handler cmd_vardata_handlers_valid2[] = {
	{ test_cmd_handler_1, true, sizeof(VARDATA_EX1) - 1 }
};

static const struct test_data test_cmd_vardata_valid_2 = {
	.cmd = &test_cmd_vardata,
	.cmd_size = sizeof(struct ipc_hdr) + sizeof(VARDATA_EX1),
	.service = 0,
	.handlers = cmd_vardata_handlers_valid2,
	.handlers_size = 1,
};

static const struct test_data test_cmd_vardata_invalid_1 = {
	.cmd = &test_cmd_vardata,
	.cmd_size = sizeof(struct ipc_hdr) + sizeof(VARDATA_EX1) - 1,
	.service = 0,
	.handlers = cmd_vardata_handlers,
	.handlers_size = 1,
	.disconnect = true,
};

static const struct ipc_hdr test_cmd_service_offrange_hdr = {
	.service_id = SERVICE_ID_MAX + 1,
	.opcode = 1,
	.len = 0
};

static const struct test_data test_cmd_service_offrange = {
	.cmd = &test_cmd_service_offrange_hdr,
	.cmd_size = sizeof(struct ipc_hdr),
	.service = 0,
	.handlers = cmd_handlers,
	.handlers_size = 1,
	.disconnect = true,
};

static const struct vardata test_cmd_invalid_data_1 = {
	.hdr.service_id = 0,
	.hdr.opcode = 1,
	.hdr.len = sizeof(VARDATA_EX1),
	.data = VARDATA_EX1,
};

static const struct test_data test_cmd_msg_invalid_1 = {
	.cmd = &test_cmd_invalid_data_1,
	.cmd_size = sizeof(struct ipc_hdr) + sizeof(VARDATA_EX1) - 1,
	.service = 0,
	.handlers = cmd_handlers,
	.handlers_size = 1,
	.disconnect = true,
};

static const struct vardata test_cmd_invalid_data_2 = {
	.hdr.service_id = 0,
	.hdr.opcode = 1,
	.hdr.len = sizeof(VARDATA_EX1) - 1,
	.data = VARDATA_EX1,
};

static const struct test_data test_cmd_msg_invalid_2 = {
	.cmd = &test_cmd_invalid_data_2,
	.cmd_size = sizeof(struct ipc_hdr) + sizeof(VARDATA_EX1),
	.service = 0,
	.handlers = cmd_handlers,
	.handlers_size = 1,
	.disconnect = true,
};

int main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);

	if (g_test_verbose())
		__btd_log_init("*", 0);

	g_test_add_data_func("/android_ipc/init", &test_init_1, test_init);
	g_test_add_data_func("/android_ipc/service_invalid_1",
				&test_cmd_service_invalid_1, test_cmd);
	g_test_add_data_func("/android_ipc/service_valid_1",
				&test_cmd_service_valid_1, test_cmd_reg);
	g_test_add_data_func("/android_ipc/service_invalid_2",
				&test_cmd_service_invalid_2, test_cmd_reg_1);
	g_test_add_data_func("/android_ipc/opcode_valid_1",
				&test_cmd_opcode_valid_1, test_cmd_reg);
	g_test_add_data_func("/android_ipc/opcode_valid_2",
				&test_cmd_opcode_valid_2, test_cmd_reg);
	g_test_add_data_func("/android_ipc/opcode_invalid_1",
				&test_cmd_opcode_invalid_1, test_cmd_reg);
	g_test_add_data_func("/android_ipc/vardata_valid",
				&test_cmd_vardata_valid, test_cmd_reg);
	g_test_add_data_func("/android_ipc/vardata_valid_2",
				&test_cmd_vardata_valid_2, test_cmd_reg);
	g_test_add_data_func("/android_ipc/vardata_invalid_1",
				&test_cmd_vardata_invalid_1, test_cmd_reg);
	g_test_add_data_func("/android_ipc/service_offrange",
				&test_cmd_service_offrange, test_cmd_reg);
	g_test_add_data_func("/android_ipc/hdr_invalid",
				&test_cmd_hdr_invalid, test_cmd_reg);
	g_test_add_data_func("/android_ipc/msg_invalid_1",
				&test_cmd_msg_invalid_1, test_cmd_reg);
	g_test_add_data_func("/android_ipc/msg_invalid_2",
				&test_cmd_msg_invalid_2, test_cmd_reg);

	return g_test_run();
}
