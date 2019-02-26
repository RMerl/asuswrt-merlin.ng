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

#include <library.h>

#include <unistd.h>
#include <errno.h>

static char testbuf[1] = "";

static bool readcb(void *data, int fd, watcher_event_t event)
{
	ck_assert_int_eq(*(int*)data, fd);
	ck_assert_int_eq(event, WATCHER_READ);

	if (recv(fd, testbuf, 1, MSG_DONTWAIT) != 1)
	{
		ck_assert(errno == EAGAIN || errno == EWOULDBLOCK);
	}
	return TRUE;
}

START_TEST(test_read)
{
	int fd[2];
	char c;

	lib->processor->set_threads(lib->processor, 8);

	ck_assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) != -1);

	lib->watcher->add(lib->watcher, fd[0], WATCHER_READ, readcb, &fd[0]);

	for (c = 'a'; c <= 'z'; c++)
	{
		ck_assert_int_eq(send(fd[1], &c, 1, 0), 1);
		while (testbuf[0] != c)
		{
			sched_yield();
		}
	}

	lib->watcher->remove(lib->watcher, fd[0]);
	close(fd[0]);
	close(fd[1]);

	lib->processor->cancel(lib->processor);
}
END_TEST

static bool writecb(void *data, int fd, watcher_event_t event)
{
	ck_assert_int_eq(event, WATCHER_WRITE);
	if (send(fd, data, 1, MSG_DONTWAIT) != 1)
	{
		ck_assert(errno == EAGAIN || errno == EWOULDBLOCK);
	}
	return TRUE;
}

START_TEST(test_write)
{
	int fd[2];
	char in = 'x', out;

	lib->processor->set_threads(lib->processor, 8);

	ck_assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) != -1);

	lib->watcher->add(lib->watcher, fd[1], WATCHER_WRITE, writecb, &in);

	ck_assert_int_eq(recv(fd[0], &out, 1, 0), 1);
	ck_assert_int_eq(out, in);

	lib->watcher->remove(lib->watcher, fd[1]);
	close(fd[1]);
	close(fd[0]);

	lib->processor->cancel(lib->processor);
}
END_TEST

static bool multiread(void *data, int fd, watcher_event_t event)
{
	ck_assert_int_eq(event, WATCHER_READ);
	if (recv(fd, data, 1, MSG_DONTWAIT) != 1)
	{
		ck_assert(errno == EAGAIN || errno == EWOULDBLOCK);
	}
	return TRUE;
}

START_TEST(test_multiread)
{
	int fd[10][2], i;
	char in, out[countof(fd)];

	lib->processor->set_threads(lib->processor, 8);

	for (i = 0; i < countof(fd); i++)
	{
		ck_assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fd[i]) != -1);
		lib->watcher->add(lib->watcher, fd[i][0],
						  WATCHER_READ, multiread, &out[i]);
	}

	for (i = 0; i < countof(fd); i++)
	{
		for (in = 'a'; in <= 'z'; in++)
		{
			ck_assert_int_eq(send(fd[i][1], &in, 1, 0), 1);
			while (out[i] != in)
			{
				sched_yield();
			}
		}
	}

	for (i = 0; i < countof(fd); i++)
	{
		lib->watcher->remove(lib->watcher, fd[i][0]);
		close(fd[i][1]);
		close(fd[i][0]);
	}

	lib->processor->cancel(lib->processor);
}
END_TEST

static bool multiwrite(void *data, int fd, watcher_event_t event)
{
	ck_assert_int_eq(event, WATCHER_WRITE);
	if (send(fd, data, 1, MSG_DONTWAIT) != 1)
	{
		ck_assert(errno == EAGAIN || errno == EWOULDBLOCK);
	}
	return TRUE;
}

START_TEST(test_multiwrite)
{
	int fd[10][2], i, j;
	u_char out, in[countof(fd)];

	lib->processor->set_threads(lib->processor, 8);

	for (i = 0; i < countof(fd); i++)
	{
		ck_assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fd[i]) != -1);
		in[i] = i;
		lib->watcher->add(lib->watcher, fd[i][1],
						  WATCHER_WRITE, multiwrite, &in[i]);
	}

	for (j = 0; j < 10; j++)
	{
		for (i = 0; i < countof(fd); i++)
		{
			ck_assert_int_eq(recv(fd[i][0], &out, 1, 0), 1);
			ck_assert_int_eq(out, i);
		}
	}

	for (i = 0; i < countof(fd); i++)
	{
		lib->watcher->remove(lib->watcher, fd[i][1]);
		close(fd[i][1]);
		close(fd[i][0]);
	}

	lib->processor->cancel(lib->processor);
}
END_TEST

Suite *watcher_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("watcher");

	tc = tcase_create("read");
	tcase_add_test(tc, test_read);
	suite_add_tcase(s, tc);

	tc = tcase_create("write");
	tcase_add_test(tc, test_write);
	suite_add_tcase(s, tc);

	tc = tcase_create("multiread");
	tcase_add_test(tc, test_multiread);
	suite_add_tcase(s, tc);

	tc = tcase_create("multiwrite");
	tcase_add_test(tc, test_multiwrite);
	suite_add_tcase(s, tc);

	return s;
}
