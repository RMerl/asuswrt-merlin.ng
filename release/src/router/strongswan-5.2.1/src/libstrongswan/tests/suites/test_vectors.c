/*
 * Copyright (C) 2014 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
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

#include <utils/test.h>

IMPORT_FUNCTION_FOR_TESTS(crypto, verify_registered_algorithms, u_int,
						  crypto_factory_t *factory);

START_TEST(test_vectors)
{
	u_int failed = TEST_FUNCTION(crypto, verify_registered_algorithms,
								 lib->crypto);
	fail_if(failed > 0, "%u test vectors failed", failed);
}
END_TEST


Suite *vectors_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("vectors");

	tc = tcase_create("failures");
	tcase_add_test(tc, test_vectors);
	suite_add_tcase(s, tc);

	return s;
}
