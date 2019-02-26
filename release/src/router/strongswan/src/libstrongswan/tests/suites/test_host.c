/*
 * Copyright (C) 2013 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#include <networking/host.h>

/**
 * Verify a netmask (a number of set bits starting at byte 0)
 * Can also be used to check for %any (mask == 0)
 */
static void verify_netmask(chunk_t addr, int mask)
{
	int byte, bit;

	for (byte = 0; byte < addr.len; byte++)
	{
		for (bit = 7; bit >= 0; bit--)
		{
			int val = (addr.ptr[byte] >> bit) & 0x01;
			if (mask-- > 0)
			{
				ck_assert_int_eq(val, 1);
			}
			else
			{
				ck_assert_int_eq(val, 0);
			}
		}
	}
}

/*******************************************************************************
 * host_create_any
 */

static void verify_any(host_t *host, int family, uint16_t port)
{
	verify_netmask(host->get_address(host), 0);
	ck_assert(host->is_anyaddr(host));
	ck_assert_int_eq(host->get_port(host), port);
	ck_assert_int_eq(host->get_family(host), family);
}

static void test_create_any(int family)
{
	host_t *host;

	host = host_create_any(family);
	verify_any(host, family, 0);
	host->destroy(host);
}

START_TEST(test_create_any_v4)
{
	test_create_any(AF_INET);
}
END_TEST

START_TEST(test_create_any_v6)
{
	test_create_any(AF_INET6);
}
END_TEST

START_TEST(test_create_any_other)
{
	host_t *host;

	host = host_create_any(AF_UNSPEC);
	ck_assert(host == NULL);
}
END_TEST

/*******************************************************************************
 * host_create_from_string
 */

static void verify_address(host_t *host, chunk_t addr, int family, uint16_t port)
{
	ck_assert(chunk_equals(host->get_address(host), addr));
	ck_assert(!host->is_anyaddr(host));
	ck_assert_int_eq(host->get_port(host), port);
	ck_assert_int_eq(host->get_family(host), family);
}

static const chunk_t addr_v4 = chunk_from_chars(0xc0, 0xa8, 0x00, 0x01);
static const chunk_t addr_v6 = chunk_from_chars(0xfe, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
												0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01);

START_TEST(test_create_from_string_v4)
{
	host_t *host;

	host = host_create_from_string(NULL, 500);
	ck_assert(!host);

	host = host_create_from_string("%any", 500);
	verify_any(host, AF_INET, 500);
	host->destroy(host);

	host = host_create_from_string("%any4", 500);
	verify_any(host, AF_INET, 500);
	host->destroy(host);

	host = host_create_from_string("0.0.0.0", 500);
	verify_any(host, AF_INET, 500);
	host->destroy(host);

	host = host_create_from_string("192.168.0.1", 500);
	verify_address(host, addr_v4, AF_INET, 500);
	host->destroy(host);

	host = host_create_from_string("192.168.0.1::500", 500);
	ck_assert(host == NULL);
	host = host_create_from_string("123.456.789.012", 500);
	ck_assert(host == NULL);
	host = host_create_from_string("1.1.1.1.1.1.1.1", 500);
	ck_assert(host == NULL);
	host = host_create_from_string("foo.b.a.r", 500);
	ck_assert(host == NULL);
}
END_TEST

START_TEST(test_create_from_string_any_v6)
{
	host_t *host;

	host = host_create_from_string("%any6", 500);
	verify_any(host, AF_INET6, 500);
	host->destroy(host);

	host = host_create_from_string("::", 500);
	verify_any(host, AF_INET6, 500);
	host->destroy(host);

	host = host_create_from_string("fec1::1", 500);
	verify_address(host, addr_v6, AF_INET6, 500);
	host->destroy(host);

	host = host_create_from_string("fec1::1.500", 500);
	ck_assert(host == NULL);
	host = host_create_from_string("f::e::c::1::1", 500);
	ck_assert(host == NULL);
	host = host_create_from_string("foo::bar", 500);
	ck_assert(host == NULL);
}
END_TEST

/*******************************************************************************
 * host_create_from_string_and_family
 */

static void test_create_from_string_and_family_any(char *string, int family,
												   int expected)
{
	host_t *host;

	host = host_create_from_string_and_family(string, family, 500);
	if (expected == AF_UNSPEC)
	{
		ck_assert(host == NULL);
	}
	else
	{
		verify_any(host, expected, 500);
		host->destroy(host);
	}
}

static void test_create_from_string_and_family_addr(char *string, chunk_t addr,
													int family, int expected)
{
	host_t *host;

	host = host_create_from_string_and_family(string, family, 500);
	if (expected == AF_UNSPEC)
	{
		ck_assert(host == NULL);
	}
	else
	{
		verify_address(host, addr, expected, 500);
		host->destroy(host);
	}
}

START_TEST(test_create_from_string_and_family_v4)
{
	test_create_from_string_and_family_any(NULL, AF_INET, AF_UNSPEC);
	test_create_from_string_and_family_any("%any", AF_INET, AF_INET);
	test_create_from_string_and_family_any("%any4", AF_INET, AF_INET);
	test_create_from_string_and_family_any("0.0.0.0", AF_INET, AF_INET);

	test_create_from_string_and_family_any("%any4", AF_INET6, AF_UNSPEC);
	test_create_from_string_and_family_any("0.0.0.0", AF_INET6, AF_UNSPEC);

	test_create_from_string_and_family_addr("192.168.0.1", addr_v4, AF_INET, AF_INET);
	test_create_from_string_and_family_addr("192.168.0.1", addr_v4, AF_INET6, AF_UNSPEC);
}
END_TEST

START_TEST(test_create_from_string_and_family_v6)
{
	test_create_from_string_and_family_any(NULL, AF_INET6, AF_UNSPEC);
	test_create_from_string_and_family_any("%any", AF_INET6, AF_INET6);
	test_create_from_string_and_family_any("%any6", AF_INET6, AF_INET6);
	test_create_from_string_and_family_any("::", AF_INET6, AF_INET6);

	test_create_from_string_and_family_any("%any6", AF_INET, AF_UNSPEC);
	test_create_from_string_and_family_any("::", AF_INET, AF_UNSPEC);

	test_create_from_string_and_family_addr("fec1::1", addr_v6, AF_INET6, AF_INET6);
	test_create_from_string_and_family_addr("fec1::1", addr_v6, AF_INET, AF_UNSPEC);
}
END_TEST

START_TEST(test_create_from_string_and_family_other)
{
	test_create_from_string_and_family_any(NULL, AF_UNSPEC, AF_UNSPEC);
	test_create_from_string_and_family_any("%any", AF_UNSPEC, AF_INET);
	test_create_from_string_and_family_any("%any4", AF_UNSPEC, AF_INET);
	test_create_from_string_and_family_any("0.0.0.0", AF_UNSPEC, AF_INET);

	test_create_from_string_and_family_any("%any6", AF_UNSPEC, AF_INET6);
	test_create_from_string_and_family_any("::", AF_UNSPEC, AF_INET6);

	test_create_from_string_and_family_addr("192.168.0.1", addr_v4, AF_UNSPEC, AF_INET);
	test_create_from_string_and_family_addr("fec1::1", addr_v6, AF_UNSPEC, AF_INET6);
}
END_TEST

/*******************************************************************************
 * host_create_from_dns
 */

static void test_create_from_dns(int family, chunk_t addr)
{
	host_t *host;

	host = host_create_from_dns("localhost", family, 500);
	if (family != AF_INET6)
	{
		ck_assert(host != NULL);
	}
	if (host)
	{
		if (family != AF_UNSPEC)
		{
			verify_address(host, addr, family, 500);
		}
		host->destroy(host);
	}
}

START_TEST(test_create_from_dns_any)
{
	test_create_from_dns(AF_UNSPEC, chunk_empty);
}
END_TEST

START_TEST(test_create_from_dns_v4)
{
	test_create_from_dns(AF_INET, chunk_from_chars(127,0,0,1));
}
END_TEST

START_TEST(test_create_from_dns_v6)
{
	test_create_from_dns(AF_INET6,
						 chunk_from_chars(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1));
}
END_TEST

/*******************************************************************************
 * host_create_from_sockaddr
 */

START_TEST(test_create_from_sockaddr_v4)
{
	struct sockaddr_in addr = {
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
		.sin_len = sizeof(struct sockaddr_in),
#endif
		.sin_family = AF_INET,
		.sin_port = htons(500),
	}, *val;
	socklen_t *socklen;
	host_t *host;

	host = host_create_from_sockaddr((sockaddr_t*)&addr);
	verify_any(host, AF_INET, 500);
	val = (struct sockaddr_in*)host->get_sockaddr(host);
	ck_assert(memeq(&addr, val, sizeof(addr)));
	socklen = host->get_sockaddr_len(host);
	ck_assert(*socklen == sizeof(addr));
	host->destroy(host);
}
END_TEST

START_TEST(test_create_from_sockaddr_v6)
{
	struct sockaddr_in6 addr = {
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
		.sin6_len = sizeof(struct sockaddr_in6),
#endif
		.sin6_family = AF_INET6,
		.sin6_port = htons(500),
	}, *val;
	socklen_t *socklen;
	host_t *host;

	host = host_create_from_sockaddr((sockaddr_t*)&addr);
	verify_any(host, AF_INET6, 500);
	val = (struct sockaddr_in6*)host->get_sockaddr(host);
	ck_assert(memeq(&addr, val, sizeof(addr)));
	socklen = host->get_sockaddr_len(host);
	ck_assert(*socklen == sizeof(addr));
	host->destroy(host);
}
END_TEST

START_TEST(test_create_from_sockaddr_other)
{
	struct sockaddr addr = {
		.sa_family = AF_UNIX,
	};
	host_t *host;

	host = host_create_from_sockaddr((sockaddr_t*)&addr);
	ck_assert(host == NULL);
}
END_TEST

/*******************************************************************************
 * host_create_from_chunk
 */

START_TEST(test_create_from_chunk_v4)
{
	host_t *host;

	host = host_create_from_chunk(AF_INET, addr_v4, 500);
	verify_address(host, addr_v4, AF_INET, 500);
	host->destroy(host);

	host = host_create_from_chunk(AF_UNSPEC, addr_v4, 500);
	verify_address(host, addr_v4, AF_INET, 500);
	host->destroy(host);

	host = host_create_from_chunk(AF_INET, chunk_empty, 500);
	ck_assert(host == NULL);
	host = host_create_from_chunk(AF_UNSPEC, chunk_empty, 500);
	ck_assert(host == NULL);
}
END_TEST

START_TEST(test_create_from_chunk_v6)
{
	host_t *host;

	host = host_create_from_chunk(AF_INET6, addr_v6, 500);
	verify_address(host, addr_v6, AF_INET6, 500);
	host->destroy(host);

	host = host_create_from_chunk(AF_UNSPEC, addr_v6, 500);
	verify_address(host, addr_v6, AF_INET6, 500);
	host->destroy(host);

	host = host_create_from_chunk(AF_INET6, chunk_empty, 500);
	ck_assert(host == NULL);
}
END_TEST

START_TEST(test_create_from_chunk_other)
{
	host_t *host;

	host = host_create_from_chunk(AF_UNIX, addr_v6, 500);
	ck_assert(host == NULL);
}
END_TEST

/*******************************************************************************
 * host_create_from_subnet
 */

START_TEST(test_create_from_subnet_v4)
{
	host_t *host;
	int bits = -1;

	host = host_create_from_subnet("0.0.0.0/0", &bits);
	verify_any(host, AF_INET, 0);
	ck_assert_int_eq(bits, 0);
	host->destroy(host);

	host = host_create_from_subnet("192.168.0.1", &bits);
	verify_address(host, addr_v4, AF_INET, 0);
	ck_assert_int_eq(bits, 32);
	host->destroy(host);

	host = host_create_from_subnet("192.168.0.1/24", &bits);
	verify_address(host, addr_v4, AF_INET, 0);
	ck_assert_int_eq(bits, 24);
	host->destroy(host);

	host = host_create_from_subnet("foo.b.a.r", &bits);
	ck_assert(host == NULL);
}
END_TEST

START_TEST(test_create_from_subnet_v6)
{
	host_t *host;
	int bits = -1;

	host = host_create_from_subnet("::/0", &bits);
	verify_any(host, AF_INET6, 0);
	ck_assert_int_eq(bits, 0);
	host->destroy(host);

	host = host_create_from_subnet("fec1::1", &bits);
	verify_address(host, addr_v6, AF_INET6, 0);
	ck_assert_int_eq(bits, 128);
	host->destroy(host);

	host = host_create_from_subnet("fec1::1/64", &bits);
	verify_address(host, addr_v6, AF_INET6, 0);
	ck_assert_int_eq(bits, 64);
	host->destroy(host);

	host = host_create_from_subnet("foo::bar", &bits);
	ck_assert(host == NULL);
}
END_TEST

/*******************************************************************************
 * host_create_from_range
 */

static const chunk_t addr_v4_to = chunk_from_chars(0xc0, 0xa8, 0x00, 0x05);
static const chunk_t addr_v6_to = chunk_from_chars(0xfe, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
												   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05);

static void verify_range(char *str, int family, chunk_t from_addr,
						 chunk_t to_addr)
{
	host_t *from, *to;

	if (!family)
	{
		ck_assert(!host_create_from_range(str, &from, &to));
	}
	else
	{
		ck_assert(host_create_from_range(str, &from, &to));
		verify_address(from, from_addr, family, 0);
		verify_address(to, to_addr, family, 0);
		from->destroy(from);
		to->destroy(to);
	}
}

START_TEST(test_create_from_range_v4)
{
	host_t *from, *to;

	ck_assert(host_create_from_range("0.0.0.0-0.0.0.0", &from, &to));
	verify_any(from, AF_INET, 0);
	verify_any(to, AF_INET, 0);
	from->destroy(from);
	to->destroy(to);

	verify_range("192.168.0.1-192.168.0.1", AF_INET, addr_v4, addr_v4);
	verify_range("192.168.0.1-192.168.0.5", AF_INET, addr_v4, addr_v4_to);
	verify_range("192.168.0.1- 192.168.0.5", AF_INET, addr_v4, addr_v4_to);
	verify_range("192.168.0.1 -192.168.0.5", AF_INET, addr_v4, addr_v4_to);
	verify_range("192.168.0.1 - 192.168.0.5", AF_INET, addr_v4, addr_v4_to);
	verify_range("192.168.0.5-192.168.0.1", AF_INET, addr_v4_to, addr_v4);

	verify_range("192.168.0.1", 0, chunk_empty, chunk_empty);
	verify_range("192.168.0.1-", 0, chunk_empty, chunk_empty);
	verify_range("-192.168.0.1", 0, chunk_empty, chunk_empty);
	verify_range("192.168.0.1-192", 0, chunk_empty, chunk_empty);
	verify_range("192.168.0.1-192.168", 0, chunk_empty, chunk_empty);
	verify_range("192.168.0.1-192.168.0", 0, chunk_empty, chunk_empty);
	verify_range("foo.b.a.r", 0, chunk_empty, chunk_empty);
	verify_range("foo.b.a.r-b.a.r.f", 0, chunk_empty, chunk_empty);
}
END_TEST

START_TEST(test_create_from_range_v6)
{
	host_t *from, *to;

	ck_assert(host_create_from_range("::-::", &from, &to));
	verify_any(from, AF_INET6, 0);
	verify_any(to, AF_INET6, 0);
	from->destroy(from);
	to->destroy(to);

	verify_range("fec1::1-fec1::1", AF_INET6, addr_v6, addr_v6);
	verify_range("fec1::1-fec1::5", AF_INET6, addr_v6, addr_v6_to);
	verify_range("fec1::1- fec1::5", AF_INET6, addr_v6, addr_v6_to);
	verify_range("fec1::1 -fec1::5", AF_INET6, addr_v6, addr_v6_to);
	verify_range("fec1::1 - fec1::5", AF_INET6, addr_v6, addr_v6_to);
	verify_range("fec1::5-fec1::1", AF_INET6, addr_v6_to, addr_v6);

	verify_range("fec1::1", 0, chunk_empty, chunk_empty);
	verify_range("fec1::1-", 0, chunk_empty, chunk_empty);
	verify_range("-fec1::1", 0, chunk_empty, chunk_empty);
	verify_range("fec1::1-fec1", 0, chunk_empty, chunk_empty);
	verify_range("foo::bar", 0, chunk_empty, chunk_empty);
	verify_range("foo::bar-bar::foo", 0, chunk_empty, chunk_empty);

	verify_range("fec1::1-192.168.0.1", 0, chunk_empty, chunk_empty);
	verify_range("192.168.0.1-fec1::1", 0, chunk_empty, chunk_empty);
}
END_TEST

/*******************************************************************************
 * host_create_netmask
 */

static void test_create_netmask(int family)
{
	host_t *netmask;
	int i, len = (family == AF_INET) ? 32 : 128;

	netmask = host_create_netmask(family, -1);
	ck_assert(netmask == NULL);
	for (i = 0; i <= len; i++)
	{
		netmask = host_create_netmask(family, i);
		verify_netmask(netmask->get_address(netmask), i);
		netmask->destroy(netmask);
	}
	netmask = host_create_netmask(family, len + 1);
	ck_assert(netmask == NULL);
}

START_TEST(test_create_netmask_v4)
{
	test_create_netmask(AF_INET);
}
END_TEST

START_TEST(test_create_netmask_v6)
{
	test_create_netmask(AF_INET6);
}
END_TEST

START_TEST(test_create_netmask_other)
{
	host_t *netmask;

	netmask = host_create_netmask(AF_UNSPEC, 0);
	ck_assert(netmask == NULL);
}
END_TEST

/*******************************************************************************
 * equals, ip_equals
 */

START_TEST(test_equals)
{
	host_t *a, *b;

	a = host_create_from_string("192.168.0.1", 500);
	b = host_create_from_string("192.168.0.1", 0);
	ck_assert(!a->equals(a, b));
	ck_assert(!b->equals(b, a));
	ck_assert(a->ip_equals(a, b));
	ck_assert(b->ip_equals(b, a));
	b->set_port(b, 500);
	ck_assert(a->equals(a, b));
	ck_assert(b->equals(b, a));
	ck_assert(a->ip_equals(a, b));
	ck_assert(b->ip_equals(b, a));
	b->destroy(b);
	b = host_create_from_string("192.168.0.2", 500);
	ck_assert(!a->ip_equals(a, b));
	ck_assert(!a->equals(a, b));
	b->destroy(b);

	b = host_create_from_string("fec1::1", 500);
	ck_assert(!a->ip_equals(a, b));
	ck_assert(!a->equals(a, b));
	a->destroy(a);
	a = host_create_from_string("fec1::1", 500);
	ck_assert(a->equals(a, b));
	ck_assert(a->ip_equals(a, b));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_equals_any)
{
	host_t *a, *b;

	a = host_create_from_string("%any", 500);
	b = host_create_from_string("%any", 0);
	ck_assert(!a->equals(a, b));
	ck_assert(a->ip_equals(a, b));
	b->set_port(b, 500);
	ck_assert(a->equals(a, b));
	ck_assert(a->ip_equals(a, b));
	b->destroy(b);
	b = host_create_from_string("%any6", 0);
	ck_assert(a->ip_equals(a, b));
	ck_assert(!a->equals(a, b));
	b->set_port(b, 500);
	ck_assert(a->ip_equals(a, b));
	ck_assert(a->equals(a, b));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

/*******************************************************************************
 * clone
 */

START_TEST(test_clone)
{
	host_t *a, *b;

	a = host_create_from_string("192.168.0.1", 500);
	b = a->clone(a);
	ck_assert(a != b);
	ck_assert(a->equals(a, b));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

/*******************************************************************************
 * printf hook
 */

static struct {
	char *addr;
	uint16_t port;
	/* results for %H, %+H, %#H (falls back to the first entry) */
	char *result[3];
} printf_data[] = {
	{NULL,          0, { "(null)" }},
	{NULL,        500, { "(null)" }},
	{"%any",        0, { "%any", "0.0.0.0", "0.0.0.0" }},
	{"%any",      500, { "%any", "0.0.0.0", "0.0.0.0[500]" }},
	{"%any6",       0, { "%any6", "::", "::" }},
	{"%any6",     500, { "%any6", "::", "::[500]" }},
	{"192.168.0.1",   0, { "192.168.0.1" }},
	{"192.168.0.1", 500, { "192.168.0.1", "192.168.0.1", "192.168.0.1[500]" }},
	{"fec1::1",     0, { "fec1::1" }},
	{"fec1::1",   500, { "fec1::1", "fec1::1", "fec1::1[500]" }},
};

static void verify_printf(host_t *host, const char *format, char *expected)
{
	char buf[64];

	snprintf(buf, sizeof(buf), format, host);
	ck_assert_str_eq(expected, buf);
}

START_TEST(test_printf_hook)
{
	static const char *formats[] = { "%H", "%+H", "%#H" };
	host_t *host = NULL;
	char *expected;
	int i;

	if (printf_data[_i].addr)
	{
		host = host_create_from_string(printf_data[_i].addr,
									   printf_data[_i].port);
	}
	for (i = 0; i < countof(formats); i++)
	{
		expected = printf_data[_i].result[i];
		expected = expected ?: printf_data[_i].result[0];
		verify_printf(host, formats[i], expected);
	}
	DESTROY_IF(host);
}
END_TEST

START_TEST(test_printf_hook_align)
{
	host_t *host;

	verify_printf(NULL, "%14H", "        (null)");
	verify_printf(NULL, "%-14H", "(null)        ");

	host = host_create_from_string("192.168.0.1", 0);
	verify_printf(host, "%14H", "   192.168.0.1");
	verify_printf(host, "%-14H", "192.168.0.1   ");
	verify_printf(host, "%4H", "192.168.0.1");
	verify_printf(host, "%-4H", "192.168.0.1");
	host->destroy(host);
}
END_TEST

Suite *host_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("host");

	tc = tcase_create("host_create_any");
	tcase_add_test(tc, test_create_any_v4);
	tcase_add_test(tc, test_create_any_v6);
	tcase_add_test(tc, test_create_any_other);
	suite_add_tcase(s, tc);

	tc = tcase_create("host_create_from_string");
	tcase_add_test(tc, test_create_from_string_v4);
	tcase_add_test(tc, test_create_from_string_any_v6);
	suite_add_tcase(s, tc);

	tc = tcase_create("host_create_from_string_and_family");
	tcase_add_test(tc, test_create_from_string_and_family_v4);
	tcase_add_test(tc, test_create_from_string_and_family_v6);
	tcase_add_test(tc, test_create_from_string_and_family_other);
	suite_add_tcase(s, tc);

	tc = tcase_create("host_create_from_dns");
	tcase_add_test(tc, test_create_from_dns_any);
	tcase_add_test(tc, test_create_from_dns_v4);
	tcase_add_test(tc, test_create_from_dns_v6);
	suite_add_tcase(s, tc);

	tc = tcase_create("host_create_from_sockaddr");
	tcase_add_test(tc, test_create_from_sockaddr_v4);
	tcase_add_test(tc, test_create_from_sockaddr_v6);
	tcase_add_test(tc, test_create_from_sockaddr_other);
	suite_add_tcase(s, tc);

	tc = tcase_create("host_create_from_chunk");
	tcase_add_test(tc, test_create_from_chunk_v4);
	tcase_add_test(tc, test_create_from_chunk_v6);
	tcase_add_test(tc, test_create_from_chunk_other);
	suite_add_tcase(s, tc);

	tc = tcase_create("host_create_from_subnet");
	tcase_add_test(tc, test_create_from_subnet_v4);
	tcase_add_test(tc, test_create_from_subnet_v6);
	suite_add_tcase(s, tc);

	tc = tcase_create("host_create_from_range");
	tcase_add_test(tc, test_create_from_range_v4);
	tcase_add_test(tc, test_create_from_range_v6);
	suite_add_tcase(s, tc);

	tc = tcase_create("host_create_netmask");
	tcase_add_test(tc, test_create_netmask_v4);
	tcase_add_test(tc, test_create_netmask_v6);
	tcase_add_test(tc, test_create_netmask_other);
	suite_add_tcase(s, tc);

	tc = tcase_create("equals, ip_equals");
	tcase_add_test(tc, test_equals);
	tcase_add_test(tc, test_equals_any);
	suite_add_tcase(s, tc);

	tc = tcase_create("clone");
	tcase_add_test(tc, test_clone);
	suite_add_tcase(s, tc);

	tc = tcase_create("printf hook");
	tcase_add_loop_test(tc, test_printf_hook, 0, countof(printf_data));
	tcase_add_test(tc, test_printf_hook_align);
	suite_add_tcase(s, tc);

	return s;
}
