/*
 * Copyright (C) 2014 Tobias Brunner
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

#include <attributes/mem_pool.h>

static void assert_host(char *expected, host_t *host)
{
	if (!expected)
	{
		ck_assert_msg(!host, "not epxecting IP != %+H", host);
	}
	else
	{
		host_t *verifier;
		verifier = host_create_from_string(expected, 0);
		ck_assert_msg(host, "expected IP %+H != NULL", verifier);
		ck_assert_msg(verifier->ip_equals(verifier, host), "expected IP %+H != "
					  "%+H", verifier, host);;
		verifier->destroy(verifier);
	}
}

static void assert_acquire(mem_pool_t *pool, char *requested, char *expected,
						   mem_pool_op_t operation)
{
	identification_t *id;
	host_t *req, *acquired;

	id = identification_create_from_string("tester");
	req = host_create_from_string(requested, 0);

	acquired = pool->acquire_address(pool, id, req, operation, NULL);
	assert_host(expected, acquired);
	DESTROY_IF(acquired);

	req->destroy(req);
	id->destroy(id);
}

static void assert_acquires_new(mem_pool_t *pool, char *pattern, int first)
{
	char expected[16];
	int i;

	for (i = 0; i < pool->get_size(pool); i++)
	{
		snprintf(expected, sizeof(expected), pattern, first + i);
		assert_acquire(pool, "0.0.0.0", expected, MEM_POOL_NEW);
		ck_assert_int_eq(i + 1, pool->get_online(pool));
	}
	assert_acquire(pool, "0.0.0.0", NULL, MEM_POOL_NEW);
}

START_TEST(test_config)
{
	mem_pool_t *pool;

	pool = mem_pool_create("test", NULL, 0);
	ck_assert_int_eq(0, pool->get_size(pool));
	assert_acquire(pool, "192.168.0.1", "192.168.0.1", MEM_POOL_NEW);
	assert_acquire(pool, "10.0.1.1", "10.0.1.1", MEM_POOL_NEW);
	assert_acquire(pool, "0.0.0.0", "0.0.0.0", MEM_POOL_NEW);
	assert_acquire(pool, "255.255.255.255", "255.255.255.255", MEM_POOL_NEW);
	ck_assert_int_eq(0, pool->get_online(pool));
	pool->destroy(pool);
}
END_TEST

START_TEST(test_cidr)
{
	mem_pool_t *pool;
	host_t *base;

	base = host_create_from_string("192.168.0.0", 0);

	pool = mem_pool_create("test", base, 32);
	ck_assert_int_eq(1, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 0);
	pool->destroy(pool);

	pool = mem_pool_create("test", base, 31);
	ck_assert_int_eq(2, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 0);
	pool->destroy(pool);

	pool = mem_pool_create("test", base, 30);
	ck_assert_int_eq(2, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 1);
	pool->destroy(pool);

	pool = mem_pool_create("test", base, 29);
	ck_assert_int_eq(6, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 1);
	pool->destroy(pool);

	pool = mem_pool_create("test", base, 24);
	ck_assert_int_eq(254, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 1);
	pool->destroy(pool);

	base->destroy(base);
}
END_TEST

START_TEST(test_cidr_offset)
{
	mem_pool_t *pool;
	host_t *base;

	base = host_create_from_string("192.168.0.1", 0);
	pool = mem_pool_create("test", base, 31);
	ck_assert_int_eq(1, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 1);
	pool->destroy(pool);

	pool = mem_pool_create("test", base, 30);
	ck_assert_int_eq(2, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 1);
	pool->destroy(pool);
	base->destroy(base);

	base = host_create_from_string("192.168.0.2", 0);
	pool = mem_pool_create("test", base, 30);
	ck_assert_int_eq(1, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 2);
	pool->destroy(pool);

	pool = mem_pool_create("test", base, 24);
	ck_assert_int_eq(253, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 2);
	pool->destroy(pool);
	base->destroy(base);

	base = host_create_from_string("192.168.0.254", 0);
	pool = mem_pool_create("test", base, 24);
	ck_assert_int_eq(1, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 254);
	pool->destroy(pool);
	base->destroy(base);

	/* due to size == 0 we get the requested IP back */
	base = host_create_from_string("192.168.0.255", 0);
	pool = mem_pool_create("test", base, 24);
	ck_assert_int_eq(0, pool->get_size(pool));
	assert_acquire(pool, "192.168.0.1", "192.168.0.1", MEM_POOL_NEW);
	pool->destroy(pool);

	base->destroy(base);
}
END_TEST

START_TEST(test_range)
{
	mem_pool_t *pool;
	host_t *from, *to;

	from = host_create_from_string("192.168.0.0", 0);
	to = host_create_from_string("192.168.0.0", 0);
	pool = mem_pool_create_range("test", from, to);
	ck_assert_int_eq(1, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 0);
	pool->destroy(pool);

	to->destroy(to);
	to = host_create_from_string("192.168.0.1", 0);
	pool = mem_pool_create_range("test", from, to);
	ck_assert_int_eq(2, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 0);
	pool->destroy(pool);

	from->destroy(from);
	from = host_create_from_string("192.168.0.10", 0);
	pool = mem_pool_create_range("test", from, to);
	ck_assert(!pool);

	to->destroy(to);
	to = host_create_from_string("192.168.0.20", 0);
	pool = mem_pool_create_range("test", from, to);
	ck_assert_int_eq(11, pool->get_size(pool));
	assert_acquires_new(pool, "192.168.0.%d", 10);
	pool->destroy(pool);

	from->destroy(from);
	from = host_create_from_string("fec::1", 0);
	to->destroy(to);
	to = host_create_from_string("fed::1", 0);
	pool = mem_pool_create_range("test", from, to);
	ck_assert(!pool);

	from->destroy(from);
	to->destroy(to);
}
END_TEST

Suite *mem_pool_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("mem_pool");

	tc = tcase_create("%config-like pool");
	tcase_add_test(tc, test_config);
	suite_add_tcase(s, tc);

	tc = tcase_create("cidr constructor");
	tcase_add_test(tc, test_cidr);
	tcase_add_test(tc, test_cidr_offset);
	suite_add_tcase(s, tc);

	tc = tcase_create("range constructor");
	tcase_add_test(tc, test_range);
	suite_add_tcase(s, tc);

	return s;
}
