/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <tests/test_suite.h>

#include <tkm/client.h>

#include "tkm.h"
#include "tkm_nonceg.h"

START_TEST(test_nonceg_creation)
{
	tkm_nonceg_t *ng = NULL;

	ng = tkm_nonceg_create();
	fail_if(ng == NULL, "Error creating tkm nonce generator");

	ng->nonce_gen.destroy(&ng->nonce_gen);
}
END_TEST

START_TEST(test_nonceg_allocate_nonce)
{
	tkm_nonceg_t *ng = tkm_nonceg_create();

	const size_t length = 256;
	uint8_t zero[length];
	memset(zero, 0, length);

	chunk_t nonce;
	const bool got_nonce = ng->nonce_gen.allocate_nonce(&ng->nonce_gen,
			length, &nonce);

	fail_unless(got_nonce, "Call to allocate_nonce failed");
	fail_unless(nonce.len = length, "Allocated nonce length mismatch");
	fail_if(memcmp(nonce.ptr, zero, length) == 0, "Unable to allocate nonce");

	tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_NONCE, 1);
	ike_nc_reset(1);

	chunk_free(&nonce);
	ng->nonce_gen.destroy(&ng->nonce_gen);
}
END_TEST

START_TEST(test_nonceg_get_nonce)
{
	tkm_nonceg_t *ng = tkm_nonceg_create();

	const size_t length = 128;
	uint8_t zero[length];
	memset(zero, 0, length);

	uint8_t *buf = malloc(length + 1);
	memset(buf, 0, length);
	/* set end marker */
	buf[length] = 255;

	const bool got_nonce = ng->nonce_gen.get_nonce(&ng->nonce_gen, length, buf);
	fail_unless(got_nonce, "Call to get_nonce failed");
	fail_if(memcmp(buf, zero, length) == 0, "Unable to get nonce");
	fail_if(buf[length] != 255, "End marker not found");

	tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_NONCE, 1);
	ike_nc_reset(1);

	free(buf);
	ng->nonce_gen.destroy(&ng->nonce_gen);
}
END_TEST

Suite *make_nonceg_tests()
{
	Suite *s;
	TCase *tc;

	s = suite_create("nonce generator");

	tc = tcase_create("creation");
	tcase_add_test(tc, test_nonceg_creation);
	suite_add_tcase(s, tc);

	tc = tcase_create("allocate");
	tcase_add_test(tc, test_nonceg_allocate_nonce);
	suite_add_tcase(s, tc);

	tc = tcase_create("get");
	tcase_add_test(tc, test_nonceg_get_nonce);
	suite_add_tcase(s, tc);

	return s;
}
