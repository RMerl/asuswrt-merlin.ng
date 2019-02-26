/*
 * Copyright (C) 2015 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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

#include <selectors/traffic_selector.h>


static void verify(const char *str, const char *alt, traffic_selector_t *ts)
{
	char buf[512];

	if (!str)
	{
		ck_assert_msg(!ts, "traffic selector not null: %R", ts);
		return;
	}
	snprintf(buf, sizeof(buf), "%R", ts);
	DESTROY_IF(ts);
	if (!streq(buf, str) && (!alt || !streq(buf, alt)))
	{
		fail("%s != %s or %s", buf, str, alt);
	}
}

START_TEST(test_create_from_string)
{
	verify("10.1.0.0/16[tcp/http]", "10.1.0.0/16[6/80]",
		traffic_selector_create_from_string(IPPROTO_TCP, TS_IPV4_ADDR_RANGE,
							"10.1.0.0", 80, "10.1.255.255", 80));
	verify("10.1.0.1..10.1.0.99[udp/1234-1235]",
		   "10.1.0.1..10.1.0.99[17/1234-1235]",
		traffic_selector_create_from_string(IPPROTO_UDP, TS_IPV4_ADDR_RANGE,
							"10.1.0.1", 1234, "10.1.0.99", 1235));
	verify("fec1::/64", NULL,
		traffic_selector_create_from_string(0, TS_IPV6_ADDR_RANGE,
							"fec1::", 0, "fec1::ffff:ffff:ffff:ffff", 65535));
	verify("fec1::1..fec1::ffff:ffff:ffff:ffff", NULL,
		traffic_selector_create_from_string(0, TS_IPV6_ADDR_RANGE,
							"fec1::1", 0, "fec1::ffff:ffff:ffff:ffff", 65535));
	verify(NULL, NULL,
		traffic_selector_create_from_string(IPPROTO_TCP, 0,
							"10.1.0.0", 80, "10.1.255.255", 80));
	verify(NULL, NULL,
		traffic_selector_create_from_string(IPPROTO_TCP, TS_IPV4_ADDR_RANGE,
							"a.b.c.d", 80, "10.1.255.255", 80));
	verify(NULL, NULL,
		traffic_selector_create_from_string(IPPROTO_TCP, TS_IPV4_ADDR_RANGE,
							"10.1.0.0", 80, "a.b.c.d", 80));
}
END_TEST

START_TEST(test_create_from_cidr)
{
	verify("10.1.0.0/16", NULL,
		traffic_selector_create_from_cidr("10.1.0.0/16", 0, 0, 65535));
	verify("10.1.0.1/32[udp]", "10.1.0.1/32[17]",
		traffic_selector_create_from_cidr("10.1.0.1/32", IPPROTO_UDP,
										  0, 65535));
	verify("10.1.0.1/32[0/domain]", "10.1.0.1/32[0/53]",
		traffic_selector_create_from_cidr("10.1.0.1/32", 0,
										  53, 53));
	verify("10.1.0.1/32[udp/1234-1235]", "10.1.0.1/32[17/1234-1235]",
		traffic_selector_create_from_cidr("10.1.0.1/32", IPPROTO_UDP,
										  1234, 1235));
	verify("10.1.0.0/16[0/OPAQUE]", NULL,
		traffic_selector_create_from_cidr("10.1.0.0/16", 0, 65535, 0));

	verify(NULL, NULL,
		traffic_selector_create_from_cidr("a.b.c.d/16", 0, 0, 65535));
}
END_TEST

START_TEST(test_create_from_bytes)
{
	verify("10.1.0.0/16", NULL,
		traffic_selector_create_from_bytes(0, TS_IPV4_ADDR_RANGE,
			chunk_from_chars(0x0a,0x01,0x00,0x00), 0,
			chunk_from_chars(0x0a,0x01,0xff,0xff), 65535));
	verify(NULL, NULL,
		traffic_selector_create_from_bytes(0, TS_IPV4_ADDR_RANGE,
			chunk_from_chars(0x0a,0x01,0x00,0x00), 0,
			chunk_from_chars(0x0a,0x01,0xff,0xff,0xff), 65535));
	verify(NULL, NULL,
		traffic_selector_create_from_bytes(0, TS_IPV4_ADDR_RANGE,
			chunk_empty, 0,
			chunk_empty, 65535));
	verify(NULL, NULL,
		traffic_selector_create_from_bytes(0, TS_IPV6_ADDR_RANGE,
			chunk_from_chars(0x0a,0x01,0x00,0x00), 0,
			chunk_from_chars(0x0a,0x01,0xff,0xff), 65535));
	verify(NULL, NULL,
		traffic_selector_create_from_bytes(0, 0,
			chunk_from_chars(0x0a,0x01,0x00,0x00), 0,
			chunk_from_chars(0x0a,0x01,0xff,0xff), 65535));
}
END_TEST

START_TEST(test_create_from_subnet)
{
	verify("10.1.0.0/16", NULL,
		traffic_selector_create_from_subnet(
					host_create_from_string("10.1.0.0", 0), 16, 0, 0, 65535));
}
END_TEST

struct {
	char *net;
	ts_type_t type;
	chunk_t enc;
} rfc3779_prefix_tests[] = {
	/* some examples from RFC 3779, for addressPrefix elements we pass the same
	 * value twice to the constructor */
	{ "10.0.0.0/8",		TS_IPV4_ADDR_RANGE,	chunk_from_chars(0x00,0x0a),				},
	{ "10.0.32.0/20",	TS_IPV4_ADDR_RANGE,	chunk_from_chars(0x04,0x0a,0x00,0x20),		},
	{ "10.0.64.0/24",	TS_IPV4_ADDR_RANGE,	chunk_from_chars(0x00,0x0a,0x00,0x40),		},
	{ "10.1.0.0/16",	TS_IPV4_ADDR_RANGE,	chunk_from_chars(0x00,0x0a,0x01),			},
	{ "10.5.0.1/32",	TS_IPV4_ADDR_RANGE,	chunk_from_chars(0x00,0x0a,0x05,0x00,0x01),	},
	{ "10.5.0.0/23",	TS_IPV4_ADDR_RANGE,	chunk_from_chars(0x01,0x0a,0x05,0x00),		},
	{ "10.64.0.0/12",	TS_IPV4_ADDR_RANGE,	chunk_from_chars(0x04,0x0a,0x40),			},
	{ "10.64.0.0/20",	TS_IPV4_ADDR_RANGE,	chunk_from_chars(0x04,0x0a,0x40,0x00),		},
	{ "128.0.0.0/4",	TS_IPV4_ADDR_RANGE,	chunk_from_chars(0x04,0x80),				},
	{ "172.16.0.0/12",	TS_IPV4_ADDR_RANGE,	chunk_from_chars(0x04,0xac,0x10),			},
	{ "0.0.0.0/0",		TS_IPV4_ADDR_RANGE,	chunk_from_chars(0x00),						},
	{ NULL,				0,					chunk_from_chars(0x00),						},
	/* FIXME: not a correct encoding, so we might want to fail here */
	{ "0.0.0.0/0",		TS_IPV4_ADDR_RANGE,	{NULL, 0},									},
	{ "2001:0:2::/48",	TS_IPV6_ADDR_RANGE,	chunk_from_chars(0x00,0x20,0x01,0x00,0x00,0x00,0x02),},
	{ "2001:0:200::/39",TS_IPV6_ADDR_RANGE,	chunk_from_chars(0x01,0x20,0x01,0x00,0x00,0x02),},
	{ "::/0",			TS_IPV6_ADDR_RANGE,	chunk_from_chars(0x00),						},
	/* FIXME: not a correct encoding, so we might want to fail here */
	{ "::/0",			TS_IPV6_ADDR_RANGE,	{NULL, 0},									},
};

START_TEST(test_create_from_rfc3779_format_prefix)
{
	verify(rfc3779_prefix_tests[_i].net, NULL,
		traffic_selector_create_from_rfc3779_format(rfc3779_prefix_tests[_i].type,
					rfc3779_prefix_tests[_i].enc, rfc3779_prefix_tests[_i].enc));
}
END_TEST

START_TEST(test_create_from_rfc3779_format_range)
{
	/* addressRange elements encode a from and to address, which may still
	 * represent prefixes */
	verify("10.5.0.0/23", NULL,
		traffic_selector_create_from_rfc3779_format(TS_IPV4_ADDR_RANGE,
					chunk_from_chars(0x00,0x0a,0x05),
					chunk_from_chars(0x01,0x0a,0x05,0x00)));
	verify("2001:0:200::/39", NULL,
		traffic_selector_create_from_rfc3779_format(TS_IPV6_ADDR_RANGE,
					chunk_from_chars(0x01,0x20,0x01,0x00,0x00,0x02),
					chunk_from_chars(0x02,0x20,0x01,0x00,0x00,0x00)));
	verify("10.2.48.0..10.2.64.255", NULL,
		traffic_selector_create_from_rfc3779_format(TS_IPV4_ADDR_RANGE,
					chunk_from_chars(0x04,0x0a,0x02,0x30),
					chunk_from_chars(0x00,0x0a,0x02,0x40)));
	verify("129.64.0.0..143.255.255.255", NULL,
		traffic_selector_create_from_rfc3779_format(TS_IPV4_ADDR_RANGE,
					chunk_from_chars(0x06,0x81,0x40),
					chunk_from_chars(0x04,0x80)));
}
END_TEST


static void verify_address(char *addr_from, char *addr_to, traffic_selector_t *ts)
{
	host_t *from, *to;

	from = host_create_from_string(addr_from, 0);
	to = host_create_from_string(addr_to, 0);

	ck_assert_chunk_eq(from->get_address(from), ts->get_from_address(ts));
	ck_assert_chunk_eq(to->get_address(to), ts->get_to_address(ts));
	from->destroy(from);
	to->destroy(to);
	ts->destroy(ts);
}

START_TEST(test_get_address_range)
{
	verify_address("10.1.0.1", "10.1.0.10",
				   traffic_selector_create_from_string(0, TS_IPV4_ADDR_RANGE,
											"10.1.0.1", 0, "10.1.0.10", 65535));
	/* currently not reordered */
	verify_address("10.1.0.10", "10.1.0.1",
				   traffic_selector_create_from_string(0, TS_IPV4_ADDR_RANGE,
											"10.1.0.10", 0, "10.1.0.1", 65535));
}
END_TEST

START_TEST(test_get_address_cidr)
{
	verify_address("10.1.0.0", "10.1.255.255",
				   traffic_selector_create_from_cidr("10.1.0.0/16", 0, 0, 65535));
	verify_address("fec1::", "fec1::ffff:ffff:ffff:ffff",
				   traffic_selector_create_from_cidr("fec1::/64", 0, 0, 65535));
}
END_TEST

struct {
	ts_type_t type;
	char *from;
	char *to;
	char *net;
	uint8_t mask;
	bool exact;
} to_subnet_tests[] = {
	{ TS_IPV4_ADDR_RANGE,	"10.0.0.1",	"10.0.0.1",			"10.0.0.1",	32,	TRUE	},
	{ TS_IPV4_ADDR_RANGE,	"10.0.0.0",	"10.255.255.255",	"10.0.0.0",	8,	TRUE	},
	{ TS_IPV4_ADDR_RANGE,	"10.0.0.1",	"10.0.0.255",		"10.0.0.0",	24,	FALSE	},
	{ TS_IPV4_ADDR_RANGE,	"10.0.0.0",	"10.0.0.15",		"10.0.0.0",	28,	TRUE	},
	{ TS_IPV4_ADDR_RANGE,	"10.0.0.1",	"10.0.0.15",		"10.0.0.0",	28,	FALSE	},
	{ TS_IPV4_ADDR_RANGE,	"10.0.0.1",	"10.0.0.16",		"10.0.0.0",	27,	FALSE	},
	{ TS_IPV6_ADDR_RANGE,	"fec1::1",	"fec1::1",						"fec1::1",	128,	TRUE	},
	{ TS_IPV6_ADDR_RANGE,	"fec1::0",	"fec1::ffff:ffff:ffff:ffff",	"fec1::",	64,		TRUE	},
	{ TS_IPV6_ADDR_RANGE,	"fec1::1",	"fec1::ffff:ffff:ffff:ffff",	"fec1::",	64,		FALSE	},
	{ TS_IPV6_ADDR_RANGE,	"fec1::1",	"fec1::7fff",					"fec1::",	113,	FALSE	},
	{ TS_IPV6_ADDR_RANGE,	"fec1::1",	"fec1::efff",					"fec1::",	112,	FALSE	},
};

START_TEST(test_to_subnet)
{
	traffic_selector_t *ts;
	host_t *net, *exp_net;
	uint8_t mask;

	ts = traffic_selector_create_from_string(0, to_subnet_tests[_i].type,
					to_subnet_tests[_i].from, 0, to_subnet_tests[_i].to, 0);
	ck_assert(ts->to_subnet(ts, &net, &mask) == to_subnet_tests[_i].exact);
	exp_net = host_create_from_string(to_subnet_tests[_i].net, 0);
	ck_assert(exp_net->ip_equals(exp_net, net));
	ck_assert_int_eq(to_subnet_tests[_i].mask, mask);
	exp_net->destroy(exp_net);
	net->destroy(net);
	ts->destroy(ts);
}
END_TEST

struct {
	char *cidr;
	uint16_t from_port;
	uint16_t to_port;
	uint16_t port;
} to_subnet_port_tests[] = {
	{ "10.0.0.0/8",		0,		0,		0	},
	{ "10.0.0.1/32",	80,		80,		80	},
	{ "10.0.0.1/32",	123,	465,	0	},
	{ "0.0.0.0/0",		0,		65535,	0	},
	{ "fec1::/64",		0,		0,		0	},
	{ "fec1::1/128",	80,		80,		80	},
	{ "fec1::1/128",	123,	465,	0	},
	{ "::/0",			0,		65535,	0	},
};

START_TEST(test_to_subnet_port)
{
	traffic_selector_t *ts;
	host_t *net, *exp_net;
	uint8_t mask;
	int exp_mask;

	ts = traffic_selector_create_from_cidr(to_subnet_port_tests[_i].cidr, 0,
										   to_subnet_port_tests[_i].from_port,
										   to_subnet_port_tests[_i].to_port);
	ck_assert(ts->to_subnet(ts, &net, &mask));
	exp_net = host_create_from_subnet(to_subnet_port_tests[_i].cidr, &exp_mask);
	ck_assert(exp_net->ip_equals(exp_net, net));
	ck_assert_int_eq(exp_mask, mask);
	ck_assert_int_eq(to_subnet_port_tests[_i].port, net->get_port(net));
	exp_net->destroy(exp_net);
	net->destroy(net);
	ts->destroy(ts);
}
END_TEST

START_TEST(test_subset)
{
	traffic_selector_t *a, *b;

	a = traffic_selector_create_from_cidr("10.1.0.0/16", 0, 0, 65535);
	b = traffic_selector_create_from_cidr("10.1.5.0/24", 0, 0, 65535);
	verify("10.1.5.0/24", NULL, a->get_subset(a, b));
	verify("10.1.5.0/24", NULL, b->get_subset(b, a));
	a->destroy(a);
	b->destroy(b);

	a = traffic_selector_create_from_cidr("fec1::/64", 0, 0, 65535);
	b = traffic_selector_create_from_cidr("fec1::1/128", 0, 0, 65535);
	verify("fec1::1/128", NULL, a->get_subset(a, b));
	verify("fec1::1/128", NULL, b->get_subset(b, a));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_subset_port)
{
	traffic_selector_t *a, *b;

	a = traffic_selector_create_from_cidr("10.0.0.0/8", IPPROTO_TCP, 55, 60);
	b = traffic_selector_create_from_cidr("10.2.7.16/30", 0, 0, 65535);
	verify("10.2.7.16/30[tcp/55-60]", "10.2.7.16/30[6/55-60]",
		a->get_subset(a, b));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_subset_equal)
{
	traffic_selector_t *a, *b;

	a = traffic_selector_create_from_cidr("10.1.0.0/16", IPPROTO_TCP, 80, 80);
	b = traffic_selector_create_from_cidr("10.1.0.0/16", IPPROTO_TCP, 80, 80);
	verify("10.1.0.0/16[tcp/http]", "10.1.0.0/16[6/80]", a->get_subset(a, b));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_subset_nonet)
{
	traffic_selector_t *a, *b;

	a = traffic_selector_create_from_cidr("10.1.0.0/16", 0, 0, 65535);
	b = traffic_selector_create_from_cidr("10.2.0.0/16", 0, 0, 65535);
	ck_assert(!a->get_subset(a, b));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_subset_noport)
{
	traffic_selector_t *a, *b;

	a = traffic_selector_create_from_cidr("10.1.0.0/16", 0, 0, 9999);
	b = traffic_selector_create_from_cidr("10.1.0.0/16", 0, 10000, 65535);
	ck_assert(!a->get_subset(a, b));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_subset_noproto)
{
	traffic_selector_t *a, *b;

	a = traffic_selector_create_from_cidr("10.1.0.0/16", IPPROTO_TCP, 0, 65535);
	b = traffic_selector_create_from_cidr("10.1.0.0/16", IPPROTO_UDP, 0, 65535);
	ck_assert(!a->get_subset(a, b));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_subset_nofamily)
{
	traffic_selector_t *a, *b;

	a = traffic_selector_create_from_cidr("0.0.0.0/0", 0, 0, 65535);
	b = traffic_selector_create_from_cidr("::/0", 0, 0, 65535);
	ck_assert(!a->get_subset(a, b));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_subset_dynamic)
{
	traffic_selector_t *a, *b;

	a = traffic_selector_create_dynamic(0, 0, 65535);
	b = traffic_selector_create_from_cidr("10.1.0.0/16", 0, 0, 65535);
	ck_assert(!a->get_subset(a, b));
	ck_assert(!b->get_subset(b, a));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_subset_opaque)
{
	traffic_selector_t *a, *b;

	a = traffic_selector_create_from_cidr("10.0.0.0/8", 0, 65535, 0);
	b = traffic_selector_create_from_cidr("10.2.7.16/30", IPPROTO_TCP, 80, 80);
	ck_assert(!a->get_subset(a, b));
	ck_assert(!b->get_subset(b, a));
	b->destroy(b);

	b = traffic_selector_create_from_cidr("10.2.7.16/30", IPPROTO_TCP, 65535, 0);
	verify("10.2.7.16/30[tcp/OPAQUE]", "10.2.7.16/30[6/OPAQUE]", a->get_subset(a, b));
	verify("10.2.7.16/30[tcp/OPAQUE]", "10.2.7.16/30[6/OPAQUE]", b->get_subset(b, a));
	b->destroy(b);

	b = traffic_selector_create_from_cidr("10.2.7.16/30", IPPROTO_TCP, 0, 65535);
	verify("10.2.7.16/30[tcp/OPAQUE]", "10.2.7.16/30[6/OPAQUE]", a->get_subset(a, b));
	verify("10.2.7.16/30[tcp/OPAQUE]", "10.2.7.16/30[6/OPAQUE]", b->get_subset(b, a));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

struct {
	char *net;
	char *host;
	bool inc;
} include_tests[] = {
	{ "0.0.0.0/0",		"192.168.1.2",			TRUE },
	{ "::/0",			"fec2::1",				TRUE },
	{ "fec2::/64",		"fec2::afaf",			TRUE },
	{ "10.1.0.0/16",	"10.1.0.1",				TRUE },
	{ "10.5.6.7/32",	"10.5.6.7",				TRUE },
	{ "0.0.0.0/0",		"fec2::1",				FALSE },
	{ "::/0",			"1.2.3.4",				FALSE },
	{ "10.0.0.0/16",	"10.1.0.0",				FALSE },
	{ "10.1.0.0/16",	"10.0.255.255",			FALSE },
	{ "fec2::/64",		"fec2:0:0:1::afaf",		FALSE },
};

START_TEST(test_includes)
{
	traffic_selector_t *ts;
	host_t *h;

	ts = traffic_selector_create_from_cidr(include_tests[_i].net, 0, 0, 65535);
	h = host_create_from_string(include_tests[_i].host, 0);
	ck_assert(ts->includes(ts, h) == include_tests[_i].inc);
	ts->destroy(ts);
	h->destroy(h);
}
END_TEST

struct {
	bool contained;
	struct {
		char *net;
		uint8_t proto;
		uint16_t from_port;
		uint16_t to_port;
	} a, b;
} is_contained_in_tests[] = {
	{  TRUE,  { "10.0.0.0/16", 0, 0, 65535 },	{ "10.0.0.0/16", 0, 0, 65535 },	},
	{  TRUE,  { "10.0.1.0/24", 0, 0, 65535 },	{ "10.0.0.0/16", 0, 0, 65535 },	},
	{  TRUE,  { "10.0.1.0/24", 17, 123, 456 },	{ "10.0.0.0/16", 0, 0, 65535 },	},
	{  TRUE,  { "10.0.1.0/24", 17, 123, 456 },	{ "10.0.0.0/16", 17, 123, 456 },},
	{  FALSE, { "10.0.0.0/8", 0, 0, 65535 },	{ "10.0.0.0/16", 0, 0, 65535 },	},
	{  FALSE, { "10.0.1.0/24", 17, 0, 65535 },	{ "10.0.0.0/16", 17, 123, 456 },},
	{  FALSE, { "fec2::/64", 0, 0, 65535 },		{ "10.0.0.0/16", 17, 123, 456 },},
};

START_TEST(test_is_contained_in)
{
	traffic_selector_t *a, *b;

	a = traffic_selector_create_from_cidr(
			is_contained_in_tests[_i].a.net, is_contained_in_tests[_i].a.proto,
			is_contained_in_tests[_i].a.from_port, is_contained_in_tests[_i].a.to_port);
	b = traffic_selector_create_from_cidr(
			is_contained_in_tests[_i].b.net, is_contained_in_tests[_i].b.proto,
			is_contained_in_tests[_i].b.from_port, is_contained_in_tests[_i].b.to_port);
	ck_assert(a->is_contained_in(a, b) == is_contained_in_tests[_i].contained);
	a->destroy(a);
	b->destroy(b);
}
END_TEST

struct {
	char *net;
	char *host;
	bool is_host;
	bool when_null;
} is_host_tests[] = {
	{ "0.0.0.0/0",		"192.168.1.2",	FALSE, FALSE },
	{ "::/0",			"fec2::1",		FALSE, FALSE },
	{ "192.168.1.0/24",	"192.168.1.0",	FALSE, FALSE },
	{ "192.168.1.2/32",	"192.168.1.2",	TRUE,  TRUE },
	{ "192.168.1.2/32",	"192.168.1.1",	FALSE, TRUE },
	{ "192.168.1.2/32",	"fec2::1",		FALSE, TRUE },
	{ "fec2::1/128",	"fec2::1",		TRUE,  TRUE },
	{ "fec2::1/128",	"fec2::2",		FALSE, TRUE },
	{ "fec2::1/128",	"192.168.1.2",	FALSE, TRUE },
};

START_TEST(test_is_host)
{
	traffic_selector_t *ts;
	host_t *h;

	ts = traffic_selector_create_from_cidr(is_host_tests[_i].net, 0, 0, 65535);
	h = host_create_from_string(is_host_tests[_i].host, 0);
	ck_assert(ts->is_host(ts, h) == is_host_tests[_i].is_host);
	ck_assert(ts->is_host(ts, NULL) == is_host_tests[_i].when_null);
	ts->destroy(ts);
	h->destroy(h);
}
END_TEST

START_TEST(test_is_host_dynamic)
{
	traffic_selector_t *ts;
	host_t *h;

	ts = traffic_selector_create_dynamic(0, 0, 65535);
	h = host_create_from_string(is_host_tests[_i].host, 0);
	ck_assert(!ts->is_host(ts, h));
	ck_assert(ts->is_host(ts, NULL));
	ts->destroy(ts);
	h->destroy(h);
}
END_TEST


struct {
	char *orig;
	char *host;
	char *after;
} set_address_tests[] = {
	{ "0.0.0.0/0",		"192.168.1.2",	"192.168.1.2/32" },
	{ "::/0",			"fec2::1",		"fec2::1/128" },
	{ "192.168.1.2/32",	"192.168.1.1",	"192.168.1.1/32" },
	{ "192.168.1.0/24",	"192.168.1.1",	"192.168.1.1/32" },
	{ "192.168.1.2/32",	"fec2::1",		"fec2::1/128" },
	{ "192.168.1.0/24",	"fec2::1",		"fec2::1/128" },
	{ "192.168.1.2/32",	"%any",			"0.0.0.0/0" },
	{ "192.168.1.0/24",	"%any",			"0.0.0.0/0" },
	{ "192.168.1.2/32",	"%any6",		"::/0" },
	{ "192.168.1.0/24",	"%any6",		"::/0" },
	{ "fec2::1/128",	"192.168.1.1",	"192.168.1.1/32" },
	{ "fec2::/64",		"192.168.1.1",	"192.168.1.1/32" },
	{ "fec2::1/128",	"fec2::2",		"fec2::2/128" },
	{ "fec2::/64",		"fec2::2",		"fec2::2/128" },
	{ "fec2::1/128",	"%any",			"0.0.0.0/0" },
	{ "fec2::/64",		"%any",			"0.0.0.0/0" },
	{ "fec2::1/128",	"%any6",		"::/0" },
	{ "fec2::/64",		"%any6",		"::/0" },
	{ NULL,				"192.168.1.1",	"192.168.1.1/32" },
	{ NULL,				"fec2::1",		"fec2::1/128" },
	{ NULL,				"%any",			"0.0.0.0/0" },
	{ NULL,				"%any6",		"::/0" },
};

START_TEST(test_set_address)
{
	traffic_selector_t *ts;
	host_t *h;

	if (set_address_tests[_i].orig)
	{
		ts = traffic_selector_create_from_cidr(set_address_tests[_i].orig, 0, 0, 65535);
		ck_assert(!ts->is_dynamic(ts));
	}
	else
	{
		ts = traffic_selector_create_dynamic(0, 0, 65535);
		ck_assert(ts->is_dynamic(ts));
	}
	h = host_create_from_string(set_address_tests[_i].host, 0);
	ts->set_address(ts, h);
	ck_assert(!ts->is_dynamic(ts));
	verify(set_address_tests[_i].after, NULL, ts);
	h->destroy(h);
}
END_TEST


struct {
	int res;
	struct {
		char *net;
		uint8_t proto;
		uint16_t from_port;
		uint16_t to_port;
	} a, b;
} cmp_tests[] = {
	{  0, { "10.0.0.0/8", 0, 0, 65535 },	{ "10.0.0.0/8", 0, 0, 65535 },	},
	{  0, { "10.0.0.0/8", 17, 123, 456 },	{ "10.0.0.0/8", 17, 123, 456 },	},
	{  0, { "fec2::/64", 0, 0, 65535 },		{ "fec2::/64", 0, 0, 65535 },	},
	{  0, { "fec2::/64", 4, 0, 65535 },		{ "fec2::/64", 4, 0, 65535 },	},

	{ -1, { "1.0.0.0/8", 0, 0, 65535 },		{ "2.0.0.0/8", 0, 0, 65535 },	},
	{  1, { "2.0.0.0/8", 0, 0, 65535 },		{ "1.0.0.0/8", 0, 0, 65535 },	},
	{ -1, { "1.0.0.0/8", 0, 0, 65535 },		{ "1.0.0.0/16", 0, 0, 65535 },	},
	{  1, { "1.0.0.0/16", 0, 0, 65535 },	{ "1.0.0.0/8", 0, 0, 65535 },	},
	{ -1, { "fec1::/64", 0, 0, 65535 },		{ "fec2::/64", 0, 0, 65535 },	},
	{  1, { "fec2::/64", 0, 0, 65535 },		{ "fec1::/64", 0, 0, 65535 },	},
	{ -1, { "fec1::/48", 0, 0, 65535 },		{ "fec1::/64", 0, 0, 65535 },	},
	{  1, { "fec1::/64", 0, 0, 65535 },		{ "fec1::/48", 0, 0, 65535 },	},

	{ -1, { "10.0.0.0/8", 0, 0, 65535 },	{ "fec2::/64", 0, 0, 65535 },	},
	{  1, { "fec2::/64", 0, 0, 65535 },		{ "10.0.0.0/8", 0, 0, 65535 },	},

	{ -1, { "10.0.0.0/8", 16, 123, 456 },	{ "10.0.0.0/8", 17, 123, 456 },	},
	{  1, { "fec2::/64", 5, 0, 65535 },		{ "fec2::/64", 4, 0, 65535 },	},

	{ -1, { "10.0.0.0/8", 17, 111, 456 },	{ "10.0.0.0/8", 17, 222, 456 },	},
	{  1, { "fec2::/64", 17, 555, 65535 },	{ "fec2::/64", 17, 444, 65535 },},

	{ -1, { "10.0.0.0/8", 17, 55, 65535 },	{ "10.0.0.0/8", 17, 55, 666 },	},
	{  1, { "fec2::/64", 17, 55, 111 },		{ "fec2::/64", 17, 55, 4567 },	},

};

START_TEST(test_cmp)
{
	traffic_selector_t *a, *b;

	a = traffic_selector_create_from_cidr(
						cmp_tests[_i].a.net, cmp_tests[_i].a.proto,
						cmp_tests[_i].a.from_port, cmp_tests[_i].a.to_port);
	b = traffic_selector_create_from_cidr(
						cmp_tests[_i].b.net, cmp_tests[_i].b.proto,
						cmp_tests[_i].b.from_port, cmp_tests[_i].b.to_port);
	switch (cmp_tests[_i].res)
	{
		case 0:
			ck_assert(traffic_selector_cmp(a, b, NULL) == 0);
			ck_assert(a->equals(a, b));
			break;
		case 1:
			ck_assert(traffic_selector_cmp(a, b, NULL) > 0);
			ck_assert(!a->equals(a, b));
			break;
		case -1:
			ck_assert(traffic_selector_cmp(a, b, NULL) < 0);
			ck_assert(!a->equals(a, b));
			break;
	}
	a->destroy(a);
	b->destroy(b);
}
END_TEST

static void verify_clone(traffic_selector_t *ts)
{
	traffic_selector_t *clone;

	clone = ts->clone(ts);
	if (!ts->equals(ts, clone))
	{
		fail("%R != %R", ts, clone);
	}
	/* equals() already compares most of these but not all */
	ck_assert(ts->get_type(ts) == clone->get_type(clone));
	ck_assert(ts->get_protocol(ts) == clone->get_protocol(clone));
	ck_assert(ts->get_from_port(ts) == clone->get_from_port(clone));
	ck_assert(ts->get_to_port(ts) == clone->get_to_port(clone));
	ck_assert_chunk_eq(ts->get_from_address(ts), clone->get_from_address(clone));
	ck_assert_chunk_eq(ts->get_to_address(ts), clone->get_to_address(clone));
	ck_assert(ts->is_host(ts, NULL) == clone->is_host(clone, NULL));
	ck_assert(ts->is_dynamic(ts) == clone->is_dynamic(clone));
	clone->destroy(clone);
	ts->destroy(ts);
}

START_TEST(test_clone)
{
	traffic_selector_t *ts;
	host_t *h;

	ts = traffic_selector_create_dynamic(0, 0, 0);
	verify_clone(ts);
	ts = traffic_selector_create_dynamic(IPPROTO_UDP, 123, 456);
	verify_clone(ts);
	ts = traffic_selector_create_dynamic(IPPROTO_UDP, 0, 65535);
	verify_clone(ts);

	h = host_create_from_string("192.168.1.1", 0);
	ts = traffic_selector_create_dynamic(0, 0, 0);
	ts->set_address(ts, h);
	verify_clone(ts);
	ts = traffic_selector_create_dynamic(IPPROTO_UDP, 123, 456);
	ts->set_address(ts, h);
	verify_clone(ts);
	h->destroy(h);

	ts = traffic_selector_create_from_string(0, TS_IPV4_ADDR_RANGE, "10.0.0.1", 0, "10.0.0.16", 65535);
	verify_clone(ts);
	ts = traffic_selector_create_from_string(IPPROTO_TCP, TS_IPV6_ADDR_RANGE, "fec1::1", 80, "fec1::1:0000", 80);
	verify_clone(ts);
	ts = traffic_selector_create_from_cidr("10.0.0.0/8", 0, 0, 65535);
	verify_clone(ts);
	ts = traffic_selector_create_from_cidr("fec1::/64", 0, 0, 65535);
	verify_clone(ts);
}
END_TEST

START_TEST(test_hash)
{
	traffic_selector_t *a, *b;
	host_t *h;

	a = traffic_selector_create_dynamic(0, 0, 0);
	b = traffic_selector_create_from_cidr("0.0.0.0/0", 0, 0, 0);
	ck_assert(a->hash(a, 0) != a->hash(a, 1));
	ck_assert_int_eq(a->hash(a, 0), b->hash(b, 0));
	ck_assert_int_eq(a->hash(a, 1), b->hash(b, 1));

	h = host_create_from_string("192.168.1.1", 0);
	a->set_address(a, h);
	ck_assert(a->hash(a, 0) != b->hash(b, 0));
	h->destroy(h);

	a->destroy(a);
	a = traffic_selector_create_from_string(0, TS_IPV4_ADDR_RANGE, "192.168.0.0", 0, "192.168.0.255", 65535);
	ck_assert(a->hash(a, 0) != b->hash(b, 0));
	b->destroy(b);
	b = traffic_selector_create_from_cidr("192.168.0.0/24", 0, 0, 65535);
	ck_assert_int_eq(a->hash(a, 0), b->hash(b, 0));
	b->destroy(b);
	b = traffic_selector_create_from_cidr("192.168.0.0/24", IPPROTO_TCP, 0, 65535);
	ck_assert(a->hash(a, 0) != b->hash(b, 0));
	b->destroy(b);
	b = traffic_selector_create_from_cidr("192.168.0.0/24", 0, 123, 456);
	ck_assert(a->hash(a, 0) != b->hash(b, 0));
	b->destroy(b);
	a->destroy(a);
}
END_TEST

struct {
	uint8_t proto;
	uint16_t from_port;
	uint16_t to_port;
	uint8_t from_type;
	uint8_t from_code;
	uint8_t to_type;
	uint8_t to_code;
	char *str;
	char *str_alt;
} icmp_tests[] = {
	{ IPPROTO_ICMP, 0, 0, 0, 0, 0, 0, "dynamic[icmp/0]", "dynamic[1/0]" },
	{ IPPROTO_ICMP, 3, 3, 3, 0, 3, 0, "dynamic[icmp/3]", "dynamic[1/3]" },
	{ IPPROTO_ICMP, 0x0307, 0x0307, 3, 7, 3, 7, "dynamic[icmp/3(7)]", "dynamic[1/3(7)]" },
	{ IPPROTO_ICMP, 0x0300, 0x040f, 3, 0, 4, 15, "dynamic[icmp/3-4(15)]", "dynamic[1/3-4(15)]" },
	{ IPPROTO_ICMP, 0x0301, 0x040f, 3, 1, 4, 15, "dynamic[icmp/3(1)-4(15)]", "dynamic[1/3(1)-4(15)]" },
	{ IPPROTO_ICMPV6, 0, 0, 0, 0, 0, 0, "dynamic[ipv6-icmp/0]", "dynamic[58/0]" },
	{ IPPROTO_ICMPV6, 1, 1, 1, 0, 1, 0, "dynamic[ipv6-icmp/1]", "dynamic[58/1]" },
	{ IPPROTO_ICMPV6, 0x0104, 0x0104, 1, 4, 1, 4, "dynamic[ipv6-icmp/1(4)]", "dynamic[58/1(4)]" },
	{ IPPROTO_ICMPV6, 0x0100, 0x040f, 1, 0, 4, 15, "dynamic[ipv6-icmp/1-4(15)]", "dynamic[58/1-4(15)]" },
	{ IPPROTO_ICMPV6, 0x0101, 0x040f, 1, 1, 4, 15, "dynamic[ipv6-icmp/1(1)-4(15)]", "dynamic[58/1(1)-4(15)]" },
};

START_TEST(test_icmp)
{
	traffic_selector_t *ts;
	uint16_t from, to;

	ts = traffic_selector_create_dynamic(icmp_tests[_i].proto,
							icmp_tests[_i].from_port, icmp_tests[_i].to_port);
	from = ts->get_from_port(ts);
	to = ts->get_to_port(ts);
	ck_assert_int_eq(icmp_tests[_i].from_type, traffic_selector_icmp_type(from));
	ck_assert_int_eq(icmp_tests[_i].from_code, traffic_selector_icmp_code(from));
	ck_assert_int_eq(icmp_tests[_i].to_type, traffic_selector_icmp_type(to));
	ck_assert_int_eq(icmp_tests[_i].to_code, traffic_selector_icmp_code(to));
	verify(icmp_tests[_i].str, icmp_tests[_i].str_alt, ts);
}
END_TEST

static void verify_list(const char *str, const char *alt, linked_list_t *list)
{
	char buf[512];

	snprintf(buf, sizeof(buf), "%#R", list);
	list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
	if (!streq(buf, str) && !streq(buf, alt))
	{
		fail("%s != %s or %s", buf, str, alt);
	}
}

START_TEST(test_printf_hook_null)
{
	verify("(null)", NULL, NULL);
}
END_TEST

START_TEST(test_printf_hook_hash)
{
	linked_list_t *list;

	list = linked_list_create_with_items(
				traffic_selector_create_from_cidr("10.1.0.0/16", 0, 0, 65535),
				NULL);
	verify_list("10.1.0.0/16", NULL, list);
	list = linked_list_create_with_items(
				traffic_selector_create_from_cidr("10.1.0.0/16", 0, 0, 65535),
				traffic_selector_create_from_cidr("10.1.0.1/32", IPPROTO_UDP, 1234, 1235),
				NULL);
	verify_list("10.1.0.0/16 10.1.0.1/32[udp/1234-1235]", "10.1.0.0/16 10.1.0.1/32[17/1234-1235]", list);
	list = linked_list_create_with_items(
				traffic_selector_create_from_cidr("10.1.0.0/16", 0, 0, 65535),
				traffic_selector_create_from_string(IPPROTO_UDP, TS_IPV4_ADDR_RANGE, "10.1.0.1", 1234, "10.1.0.99", 1235),
				NULL);
	verify_list("10.1.0.0/16 10.1.0.1..10.1.0.99[udp/1234-1235]", "10.1.0.0/16 10.1.0.1..10.1.0.99[17/1234-1235]", list);
}
END_TEST

Suite *traffic_selector_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("traffic selector");

	tc = tcase_create("create");
	tcase_add_test(tc, test_create_from_string);
	tcase_add_test(tc, test_create_from_cidr);
	tcase_add_test(tc, test_create_from_bytes);
	tcase_add_test(tc, test_create_from_subnet);
	tcase_add_loop_test(tc, test_create_from_rfc3779_format_prefix, 0, countof(rfc3779_prefix_tests));
	tcase_add_test(tc, test_create_from_rfc3779_format_range);
	suite_add_tcase(s, tc);

	tc = tcase_create("addresses");
	tcase_add_test(tc, test_get_address_range);
	tcase_add_test(tc, test_get_address_cidr);
	suite_add_tcase(s, tc);

	tc = tcase_create("to_subnet");
	tcase_add_loop_test(tc, test_to_subnet, 0, countof(to_subnet_tests));
	tcase_add_loop_test(tc, test_to_subnet_port, 0, countof(to_subnet_port_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("subset");
	tcase_add_test(tc, test_subset);
	tcase_add_test(tc, test_subset_port);
	tcase_add_test(tc, test_subset_equal);
	tcase_add_test(tc, test_subset_nonet);
	tcase_add_test(tc, test_subset_noport);
	tcase_add_test(tc, test_subset_noproto);
	tcase_add_test(tc, test_subset_nofamily);
	tcase_add_test(tc, test_subset_dynamic);
	tcase_add_test(tc, test_subset_opaque);
	suite_add_tcase(s, tc);

	tc = tcase_create("includes");
	tcase_add_loop_test(tc, test_includes, 0, countof(include_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("is_contained_in");
	tcase_add_loop_test(tc, test_is_contained_in, 0, countof(is_contained_in_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("is_host");
	tcase_add_loop_test(tc, test_is_host, 0, countof(is_host_tests));
	tcase_add_loop_test(tc, test_is_host_dynamic, 0, countof(is_host_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("set_address");
	tcase_add_loop_test(tc, test_set_address, 0, countof(is_host_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("cmp");
	tcase_add_loop_test(tc, test_cmp, 0, countof(cmp_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("clone");
	tcase_add_test(tc, test_clone);
	suite_add_tcase(s, tc);

	tc = tcase_create("hash");
	tcase_add_test(tc, test_hash);
	suite_add_tcase(s, tc);

	tc = tcase_create("icmp");
	tcase_add_loop_test(tc, test_icmp, 0, countof(icmp_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("printf hook");
	tcase_add_test(tc, test_printf_hook_null);
	tcase_add_test(tc, test_printf_hook_hash);
	suite_add_tcase(s, tc);

	return s;
}
