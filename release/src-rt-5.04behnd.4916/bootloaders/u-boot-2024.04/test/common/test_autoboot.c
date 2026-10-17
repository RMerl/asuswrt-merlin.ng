// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Steffen Jaeckel
 *
 * Unit tests for autoboot functionality
 */

#include <autoboot.h>
#include <common.h>
#include <test/common.h>
#include <test/test.h>
#include <test/ut.h>

#include <crypt.h>

static int check_for_input(struct unit_test_state *uts, const char *in,
			   bool correct)
{
	bool old_val;
	/* The bootdelay is set to 1 second in test_autoboot() */
	const char *autoboot_prompt =
		"Enter password \"a\" in 1 seconds to stop autoboot";

	console_record_reset_enable();
	console_in_puts(in);

	/* turn on keyed autoboot for the test, if possible */
	old_val = autoboot_set_keyed(true);
	autoboot_command("echo Autoboot password unlock not successful");
	old_val = autoboot_set_keyed(old_val);

	ut_assert_nextline(autoboot_prompt);
	if (!correct)
		ut_assert_nextline("Autoboot password unlock not successful");
	ut_assert_console_end();
	return 0;
}

/**
 * test_autoboot() - unit test for autoboot
 *
 * @uts:	unit test state
 * Return:	0 = success, 1 = failure
 */
static int test_autoboot(struct unit_test_state *uts)
{
	/* make sure that the bootdelay is set to something,
	 * otherwise the called functions will time out
	 */
	ut_assertok(env_set("bootdelay", "1"));
	bootdelay_process();

	/* unset all relevant environment variables */
	env_set("bootstopusesha256", NULL);
	env_set("bootstopkeycrypt", NULL);
	env_set("bootstopkeysha256", NULL);

	if (IS_ENABLED(CONFIG_CRYPT_PW_SHA256)) {
		/* test the default password from CONFIG_AUTOBOOT_STOP_STR_CRYPT */
		ut_assertok(check_for_input(uts, "a\n", true));
		/* test a password from the `bootstopkeycrypt` environment variable */
		ut_assertok(env_set(
			"bootstopkeycrypt",
			"$5$rounds=640000$ycgRgpnRq4lmu.eb$aZ6YJWdklvyLML13w7mEHMHJnJOux6aptnp6VlsR5a9"));

		ut_assertok(check_for_input(uts, "test\n", true));

		/* verify that the `bootstopusesha256` variable is treated correctly */
		ut_assertok(env_set("bootstopusesha256", "false"));
		ut_assertok(check_for_input(uts, "test\n", true));
	}

	if (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		/* test the `bootstopusesha256` and `bootstopkeysha256` features */
		ut_assertok(env_set("bootstopusesha256", "true"));
		ut_assertok(env_set(
			"bootstopkeysha256",
			"edeaaff3f1774ad2888673770c6d64097e391bc362d7d6fb34982ddf0efd18cb"));

		ut_assertok(check_for_input(uts, "abc\n", true));

		ut_assertok(env_set(
			"bootstopkeysha256",
			"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));

		ut_assertok(check_for_input(uts, "abc", true));

		ut_assertok(check_for_input(uts, "abc\n", true));

		ut_assertok(check_for_input(uts, "abd", false));
	}

	return CMD_RET_SUCCESS;
}

COMMON_TEST(test_autoboot, 0);
