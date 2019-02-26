/*
 * Copyright (C) 2016 Andreas Steffen
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

#include <newhope_ke.h>

#include <library.h>

#include <time.h>

const int count = 1000;

START_TEST(test_newhope_ke_good)
{
	chunk_t i_msg, r_msg, i_shared_secret, r_shared_secret;
	diffie_hellman_t *i_nh, *r_nh;
	struct timespec start, stop;
	int i;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);

	for (i = 0; i < count; i++)
	{
		i_nh = lib->crypto->create_dh(lib->crypto, NH_128_BIT);
		ck_assert(i_nh != NULL);
		ck_assert(i_nh->get_dh_group(i_nh) == NH_128_BIT);

		ck_assert(i_nh->get_my_public_value(i_nh, &i_msg));
		ck_assert(i_msg.len = 1824);

		r_nh = lib->crypto->create_dh(lib->crypto, NH_128_BIT);
		ck_assert(r_nh != NULL);

		ck_assert(r_nh->set_other_public_value(r_nh, i_msg));
		ck_assert(r_nh->get_my_public_value(r_nh, &r_msg));
		ck_assert(r_msg.len == 2048);

		ck_assert(r_nh->get_shared_secret(r_nh, &r_shared_secret));
		ck_assert(r_shared_secret.len == 32);

		ck_assert(i_nh->set_other_public_value(i_nh, r_msg));
		ck_assert(i_nh->get_shared_secret(i_nh, &i_shared_secret));
		ck_assert(i_shared_secret.len == 32);
		ck_assert(chunk_equals(i_shared_secret, r_shared_secret));

		/* cleanup */
		chunk_clear(&i_shared_secret);
		chunk_clear(&r_shared_secret);
		chunk_free(&i_msg);
		chunk_free(&r_msg);
		i_nh->destroy(i_nh);
		r_nh->destroy(r_nh);
	}

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop);

	DBG0(DBG_LIB, "%d Newhope DH loops in %d ms\n", count,
				  (stop.tv_nsec - start.tv_nsec) / 1000000 +
				  (stop.tv_sec - start.tv_sec) * 1000);
}
END_TEST

START_TEST(test_newhope_ke_wrong)
{
	chunk_t i_msg, r_msg, i_shared_secret, r_shared_secret;
	diffie_hellman_t *i_nh, *r_nh;

	i_nh = lib->crypto->create_dh(lib->crypto, NH_128_BIT);
	ck_assert(i_nh != NULL);
	ck_assert(i_nh->get_my_public_value(i_nh, &i_msg));

	r_nh = lib->crypto->create_dh(lib->crypto, NH_128_BIT);
	ck_assert(r_nh != NULL);
	ck_assert(r_nh->set_other_public_value(r_nh, i_msg));
	ck_assert(r_nh->get_my_public_value(r_nh, &r_msg));

	/* destroy 1st instance of i_nh */
	i_nh->destroy(i_nh);
	chunk_free(&i_msg);

	/* create 2nd instance of i_nh */
	i_nh = lib->crypto->create_dh(lib->crypto, NH_128_BIT);
	ck_assert(i_nh != NULL);
	ck_assert(i_nh->get_my_public_value(i_nh, &i_msg));
	ck_assert(i_nh->set_other_public_value(i_nh, r_msg));

	ck_assert(r_nh->get_shared_secret(r_nh, &r_shared_secret));
	ck_assert(i_nh->get_shared_secret(i_nh, &i_shared_secret));
	ck_assert(!chunk_equals(i_shared_secret, r_shared_secret));

	/* cleanup */
	chunk_clear(&i_shared_secret);
	chunk_clear(&r_shared_secret);
	chunk_free(&i_msg);
	chunk_free(&r_msg);
	i_nh->destroy(i_nh);
	r_nh->destroy(r_nh);
}
END_TEST

START_TEST(test_newhope_ke_fail_i)
{
	diffie_hellman_t *i_nh;
	char buf_ff[2048];
	int i;

	chunk_t i_msg;

	chunk_t r_msg[] = {
		chunk_empty,
		chunk_from_chars(0x00),
		chunk_create(buf_ff, 2047),
		chunk_create(buf_ff, 2048),
	};

	memset(buf_ff, 0xff, sizeof(buf_ff));

		for (i = 0; i < countof(r_msg); i++)
	{
		i_nh = lib->crypto->create_dh(lib->crypto, NH_128_BIT);
		ck_assert(i_nh != NULL);
		ck_assert(i_nh->get_my_public_value(i_nh, &i_msg));
		ck_assert(!i_nh->set_other_public_value(i_nh, r_msg[i]));
		chunk_free(&i_msg);
		i_nh->destroy(i_nh);
	}
}
END_TEST

START_TEST(test_newhope_ke_fail_r)
{
	diffie_hellman_t *r_nh;
	char buf_ff[1824];
	int i;

	chunk_t i_msg[] = {
		chunk_empty,
		chunk_from_chars(0x00),
		chunk_create(buf_ff, 1823),
		chunk_create(buf_ff, 1824),
	};

	memset(buf_ff, 0xff, sizeof(buf_ff));

	for (i = 0; i < countof(i_msg); i++)
	{
		r_nh = lib->crypto->create_dh(lib->crypto, NH_128_BIT);
		ck_assert(r_nh != NULL);
		ck_assert(!r_nh->set_other_public_value(r_nh, i_msg[i]));
		r_nh->destroy(r_nh);
	}
}
END_TEST

Suite *newhope_ke_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("newhope_ke");

	tc = tcase_create("ke_good");
	test_case_set_timeout(tc, 30);
	tcase_add_test(tc, test_newhope_ke_good);
	suite_add_tcase(s, tc);

	tc = tcase_create("ke_wrong");
	tcase_add_test(tc, test_newhope_ke_wrong);
	suite_add_tcase(s, tc);

	tc = tcase_create("ke_fail_i");
	tcase_add_test(tc, test_newhope_ke_fail_i);
	suite_add_tcase(s, tc);

	tc = tcase_create("ke_fail_r");
	tcase_add_test(tc, test_newhope_ke_fail_r);
	suite_add_tcase(s, tc);

	return s;
}
