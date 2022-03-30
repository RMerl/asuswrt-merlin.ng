// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Texas Instruments Incorporated - http://www.ti.com/
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <remoteproc.h>
#include <dm/test.h>
#include <test/ut.h>
/**
 * dm_test_remoteproc_base() - test the operations after initializations
 * @uts:	unit test state
 *
 * Return:	0 if test passed, else error
 */
static int dm_test_remoteproc_base(struct unit_test_state *uts)
{
	if (!rproc_is_initialized())
		ut_assertok(rproc_init());

	/* Ensure we are initialized */
	ut_asserteq(true, rproc_is_initialized());


	/* platform data device 1 */
	ut_assertok(rproc_stop(0));
	ut_assertok(rproc_reset(0));
	/* -> invalid attempt tests */
	ut_asserteq(-EINVAL, rproc_start(0));
	ut_asserteq(-EINVAL, rproc_ping(0));
	/* Valid tests */
	ut_assertok(rproc_load(0, 1, 0));
	ut_assertok(rproc_start(0));
	ut_assertok(rproc_is_running(0));
	ut_assertok(rproc_ping(0));
	ut_assertok(rproc_reset(0));
	ut_assertok(rproc_stop(0));

	/* dt device device 1 */
	ut_assertok(rproc_stop(1));
	ut_assertok(rproc_reset(1));
	ut_assertok(rproc_load(1, 1, 0));
	ut_assertok(rproc_start(1));
	ut_assertok(rproc_is_running(1));
	ut_assertok(rproc_ping(1));
	ut_assertok(rproc_reset(1));
	ut_assertok(rproc_stop(1));

	/* dt device device 2 */
	ut_assertok(rproc_stop(0));
	ut_assertok(rproc_reset(0));
	/* -> invalid attempt tests */
	ut_asserteq(-EINVAL, rproc_start(0));
	ut_asserteq(-EINVAL, rproc_ping(0));
	/* Valid tests */
	ut_assertok(rproc_load(2, 1, 0));
	ut_assertok(rproc_start(2));
	ut_assertok(rproc_is_running(2));
	ut_assertok(rproc_ping(2));
	ut_assertok(rproc_reset(2));
	ut_assertok(rproc_stop(2));

	return 0;
}
DM_TEST(dm_test_remoteproc_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
