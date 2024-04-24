/*
 * Copyright (C) 2019 Tobias Brunner
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

#include "test_suite.h"

#include <daemon.h>
#include <tests/utils/exchange_test_helper.h>
#include <tests/utils/exchange_test_asserts.h>
#include <tests/utils/sa_asserts.h>

/**
 * Childless initiation of the IKE_SA. The first CHILD_SA is automatically
 * initiated in a separate CREATE_CHILD_SA exchange including DH.
 */
START_TEST(test_regular)
{
	childless_t childless[] = {
		CHILDLESS_FORCE,
		CHILDLESS_PREFER,
	};
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.childless = childless[_i],
			.esp = "aes128-sha256-modp3072",
		},
		.responder = {
			.esp = "aes128-sha256-modp3072",
		},
	};
	ike_sa_t *a, *b;
	ike_sa_id_t *id_a, *id_b;
	child_cfg_t *child_cfg;

	child_cfg = exchange_test_helper->create_sa(exchange_test_helper, &a, &b,
												&conf);
	id_a = a->get_id(a);
	id_b = b->get_id(b);

	call_ikesa(a, initiate, child_cfg, NULL);

	/* IKE_SA_INIT --> */
	id_b->set_initiator_spi(id_b, id_a->get_initiator_spi(id_a));
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	/* <-- IKE_SA_INIT */
	assert_notify(IN, CHILDLESS_IKEV2_SUPPORTED);
	id_a->set_responder_spi(id_a, id_b->get_responder_spi(id_b));
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);

	/* IKE_AUTH --> */
	assert_hook_not_called(child_updown);
	assert_no_payload(IN, PLV2_SECURITY_ASSOCIATION);
	assert_no_payload(IN, PLV2_TS_INITIATOR);
	assert_no_payload(IN, PLV2_TS_RESPONDER);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);

	/* <-- IKE_AUTH */
	assert_no_payload(IN, PLV2_SECURITY_ASSOCIATION);
	assert_no_payload(IN, PLV2_TS_INITIATOR);
	assert_no_payload(IN, PLV2_TS_RESPONDER);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 0);
	assert_child_sa_count(b, 0);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Ni, KEi, TSi, TSr } --> */
	assert_hook_called(child_updown);
	assert_payload(IN, PLV2_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_count(b, 1);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	assert_hook_called(child_updown);
	assert_payload(IN, PLV2_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 1);
	assert_hook();

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Childless initiation of the IKE_SA, no CHILD_SA created automatically.
 * It's created with a separate initiation and exchange afterwards.
 */
START_TEST(test_regular_manual)
{
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.esp = "aes128-sha256-modp3072",
		},
		.responder = {
			.esp = "aes128-sha256-modp3072",
		},
	};
	ike_sa_t *a, *b;
	ike_sa_id_t *id_a, *id_b;
	child_cfg_t *child_cfg;

	child_cfg = exchange_test_helper->create_sa(exchange_test_helper, &a, &b,
												&conf);
	id_a = a->get_id(a);
	id_b = b->get_id(b);

	call_ikesa(a, initiate, NULL, NULL);

	/* IKE_SA_INIT --> */
	id_b->set_initiator_spi(id_b, id_a->get_initiator_spi(id_a));
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	/* <-- IKE_SA_INIT */
	assert_notify(IN, CHILDLESS_IKEV2_SUPPORTED);
	id_a->set_responder_spi(id_a, id_b->get_responder_spi(id_b));
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);

	/* IKE_AUTH --> */
	assert_hook_not_called(child_updown);
	assert_no_payload(IN, PLV2_SECURITY_ASSOCIATION);
	assert_no_payload(IN, PLV2_TS_INITIATOR);
	assert_no_payload(IN, PLV2_TS_RESPONDER);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);

	/* <-- IKE_AUTH */
	assert_no_payload(IN, PLV2_SECURITY_ASSOCIATION);
	assert_no_payload(IN, PLV2_TS_INITIATOR);
	assert_no_payload(IN, PLV2_TS_RESPONDER);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 0);
	assert_child_sa_count(b, 0);
	assert_hook();

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, initiate, child_cfg, NULL);

	/* CREATE_CHILD_SA { SA, Ni, KEi, TSi, TSr } --> */
	assert_hook_called(child_updown);
	assert_payload(IN, PLV2_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_count(b, 1);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	assert_hook_called(child_updown);
	assert_payload(IN, PLV2_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 1);
	assert_hook();

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * The initiator aborts the initiation once it notices the responder does not
 * support childless IKE_SAs.
 */
START_TEST(test_failure_init)
{
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.childless = CHILDLESS_FORCE,
		},
		.responder = {
			.childless = CHILDLESS_NEVER,
		},
	};
	ike_sa_t *a, *b;
	ike_sa_id_t *id_a, *id_b;
	child_cfg_t *child_cfg;
	status_t status;

	child_cfg = exchange_test_helper->create_sa(exchange_test_helper, &a, &b,
												&conf);
	id_a = a->get_id(a);
	id_b = b->get_id(b);

	call_ikesa(a, initiate, child_cfg, NULL);

	/* IKE_SA_INIT --> */
	id_b->set_initiator_spi(id_b, id_a->get_initiator_spi(id_a));
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	/* <-- IKE_SA_INIT */
	assert_no_notify(IN, CHILDLESS_IKEV2_SUPPORTED);
	id_a->set_responder_spi(id_a, id_b->get_responder_spi(id_b));
	status = exchange_test_helper->process_message(exchange_test_helper, a,
												   NULL);
	ck_assert_int_eq(DESTROY_ME, status);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * The responder aborts the initiation once it notices the initiator does not
 * create a childless IKE_SA.
 */
START_TEST(test_failure_resp)
{
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.childless = CHILDLESS_NEVER,
		},
		.responder = {
			.childless = CHILDLESS_FORCE,
		},
	};
	ike_sa_t *a, *b;
	ike_sa_id_t *id_a, *id_b;
	child_cfg_t *child_cfg;
	status_t status;

	child_cfg = exchange_test_helper->create_sa(exchange_test_helper, &a, &b,
												&conf);
	id_a = a->get_id(a);
	id_b = b->get_id(b);

	call_ikesa(a, initiate, child_cfg, NULL);

	/* IKE_SA_INIT --> */
	id_b->set_initiator_spi(id_b, id_a->get_initiator_spi(id_a));
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	/* <-- IKE_SA_INIT */
	assert_notify(IN, CHILDLESS_IKEV2_SUPPORTED);
	id_a->set_responder_spi(id_a, id_b->get_responder_spi(id_b));
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);

	/* IKE_AUTH --> */
	assert_hook_not_called(child_updown);
	assert_payload(IN, PLV2_SECURITY_ASSOCIATION);
	assert_payload(IN, PLV2_TS_INITIATOR);
	assert_payload(IN, PLV2_TS_RESPONDER);
	status = exchange_test_helper->process_message(exchange_test_helper, b,
												   NULL);
	ck_assert_int_eq(DESTROY_ME, status);
	assert_hook();

	/* <-- IKE_AUTH */
	assert_hook_not_called(child_updown);
	assert_no_payload(IN, PLV2_SECURITY_ASSOCIATION);
	assert_no_payload(IN, PLV2_TS_INITIATOR);
	assert_no_payload(IN, PLV2_TS_RESPONDER);
	assert_notify(IN, INVALID_SYNTAX);
	status = exchange_test_helper->process_message(exchange_test_helper, a,
												   NULL);
	ck_assert_int_eq(DESTROY_ME, status);
	assert_hook();

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

Suite *childless_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("childless");

	tc = tcase_create("initiation");
	tcase_add_loop_test(tc, test_regular, 0, 2);
	tcase_add_test(tc, test_regular_manual);
	suite_add_tcase(s, tc);

	tc = tcase_create("failure");
	tcase_add_test(tc, test_failure_init);
	tcase_add_test(tc, test_failure_resp);
	suite_add_tcase(s, tc);

	return s;
}
