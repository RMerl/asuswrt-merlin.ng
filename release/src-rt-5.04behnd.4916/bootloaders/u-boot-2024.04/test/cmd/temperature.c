// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Executes tests for temperature command
 *
 * Copyright (C) 2022 Sartura Ltd.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_cmd_temperature(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_THERMAL, 0, &dev));
	ut_assertnonnull(dev);

	ut_assertok(console_record_reset_enable());

	/* Test that "temperature list" shows the sandbox device */
	ut_assertok(run_command("temperature list", 0));
	ut_assert_nextline("| Device                        | Driver                        | Parent");
	ut_assert_nextline("| thermal                       | thermal-sandbox               | root_driver");
	ut_assert_console_end();

	/* Test that "temperature get thermal" returns expected value */
	console_record_reset();
	ut_assertok(run_command("temperature get thermal", 0));
	ut_assert_nextline("thermal: 100 C");
	ut_assert_console_end();

	return 0;
}

DM_TEST(dm_test_cmd_temperature, UT_TESTF_SCAN_FDT | UT_TESTF_CONSOLE_REC);
