// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for pwm command
 *
 * Copyright 2020 SiFive, Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <dm.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Basic test of 'pwm' command */
static int dm_test_pwm_cmd(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* cros-ec-pwm */
	ut_assertok(uclass_get_device(UCLASS_PWM, 0, &dev));
	ut_assertnonnull(dev);

	ut_assertok(console_record_reset_enable());

	/* pwm <invert> <pwm_dev_num> <channel> <polarity> */
	/* cros-ec-pwm doesn't support invert */
	ut_asserteq(1, run_command("pwm invert 0 0 1", 0));
	ut_assert_nextline("error(-38)");
	ut_assert_console_end();

	ut_asserteq(1, run_command("pwm invert 0 0 0", 0));
	ut_assert_nextline("error(-38)");
	ut_assert_console_end();

	/* pwm <config> <pwm_dev_num> <channel> <period_ns> <duty_ns> */
	ut_assertok(run_command("pwm config 0 0 10 50", 0));
	ut_assert_console_end();

	/* pwm <enable/disable> <pwm_dev_num> <channel> */
	ut_assertok(run_command("pwm enable 0 0", 0));
	ut_assert_console_end();

	ut_assertok(run_command("pwm disable 0 0", 0));
	ut_assert_console_end();

	/* sandbox-pwm */
	ut_assertok(uclass_get_device(UCLASS_PWM, 1, &dev));
	ut_assertnonnull(dev);

	ut_assertok(console_record_reset_enable());

	/* pwm <invert> <pwm_dev_num> <channel> <polarity> */
	ut_assertok(run_command("pwm invert 1 0 1", 0));
	ut_assert_console_end();

	ut_assertok(run_command("pwm invert 1 0 0", 0));
	ut_assert_console_end();

	/* pwm <config> <pwm_dev_num> <channel> <period_ns> <duty_ns> */
	ut_assertok(run_command("pwm config 1 0 10 50", 0));
	ut_assert_console_end();

	/* pwm <enable/disable> <pwm_dev_num> <channel> */
	ut_assertok(run_command("pwm enable 1 0", 0));
	ut_assert_console_end();

	ut_assertok(run_command("pwm disable 1 0", 0));
	ut_assert_console_end();

	return 0;
}

DM_TEST(dm_test_pwm_cmd, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT | UT_TESTF_CONSOLE_REC);
