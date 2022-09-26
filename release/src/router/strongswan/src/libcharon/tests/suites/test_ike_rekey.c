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
#include <tests/utils/job_asserts.h>
#include <tests/utils/sa_asserts.h>

/**
 * Initiate rekeying the given IKE_SA.
 */
#define initiate_rekey(sa) ({ \
	assert_hook_not_called(ike_rekey); \
	call_ikesa(sa, rekey); \
	assert_ike_sa_state(a, IKE_REKEYING); \
	assert_hook(); \
})

/**
 * Regular IKE_SA rekeying either initiated by the original initiator or
 * responder of the IKE_SA.
 */
START_TEST(test_regular)
{
	ike_sa_t *a, *b, *new_sa;
	status_t s;

	if (_i)
	{	/* responder rekeys the IKE_SA */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, NULL);
	}
	else
	{	/* initiator rekeys the IKE_SA */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, NULL);
	}
	/* these should never get called as this results in a successful rekeying */
	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);

	initiate_rekey(a);

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	assert_hook_rekey(ike_rekey, 1, 3);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_REKEYED);
	assert_child_sa_count(b, 0);
	new_sa = assert_ike_sa_checkout(3, 4, FALSE);
	assert_ike_sa_state(new_sa, IKE_ESTABLISHED);
	assert_child_sa_count(new_sa, 1);
	assert_ike_sa_count(1);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, KEr } */
	assert_hook_rekey(ike_rekey, 1, 3);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_DELETING);
	assert_child_sa_count(a, 0);
	new_sa = assert_ike_sa_checkout(3, 4, TRUE);
	assert_ike_sa_state(new_sa, IKE_ESTABLISHED);
	assert_child_sa_count(new_sa, 1);
	assert_ike_sa_count(2);
	assert_hook();

	/* we don't expect this hook to get called anymore */
	assert_hook_not_called(ike_rekey);

	/* INFORMATIONAL { D } --> */
	assert_single_payload(IN, PLV2_DELETE);
	s = exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(b, destroy);
	/* <-- INFORMATIONAL { } */
	assert_message_empty(IN);
	s = exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(a, destroy);

	/* ike_rekey/ike_updown/child_updown */
	assert_hook();
	assert_hook();
	assert_hook();

	charon->ike_sa_manager->flush(charon->ike_sa_manager);
}
END_TEST

/**
 * IKE_SA rekeying where the responder does not agree with the DH group selected
 * by the initiator, either initiated by the original initiator or responder of
 * the IKE_SA.
 */
START_TEST(test_regular_ke_invalid)
{
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.ike = "aes128-sha256-modp2048-modp3072",
		},
		.responder = {
			.ike = "aes128-sha256-modp3072-modp2048",
		},
	};
	ike_sa_t *a, *b, *sa;
	status_t s;

	lib->settings->set_bool(lib->settings, "%s.prefer_configured_proposals",
							FALSE, lib->ns);
	if (_i)
	{	/* responder rekeys the IKE_SA */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, &conf);
	}
	else
	{	/* initiator rekeys the IKE_SA */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, &conf);
	}
	/* these should never get called as this results in a successful rekeying */
	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);

	lib->settings->set_bool(lib->settings, "%s.prefer_configured_proposals",
							TRUE, lib->ns);
	lib->settings->set_bool(lib->settings, "%s.prefer_previous_dh_group",
							FALSE, lib->ns);

	initiate_rekey(a);

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_ESTABLISHED);
	assert_child_sa_count(b, 1);
	assert_ike_sa_count(0);

	/* <-- CREATE_CHILD_SA { N(INVAL_KE) } */
	assert_single_notify(IN, INVALID_KE_PAYLOAD);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	assert_hook_rekey(ike_rekey, 1, 3);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_REKEYED);
	assert_child_sa_count(b, 0);
	sa = assert_ike_sa_checkout(3, 5, FALSE);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(1);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, KEr } */
	assert_hook_rekey(ike_rekey, 1, 3);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_DELETING);
	assert_child_sa_count(a, 0);
	sa = assert_ike_sa_checkout(3, 5, TRUE);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(2);
	assert_hook();

	/* we don't expect this hook to get called anymore */
	assert_hook_not_called(ike_rekey);

	/* INFORMATIONAL { D } --> */
	assert_single_payload(IN, PLV2_DELETE);
	s = exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(b, destroy);
	/* <-- INFORMATIONAL { } */
	assert_message_empty(IN);
	s = exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(a, destroy);

	/* ike_rekey/ike_updown/child_updown */
	assert_hook();
	assert_hook();
	assert_hook();

	charon->ike_sa_manager->flush(charon->ike_sa_manager);
}
END_TEST

/**
 * Both peers initiate the IKE_SA rekeying concurrently and should handle the
 * collision properly depending on the nonces.
 */
START_TEST(test_collision)
{
	ike_sa_t *a, *b, *sa;
	status_t status;

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	/* When rekeyings collide we get two IKE_SAs with a total of four nonces.
	 * The IKE_SA with the lowest nonce SHOULD be deleted by the peer that
	 * created that IKE_SA.  The replaced IKE_SA is deleted by the peer that
	 * initiated the surviving SA.
	 * Four nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * IKE_SA):
	 *   N1/3 -----\    /----- N2/4
	 *              \--/-----> N3/5
	 *   N4/6 <-------/ /----- ...
	 *   ...  -----\
	 * We test this four times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[4];
		/* SPIs of the deleted IKE_SAs (either redundant or replaced) */
		uint32_t del_a_i, del_a_r;
		uint32_t del_b_i, del_b_r;
		/* SPIs of the kept IKE_SA */
		uint32_t spi_i, spi_r;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF, 0xFF }, 3, 5, 1, 2, 4, 6 },
		{ { 0xFF, 0x00, 0xFF, 0xFF }, 1, 2, 4, 6, 3, 5 },
		{ { 0xFF, 0xFF, 0x00, 0xFF }, 3, 5, 1, 2, 4, 6 },
		{ { 0xFF, 0xFF, 0xFF, 0x00 }, 1, 2, 4, 6, 3, 5 },
	};
	/* these should never get called as this results in a successful rekeying */
	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b);

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_REKEYING);
	assert_child_sa_count(b, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[3];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* simplify next steps by checking in original IKE_SAs */
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, a);
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, b);
	assert_ike_sa_count(2);

	/* <-- CREATE_CHILD_SA { SA, Nr, KEr } */
	assert_hook_rekey(ike_rekey, 1, data[_i].spi_i);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	/* as original initiator a is initiator of both SAs it could delete */
	sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, TRUE);
	assert_ike_sa_state(sa, IKE_DELETING);
	assert_child_sa_count(sa, 0);
	/* if b won it will delete the original SA a initiated */
	sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r,
								data[_i].del_b_i == 1);
	assert_ike_sa_state(sa, IKE_REKEYED);
	assert_child_sa_count(sa, 0);
	sa = assert_ike_sa_checkout(data[_i].spi_i, data[_i].spi_r,
								data[_i].del_a_i == 1);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(4);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Nr, KEr } --> */
	assert_hook_rekey(ike_rekey, 1, data[_i].spi_i);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	/* if b wins it deletes the SA originally initiated by a */
	sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r,
								data[_i].del_b_i != 1);
	assert_ike_sa_state(sa, IKE_DELETING);
	assert_child_sa_count(sa, 0);
	/* a only deletes SAs for which b is responder */
	sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, FALSE);
	assert_ike_sa_state(sa, IKE_REKEYED);
	assert_child_sa_count(sa, 0);
	sa = assert_ike_sa_checkout(data[_i].spi_i, data[_i].spi_r,
								data[_i].del_b_i == 1);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(6);
	assert_hook();

	/* we don't expect this hook to get called anymore */
	assert_hook_not_called(ike_rekey);

	/* INFORMATIONAL { D } --> */
	assert_single_payload(IN, PLV2_DELETE);
	sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, FALSE);
	status = exchange_test_helper->process_message(exchange_test_helper, sa,
												   NULL);
	ck_assert_int_eq(DESTROY_ME, status);
	charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
	assert_ike_sa_count(5);
	/* <-- INFORMATIONAL { D } */
	assert_single_payload(IN, PLV2_DELETE);
	sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r,
								data[_i].del_b_i == 1);
	status = exchange_test_helper->process_message(exchange_test_helper, sa,
												   NULL);
	ck_assert_int_eq(DESTROY_ME, status);
	charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
	assert_ike_sa_count(4);
	/* <-- INFORMATIONAL { } */
	assert_message_empty(IN);
	sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, TRUE);
	status = exchange_test_helper->process_message(exchange_test_helper, sa,
												   NULL);
	ck_assert_int_eq(DESTROY_ME, status);
	charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
	assert_ike_sa_count(3);
	/* INFORMATIONAL { } --> */
	assert_message_empty(IN);
	sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r,
								data[_i].del_b_i != 1);
	status = exchange_test_helper->process_message(exchange_test_helper, sa,
												   NULL);
	ck_assert_int_eq(DESTROY_ME, status);
	charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
	assert_ike_sa_count(2);

	/* ike_rekey/ike_updown/child_updown */
	assert_hook();
	assert_hook();
	assert_hook();

	charon->ike_sa_manager->flush(charon->ike_sa_manager);
}
END_TEST

/**
 * Both peers initiate the IKE_SA rekeying concurrently but the proposed DH
 * groups are not the same.  After handling the INVALID_KE_PAYLOAD they should
 * still handle the collision properly depending on the nonces.
 */
START_TEST(test_collision_ke_invalid)
{
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.ike = "aes128-sha256-modp2048-modp3072",
		},
		.responder = {
			.ike = "aes128-sha256-modp3072-modp2048",
		},
	};
	ike_sa_t *a, *b, *sa;
	status_t status;

	lib->settings->set_bool(lib->settings, "%s.prefer_configured_proposals",
							FALSE, lib->ns);

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, &conf);

	lib->settings->set_bool(lib->settings, "%s.prefer_configured_proposals",
							TRUE, lib->ns);
	lib->settings->set_bool(lib->settings, "%s.prefer_previous_dh_group",
							FALSE, lib->ns);

	/* Six nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * IKE_SA):
	 *     N1/3 -----\    /----- N2/4
	 *                \--/-----> N3/5
	 *     N4/6 <-------/  /---- INVAL_KE
	 * INVAL_KE -----\    /
	 *          <-----\--/
	 *     N1/3 -----\ \------->
	 *                \    /---- N2/4
	 *                 \--/----> N5/7
	 *     N6/8 <--------/ /---- ...
	 *      ... ------\
	 * We test this four times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[4];
		/* SPIs of the deleted IKE_SAs (either redundant or replaced) */
		uint32_t del_a_i, del_a_r;
		uint32_t del_b_i, del_b_r;
		/* SPIs of the kept IKE_SA */
		uint32_t spi_i, spi_r;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF, 0xFF }, 3, 7, 1, 2, 4, 8 },
		{ { 0xFF, 0x00, 0xFF, 0xFF }, 1, 2, 4, 8, 3, 7 },
		{ { 0xFF, 0xFF, 0x00, 0xFF }, 3, 7, 1, 2, 4, 8 },
		{ { 0xFF, 0xFF, 0xFF, 0x00 }, 1, 2, 4, 8, 3, 7 },
	};
	/* these should never get called as this results in a successful rekeying */
	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b);

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_REKEYING);
	assert_child_sa_count(b, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[3];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(INVAL_KE) } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	assert_hook_not_called(ike_rekey);
	assert_single_notify(IN, INVALID_KE_PAYLOAD);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* CREATE_CHILD_SA { N(INVAL_KE) } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	assert_hook_not_called(child_rekey);
	assert_single_notify(IN, INVALID_KE_PAYLOAD);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_REKEYING);
	assert_child_sa_count(b, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_REKEYING);
	assert_child_sa_count(b, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[3];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* simplify next steps by checking in original IKE_SAs */
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, a);
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, b);
	assert_ike_sa_count(2);

	/* <-- CREATE_CHILD_SA { SA, Nr, KEr } */
	assert_hook_rekey(ike_rekey, 1, data[_i].spi_i);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	/* as original initiator a is initiator of both SAs it could delete */
	sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, TRUE);
	assert_ike_sa_state(sa, IKE_DELETING);
	assert_child_sa_count(sa, 0);
	/* if b won it will delete the original SA a initiated */
	sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r,
								data[_i].del_b_i == 1);
	assert_ike_sa_state(sa, IKE_REKEYED);
	assert_child_sa_count(sa, 0);
	sa = assert_ike_sa_checkout(data[_i].spi_i, data[_i].spi_r,
								data[_i].del_a_i == 1);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(4);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Nr, KEr } --> */
	assert_hook_rekey(ike_rekey, 1, data[_i].spi_i);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	/* if b wins it deletes the SA originally initiated by a */
	sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r,
								data[_i].del_b_i != 1);
	assert_ike_sa_state(sa, IKE_DELETING);
	assert_child_sa_count(sa, 0);
	/* a only deletes SAs for which b is responder */
	sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, FALSE);
	assert_ike_sa_state(sa, IKE_REKEYED);
	assert_child_sa_count(sa, 0);
	sa = assert_ike_sa_checkout(data[_i].spi_i, data[_i].spi_r,
								data[_i].del_b_i == 1);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(6);
	assert_hook();

	/* we don't expect this hook to get called anymore */
	assert_hook_not_called(ike_rekey);

	/* INFORMATIONAL { D } --> */
	assert_single_payload(IN, PLV2_DELETE);
	sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, FALSE);
	status = exchange_test_helper->process_message(exchange_test_helper, sa,
												   NULL);
	ck_assert_int_eq(DESTROY_ME, status);
	charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
	assert_ike_sa_count(5);
	/* <-- INFORMATIONAL { D } */
	assert_single_payload(IN, PLV2_DELETE);
	sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r,
								data[_i].del_b_i == 1);
	status = exchange_test_helper->process_message(exchange_test_helper, sa,
												   NULL);
	ck_assert_int_eq(DESTROY_ME, status);
	charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
	assert_ike_sa_count(4);
	/* <-- INFORMATIONAL { } */
	assert_message_empty(IN);
	sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, TRUE);
	status = exchange_test_helper->process_message(exchange_test_helper, sa,
												   NULL);
	ck_assert_int_eq(DESTROY_ME, status);
	charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
	assert_ike_sa_count(3);
	/* INFORMATIONAL { } --> */
	assert_message_empty(IN);
	sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r,
								data[_i].del_b_i != 1);
	status = exchange_test_helper->process_message(exchange_test_helper, sa,
												   NULL);
	ck_assert_int_eq(DESTROY_ME, status);
	charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
	assert_ike_sa_count(2);

	/* ike_rekey/ike_updown/child_updown */
	assert_hook();
	assert_hook();
	assert_hook();

	charon->ike_sa_manager->flush(charon->ike_sa_manager);
}
END_TEST

/**
 * This is like the collision above but one of the retries is delayed.
 */
START_TEST(test_collision_ke_invalid_delayed_retry)
{
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.ike = "aes128-sha256-modp2048-modp3072",
		},
		.responder = {
			.ike = "aes128-sha256-modp3072-modp2048",
		},
	};
	ike_sa_t *a, *b, *sa;
	message_t *msg;
	status_t s;

	lib->settings->set_bool(lib->settings, "%s.prefer_configured_proposals",
							FALSE, lib->ns);

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, &conf);

	lib->settings->set_bool(lib->settings, "%s.prefer_configured_proposals",
							TRUE, lib->ns);
	lib->settings->set_bool(lib->settings, "%s.prefer_previous_dh_group",
							FALSE, lib->ns);

	/* Five nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * IKE_SA):
	 *     N1/3 -----\    /----- N2/4
	 *                \--/-----> N3/5
	 *     N4/6 <-------/  /---- INVAL_KE
	 * INVAL_KE -----\    /
	 *          <-----\--/
	 *     N1/3 -----\ \------->
	 *          <-----\--------- N2/4
	 *     N5/7 -------\------->
	 *          <-------\------- DELETE
	 *      ... ------\  \----->
	 *                     /---- TEMP_FAIL
	 *
	 * We test this three times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[3];
	} data[] = {
		{ { 0x00, 0xFF, 0xFF } },
		{ { 0xFF, 0x00, 0xFF } },
		{ { 0xFF, 0xFF, 0x00 } },
	};
	/* these should never get called as this results in a successful rekeying */
	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b);

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_REKEYING);
	assert_child_sa_count(b, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(INVAL_KE) } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	assert_hook_not_called(ike_rekey);
	assert_single_notify(IN, INVALID_KE_PAYLOAD);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* CREATE_CHILD_SA { N(INVAL_KE) } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	assert_hook_not_called(child_rekey);
	assert_single_notify(IN, INVALID_KE_PAYLOAD);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_REKEYING);
	assert_child_sa_count(b, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* delay the CREATE_CHILD_SA request from a to b */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Nr, KEr } --> */
	assert_hook_rekey(ike_rekey, 1, 4);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_DELETING);
	assert_child_sa_count(b, 0);
	sa = assert_ike_sa_checkout(4, 7, TRUE);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(1);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> (delayed) */
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, msg);
	assert_ike_sa_state(b, IKE_DELETING);

	/* <-- INFORMATIONAL { D } */
	assert_hook_rekey(ike_rekey, 1, 4);
	assert_single_payload(IN, PLV2_DELETE);
	s = exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(a, destroy);
	sa = assert_ike_sa_checkout(4, 7, FALSE);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(2);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	/* the SA is already gone */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);
	msg->destroy(msg);

	/* INFORMATIONAL { } --> */
	assert_hook_not_called(ike_rekey);
	assert_message_empty(IN);
	s = exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(b, destroy);
	assert_hook();

	/* ike_updown/child_updown */
	assert_hook();
	assert_hook();

	charon->ike_sa_manager->flush(charon->ike_sa_manager);
}
END_TEST

/**
 * This is like the rekey collision above, but one peer deletes the
 * redundant/old SA before the other peer receives the CREATE_CHILD_SA
 * response:
 *           Peer A                   Peer B
 *            rekey ----\       /---- rekey
 *                       \-----/----> detect collision
 * detect collision <---------/ /----
 *                  -----------/---->
 *    handle delete <---------/------ delete redundant/old SA
 *                  ---------/------>
 *     handle rekey <-------/
 *        delete SA ---------------->
 *                  <----------------
 *
 * If peer B won the collision it deletes the old IKE_SA, in which case
 * this situation is handled as if peer B was not aware of the collision (see
 * below).  That is, peer A finalizes the rekeying initiated by the peer and
 * deletes the IKE_SA (it has no way of knowing whether the peer was aware of
 * the collision or not).  Peer B will expect the redundant IKE_SA to get
 * deleted, but that will never happen if the response arrives after the SA is
 * already gone.  So a job should be queued that deletes it after a while.
 *
 * If peer B lost it will switch to the new IKE_SA and delete the redundant
 * IKE_SA and expect a delete for the old IKE_SA.  In this case peer A will
 * simply retransmit until it receives a response to the rekey request, all the
 * while ignoring the delete requests for the unknown IKE_SA.  Afterwards,
 * everything works as in a regular collision (however, until peer A receives
 * the response it will not be able to receive any messages on the new IKE_SA).
 */
START_TEST(test_collision_delayed_response)
{
	ike_sa_t *a, *b, *sa;
	message_t *msg, *d;
	status_t s;

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	/* Four nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * IKE_SA):
	 *   N1/3 -----\    /----- N2/4
	 *              \--/-----> N3/5
	 *   N4/6 <-------/ /----- ...
	 *   ...  -----\
	 * We test this four times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[4];
		/* SPIs of the deleted IKE_SAs (either redundant or replaced) */
		uint32_t del_a_i, del_a_r;
		uint32_t del_b_i, del_b_r;
		/* SPIs of the kept IKE_SA */
		uint32_t spi_i, spi_r;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF, 0xFF }, 3, 5, 1, 2, 4, 6 },
		{ { 0xFF, 0x00, 0xFF, 0xFF }, 1, 2, 4, 6, 3, 5 },
		{ { 0xFF, 0xFF, 0x00, 0xFF }, 3, 5, 1, 2, 4, 6 },
		{ { 0xFF, 0xFF, 0xFF, 0x00 }, 1, 2, 4, 6, 3, 5 },
	};
	/* these should never get called as this results in a successful rekeying */
	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b);

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_REKEYING);
	assert_child_sa_count(b, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[3];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* delay the CREATE_CHILD_SA response from b to a */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* simplify next steps by checking in original IKE_SAs */
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, a);
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, b);
	assert_ike_sa_count(2);

	/* CREATE_CHILD_SA { SA, Nr, KEr } --> */
	assert_hook_rekey(ike_rekey, 1, data[_i].spi_i);
	/* besides the job that retransmits the delete, we expect a job that
	 * deletes the redundant IKE_SA if we expect the other to delete it */
	assert_jobs_scheduled(data[_i].del_b_i == 1 ? 2 : 1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	/* if b wins it deletes the SA originally initiated by a */
	sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r,
								data[_i].del_b_i != 1);
	assert_ike_sa_state(sa, IKE_DELETING);
	assert_child_sa_count(sa, 0);
	/* a only deletes SAs for which b is responder */
	sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, FALSE);
	assert_ike_sa_state(sa, IKE_REKEYED);
	assert_child_sa_count(sa, 0);
	sa = assert_ike_sa_checkout(data[_i].spi_i, data[_i].spi_r,
								data[_i].del_b_i == 1);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(4);
	assert_scheduler();
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	if (data[_i].del_b_i == 1)
	{	/* b won, it deletes the replaced IKE_SA */
		assert_hook_rekey(ike_rekey, 1, data[_i].spi_i);
		assert_single_payload(IN, PLV2_DELETE);
		s = exchange_test_helper->process_message(exchange_test_helper, a,
												  NULL);
		ck_assert_int_eq(DESTROY_ME, s);
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, a);
		sa = assert_ike_sa_checkout(data[_i].spi_i, data[_i].spi_r, FALSE);
		assert_ike_sa_state(sa, IKE_ESTABLISHED);
		assert_child_sa_count(sa, 1);
		assert_ike_sa_count(4);
		assert_hook();

		/* INFORMATIONAL { } --> */
		assert_hook_not_called(ike_rekey);
		assert_message_empty(IN);
		s = exchange_test_helper->process_message(exchange_test_helper, b,
												  NULL);
		ck_assert_int_eq(DESTROY_ME, s);
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, b);
		assert_ike_sa_count(3);
		assert_hook();
		/* the job will later remove this redundant IKE_SA on b */
		sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, FALSE);
		assert_ike_sa_state(sa, IKE_REKEYED);
		assert_sa_idle(sa);
		/* <-- CREATE_CHILD_SA { SA, Nr, KEr } (delayed) */
		/* the IKE_SA (a) does not exist anymore */
		msg->destroy(msg);
	}
	else
	{	/* b lost, the delete is for the non-existing redundant IKE_SA */
		d = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

		/* <-- CREATE_CHILD_SA { SA, Nr, KEr } (delayed) */
		assert_hook_rekey(ike_rekey, 1, data[_i].spi_i);
		exchange_test_helper->process_message(exchange_test_helper, a, msg);
		/* as original initiator a is initiator of both SAs it could delete */
		sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, TRUE);
		assert_ike_sa_state(sa, IKE_DELETING);
		assert_child_sa_count(sa, 0);
		/* this is the redundant SA b is trying to delete */
		sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r, FALSE);
		assert_ike_sa_state(sa, IKE_REKEYED);
		assert_child_sa_count(sa, 0);
		sa = assert_ike_sa_checkout(data[_i].spi_i, data[_i].spi_r,
									data[_i].del_a_i == 1);
		assert_ike_sa_state(sa, IKE_ESTABLISHED);
		assert_child_sa_count(sa, 1);
		assert_ike_sa_count(6);
		assert_hook();

		/* we don't expect this hook to get called anymore */
		assert_hook_not_called(ike_rekey);

		/* INFORMATIONAL { D } --> */
		assert_single_payload(IN, PLV2_DELETE);
		sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, FALSE);
		s = exchange_test_helper->process_message(exchange_test_helper, sa,
												  NULL);
		ck_assert_int_eq(DESTROY_ME, s);
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
		assert_ike_sa_count(5);
		/* <-- INFORMATIONAL { } */
		assert_message_empty(IN);
		sa = assert_ike_sa_checkout(data[_i].del_a_i, data[_i].del_a_r, TRUE);
		s = exchange_test_helper->process_message(exchange_test_helper, sa,
												  NULL);
		ck_assert_int_eq(DESTROY_ME, s);
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
		assert_ike_sa_count(4);

		/* <-- INFORMATIONAL { D } (retransmit/delayed) */
		assert_single_payload(IN, PLV2_DELETE);
		sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r, FALSE);
		s = exchange_test_helper->process_message(exchange_test_helper, sa, d);
		ck_assert_int_eq(DESTROY_ME, s);
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
		assert_ike_sa_count(3);
		/* INFORMATIONAL { } --> */
		assert_message_empty(IN);
		sa = assert_ike_sa_checkout(data[_i].del_b_i, data[_i].del_b_r, TRUE);
		s = exchange_test_helper->process_message(exchange_test_helper, sa,
												  NULL);
		ck_assert_int_eq(DESTROY_ME, s);
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
		assert_ike_sa_count(2);
		/* ike_rekey */
		assert_hook();
	}

	/* ike_updown/child_updown */
	assert_hook();
	assert_hook();

	charon->ike_sa_manager->flush(charon->ike_sa_manager);
}
END_TEST

/**
 * In this scenario one of the peers does not notice that there is a rekey
 * collision because the other request is dropped:
 *
 *            rekey ----\       /---- rekey
 *                       \     /
 * detect collision <-----\---/
 *                  -------\-------->
 * detect collision <-------\-------- delete old SA
 *           delete ---------\------>
 *       rekey done           \-----> SA not found (or it never arrives)
 */
START_TEST(test_collision_dropped_request)
{
	ike_sa_t *a, *b, *sa;
	message_t *msg;
	status_t s;

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	/* Three nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * CHILD_SA):
	 *   N1/3 -----\    /----- N2/4
	 *   N3/5 <-----\--/
	 *   ...  -----\ \-------> ...
	 * We test this three times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[3];
		/* SPIs of the deleted IKE_SAs (either redundant or replaced) */
		uint32_t del_a_i, del_a_r;
		uint32_t del_b_i, del_b_r;
		/* SPIs of the kept IKE_SA */
		uint32_t spi_i, spi_r;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF }, 3, 5, 1, 2, 4, 6 },
		{ { 0xFF, 0x00, 0xFF }, 1, 2, 4, 6, 3, 5 },
		{ { 0xFF, 0xFF, 0x00 }, 3, 5, 1, 2, 4, 6 },
		{ { 0xFF, 0xFF, 0xFF }, 1, 2, 4, 6, 3, 5 },
	};
	/* these should never get called as this results in a successful rekeying */
	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a);
	/* drop the CREATE_CHILD_SA request from a to b */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);
	msg->destroy(msg);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b);

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	assert_hook_rekey(ike_rekey, 1, 4);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_DELETING);
	assert_child_sa_count(b, 0);
	sa = assert_ike_sa_checkout(4, 5, TRUE);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(1);
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_rekey(ike_rekey, 1, 4);
	assert_single_payload(IN, PLV2_DELETE);
	s = exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(a, destroy);
	sa = assert_ike_sa_checkout(4, 5, FALSE);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(2);
	assert_hook();

	/* INFORMATIONAL { } --> */
	assert_hook_not_called(ike_rekey);
	assert_message_empty(IN);
	s = exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(b, destroy);
	assert_hook();

	/* ike_updown/child_updown */
	assert_hook();
	assert_hook();

	charon->ike_sa_manager->flush(charon->ike_sa_manager);
}
END_TEST

/**
 * In this scenario one of the peers does not notice that there is a rekey
 * collision because the other request is delayed:
 *
 *            rekey ----\       /---- rekey
 *                       \     /
 * detect collision <-----\---/
 *                  -------\-------->
 *                          \   /---- delete old SA
 *                           \-/----> detect collision
 * detect collision <---------/ /---- TEMP_FAIL
 *           delete -----------/---->
 *       rekey done           /
 *  sa already gone <--------/
 */
START_TEST(test_collision_delayed_request)
{
	ike_sa_t *a, *b, *sa;
	message_t *msg;
	status_t s;

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	/* Three nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * CHILD_SA):
	 *   N1/3 -----\    /----- N2/4
	 *   N3/5 <-----\--/
	 *   ...  -----\ \-------> ...
	 * We test this three times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[3];
		/* SPIs of the deleted IKE_SAs (either redundant or replaced) */
		uint32_t del_a_i, del_a_r;
		uint32_t del_b_i, del_b_r;
		/* SPIs of the kept IKE_SA */
		uint32_t spi_i, spi_r;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF }, 3, 5, 1, 2, 4, 6 },
		{ { 0xFF, 0x00, 0xFF }, 1, 2, 4, 6, 3, 5 },
		{ { 0xFF, 0xFF, 0x00 }, 3, 5, 1, 2, 4, 6 },
		{ { 0xFF, 0xFF, 0xFF }, 1, 2, 4, 6, 3, 5 },
	};
	/* these should never get called as this results in a successful rekeying */
	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b);

	/* delay the CREATE_CHILD_SA request from a to b */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	assert_hook_rekey(ike_rekey, 1, 4);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_DELETING);
	assert_child_sa_count(b, 0);
	sa = assert_ike_sa_checkout(4, 5, TRUE);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(1);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> (delayed) */
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, msg);
	assert_ike_sa_state(b, IKE_DELETING);

	/* <-- INFORMATIONAL { D } */
	assert_hook_rekey(ike_rekey, 1, 4);
	assert_single_payload(IN, PLV2_DELETE);
	s = exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(a, destroy);
	sa = assert_ike_sa_checkout(4, 5, FALSE);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(2);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	/* the SA is already gone */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);
	msg->destroy(msg);

	/* INFORMATIONAL { } --> */
	assert_hook_not_called(ike_rekey);
	assert_message_empty(IN);
	s = exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(b, destroy);
	assert_hook();

	/* ike_updown/child_updown */
	assert_hook();
	assert_hook();

	charon->ike_sa_manager->flush(charon->ike_sa_manager);
}
END_TEST

/**
 * In this scenario one of the peers does not notice that there is a rekey
 * collision and the delete arrives after the TEMPORARY_FAILURE notify:
 *
 *            rekey ----\       /---- rekey
 *                       \     /
 * detect collision <-----\---/
 *                  -------\-------->
 *                          \   /---- delete old SA
 *                           \-/----> detect collision
 *    no reschedule <---------/------ TEMP_FAIL
 * detect collision <--------/
 *           delete ---------------->
 *       rekey done
 */
START_TEST(test_collision_delayed_request_and_delete)
{
	ike_sa_t *a, *b, *sa;
	message_t *msg;
	status_t s;

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	/* Three nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * CHILD_SA):
	 *   N1/3 -----\    /----- N2/4
	 *   N3/5 <-----\--/
	 *   ...  -----\ \-------> ...
	 * We test this three times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[3];
		/* SPIs of the deleted IKE_SAs (either redundant or replaced) */
		uint32_t del_a_i, del_a_r;
		uint32_t del_b_i, del_b_r;
		/* SPIs of the kept IKE_SA */
		uint32_t spi_i, spi_r;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF }, 3, 5, 1, 2, 4, 6 },
		{ { 0xFF, 0x00, 0xFF }, 1, 2, 4, 6, 3, 5 },
		{ { 0xFF, 0xFF, 0x00 }, 3, 5, 1, 2, 4, 6 },
		{ { 0xFF, 0xFF, 0xFF }, 1, 2, 4, 6, 3, 5 },
	};
	/* these should never get called as this results in a successful rekeying */
	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b);

	/* delay the CREATE_CHILD_SA request from a to b */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(ike_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ike_sa_count(0);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	assert_hook_rekey(ike_rekey, 1, 4);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_DELETING);
	assert_child_sa_count(b, 0);
	sa = assert_ike_sa_checkout(4, 5, TRUE);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(1);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> (delayed) */
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, msg);
	assert_ike_sa_state(b, IKE_DELETING);

	/* delay the INFORMATIONAL request from b to a */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	assert_hook_rekey(ike_rekey, 1, 4);
	assert_no_jobs_scheduled();
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_REKEYED);
	assert_child_sa_count(a, 0);
	sa = assert_ike_sa_checkout(4, 5, FALSE);
	assert_ike_sa_state(sa, IKE_ESTABLISHED);
	assert_child_sa_count(sa, 1);
	assert_ike_sa_count(2);
	assert_scheduler();
	assert_hook();

	/* <-- INFORMATIONAL { D } (delayed) */
	assert_single_payload(IN, PLV2_DELETE);
	s = exchange_test_helper->process_message(exchange_test_helper, a, msg);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(a, destroy);

	/* INFORMATIONAL { } --> */
	assert_hook_not_called(ike_rekey);
	assert_message_empty(IN);
	s = exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(b, destroy);
	assert_hook();

	/* ike_updown/child_updown */
	assert_hook();
	assert_hook();

	charon->ike_sa_manager->flush(charon->ike_sa_manager);
}
END_TEST

/**
 * One of the hosts initiates a DELETE of the IKE_SA the other peer is
 * concurrently trying to rekey.
 *
 *            rekey ----\       /---- delete
 *                       \-----/----> detect collision
 * detect collision <---------/ /---- TEMP_FAIL
 *           delete ----\      /
 *                       \----/----->
 *  sa already gone <--------/
 */
START_TEST(test_collision_delete)
{
	ike_sa_t *a, *b;
	message_t *msg;
	status_t s;

	if (_i)
	{	/* responder rekeys the IKE_SA */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, NULL);
	}
	else
	{	/* initiator rekeys the IKE_SA */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, NULL);
	}
	/* this should never get called as this does not result in a successful
	 * rekeying on either side */
	assert_hook_not_called(ike_rekey);

	initiate_rekey(a);
	call_ikesa(b, delete, FALSE);
	assert_ike_sa_state(b, IKE_DELETING);

	/* RFC 7296, 2.25.2: If a peer receives a request to rekey an IKE SA that
	 * it is currently trying to close, it SHOULD reply with TEMPORARY_FAILURE.
	 */

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	assert_hook_not_called(ike_updown);
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_DELETING);
	assert_ike_sa_count(0);
	assert_hook();

	/* RFC 7296, 2.25.2: If a peer receives a request to close an IKE SA that
	 * it is currently rekeying, it SHOULD reply as usual, and forget its own
	 * rekeying request.
	 */

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

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	/* the SA is already gone */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);
	msg->destroy(msg);

	/* INFORMATIONAL { } --> */
	assert_hook_updown(ike_updown, FALSE);
	assert_hook_updown(child_updown, FALSE);
	s = exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(b, destroy);
	assert_hook();
	assert_hook();

	/* ike_rekey */
	assert_hook();
}
END_TEST

/**
 * One of the hosts initiates a DELETE of the IKE_SA the other peer is
 * concurrently trying to rekey.  However, the delete request is delayed or
 * dropped, so the peer doing the rekeying is unaware of the collision.
 *
 *            rekey ----\       /---- delete
 *                       \-----/----> detect collision
 *       reschedule <---------/------ TEMP_FAIL
 *                  <--------/
 *           delete ---------------->
 */
START_TEST(test_collision_delete_drop_delete)
{
	ike_sa_t *a, *b;
	message_t *msg;
	status_t s;

	if (_i)
	{	/* responder rekeys the IKE_SA */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, NULL);
	}
	else
	{	/* initiator rekeys the IKE_SA */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, NULL);
	}
	/* this should never get called as this does not result in a successful
	 * rekeying on either side */
	assert_hook_not_called(ike_rekey);

	initiate_rekey(a);
	call_ikesa(b, delete, FALSE);
	assert_ike_sa_state(b, IKE_DELETING);

	/* RFC 7296, 2.25.2: If a peer receives a request to rekey an IKE SA that
	 * it is currently trying to close, it SHOULD reply with TEMPORARY_FAILURE.
	 */

	/* CREATE_CHILD_SA { SA, Ni, KEi } --> */
	assert_hook_not_called(ike_updown);
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_DELETING);
	assert_ike_sa_count(0);
	assert_hook();

	/* delay the DELETE request */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	assert_hook_not_called(ike_updown);
	assert_hook_not_called(child_updown);
	/* we expect a job to retry the rekeying is scheduled */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_ike_sa_state(a, IKE_ESTABLISHED);
	assert_scheduler();
	assert_hook();
	assert_hook();

	/* <-- INFORMATIONAL { D } (delayed) */
	assert_hook_updown(ike_updown, FALSE);
	assert_hook_updown(child_updown, FALSE);
	assert_single_payload(IN, PLV2_DELETE);
	assert_message_empty(OUT);
	s = exchange_test_helper->process_message(exchange_test_helper, a, msg);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(a, destroy);
	assert_hook();
	assert_hook();

	/* INFORMATIONAL { } --> */
	assert_hook_updown(ike_updown, FALSE);
	assert_hook_updown(child_updown, FALSE);
	s = exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	ck_assert_int_eq(DESTROY_ME, s);
	call_ikesa(b, destroy);
	assert_hook();
	assert_hook();

	/* ike_rekey */
	assert_hook();
}
END_TEST

Suite *ike_rekey_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("ike rekey");

	tc = tcase_create("regular");
	tcase_add_loop_test(tc, test_regular, 0, 2);
	tcase_add_loop_test(tc, test_regular_ke_invalid, 0, 2);
	suite_add_tcase(s, tc);

	tc = tcase_create("collisions rekey");
	tcase_add_loop_test(tc, test_collision, 0, 4);
	tcase_add_loop_test(tc, test_collision_ke_invalid, 0, 4);
	tcase_add_loop_test(tc, test_collision_ke_invalid_delayed_retry, 0, 3);
	tcase_add_loop_test(tc, test_collision_delayed_response, 0, 4);
	tcase_add_loop_test(tc, test_collision_dropped_request, 0, 3);
	tcase_add_loop_test(tc, test_collision_delayed_request, 0, 3);
	tcase_add_loop_test(tc, test_collision_delayed_request_and_delete, 0, 3);
	suite_add_tcase(s, tc);

	tc = tcase_create("collisions delete");
	tcase_add_loop_test(tc, test_collision_delete, 0, 2);
	tcase_add_loop_test(tc, test_collision_delete_drop_delete, 0, 2);
	suite_add_tcase(s, tc);

	return s;
}
