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

#include "../vici_socket.h"

#include <unistd.h>

typedef struct {
	vici_socket_t *s;
	int disconnect;
	int bytes;
	u_int id;
} test_data_t;

static void echo_inbound(void *user, u_int id, chunk_t buf)
{
	test_data_t *data = user;

	ck_assert_int_eq(data->id, id);
	/* count number of bytes, including the header */
	data->bytes += buf.len + sizeof(uint32_t);
	/* echo back data chunk */
	data->s->send(data->s, id, chunk_clone(buf));
}

static void echo_connect(void *user, u_int id)
{
	test_data_t *data = user;

	data->id = id;
}

static void echo_disconnect(void *user, u_int id)
{
	test_data_t *data = user;

	ck_assert(id == data->id);
	data->disconnect++;
}

static struct {
	char *uri;
	u_int chunksize;
} echo_tests[] = {
	{ "tcp://127.0.0.1:6543", ~0 },
	{ "tcp://127.0.0.1:6543",  1 },
	{ "tcp://127.0.0.1:6543",  2 },
	{ "tcp://127.0.0.1:6543",  3 },
	{ "tcp://127.0.0.1:6543",  7 },
#ifndef WIN32
	{ "unix:///tmp/strongswan-tests-vici-socket", ~0 },
	{ "unix:///tmp/strongswan-tests-vici-socket",  1 },
	{ "unix:///tmp/strongswan-tests-vici-socket",  2 },
	{ "unix:///tmp/strongswan-tests-vici-socket",  3 },
	{ "unix:///tmp/strongswan-tests-vici-socket",  7 },
#endif /* !WIN32 */
};

START_TEST(test_echo)
{
	stream_t *c;
	test_data_t data = {};
	chunk_t x, m = chunk_from_chars(
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x01,	0x01,
		0x00,0x00,0x00,0x05,	0x11,0x12,0x13,0x14,0x15,
		0x00,0x00,0x00,0x0A,	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x02A,
	);
	char buf[m.len];
	uint32_t len;

	lib->processor->set_threads(lib->processor, 4);

	/* create socket, connect with stream */
	data.s = vici_socket_create(echo_tests[_i].uri, echo_inbound, echo_connect,
								echo_disconnect, &data);
	ck_assert(data.s != NULL);
	c = lib->streams->connect(lib->streams, echo_tests[_i].uri);
	ck_assert(c != NULL);

	/* write arbitrary chunks of messages blob depending on test */
	x = m;
	while (x.len)
	{
		len = min(x.len, echo_tests[_i].chunksize);
		ck_assert(c->write_all(c, x.ptr, len));
		x = chunk_skip(x, len);
	}

	/* verify echo */
	ck_assert(c->read_all(c, buf, sizeof(buf)));
	ck_assert(chunk_equals(m, chunk_from_thing(buf)));

	/* wait for completion */
	c->destroy(c);
	while (data.disconnect != 1)
	{
		usleep(1000);
	}
	/* check that we got correct number of bytes/invocations */
	ck_assert_int_eq(data.bytes, m.len);

	data.s->destroy(data.s);
}
END_TEST

Suite *socket_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("vici socket");

	tc = tcase_create("echo");
	tcase_add_loop_test(tc, test_echo, 0, countof(echo_tests));
	suite_add_tcase(s, tc);

	return s;
}
