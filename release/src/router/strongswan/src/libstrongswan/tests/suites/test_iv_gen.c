/*
 * Copyright (C) 2015 Tobias Brunner
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

#include <crypto/iv/iv_gen_seq.h>
#include <utils/test.h>

START_TEST(test_iv_gen_seq)
{
	iv_gen_t *iv_gen;
	uint64_t iv0, iv1_1, iv1_2;

	iv_gen = iv_gen_seq_create();
	ck_assert(iv_gen->get_iv(iv_gen, 0, 8, (uint8_t*)&iv0));
	ck_assert(iv_gen->get_iv(iv_gen, 1, 8, (uint8_t*)&iv1_1));
	ck_assert(iv0 != iv1_1);
	/* every sequence number may be used twice, but results in a different IV */
	ck_assert(iv_gen->get_iv(iv_gen, 1, 8, (uint8_t*)&iv1_2));
	ck_assert(iv0 != iv1_2);
	ck_assert(iv1_1 != iv1_2);
	ck_assert(!iv_gen->get_iv(iv_gen, 1, 8, (uint8_t*)&iv1_2));
	iv_gen->destroy(iv_gen);
}
END_TEST

START_TEST(test_iv_gen_seq_len)
{
	iv_gen_t *iv_gen;
	uint64_t iv;
	uint8_t buf[9];

	iv_gen = iv_gen_seq_create();
	ck_assert(!iv_gen->get_iv(iv_gen, 0, 0, (uint8_t*)&iv));
	ck_assert(!iv_gen->get_iv(iv_gen, 0, 1, (uint8_t*)&iv));
	ck_assert(!iv_gen->get_iv(iv_gen, 0, 2, (uint8_t*)&iv));
	ck_assert(!iv_gen->get_iv(iv_gen, 0, 3, (uint8_t*)&iv));
	ck_assert(!iv_gen->get_iv(iv_gen, 0, 4, (uint8_t*)&iv));
	ck_assert(!iv_gen->get_iv(iv_gen, 0, 5, (uint8_t*)&iv));
	ck_assert(!iv_gen->get_iv(iv_gen, 0, 6, (uint8_t*)&iv));
	ck_assert(!iv_gen->get_iv(iv_gen, 0, 7, (uint8_t*)&iv));
	ck_assert(iv_gen->get_iv(iv_gen, 0, 8, (uint8_t*)&iv));
	ck_assert(iv_gen->get_iv(iv_gen, 0, 9, buf));
	iv_gen->destroy(iv_gen);
}
END_TEST

Suite *iv_gen_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("iv-gen");

	tc = tcase_create("iv-gen-seq");
	tcase_add_test(tc, test_iv_gen_seq);
	tcase_add_test(tc, test_iv_gen_seq_len);
	suite_add_tcase(s, tc);

	return s;
}
