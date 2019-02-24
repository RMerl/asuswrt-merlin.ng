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

#include <threading/thread.h>

#include "../kernel_netlink_shared.h"

/**
 * Netlink message drop configuration
 */
static int drop_interval = 0;

/**
 * Netlink message drop hook
 */
bool netlink_msg_loss(struct nlmsghdr *hdr)
{
	static refcount_t i;

	if (drop_interval)
	{
		return ref_get(&i) % drop_interval == drop_interval - 1;
	}
	return FALSE;
}

START_TEST(test_echo)
{
	netlink_socket_t *s;
	struct nlmsghdr *out;
	struct rtmsg *msg;
	char dst[] = {
		127,0,0,1
	};
	size_t len;
	netlink_buf_t request = {
		.hdr = {
			.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)),
			.nlmsg_flags = NLM_F_REQUEST,
			.nlmsg_type = RTM_GETROUTE,
		},
	};

	msg = NLMSG_DATA(&request.hdr);
	msg->rtm_family = AF_INET;
	netlink_add_attribute(&request.hdr, RTA_DST,
						  chunk_from_thing(dst), sizeof(request));

	s = netlink_socket_create(NETLINK_ROUTE, NULL, _i != 0);

	ck_assert(s->send(s, &request.hdr, &out, &len) == SUCCESS);
	ck_assert_int_eq(out->nlmsg_type, RTM_NEWROUTE);
	free(out);
	s->destroy(s);
}
END_TEST

START_TEST(test_echo_dump)
{
	netlink_socket_t *s;
	struct nlmsghdr *out, *current;
	struct rtgenmsg *msg;
	size_t len;
	netlink_buf_t request = {
		.hdr = {
			.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg)),
			.nlmsg_flags = NLM_F_REQUEST | NLM_F_MATCH | NLM_F_ROOT,
			.nlmsg_type = RTM_GETLINK,
		},
	};

	s = netlink_socket_create(NETLINK_ROUTE, NULL, _i != 0);
	msg = NLMSG_DATA(&request.hdr);
	msg->rtgen_family = AF_UNSPEC;

	ck_assert(s->send(s, &request.hdr, &out, &len) == SUCCESS);
	current = out;
	while (TRUE)
	{
		ck_assert(NLMSG_OK(current, len));
		if (current->nlmsg_type == NLMSG_DONE)
		{
			break;
		}
		ck_assert_int_eq(current->nlmsg_type, RTM_NEWLINK);
		current = NLMSG_NEXT(current, len);
	}
	free(out);
	s->destroy(s);
}
END_TEST

CALLBACK(stress, void*,
	netlink_socket_t *s)
{
	struct nlmsghdr *out;
	struct rtmsg *msg;
	char dst[] = {
		127,0,0,1
	};
	size_t len;
	int i;
	netlink_buf_t request = {
		.hdr = {
			.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)),
			.nlmsg_flags = NLM_F_REQUEST,
			.nlmsg_type = RTM_GETROUTE,
		},
	};

	for (i = 0; i < 10; i++)
	{
		msg = NLMSG_DATA(&request.hdr);
		msg->rtm_family = AF_INET;
		netlink_add_attribute(&request.hdr, RTA_DST,
							  chunk_from_thing(dst), sizeof(request));

		ck_assert(s->send(s, &request.hdr, &out, &len) == SUCCESS);
		ck_assert_int_eq(out->nlmsg_type, RTM_NEWROUTE);
		free(out);
	}
	return NULL;
}

CALLBACK(stress_dump, void*,
	netlink_socket_t *s)
{
	struct nlmsghdr *out, *current;
	struct rtgenmsg *msg;
	size_t len;
	int i;
	netlink_buf_t request = {
		.hdr = {
			.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg)),
			.nlmsg_flags = NLM_F_REQUEST | NLM_F_MATCH | NLM_F_ROOT,
			.nlmsg_type = RTM_GETLINK,
		},
	};

	msg = NLMSG_DATA(&request.hdr);
	msg->rtgen_family = AF_UNSPEC;

	for (i = 0; i < 10; i++)
	{
		ck_assert(s->send(s, &request.hdr, &out, &len) == SUCCESS);
		current = out;
		while (TRUE)
		{
			ck_assert(NLMSG_OK(current, len));
			if (current->nlmsg_type == NLMSG_DONE)
			{
				break;
			}
			ck_assert_int_eq(current->nlmsg_type, RTM_NEWLINK);
			current = NLMSG_NEXT(current, len);
		}
		free(out);
	}
	return NULL;
}

START_TEST(test_stress)
{
	thread_t *threads[10];
	netlink_socket_t *s;
	int i;

	s = netlink_socket_create(NETLINK_ROUTE, NULL, _i != 0);
	for (i = 0; i < countof(threads); i++)
	{
		threads[i] = thread_create(stress, s);
	}
	for (i = 0; i < countof(threads); i++)
	{
		threads[i]->join(threads[i]);
	}
	s->destroy(s);
}
END_TEST

START_TEST(test_stress_dump)
{
	thread_t *threads[10];
	netlink_socket_t *s;
	int i;

	s = netlink_socket_create(NETLINK_ROUTE, NULL, _i != 0);
	for (i = 0; i < countof(threads); i++)
	{
		threads[i] = thread_create(stress_dump, s);
	}
	for (i = 0; i < countof(threads); i++)
	{
		threads[i]->join(threads[i]);
	}
	s->destroy(s);
}
END_TEST

START_TEST(test_retransmit_success)
{
	netlink_socket_t *s;
	struct nlmsghdr *out;
	struct rtgenmsg *msg;
	size_t len;
	netlink_buf_t request = {
		.hdr = {
			.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg)),
			.nlmsg_flags = NLM_F_REQUEST | NLM_F_MATCH | NLM_F_ROOT,
			.nlmsg_type = RTM_GETLINK,
		},
	};

	drop_interval = 2;

	lib->settings->set_int(lib->settings,
							"%s.plugins.kernel-netlink.timeout", 100, lib->ns);
	lib->settings->set_int(lib->settings,
							"%s.plugins.kernel-netlink.retries", 1, lib->ns);

	s = netlink_socket_create(NETLINK_ROUTE, NULL, _i != 0);
	msg = NLMSG_DATA(&request.hdr);
	msg->rtgen_family = AF_UNSPEC;

	ck_assert(s->send(s, &request.hdr, &out, &len) == SUCCESS);
	free(out);
	s->destroy(s);

	drop_interval = 0;
}
END_TEST

START_TEST(test_retransmit_fail)
{
	netlink_socket_t *s;
	struct nlmsghdr *out;
	struct rtgenmsg *msg;
	size_t len;
	netlink_buf_t request = {
		.hdr = {
			.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg)),
			.nlmsg_flags = NLM_F_REQUEST | NLM_F_MATCH | NLM_F_ROOT,
			.nlmsg_type = RTM_GETLINK,
		},
	};

	drop_interval = 1;

	lib->settings->set_int(lib->settings,
							"%s.plugins.kernel-netlink.timeout", 50, lib->ns);
	lib->settings->set_int(lib->settings,
							"%s.plugins.kernel-netlink.retries", 3, lib->ns);

	s = netlink_socket_create(NETLINK_ROUTE, NULL, _i != 0);
	msg = NLMSG_DATA(&request.hdr);
	msg->rtgen_family = AF_UNSPEC;

	ck_assert(s->send(s, &request.hdr, &out, &len) == OUT_OF_RES);
	s->destroy(s);

	drop_interval = 0;
}
END_TEST

Suite *socket_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("netlink socket");

	tc = tcase_create("echo");
	tcase_add_loop_test(tc, test_echo, 0, 2);
	tcase_add_loop_test(tc, test_echo_dump, 0, 2);
	suite_add_tcase(s, tc);

	tc = tcase_create("stress");
	tcase_add_loop_test(tc, test_stress, 0, 2);
	tcase_add_loop_test(tc, test_stress_dump, 0, 2);
	suite_add_tcase(s, tc);

	tc = tcase_create("retransmit");
	tcase_add_loop_test(tc, test_retransmit_success, 0, 2);
	tcase_add_loop_test(tc, test_retransmit_fail, 0, 2);
	suite_add_tcase(s, tc);

	return s;
}
