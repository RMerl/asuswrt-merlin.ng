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

#include <tests/test_suite.h>

#include "tkm_id_manager.h"

static const tkm_limits_t limits = {125, 100, 55, 30, 200, 42};

START_TEST(test_id_mgr_creation)
{
	tkm_id_manager_t *idmgr = NULL;

	idmgr = tkm_id_manager_create(limits);
	fail_if(idmgr == NULL, "Error creating tkm id manager");

	idmgr->destroy(idmgr);
}
END_TEST

START_TEST(test_acquire_id)
{
	int i, id = 0;
	tkm_id_manager_t *idmgr = tkm_id_manager_create(limits);

	for (i = 0; i < TKM_CTX_MAX; i++)
	{
		id = idmgr->acquire_id(idmgr, i);
		fail_unless(id > 0, "Error acquiring id of context kind %d", i);

		/* Reset test variable */
		id = 0;
	}

	idmgr->destroy(idmgr);
}
END_TEST

START_TEST(test_acquire_id_invalid_kind)
{
	int id = 0;
	tkm_id_manager_t *idmgr = tkm_id_manager_create(limits);

	id = idmgr->acquire_id(idmgr, TKM_CTX_MAX);
	fail_unless(id == 0, "Acquired id for invalid context kind %d", TKM_CTX_MAX);

	/* Reset test variable */
	id = 0;

	id = idmgr->acquire_id(idmgr, -1);
	fail_unless(id == 0, "Acquired id for invalid context kind %d", -1);

	idmgr->destroy(idmgr);
}
END_TEST

START_TEST(test_acquire_id_same)
{
	int id1 = 0, id2 = 0;
	tkm_id_manager_t *idmgr = tkm_id_manager_create(limits);

	id1 = idmgr->acquire_id(idmgr, TKM_CTX_NONCE);
	fail_unless(id1 > 0, "Unable to acquire first id");

	/* Acquire another id, must be different than first */
	id2 = idmgr->acquire_id(idmgr, TKM_CTX_NONCE);
	fail_unless(id2 > 0, "Unable to acquire second id");
	fail_unless(id1 != id2, "Same id received twice");

	idmgr->destroy(idmgr);
}
END_TEST

START_TEST(test_release_id)
{
	int i, id = 0;
	bool released = false;
	tkm_id_manager_t *idmgr = tkm_id_manager_create(limits);

	for (i = 0; i < TKM_CTX_MAX; i++)
	{
		id = idmgr->acquire_id(idmgr, i);
		released = idmgr->release_id(idmgr, i, id);

		fail_unless(released, "Error releasing id of context kind %d", i);

		/* Reset released variable */
		released = FALSE;
	}

	idmgr->destroy(idmgr);
}
END_TEST

START_TEST(test_release_id_invalid_kind)
{
	bool released = TRUE;
	tkm_id_manager_t *idmgr = tkm_id_manager_create(limits);

	released = idmgr->release_id(idmgr, TKM_CTX_MAX, 1);
	fail_if(released, "Released id for invalid context kind %d", TKM_CTX_MAX);

	/* Reset test variable */
	released = TRUE;

	released = idmgr->release_id(idmgr, -1, 1);
	fail_if(released, "Released id for invalid context kind %d", -1);

	idmgr->destroy(idmgr);
}
END_TEST

START_TEST(test_release_id_nonexistent)
{
	bool released = FALSE;
	tkm_id_manager_t *idmgr = tkm_id_manager_create(limits);

	released = idmgr->release_id(idmgr, TKM_CTX_NONCE, 1);
	fail_unless(released, "Release of nonexistent id failed");

	idmgr->destroy(idmgr);
}
END_TEST

Suite *make_id_manager_tests()
{
	Suite *s;
	TCase *tc;

	s = suite_create("context id manager");

	tc = tcase_create("creation");
	tcase_add_test(tc, test_id_mgr_creation);
	suite_add_tcase(s, tc);

	tc = tcase_create("acquire");
	tcase_add_test(tc, test_acquire_id);
	tcase_add_test(tc, test_acquire_id_invalid_kind);
	tcase_add_test(tc, test_acquire_id_same);
	suite_add_tcase(s, tc);

	tc = tcase_create("release");
	tcase_add_test(tc, test_release_id);
	tcase_add_test(tc, test_release_id_invalid_kind);
	tcase_add_test(tc, test_release_id_nonexistent);
	suite_add_tcase(s, tc);

	return s;
}
