// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <fs.h>
#include <os.h>
#include <sandbox_host.h>
#include <asm/test.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Basic test of host interface */
static int dm_test_host(struct unit_test_state *uts)
{
	static char label[] = "test";
	struct udevice *dev, *part, *chk, *blk;
	struct host_sb_plat *plat;
	struct blk_desc *desc;
	char fname[256];
	ulong mem_start;
	loff_t actwrite;

	ut_asserteq(-ENODEV, uclass_first_device_err(UCLASS_HOST, &dev));
	ut_asserteq(-ENODEV, uclass_first_device_err(UCLASS_PARTITION, &part));

	mem_start = ut_check_delta(0);
	ut_assertok(host_create_device(label, true, DEFAULT_BLKSZ, &dev));

	/* Check that the plat data has been allocated */
	plat = dev_get_plat(dev);
	ut_asserteq_str("test", plat->label);
	ut_assert(label != plat->label);
	ut_asserteq(0, plat->fd);

	/* Attach a file created in test_ut_dm_init */
	ut_assertok(os_persistent_file(fname, sizeof(fname), "2MB.ext2.img"));

	ut_assertok(host_attach_file(dev, fname));
	ut_assertok(uclass_first_device_err(UCLASS_HOST, &chk));
	ut_asserteq_ptr(chk, dev);

	ut_asserteq_str(fname, plat->filename);
	ut_assert(fname != plat->filename);
	ut_assert(plat->fd != 0);

	/* Get the block device */
	ut_assertok(blk_get_from_parent(dev, &blk));
	ut_assertok(device_probe(blk));

	/* There should be no partition table in this device */
	ut_asserteq(-ENODEV, uclass_first_device_err(UCLASS_PARTITION, &part));

	/* Write to a file on the ext4 filesystem */
	desc = dev_get_uclass_plat(blk);
	ut_asserteq(true, desc->removable);
	ut_assertok(fs_set_blk_dev_with_part(desc, 0));
	ut_assertok(fs_write("/testing", 0, 0, 0x1000, &actwrite));

	ut_assertok(host_detach_file(dev));
	ut_asserteq(0, plat->fd);
	ut_asserteq(-ENODEV, blk_get_from_parent(dev, &blk));
	ut_assertok(device_unbind(dev));

	/* check there were no memory leaks */
	ut_asserteq(0, ut_check_delta(mem_start));

	return 0;
}
DM_TEST(dm_test_host, UT_TESTF_SCAN_FDT);

/* reusing the same label should work */
static int dm_test_host_dup(struct unit_test_state *uts)
{
	static char label[] = "test";
	struct udevice *dev, *chk;
	char fname[256];

	ut_asserteq(0, uclass_id_count(UCLASS_HOST));
	ut_assertok(host_create_device(label, true, DEFAULT_BLKSZ, &dev));

	/* Attach a file created in test_ut_dm_init */
	ut_assertok(os_persistent_file(fname, sizeof(fname), "2MB.ext2.img"));
	ut_assertok(host_attach_file(dev, fname));
	ut_assertok(uclass_first_device_err(UCLASS_HOST, &chk));
	ut_asserteq_ptr(chk, dev);
	ut_asserteq(1, uclass_id_count(UCLASS_HOST));

	/* Create another device with the same label (should remove old one) */
	ut_assertok(host_create_device(label, true, DEFAULT_BLKSZ, &dev));

	/* Attach a different file created in test_ut_dm_init */
	ut_assertok(os_persistent_file(fname, sizeof(fname), "1MB.fat32.img"));
	ut_assertok(host_attach_file(dev, fname));

	ut_assertok(uclass_first_device_err(UCLASS_HOST, &chk));
	ut_asserteq_ptr(chk, dev);

	/* Make sure there is still only one device */
	ut_asserteq(1, uclass_id_count(UCLASS_HOST));

	return 0;
}
DM_TEST(dm_test_host_dup, UT_TESTF_SCAN_FDT);

/* Basic test of 'host' command */
static int dm_test_cmd_host(struct unit_test_state *uts)
{
	struct udevice *dev, *blk;
	struct blk_desc *desc;
	char fname[256];

	console_record_reset();

	/* first check 'host info' with binding */
	ut_assertok(run_command("host info", 0));
	ut_assert_nextline("dev       blocks  blksz label           path");
	ut_assert_console_end();

	ut_assertok(os_persistent_file(fname, sizeof(fname), "2MB.ext2.img"));
	ut_assertok(run_commandf("host bind -r test2 %s", fname));

	/* Check the -r flag worked */
	ut_assertok(uclass_first_device_err(UCLASS_HOST, &dev));
	ut_assertok(blk_get_from_parent(dev, &blk));
	desc = dev_get_uclass_plat(blk);
	ut_asserteq(true, desc->removable);

	ut_assertok(run_command("host info", 0));
	ut_assert_nextline("dev       blocks  blksz label           path");
	ut_assert_nextlinen("  0         4096    512 test2");
	ut_assert_console_end();

	ut_assertok(os_persistent_file(fname, sizeof(fname), "1MB.fat32.img"));
	ut_assertok(run_commandf("host bind fat %s", fname));

	/* Check it is not removable (no '-r') */
	ut_assertok(uclass_next_device_err(&dev));
	ut_assertok(blk_get_from_parent(dev, &blk));
	desc = dev_get_uclass_plat(blk);
	ut_asserteq(false, desc->removable);

	ut_assertok(run_command("host info", 0));
	ut_assert_nextline("dev       blocks  blksz label           path");
	ut_assert_nextlinen("  0         4096    512 test2");
	ut_assert_nextlinen("  1         2048    512 fat");
	ut_assert_console_end();

	ut_asserteq(1, run_command("host info test", 0));
	ut_assert_nextline("No such device 'test'");
	ut_assert_console_end();

	ut_assertok(run_command("host info fat", 0));
	ut_assert_nextline("dev       blocks  blksz label           path");
	ut_assert_nextlinen("  1         2048    512 fat");
	ut_assert_console_end();

	/* check 'host dev' */
	ut_asserteq(1, run_command("host dev", 0));
	ut_assert_nextline("No current host device");
	ut_assert_console_end();

	ut_asserteq(1, run_command("host dev missing", 0));
	ut_assert_nextline("No such device 'missing'");
	ut_assert_console_end();

	ut_assertok(run_command("host dev fat", 0));
	ut_assert_console_end();

	ut_assertok(run_command("host dev", 0));
	ut_assert_nextline("Current host device: 1: fat");
	ut_assert_console_end();

	/* Try a numerical label */
	ut_assertok(run_command("host dev 0", 0));
	ut_assert_console_end();

	ut_assertok(run_command("host dev", 0));
	ut_assert_nextline("Current host device: 0: test2");
	ut_assert_console_end();

	/* Remove one of the bindings */
	ut_assertok(run_commandf("host unbind test2"));

	/* There should now be no current device */
	ut_asserteq(1, run_command("host dev", 0));
	ut_assert_nextline("No current host device");
	ut_assert_console_end();

	ut_assertok(run_command("host info", 0));
	ut_assert_nextline("dev       blocks  blksz label           path");
	ut_assert_nextlinen("  1         2048    512 fat");
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_cmd_host, UT_TESTF_SCAN_FDT);
