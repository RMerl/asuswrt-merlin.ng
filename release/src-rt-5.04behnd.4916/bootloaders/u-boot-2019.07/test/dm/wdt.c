// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <asm/state.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

/* Test that watchdog driver functions are called */
static int dm_test_wdt_base(struct unit_test_state *uts)
{
	struct sandbox_state *state = state_get_current();
	struct udevice *dev;
	const u64 timeout = 42;

	ut_assertok(uclass_get_device(UCLASS_WDT, 0, &dev));
	ut_assertnonnull(dev);
	ut_asserteq(0, state->wdt.counter);
	ut_asserteq(false, state->wdt.running);

	ut_assertok(wdt_start(dev, timeout, 0));
	ut_asserteq(timeout, state->wdt.counter);
	ut_asserteq(true, state->wdt.running);

	uint reset_count = state->wdt.reset_count;
	ut_assertok(wdt_reset(dev));
	ut_asserteq(reset_count + 1, state->wdt.reset_count);
	ut_asserteq(true, state->wdt.running);

	ut_assertok(wdt_stop(dev));
	ut_asserteq(false, state->wdt.running);

	return 0;
}
DM_TEST(dm_test_wdt_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
