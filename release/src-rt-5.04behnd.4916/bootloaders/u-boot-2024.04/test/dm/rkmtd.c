// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Test derived from:
 * /test/dm/host.c
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Copyright (C) 2023 Johan Jonker <jbx6244@gmail.com>
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <fs.h>
#include <rkmtd.h>
#include <asm/test.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

#define RW_BUF_SIZE	12 * 512

/* Basic test of the RKMTD interface */
static int dm_test_rkmtd(struct unit_test_state *uts)
{
	struct udevice *dev, *part, *chk, *blk;
	char write[RW_BUF_SIZE], read[RW_BUF_SIZE];
	static const char label[] = "test";
	struct rkmtd_dev *plat;
	struct blk_desc *desc;
	struct sector0 *sec0;
	int i;

	ut_asserteq(-ENODEV, uclass_first_device_err(UCLASS_RKMTD, &dev));
	ut_asserteq(-ENODEV, uclass_first_device_err(UCLASS_PARTITION, &part));

	ut_assertok(rkmtd_create_device(label, &dev));

	/* Check that the plat data has been allocated */
	plat = dev_get_plat(dev);
	ut_asserteq_str("test", plat->label);
	ut_assert(label != plat->label);

	/* Attach RKMTD driver */
	ut_assertok(rkmtd_attach(dev));
	ut_assertok(uclass_first_device_err(UCLASS_RKMTD, &chk));
	ut_asserteq_ptr(chk, dev);

	/* Get RKMTD block device */
	ut_assertok(blk_get_from_parent(dev, &blk));
	ut_assertok(device_probe(blk));

	/* There should be a GPT partition table in this device */
	ut_asserteq(0, uclass_first_device_err(UCLASS_PARTITION, &part));

	/* Write a boot block and verify that we get the same data back */
	desc = dev_get_uclass_plat(blk);
	ut_asserteq(true, desc->removable);
	ut_asserteq(LBA, desc->lba);

	memset(write, '\0', BLK_SIZE);

	for (i = BLK_SIZE; i < sizeof(write); i++)
		write[i] = i;

	sec0 = (struct sector0 *)write;
	sec0->magic = 0x0FF0AA55;
	sec0->rc4_flag = 0;
	sec0->boot_code1_offset = 4;
	sec0->boot_code2_offset = 4;
	sec0->flash_data_size = 4;
	sec0->flash_boot_size = 8;

	rkmtd_rc4(write, 512);
	ut_asserteq(RK_TAG, sec0->magic);

	ut_asserteq(12, blk_dwrite(desc, 64, 12, write));
	ut_asserteq(12, blk_dread(desc, 64, 12, read));
	ut_asserteq_mem(write, read, RW_BUF_SIZE);

	ut_assertok(rkmtd_detach(dev));

	ut_asserteq(-ENODEV, blk_get_from_parent(dev, &blk));
	ut_assertok(device_unbind(dev));

	return 0;
}
DM_TEST(dm_test_rkmtd, UT_TESTF_SCAN_FDT);

/* Reusing the same label should work */
static int dm_test_rkmtd_dup(struct unit_test_state *uts)
{
	static const char label[] = "test";
	struct udevice *dev, *chk;

	/* Create a RKMTD device with label "test" */
	ut_asserteq(0, uclass_id_count(UCLASS_RKMTD));
	ut_assertok(rkmtd_create_device(label, &dev));
	ut_assertok(rkmtd_attach(dev));
	ut_assertok(uclass_first_device_err(UCLASS_RKMTD, &chk));
	ut_asserteq_ptr(chk, dev);
	ut_asserteq(1, uclass_id_count(UCLASS_RKMTD));

	/* Create another device with the same label (should remove old one) */
	ut_assertok(rkmtd_create_device(label, &dev));
	ut_assertok(rkmtd_attach(dev));
	ut_assertok(uclass_first_device_err(UCLASS_RKMTD, &chk));
	ut_asserteq_ptr(chk, dev);

	/* Make sure there is still only one device */
	ut_asserteq(1, uclass_id_count(UCLASS_RKMTD));

	return 0;
}
DM_TEST(dm_test_rkmtd_dup, UT_TESTF_SCAN_FDT);

/* Basic test of the 'rkmtd' command */
static int dm_test_rkmtd_cmd(struct unit_test_state *uts)
{
	struct udevice *dev, *blk;
	struct blk_desc *desc;

	/* First check 'rkmtd info' with binding */
	ut_assertok(run_command("rkmtd info", 0));
	ut_assert_nextline("dev       blocks label          ");
	ut_assert_console_end();

	/* Bind device 1 */
	ut_assertok(run_commandf("rkmtd bind test1"));
	ut_assertok(uclass_first_device_err(UCLASS_RKMTD, &dev));
	ut_assertok(blk_get_from_parent(dev, &blk));
	desc = dev_get_uclass_plat(blk);

	ut_assertok(run_command("rkmtd info", 0));
	ut_assert_nextline("dev       blocks label          ");
	ut_assert_nextline("  0          609 test1          ");
	ut_assert_console_end();

	/* Bind device 2 */
	ut_assertok(run_commandf("rkmtd bind test2"));
	ut_assertok(uclass_next_device_err(&dev));
	ut_assertok(blk_get_from_parent(dev, &blk));
	desc = dev_get_uclass_plat(blk);

	ut_assertok(run_command("rkmtd info", 0));
	ut_assert_nextline("dev       blocks label          ");
	ut_assert_nextline("  0          609 test1          ");
	ut_assert_nextline("  1          609 test2          ");
	ut_assert_console_end();

	ut_asserteq(1, run_command("rkmtd info test", 0));
	ut_assert_nextline("No such device 'test'");
	ut_assert_console_end();

	ut_assertok(run_command("rkmtd info test2", 0));
	ut_assert_nextline("dev       blocks label          ");
	ut_assert_nextline("  1          609 test2          ");
	ut_assert_console_end();

	/* Check 'rkmtd dev' */
	ut_asserteq(1, run_command("rkmtd dev", 0));
	ut_assert_nextline("No current rkmtd device");
	ut_assert_console_end();

	ut_asserteq(1, run_command("rkmtd dev missing", 0));
	ut_assert_nextline("No such device 'missing'");
	ut_assert_console_end();

	ut_assertok(run_command("rkmtd dev test2", 0));
	ut_assert_console_end();

	ut_assertok(run_command("rkmtd dev", 0));
	ut_assert_nextline("Current rkmtd device: 1: test2");
	ut_assert_console_end();

	/* Try a numerical label */
	ut_assertok(run_command("rkmtd dev 0", 0));
	ut_assert_console_end();

	ut_assertok(run_command("rkmtd dev", 0));
	ut_assert_nextline("Current rkmtd device: 0: test1");
	ut_assert_console_end();

	/* Remove one of the bindings */
	ut_assertok(run_commandf("rkmtd unbind test1"));

	/* There should now be no current device */
	ut_asserteq(1, run_command("rkmtd dev", 0));
	ut_assert_nextline("No current rkmtd device");
	ut_assert_console_end();

	ut_assertok(run_command("rkmtd info", 0));
	ut_assert_nextline("dev       blocks label          ");
	ut_assert_nextline("  1          609 test2          ");
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_rkmtd_cmd, UT_TESTF_SCAN_FDT | UT_TESTF_CONSOLE_REC);
