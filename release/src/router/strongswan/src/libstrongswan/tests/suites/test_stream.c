/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "test_suite.h"

#include <unistd.h>

static char* services[] = {
#ifndef WIN32
	"unix:///tmp/strongswan-test-service.sck",
#endif
	"tcp://127.0.0.1:7766",
	"tcp://[::1]:7766",
};

static char msg[] = "testmessage";
static int msglen = 12;

static bool servicing(void *data, stream_t *stream)
{
	char buf[64];
	ssize_t len, total;

	ck_assert(streq((char*)data, "test"));

	for (total = 0; total < msglen;)
	{
		len = stream->read(stream, buf, sizeof(buf), TRUE);
		ck_assert(len > 0);
		total += len;
	}
	for (total = 0; total < msglen;)
	{
		len = stream->write(stream, buf, len, TRUE);
		ck_assert(len > 0);
		total += len;
	}

	return FALSE;
}

START_TEST(test_sync)
{
	char buf[64];
	stream_service_t *service;
	stream_t *stream;
	ssize_t len, total;

	lib->processor->set_threads(lib->processor, 8);

	service = lib->streams->create_service(lib->streams, services[_i], 1);
	ck_assert(service != NULL);
	service->on_accept(service, servicing, "test", JOB_PRIO_HIGH, 1);

	stream = lib->streams->connect(lib->streams, services[_i]);
	ck_assert(stream != NULL);
	for (total = 0; total < msglen;)
	{
		len = stream->write(stream, msg, msglen, TRUE);
		ck_assert(len > 0);
		total += len;
	}
	for (total = 0; total < msglen;)
	{
		len = stream->read(stream, buf, sizeof(buf), TRUE);
		ck_assert(len > 0);
		total += len;
	}
	ck_assert(streq(buf, msg));
	stream->destroy(stream);

	service->destroy(service);
}
END_TEST

static bool on_write(void *data, stream_t *stream)
{
	ssize_t len, total;

	ck_assert(streq((char*)data, "test-write"));
	for (total = 0; total < msglen;)
	{
		len = stream->write(stream, msg, msglen, TRUE);
		ck_assert(len > 0);
		total += len;
	}
	return FALSE;
}

static bool read_done = FALSE;

static bool on_read(void *data, stream_t *stream)
{
	ssize_t len, total;
	char buf[64];

	ck_assert(streq((char*)data, "test-read"));
	for (total = 0; total < msglen;)
	{
		len = stream->read(stream, buf, sizeof(buf), TRUE);
		ck_assert(len > 0);
		total += len;
	}
	ck_assert(streq(buf, msg));
	read_done = TRUE;
	return FALSE;
}

START_TEST(test_async)
{
	stream_service_t *service;
	stream_t *stream;

	lib->processor->set_threads(lib->processor, 8);

	service = lib->streams->create_service(lib->streams, services[_i], 1);
	ck_assert(service != NULL);
	service->on_accept(service, servicing, "test", JOB_PRIO_HIGH, 0);

	stream = lib->streams->connect(lib->streams, services[_i]);
	ck_assert(stream != NULL);
	read_done = FALSE;
	stream->on_write(stream, (stream_cb_t)on_write, "test-write");
	stream->on_read(stream, (stream_cb_t)on_read, "test-read");

	while (!read_done)
	{
		usleep(1000);
	}
	stream->destroy(stream);

	service->destroy(service);
}
END_TEST

static bool all(void *data, stream_t *stream)
{
	char buf[64], *pos;
	ssize_t len;
	int i;

	pos = buf;
	for (i = 0; i < msglen; i++)
	{
		len = stream->read(stream, pos, 1, TRUE);
		ck_assert_int_eq(len, 1);
		pos += len;
	}
	pos = buf;
	for (i = 0; i < msglen; i++)
	{
		len = stream->write(stream, pos, 1, TRUE);
		ck_assert_int_eq(len, 1);
		pos += len;
	}

	return FALSE;
}

START_TEST(test_all)
{
	char buf[64];
	stream_service_t *service;
	stream_t *stream;

	lib->processor->set_threads(lib->processor, 8);

	service = lib->streams->create_service(lib->streams, services[_i], 1);
	ck_assert(service != NULL);
	service->on_accept(service, all, NULL, JOB_PRIO_HIGH, 1);

	stream = lib->streams->connect(lib->streams, services[_i]);
	ck_assert(stream != NULL);
	ck_assert(stream->write_all(stream, msg, msglen));
	ck_assert(stream->read_all(stream, buf, msglen));
	ck_assert(streq(buf, msg));
	stream->destroy(stream);

	service->destroy(service);
}
END_TEST

static bool concurrency(void *data, stream_t *stream)
{
	static refcount_t refs = 0;
	u_int current;
	ssize_t len;

	current = ref_get(&refs);
	ck_assert(current <= 3);
	len = stream->write(stream, "x", 1, TRUE);
	ck_assert_int_eq(len, 1);
	usleep(1000);
	ignore_result(ref_put(&refs));

	return FALSE;
}

START_TEST(test_concurrency)
{
	stream_service_t *service;
	stream_t *streams[10];
	ssize_t len;
	char x;
	int i;

	lib->processor->set_threads(lib->processor, 8);

	service = lib->streams->create_service(lib->streams, services[_i], 10);
	ck_assert(service != NULL);
	service->on_accept(service, concurrency, NULL, JOB_PRIO_HIGH, 3);

	for (i = 0; i < countof(streams); i++)
	{
		streams[i] = lib->streams->connect(lib->streams, services[_i]);
		ck_assert(streams[i] != NULL);
	}
	for (i = 0; i < countof(streams); i++)
	{
		len = streams[i]->read(streams[i], &x, 1, TRUE);
		ck_assert_int_eq(len, 1);
		ck_assert_int_eq(x, 'x');
	}
	for (i = 0; i < countof(streams); i++)
	{
		streams[i]->destroy(streams[i]);
	}
	service->destroy(service);
}
END_TEST

Suite *stream_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("stream");

	tc = tcase_create("sync");
	tcase_add_loop_test(tc, test_sync, 0, countof(services));
	suite_add_tcase(s, tc);

	tc = tcase_create("async");
	tcase_add_loop_test(tc, test_async, 0, countof(services));
	suite_add_tcase(s, tc);

	tc = tcase_create("all");
	tcase_add_loop_test(tc, test_all, 0, countof(services));
	suite_add_tcase(s, tc);

	tc = tcase_create("concurrency");
	tcase_add_loop_test(tc, test_concurrency, 0, countof(services));
	suite_add_tcase(s, tc);

	return s;
}
