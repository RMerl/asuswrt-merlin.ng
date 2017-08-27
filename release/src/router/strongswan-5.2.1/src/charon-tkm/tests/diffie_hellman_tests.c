/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 * Hochschule fuer Technik Rapperswil
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

#include "tkm_diffie_hellman.h"

START_TEST(test_dh_creation)
{
	tkm_diffie_hellman_t *dh = NULL;

	dh = tkm_diffie_hellman_create(MODP_768_BIT);
	fail_if(dh, "MODP_768 created");

	dh = tkm_diffie_hellman_create(MODP_4096_BIT);
	fail_if(!dh, "MODP_4096 not created");
	fail_if(!dh->get_id(dh), "Invalid context id (0)");

	dh->dh.destroy(&dh->dh);
}
END_TEST

START_TEST(test_dh_get_my_pubvalue)
{
	tkm_diffie_hellman_t *dh = tkm_diffie_hellman_create(MODP_4096_BIT);
	fail_if(!dh, "Unable to create DH");

	chunk_t value;
	dh->dh.get_my_public_value(&dh->dh, &value);
	dh->dh.destroy(&dh->dh);

	fail_if(value.ptr == NULL, "Pubvalue is NULL");
	fail_if(value.len != 512, "Pubvalue size mismatch");

	chunk_free(&value);
}
END_TEST

Suite *make_diffie_hellman_tests()
{
	Suite *s;
	TCase *tc;

	s = suite_create("Diffie-Hellman");

	tc = tcase_create("creation");
	tcase_add_test(tc, test_dh_creation);
	suite_add_tcase(s, tc);

	tc = tcase_create("get_my_pubvalue");
	tcase_add_test(tc, test_dh_get_my_pubvalue);
	suite_add_tcase(s, tc);

	return s;
}
