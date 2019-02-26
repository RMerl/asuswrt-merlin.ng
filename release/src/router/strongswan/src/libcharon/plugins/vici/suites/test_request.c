/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <test_suite.h>

#include "../vici_dispatcher.h"
#include "../libvici.h"

#include <unistd.h>

#ifdef WIN32
# define URI "tcp://127.0.0.1:6543"
#else /* !WIN32 */
# define URI "unix:///tmp/strongswan-vici-request-test"
#endif /* !WIN32 */

static void encode_section(vici_req_t *req)
{
	vici_begin_section(req, "section1");
	vici_add_key_valuef(req, "key1", "value%u", 1);
	vici_add_key_value(req, "key2", "value2", strlen("value2"));
	vici_end_section(req);
}

static void decode_section(vici_res_t *res)
{
	char *str;
	int len;

	ck_assert(vici_parse(res) == VICI_PARSE_BEGIN_SECTION);
	ck_assert_str_eq(vici_parse_name(res), "section1");
	ck_assert(vici_parse(res) == VICI_PARSE_KEY_VALUE);
	ck_assert_str_eq(vici_parse_name(res), "key1");
	ck_assert_str_eq(vici_parse_value_str(res), "value1");
	ck_assert(vici_parse(res) == VICI_PARSE_KEY_VALUE);
	ck_assert_str_eq(vici_parse_name(res), "key2");
	str = vici_parse_value(res, &len);
	ck_assert(chunk_equals(chunk_from_str("value2"), chunk_create(str, len)));
	ck_assert(vici_parse(res) == VICI_PARSE_END_SECTION);
	ck_assert(vici_parse(res) == VICI_PARSE_END);
}

static void encode_list(vici_req_t *req)
{
	vici_begin_list(req, "list1");
	vici_add_list_item(req, "item1", strlen("item1"));
	vici_add_list_itemf(req, "item%u", 2);
	vici_end_list(req);
}

static void decode_list(vici_res_t *res)
{
	char *str;
	int len;

	ck_assert(vici_parse(res) == VICI_PARSE_BEGIN_LIST);
	ck_assert_str_eq(vici_parse_name(res), "list1");
	ck_assert(vici_parse(res) == VICI_PARSE_LIST_ITEM);
	ck_assert_str_eq(vici_parse_value_str(res), "item1");
	ck_assert(vici_parse(res) == VICI_PARSE_LIST_ITEM);
	str = vici_parse_value(res, &len);
	ck_assert(chunk_equals(chunk_from_str("item2"), chunk_create(str, len)));
	ck_assert(vici_parse(res) == VICI_PARSE_END_LIST);
	ck_assert(vici_parse(res) == VICI_PARSE_END);
}

static struct {
	void (*encode)(vici_req_t* req);
	void (*decode)(vici_res_t* res);
} echo_tests[] = {
	{ encode_section, decode_section },
	{ encode_list, decode_list },
};

static vici_message_t* echo_cb(void *user, char *name,
							   u_int id, vici_message_t *request)
{
	ck_assert_str_eq(name, "echo");
	ck_assert_int_eq((uintptr_t)user, 1);

	return vici_message_create_from_enumerator(request->create_enumerator(request));
}

START_TEST(test_echo)
{
	vici_dispatcher_t *dispatcher;
	vici_conn_t *conn;
	vici_req_t *req;
	vici_res_t *res;

	lib->processor->set_threads(lib->processor, 8);

	dispatcher = vici_dispatcher_create(URI);
	ck_assert(dispatcher);

	dispatcher->manage_command(dispatcher, "echo", echo_cb, (void*)(uintptr_t)1);

	vici_init();
	conn = vici_connect(URI);
	ck_assert(conn);

	req = vici_begin("echo");
	echo_tests[_i].encode(req);
	res = vici_submit(req, conn);
	ck_assert(res);
	echo_tests[_i].decode(res);
	vici_free_res(res);

	vici_disconnect(conn);

	dispatcher->manage_command(dispatcher, "echo", NULL, NULL);

	lib->processor->cancel(lib->processor);
	dispatcher->destroy(dispatcher);

	vici_deinit();
}
END_TEST

START_TEST(test_missing)
{
	vici_dispatcher_t *dispatcher;
	vici_conn_t *conn;
	vici_req_t *req;
	vici_res_t *res;

	lib->processor->set_threads(lib->processor, 8);

	dispatcher = vici_dispatcher_create(URI);
	ck_assert(dispatcher);

	vici_init();
	conn = vici_connect(URI);
	ck_assert(conn);

	req = vici_begin("nonexistent");
	encode_section(req);
	res = vici_submit(req, conn);
	ck_assert(res == NULL);

	vici_disconnect(conn);

	dispatcher->manage_command(dispatcher, "echo", NULL, NULL);

	lib->processor->cancel(lib->processor);
	dispatcher->destroy(dispatcher);

	vici_deinit();
}
END_TEST

static void event_cb(void *user, char *name, vici_res_t *ev)
{
	int *events = (int*)user;

	(*events)++;
}

START_TEST(test_stress)
{
	vici_dispatcher_t *dispatcher;
	vici_conn_t *conn;
	vici_req_t *req;
	vici_res_t *res;
	int i, total = 50, events = 0;

	lib->processor->set_threads(lib->processor, 8);

	dispatcher = vici_dispatcher_create(URI);
	ck_assert(dispatcher);

	dispatcher->manage_command(dispatcher, "echo", echo_cb, (void*)(uintptr_t)1);
	dispatcher->manage_event(dispatcher, "dummy", TRUE);

	vici_init();
	conn = vici_connect(URI);
	ck_assert(conn);

	for (i = 0; i < total; i++)
	{
		/* do some event management in between */
		ck_assert(vici_register(conn, "dummy", event_cb, &events) == 0);
		dispatcher->raise_event(dispatcher, "dummy", 0,
			vici_message_create_from_args(
				 VICI_KEY_VALUE, "key1", chunk_from_str("value1"),
				VICI_END));

		req = vici_begin("echo");
		encode_section(req);
		res = vici_submit(req, conn);
		ck_assert(res);
		decode_section(res);
		vici_free_res(res);

		ck_assert(vici_register(conn, "dummy", NULL, NULL) == 0);
	}

	while (events < total)
	{
		usleep(1000);
	}

	vici_disconnect(conn);

	dispatcher->manage_command(dispatcher, "echo", NULL, NULL);
	dispatcher->manage_event(dispatcher, "dummy", FALSE);

	lib->processor->cancel(lib->processor);
	dispatcher->destroy(dispatcher);

	vici_deinit();
}
END_TEST

Suite *request_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("vici request");

	tc = tcase_create("echo");
	tcase_add_loop_test(tc, test_echo, 0, countof(echo_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("missing");
	tcase_add_test(tc, test_missing);
	suite_add_tcase(s, tc);

	tc = tcase_create("stress");
	tcase_add_test(tc, test_stress);
	suite_add_tcase(s, tc);

	return s;
}
