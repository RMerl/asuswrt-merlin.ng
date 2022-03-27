/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
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

#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include "src/shared/hfp.h"
#include "src/shared/tester.h"

struct context {
	guint watch_id;
	int fd_server;
	int fd_client;
	struct hfp_gw *hfp;
	struct hfp_hf *hfp_hf;
	const struct test_data *data;
	unsigned int pdu_offset;
};

struct test_pdu {
	bool valid;
	const uint8_t *data;
	size_t size;
	enum hfp_gw_cmd_type type;
	bool fragmented;
};

struct test_data {
	char *test_name;
	struct test_pdu *pdu_list;
	hfp_result_func_t result_func;
	hfp_response_func_t response_func;
	hfp_hf_result_func_t hf_result_func;
	GIOFunc test_handler;
};

#define data(args...) ((const unsigned char[]) { args })

#define raw_pdu(args...)					\
	{							\
		.valid = true,					\
		.data = data(args),				\
		.size = sizeof(data(args)),			\
	}

#define data_end()						\
	{							\
		.valid = false,					\
	}

#define type_pdu(cmd_type, args...)				\
	{							\
		.valid = true,					\
		.data = data(args),				\
		.size = sizeof(data(args)),			\
		.type = cmd_type,				\
	}

#define frg_pdu(args...)					\
	{							\
		.valid = true,					\
		.data = data(args),				\
		.size = sizeof(data(args)),			\
		.fragmented = true,				\
	}

#define define_test(name, function, result_function, args...)		\
	do {								\
		const struct test_pdu pdus[] = {			\
			args, { }					\
		};							\
		static struct test_data data;				\
		data.test_name = g_strdup(name);			\
		data.pdu_list = g_memdup(pdus, sizeof(pdus));		\
		data.result_func = result_function;			\
		tester_add(name, &data, NULL, function, NULL);		\
		data.test_handler = test_handler;			\
	} while (0)

#define define_hf_test(name, function, result_func, response_function,	\
								args...)\
	do {								\
		const struct test_pdu pdus[] = {			\
			args, { }					\
		};							\
		static struct test_data data;				\
		data.test_name = g_strdup(name);			\
		data.pdu_list = g_memdup(pdus, sizeof(pdus));		\
		data.hf_result_func = result_func;			\
		data.response_func = response_function;			\
		tester_add(name, &data, NULL, function, NULL);		\
		data.test_handler = test_hf_handler;			\
	} while (0)

static void test_free(gconstpointer user_data)
{
	const struct test_data *data = user_data;

	g_free(data->test_name);
	g_free(data->pdu_list);
}

static void destroy_context(struct context *context)
{
	if (context->watch_id)
		g_source_remove(context->watch_id);

	test_free(context->data);

	if (context->hfp)
		hfp_gw_unref(context->hfp);

	if (context->hfp_hf)
		hfp_hf_unref(context->hfp_hf);

	g_free(context);
}

static gboolean context_quit(gpointer user_data)
{
	struct context *context = user_data;

	if (context == NULL)
		return FALSE;

	destroy_context(context);
	tester_test_passed();
	return FALSE;
}

static gboolean test_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;

	pdu = &context->data->pdu_list[context->pdu_offset++];

	g_assert(!pdu->valid);
	context->watch_id = 0;

	context_quit(context);

	return FALSE;
}

static gboolean send_pdu(gpointer user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;
	ssize_t len;

	pdu = &context->data->pdu_list[context->pdu_offset++];
	if (!pdu || !pdu->valid)
		return FALSE;

	len = write(context->fd_server, pdu->data, pdu->size);
	g_assert_cmpint(len, ==, pdu->size);

	pdu = &context->data->pdu_list[context->pdu_offset];
	if (pdu->fragmented)
		g_idle_add(send_pdu, context);

	return FALSE;
}

static gboolean test_hf_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	struct context *context = user_data;
	gchar buf[60];
	gsize bytes_read;
	GError *error = NULL;

	if (cond & (G_IO_HUP | G_IO_ERR | G_IO_NVAL))
		goto done;

	/* dummy read */
	g_io_channel_read_chars(channel, buf, 60, &bytes_read, &error);

	send_pdu(context);

	return TRUE;

done:
	context->watch_id = 0;

	context_quit(context);

	return FALSE;
}

static void cmd_handler(const char *command, void *user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;
	unsigned int cmd_len = strlen(command);

	pdu = &context->data->pdu_list[context->pdu_offset++];

	g_assert(cmd_len == pdu->size);
	g_assert(!memcmp(command, pdu->data, cmd_len));

	hfp_gw_send_result(context->hfp, HFP_RESULT_ERROR);
}

static void prefix_handler(struct hfp_context *result,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;

	pdu = &context->data->pdu_list[context->pdu_offset++];

	g_assert(type == pdu->type);

	hfp_gw_send_result(context->hfp, HFP_RESULT_ERROR);
}

static struct context *create_context(gconstpointer data)
{
	struct context *context = g_new0(struct context, 1);
	GIOChannel *channel;
	int err, sv[2];
	const struct test_data *d = data;

	err = socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sv);
	g_assert(err == 0);

	channel = g_io_channel_unix_new(sv[1]);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	context->watch_id = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				d->test_handler, context);

	g_assert(context->watch_id > 0);

	g_io_channel_unref(channel);

	context->fd_server = sv[1];
	context->fd_client = sv[0];
	context->data = data;

	return context;
}

static void test_init(gconstpointer data)
{
	struct context *context = create_context(data);

	context->hfp = hfp_gw_new(context->fd_client);

	g_assert(context->hfp);
	g_assert(hfp_gw_set_close_on_unref(context->hfp, true));

	hfp_gw_unref(context->hfp);
	context->hfp = NULL;

	context_quit(context);
}

static void test_command_handler(gconstpointer data)
{
	struct context *context = create_context(data);
	const struct test_pdu *pdu;
	ssize_t len;
	bool ret;

	context->hfp = hfp_gw_new(context->fd_client);
	g_assert(context->hfp);

	pdu = &context->data->pdu_list[context->pdu_offset++];

	ret = hfp_gw_set_close_on_unref(context->hfp, true);
	g_assert(ret);

	ret = hfp_gw_set_command_handler(context->hfp, cmd_handler,
								context, NULL);
	g_assert(ret);

	len = write(context->fd_server, pdu->data, pdu->size);
	g_assert_cmpint(len, ==, pdu->size);

	context_quit(context);
}

static void test_register(gconstpointer data)
{
	struct context *context = create_context(data);
	const struct test_pdu *pdu;
	ssize_t len;
	bool ret;

	context->hfp = hfp_gw_new(context->fd_client);
	g_assert(context->hfp);

	pdu = &context->data->pdu_list[context->pdu_offset++];

	ret = hfp_gw_set_close_on_unref(context->hfp, true);
	g_assert(ret);

	if (context->data->result_func) {
		ret = hfp_gw_register(context->hfp, context->data->result_func,
					(char *)pdu->data, context, NULL);
		g_assert(ret);
	}

	pdu = &context->data->pdu_list[context->pdu_offset++];

	len = write(context->fd_server, pdu->data, pdu->size);
	g_assert_cmpint(len, ==, pdu->size);

	context_quit(context);
}

static void test_fragmented(gconstpointer data)
{
	struct context *context = create_context(data);
	bool ret;

	context->hfp = hfp_gw_new(context->fd_client);
	g_assert(context->hfp);

	ret = hfp_gw_set_close_on_unref(context->hfp, true);
	g_assert(ret);

	g_idle_add(send_pdu, context);
}

static void test_send_and_close(gconstpointer data)
{
	struct context *context = create_context(data);
	bool ret;

	context->hfp = hfp_gw_new(context->fd_client);
	g_assert(context->hfp);

	ret = hfp_gw_set_close_on_unref(context->hfp, true);
	g_assert(ret);

	send_pdu(context);

	hfp_gw_unref(context->hfp);
	context->hfp = NULL;

	context_quit(context);
}

static void check_ustring_1(struct hfp_context *result,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;
	unsigned int i = 3, j = 0;
	char str[10];

	pdu = &context->data->pdu_list[context->pdu_offset++];

	g_assert(type == pdu->type);

	g_assert(hfp_context_get_unquoted_string(result, str, sizeof(str)));

	while (context->data->pdu_list[1].data[i] != '\r') {
		g_assert(j < sizeof(str));
		g_assert(str[j] == context->data->pdu_list[1].data[i]);

		i++;
		j++;
	}

	g_assert(str[j] == '\0');

	hfp_gw_send_result(context->hfp, HFP_RESULT_ERROR);
}

static void check_ustring_2(struct hfp_context *result,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;
	char str[10];

	memset(str, 'X', sizeof(str));

	pdu = &context->data->pdu_list[context->pdu_offset++];

	g_assert(type == pdu->type);

	g_assert(!hfp_context_get_unquoted_string(result, str, 3));

	g_assert(str[3] == 'X');

	hfp_gw_send_result(context->hfp, HFP_RESULT_ERROR);
}

static void check_string_1(struct hfp_context *result,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;
	unsigned int i = 4, j = 0;
	char str[10];

	pdu = &context->data->pdu_list[context->pdu_offset++];

	g_assert(type == pdu->type);

	g_assert(hfp_context_get_string(result, str, sizeof(str)));

	while (context->data->pdu_list[1].data[i] != '\"') {
		g_assert(j < sizeof(str));
		g_assert(str[j] == context->data->pdu_list[1].data[i]);

		i++;
		j++;
	}

	g_assert(context->data->pdu_list[1].data[i] == '\"');
	g_assert(str[j] == '\0');

	hfp_gw_send_result(context->hfp, HFP_RESULT_ERROR);
}

static void check_string_2(struct hfp_context *result,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;
	char str[10];

	memset(str, 'X', sizeof(str));

	pdu = &context->data->pdu_list[context->pdu_offset++];

	g_assert(type == pdu->type);

	g_assert(!hfp_context_get_string(result, str, 3));

	g_assert(str[3] == 'X');

	hfp_gw_send_result(context->hfp, HFP_RESULT_ERROR);
}

static void check_string_3(struct hfp_context *result,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;

	pdu = &context->data->pdu_list[context->pdu_offset++];

	g_assert(type == pdu->type);

	hfp_gw_send_result(context->hfp, HFP_RESULT_ERROR);
}

static void test_hf_init(gconstpointer data)
{
	struct context *context = create_context(data);

	context->hfp_hf = hfp_hf_new(context->fd_client);
	g_assert(context->hfp_hf);
	g_assert(hfp_hf_set_close_on_unref(context->hfp_hf, true));

	hfp_hf_unref(context->hfp_hf);
	context->hfp_hf = NULL;

	context_quit(context);
}

static bool unsolicited_resp = false;

static void hf_unsolicited_resp_cb(struct hfp_context *context,
							void *user_data) {
	unsolicited_resp = true;
}

static void hf_response_with_data(enum hfp_result res,
							enum hfp_error cme_err,
							void *user_data)
{
	struct context *context = user_data;

	g_assert(unsolicited_resp);
	unsolicited_resp = false;

	hfp_hf_disconnect(context->hfp_hf);
}

static void hf_cme_error_response_cb(enum hfp_result res,
							enum hfp_error cme_err,
							void *user_data)
{
	struct context *context = user_data;

	g_assert_cmpint(res, ==, HFP_RESULT_CME_ERROR);
	g_assert_cmpint(cme_err, ==, 30);

	hfp_hf_disconnect(context->hfp_hf);
}

static void hf_response_cb(enum hfp_result res, enum hfp_error cme_err,
							void *user_data)
{
	struct context *context = user_data;

	hfp_hf_disconnect(context->hfp_hf);
}

static void test_hf_send_command(gconstpointer data)
{
	struct context *context = create_context(data);
	const struct test_pdu *pdu;
	bool ret;

	context->hfp_hf = hfp_hf_new(context->fd_client);
	g_assert(context->hfp_hf);

	ret = hfp_hf_set_close_on_unref(context->hfp_hf, true);
	g_assert(ret);

	if (context->data->response_func) {
		if (context->data->hf_result_func) {
			pdu = &context->data->pdu_list[context->pdu_offset++];

			ret = hfp_hf_register(context->hfp_hf,
						context->data->hf_result_func,
						(char *)pdu->data,
						NULL, NULL);
			g_assert(ret);
		}

		pdu = &context->data->pdu_list[context->pdu_offset++];

		ret = hfp_hf_send_command(context->hfp_hf,
						context->data->response_func,
						context, (char *)pdu->data);
		g_assert(ret);
	}

	context_quit(context);
}
static void hf_chld_result_handler(struct hfp_context *hf_context,
							void *user_data)
{
	struct context *context = user_data;
	char str[3];

	g_assert(hf_context);
	g_assert(hfp_context_get_unquoted_string(hf_context, str,
							sizeof(str)));
	g_assert_cmpstr(str, ==, "1");
	g_assert(hfp_context_get_unquoted_string(hf_context, str,
							sizeof(str)));
	g_assert_cmpstr(str, ==, "2x");

	hfp_hf_disconnect(context->hfp_hf);
}

static void hf_chld_skip_field(struct hfp_context *hf_context,
							void *user_data)
{
	struct context *context = user_data;
	char str[3];

	g_assert(hf_context);

	hfp_context_skip_field(hf_context);

	g_assert(hfp_context_get_unquoted_string(hf_context, str,
								sizeof(str)));
	g_assert_cmpstr(str, ==, "2x");

	hfp_hf_disconnect(context->hfp_hf);
}

static void hf_clcc_result_handler(struct hfp_context *hf_context,
							void *user_data)
{
	struct context *context = user_data;
	char name[10];
	uint32_t val1, val2;

	g_assert(hf_context);
	g_assert(hfp_context_open_container(hf_context));
	g_assert(hfp_context_get_string(hf_context, name, sizeof(name)));
	g_assert_cmpstr(name, ==, "call");
	g_assert(hfp_context_open_container(hf_context));
	g_assert(hfp_context_get_number(hf_context, &val1));
	g_assert_cmpint(val1, ==, 0);
	g_assert(hfp_context_get_number(hf_context, &val1));
	g_assert_cmpint(val1, ==, 1);
	g_assert(hfp_context_close_container(hf_context));
	g_assert(hfp_context_close_container(hf_context));

	g_assert(hfp_context_open_container(hf_context));
	g_assert(hfp_context_get_string(hf_context, name, sizeof(name)));
	g_assert_cmpstr(name, ==, "callsetup");
	g_assert(hfp_context_open_container(hf_context));
	g_assert(hfp_context_get_range(hf_context, &val1, &val2));
	g_assert_cmpint(val1, ==, 0);
	g_assert_cmpint(val2, ==, 3);
	g_assert(hfp_context_close_container(hf_context));
	g_assert(hfp_context_close_container(hf_context));

	hfp_hf_disconnect(context->hfp_hf);
}

static void hf_result_handler(struct hfp_context *result,
							void *user_data)
{
	struct context *context = user_data;

	hfp_hf_disconnect(context->hfp_hf);
}

static void test_hf_unsolicited(gconstpointer data)
{
	struct context *context = create_context(data);
	bool ret;

	context->hfp_hf = hfp_hf_new(context->fd_client);
	g_assert(context->hfp_hf);

	ret = hfp_hf_set_close_on_unref(context->hfp_hf, true);
	g_assert(ret);

	if (context->data->hf_result_func) {
		const struct test_pdu *pdu;

		pdu = &context->data->pdu_list[context->pdu_offset++];

		ret = hfp_hf_register(context->hfp_hf,
						context->data->hf_result_func,
						(char *)pdu->data, context,
						NULL);
		g_assert(ret);
	}

	send_pdu(context);
}

static void test_hf_robustness(gconstpointer data)
{
	struct context *context = create_context(data);
	bool ret;

	context->hfp_hf = hfp_hf_new(context->fd_client);
	g_assert(context->hfp_hf);

	ret = hfp_hf_set_close_on_unref(context->hfp_hf, true);
	g_assert(ret);

	send_pdu(context);

	hfp_hf_unref(context->hfp_hf);
	context->hfp_hf = NULL;

	context_quit(context);
}

int main(int argc, char *argv[])
{
	tester_init(&argc, &argv);

	define_test("/hfp/test_init", test_init, NULL, data_end());
	define_test("/hfp/test_cmd_handler_1", test_command_handler, NULL,
			raw_pdu('A', 'T', '+', 'B', 'R', 'S', 'F', '\r'),
			raw_pdu('A', 'T', '+', 'B', 'R', 'S', 'F'),
			data_end());
	define_test("/hfp/test_cmd_handler_2", test_command_handler, NULL,
			raw_pdu('A', 'T', 'D', '1', '2', '3', '4', '\r'),
			raw_pdu('A', 'T', 'D', '1', '2', '3', '4'),
			data_end());
	define_test("/hfp/test_register_1", test_register, prefix_handler,
			raw_pdu('+', 'B', 'R', 'S', 'F', '\0'),
			raw_pdu('A', 'T', '+', 'B', 'R', 'S', 'F', '\r'),
			type_pdu(HFP_GW_CMD_TYPE_COMMAND, 0),
			data_end());
	define_test("/hfp/test_register_2", test_register, prefix_handler,
			raw_pdu('+', 'B', 'R', 'S', 'F', '\0'),
			raw_pdu('A', 'T', '+', 'B', 'R', 'S', 'F', '=', '\r'),
			type_pdu(HFP_GW_CMD_TYPE_SET, 0),
			data_end());
	define_test("/hfp/test_register_3", test_register, prefix_handler,
			raw_pdu('+', 'B', 'R', 'S', 'F', '\0'),
			raw_pdu('A', 'T', '+', 'B', 'R', 'S', 'F', '?', '\r'),
			type_pdu(HFP_GW_CMD_TYPE_READ, 0),
			data_end());
	define_test("/hfp/test_register_4", test_register, prefix_handler,
			raw_pdu('+', 'B', 'R', 'S', 'F', '\0'),
			raw_pdu('A', 'T', '+', 'B', 'R', 'S', 'F', '=', '?',
									'\r'),
			type_pdu(HFP_GW_CMD_TYPE_TEST, 0),
			data_end());
	define_test("/hfp/test_register_5", test_register, prefix_handler,
			raw_pdu('D', '\0'),
			raw_pdu('A', 'T', 'D', '1', '2', '3', '4', '5', '\r'),
			type_pdu(HFP_GW_CMD_TYPE_SET, 0),
			data_end());
	define_test("/hfp/test_fragmented_1", test_fragmented, NULL,
			frg_pdu('A'), frg_pdu('T'), frg_pdu('+'), frg_pdu('B'),
			frg_pdu('R'), frg_pdu('S'), frg_pdu('F'), frg_pdu('\r'),
			data_end());
	define_test("/hfp/test_ustring_1", test_register, check_ustring_1,
			raw_pdu('D', '\0'),
			raw_pdu('A', 'T', 'D', '0', '1', '2', '3', '\r'),
			type_pdu(HFP_GW_CMD_TYPE_SET, 0),
			data_end());
	define_test("/hfp/test_ustring_2", test_register, check_ustring_2,
			raw_pdu('D', '\0'),
			raw_pdu('A', 'T', 'D', '0', '1', '2', '3', '\r'),
			type_pdu(HFP_GW_CMD_TYPE_SET, 0),
			data_end());
	define_test("/hfp/test_string_1", test_register, check_string_1,
			raw_pdu('D', '\0'),
			raw_pdu('A', 'T', 'D', '\"', '0', '1', '2', '3', '\"',
									'\r'),
			type_pdu(HFP_GW_CMD_TYPE_SET, 0),
			data_end());
	define_test("/hfp/test_string_2", test_register, check_string_2,
			raw_pdu('D', '\0'),
			raw_pdu('A', 'T', 'D', '\"', '0', '1', '2', '3', '\"',
									'\r'),
			type_pdu(HFP_GW_CMD_TYPE_SET, 0),
			data_end());
	define_test("/hfp/test_corrupted_1", test_register, check_string_3,
			raw_pdu('D', '\0'),
			raw_pdu('\r', 'A', 'T', 'D', '\"', '0', '1', '2', '3',
								'\"', '\r'),
			type_pdu(HFP_GW_CMD_TYPE_SET, 0),
			data_end());
	define_test("/hfp/test_empty", test_send_and_close, NULL,
			raw_pdu('\r'),
			data_end());
	define_hf_test("/hfp_hf/test_init", test_hf_init, NULL, NULL,
			data_end());
	define_hf_test("/hfp_hf/test_send_command_1", test_hf_send_command,
			NULL, hf_response_cb,
			raw_pdu('A', 'T', '+', 'B', 'R', 'S', 'F', '\0'),
			raw_pdu('\r', '\n', 'O', 'k', '\r', '\n'),
			data_end());

	define_hf_test("/hfp_hf/test_send_command_2", test_hf_send_command,
			hf_unsolicited_resp_cb,
			hf_response_with_data,
			raw_pdu('+', 'B', 'R', 'S', 'F', '\0'),
			raw_pdu('A', 'T', '+', 'B', 'R', 'S', 'F', '\0'),
			frg_pdu('\r', '\n', '+', 'B', 'R', 'S', 'F', '\r',
									'\n'),
			frg_pdu('\r', '\n', 'O', 'k', '\r', '\n'),
			data_end());

	define_hf_test("/hfp_hf/test_send_command_3", test_hf_send_command,
			NULL, hf_cme_error_response_cb,
			raw_pdu('A', 'T', '+', 'C', 'H', 'L', 'D', '=',
								'1', '\0'),
			frg_pdu('\r', '\n', '+', 'C', 'M', 'E', ' ', 'E'),
			frg_pdu('R', 'R', 'O', 'R', ':', '3', '0', '\r', '\n'),
			data_end());

	define_hf_test("/hfp_hf/test_unsolicited_1", test_hf_unsolicited,
			hf_result_handler, NULL,
			raw_pdu('+', 'C', 'L', 'C', 'C', '\0'),
			frg_pdu('\r', '\n', '+', 'C', 'L', 'C'),
			frg_pdu('C', '\r', '\n'),
			data_end());

	define_hf_test("/hfp_hf/test_unsolicited_2", test_hf_unsolicited,
			hf_result_handler, NULL,
			raw_pdu('+', 'C', 'L', 'C', 'C', '\0'),
			frg_pdu('\r', '\n', '+', 'C', 'L', 'C', 'C', ':', '1'),
			frg_pdu(',', '3', ',', '0', '\r', '\n'),
			data_end());

	define_hf_test("/hfp_hf/test_unsolicited_3", test_hf_unsolicited,
			hf_result_handler, NULL,
			raw_pdu('+', 'C', 'L', 'C', 'C', '\0'),
			frg_pdu('\r'), frg_pdu('\n'), frg_pdu('+'),
			frg_pdu('C'), frg_pdu('L'), frg_pdu('C'), frg_pdu('C'),
			frg_pdu(':'), frg_pdu('1'), frg_pdu(','), frg_pdu('3'),
			frg_pdu(','), frg_pdu('0'), frg_pdu('\r'),
			frg_pdu('\n'),
			data_end());

	define_hf_test("/hfp_hf/test_corrupted_1", test_hf_unsolicited,
			hf_result_handler, NULL,
			raw_pdu('+', 'C', 'L', 'C', 'C', '\0'),
			frg_pdu('\r', 'X', '\r', '\n'),
			frg_pdu('+', 'C', 'L', 'C', 'C', ':', '1', ',', '3'),
			frg_pdu(',', '0', '\r', '\n'),
			data_end());

	define_hf_test("/hfp_hf/test_corrupted_2", test_hf_unsolicited,
			hf_result_handler, NULL,
			raw_pdu('+', 'C', 'L', 'C', 'C', '\0'),
			raw_pdu('+', 'C', 'L', 'C', 'C', '\r', '\n'),
			data_end());

	define_hf_test("/hfp_hf/test_empty", test_hf_robustness, NULL, NULL,
			raw_pdu('\r'), data_end());

	define_hf_test("/hfp_hf/test_unknown", test_hf_robustness, NULL, NULL,
			raw_pdu('\r', '\n', 'B', 'R', '\r', '\n'),
			data_end());

	define_hf_test("/hfp_hf/test_context_parser_1", test_hf_unsolicited,
			hf_clcc_result_handler, NULL,
			raw_pdu('+', 'C', 'L', 'C', 'C', '\0'),
			frg_pdu('+', 'C', 'L', 'C', 'C', ':'),
			frg_pdu('(', '\"', 'c', 'a', 'l', 'l', '\"'),
			frg_pdu('(', '0', ',', '1', ')', ')', ','),
			frg_pdu('(', '\"', 'c', 'a', 'l', 'l', 's', 'e', 't'),
			frg_pdu('u', 'p', '\"', ',', '(', '0', '-', '3', ')'),
			frg_pdu(')', '\r', '\n'),
			data_end());

	define_hf_test("/hfp_hf/test_context_parser_2", test_hf_unsolicited,
			hf_chld_result_handler, NULL,
			raw_pdu('+', 'C', 'H', 'L', 'D', '\0'),
			frg_pdu('+', 'C', 'H', 'L', 'D', ':'),
			frg_pdu('1', ',', '2', 'x', '\r', '\n'),
			data_end());

	define_hf_test("/hfp_hf/test_context_skip_field", test_hf_unsolicited,
			hf_chld_skip_field, NULL,
			raw_pdu('+', 'C', 'H', 'L', 'D', '\0'),
			frg_pdu('+', 'C', 'H', 'L', 'D', ':'),
			frg_pdu('1', ',', '2', 'x', '\r', '\n'),
			data_end());

	return tester_run();
}
