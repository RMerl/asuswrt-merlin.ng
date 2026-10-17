// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for armffa command
 *
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#include <common.h>
#include <string.h>
#include <asm/sandbox_arm_ffa.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Basic test of 'armffa' command */
static int dm_test_armffa_cmd(struct unit_test_state *uts)
{
	/* armffa getpart <UUID> */
	ut_assertok(run_command("armffa getpart " SANDBOX_SERVICE1_UUID, 0));

	/* armffa ping <ID> */
	ut_assertok(run_commandf("armffa ping 0x%x", SANDBOX_SP1_ID));

	/* armffa devlist */
	ut_assertok(run_command("armffa devlist", 0));

	return 0;
}

DM_TEST(dm_test_armffa_cmd, UT_TESTF_SCAN_FDT | UT_TESTF_CONSOLE_REC);
