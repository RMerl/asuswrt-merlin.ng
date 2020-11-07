/*
 * tests/check-attr.c		nla_attr unit tests
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Thomas Graf <tgraf@suug.ch>
 */

#include "util.h"
#include <netlink/attr.h>
#include <netlink/msg.h>

START_TEST(attr_size)
{
	fail_if(nla_attr_size(0) != NLA_HDRLEN,
		"Length of empty attribute should match header size");
	fail_if(nla_attr_size(1) != NLA_HDRLEN + 1,
	        "Length of 1 bytes payload should be NLA_HDRLEN + 1");
	fail_if(nla_attr_size(2) != NLA_HDRLEN + 2,
	        "Length of 2 bytes payload should be NLA_HDRLEN + 2");
	fail_if(nla_attr_size(3) != NLA_HDRLEN + 3,
	        "Length of 3 bytes payload should be NLA_HDRLEN + 3");
	fail_if(nla_attr_size(4) != NLA_HDRLEN + 4,
	        "Length of 4 bytes payload should be NLA_HDRLEN + 4");

	fail_if(nla_total_size(1) != NLA_HDRLEN + 4,
		"Total size of 1 bytes payload should result in 8 bytes");
	fail_if(nla_total_size(2) != NLA_HDRLEN + 4,
		"Total size of 2 bytes payload should result in 8 bytes");
	fail_if(nla_total_size(3) != NLA_HDRLEN + 4,
		"Total size of 3 bytes payload should result in 8 bytes");
	fail_if(nla_total_size(4) != NLA_HDRLEN + 4,
		"Total size of 4 bytes payload should result in 8 bytes");

	fail_if(nla_padlen(1) != 3,
		"2 bytes of payload should result in 3 padding bytes");
	fail_if(nla_padlen(2) != 2,
		"2 bytes of payload should result in 2 padding bytes");
	fail_if(nla_padlen(3) != 1,
		"3 bytes of payload should result in 1 padding bytes");
	fail_if(nla_padlen(4) != 0,
		"4 bytes of payload should result in 0 padding bytes");
	fail_if(nla_padlen(5) != 3,
		"5 bytes of payload should result in 3 padding bytes");
}
END_TEST

START_TEST(msg_construct)
{
	struct nl_msg *msg;
	struct nlmsghdr *nlh;
	struct nlattr *a;
	int i, rem;

	msg = nlmsg_alloc();
	fail_if(!msg, "Unable to allocate netlink message");

	for (i = 1; i < 256; i++) {
		fail_if(nla_put_u32(msg, i, i+1) != 0,
			"Unable to add attribute %d", i);
	}

	nlh = nlmsg_hdr(msg);
	i = 1;
	nlmsg_for_each_attr(a, nlh, 0, rem) {
		fail_if(nla_type(a) != i, "Expected attribute %d", i);
		i++;
		fail_if(nla_get_u32(a) != i, "Expected attribute value %d", i);
	}

	nlmsg_free(msg);
}
END_TEST

Suite *make_nl_attr_suite(void)
{
	Suite *suite = suite_create("Netlink attributes");

	TCase *nl_attr = tcase_create("Core");
	tcase_add_test(nl_attr, attr_size);
	tcase_add_test(nl_attr, msg_construct);
	suite_add_tcase(suite, nl_attr);

	return suite;
}
