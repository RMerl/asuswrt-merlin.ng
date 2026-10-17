// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <hwspinlock.h>
#include <asm/state.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Test that hwspinlock driver functions are called */
static int dm_test_hwspinlock_base(struct unit_test_state *uts)
{
	struct sandbox_state *state = state_get_current();
	struct hwspinlock hws;

	ut_assertok(uclass_get_device(UCLASS_HWSPINLOCK, 0, &hws.dev));
	ut_assertnonnull(hws.dev);
	ut_asserteq(false, state->hwspinlock);

	hws.id = 0;
	ut_assertok(hwspinlock_lock_timeout(&hws, 1));
	ut_asserteq(true, state->hwspinlock);

	ut_assertok(hwspinlock_unlock(&hws));
	ut_asserteq(false, state->hwspinlock);

	ut_assertok(hwspinlock_lock_timeout(&hws, 1));
	ut_assertok(!hwspinlock_lock_timeout(&hws, 1));

	ut_assertok(hwspinlock_unlock(&hws));
	ut_assertok(!hwspinlock_unlock(&hws));

	return 0;
}

DM_TEST(dm_test_hwspinlock_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
