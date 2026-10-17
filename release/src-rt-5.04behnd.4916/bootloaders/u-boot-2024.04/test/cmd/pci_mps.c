// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests that the PCI Maximum Payload Size (MPS) command can set the sandbox
 * PCI Express device to safe mode and determine the correct payload size.
 *
 * Copyright 2023 Microsoft
 * Written by Stephen Carlson <stcarlso@linux.microsoft.com>
 */

#include <common.h>
#include <console.h>
#include <test/suites.h>
#include <test/ut.h>

#define PCI_MPS_TEST(_name, _flags) UNIT_TEST(_name, _flags, pci_mps_test)

/* Test "pci_mps" command in safe "s" mode */
static int test_pci_mps_safe(struct unit_test_state *uts)
{
	/* Enumerate PCI Express first */
	ut_assertok(run_command("pci e", 0));
	ut_assert_console_end();

	/* Test pci_mps s */
	ut_assertok(run_command("pci_mps s", 0));
	ut_assert_nextline("Setting MPS of all devices to 256B");
	ut_assert_console_end();

	return 0;
}

PCI_MPS_TEST(test_pci_mps_safe, UT_TESTF_CONSOLE_REC);

int do_ut_pci_mps(struct cmd_tbl *cmdtp, int flag, int argc,
		  char * const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(pci_mps_test);
	const int n = UNIT_TEST_SUITE_COUNT(pci_mps_test);

	return cmd_ut_category("cmd_pci_mps", "pci_mps_test_", tests, n,
			       argc, argv);
}
