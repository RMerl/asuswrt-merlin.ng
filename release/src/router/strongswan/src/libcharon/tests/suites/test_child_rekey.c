/*
 * Copyright (C) 2016-2022 Tobias Brunner
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
#include <encoding/payloads/delete_payload.h>
#include <tests/utils/exchange_test_helper.h>
#include <tests/utils/exchange_test_asserts.h>
#include <tests/utils/job_asserts.h>
#include <tests/utils/sa_asserts.h>

/**
 * Initiate rekeying the CHILD_SA with the given SPI on the given IKE_SA.
 */
#define initiate_rekey(sa, spi) ({ \
	assert_hook_not_called(child_updown); \
	assert_hook_not_called(child_rekey); \
	call_ikesa(sa, rekey_child_sa, PROTO_ESP, spi); \
	assert_child_sa_state(sa, spi, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED); \
	assert_hook(); \
	assert_hook(); \
})

/**
 * Destroy a rekeyed CHILD_SA that was kept around to accept inbound traffic.
 * Simulates the job that's scheduled to do this.
 */
#define destroy_rekeyed(sa, spi) ({ \
	assert_hook_not_called(child_updown); \
	assert_hook_not_called(child_rekey); \
	assert_no_jobs_scheduled(); \
	assert_child_sa_state(sa, spi, CHILD_DELETED, CHILD_OUTBOUND_NONE); \
	call_ikesa(sa, delete_child_sa, PROTO_ESP, spi, FALSE); \
	assert_child_sa_not_exists(sa, spi); \
	assert_scheduler(); \
	assert_hook(); \
	assert_hook(); \
})

/**
 * Regular CHILD_SA rekey either initiated by the original initiator or
 * responder of the IKE_SA.
 */
START_TEST(test_regular)
{
	ike_sa_t *a, *b;
	uint32_t spi_a = _i+1, spi_b = 2-_i;

	assert_track_sas_start();

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
	initiate_rekey(a, spi_a);
	assert_ipsec_sas_installed(a, spi_a, spi_b);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, spi_a, spi_b, 4);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } */
	assert_hook_rekey(child_rekey, spi_a, 3);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, spi_a, 3, 4);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_rekey(child_rekey, spi_b, 4);
	assert_jobs_scheduled(1);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, spi_b, 3, 4);
	assert_scheduler();
	assert_hook();
	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_rekey);
	assert_jobs_scheduled(1);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, spi_a, 3, 4);
	assert_scheduler();
	assert_hook();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, spi_a);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 3, 4);
	destroy_rekeyed(b, spi_b);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(a, 3, 4);

	/* child_updown */
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Config for multiple KE exchange tests
 */
static exchange_test_sa_conf_t multi_ke_conf = {
	.initiator = {
		.esp = "aes256-sha256-modp3072-ke1_ecp256",
	},
	.responder = {
		.esp = "aes256-sha256-modp3072-ke1_ecp256",
	},
};

/**
 * Regular CHILD_SA rekey with multiple key exchanges either initiated by the
 * original initiator or responder of the IKE_SA.
 */
START_TEST(test_regular_multi_ke)
{
	ike_sa_t *a, *b;
	uint32_t spi_a = _i+1, spi_b = 2-_i;

	assert_track_sas_start();

	if (_i)
	{	/* responder rekeys the CHILD_SA (SPI 2) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, &multi_ke_conf);
	}
	else
	{	/* initiator rekeys the CHILD_SA (SPI 1) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, &multi_ke_conf);
	}
	initiate_rekey(a, spi_a);
	assert_ipsec_sas_installed(a, spi_a, spi_b);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, KEi, TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, spi_a, spi_b);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, KEr, TSi, TSr, N(ADD_KE) } */
	assert_hook_not_called(child_rekey);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, spi_a, spi_b);
	assert_hook();

	/* IKE_FOLLOWUP_KE { KEi, N(ADD_KE) } --> */
	assert_hook_not_called(child_rekey);
	assert_payload(IN, PLV2_KEY_EXCHANGE);
	assert_notify(IN, ADDITIONAL_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, spi_a, spi_b, 4);
	assert_hook();

	/* <-- IKE_FOLLOWUP_KE { KEr } */
	assert_hook_rekey(child_rekey, spi_a, 3);
	assert_payload(IN, PLV2_KEY_EXCHANGE);
	assert_no_notify(IN, ADDITIONAL_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, spi_a, 3, 4);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_rekey(child_rekey, spi_b, 4);
	assert_jobs_scheduled(1);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, spi_b, 3, 4);
	assert_scheduler();
	assert_hook();
	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_rekey);
	assert_jobs_scheduled(1);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, spi_a, 3, 4);
	assert_scheduler();
	assert_hook();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, spi_a);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 3, 4);
	destroy_rekeyed(b, spi_b);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(a, 3, 4);

	/* child_updown */
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * CHILD_SA rekey where the responder does not agree with the DH group selected
 * by the initiator, either initiated by the original initiator or responder of
 * the IKE_SA.
 */
START_TEST(test_regular_ke_invalid)
{
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.esp = "aes128-sha256-modp2048-modp3072",
		},
		.responder = {
			.esp = "aes128-sha256-modp3072-modp2048",
		},
	};
	ike_sa_t *a, *b;
	uint32_t spi_a = _i+1, spi_b = 2-_i;

	assert_track_sas_start();

	if (_i)
	{	/* responder rekeys the CHILD_SA (SPI 2) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, &conf);
	}
	else
	{	/* initiator rekeys the CHILD_SA (SPI 1) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, &conf);
	}
	initiate_rekey(a, spi_a);
	assert_ipsec_sas_installed(a, spi_a, spi_b);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_INSTALLED);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, spi_a, spi_b);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(INVAL_KE) } */
	assert_hook_not_called(child_rekey);
	assert_single_notify(IN, INVALID_KE_PAYLOAD);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, spi_a, spi_b);
	assert_hook();

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_REKEYED);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, spi_a, spi_b, 5);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } */
	assert_hook_rekey(child_rekey, spi_a, 4);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, spi_a, 4, 5);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_rekey(child_rekey, spi_b, 5);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, spi_b, 4, 5);
	assert_hook();
	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_rekey);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 4, CHILD_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, spi_a, 4, 5);
	assert_hook();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, spi_a);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 4, 5);
	destroy_rekeyed(b, spi_b);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 4, 5);

	/* child_updown */
	assert_hook();

	/* because the DH group should get reused another rekeying should complete
	 * without additional exchange */
	initiate_rekey(a, 4);
	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 5, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 7, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, 4, 5, 7);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } */
	assert_hook_called(child_rekey);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 4, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 4, 6, 7);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_called(child_rekey);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 5, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 7, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, 5, 6, 7);
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_rekey);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 4, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 6, CHILD_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 4, 6, 7);
	assert_hook();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, 4);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 6, 7);
	destroy_rekeyed(b, 5);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 6, 7);

	/* child_updown */
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * CHILD_SA rekey with multiple key exchanges where the responder does not agree
 * with the first key exchange method selected by the initiator, either
 * initiated by the original initiator or responder of the IKE_SA.
 */
START_TEST(test_regular_ke_invalid_multi_ke)
{
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.esp = "aes128-sha256-modp2048-modp3072-ke1_ecp256",
		},
		.responder = {
			.esp = "aes128-sha256-modp3072-modp2048-ke1_ecp256",
		},
	};
	ike_sa_t *a, *b;
	uint32_t spi_a = _i+1, spi_b = 2-_i;

	assert_track_sas_start();

	if (_i)
	{	/* responder rekeys the CHILD_SA (SPI 2) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, &conf);
	}
	else
	{	/* initiator rekeys the CHILD_SA (SPI 1) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, &conf);
	}
	initiate_rekey(a, spi_a);
	assert_ipsec_sas_installed(a, spi_a, spi_b);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_INSTALLED);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, spi_a, spi_b);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(INVAL_KE) } */
	assert_hook_not_called(child_rekey);
	assert_single_notify(IN, INVALID_KE_PAYLOAD);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_REKEYING);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, spi_a, spi_b);
	assert_hook();

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, spi_a, spi_b);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } */
	assert_hook_not_called(child_rekey);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, spi_a, spi_b);
	assert_hook();

	/* IKE_FOLLOWUP_KE { KEi, N(ADD_KE) } --> */
	assert_hook_not_called(child_rekey);
	assert_payload(IN, PLV2_KEY_EXCHANGE);
	assert_notify(IN, ADDITIONAL_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, spi_a, spi_b, 5);
	assert_hook();

	/* <-- IKE_FOLLOWUP_KE { KEr } */
	assert_hook_rekey(child_rekey, spi_a, 4);
	assert_payload(IN, PLV2_KEY_EXCHANGE);
	assert_no_notify(IN, ADDITIONAL_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, spi_a, 4, 5);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_rekey(child_rekey, spi_b, 5);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, spi_b, 4, 5);
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_rekey);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 4, CHILD_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, spi_a, 4, 5);
	assert_hook();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, spi_a);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 4, 5);
	destroy_rekeyed(b, spi_b);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 4, 5);

	/* child_updown */
	assert_hook();

	/* because the DH group should get reused another rekeying should complete
	 * without additional exchange */
	initiate_rekey(a, 4);
	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 5, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 4, 5);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } */
	assert_hook_not_called(child_rekey);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 4, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 4, 5);
	assert_hook();

	/* IKE_FOLLOWUP_KE { KEi, N(ADD_KE) } --> */
	assert_hook_not_called(child_rekey);
	assert_payload(IN, PLV2_KEY_EXCHANGE);
	assert_notify(IN, ADDITIONAL_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 5, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 7, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, 4, 5, 7);
	assert_hook();

	/* <-- IKE_FOLLOWUP_KE { KEr } */
	assert_hook_called(child_rekey);
	assert_payload(IN, PLV2_KEY_EXCHANGE);
	assert_no_notify(IN, ADDITIONAL_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 4, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 4, 6, 7);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_called(child_rekey);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 5, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 7, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, 5, 6, 7);
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_rekey);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 4, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 6, CHILD_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 4, 6, 7);
	assert_hook();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, 4);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 6, 7);
	destroy_rekeyed(b, 5);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 6, 7);

	/* child_updown */
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Check that the responder ignores soft expires while waiting for the delete
 * after a rekeying.
 */
START_TEST(test_regular_responder_ignore_soft_expire)
{
	ike_sa_t *a, *b;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYED);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, 1, 2, 4);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } */
	assert_hook_rekey(child_rekey, 1, 3);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 1, 3, 4);
	assert_hook();

	/* this should not produce a message, if it does there won't be a delete
	 * payload below */
	call_ikesa(b, rekey_child_sa, PROTO_ESP, 2);
	assert_child_sa_state(b, 2, CHILD_REKEYED);

	/* INFORMATIONAL { D } --> */
	assert_hook_rekey(child_rekey, 2, 4);
	assert_jobs_scheduled(1);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, 2, 3, 4);
	assert_scheduler();
	assert_hook();

	/* we don't expect this to get called anymore */
	assert_hook_not_called(child_rekey);
	/* <-- INFORMATIONAL { D } */
	assert_jobs_scheduled(1);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 3, CHILD_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 1, 3, 4);
	assert_scheduler();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, 1);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 3, 4);
	destroy_rekeyed(b, 2);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 3, 4);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Check that the responder handles hard expires properly while waiting for the
 * delete after a rekeying (e.g. if the rekey settings are tight or the
 * CREATE_CHILD_SA response is delayed).
 */
START_TEST(test_regular_responder_handle_hard_expire)
{
	ike_sa_t *a, *b;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYED);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, 1, 2, 4);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } */
	assert_hook_rekey(child_rekey, 1, 3);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 1, 3, 4);
	assert_hook();

	/* this is similar to a regular delete collision, but we don't actually
	 * want to send a delete as that might conflict with a delayed
	 * CREATE_CHILD_SA response and the peer is expected to delete it anyway */
	assert_hook_rekey(child_rekey, 2, 4);
	call_ikesa(b, delete_child_sa, PROTO_ESP, 2, TRUE);
	assert_child_sa_count(b, 1);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	/* the expire causes the outbound SA to get installed */
	assert_ipsec_sas_installed(b, 3, 4);
	assert_hook();

	/* we don't expect this to get called anymore */
	assert_hook_not_called(child_rekey);
	/* INFORMATIONAL { D } --> */
	assert_no_jobs_scheduled();
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 3, 4);
	assert_scheduler();
	/* <-- INFORMATIONAL { } */
	assert_jobs_scheduled(1);
	assert_message_empty(IN);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 1, 3, 4);
	assert_scheduler();

	/* simulate the execution of the scheduled job */
	destroy_rekeyed(a, 1);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 3, 4);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Check that the responder and initiator handle deletes for the new SA
 * properly while waiting for the delete after a rekeying (e.g. if a script or
 * user deletes the new, not fully installed, SA manually).
 */
START_TEST(test_regular_responder_delete)
{
	ike_sa_t *a, *b;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);

	/* this should not get called until the new SA is deleted */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, 1, 2, 4);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } */
	assert_hook_rekey(child_rekey, 1, 3);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 1, 3, 4);
	assert_hook();

	assert_hook_rekey(child_rekey, 2, 4);
	call_ikesa(b, delete_child_sa, PROTO_ESP, 4, FALSE);
	assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	/* the delete causes the outbound SA to get installed/uninstalled */
	assert_ipsec_sas_installed(b, 2, 3, 4);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_not_called(child_rekey);
	assert_jobs_scheduled(1);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETED);
	assert_child_sa_state(b, 4, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 2, 3, 4);
	assert_scheduler();
	assert_hook();

	/* child_updown */
	assert_hook();

	/* we don't expect this to get called anymore */
	assert_hook_not_called(child_rekey);

	/* <-- INFORMATIONAL { D } */
	assert_hook_updown(child_updown, FALSE);
	assert_no_jobs_scheduled();
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 1);
	assert_scheduler();
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_updown);
	assert_jobs_scheduled(1);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 1);
	assert_scheduler();
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(child_updown, FALSE);
	assert_no_jobs_scheduled();
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETED);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 2);
	assert_scheduler();
	assert_hook();

	/* we don't expect this to get called anymore */
	assert_hook_not_called(child_updown);

	/* simulate the execution of the scheduled job */
	destroy_rekeyed(a, 1);
	assert_child_sa_count(a, 0);
	assert_ipsec_sas_installed(a);
	destroy_rekeyed(b, 2);
	assert_child_sa_count(b, 0);
	assert_ipsec_sas_installed(b);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 0);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * This simulates what happens if the responder for some reason lost the
 * CHILD_SA the initiator is trying to rekey.
 */
START_TEST(test_regular_responder_lost_sa)
{
	ike_sa_t *a, *b;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);

	/* destroy the CHILD_SA on the responder without notification */
	call_ikesa(b, destroy_child_sa, PROTO_ESP, 2);
	assert_child_sa_count(b, 0);

	/* this should never get called as there is no successful rekeying on
	 * either side */
	assert_hook_not_called(child_rekey);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_notify(IN, REKEY_SA);
	assert_single_notify(OUT, CHILD_SA_NOT_FOUND);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);

	/* <-- CREATE_CHILD_SA { N(NO_CHILD_SA) } */
	assert_hook_updown(child_updown, FALSE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 0);
	assert_ipsec_sas_installed(a);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_updown(child_updown, TRUE);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 4, 5);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Ni, [KEi,] TSi, TSr } */
	assert_hook_updown(child_updown, TRUE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 4, 5);
	assert_hook();

	/* child_rekey */
	assert_hook();
	/* the additional CHILD_SA here is the one we destroyed on b without
	 * triggering an event */
	assert_track_sas(2, 3);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Helper to add DELETE payload.
 */
typedef struct {
	listener_t listener;
	uint32_t spi;
} incorrect_delete_listener_t;

/**
 * Add a DELETE payload to a message.
 */
static bool add_delete(incorrect_delete_listener_t *listener, ike_sa_t *ike_sa,
					   message_t *message, bool incoming, bool plain)
{
	delete_payload_t *payload;

	if (plain && !incoming && message->get_request(message))
	{
		payload = delete_payload_create(PLV2_DELETE, PROTO_ESP);
		payload->add_spi(payload, listener->spi);
		message->add_payload(message, (payload_t*)payload);
		return FALSE;
	}
	return TRUE;
}

/**
 * Send a DELETE for the given SPI from an SA.
 */
static void send_child_delete(ike_sa_t *sa, uint32_t spi)
{
	incorrect_delete_listener_t del = {
		.listener = { .message = (void*)add_delete, },
		.spi = spi,
	};

	exchange_test_helper->add_listener(exchange_test_helper, &del.listener);
	call_ikesa(sa, send_dpd);
}

/**
 * This simulates incorrect behavior by some IKEv2 responders, which send
 * a delete for the old CHILD_SA even if there was no collision (don't know
 * how they'd behave if there was a collision, maybe they'd send two).
 * This is an issue if the DELETE arrives before the CREATE_CHILD_SA response.
 */
START_TEST(test_regular_responder_incorrect_delete)
{
	ike_sa_t *a, *b;
	message_t *msg;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYED);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, 1, 2, 4);
	assert_hook();

	/* delay the CREATE_CHILD_SA response */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* inject an incorrect delete for the old CHILD_SA by the responder,
	 * without messing with its internal state */
	send_child_delete(b, 2);

	/* <-- INFORMATIONAL { D } (incorrect behavior!) */
	assert_hook_not_called(child_rekey);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 1, 2);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } (delayed) */
	assert_jobs_scheduled(1);
	assert_hook_rekey(child_rekey, 1, 3);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, msg);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 1, 3, 4);
	assert_hook();
	assert_scheduler();

	/* INFORMATIONAL { D } (response to incorrect DELETE) -->
	 * this is ignored here because the DPD task doesn't handle the DELETE, so
	 * we simulate handling of the delete via expire, does not delay destroy */
	assert_no_jobs_scheduled();
	assert_hook_rekey(child_rekey, 2, 4);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	call_ikesa(b, delete_child_sa, PROTO_ESP, 2, TRUE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 3, 4);
	assert_hook();
	assert_scheduler();

	/* we don't expect this to get called anymore */
	assert_hook_not_called(child_rekey);

	/* simulate the execution of the scheduled job */
	destroy_rekeyed(a, 1);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 3, 4);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 3, 4);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Both peers initiate the CHILD_SA rekeying concurrently and should handle
 * the collision properly depending on the nonces.
 */
START_TEST(test_collision)
{
	ike_sa_t *a, *b;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	/* When rekeyings collide we get two CHILD_SAs with a total of four nonces.
	 * The CHILD_SA with the lowest nonce SHOULD be deleted by the peer that
	 * created that CHILD_SA.  The replaced CHILD_SA is deleted by the peer that
	 * initiated the surviving SA.
	 * Four nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * CHILD_SA):
	 *   N1/3 -----\    /----- N2/4
	 *              \--/-----> N3/5
	 *   N4/6 <-------/ /----- ...
	 *   ...  -----\
	 * We test this four times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[4];
		/* SPIs of the deleted CHILD_SA (either redundant or replaced) */
		uint32_t spi_del_a, spi_del_b;
		/* SPIs of the kept CHILD_SA */
		uint32_t spi_a, spi_b;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF, 0xFF }, 3, 2, 6, 4 },
		{ { 0xFF, 0x00, 0xFF, 0xFF }, 1, 4, 3, 5 },
		{ { 0xFF, 0xFF, 0x00, 0xFF }, 3, 2, 6, 4 },
		{ { 0xFF, 0xFF, 0xFF, 0x00 }, 1, 4, 3, 5 },
	};

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b, 2);
	assert_ipsec_sas_installed(b, 1, 2);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, 1, 2, 5);
	assert_hook();
	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[3];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(a, 1, 2, 6);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } */
	if (data[_i].spi_del_a == 1)
	{
		assert_hook_rekey(child_rekey, 1, data[_i].spi_a);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_hook();
		assert_child_sa_state(a, data[_i].spi_del_b, CHILD_REKEYED,
							  CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(a, 1, 3, 5, 6);
	}
	else
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_hook();
		assert_child_sa_state(a, data[_i].spi_del_b, CHILD_REKEYED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
							  CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(a, 1, 2, 3, 6);
	}
	assert_child_sa_state(a, data[_i].spi_del_a, CHILD_DELETING,
						  CHILD_OUTBOUND_NONE);
	/* CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } --> */
	if (data[_i].spi_del_b == 2)
	{
		assert_hook_rekey(child_rekey, 2, data[_i].spi_b);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_hook();
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(b, 2, 4, 5, 6);
	}
	else
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_hook();
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(b, 1, 2, 4, 5);
	}
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETING,
						  CHILD_OUTBOUND_NONE);

	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	if (data[_i].spi_del_b == 2)
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_hook();
	}
	else
	{
		assert_hook_rekey(child_rekey, 2, 5);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_hook();
	}
	assert_scheduler();
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETING,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_del_a, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
						  CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 3);
	if (data[_i].spi_del_b == 2)
	{
		assert_ipsec_sas_installed(b, 2, 4, 5, 6);
	}
	else
	{
		assert_ipsec_sas_installed(b, 2, 3, 4, 5);
	}
	/* <-- INFORMATIONAL { D } */
	assert_jobs_scheduled(1);
	if (data[_i].spi_del_a == 1)
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_hook();
	}
	else
	{
		assert_hook_rekey(child_rekey, 1, 6);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_hook();
	}
	assert_scheduler();
	assert_child_sa_state(a, data[_i].spi_del_a, CHILD_DELETING,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
						  CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 3);
	if (data[_i].spi_del_a == 1)
	{
		assert_ipsec_sas_installed(a, 1, 3, 5, 6);
	}
	else
	{
		assert_ipsec_sas_installed(a, 1, 3, 4, 6);
	}

	/* we don't expect this to get called anymore */
	assert_hook_not_called(child_rekey);

	/* <-- INFORMATIONAL { D } */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, data[_i].spi_del_a, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
						  CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 3);
	assert_ipsec_sas_installed(a, 1, 3, 6,
							   data[_i].spi_del_a == 1 ? 5 : 4);
	assert_scheduler();
	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_del_a, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
						  CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 3);
	assert_ipsec_sas_installed(b, 2, 4, 5,
							   data[_i].spi_del_b == 2 ? 6 : 3);
	assert_scheduler();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, data[_i].spi_del_a);
	destroy_rekeyed(a, data[_i].spi_del_b);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, data[_i].spi_a, data[_i].spi_b);
	destroy_rekeyed(b, data[_i].spi_del_a);
	destroy_rekeyed(b, data[_i].spi_del_b);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, data[_i].spi_a, data[_i].spi_b);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Both peers initiate a multi-KE CHILD_SA rekeying concurrently and should
 * handle the collision properly depending on the nonces.
 */
START_TEST(test_collision_multi_ke)
{
	ike_sa_t *a, *b;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, &multi_ke_conf);

	/* When rekeyings collide we get two CHILD_SAs with a total of four nonces.
	 * The CHILD_SA with the lowest nonce SHOULD be deleted by the peer that
	 * created that CHILD_SA.  However, with multiple key exchanges, no CHILD_SA
	 * has yet been established, so the losing peer just doesn't continue with
	 * IKE_FOLLOWUP_KE exchanges (i.e. that SA is not explicitly deleted later).
	 * The replaced CHILD_SA is deleted by the peer that initiated the
	 * surviving SA.  Four nonces and SPIs are needed (SPI 1 and 2 are used for
	 * the initial CHILD_SA):
	 *   N1/3 -----\    /----- N2/4
	 *              \--/-----> N3/5
	 *   N4/6 <-------/ /----- ...
	 *   ...  -----\
	 * We test this four times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[4];
		/* SPIs of the deleted CHILD_SA (either redundant or replaced) */
		uint32_t spi_del_a, spi_del_b;
		/* SPIs of the kept CHILD_SA */
		uint32_t spi_a, spi_b;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF, 0xFF }, 3, 2, 6, 4 },
		{ { 0xFF, 0x00, 0xFF, 0xFF }, 1, 4, 3, 5 },
		{ { 0xFF, 0xFF, 0x00, 0xFF }, 3, 2, 6, 4 },
		{ { 0xFF, 0xFF, 0xFF, 0x00 }, 1, 4, 3, 5 },
	};

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b, 2);
	assert_ipsec_sas_installed(b, 1, 2);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, KEi, TSi, TSr } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 1, 2);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, KEi, TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[3];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 1, 2);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, KEr, TSi, TSr, N(ADD_KE) } */
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	/* if a won, it must remove the passive task, otherwise the active task,
	 * no new SA is yet created */
	assert_num_tasks(a, data[_i].spi_del_a == 1 ? 0 : 1, TASK_QUEUE_PASSIVE);
	assert_num_tasks(a, data[_i].spi_del_a == 1 ? 1 : 0, TASK_QUEUE_ACTIVE);
	assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 1, 2);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Nr, KEr, TSi, TSr, N(ADD_KE) } --> */
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_num_tasks(b, data[_i].spi_del_b == 2 ? 0 : 1, TASK_QUEUE_PASSIVE);
	assert_num_tasks(b, data[_i].spi_del_b == 2 ? 1 : 0, TASK_QUEUE_ACTIVE);
	assert_child_sa_state(b, 2, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 1, 2);
	assert_hook();

	if (data[_i].spi_del_a == 1)
	{
		/* IKE_FOLLOWUP_KE { KEi, N(ADD_KE) } --> */
		assert_hook_not_called(child_rekey);
		assert_payload(IN, PLV2_KEY_EXCHANGE);
		assert_notify(IN, ADDITIONAL_KEY_EXCHANGE);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(b, 1, 2, 5);
		assert_hook();

		/* <-- IKE_FOLLOWUP_KE { KEr } */
		assert_hook_rekey(child_rekey, 1, data[_i].spi_a);
		assert_payload(IN, PLV2_KEY_EXCHANGE);
		assert_no_notify(IN, ADDITIONAL_KEY_EXCHANGE);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_DELETING, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(a, 1, 3, 5);
		assert_hook();
	}
	else
	{
		/* <-- IKE_FOLLOWUP_KE { KEi, N(ADD_KE) } */
		assert_hook_not_called(child_rekey);
		assert_payload(IN, PLV2_KEY_EXCHANGE);
		assert_notify(IN, ADDITIONAL_KEY_EXCHANGE);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(a, 1, 2, 6);
		assert_hook();

		/* IKE_FOLLOWUP_KE { KEr } --> */
		assert_hook_rekey(child_rekey, 2, data[_i].spi_b);
		assert_payload(IN, PLV2_KEY_EXCHANGE);
		assert_no_notify(IN, ADDITIONAL_KEY_EXCHANGE);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, 2, CHILD_DELETING, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(b, 2, 4, 6);
		assert_hook();
	}

	if (data[_i].spi_del_a == 1)
	{
		/* INFORMATIONAL { D } --> */
		assert_hook_rekey(child_rekey, 2, 5);
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(b, 2);
		assert_ipsec_sas_installed(b, 2, 3, 5);
		assert_scheduler();
		assert_hook();

		/* <-- INFORMATIONAL { D } */
		assert_hook_not_called(child_rekey);
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(a, 2);
		assert_ipsec_sas_installed(a, 1, 3, 5);
		assert_scheduler();
		assert_hook();
	}
	else
	{
		/* <-- INFORMATIONAL { D } */
		assert_hook_rekey(child_rekey, 1, 6);
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(a, 2);
		assert_ipsec_sas_installed(a, 1, 4, 6);
		assert_scheduler();
		assert_hook();

		/* INFORMATIONAL { D } --> */
		assert_hook_not_called(child_rekey);
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(b, 2);
		assert_ipsec_sas_installed(b, 2, 4, 6);
		assert_scheduler();
		assert_hook();
	}

	/* we don't expect this hook to get called anymore */
	assert_hook_not_called(child_rekey);

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, data[_i].spi_del_a == 1 ? data[_i].spi_del_a
											   : data[_i].spi_del_b);
	destroy_rekeyed(b, data[_i].spi_del_a == 1 ? data[_i].spi_del_a
											   : data[_i].spi_del_b);

	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, data[_i].spi_a, data[_i].spi_b);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, data[_i].spi_a, data[_i].spi_b);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Both peers initiate a CHILD_SA rekeying concurrently, but only one of them
 * proposes multiple key exchanges, they should still handle the collision
 * properly.
 */
START_TEST(test_collision_mixed)
{
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.esp = "aes256-sha256-modp3072-ke1_ecp256,aes256-sha256-modp3072",
		},
		.responder = {
			.esp = "aes256-sha256-modp3072,aes256-sha256-modp3072-ke1_ecp256",
		},
	};
	ike_sa_t *a, *b;

	assert_track_sas_start();

	/* let's accept what the peer proposes first */
	lib->settings->set_bool(lib->settings, "%s.prefer_configured_proposals",
							FALSE, lib->ns);

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, &conf);

	/* When rekeyings collide we get two CHILD_SAs with a total of four nonces.
	 * The CHILD_SA with the lowest nonce SHOULD be deleted by the peer that
	 * created that CHILD_SA.  However, with multiple key exchanges, no CHILD_SA
	 * has yet been established, so the losing peer just doesn't continue with
	 * IKE_FOLLOWUP_KE exchanges (i.e. that SA is not explicitly deleted later).
	 * The replaced CHILD_SA is deleted by the peer that initiated the
	 * surviving SA.  Four nonces and SPIs are needed (SPI 1 and 2 are used for
	 * the initial CHILD_SA):
	 *   N1/3 -----\    /----- N2/4
	 *              \--/-----> N3/5
	 *   N4/6 <-------/ /----- ...
	 *   ...  -----\
	 * We test this four times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[4];
		/* SPIs of the deleted CHILD_SA (either redundant or replaced) */
		uint32_t spi_del_a, spi_del_b;
		/* SPIs of the kept CHILD_SA */
		uint32_t spi_a, spi_b;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF, 0xFF }, 3, 2, 6, 4 },
		{ { 0xFF, 0x00, 0xFF, 0xFF }, 1, 4, 3, 5 },
		{ { 0xFF, 0xFF, 0x00, 0xFF }, 3, 2, 6, 4 },
		{ { 0xFF, 0xFF, 0xFF, 0x00 }, 1, 4, 3, 5 },
	};

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b, 2);
	assert_ipsec_sas_installed(b, 1, 2);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, KEi, TSi, TSr } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 1, 2);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, KEi, TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[3];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(a, 1, 2, 6);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, KEr, TSi, TSr, N(ADD_KE) } */
	if (data[_i].spi_del_a == 1)
	{	/* a's multi-KE SA is the winner */
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		/* the single-KE passive task was completed and adopted */
		assert_num_tasks(a, 0, TASK_QUEUE_PASSIVE);
		assert_num_tasks(a, 1, TASK_QUEUE_ACTIVE);
		assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, 6, CHILD_REKEYED, CHILD_OUTBOUND_NONE);
		assert_hook();
	}
	else
	{	/* b's single-KE SA is the winner */
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		/* the single-KE passive task was completed and adopted */
		assert_num_tasks(a, 0, TASK_QUEUE_PASSIVE);
		assert_num_tasks(a, 0, TASK_QUEUE_ACTIVE);
		assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
		assert_hook();
	}
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 1, 2, 6);

	if (data[_i].spi_del_a == 1)
	{
		/* CREATE_CHILD_SA { SA, Nr, KEr, TSi, TSr, N(ADD_KE) } --> */
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_num_tasks(b, 1, TASK_QUEUE_PASSIVE);
		assert_num_tasks(b, 1, TASK_QUEUE_ACTIVE);
		assert_child_sa_state(b, 2, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(b, 4, CHILD_DELETING, CHILD_OUTBOUND_NONE);
		assert_child_sa_count(b, 2);
		assert_ipsec_sas_installed(b, 1, 2, 4);
		assert_hook();

		/* IKE_FOLLOWUP_KE { KEi, N(ADD_KE) } --> */
		assert_hook_not_called(child_rekey);
		assert_payload(IN, PLV2_KEY_EXCHANGE);
		assert_notify(IN, ADDITIONAL_KEY_EXCHANGE);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(b, 4, CHILD_DELETING, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
		assert_child_sa_count(b, 3);
		assert_ipsec_sas_installed(b, 1, 2, 4, 5);
		assert_hook();

		/* <-- INFORMATIONAL { D } */
		assert_hook_not_called(child_rekey);
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, 6, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_count(a, 2);
		assert_ipsec_sas_installed(a, 1, 2, 6);
		assert_scheduler();
		assert_hook();

		/* <-- IKE_FOLLOWUP_KE { KEr } */
		assert_hook_rekey(child_rekey, 1, data[_i].spi_a);
		assert_payload(IN, PLV2_KEY_EXCHANGE);
		assert_no_notify(IN, ADDITIONAL_KEY_EXCHANGE);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_DELETING, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, 6, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_ipsec_sas_installed(a, 1, 3, 5, 6);
		assert_hook();
	}
	else
	{
		/* CREATE_CHILD_SA { SA, Nr, KEr, TSi, TSr, N(ADD_KE) } --> */
		assert_hook_rekey(child_rekey, 2, 4);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_num_tasks(b, 0, TASK_QUEUE_PASSIVE);
		assert_num_tasks(b, 1, TASK_QUEUE_ACTIVE);
		assert_child_sa_state(b, 2, CHILD_DELETING, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(a, 2);
		assert_ipsec_sas_installed(b, 2, 4, 6);
		assert_hook();
	}

	if (data[_i].spi_del_a == 1)
	{
		/* INFORMATIONAL { D } --> */
		assert_hook_not_called(child_rekey);
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(b, 4, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
		assert_child_sa_count(b, 3);
		assert_ipsec_sas_installed(b, 1, 2, 4, 5);
		assert_scheduler();
		assert_hook();


		/* INFORMATIONAL { D } --> */
		assert_hook_rekey(child_rekey, 2, 5);
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, 4, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(b, 3);
		assert_ipsec_sas_installed(b, 2, 4, 3, 5);
		assert_scheduler();
		assert_hook();

		assert_hook_not_called(child_rekey);

		/* <-- INFORMATIONAL { D } */
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, 6, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_count(a, 3);
		assert_ipsec_sas_installed(a, 1, 3, 5, 6);
		assert_scheduler();

		/* simulate the execution of the scheduled jobs */
		destroy_rekeyed(a, data[_i].spi_del_a);
		destroy_rekeyed(a, data[_i].spi_del_b);
		destroy_rekeyed(b, data[_i].spi_del_a);
		destroy_rekeyed(b, data[_i].spi_del_b);

		assert_hook();
	}
	else
	{
		/* <-- INFORMATIONAL { D } */
		assert_hook_rekey(child_rekey, 1, 6);
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(a, 2);
		assert_ipsec_sas_installed(a, 1, 4, 6);
		assert_scheduler();
		assert_hook();

		assert_hook_not_called(child_rekey);

		/* INFORMATIONAL { D } --> */
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(b, 2);
		assert_ipsec_sas_installed(b, 2, 4, 6);
		assert_scheduler();

		/* simulate the execution of the scheduled jobs */
		destroy_rekeyed(a, data[_i].spi_del_b);
		destroy_rekeyed(b, data[_i].spi_del_b);

		assert_hook();
	}

	/* we don't expect this hook to get called anymore */
	assert_hook_not_called(child_rekey);

	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, data[_i].spi_a, data[_i].spi_b);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, data[_i].spi_a, data[_i].spi_b);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * This is like the rekey collision above, but one peer deletes the
 * redundant/old SA before the other peer receives the CREATE_CHILD_SA
 * response:
 *
 *            rekey ----\       /---- rekey
 *                       \-----/----> detect collision
 * detect collision <---------/ /----
 *                  ----\      /
 *                       \----/----->
 *    handle delete <--------/------- delete SA
 *                  --------/------->
 *     handle rekey <------/
 *        delete SA ---------------->
 *                  <----------------
 */
START_TEST(test_collision_delayed_response)
{
	ike_sa_t *a, *b;
	message_t *msg;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	/* Four nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * CHILD_SA):
	 *   N1/3 -----\    /----- N2/4
	 *              \--/-----> N3/5
	 *   N4/6 <-------/ /----- ...
	 *   ...  -----\
	 * We test this four times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[4];
		/* SPIs of the deleted CHILD_SA (either redundant or replaced) */
		uint32_t spi_del_a, spi_del_b;
		/* SPIs of the kept CHILD_SA */
		uint32_t spi_a, spi_b;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF, 0xFF }, 3, 2, 6, 4 },
		{ { 0xFF, 0x00, 0xFF, 0xFF }, 1, 4, 3, 5 },
		{ { 0xFF, 0xFF, 0x00, 0xFF }, 3, 2, 6, 4 },
		{ { 0xFF, 0xFF, 0xFF, 0x00 }, 1, 4, 3, 5 },
	};

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b, 2);
	assert_ipsec_sas_installed(b, 1, 2);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, 1, 2, 5);
	assert_hook();
	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[3];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(a, 1, 2, 6);
	assert_hook();

	/* delay the CREATE_CHILD_SA response from b to a */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } --> */
	if (data[_i].spi_del_b == 2)
	{
		assert_hook_rekey(child_rekey, 2, data[_i].spi_b);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(b, 2, 4, 5, 6);
		assert_hook();
	}
	else
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(b, 1, 2, 4, 5);
		assert_hook();
	}
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETING,
						  CHILD_OUTBOUND_NONE);

	/* <-- INFORMATIONAL { D } */
	assert_no_jobs_scheduled();
	if (data[_i].spi_del_b == 2)
	{
		assert_hook_rekey(child_rekey, 1, 6);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(a, 1, 4, 6);
		assert_hook();
	}
	else
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, data[_i].spi_del_b, CHILD_DELETING,
							  CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(a, 1, 2, 6);
		assert_hook();
	}
	assert_scheduler();
	assert_child_sa_count(a, 2);
	/* INFORMATIONAL { D } --> */
	assert_hook_not_called(child_rekey);
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	if (data[_i].spi_del_b == 2)
	{
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(b, 2, 4, 5, 6);
	}
	else
	{
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(b, 1, 2, 4, 5);
	}
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_count(b, 3);
	assert_scheduler();
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } (delayed) */
	/* the second job here is for the retransmit of the delete */
	assert_jobs_scheduled(2);
	if (data[_i].spi_del_a == 1)
	{
		assert_hook_rekey(child_rekey, 1, data[_i].spi_a);
		exchange_test_helper->process_message(exchange_test_helper, a, msg);
		assert_hook();
	}
	else
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, a, msg);
		assert_hook();
	}
	assert_scheduler();
	assert_child_sa_state(a, data[_i].spi_del_a, CHILD_DELETING,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
						  CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 3);
	assert_ipsec_sas_installed(a, 1, 3, 6,
							   data[_i].spi_del_a == 1 ? 5 : 4);

	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	if (data[_i].spi_del_b == 2)
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_hook();
	}
	else
	{
		assert_hook_rekey(child_rekey, 2, 5);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_hook();
	}
	assert_child_sa_state(b, data[_i].spi_del_a, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
						  CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 2, 4, 5,
							   data[_i].spi_del_b == 2 ? 6 : 3);
	assert_child_sa_count(b, 3);
	assert_scheduler();

	/* we don't expect this hook to get called anymore */
	assert_hook_not_called(child_rekey);
	/* <-- INFORMATIONAL { D } */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, data[_i].spi_del_a, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
						  CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 1, 3, 6,
							   data[_i].spi_del_a == 1 ? 5 : 4);
	assert_child_sa_count(a, 3);
	assert_scheduler();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, data[_i].spi_del_a);
	destroy_rekeyed(a, data[_i].spi_del_b);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, data[_i].spi_a, data[_i].spi_b);
	destroy_rekeyed(b, data[_i].spi_del_a);
	destroy_rekeyed(b, data[_i].spi_del_b);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, data[_i].spi_a, data[_i].spi_b);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * This is like the rekey collision above, but one peer deletes the
 * redundant/old SA and then also the new one before the other peer receives
 * the CREATE_CHILD_SA response:
 *
 *            rekey ----\       /---- rekey
 *                       \-----/----> detect collision
 * detect collision <---------/ /----
 *                  ----\      /
 *                       \----/----->
 *    handle delete <--------/------- delete old SA
 *                  --------/------->
 *    handle delete <------/--------- delete new SA
 *                  ------/--------->
 *     ignore rekey <----/
 */
START_TEST(test_collision_delayed_response_delete)
{
	ike_sa_t *a, *b;
	message_t *msg;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	/* Four nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * CHILD_SA):
	 *   N1/3 -----\    /----- N2/4
	 *              \--/-----> N3/5
	 *   N4/6 <-------/ /----- ...
	 *   ...  -----\
	 * We test this four times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[4];
		/* SPIs of the deleted CHILD_SA (either redundant or replaced) */
		uint32_t spi_del_a, spi_del_b;
		/* SPIs of the kept CHILD_SA */
		uint32_t spi_a, spi_b;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF, 0xFF }, 3, 2, 6, 4 },
		{ { 0xFF, 0x00, 0xFF, 0xFF }, 1, 4, 3, 5 },
		{ { 0xFF, 0xFF, 0x00, 0xFF }, 3, 2, 6, 4 },
		{ { 0xFF, 0xFF, 0xFF, 0x00 }, 1, 4, 3, 5 },
	};

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b, 2);
	assert_ipsec_sas_installed(b, 1, 2);

	/* this should not get called until the replacement SA is deleted */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, 1, 2, 5);
	assert_hook();
	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[3];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(a, 1, 2, 6);
	assert_hook();

	/* delay the CREATE_CHILD_SA response from b to a */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } --> */
	if (data[_i].spi_del_b == 2)
	{
		assert_hook_rekey(child_rekey, 2, data[_i].spi_b);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(b, 2, 4, 5, 6);
		assert_hook();
	}
	else
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(b, 1, 2, 4, 5);
		assert_hook();
	}
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETING,
						  CHILD_OUTBOUND_NONE);

	/* <-- INFORMATIONAL { D } */
	if (data[_i].spi_del_b == 2)
	{
		assert_hook_rekey(child_rekey, 1, 6);
		assert_no_jobs_scheduled();
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(a, 1, 4, 6);
		assert_scheduler();
		assert_hook();
	}
	else
	{
		assert_hook_not_called(child_rekey);
		assert_no_jobs_scheduled();
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, data[_i].spi_del_b, CHILD_DELETING,
							  CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(a, 1, 2, 6);
		assert_scheduler();
		assert_hook();
	}
	assert_child_sa_count(a, 2);
	/* INFORMATIONAL { D } --> */
	assert_hook_not_called(child_rekey);
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	if (data[_i].spi_del_b == 2)
	{
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(b, 2, 4, 5, 6);
	}
	else
	{
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(b, 1, 2, 4, 5);
	}
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_count(b, 3);
	assert_scheduler();
	assert_hook();

	/* trigger a delete for the new CHILD_SA */
	call_ikesa(b, delete_child_sa, PROTO_ESP, data[_i].spi_b, FALSE);
	assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_b, CHILD_DELETING,
						  CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 3);
	assert_ipsec_sas_installed(b, 2, 4, 5,
							   data[_i].spi_del_b == 2 ? 6 : 3);

	/* child_updown */
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_rekey);
	assert_hook_not_called(child_updown);
	if (data[_i].spi_del_b == 2)
	{
		assert_no_jobs_scheduled();
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, data[_i].spi_a, CHILD_DELETING,
							  CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(a, 1, 4, 6);
		assert_scheduler();
	}
	else
	{
		assert_no_jobs_scheduled();
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, data[_i].spi_del_b, CHILD_DELETING,
							  CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(a, 1, 2, 6);
		assert_scheduler();
	}
	assert_child_sa_count(a, 2);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(child_updown, FALSE);
	assert_no_jobs_scheduled();
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, 2, data[_i].spi_del_b == 2 ? 5 : 4);
	assert_scheduler();
	assert_hook();
	/* child_rekey */
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } (delayed) */
	assert_hook_updown(child_updown, FALSE);
	if (data[_i].spi_del_a == 1)
	{
		assert_hook_rekey(child_rekey, 1, 3);
		exchange_test_helper->process_message(exchange_test_helper, a, msg);
		assert_child_sa_state(a, data[_i].spi_del_a, CHILD_DELETING,
							  CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(a, 1, 2, 6);
		assert_hook();
	}
	else
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, a, msg);
		assert_child_sa_state(a, data[_i].spi_del_a, CHILD_DELETING,
							  CHILD_OUTBOUND_NONE);
		assert_ipsec_sas_installed(a, 1, 3);
		assert_hook();
	}
	assert_child_sa_state(a, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_count(a, 2);
	assert_hook();

	/* we don't expect these hooks to get called anymore */
	assert_hook_not_called(child_updown);
	assert_hook_not_called(child_rekey);
	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, data[_i].spi_del_a, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_ipsec_sas_installed(b, 2, data[_i].spi_del_b == 2 ? 5 : 4);
	assert_child_sa_count(b, 2);
	assert_scheduler();
	/* <-- INFORMATIONAL { D } */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, data[_i].spi_del_a, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 1, data[_i].spi_del_a == 1 ? 6 : 3);
	assert_scheduler();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, data[_i].spi_del_a);
	destroy_rekeyed(a, data[_i].spi_del_b);
	assert_child_sa_count(a, 0);
	assert_ipsec_sas_installed(a);
	destroy_rekeyed(b, data[_i].spi_del_a);
	destroy_rekeyed(b, data[_i].spi_del_b);
	assert_child_sa_count(b, 0);
	assert_ipsec_sas_installed(b);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 0);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * This is like a regular rekey collision, but one CREATE_CHILD_SA response
 * is delayed:
 *           Peer A                   Peer B
 *            rekey ----\       /---- rekey
 *                       \-----/----> detect collision
 * detect collision <---------/ /----
 *                  -----------/---->
 *        handle KE <---------/------ send additional KE (if won)
 *                  ---------/------>
 *     handle rekey <-------/
 *    additional KE ----------------> handle KE (if lost)
 *                  <----------------
 *                         ...        the winner deletes the old SA
 *
 * If A wins the collision, this is just a regular collision as B will simply
 * abort its own rekeying and wait until A receives the response and continues
 * with its IKE_FOLLOWUP_KE request.  So we only look at the cases in which
 * B wins.
 *
 * Besides the scenario depicted above, i.e. where the response arrives after
 * handling B's IKE_FOLLOWUP_KE request, we also test when it arrives after
 * handling the delete.
 */
START_TEST(test_collision_delayed_response_multi_ke)
{
	ike_sa_t *a, *b;
	message_t *msg;
	bool after_delete = _i >= 2;

	_i %= 2;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, &multi_ke_conf);

	/* Four nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * CHILD_SA):
	 *   N1/3 -----\    /----- N2/4
	 *              \--/-----> N3/5
	 *   N4/6 <-------/ /----- ...
	 *   ...  -----\
	 * We test this four times, B wins each time (with either of its nonces),
	 * but the response arrives at different times.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[4];
		/* SPIs of the deleted CHILD_SA (either redundant or replaced) */
		uint32_t spi_del_a, spi_del_b;
		/* SPIs of the kept CHILD_SA */
		uint32_t spi_a, spi_b;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF, 0xFF }, 3, 2, 6, 4 },
		{ { 0xFF, 0xFF, 0x00, 0xFF }, 3, 2, 6, 4 },
	};

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b, 2);
	assert_ipsec_sas_installed(b, 1, 2);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 1, 2);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[3];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 1, 2);
	assert_hook();

	/* delay the CREATE_CHILD_SA response from b to a */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_num_tasks(b, 0, TASK_QUEUE_PASSIVE);
	assert_num_tasks(b, 1, TASK_QUEUE_ACTIVE);
	assert_child_sa_state(b, 2, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 1, 2);
	assert_hook();

	/* <-- IKE_FOLLOWUP_KE { KEi, N(ADD_KE) } */
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(a, 1, 2, 6);
	assert_hook();

	/* IKE_FOLLOWUP_KE { KEr } --> */
	assert_hook_rekey(child_rekey, 2, 4);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 2, 4, 6);
	assert_hook();

	if (!after_delete)
	{	/* a receives the response right after the IKE_FOLLOWUP_KE, the passive
		 * rekeying is completed and the active aborted */
		/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } (delayed) */
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, a, msg);
		assert_num_tasks(a, 0, TASK_QUEUE_PASSIVE);
		assert_num_tasks(a, 0, TASK_QUEUE_ACTIVE);
		assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(a, 1, 2, 6);
		assert_hook();
	}

	/* <-- INFORMATIONAL { D } */
	assert_hook_rekey(child_rekey, 1, 6);
	assert_jobs_scheduled(after_delete ? 0 : 1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 1, 4, 6);
	assert_child_sa_count(a, 2);
	assert_scheduler();
	assert_hook();

	/* we don't expect this hook to get called anymore */
	assert_hook_not_called(child_rekey);

	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 2, 4, 6);
	assert_child_sa_count(b, 2);
	assert_scheduler();

	if (after_delete)
	{
		/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } (delayed) */
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, a, msg);
		assert_num_tasks(a, 0, TASK_QUEUE_PASSIVE);
		assert_num_tasks(a, 0, TASK_QUEUE_ACTIVE);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_ipsec_sas_installed(a, 1, 4, 6);
		assert_scheduler();
	}

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, data[_i].spi_del_b);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, data[_i].spi_a, data[_i].spi_b);
	destroy_rekeyed(b, data[_i].spi_del_b);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, data[_i].spi_a, data[_i].spi_b);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * In this scenario one of the peers does not notice that there is a
 * rekey collision:
 *
 *            rekey ----\       /---- rekey
 *                       \     /
 * detect collision <-----\---/
 *                  -------\-------->
 *                          \   /---- delete old SA
 *                           \-/----> detect collision
 *    handle delete <---------/ /---- TEMP_FAIL
 *                  -----------/---->
 *  aborts rekeying <---------/
 *
 * Besides the scenario depicted above, i.e. where the response arrives after
 * handling B's delete request, we also test when it arrives before that:
 *
 *                  ...
 *                          \   /---- delete old SA
 *                           \-/----> detect collision
 *  aborts rekeying <---------/------ TEMP_FAIL
 *    handle delete <--------/
 *                  ---------------->
 */
START_TEST(test_collision_delayed_request)
{
	ike_sa_t *a, *b;
	message_t *msg;
	bool before_delete = _i >= 3;

	_i %= 3;

	assert_track_sas_start();

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
	} data[] = {
		{ { 0x00, 0xFF, 0xFF } },
		{ { 0xFF, 0x00, 0xFF } },
		{ { 0xFF, 0xFF, 0x00 } },
	};

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b, 2);
	assert_ipsec_sas_installed(b, 1, 2);

	/* delay the CREATE_CHILD_SA request from a to b */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(a, 1, 2, 5);
	assert_hook();
	/* CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } --> */
	assert_hook_rekey(child_rekey, 2, 4);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 2, 4, 5);
	assert_hook();


	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> (delayed) */
	assert_hook_not_called(child_rekey);
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, msg);
	assert_child_sa_state(b, 2, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_hook();

	if (before_delete)
	{
		/* delay the DELETE request from b to a so TEMP_FAIL arrives before */
		msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);
	}
	else
	{
		/* <-- INFORMATIONAL { D } */
		assert_hook_rekey(child_rekey, 1, 5);
		assert_no_jobs_scheduled();
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(a, 2);
		assert_ipsec_sas_installed(a, 1, 4, 5);
		assert_scheduler();
		assert_hook();
	}

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	assert_hook_not_called(child_rekey);
	if (before_delete)
	{
		assert_no_jobs_scheduled();
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
		assert_child_sa_count(a, 2);
		assert_ipsec_sas_installed(a, 1, 2, 5);
		assert_scheduler();
	}
	else
	{
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(a, 2);
		assert_ipsec_sas_installed(a, 1, 4, 5);
		assert_scheduler();
	}
	assert_hook();

	if (before_delete)
	{
		/* <-- INFORMATIONAL { D } (delayed) */
		assert_hook_rekey(child_rekey, 1, 5);
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, a, msg);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(a, 2);
		assert_ipsec_sas_installed(a, 1, 4, 5);
		assert_scheduler();
		assert_hook();
	}

	/* we don't expect this hook to get called anymore */
	assert_hook_not_called(child_rekey);

	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, 2, 4, 5);
	assert_scheduler();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, 1);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 4, 5);
	destroy_rekeyed(b, 2);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 4, 5);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Similar to above one peer fails to notice the collision but the
 * CREATE_CHILD_SA request is even more delayed:
 *
 *            rekey ----\       /---- rekey
 *                       \     /
 * detect collision <-----\---/
 *                  -------\-------->
 * detect collision <-------\-------- delete old SA
 *           delete ---------\------>
 *                            \----->
 *                              /---- CHILD_SA_NOT_FOUND
 *  aborts rekeying <----------/
 */
START_TEST(test_collision_delayed_request_more)
{
	ike_sa_t *a, *b;
	message_t *msg;

	assert_track_sas_start();

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
	} data[] = {
		{ { 0x00, 0xFF, 0xFF } },
		{ { 0xFF, 0x00, 0xFF } },
		{ { 0xFF, 0xFF, 0x00 } },
	};

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b, 2);
	assert_ipsec_sas_installed(b, 1, 2);

	/* delay the CREATE_CHILD_SA request from a to b */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(a, 1, 2, 5);
	assert_hook();
	/* CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } --> */
	assert_hook_rekey(child_rekey, 2, 4);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 2, 4, 5);
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_rekey(child_rekey, 1, 5);
	assert_no_jobs_scheduled();
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 1, 4, 5);
	assert_scheduler();
	assert_hook();

	/* we don't expect this to get called anymore */
	assert_hook_not_called(child_rekey);

	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, 2, 4, 5);
	assert_scheduler();

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_single_notify(OUT, CHILD_SA_NOT_FOUND);
	exchange_test_helper->process_message(exchange_test_helper, b, msg);
	assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, 2, 4, 5);

	/* <-- CREATE_CHILD_SA { N(NO_CHILD_SA) } */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 1, 4, 5);
	assert_scheduler();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, 1);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 4, 5);
	destroy_rekeyed(b, 2);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 4, 5);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Similar to above one peer fails to notice the collision but the
 * CREATE_CHILD_SA request is even more delayed:
 *
 *            rekey ----\       /---- rekey
 *                       \     /
 * detect collision <-----\---/
 *                  -------\-------->
 *    handle delete <-------\-------- delete old SA
 *                  ---------\------>
 *    handle delete <---------\------ delete new SA
 *                  -----------\---->
 *                              \--->
 *                              /---- CHILD_SA_NOT_FOUND
 *  aborts rekeying <----------/
 */
START_TEST(test_collision_delayed_request_more_delete)
{
	ike_sa_t *a, *b;
	message_t *msg;

	assert_track_sas_start();

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
	} data[] = {
		{ { 0x00, 0xFF, 0xFF } },
		{ { 0xFF, 0x00, 0xFF } },
		{ { 0xFF, 0xFF, 0x00 } },
	};

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b, 2);
	assert_ipsec_sas_installed(b, 1, 2);

	/* delay the CREATE_CHILD_SA request from a to b */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* this should not get called until the new SA is deleted */
	assert_hook_not_called(child_updown);

	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(a, 1, 2, 5);
	assert_hook();
	/* CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } --> */
	assert_hook_rekey(child_rekey, 2, 4);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 2, 4, 5);
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_rekey(child_rekey, 1, 5);
	assert_no_jobs_scheduled();
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 1, 4, 5);
	assert_scheduler();
	assert_hook();

	/* child_updown() */
	assert_hook();

	/* we don't expect this to get called anymore */
	assert_hook_not_called(child_rekey);
	/* this is expected later */
	assert_hook_not_called(child_updown);


	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, 2, 4, 5);
	assert_scheduler();

	/* trigger a delete for the new CHILD_SA */
	call_ikesa(b, delete_child_sa, PROTO_ESP, 5, FALSE);

	/* <-- INFORMATIONAL { D } */
	assert_no_jobs_scheduled();
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 5, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 1, 4, 5);
	assert_scheduler();

	/* child_updown() */
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(child_updown, FALSE);
	assert_no_jobs_scheduled();
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 2);
	assert_scheduler();
	assert_hook();

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_updown);
	assert_single_notify(OUT, CHILD_SA_NOT_FOUND);
	exchange_test_helper->process_message(exchange_test_helper, b, msg);
	assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 2);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(NO_CHILD_SA) } */
	assert_hook_updown(child_updown, FALSE);
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 1);
	assert_scheduler();
	assert_hook();

	/* we don't expect this to get called anymore */
	assert_hook_not_called(child_updown);

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, 1);
	assert_child_sa_count(a, 0);
	assert_ipsec_sas_installed(a);
	destroy_rekeyed(b, 2);
	assert_child_sa_count(b, 0);
	assert_ipsec_sas_installed(b);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 0);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * In this scenario one of the peers does not notice that there is a
 * rekey collision:
 *
 *            rekey ----\       /---- rekey
 *                       \     /
 * detect collision <-----\---/
 *                  -------\-------->
 *                          \   /---- send additional KE
 *                           \-/----> detect collision
 *        handle KE <---------/ /---- TEMP_FAIL
 *                  -----------/---->
 *                  <---------/------ delete old SA
 *           delete ---------/------>
 *  aborts rekeying <-------/
 *
 * In a variation of this scenario, the TEMP_FAIL notify arrives before the
 * delete does, similar to the non-multi-KE scenario above.
 */
START_TEST(test_collision_delayed_request_multi_ke)
{
	ike_sa_t *a, *b;
	message_t *msg;
	bool after_delete = _i >= 3;

	_i %= 3;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, &multi_ke_conf);

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
	} data[] = {
		{ { 0x00, 0xFF, 0xFF } },
		{ { 0xFF, 0x00, 0xFF } },
		{ { 0xFF, 0xFF, 0x00 } },
	};

	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(b, 2);
	assert_ipsec_sas_installed(b, 1, 2);

	/* delay the CREATE_CHILD_SA request from a to b */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(a, 1, 2);
	assert_hook();

	/* CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr, N(ADD_KE) } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, ADDITIONAL_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 1, 2);
	assert_hook();

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> (delayed) */
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, msg);

	/* <-- IKE_FOLLOWUP_KE { KEi, N(ADD_KE) } */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, ADDITIONAL_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_num_tasks(a, 0, TASK_QUEUE_PASSIVE);
	assert_num_tasks(a, 1, TASK_QUEUE_ACTIVE);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(a, 1, 2, 5);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	if (!after_delete)
	{
		assert_hook_not_called(child_rekey);
		assert_no_jobs_scheduled();
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_num_tasks(a, 0, TASK_QUEUE_ACTIVE);
		assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
		assert_ipsec_sas_installed(a, 1, 2, 5);
		assert_scheduler();
		assert_hook();
	}
	else
	{	/* delay until we received the delete */
		msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);
	}

	/* IKE_FOLLOWUP_KE { KEr } --> */
	assert_hook_rekey(child_rekey, 2, 4);
	assert_payload(IN, PLV2_KEY_EXCHANGE);
	assert_no_notify(IN, ADDITIONAL_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, 2, 4, 5);
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_rekey(child_rekey, 1, 5);
	assert_jobs_scheduled(after_delete ? 0 : 1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 1, 4, 5);
	assert_scheduler();
	assert_hook();

	assert_hook_not_called(child_rekey);

	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, 2, 4, 5);
	assert_scheduler();

	if (after_delete)
	{
		/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
		assert_jobs_scheduled(1);
		exchange_test_helper->process_message(exchange_test_helper, a, msg);
		assert_num_tasks(a, 0, TASK_QUEUE_ACTIVE);
		assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_count(a, 2);
		assert_ipsec_sas_installed(a, 1, 4, 5);
		assert_scheduler();
	}

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, 1);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 4, 5);
	destroy_rekeyed(b, 2);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 4, 5);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * Both peers initiate the CHILD_SA reekying concurrently but the proposed DH
 * groups are not the same after handling the INVALID_KE_PAYLOAD they should
 * still handle the collision properly depending on the nonces.
 */
START_TEST(test_collision_ke_invalid)
{
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.esp = "aes128-sha256-modp2048-modp3072",
		},
		.responder = {
			.esp = "aes128-sha256-modp3072-modp2048",
		},
	};
	ike_sa_t *a, *b;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, &conf);

	/* Eight nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * CHILD_SA):
	 *     N1/3 -----\    /----- N2/4
	 *                \--/-----> N3/-
	 *     N4/- <-------/  /---- INVAL_KE
	 * INVAL_KE -----\    /
	 *          <-----\--/
	 *     N5/5 -----\ \------->
	 *                \    /---- N6/6
	 *                 \--/----> N7/7
	 *     N8/8 <--------/ /---- ...
	 *      ... ------\
	 *
	 * We test this four times, each time a different nonce is the lowest.
	 */
	struct {
		/* Nonces used at each point */
		u_char nonces[4];
		/* SPIs of the deleted CHILD_SA (either redundant or replaced) */
		uint32_t spi_del_a, spi_del_b;
		/* SPIs of the kept CHILD_SA */
		uint32_t spi_a, spi_b;
	} data[] = {
		{ { 0x00, 0xFF, 0xFF, 0xFF }, 5, 2, 8, 6 },
		{ { 0xFF, 0x00, 0xFF, 0xFF }, 1, 6, 5, 7 },
		{ { 0xFF, 0xFF, 0x00, 0xFF }, 5, 2, 8, 6 },
		{ { 0xFF, 0xFF, 0xFF, 0x00 }, 1, 6, 5, 7 },
	};

	/* make sure the nonces of the first try don't affect the retries */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(a, 1);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(b, 2);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);
	/* this should not be called until the active rekeyings are concluded */
	assert_hook_not_called(child_rekey);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 1);
	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 1);

	/* <-- CREATE_CHILD_SA { N(INVAL_KE) } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	assert_single_notify(IN, INVALID_KE_PAYLOAD);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 1);
	/* CREATE_CHILD_SA { N(INVAL_KE) } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	assert_single_notify(IN, INVALID_KE_PAYLOAD);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 1);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 7, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[3];
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 8, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);

	/* child_rekey */
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } */
	if (data[_i].spi_del_a == 1)
	{
		assert_hook_rekey(child_rekey, 1, data[_i].spi_a);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, data[_i].spi_del_b, CHILD_REKEYED,
							  CHILD_OUTBOUND_NONE);
		assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_hook();
	}
	else
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_child_sa_state(a, data[_i].spi_del_b, CHILD_REKEYED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
							  CHILD_OUTBOUND_REGISTERED);
		assert_hook();
	}
	assert_child_sa_state(a, data[_i].spi_del_a, CHILD_DELETING,
						  CHILD_OUTBOUND_NONE);

	/* CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } --> */
	if (data[_i].spi_del_b == 2)
	{
		assert_hook_rekey(child_rekey, 2, data[_i].spi_b);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_NONE);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_hook();
	}
	else
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_child_sa_state(b, data[_i].spi_del_a, CHILD_REKEYED,
							  CHILD_OUTBOUND_INSTALLED);
		assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
							  CHILD_OUTBOUND_REGISTERED);
		assert_hook();
	}
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETING,
						  CHILD_OUTBOUND_NONE);

	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	if (data[_i].spi_del_b == 2)
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_hook();
	}
	else
	{
		assert_hook_rekey(child_rekey, 2, data[_i].spi_b);
		exchange_test_helper->process_message(exchange_test_helper, b, NULL);
		assert_hook();
	}
	assert_scheduler();
	assert_child_sa_state(b, data[_i].spi_del_a, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETING,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
						  CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 3);
	/* <-- INFORMATIONAL { D } */
	assert_jobs_scheduled(1);
	if (data[_i].spi_del_a == 1)
	{
		assert_hook_not_called(child_rekey);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_hook();
	}
	else
	{
		assert_hook_rekey(child_rekey, 1, data[_i].spi_a);
		exchange_test_helper->process_message(exchange_test_helper, a, NULL);
		assert_hook();
	}
	assert_scheduler();
	assert_child_sa_state(a, data[_i].spi_del_a, CHILD_DELETING,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
						  CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 3);

	/* we don't expect this hook to get called anymore */
	assert_hook_not_called(child_rekey);

	/* <-- INFORMATIONAL { D } */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, data[_i].spi_del_a, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, data[_i].spi_a, CHILD_INSTALLED,
						  CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 3);
	assert_scheduler();
	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, data[_i].spi_del_b, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_del_a, CHILD_DELETED,
						  CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, data[_i].spi_b, CHILD_INSTALLED,
						  CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 3);
	assert_scheduler();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, data[_i].spi_del_a);
	destroy_rekeyed(a, data[_i].spi_del_b);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, data[_i].spi_a, data[_i].spi_b);
	destroy_rekeyed(b, data[_i].spi_del_a);
	destroy_rekeyed(b, data[_i].spi_del_b);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, data[_i].spi_a, data[_i].spi_b);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * This is a variation of the above but with the retry by one peer delayed so
 * that to the other peer it looks like there is no collision.
 */
START_TEST(test_collision_ke_invalid_delayed_retry)
{
	exchange_test_sa_conf_t conf = {
		.initiator = {
			.esp = "aes128-sha256-modp2048-modp3072",
		},
		.responder = {
			.esp = "aes128-sha256-modp3072-modp2048",
		},
	};
	ike_sa_t *a, *b;
	message_t *msg;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, &conf);

	/* Seven nonces and SPIs are needed (SPI 1 and 2 are used for the initial
	 * CHILD_SA):
	 *     N1/3 -----\    /----- N2/4
	 *                \--/-----> N3/-
	 *     N4/- <-------/  /---- INVAL_KE
	 * INVAL_KE -----\    /
	 *          <-----\--/
	 *     N5/5 -----\ \------->
	 *          <-----\--------- N6/6
	 *     N7/7 -------\------->
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

	/* make sure the nonces of the first try don't affect the retries */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	initiate_rekey(a, 1);
	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	initiate_rekey(b, 2);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);
	/* this should not be called until b doesn't notice a collision */
	assert_hook_not_called(child_rekey);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 1);
	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 1);

	/* <-- CREATE_CHILD_SA { N(INVAL_KE) } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[0];
	assert_single_notify(IN, INVALID_KE_PAYLOAD);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 1);
	/* CREATE_CHILD_SA { N(INVAL_KE) } --> */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[1];
	assert_single_notify(IN, INVALID_KE_PAYLOAD);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 1);

	/* delay the CREATE_CHILD_SA request from a to b */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->nonce_first_byte = data[_i].nonces[2];
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 7, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);

	/* child_rekey */
	assert_hook();

	/* CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } --> */
	assert_hook_rekey(child_rekey, 2, 6);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 6, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_hook();

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> (delayed) */
	assert_hook_not_called(child_rekey);
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, msg);
	assert_child_sa_state(b, 2, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 6, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_rekey(child_rekey, 1, 7);
	assert_no_jobs_scheduled();
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 7, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_scheduler();
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	assert_hook_not_called(child_rekey);
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 7, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 2);
	assert_scheduler();
	assert_hook();

	/* we don't expect this hook to get called anymore */
	assert_hook_not_called(child_rekey);

	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 6, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_scheduler();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, 1);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 6, 7);
	destroy_rekeyed(b, 2);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 6, 7);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * This simulates incorrect behavior by a hypothetical IKEv2 responder, which
 * might send a delete for the old CHILD_SA even if it lost the collision
 * (compared to the incorrect delete without collision, see above, this hasn't
 * been observed in the wild).
 * This is an issue if the DELETE arrives before the CREATE_CHILD_SA response.
 */
START_TEST(test_collision_responder_incorrect_delete)
{
	ike_sa_t *a, *b;
	message_t *msg;

	assert_track_sas_start();

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	/* make sure the responder looses the collision */
	exchange_test_helper->nonce_first_byte = 0xff;
	initiate_rekey(a, 1);
	assert_ipsec_sas_installed(a, 1, 2);
	exchange_test_helper->nonce_first_byte = 0x00;
	initiate_rekey(b, 2);
	assert_ipsec_sas_installed(b, 1, 2);

	/* this should never get called as this results in a successful rekeying */
	assert_hook_not_called(child_updown);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	exchange_test_helper->nonce_first_byte = 0xff;
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, 1, 2, 5);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } */
	exchange_test_helper->nonce_first_byte = 0xff;
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, 1, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 6, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(a, 1, 2, 6);
	assert_hook();

	/* delay the CREATE_CHILD_SA response */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_hook();
	assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 4, CHILD_DELETING, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, 1, 2, 4, 5);

	/* <-- INFORMATIONAL { D } */
	assert_no_jobs_scheduled();
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_hook();
	assert_child_sa_state(a, 1, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 6, CHILD_DELETING, CHILD_OUTBOUND_REGISTERED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 1, 2, 6);
	assert_scheduler();

	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_hook();
	assert_child_sa_state(b, 2, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 4, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_child_sa_count(b, 3);
	assert_ipsec_sas_installed(b, 1, 2, 4, 5);
	assert_scheduler();

	/* inject an incorrect delete for the old CHILD_SA by the responder,
	 * without messing with its internal state */
	send_child_delete(b, 2);

	/* <-- INFORMATIONAL { D } */
	assert_no_jobs_scheduled();
	assert_hook_not_called(child_rekey);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_hook();
	assert_child_sa_state(a, 1, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 6, CHILD_DELETING, CHILD_OUTBOUND_REGISTERED);
	assert_child_sa_count(a, 2);
	assert_ipsec_sas_installed(a, 1, 2, 6);
	assert_scheduler();


	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr } (delayed) */
	assert_jobs_scheduled(2);
	assert_hook_rekey(child_rekey, 1, 3);
	exchange_test_helper->process_message(exchange_test_helper, a, msg);
	assert_hook();
	assert_child_sa_state(a, 1, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(a, 3, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(a, 6, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_count(a, 3);
	assert_ipsec_sas_installed(a, 1, 3, 5, 6);
	assert_scheduler();

	/* INFORMATIONAL { D } (response to incorrect DELETE) --> */
	assert_no_jobs_scheduled();
	assert_hook_rekey(child_rekey, 2, 5);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	/* simulate handling of the delete via expire, does not delay destroy */
	call_ikesa(b, delete_child_sa, PROTO_ESP, 2, TRUE);
	assert_child_sa_state(b, 4, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 5, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(b, 2);
	assert_ipsec_sas_installed(b, 3, 4, 5);
	assert_hook();
	assert_scheduler();

	/* we don't expect this to get called anymore */
	assert_hook_not_called(child_rekey);

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, 1);
	destroy_rekeyed(a, 6);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, 3, 5);
	destroy_rekeyed(b, 4);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, 3, 5);

	/* child_rekey/child_updown */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * One of the hosts initiates a DELETE of the CHILD_SA the other peer is
 * concurrently trying to rekey.
 *
 *            rekey ----\       /---- delete
 *                       \-----/----> detect collision
 * detect collision <---------/ /---- TEMP_FAIL
 *           delete ----\      /
 *                       \----/----->
 *  aborts rekeying <--------/
 */
START_TEST(test_collision_delete)
{
	ike_sa_t *a, *b;
	uint32_t spi_a = _i+1, spi_b = 2-_i;

	assert_track_sas_start();

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
	initiate_rekey(a, spi_a);
	call_ikesa(b, delete_child_sa, PROTO_ESP, spi_b, FALSE);
	assert_child_sa_state(b, spi_b, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);

	/* this should never get called as there is no successful rekeying on
	 * either side */
	assert_hook_not_called(child_rekey);

	/* RFC 7296, 2.25.1: If a peer receives a request to rekey a CHILD_SA that
	 * it is currently trying to close, it SHOULD reply with TEMPORARY_FAILURE.
	 */

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_updown);
	assert_notify(IN, REKEY_SA);
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_hook();

	/* RFC 7296, 2.25.1: If a peer receives a request to delete a CHILD_SA that
	 * it is currently trying to rekey, it SHOULD reply as usual, with a DELETE
	 * payload.
	 */

	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_updown);
	assert_single_payload(IN, PLV2_DELETE);
	assert_single_payload(OUT, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	/* the SA is not destroyed until we get the CREATE_CHILD_SA response */
	assert_child_sa_state(a, spi_a, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 1);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	assert_hook_updown(child_updown, FALSE);
	/* we don't expect a job to retry the rekeying */
	assert_no_jobs_scheduled();
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 0);
	assert_scheduler();
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(child_updown, FALSE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_count(b, 0);
	assert_hook();

	/* child_rekey */
	assert_hook();
	assert_track_sas(2, 0);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * One of the hosts initiates a DELETE of the CHILD_SA the other peer is
 * concurrently trying to rekey with multiple key exchanges.
 *
 *            rekey ---------------->
 *                  <----------------
 *    additional ke ----\       /---- delete
 *                       \-----/----> detect collision
 * detect collision <---------/ /---- TEMP_FAIL
 *           delete -----------/---->
 *  aborts rekeying <---------/
 */
START_TEST(test_collision_delete_multi_ke)
{
	ike_sa_t *a, *b;
	uint32_t spi_a = _i+1, spi_b = 2-_i;

	assert_track_sas_start();

	if (_i)
	{	/* responder rekeys the CHILD_SA (SPI 2) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &b, &a, &multi_ke_conf);
	}
	else
	{	/* initiator rekeys the CHILD_SA (SPI 1) */
		exchange_test_helper->establish_sa(exchange_test_helper,
										   &a, &b, &multi_ke_conf);
	}
	initiate_rekey(a, spi_a);

	/* this should never get called as there is no successful rekeying on
	 * either side */
	assert_hook_not_called(child_rekey);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);

	/* <-- CREATE_CHILD_SA { SA, Nr, [KEr,] TSi, TSr, N(ADD_KE) } */
	assert_notify(IN, ADDITIONAL_KEY_EXCHANGE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);

	call_ikesa(b, delete_child_sa, PROTO_ESP, spi_b, FALSE);
	assert_child_sa_state(b, spi_b, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);

	/* IKE_FOLLOWUP_KE { KEi, N(ADD_KE) } --> */
	assert_hook_not_called(child_updown);
	assert_notify(IN, ADDITIONAL_KEY_EXCHANGE);
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_hook();

	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_updown);
	assert_single_payload(IN, PLV2_DELETE);
	assert_single_payload(OUT, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 1);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	assert_hook_updown(child_updown, FALSE);
	/* we don't expect a job to retry the rekeying */
	assert_no_jobs_scheduled();
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 0);
	assert_num_tasks(a, 0, TASK_QUEUE_ACTIVE);
	assert_scheduler();
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(child_updown, FALSE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_count(b, 0);
	assert_hook();

	/* child_rekey */
	assert_hook();
	assert_track_sas(2, 0);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * One of the hosts initiates a DELETE of the CHILD_SA the other peer is
 * concurrently trying to rekey.  However, the delete request is delayed or
 * dropped, so the peer doing the rekeying is unaware of the collision.
 *
 *            rekey ----\       /---- delete
 *                       \-----/----> detect collision
 *       reschedule <---------/------ TEMP_FAIL
 *                  <--------/
 *           delete ---------------->
 *
 * The job will not find the SA to retry rekeying.
 */
START_TEST(test_collision_delete_drop_delete)
{
	ike_sa_t *a, *b;
	message_t *msg;
	uint32_t spi_a = _i+1, spi_b = 2-_i;

	assert_track_sas_start();

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
	initiate_rekey(a, spi_a);
	call_ikesa(b, delete_child_sa, PROTO_ESP, spi_b, FALSE);
	assert_child_sa_state(b, spi_b, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);

	/* this should never get called as there is no successful rekeying on
	 * either side */
	assert_hook_not_called(child_rekey);

	/* RFC 7296, 2.25.1: If a peer receives a request to rekey a CHILD_SA that
	 * it is currently trying to close, it SHOULD reply with TEMPORARY_FAILURE.
	 */

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_updown);
	assert_notify(IN, REKEY_SA);
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_hook();

	/* delay the DELETE request */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	assert_hook_not_called(child_updown);
	/* we expect a job to retry the rekeying is scheduled */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_scheduler();
	assert_hook();

	/* <-- INFORMATIONAL { D } (delayed) */
	assert_hook_updown(child_updown, FALSE);
	assert_single_payload(IN, PLV2_DELETE);
	assert_single_payload(OUT, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, msg);
	assert_child_sa_count(a, 0);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(child_updown, FALSE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_count(b, 0);
	assert_hook();

	/* child_rekey */
	assert_hook();
	assert_track_sas(2, 0);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * One of the hosts initiates a DELETE of the CHILD_SA the other peer is
 * concurrently trying to rekey.  However, the rekey request is delayed or
 * dropped, so the peer doing the deleting is unaware of the collision.
 *
 *            rekey ----\       /---- delete
 * detect collision <----\-----/
 *           delete ------\--------->
 *                         \-------->
 *                              /---- CHILD_SA_NOT_FOUND
 *  aborts rekeying <----------/
 */
START_TEST(test_collision_delete_drop_rekey)
{
	ike_sa_t *a, *b;
	message_t *msg;
	uint32_t spi_a = _i+1, spi_b = 2-_i;

	assert_track_sas_start();

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
	initiate_rekey(a, spi_a);
	call_ikesa(b, delete_child_sa, PROTO_ESP, spi_b, FALSE);
	assert_child_sa_state(b, spi_b, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);

	/* this should never get called as there is no successful rekeying on
	 * either side */
	assert_hook_not_called(child_rekey);

	/* delay the CREATE_CHILD_SA request */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	/* RFC 7296, 2.25.1: If a peer receives a request to delete a CHILD_SA that
	 * it is currently trying to rekey, it SHOULD reply as usual, with a DELETE
	 * payload.
	 */

	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_updown);
	assert_single_payload(IN, PLV2_DELETE);
	assert_single_payload(OUT, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 1);
	assert_hook();

	/* INFORMATIONAL { D } --> */
	assert_hook_updown(child_updown, FALSE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_count(b, 0);
	assert_hook();

	/* RFC 7296, 2.25.1: If a peer receives a to rekey a Child SA that does not
	 * exist, it SHOULD reply with CHILD_SA_NOT_FOUND.
	 */

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> (delayed) */
	assert_hook_not_called(child_updown);
	assert_notify(IN, REKEY_SA);
	assert_single_notify(OUT, CHILD_SA_NOT_FOUND);
	exchange_test_helper->process_message(exchange_test_helper, b, msg);
	assert_hook();

	/* <-- CREATE_CHILD_SA { N(NO_CHILD_SA) } */
	assert_hook_updown(child_updown, FALSE);
	/* no jobs or tasks should get scheduled/queued */
	assert_no_jobs_scheduled();
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 0);
	assert_scheduler();
	assert_hook();

	/* child_rekey */
	assert_hook();
	assert_track_sas(2, 0);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * One of the hosts initiates a rekeying of a CHILD_SA and after responding to
 * it the other peer deletes the new SA.  However, the rekey response is
 * delayed or dropped, so the peer doing the rekeying receives a delete for an
 * unknown CHILD_SA and has to consider this when processing the rekey response.
 *
 *            rekey ---------------->
 *                              /---- rekey
 *       unknown SA <----------/----- delete new SA
 *                  ----------/----->
 *        delete SA <--------/
 */
START_TEST(test_collision_delete_delayed_response)
{
	ike_sa_t *a, *b;
	message_t *msg;
	uint32_t spi_a = _i+1, spi_b = 2-_i;

	assert_track_sas_start();

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
	initiate_rekey(a, spi_a);

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_rekey);
	assert_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_REKEYED, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_state(b, 4, CHILD_INSTALLED, CHILD_OUTBOUND_REGISTERED);
	assert_ipsec_sas_installed(b, spi_a, spi_b, 4);
	assert_hook();


	/* delay the CREATE_CHILD_SA response */
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);

	assert_hook_rekey(child_rekey, spi_b, 4);
	assert_hook_not_called(child_updown);
	call_ikesa(b, delete_child_sa, PROTO_ESP, 4, FALSE);
	assert_child_sa_state(b, spi_b, CHILD_REKEYED, CHILD_OUTBOUND_NONE);
	assert_child_sa_state(b, 4, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_ipsec_sas_installed(b, spi_b, 3, 4);
	assert_child_sa_count(b, 2);
	assert_hook();
	assert_hook();

	/* this is not expected to get called until the response is processed */
	assert_hook_not_called(child_rekey);

	/* <-- INFORMATIONAL { D } */
	assert_hook_not_called(child_updown);
	assert_single_payload(IN, PLV2_DELETE);
	assert_message_empty(OUT);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, spi_a, spi_b);
	assert_hook();

	/* INFORMATIONAL { } --> */
	assert_hook_updown(child_updown, FALSE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_REKEYED, CHILD_OUTBOUND_NONE);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, spi_b);
	assert_hook();

	/* child_rekey */
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Ni, [KEi,] TSi, TSr } (delayed) */
	assert_hook_rekey(child_rekey, spi_a, 3);
	assert_hook_updown(child_updown, FALSE);
	/* the job scheduled here is for the retransmit of the delete */
	assert_jobs_scheduled(1);
	assert_no_notify(IN, REKEY_SA);
	exchange_test_helper->process_message(exchange_test_helper, a, msg);
	assert_child_sa_state(a, spi_a, CHILD_DELETING, CHILD_OUTBOUND_INSTALLED);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, spi_a, spi_b);
	assert_scheduler();
	assert_hook();
	assert_hook();

	/* this is not expected to get called anymore */
	assert_hook_not_called(child_rekey);

	/* INFORMATIONAL { D } --> */
	assert_jobs_scheduled(1);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_state(b, spi_b, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_count(b, 1);
	assert_ipsec_sas_installed(b, spi_b);
	assert_scheduler();
	/* <-- INFORMATIONAL { D } */
	assert_jobs_scheduled(1);
	assert_single_payload(IN, PLV2_DELETE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_DELETED, CHILD_OUTBOUND_NONE);
	assert_child_sa_count(a, 1);
	assert_ipsec_sas_installed(a, spi_a);
	assert_scheduler();

	/* simulate the execution of the scheduled jobs */
	destroy_rekeyed(a, spi_a);
	assert_child_sa_count(a, 0);
	assert_ipsec_sas_installed(a);
	destroy_rekeyed(b, spi_b);
	assert_child_sa_count(b, 0);
	assert_ipsec_sas_installed(a);

	/* child_rekey */
	assert_hook();
	assert_track_sas(2, 0);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * One of the hosts initiates a rekey of the IKE_SA of the CHILD_SA the other
 * peer is concurrently trying to rekey.
 *
 *            rekey ----\       /---- rekey IKE
 *                       \-----/----> detect collision
 * detect collision <---------/ /---- TEMP_FAIL
 *        TEMP_FAIL ----\      /
 *                       \----/----->
 *                  <--------/
 */
START_TEST(test_collision_ike_rekey)
{
	ike_sa_t *a, *b;
	uint32_t spi_a = _i+1;

	assert_track_sas_start();

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
	initiate_rekey(a, spi_a);
	call_ikesa(b, rekey);
	assert_ike_sa_state(b, IKE_REKEYING);

	/* these should never get called as there is no successful rekeying on
	 * either side */
	assert_hook_not_called(ike_rekey);
	assert_hook_not_called(child_rekey);

	/* RFC 7296, 2.25.2: If a peer receives a request to rekey a CHILD_SA when
	 * it is currently rekeying the IKE SA, it SHOULD reply with
	 * TEMPORARY_FAILURE.
	 */

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_REKEYING);

	/* RFC 7296, 2.25.1: If a peer receives a request to rekey the IKE SA, and
	 * it is currently, rekeying, or closing a Child SA of that IKE SA, it
	 * SHOULD reply with TEMPORARY_FAILURE.
	 */

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_REKEYING, CHILD_OUTBOUND_INSTALLED);

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	/* we expect a job to retry the rekeying is scheduled */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_state(a, spi_a, CHILD_INSTALLED, CHILD_OUTBOUND_INSTALLED);
	assert_scheduler();

	/* CREATE_CHILD_SA { N(TEMP_FAIL) } --> */
	/* we expect a job to retry the rekeying is scheduled */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_ESTABLISHED);
	assert_scheduler();

	/* ike_rekey/child_rekey */
	assert_hook();
	assert_hook();
	assert_track_sas(2, 2);

	assert_sa_idle(a);
	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

/**
 * One of the hosts initiates a delete of the IKE_SA of the CHILD_SA the other
 * peer is concurrently trying to rekey.
 *
 *            rekey ----\       /---- delete IKE
 *                       \-----/----> detect collision
 *                  <---------/ /---- TEMP_FAIL
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

	assert_track_sas_start();

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
	initiate_rekey(a, spi_a);
	call_ikesa(b, delete, FALSE);
	assert_ike_sa_state(b, IKE_DELETING);

	/* this should never get called as there is no successful rekeying on
	 * either side */
	assert_hook_not_called(child_rekey);

	/* RFC 7296, 2.25.2 does not explicitly state what the behavior SHOULD be if
	 * a peer receives a request to rekey a CHILD_SA when it is currently
	 * closing the IKE SA.  We expect a TEMPORARY_FAILURE notify.
	 */

	/* CREATE_CHILD_SA { N(REKEY_SA), SA, Ni, [KEi,] TSi, TSr } --> */
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_DELETING);

	/* RFC 7296, 2.25.1 does not explicitly state what the behavior SHOULD be if
	 * a peer receives a request to close the IKE SA if it is currently rekeying
	 * a Child SA of that IKE SA.  Let's just close the IKE_SA and forget the
	 * rekeying.
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

	/* child_rekey */
	assert_hook();
	assert_track_sas(0, 0);
}
END_TEST

Suite *child_rekey_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("child rekey");

	tc = tcase_create("regular");
	tcase_add_loop_test(tc, test_regular, 0, 2);
	tcase_add_loop_test(tc, test_regular_multi_ke, 0, 2);
	tcase_add_loop_test(tc, test_regular_ke_invalid, 0, 2);
	tcase_add_loop_test(tc, test_regular_ke_invalid_multi_ke, 0, 2);
	tcase_add_test(tc, test_regular_responder_ignore_soft_expire);
	tcase_add_test(tc, test_regular_responder_handle_hard_expire);
	tcase_add_test(tc, test_regular_responder_delete);
	tcase_add_test(tc, test_regular_responder_lost_sa);
	tcase_add_test(tc, test_regular_responder_incorrect_delete);
	suite_add_tcase(s, tc);

	tc = tcase_create("collisions rekey");
	tcase_add_loop_test(tc, test_collision, 0, 4);
	tcase_add_loop_test(tc, test_collision_multi_ke, 0, 4);
	tcase_add_loop_test(tc, test_collision_mixed, 0, 4);
	tcase_add_loop_test(tc, test_collision_delayed_response, 0, 4);
	tcase_add_loop_test(tc, test_collision_delayed_response_delete, 0, 4);
	tcase_add_loop_test(tc, test_collision_delayed_response_multi_ke, 0, 4);
	tcase_add_loop_test(tc, test_collision_delayed_request, 0, 6);
	tcase_add_loop_test(tc, test_collision_delayed_request_more, 0, 3);
	tcase_add_loop_test(tc, test_collision_delayed_request_more_delete, 0, 3);
	tcase_add_loop_test(tc, test_collision_delayed_request_multi_ke, 0, 6);
	tcase_add_loop_test(tc, test_collision_ke_invalid, 0, 4);
	tcase_add_loop_test(tc, test_collision_ke_invalid_delayed_retry, 0, 3);
	tcase_add_test(tc, test_collision_responder_incorrect_delete);
	suite_add_tcase(s, tc);

	tc = tcase_create("collisions delete");
	tcase_add_loop_test(tc, test_collision_delete, 0, 2);
	tcase_add_loop_test(tc, test_collision_delete_multi_ke, 0, 2);
	tcase_add_loop_test(tc, test_collision_delete_drop_delete, 0, 2);
	tcase_add_loop_test(tc, test_collision_delete_drop_rekey, 0, 2);
	tcase_add_loop_test(tc, test_collision_delete_delayed_response, 0, 2);
	suite_add_tcase(s, tc);

	tc = tcase_create("collisions ike rekey");
	tcase_add_loop_test(tc, test_collision_ike_rekey, 0, 2);
	suite_add_tcase(s, tc);

	tc = tcase_create("collisions ike delete");
	tcase_add_loop_test(tc, test_collision_ike_delete, 0, 2);
	suite_add_tcase(s, tc);

	return s;
}
