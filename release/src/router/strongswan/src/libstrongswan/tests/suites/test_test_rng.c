/*
 * Copyright (C) 2013 Andreas Steffen
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

#include <tests/utils/test_rng.h>
#include <utils/test.h>

START_TEST(test_test_rng)
{
	rng_t *entropy;
	chunk_t in, in1, in2, out;

	in1 = chunk_from_chars(0x01, 0x02, 0x03, 0x04, 0x05, 0x06);
	in2 = chunk_from_chars(0x07, 0x08);
	in = chunk_cat("cc", in1, in2);

	entropy = test_rng_create(in);
	ck_assert(entropy->allocate_bytes(entropy, 6, &out));
	ck_assert(chunk_equals(in1, out));
	ck_assert(entropy->get_bytes(entropy, 2, out.ptr));
	ck_assert(memeq(in2.ptr, out.ptr, in2.len));
	ck_assert(!entropy->get_bytes(entropy, 4, out.ptr));
	chunk_free(&out);
	ck_assert(!entropy->allocate_bytes(entropy, 4, &out));
	entropy->destroy(entropy);
	chunk_free(&in);
}
END_TEST


Suite *test_rng_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("test_rng");

	tc = tcase_create("test_rng");
	tcase_add_test(tc, test_test_rng);
	suite_add_tcase(s, tc);

	return s;
}
