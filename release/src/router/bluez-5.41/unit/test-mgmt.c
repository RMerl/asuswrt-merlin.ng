/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
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

#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/mgmt.h"

#include "src/shared/mgmt.h"

struct context {
	GMainLoop *main_loop;
	int fd;
	struct mgmt *mgmt_client;
	guint server_source;
	GList *handler_list;
};

enum action {
	ACTION_PASSED,
	ACTION_IGNORE,
	ACTION_RESPOND,
};

struct handler {
	const void *cmd_data;
	uint16_t cmd_size;
	const void *rsp_data;
	uint16_t rsp_size;
	uint8_t rsp_status;
	bool match_prefix;
	enum action action;
};

static void mgmt_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	g_print("%s%s\n", prefix, str);
}

static void context_quit(struct context *context)
{
	g_main_loop_quit(context->main_loop);
}

static void check_actions(struct context *context, int fd,
					const void *data, uint16_t size)
{
	GList *list;

	for (list = g_list_first(context->handler_list); list;
						list = g_list_next(list)) {
		struct handler *handler = list->data;
		int ret;

		if (handler->match_prefix) {
			if (size < handler->cmd_size)
				continue;
		} else {
			if (size != handler->cmd_size)
				continue;
		}

		if (memcmp(data, handler->cmd_data, handler->cmd_size))
			continue;

		switch (handler->action) {
		case ACTION_PASSED:
			context_quit(context);
			return;
		case ACTION_RESPOND:
			ret = write(fd, handler->rsp_data, handler->rsp_size);
			g_assert(ret >= 0);
			return;
		case ACTION_IGNORE:
			return;
		}
	}

	g_test_message("Command not handled\n");
	g_assert_not_reached();
}

static gboolean server_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	struct context *context = user_data;
	unsigned char buf[512];
	ssize_t result;
	int fd;

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP))
		return FALSE;

	fd = g_io_channel_unix_get_fd(channel);

	result = read(fd, buf, sizeof(buf));
	if (result < 0)
		return FALSE;

	check_actions(context, fd, buf, result);

	return TRUE;
}

static struct context *create_context(void)
{
	struct context *context = g_new0(struct context, 1);
	GIOChannel *channel;
	int err, sv[2];

	context->main_loop = g_main_loop_new(NULL, FALSE);
	g_assert(context->main_loop);

	err = socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sv);
	g_assert(err == 0);

	context->fd = sv[0];
	channel = g_io_channel_unix_new(sv[0]);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	context->server_source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				server_handler, context);
	g_assert(context->server_source > 0);

	g_io_channel_unref(channel);

	context->mgmt_client = mgmt_new(sv[1]);
	g_assert(context->mgmt_client);

	if (g_test_verbose() == TRUE)
		mgmt_set_debug(context->mgmt_client,
					mgmt_debug, "mgmt: ", NULL);

	mgmt_set_close_on_unref(context->mgmt_client, true);

	return context;
}

static void execute_context(struct context *context)
{
	g_main_loop_run(context->main_loop);

	g_list_free_full(context->handler_list, g_free);

	g_source_remove(context->server_source);

	mgmt_unref(context->mgmt_client);

	g_main_loop_unref(context->main_loop);

	g_free(context);
}

static void add_action(struct context *context,
				const void *cmd_data, uint16_t cmd_size,
				const void *rsp_data, uint16_t rsp_size,
				uint8_t rsp_status, bool match_prefix,
				enum action action)
{
	struct handler *handler = g_new0(struct handler, 1);

	handler->cmd_data = cmd_data;
	handler->cmd_size = cmd_size;
	handler->rsp_data = rsp_data;
	handler->rsp_size = rsp_size;
	handler->rsp_status = rsp_status;
	handler->match_prefix = match_prefix;
	handler->action = action;

	context->handler_list = g_list_append(context->handler_list, handler);
}

struct command_test_data {
	uint16_t opcode;
	uint16_t index;
	uint16_t length;
	const void *param;
	const void *cmd_data;
	uint16_t cmd_size;
	const void *rsp_data;
	uint16_t rsp_size;
	uint8_t rsp_status;
};

static const unsigned char read_version_command[] =
				{ 0x01, 0x00, 0xff, 0xff, 0x00, 0x00 };
static const unsigned char read_version_response[] =
				{ 0x01, 0x00, 0xff, 0xff, 0x06, 0x00,
				0x01, 0x00, 0x00, 0x01, 0x06, 0x00 };

static const struct command_test_data command_test_1 = {
	.opcode = MGMT_OP_READ_VERSION,
	.index = MGMT_INDEX_NONE,
	.cmd_data = read_version_command,
	.cmd_size = sizeof(read_version_command),
	.rsp_data = read_version_response,
	.rsp_size = sizeof(read_version_response),
	.rsp_status = MGMT_STATUS_SUCCESS,
};

static const unsigned char read_info_command[] =
				{ 0x04, 0x00, 0x00, 0x02, 0x00, 0x00 };

static const struct command_test_data command_test_2 = {
	.opcode = MGMT_OP_READ_INFO,
	.index = 512,
	.cmd_data = read_info_command,
	.cmd_size = sizeof(read_info_command),
};

static const unsigned char invalid_index_response[] =
				{ 0x02, 0x00, 0xff, 0xff, 0x03, 0x00,
				0x01, 0x00, 0x11 };

static const struct command_test_data command_test_3 = {
	.opcode = MGMT_OP_READ_VERSION,
	.index = MGMT_INDEX_NONE,
	.cmd_data = read_version_command,
	.cmd_size = sizeof(read_version_command),
	.rsp_data = invalid_index_response,
	.rsp_size = sizeof(invalid_index_response),
	.rsp_status = MGMT_STATUS_INVALID_INDEX,
};

static const unsigned char event_index_added[] =
				{ 0x04, 0x00, 0x01, 0x00, 0x00, 0x00 };

static const struct command_test_data event_test_1 = {
	.opcode = MGMT_EV_INDEX_ADDED,
	.index = MGMT_INDEX_NONE,
	.cmd_data = event_index_added,
	.cmd_size = sizeof(event_index_added),
};

static void test_command(gconstpointer data)
{
	const struct command_test_data *test = data;
	struct context *context = create_context();

	add_action(context, test->cmd_data, test->cmd_size,
			test->rsp_data, test->rsp_size, test->rsp_status,
			false, ACTION_PASSED);

	mgmt_send(context->mgmt_client, test->opcode, test->index,
					test->length, test->param,
						NULL ,NULL, NULL);

	execute_context(context);
}

static void response_cb(uint8_t status, uint16_t length, const void *param,
							void *user_data)
{
	struct context *context = user_data;
	struct handler *handler = context->handler_list->data;

	g_assert_cmpint(status, ==, handler->rsp_status);

	context_quit(context);
}

static void test_response(gconstpointer data)
{
	const struct command_test_data *test = data;
	struct context *context = create_context();

	add_action(context, test->cmd_data, test->cmd_size,
			test->rsp_data, test->rsp_size, test->rsp_status,
			false, ACTION_RESPOND);

	mgmt_send(context->mgmt_client, test->opcode, test->index,
					test->length, test->param,
					response_cb, context, NULL);

	execute_context(context);
}

static void event_cb(uint16_t index, uint16_t length, const void *param,
							void *user_data)
{
	struct context *context = user_data;

	if (g_test_verbose())
		printf("Event received\n");

	context_quit(context);
}

static void test_event(gconstpointer data)
{
	const struct command_test_data *test = data;
	struct context *context = create_context();

	mgmt_register(context->mgmt_client, test->opcode, test->index,
						event_cb, context, NULL);

	g_assert_cmpint(write(context->fd, test->cmd_data, test->cmd_size), ==,
								test->cmd_size);

	execute_context(context);
}

static void test_event2(gconstpointer data)
{
	const struct command_test_data *test = data;
	struct context *context = create_context();

	mgmt_register(context->mgmt_client, test->opcode, test->index,
						event_cb, context, NULL);
	mgmt_register(context->mgmt_client, test->opcode, test->index,
						event_cb, context, NULL);

	g_assert_cmpint(write(context->fd, test->cmd_data, test->cmd_size), ==,
								test->cmd_size);

	execute_context(context);
}

static void unregister_all_cb(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct context *context = user_data;

	mgmt_unregister_all(context->mgmt_client);

	context_quit(context);
}

static void test_unregister_all(gconstpointer data)
{
	const struct command_test_data *test = data;
	struct context *context = create_context();

	mgmt_register(context->mgmt_client, test->opcode, test->index,
					unregister_all_cb, context, NULL);
	mgmt_register(context->mgmt_client, test->opcode, test->index,
						event_cb, context, NULL);

	g_assert_cmpint(write(context->fd, test->cmd_data, test->cmd_size), ==,
								test->cmd_size);

	execute_context(context);
}

static void unregister_index_cb(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct context *context = user_data;

	mgmt_unregister_index(context->mgmt_client, index);

	context_quit(context);
}

static void destroy_cb(uint16_t index, uint16_t length, const void *param,
							void *user_data)
{
	struct context *context = user_data;

	mgmt_unref(context->mgmt_client);
	context->mgmt_client = NULL;

	context_quit(context);
}

static void test_unregister_index(gconstpointer data)
{
	const struct command_test_data *test = data;
	struct context *context = create_context();

	mgmt_register(context->mgmt_client, test->opcode, test->index,
					unregister_index_cb, context, NULL);
	mgmt_register(context->mgmt_client, test->opcode, test->index,
						event_cb, context, NULL);

	g_assert_cmpint(write(context->fd, test->cmd_data, test->cmd_size), ==,
								test->cmd_size);

	execute_context(context);
}

static void test_destroy(gconstpointer data)
{
	const struct command_test_data *test = data;
	struct context *context = create_context();

	mgmt_register(context->mgmt_client, test->opcode, test->index,
					destroy_cb, context, NULL);
	mgmt_register(context->mgmt_client, test->opcode, test->index,
						event_cb, context, NULL);

	g_assert_cmpint(write(context->fd, test->cmd_data, test->cmd_size), ==,
								test->cmd_size);

	execute_context(context);
}

int main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);

	g_test_add_data_func("/mgmt/command/1", &command_test_1, test_command);
	g_test_add_data_func("/mgmt/command/2", &command_test_2, test_command);

	g_test_add_data_func("/mgmt/response/1", &command_test_1,
								test_response);
	g_test_add_data_func("/mgmt/response/2", &command_test_3,
								test_response);

	g_test_add_data_func("/mgmt/event/1", &event_test_1, test_event);
	g_test_add_data_func("/mgmt/event/2", &event_test_1, test_event2);

	g_test_add_data_func("/mgmt/unregister/1", &event_test_1,
							test_unregister_all);
	g_test_add_data_func("/mgmt/unregister/2", &event_test_1,
							test_unregister_index);

	g_test_add_data_func("/mgmt/destroy/1", &event_test_1, test_destroy);

	return g_test_run();
}
