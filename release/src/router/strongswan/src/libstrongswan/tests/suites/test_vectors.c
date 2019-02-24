/*
 * Copyright (C) 2014 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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
#include <threading/thread.h>
#include <crypto/transform.h>

static transform_type_t tfs[] = {
	ENCRYPTION_ALGORITHM,
	AEAD_ALGORITHM,
	INTEGRITY_ALGORITHM,
	HASH_ALGORITHM,
	PSEUDO_RANDOM_FUNCTION,
	RANDOM_NUMBER_GENERATOR,
	DIFFIE_HELLMAN_GROUP,
};

START_TEST(test_vectors)
{
	enumerator_t *enumerator;
	char *plugin;
	bool success;
	u_int alg;

	enumerator = lib->crypto->create_verify_enumerator(lib->crypto, tfs[_i]);
	thread_cleanup_push((void*)enumerator->destroy, enumerator);
	while (enumerator->enumerate(enumerator, &alg, &plugin, &success))
	{
		ck_assert_msg(success, "test vector for %N from '%s' plugin failed",
					  transform_get_enum_names(tfs[_i]), alg, plugin);
	}
	thread_cleanup_pop(TRUE);
}
END_TEST


Suite *vectors_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("vectors");

	tc = tcase_create("transforms");
	tcase_add_loop_test(tc, test_vectors, 0, countof(tfs));
	tcase_set_timeout(tc, 20);
	suite_add_tcase(s, tc);

	return s;
}
