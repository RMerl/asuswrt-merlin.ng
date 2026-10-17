// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for bootdev functions. All start with 'bootmeth'
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <dm.h>
#include <test/suites.h>
#include <test/ut.h>
#include "bootstd_common.h"

/* Check 'bootmeth list' command */
static int bootmeth_cmd_list(struct unit_test_state *uts)
{
	console_record_reset_enable();
	ut_assertok(run_command("bootmeth list", 0));
	ut_assert_nextline("Order  Seq  Name                Description");
	ut_assert_nextlinen("---");
	ut_assert_nextline("    0    0  extlinux            Extlinux boot from a block device");
	ut_assert_nextline("    1    1  efi                 EFI boot from an .efi file");
	if (IS_ENABLED(CONFIG_BOOTMETH_GLOBAL))
		ut_assert_nextline(" glob    2  firmware0           VBE simple");
	ut_assert_nextlinen("---");
	ut_assert_nextline(IS_ENABLED(CONFIG_BOOTMETH_GLOBAL) ?
		 "(3 bootmeths)" : "(2 bootmeths)");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootmeth_cmd_list, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootmeth order' command */
static int bootmeth_cmd_order(struct unit_test_state *uts)
{
	/* Select just one bootmethod */
	console_record_reset_enable();
	ut_assertok(run_command("bootmeth order extlinux", 0));
	ut_assert_console_end();
	ut_assertnonnull(env_get("bootmeths"));
	ut_asserteq_str("extlinux", env_get("bootmeths"));

	/* Only that one should be listed */
	ut_assertok(run_command("bootmeth list", 0));
	ut_assert_nextline("Order  Seq  Name                Description");
	ut_assert_nextlinen("---");
	ut_assert_nextline("    0    0  extlinux            Extlinux boot from a block device");
	ut_assert_nextlinen("---");
	ut_assert_nextline("(1 bootmeth)");
	ut_assert_console_end();

	/* Check the -a flag, efi should show as not in the order ("-") */
	ut_assertok(run_command("bootmeth list -a", 0));
	ut_assert_nextline("Order  Seq  Name                Description");
	ut_assert_nextlinen("---");
	ut_assert_nextline("    0    0  extlinux            Extlinux boot from a block device");
	ut_assert_nextline("    -    1  efi                 EFI boot from an .efi file");
	if (IS_ENABLED(CONFIG_BOOTMETH_GLOBAL))
		ut_assert_nextline(" glob    2  firmware0           VBE simple");
	ut_assert_nextlinen("---");
	ut_assert_nextline(IS_ENABLED(CONFIG_BOOTMETH_GLOBAL) ?
		 "(3 bootmeths)" : "(2 bootmeths)");
	ut_assert_console_end();

	/* Check the -a flag with the reverse order */
	ut_assertok(run_command("bootmeth order \"efi extlinux\"", 0));
	ut_assert_console_end();
	ut_assertok(run_command("bootmeth list -a", 0));
	ut_assert_nextline("Order  Seq  Name                Description");
	ut_assert_nextlinen("---");
	ut_assert_nextline("    1    0  extlinux            Extlinux boot from a block device");
	ut_assert_nextline("    0    1  efi                 EFI boot from an .efi file");
	if (IS_ENABLED(CONFIG_BOOTMETH_GLOBAL))
		ut_assert_nextline(" glob    2  firmware0           VBE simple");
	ut_assert_nextlinen("---");
	ut_assert_nextline(IS_ENABLED(CONFIG_BOOTMETH_GLOBAL) ?
		 "(3 bootmeths)" : "(2 bootmeths)");
	ut_assert_console_end();

	/* Now reset the order to empty, which should show all of them again */
	ut_assertok(run_command("bootmeth order", 0));
	ut_assert_console_end();
	ut_assertnull(env_get("bootmeths"));
	ut_assertok(run_command("bootmeth list", 0));
	ut_assert_skip_to_line(IS_ENABLED(CONFIG_BOOTMETH_GLOBAL) ?
		 "(3 bootmeths)" : "(2 bootmeths)");

	/* Try reverse order */
	ut_assertok(run_command("bootmeth order \"efi extlinux\"", 0));
	ut_assert_console_end();
	ut_assertok(run_command("bootmeth list", 0));
	ut_assert_nextline("Order  Seq  Name                Description");
	ut_assert_nextlinen("---");
	ut_assert_nextline("    0    1  efi                 EFI boot from an .efi file");
	ut_assert_nextline("    1    0  extlinux            Extlinux boot from a block device");
	ut_assert_nextlinen("---");
	ut_assert_nextline("(2 bootmeths)");
	ut_assertnonnull(env_get("bootmeths"));
	ut_asserteq_str("efi extlinux", env_get("bootmeths"));
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootmeth_cmd_order, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootmeth order' command with global bootmeths */
static int bootmeth_cmd_order_glob(struct unit_test_state *uts)
{
	if (!IS_ENABLED(CONFIG_BOOTMETH_GLOBAL))
		return -EAGAIN;

	console_record_reset_enable();
	ut_assertok(run_command("bootmeth order \"efi firmware0\"", 0));
	ut_assert_console_end();
	ut_assertok(run_command("bootmeth list", 0));
	ut_assert_nextline("Order  Seq  Name                Description");
	ut_assert_nextlinen("---");
	ut_assert_nextline("    0    1  efi                 EFI boot from an .efi file");
	ut_assert_nextline(" glob    2  firmware0           VBE simple");
	ut_assert_nextlinen("---");
	ut_assert_nextline("(2 bootmeths)");
	ut_assertnonnull(env_get("bootmeths"));
	ut_asserteq_str("efi firmware0", env_get("bootmeths"));
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootmeth_cmd_order_glob, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootmeths' env var */
static int bootmeth_env(struct unit_test_state *uts)
{
	struct bootstd_priv *std;

	ut_assertok(bootstd_get_priv(&std));

	/* Select just one bootmethod */
	console_record_reset_enable();
	ut_assertok(env_set("bootmeths", "extlinux"));
	ut_asserteq(1, std->bootmeth_count);

	/* Select an invalid bootmethod */
	ut_asserteq(1, run_command("setenv bootmeths fred", 0));
	ut_assert_nextline("Unknown bootmeth 'fred'");
	ut_assert_nextlinen("## Error inserting");
	ut_assert_console_end();

	ut_assertok(env_set("bootmeths", "efi extlinux"));
	ut_asserteq(2, std->bootmeth_count);
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootmeth_env, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check the get_state_desc() method */
static int bootmeth_state(struct unit_test_state *uts)
{
	struct udevice *dev;
	char buf[50];

	ut_assertok(uclass_first_device_err(UCLASS_BOOTMETH, &dev));
	ut_assertnonnull(dev);

	ut_assertok(bootmeth_get_state_desc(dev, buf, sizeof(buf)));
	ut_asserteq_str("OK", buf);

	return 0;
}
BOOTSTD_TEST(bootmeth_state, UT_TESTF_DM | UT_TESTF_SCAN_FDT);
