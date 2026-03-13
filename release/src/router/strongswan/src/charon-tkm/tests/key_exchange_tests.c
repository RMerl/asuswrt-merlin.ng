/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 *
 * Copyright (C) secunet Security Networks AG
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

#include <daemon.h>
#include <tests/test_suite.h>

#include "tkm_key_exchange.h"

START_TEST(test_ke_creation)
{
	tkm_key_exchange_t *ke = NULL;

	ke = tkm_key_exchange_create(MODP_768_BIT);
	fail_if(ke, "MODP_768 created");

	ke = tkm_key_exchange_create(MODP_4096_BIT);
	fail_if(!ke, "MODP_4096 not created");
	fail_if(!ke->get_id(ke), "Invalid context id (0)");

	ke->ke.destroy(&ke->ke);
}
END_TEST

START_TEST(test_ke_get_my_pubvalue)
{
	tkm_key_exchange_t *ke = tkm_key_exchange_create(MODP_4096_BIT);
	fail_if(!ke, "Unable to create KE");

	chunk_t value;
	ck_assert(ke->ke.get_public_key(&ke->ke, &value));
	ke->ke.destroy(&ke->ke);

	fail_if(value.ptr == NULL, "Pubvalue is NULL");
	fail_if(value.len != 512, "Pubvalue size mismatch");

	chunk_free(&value);
}
END_TEST

Suite *make_key_exchange_tests()
{
	Suite *s;
	TCase *tc;

	s = suite_create("key exchange");

	tc = tcase_create("creation");
	tcase_add_test(tc, test_ke_creation);
	suite_add_tcase(s, tc);

	tc = tcase_create("get_my_pubvalue");
	tcase_add_test(tc, test_ke_get_my_pubvalue);
	suite_add_tcase(s, tc);

	return s;
}
