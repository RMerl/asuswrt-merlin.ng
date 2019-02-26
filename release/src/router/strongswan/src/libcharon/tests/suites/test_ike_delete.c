/*
 * Copyright (C) 2016 Tobias Brunner
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

#include <tests/utils/exchange_test_helper.h>
#include <tests/utils/exchange_test_asserts.h>
#include <tests/utils/sa_asserts.h>

/**
 * Regular IKE_SA delete either initiated by the original initiator or
 * responder of the IKE_SA.
 */
START_TEST(test_regular)
{
	ike_sa_t *a, *b;
	status_t s;

	if (_i)
	{	/* responder deletes the IKE_SA */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, NULL);
	}
	else
	{	/* initiator deletes the IKE_SA */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, NULL);
	}
	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);
	call_ikesa(a, delete, FALSE);
	assert_ike_sa_state(a, IKE_DELETING);
	assert_hook();
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(ike_updown, FALSE);
	assert_hook_updown(child_updown, FALSE);
	assert_single_payload(IN, PLV2_DELETE);
	s = exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(b, destroy);
	assert_hook();
	assert_hook();

	/* <-- INFORMATIONAL { } */
	assert_hook_updown(ike_updown, FALSE);
	assert_hook_updown(child_updown, FALSE);
	assert_message_empty(IN);
	s = exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(a, destroy);
	assert_hook();
	assert_hook();
}
END_TEST

/**
 * Both peers initiate the IKE_SA deletion concurrently and should handle the
 * collision properly.
 */
START_TEST(test_collision)
{
	ike_sa_t *a, *b;
	status_t s;

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);
	call_ikesa(a, delete, FALSE);
	assert_ike_sa_state(a, IKE_DELETING);
	call_ikesa(b, delete, FALSE);
	assert_ike_sa_state(b, IKE_DELETING);
	assert_hook();
	assert_hook();

	/* RFC 7296 says:  If a peer receives a request to close an IKE SA that it
	 * is currently trying to close, it SHOULD reply as usual, and forget about
	 * its own close request.
	 * So we expect the SA to just get closed with an empty response still sent.
	 */

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(ike_updown, FALSE);
	assert_hook_updown(child_updown, FALSE);
	assert_single_payload(IN, PLV2_DELETE);
	assert_message_empty(OUT);
	s = exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(b, destroy);
	assert_hook();
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_updown(ike_updown, FALSE);
	assert_hook_updown(child_updown, FALSE);
	assert_single_payload(IN, PLV2_DELETE);
	assert_message_empty(OUT);
	s = exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(a, destroy);
	assert_hook();
	assert_hook();
}
END_TEST

Suite *ike_delete_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("ike delete");

	tc = tcase_create("regular");
	tcase_add_loop_test(tc, test_regular, 0, 2);
	suite_add_tcase(s, tc);

	tc = tcase_create("collisions");
	tcase_add_test(tc, test_collision);
	suite_add_tcase(s, tc);

	return s;
}
