// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for bootm routines
 *
 * Copyright 2020 Google LLC
 */

#include <common.h>
#include <bootm.h>
#include <asm/global_data.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

#define BOOTM_TEST(_name, _flags)	UNIT_TEST(_name, _flags, bootm_test)

enum {
	BUF_SIZE	= 1024,
};

#define CONSOLE_STR	"console=/dev/ttyS0"

/* Test cmdline processing where nothing happens */
static int bootm_test_nop(struct unit_test_state *uts)
{
	char buf[BUF_SIZE];

	*buf = '\0';
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, true));
	ut_asserteq_str("", buf);

	strcpy(buf, "test");
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, true));
	ut_asserteq_str("test", buf);

	return 0;
}
BOOTM_TEST(bootm_test_nop, 0);

/* Test cmdline processing when out of space */
static int bootm_test_nospace(struct unit_test_state *uts)
{
	char buf[BUF_SIZE];

	/* Zero buffer size */
	*buf = '\0';
	ut_asserteq(-ENOSPC, bootm_process_cmdline(buf, 0, true));

	/* Buffer string not terminated */
	memset(buf, 'a', BUF_SIZE);
	ut_asserteq(-ENOSPC, bootm_process_cmdline(buf, BUF_SIZE, true));

	/* Not enough space to copy string */
	memset(buf, '\0', BUF_SIZE);
	memset(buf, 'a', BUF_SIZE / 2);
	ut_asserteq(-ENOSPC, bootm_process_cmdline(buf, BUF_SIZE, true));

	/* Just enough space */
	memset(buf, '\0', BUF_SIZE);
	memset(buf, 'a', BUF_SIZE / 2 - 1);
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, true));

	return 0;
}
BOOTM_TEST(bootm_test_nospace, 0);

/* Test silent processing */
static int bootm_test_silent(struct unit_test_state *uts)
{
	char buf[BUF_SIZE];

	/* 'silent_linux' not set should do nothing */
	env_set("silent_linux", NULL);
	strcpy(buf, CONSOLE_STR);
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SILENT));
	ut_asserteq_str(CONSOLE_STR, buf);

	ut_assertok(env_set("silent_linux", "no"));
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SILENT));
	ut_asserteq_str(CONSOLE_STR, buf);

	ut_assertok(env_set("silent_linux", "yes"));
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SILENT));
	ut_asserteq_str("console=ttynull", buf);

	/* Empty buffer should still add the string */
	*buf = '\0';
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SILENT));
	ut_asserteq_str("console=ttynull", buf);

	/* Check nothing happens when do_silent is false */
	*buf = '\0';
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, 0));
	ut_asserteq_str("", buf);

	/* Not enough space */
	*buf = '\0';
	ut_asserteq(-ENOSPC, bootm_process_cmdline(buf, 15, BOOTM_CL_SILENT));

	/* Just enough space */
	*buf = '\0';
	ut_assertok(bootm_process_cmdline(buf, 16, BOOTM_CL_SILENT));

	/* add at end */
	strcpy(buf, "something");
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SILENT));
	ut_asserteq_str("something console=ttynull", buf);

	/* change at start */
	strcpy(buf, CONSOLE_STR " something");
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SILENT));
	ut_asserteq_str("console=ttynull something", buf);

	return 0;
}
BOOTM_TEST(bootm_test_silent, 0);

/* Test substitution processing */
static int bootm_test_subst(struct unit_test_state *uts)
{
	char buf[BUF_SIZE];

	/* try with an unset variable */
	ut_assertok(env_set("var", NULL));
	strcpy(buf, "some${var}thing");
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SUBST));
	ut_asserteq_str("something", buf);

	/* Replace with shorter string */
	ut_assertok(env_set("var", "bb"));
	strcpy(buf, "some${var}thing");
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SUBST));
	ut_asserteq_str("somebbthing", buf);

	/* Replace with same-length string */
	ut_assertok(env_set("var", "abc"));
	strcpy(buf, "some${var}thing");
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SUBST));
	ut_asserteq_str("someabcthing", buf);

	/* Replace with longer string */
	ut_assertok(env_set("var", "abcde"));
	strcpy(buf, "some${var}thing");
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SUBST));
	ut_asserteq_str("someabcdething", buf);

	/* Check it is case sensitive */
	ut_assertok(env_set("VAR", NULL));
	strcpy(buf, "some${VAR}thing");
	ut_assertok(bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SUBST));
	ut_asserteq_str("something", buf);

	/* Check too long - need 12 bytes for each string */
	strcpy(buf, "some${var}thing");
	ut_asserteq(-ENOSPC,
		    bootm_process_cmdline(buf, 12 * 2 - 1, BOOTM_CL_SUBST));

	/* Check just enough space */
	strcpy(buf, "some${var}thing");
	ut_assertok(bootm_process_cmdline(buf, 16 * 2, BOOTM_CL_SUBST));
	ut_asserteq_str("someabcdething", buf);

	/*
	 * Check the substition string being too long. This results in a string
	 * of 12 (13 bytes). We need enough space for that plus the original
	 * "a${var}c" string of 9 bytes. So 12 + 9 = 21 bytes.
	 */
	ut_assertok(env_set("var", "1234567890"));
	strcpy(buf, "a${var}c");
	ut_asserteq(-ENOSPC, bootm_process_cmdline(buf, 21, BOOTM_CL_SUBST));

	strcpy(buf, "a${var}c");
	ut_asserteq(0, bootm_process_cmdline(buf, 22, BOOTM_CL_SUBST));

	/* Check multiple substitutions */
	ut_assertok(env_set("var", "abc"));
	strcpy(buf, "some${var}thing${bvar}else");
	ut_asserteq(0, bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SUBST));
	ut_asserteq_str("someabcthingelse", buf);

	/* Check multiple substitutions */
	ut_assertok(env_set("bvar", "123"));
	strcpy(buf, "some${var}thing${bvar}else");
	ut_asserteq(0, bootm_process_cmdline(buf, BUF_SIZE, BOOTM_CL_SUBST));
	ut_asserteq_str("someabcthing123else", buf);

	return 0;
}
BOOTM_TEST(bootm_test_subst, 0);

/* Test silent processing in the bootargs variable */
static int bootm_test_silent_var(struct unit_test_state *uts)
{
	env_set("bootargs", NULL);
	ut_assertok(bootm_process_cmdline_env(BOOTM_CL_SUBST));
	ut_assertnull(env_get("bootargs"));

	ut_assertok(env_set("bootargs", "some${var}thing"));
	ut_assertok(bootm_process_cmdline_env(BOOTM_CL_SUBST));
	ut_asserteq_str("something", env_get("bootargs"));

	return 0;
}
BOOTM_TEST(bootm_test_silent_var, 0);

/* Test substitution processing in the bootargs variable */
static int bootm_test_subst_var(struct unit_test_state *uts)
{
	ut_assertok(env_set("silent_linux", "yes"));
	ut_assertok(env_set("bootargs", NULL));
	ut_assertok(bootm_process_cmdline_env(BOOTM_CL_SILENT));
	ut_asserteq_str("console=ttynull", env_get("bootargs"));

	ut_assertok(env_set("var", "abc"));
	ut_assertok(env_set("bootargs", "some${var}thing"));
	ut_assertok(bootm_process_cmdline_env(BOOTM_CL_SILENT));
	ut_asserteq_str("some${var}thing console=ttynull", env_get("bootargs"));

	return 0;
}
BOOTM_TEST(bootm_test_subst_var, 0);

/* Test substitution and silent console processing in the bootargs variable */
static int bootm_test_subst_both(struct unit_test_state *uts)
{
	ut_assertok(env_set("silent_linux", "yes"));
	env_set("bootargs", NULL);
	ut_assertok(bootm_process_cmdline_env(BOOTM_CL_ALL));
	ut_asserteq_str("console=ttynull", env_get("bootargs"));

	ut_assertok(env_set("bootargs", "some${var}thing " CONSOLE_STR));
	ut_assertok(env_set("var", "1234567890"));
	ut_assertok(bootm_process_cmdline_env(BOOTM_CL_ALL));
	ut_asserteq_str("some1234567890thing console=ttynull", env_get("bootargs"));

	return 0;
}
BOOTM_TEST(bootm_test_subst_both, 0);

int do_ut_bootm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(bootm_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(bootm_test);

	return cmd_ut_category("bootm", "bootm_test_", tests, n_ents,
			       argc, argv);
}
