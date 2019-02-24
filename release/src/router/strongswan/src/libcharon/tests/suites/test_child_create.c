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
 * One of the peers tries to create a new CHILD_SA while the other concurrently
 * started to rekey the IKE_SA. TEMPORARY_FAILURE should be returned on both
 * sides and the peers should prepare to retry.
 */
START_TEST(test_collision_ike_rekey)
{
	child_cfg_t *child_cfg;
	child_cfg_create_t child = {
		.mode = MODE_TUNNEL,
	};
	ike_sa_t *a, *b;

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	assert_hook_not_called(child_updown);
	child_cfg = child_cfg_create("child", &child);
	child_cfg->add_proposal(child_cfg, proposal_create_default(PROTO_ESP));
	child_cfg->add_traffic_selector(child_cfg, TRUE,
								traffic_selector_create_dynamic(0, 0, 65535));
	child_cfg->add_traffic_selector(child_cfg, FALSE,
								traffic_selector_create_dynamic(0, 0, 65535));
	call_ikesa(a, initiate, child_cfg, 0, NULL, NULL);
	assert_child_sa_count(a, 1);
	assert_hook();

	call_ikesa(b, rekey);

	/* CREATE_CHILD_SA { SA, Ni, [KEi,] TSi, TSr } --> */
	assert_hook_not_called(child_updown);
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_child_sa_count(b, 1);
	assert_hook();

	/* <-- CREATE_CHILD_SA { SA, Ni, KEi } */
	assert_single_notify(OUT, TEMPORARY_FAILURE);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);

	/* <-- CREATE_CHILD_SA { N(TEMP_FAIL) } */
	assert_hook_not_called(child_updown);
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	assert_child_sa_count(a, 1);
	assert_scheduler();
	assert_hook();

	/* CREATE_CHILD_SA { N(TEMP_FAIL) } --> */
	assert_jobs_scheduled(1);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	assert_ike_sa_state(b, IKE_ESTABLISHED);
	assert_scheduler();

	/* make sure no message was sent after handling the TEMPORARY_FAILURE and
	 * that the task to retry creating the CHILD_SA is queued and not active
	 * and it can't be initiated immediately */
	ck_assert(!exchange_test_helper->sender->dequeue(exchange_test_helper->sender));
	assert_num_tasks(a, 0, TASK_QUEUE_ACTIVE);
	assert_num_tasks(a, 1, TASK_QUEUE_QUEUED);
	call_ikesa(a, initiate, NULL, 0, NULL, NULL);
	assert_num_tasks(a, 0, TASK_QUEUE_ACTIVE);

	assert_sa_idle(b);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
}
END_TEST

Suite *child_create_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("child create");

	tc = tcase_create("collisions ike rekey");
	tcase_add_test(tc, test_collision_ike_rekey);
	suite_add_tcase(s, tc);

	return s;
}
