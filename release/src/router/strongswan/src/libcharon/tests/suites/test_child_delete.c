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

#include <daemon.h>
#include <tests/utils/exchange_test_helper.h>
#include <tests/utils/exchange_test_asserts.h>
#include <tests/utils/job_asserts.h>
#include <tests/utils/sa_asserts.h>

/**
 * Regular CHILD_SA deletion either initiated by the original initiator or
 * responder of the IKE_SA.
 */
START_TEST(test_regular)
{
	ike_sa_t *a, *b;

	if (_i)
	{	/* responder deletes the CHILD_SA (SPI 2) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, NULL);
	}
	else
	{	/* initiator deletes the CHILD_SA (SPI 1) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, NULL);
	}
	assert_hook_not_called(child_updown);
	call_ikesa(a, delete_child_sa, PROTO_ESP, _i+1, FALSE);
	assert_child_sa_state(a, _i+1, CHILD_DELETING);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(child_updown, FALSE);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_count(b, 0);
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_updown(child_updown, FALSE);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 0);
	assert_hook();

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Both peers initiate the CHILD_SA deletion concurrently and should handle
 * the collision properly.
 */
START_TEST(test_collision)
{
	ike_sa_t *a, *b;

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);
	/* both peers delete the CHILD_SA concurrently */
	assert_hook_not_called(child_updown);
	call_ikesa(a, delete_child_sa, PROTO_ESP, 1, FALSE);
	assert_child_sa_state(a, 1, CHILD_DELETING);
	call_ikesa(b, delete_child_sa, PROTO_ESP, 2, FALSE);
	assert_child_sa_state(b, 2, CHILD_DELETING);
	assert_hook();

	/* RFC 7296 says:
	 *
	 *   Normally, the response in the INFORMATIONAL exchange will contain
	 *   Delete payloads for the paired SAs going in the other direction.
	 *   There is one exception.  If, by chance, both ends of a set of SAs
	 *   independently decide to close them, each may send a Delete payload
	 *   and the two requests may cross in the network.  If a node receives a
	 *   delete request for SAs for which it has already issued a delete
	 *   request, it MUST delete the outgoing SAs while processing the request
	 *   and the incoming SAs while processing the response.  In that case,
	 *   the responses MUST NOT include Delete payloads for the deleted SAs,
	 *   since that would result in duplicate deletion and could in theory
	 *   delete the wrong SA.
	 *
	 * We don't handle SAs separately so we expect both are still installed,
	 * but the INFORMATIONAL response should not contain a DELETE payload.
	 */

	/* INFORMATIONAL { D } --> */
	assert_hook_not_called(child_updown);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETING);
	/* <-- INFORMATIONAL { D } */
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETING);
	assert_hook();

	/* <-- INFORMATIONAL { } */
	assert_hook_updown(child_updown, FALSE);
	assert_message_empty(IN);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 0);
	assert_hook();
	/* INFORMATIONAL { } --> */
	assert_hook_updown(child_updown, FALSE);
	assert_message_empty(IN);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_count(b, 0);
	assert_hook();

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * This is like the collision above but one of the DELETEs is dropped or delayed
 * so the other peer is not aware that there is a collision.
 */
START_TEST(test_collision_drop)
{
	ike_sa_t *a, *b;
	message_t *msg;

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);
	/* both peers delete the CHILD_SA concurrently */
	assert_hook_not_called(child_updown);
	call_ikesa(a, delete_child_sa, PROTO_ESP, 1, FALSE);
	assert_child_sa_state(a, 1, CHILD_DELETING);
	call_ikesa(b, delete_child_sa, PROTO_ESP, 2, FALSE);
	assert_child_sa_state(b, 2, CHILD_DELETING);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_not_called(child_updown);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETING);
	assert_hook();

	/* drop/delay the responder's message */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* <-- INFORMATIONAL { } */
	assert_hook_updown(child_updown, FALSE);
	assert_message_empty(IN);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 0);
	assert_hook();

	/* <-- INFORMATIONAL { D } (delayed/retransmitted) */
	assert_hook_not_called(child_updown);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, msg);
	assert_hook();

	/* INFORMATIONAL { } --> */
	assert_hook_updown(child_updown, FALSE);
	assert_message_empty(IN);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_count(b, 0);
	assert_hook();

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * One of the hosts initiates a rekey of the IKE_SA of the CHILD_SA the other
 * peer is concurrently trying to delete.
 *
 *           delete ----\       /---- rekey IKE
 *                       \-----/----> detect collision
 * detect collision <---------/ /---- delete
 *        TEMP_FAIL ----\      /
 *                       \----/----->
 *                  <--------/
 */
START_TEST(test_collision_ike_rekey)
{
	ike_sa_t *a, *b;
	uint32_t spi_a = _i+1;

	if (_i)
	{	/* responder deletes the CHILD_SA (SPI 2) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, NULL);
	}
	else
	{	/* initiator deletes the CHILD_SA (SPI 1) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, NULL);
	}
	call_ikesa(a, delete_child_sa, PROTO_ESP, spi_a, FALSE);
	assert_child_sa_state(a, spi_a, CHILD_DELETING);
	call_ikesa(b, rekey);
	assert_ike_sa_state(b, IKE_REKEYING);

	/* this should never get called as there is no successful rekeying */
	assert_hook_not_called(ike_rekey);

	/* RFC 7296, 2.25.2: If a peer receives a request to delete a Child SA when
	 * it is currently rekeying the IKE SA, it SHOULD reply as usual, with a
	 * Delete payload.
	 */

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(child_updown, FALSE);
	assert_single_payload(OUT, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_REKEYING);
	assert_child_sa_count(b, 0);
	assert_hook();

	/* RFC 7296, 2.25.1: If a peer receives a request to rekey the IKE SA, and
	 * it is currently, rekeying, or closing a Child SA of that IKE SA, it
	 * SHOULD reply with TEMPORARY_FAILURE.
	 */

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETING);

	/* <-- INFORMATIONAL { D } */
	assert_hook_updown(child_updown, FALSE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 0);
	assert_hook();

	/* CREATE_CHILD_SA { N(TEMP_FAIL) } --> */
	/* we expect a job to retry the rekeying is scheduled */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_ESTABLISHED);
	assert_scheduler();

	/* ike_rekey */
	assert_hook();

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * One of the hosts initiates a delete of the IKE_SA of the CHILD_SA the other
 * peer is concurrently trying to delete.
 *
 *           delete ----\       /---- delete IKE
 *                       \-----/----> detect collision
 *                  <---------/ /---- delete
 *           delete ----\      /
 *                       \----/----->
 *  sa already gone <--------/
 */
START_TEST(test_collision_ike_delete)
{
	ike_sa_t *a, *b;
	uint32_t spi_a = _i+1;
	message_t *msg;
	status_t s;

	if (_i)
	{	/* responder rekeys the CHILD_SA (SPI 2) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, NULL);
	}
	else
	{	/* initiator rekeys the CHILD_SA (SPI 1) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, NULL);
	}
	call_ikesa(a, delete_child_sa, PROTO_ESP, spi_a, FALSE);
	assert_child_sa_state(a, spi_a, CHILD_DELETING);
	call_ikesa(b, delete, FALSE);
	assert_ike_sa_state(b, IKE_DELETING);

	/* RFC 7296, 2.25.2 does not explicitly state what the behavior SHOULD be if
	 * a peer receives a request to delete a CHILD_SA when it is currently
	 * closing the IKE SA.  We expect a regular response.
	 */

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(child_updown, FALSE);
	assert_single_payload(OUT, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_DELETING);
	assert_child_sa_count(b, 0);
	assert_hook();

	/* RFC 7296, 2.25.1 does not explicitly state what the behavior SHOULD be if
	 * a peer receives a request to close the IKE SA if it is currently deleting
	 * a Child SA of that IKE SA.  Let's just close the IKE_SA and forget the
	 * delete.
	 */

	/* <-- INFORMATIONAL { D } */
	assert_hook_updown(ike_updown, FALSE);
	assert_hook_updown(child_updown, FALSE);
	assert_message_empty(OUT);
	s = exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(a, destroy);
	assert_hook();
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	/* the SA is already gone */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);
	msg->destroy(msg);

	/* INFORMATIONAL { } --> */
	assert_hook_updown(ike_updown, FALSE);
	assert_hook_not_called(child_updown);
	s = exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(b, destroy);
	assert_hook();
	assert_hook();
}
END_TEST

Suite *child_delete_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("child delete");

	tc = tcase_create("regular");
	tcase_add_loop_test(tc, test_regular, 0, 2);
	suite_add_tcase(s, tc);

	tc = tcase_create("collisions");
	tcase_add_test(tc, test_collision);
	tcase_add_test(tc, test_collision_drop);
	suite_add_tcase(s, tc);

	tc = tcase_create("collisions ike rekey");
	tcase_add_loop_test(tc, test_collision_ike_rekey, 0, 2);
	suite_add_tcase(s, tc);

	tc = tcase_create("collisions ike delete");
	tcase_add_loop_test(tc, test_collision_ike_delete, 0, 2);
	suite_add_tcase(s, tc);

	return s;
}
