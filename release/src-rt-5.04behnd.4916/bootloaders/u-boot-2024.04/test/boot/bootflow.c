// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for bootdev functions. All start with 'bootdev'
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <cli.h>
#include <dm.h>
#include <expo.h>
#ifdef CONFIG_SANDBOX
#include <asm/test.h>
#endif
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <test/suites.h>
#include <test/ut.h>
#include "bootstd_common.h"
#include "../../boot/bootflow_internal.h"
#include "../../boot/scene_internal.h"

DECLARE_GLOBAL_DATA_PTR;

extern U_BOOT_DRIVER(bootmeth_cros);
extern U_BOOT_DRIVER(bootmeth_2script);

static int inject_response(struct unit_test_state *uts)
{
	/*
	 * The image being booted presents a menu of options:
	 *
	 * Fedora-Workstation-armhfp-31-1.9 Boot Options.
	 * 1:   Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
	 * Enter choice:
	 *
	 * Provide input for this, to avoid waiting two seconds for a timeout.
	 */
	ut_asserteq(2, console_in_puts("1\n"));

	return 0;
}

/* Check 'bootflow scan/list' commands */
static int bootflow_cmd(struct unit_test_state *uts)
{
	console_record_reset_enable();
	ut_assertok(run_command("bootdev select 1", 0));
	ut_assert_console_end();
	ut_assertok(run_command("bootflow scan -lH", 0));
	ut_assert_nextline("Scanning for bootflows in bootdev 'mmc1.bootdev'");
	ut_assert_nextline("Seq  Method       State   Uclass    Part  Name                      Filename");
	ut_assert_nextlinen("---");
	ut_assert_nextline("Scanning bootdev 'mmc2.bootdev':");
	ut_assert_nextline("Scanning bootdev 'mmc1.bootdev':");
	ut_assert_nextline("  0  extlinux     ready   mmc          1  mmc1.bootdev.part_1       /extlinux/extlinux.conf");
	ut_assert_nextline("No more bootdevs");
	ut_assert_nextlinen("---");
	ut_assert_nextline("(1 bootflow, 1 valid)");
	ut_assert_console_end();

	ut_assertok(run_command("bootflow list", 0));
	ut_assert_nextline("Showing bootflows for bootdev 'mmc1.bootdev'");
	ut_assert_nextline("Seq  Method       State   Uclass    Part  Name                      Filename");
	ut_assert_nextlinen("---");
	ut_assert_nextline("  0  extlinux     ready   mmc          1  mmc1.bootdev.part_1       /extlinux/extlinux.conf");
	ut_assert_nextlinen("---");
	ut_assert_nextline("(1 bootflow, 1 valid)");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_cmd, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootflow scan' with a label / seq */
static int bootflow_cmd_label(struct unit_test_state *uts)
{
	test_set_eth_enable(false);

	console_record_reset_enable();
	ut_assertok(run_command("bootflow scan -lH mmc1", 0));
	ut_assert_nextline("Scanning for bootflows with label 'mmc1'");
	ut_assert_skip_to_line("(1 bootflow, 1 valid)");
	ut_assert_console_end();

	ut_assertok(run_command("bootflow scan -lH 0", 0));
	ut_assert_nextline("Scanning for bootflows with label '0'");
	ut_assert_skip_to_line("(0 bootflows, 0 valid)");
	ut_assert_console_end();

	/*
	 * with ethernet enabled we have 8 devices ahead of the mmc ones:
	 *
	 * ut_assertok(run_command("bootdev list", 0));
	 * Seq  Probed  Status  Uclass    Name
	 * ---  ------  ------  --------  ------------------
	 * 0   [ + ]      OK  ethernet  eth@10002000.bootdev
	 * 1   [   ]      OK  ethernet  eth@10003000.bootdev
	 * 2   [   ]      OK  ethernet  sbe5.bootdev
	 * 3   [   ]      OK  ethernet  eth@10004000.bootdev
	 * 4   [   ]      OK  ethernet  phy-test-eth.bootdev
	 * 5   [   ]      OK  ethernet  dsa-test-eth.bootdev
	 * 6   [   ]      OK  ethernet  dsa-test@0.bootdev
	 * 7   [   ]      OK  ethernet  dsa-test@1.bootdev
	 * 8   [   ]      OK  mmc       mmc2.bootdev
	 * 9   [ + ]      OK  mmc       mmc1.bootdev
	 * a   [   ]      OK  mmc       mmc0.bootdev
	 */
	ut_assertok(run_command("bootflow scan -lH 9", 0));
	ut_assert_nextline("Scanning for bootflows with label '9'");
	ut_assert_skip_to_line("(1 bootflow, 1 valid)");

	ut_assertok(run_command("bootflow scan -lH 0", 0));
	ut_assert_nextline("Scanning for bootflows with label '0'");
	ut_assert_skip_to_line("(0 bootflows, 0 valid)");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_cmd_label, UT_TESTF_DM | UT_TESTF_SCAN_FDT |
	     UT_TESTF_ETH_BOOTDEV);

/* Check 'bootflow scan/list' commands using all bootdevs */
static int bootflow_cmd_glob(struct unit_test_state *uts)
{
	ut_assertok(bootstd_test_drop_bootdev_order(uts));

	console_record_reset_enable();
	ut_assertok(run_command("bootflow scan -lGH", 0));
	ut_assert_nextline("Scanning for bootflows in all bootdevs");
	ut_assert_nextline("Seq  Method       State   Uclass    Part  Name                      Filename");
	ut_assert_nextlinen("---");
	ut_assert_nextline("Scanning bootdev 'mmc2.bootdev':");
	ut_assert_nextline("Scanning bootdev 'mmc1.bootdev':");
	ut_assert_nextline("  0  extlinux     ready   mmc          1  mmc1.bootdev.part_1       /extlinux/extlinux.conf");
	ut_assert_nextline("Scanning bootdev 'mmc0.bootdev':");
	ut_assert_nextline("No more bootdevs");
	ut_assert_nextlinen("---");
	ut_assert_nextline("(1 bootflow, 1 valid)");
	ut_assert_console_end();

	ut_assertok(run_command("bootflow list", 0));
	ut_assert_nextline("Showing all bootflows");
	ut_assert_nextline("Seq  Method       State   Uclass    Part  Name                      Filename");
	ut_assert_nextlinen("---");
	ut_assert_nextline("  0  extlinux     ready   mmc          1  mmc1.bootdev.part_1       /extlinux/extlinux.conf");
	ut_assert_nextlinen("---");
	ut_assert_nextline("(1 bootflow, 1 valid)");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_cmd_glob, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootflow scan -e' */
static int bootflow_cmd_scan_e(struct unit_test_state *uts)
{
	ut_assertok(bootstd_test_drop_bootdev_order(uts));

	console_record_reset_enable();
	ut_assertok(run_command("bootflow scan -aleGH", 0));
	ut_assert_nextline("Scanning for bootflows in all bootdevs");
	ut_assert_nextline("Seq  Method       State   Uclass    Part  Name                      Filename");
	ut_assert_nextlinen("---");
	ut_assert_nextline("Scanning bootdev 'mmc2.bootdev':");
	ut_assert_nextline("  0  extlinux     media   mmc          0  mmc2.bootdev.whole        ");
	ut_assert_nextline("     ** No partition found, err=-93: Protocol not supported");
	ut_assert_nextline("  1  efi          media   mmc          0  mmc2.bootdev.whole        ");
	ut_assert_nextline("     ** No partition found, err=-93: Protocol not supported");

	ut_assert_nextline("Scanning bootdev 'mmc1.bootdev':");
	ut_assert_nextline("  2  extlinux     media   mmc          0  mmc1.bootdev.whole        ");
	ut_assert_nextline("     ** No partition found, err=-2: No such file or directory");
	ut_assert_nextline("  3  efi          media   mmc          0  mmc1.bootdev.whole        ");
	ut_assert_nextline("     ** No partition found, err=-2: No such file or directory");
	ut_assert_nextline("  4  extlinux     ready   mmc          1  mmc1.bootdev.part_1       /extlinux/extlinux.conf");
	ut_assert_nextline("  5  efi          fs      mmc          1  mmc1.bootdev.part_1       efi/boot/bootsbox.efi");

	ut_assert_skip_to_line("Scanning bootdev 'mmc0.bootdev':");
	ut_assert_skip_to_line(
		" 3f  efi          media   mmc          0  mmc0.bootdev.whole        ");
	ut_assert_nextline("     ** No partition found, err=-93: Protocol not supported");
	ut_assert_nextline("No more bootdevs");
	ut_assert_nextlinen("---");
	ut_assert_nextline("(64 bootflows, 1 valid)");
	ut_assert_console_end();

	ut_assertok(run_command("bootflow list", 0));
	ut_assert_nextline("Showing all bootflows");
	ut_assert_nextline("Seq  Method       State   Uclass    Part  Name                      Filename");
	ut_assert_nextlinen("---");
	ut_assert_nextline("  0  extlinux     media   mmc          0  mmc2.bootdev.whole        ");
	ut_assert_nextline("  1  efi          media   mmc          0  mmc2.bootdev.whole        ");
	ut_assert_skip_to_line(
		"  4  extlinux     ready   mmc          1  mmc1.bootdev.part_1       /extlinux/extlinux.conf");
	ut_assert_skip_to_line(" 3f  efi          media   mmc          0  mmc0.bootdev.whole        ");
	ut_assert_nextlinen("---");
	ut_assert_nextline("(64 bootflows, 1 valid)");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_cmd_scan_e, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootflow info' */
static int bootflow_cmd_info(struct unit_test_state *uts)
{
	console_record_reset_enable();
	ut_assertok(run_command("bootdev select 1", 0));
	ut_assert_console_end();
	ut_assertok(run_command("bootflow scan", 0));
	ut_assert_console_end();
	ut_assertok(run_command("bootflow select 0", 0));
	ut_assert_console_end();
	ut_assertok(run_command("bootflow info", 0));
	ut_assert_nextline("Name:      mmc1.bootdev.part_1");
	ut_assert_nextline("Device:    mmc1.bootdev");
	ut_assert_nextline("Block dev: mmc1.blk");
	ut_assert_nextline("Method:    extlinux");
	ut_assert_nextline("State:     ready");
	ut_assert_nextline("Partition: 1");
	ut_assert_nextline("Subdir:    (none)");
	ut_assert_nextline("Filename:  /extlinux/extlinux.conf");
	ut_assert_nextlinen("Buffer:    ");
	ut_assert_nextline("Size:      253 (595 bytes)");
	ut_assert_nextline("OS:        Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)");
	ut_assert_nextline("Cmdline:   (none)");
	ut_assert_nextline("Logo:      (none)");
	ut_assert_nextline("FDT:       <NULL>");
	ut_assert_nextline("Error:     0");
	ut_assert_console_end();

	ut_assertok(run_command("bootflow info -d", 0));
	ut_assert_nextline("Name:      mmc1.bootdev.part_1");
	ut_assert_skip_to_line("Error:     0");
	ut_assert_nextline("Contents:");
	ut_assert_nextline("%s", "");
	ut_assert_nextline("# extlinux.conf generated by appliance-creator");
	ut_assert_skip_to_line("        initrd /initramfs-5.3.7-301.fc31.armv7hl.img");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_cmd_info, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootflow scan -b' to boot the first available bootdev */
static int bootflow_scan_boot(struct unit_test_state *uts)
{
	console_record_reset_enable();
	ut_assertok(inject_response(uts));
	ut_assertok(run_command("bootflow scan -b", 0));
	ut_assert_nextline(
		"** Booting bootflow 'mmc1.bootdev.part_1' with extlinux");
	ut_assert_nextline("Ignoring unknown command: ui");

	/*
	 * We expect it to get through to boot although sandbox always returns
	 * -EFAULT as it cannot actually boot the kernel
	 */
	ut_assert_skip_to_line("sandbox: continuing, as we cannot run Linux");
	ut_assert_nextline("Boot failed (err=-14)");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_scan_boot, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check iterating through available bootflows */
static int bootflow_iter(struct unit_test_state *uts)
{
	struct bootflow_iter iter;
	struct bootflow bflow;

	bootstd_clear_glob();

	/* The first device is mmc2.bootdev which has no media */
	ut_asserteq(-EPROTONOSUPPORT,
		    bootflow_scan_first(NULL, NULL, &iter,
					BOOTFLOWIF_ALL | BOOTFLOWIF_SKIP_GLOBAL, &bflow));
	ut_asserteq(2, iter.num_methods);
	ut_asserteq(0, iter.cur_method);
	ut_asserteq(0, iter.part);
	ut_asserteq(0, iter.max_part);
	ut_asserteq_str("extlinux", iter.method->name);
	ut_asserteq(0, bflow.err);

	/*
	 * This shows MEDIA even though there is none, since in
	 * bootdev_find_in_blk() we call part_get_info() which returns
	 * -EPROTONOSUPPORT. Ideally it would return -EEOPNOTSUPP and we would
	 * know.
	 */
	ut_asserteq(BOOTFLOWST_MEDIA, bflow.state);

	ut_asserteq(-EPROTONOSUPPORT, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(2, iter.num_methods);
	ut_asserteq(1, iter.cur_method);
	ut_asserteq(0, iter.part);
	ut_asserteq(0, iter.max_part);
	ut_asserteq_str("efi", iter.method->name);
	ut_asserteq(0, bflow.err);
	ut_asserteq(BOOTFLOWST_MEDIA, bflow.state);
	bootflow_free(&bflow);

	/* The next device is mmc1.bootdev - at first we use the whole device */
	ut_asserteq(-ENOENT, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(2, iter.num_methods);
	ut_asserteq(0, iter.cur_method);
	ut_asserteq(0, iter.part);
	ut_asserteq(0x1e, iter.max_part);
	ut_asserteq_str("extlinux", iter.method->name);
	ut_asserteq(0, bflow.err);
	ut_asserteq(BOOTFLOWST_MEDIA, bflow.state);
	bootflow_free(&bflow);

	ut_asserteq(-ENOENT, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(2, iter.num_methods);
	ut_asserteq(1, iter.cur_method);
	ut_asserteq(0, iter.part);
	ut_asserteq(0x1e, iter.max_part);
	ut_asserteq_str("efi", iter.method->name);
	ut_asserteq(0, bflow.err);
	ut_asserteq(BOOTFLOWST_MEDIA, bflow.state);
	bootflow_free(&bflow);

	/* Then more to partition 1 where we find something */
	ut_assertok(bootflow_scan_next(&iter, &bflow));
	ut_asserteq(2, iter.num_methods);
	ut_asserteq(0, iter.cur_method);
	ut_asserteq(1, iter.part);
	ut_asserteq(0x1e, iter.max_part);
	ut_asserteq_str("extlinux", iter.method->name);
	ut_asserteq(0, bflow.err);
	ut_asserteq(BOOTFLOWST_READY, bflow.state);
	bootflow_free(&bflow);

	ut_asserteq(-ENOENT, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(2, iter.num_methods);
	ut_asserteq(1, iter.cur_method);
	ut_asserteq(1, iter.part);
	ut_asserteq(0x1e, iter.max_part);
	ut_asserteq_str("efi", iter.method->name);
	ut_asserteq(0, bflow.err);
	ut_asserteq(BOOTFLOWST_FS, bflow.state);
	bootflow_free(&bflow);

	/* Then more to partition 2 which exists but is not bootable */
	ut_asserteq(-EINVAL, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(2, iter.num_methods);
	ut_asserteq(0, iter.cur_method);
	ut_asserteq(2, iter.part);
	ut_asserteq(0x1e, iter.max_part);
	ut_asserteq_str("extlinux", iter.method->name);
	ut_asserteq(0, bflow.err);
	ut_asserteq(BOOTFLOWST_MEDIA, bflow.state);
	bootflow_free(&bflow);

	bootflow_iter_uninit(&iter);

	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_iter, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

#if defined(CONFIG_SANDBOX) && defined(CONFIG_BOOTMETH_GLOBAL)
/* Check using the system bootdev */
static int bootflow_system(struct unit_test_state *uts)
{
	struct udevice *bootstd, *dev;

	if (!IS_ENABLED(CONFIG_EFI_BOOTMGR))
		return -EAGAIN;
	ut_assertok(uclass_first_device_err(UCLASS_BOOTSTD, &bootstd));
	ut_assertok(device_bind(bootstd, DM_DRIVER_GET(bootmeth_efi_mgr),
				"efi_mgr", 0, ofnode_null(), &dev));
	ut_assertok(device_probe(dev));
	sandbox_set_fake_efi_mgr_dev(dev, true);

	/* We should get a single 'bootmgr' method right at the end */
	bootstd_clear_glob();
	console_record_reset_enable();
	ut_assertok(run_command("bootflow scan -lH", 0));
	ut_assert_skip_to_line(
		"  0  efi_mgr      ready   (none)       0  <NULL>                    ");
	ut_assert_skip_to_line("No more bootdevs");
	ut_assert_skip_to_line("(2 bootflows, 2 valid)");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_system, UT_TESTF_DM | UT_TESTF_SCAN_PDATA |
	     UT_TESTF_SCAN_FDT);
#endif

/* Check disabling a bootmethod if it requests it */
static int bootflow_iter_disable(struct unit_test_state *uts)
{
	struct udevice *bootstd, *dev;
	struct bootflow_iter iter;
	struct bootflow bflow;
	int i;

	/* Add the EFI bootmgr driver */
	ut_assertok(uclass_first_device_err(UCLASS_BOOTSTD, &bootstd));
	ut_assertok(device_bind_driver(bootstd, "bootmeth_sandbox", "sandbox",
				       &dev));

	ut_assertok(bootstd_test_drop_bootdev_order(uts));

	bootstd_clear_glob();
	console_record_reset_enable();
	ut_assertok(inject_response(uts));
	ut_assertok(run_command("bootflow scan -lbH", 0));

	/* Try to boot the bootmgr flow, which will fail */
	console_record_reset_enable();
	ut_assertok(bootflow_scan_first(NULL, NULL, &iter, 0, &bflow));
	ut_asserteq(3, iter.num_methods);
	ut_asserteq_str("sandbox", iter.method->name);
	ut_assertok(inject_response(uts));
	ut_asserteq(-ENOTSUPP, bootflow_run_boot(&iter, &bflow));

	ut_assert_skip_to_line("Boot method 'sandbox' failed and will not be retried");
	ut_assert_console_end();

	/* Check that the sandbox bootmeth has been removed */
	ut_asserteq(2, iter.num_methods);
	for (i = 0; i < iter.num_methods; i++)
		ut_assert(strcmp("sandbox", iter.method_order[i]->name));

	return 0;
}
BOOTSTD_TEST(bootflow_iter_disable, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootflow scan' with a bootmeth ordering including a global bootmeth */
static int bootflow_scan_glob_bootmeth(struct unit_test_state *uts)
{
	if (!IS_ENABLED(CONFIG_BOOTMETH_GLOBAL))
		return -EAGAIN;

	ut_assertok(bootstd_test_drop_bootdev_order(uts));

	/*
	 * Make sure that the -G flag makes the scan fail, since this is not
	 * supported when an ordering is provided
	 */
	console_record_reset_enable();
	ut_assertok(bootmeth_set_order("efi firmware0"));
	ut_assertok(run_command("bootflow scan -lGH", 0));
	ut_assert_nextline("Scanning for bootflows in all bootdevs");
	ut_assert_nextline(
		"Seq  Method       State   Uclass    Part  Name                      Filename");
	ut_assert_nextlinen("---");
	ut_assert_nextlinen("---");
	ut_assert_nextline("(0 bootflows, 0 valid)");
	ut_assert_console_end();

	ut_assertok(run_command("bootflow scan -lH", 0));
	ut_assert_nextline("Scanning for bootflows in all bootdevs");
	ut_assert_nextline(
		"Seq  Method       State   Uclass    Part  Name                      Filename");
	ut_assert_nextlinen("---");
	ut_assert_nextline("Scanning global bootmeth 'firmware0':");
	ut_assert_nextline("Scanning bootdev 'mmc2.bootdev':");
	ut_assert_nextline("Scanning bootdev 'mmc1.bootdev':");
	ut_assert_nextline("Scanning bootdev 'mmc0.bootdev':");
	ut_assert_nextline("No more bootdevs");
	ut_assert_nextlinen("---");
	ut_assert_nextline("(0 bootflows, 0 valid)");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_scan_glob_bootmeth, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootflow boot' to boot a selected bootflow */
static int bootflow_cmd_boot(struct unit_test_state *uts)
{
	console_record_reset_enable();
	ut_assertok(run_command("bootdev select 1", 0));
	ut_assert_console_end();
	ut_assertok(run_command("bootflow scan", 0));
	ut_assert_console_end();
	ut_assertok(run_command("bootflow select 0", 0));
	ut_assert_console_end();

	ut_assertok(inject_response(uts));
	ut_asserteq(1, run_command("bootflow boot", 0));
	ut_assert_nextline(
		"** Booting bootflow 'mmc1.bootdev.part_1' with extlinux");
	ut_assert_nextline("Ignoring unknown command: ui");

	/*
	 * We expect it to get through to boot although sandbox always returns
	 * -EFAULT as it cannot actually boot the kernel
	 */
	ut_assert_skip_to_line("sandbox: continuing, as we cannot run Linux");
	ut_assert_nextline("Boot failed (err=-14)");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_cmd_boot, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/**
 * prep_mmc_bootdev() - Set up an mmc bootdev so we can access other distros
 *
 * After calling this function, set std->bootdev_order to *@old_orderp to
 * restore normal operation of bootstd (i.e. with the original bootdev order)
 *
 * @uts: Unit test state
 * @mmc_dev: MMC device to use, e.g. "mmc4". Note that this must remain valid
 *	in the caller until
 * @bind_cros: true to bind the ChromiumOS bootmeth
 * @old_orderp: Returns the original bootdev order, which must be restored
 * Returns 0 on success, -ve on failure
 */
static int prep_mmc_bootdev(struct unit_test_state *uts, const char *mmc_dev,
			    bool bind_cros, const char ***old_orderp)
{
	static const char *order[] = {"mmc2", "mmc1", NULL, NULL};
	struct udevice *dev, *bootstd;
	struct bootstd_priv *std;
	const char **old_order;
	ofnode root, node;

	order[2] = mmc_dev;

	/* Enable the mmc4 node since we need a second bootflow */
	root = oftree_root(oftree_default());
	node = ofnode_find_subnode(root, mmc_dev);
	ut_assert(ofnode_valid(node));
	ut_assertok(lists_bind_fdt(gd->dm_root, node, &dev, NULL, false));

	/* Enable the script bootmeth too */
	ut_assertok(uclass_first_device_err(UCLASS_BOOTSTD, &bootstd));
	ut_assertok(device_bind(bootstd, DM_DRIVER_REF(bootmeth_2script),
				"bootmeth_script", 0, ofnode_null(), &dev));

	/* Enable the cros bootmeth if needed */
	if (bind_cros) {
		ut_assertok(uclass_first_device_err(UCLASS_BOOTSTD, &bootstd));
		ut_assertok(device_bind(bootstd, DM_DRIVER_REF(bootmeth_cros),
					"cros", 0, ofnode_null(), &dev));
	}

	/* Change the order to include the device */
	std = dev_get_priv(bootstd);
	old_order = std->bootdev_order;
	std->bootdev_order = order;
	*old_orderp = old_order;

	return 0;
}

/**
 * scan_mmc_bootdev() - Set up an mmc bootdev so we can access other distros
 *
 * @uts: Unit test state
 * @mmc_dev: MMC device to use, e.g. "mmc4"
 * @bind_cros: true to bind the ChromiumOS bootmeth
 * Returns 0 on success, -ve on failure
 */
static int scan_mmc_bootdev(struct unit_test_state *uts, const char *mmc_dev,
			    bool bind_cros)
{
	struct bootstd_priv *std;
	struct udevice *bootstd;
	const char **old_order;

	ut_assertok(prep_mmc_bootdev(uts, mmc_dev, bind_cros, &old_order));

	console_record_reset_enable();
	ut_assertok(run_command("bootflow scan", 0));
	ut_assert_console_end();

	/* Restore the order used by the device tree */
	ut_assertok(uclass_first_device_err(UCLASS_BOOTSTD, &bootstd));
	std = dev_get_priv(bootstd);
	std->bootdev_order = old_order;

	return 0;
}

/**
 * scan_mmc4_bootdev() - Set up the mmc4 bootdev so we can access a fake Armbian
 *
 * @uts: Unit test state
 * Returns 0 on success, -ve on failure
 */
static int scan_mmc4_bootdev(struct unit_test_state *uts)
{
	ut_assertok(scan_mmc_bootdev(uts, "mmc4", false));

	return 0;
}

/* Check 'bootflow menu' to select a bootflow */
static int bootflow_cmd_menu(struct unit_test_state *uts)
{
	struct bootstd_priv *std;
	char prev[3];

	/* get access to the current bootflow */
	ut_assertok(bootstd_get_priv(&std));

	ut_assertok(scan_mmc4_bootdev(uts));

	/* Add keypresses to move to and select the second one in the list */
	prev[0] = CTL_CH('n');
	prev[1] = '\r';
	prev[2] = '\0';
	ut_asserteq(2, console_in_puts(prev));

	ut_assertok(run_command("bootflow menu", 0));
	ut_assert_nextline("Selected: Armbian");
	ut_assertnonnull(std->cur_bootflow);
	ut_assert_console_end();

	/* Check not selecting anything */
	prev[0] = '\e';
	prev[1] = '\0';
	ut_asserteq(1, console_in_puts(prev));

	ut_asserteq(1, run_command("bootflow menu", 0));
	ut_assertnull(std->cur_bootflow);
	ut_assert_nextline("Nothing chosen");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_cmd_menu, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootflow scan -m' to select a bootflow using a menu */
static int bootflow_scan_menu(struct unit_test_state *uts)
{
	struct bootstd_priv *std;
	const char **old_order, **new_order;
	char prev[3];

	/* get access to the current bootflow */
	ut_assertok(bootstd_get_priv(&std));

	ut_assertok(prep_mmc_bootdev(uts, "mmc4", false, &old_order));

	/* Add keypresses to move to and select the second one in the list */
	prev[0] = CTL_CH('n');
	prev[1] = '\r';
	prev[2] = '\0';
	ut_asserteq(2, console_in_puts(prev));

	ut_assertok(run_command("bootflow scan -lm", 0));
	new_order = std->bootdev_order;
	std->bootdev_order = old_order;

	ut_assert_skip_to_line("No more bootdevs");
	ut_assert_nextlinen("--");
	ut_assert_nextline("(2 bootflows, 2 valid)");

	ut_assert_nextline("Selected: Armbian");
	ut_assertnonnull(std->cur_bootflow);
	ut_assert_console_end();

	/* Check not selecting anything */
	prev[0] = '\e';
	prev[1] = '\0';
	ut_asserteq(1, console_in_puts(prev));

	std->bootdev_order = new_order; /* Blue Monday */
	ut_assertok(run_command("bootflow scan -lm", 0));
	std->bootdev_order = old_order;

	ut_assertnull(std->cur_bootflow);
	ut_assert_skip_to_line("(2 bootflows, 2 valid)");
	ut_assert_nextline("Nothing chosen");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_scan_menu,
	     UT_TESTF_DM | UT_TESTF_SCAN_FDT | UT_TESTF_CONSOLE_REC);

/* Check 'bootflow scan -mb' to select and boot a bootflow using a menu */
static int bootflow_scan_menu_boot(struct unit_test_state *uts)
{
	struct bootstd_priv *std;
	const char **old_order;
	char prev[3];

	/* get access to the current bootflow */
	ut_assertok(bootstd_get_priv(&std));

	ut_assertok(prep_mmc_bootdev(uts, "mmc4", false, &old_order));

	/* Add keypresses to move to and select the second one in the list */
	prev[0] = CTL_CH('n');
	prev[1] = '\r';
	prev[2] = '\0';
	ut_asserteq(2, console_in_puts(prev));

	ut_assertok(run_command("bootflow scan -lmb", 0));
	std->bootdev_order = old_order;

	ut_assert_skip_to_line("(2 bootflows, 2 valid)");

	ut_assert_nextline("Selected: Armbian");

	if (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		/*
		 * With old hush, despite booti failing to boot, i.e. returning
		 * CMD_RET_FAILURE, run_command() returns 0 which leads bootflow_boot(), as
		 * we are using bootmeth_script here, to return -EFAULT.
		 */
		ut_assert_skip_to_line("Boot failed (err=-14)");
	} else if (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		/*
		 * While with modern one, run_command() propagates CMD_RET_FAILURE returned
		 * by booti, so we get 1 here.
		 */
		ut_assert_skip_to_line("Boot failed (err=1)");
	}
	ut_assertnonnull(std->cur_bootflow);
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_scan_menu_boot,
	     UT_TESTF_DM | UT_TESTF_SCAN_FDT | UT_TESTF_CONSOLE_REC);

/* Check searching for a single bootdev using the hunters */
static int bootflow_cmd_hunt_single(struct unit_test_state *uts)
{
	struct bootstd_priv *std;

	/* get access to the used hunters */
	ut_assertok(bootstd_get_priv(&std));

	ut_assertok(bootstd_test_drop_bootdev_order(uts));

	console_record_reset_enable();
	ut_assertok(run_command("bootflow scan -l mmc1", 0));
	ut_assert_nextline("Scanning for bootflows with label 'mmc1'");
	ut_assert_skip_to_line("(1 bootflow, 1 valid)");
	ut_assert_console_end();

	/* check that the hunter was used */
	ut_asserteq(BIT(MMC_HUNTER) | BIT(1), std->hunters_used);

	return 0;
}
BOOTSTD_TEST(bootflow_cmd_hunt_single, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check searching for a uclass label using the hunters */
static int bootflow_cmd_hunt_label(struct unit_test_state *uts)
{
	struct bootstd_priv *std;

	/* get access to the used hunters */
	ut_assertok(bootstd_get_priv(&std));

	test_set_skip_delays(true);
	test_set_eth_enable(false);
	ut_assertok(bootstd_test_drop_bootdev_order(uts));

	console_record_reset_enable();
	ut_assertok(run_command("bootflow scan -l mmc", 0));

	/* check that the hunter was used */
	ut_asserteq(BIT(MMC_HUNTER) | BIT(1), std->hunters_used);

	/* check that we got the mmc1 bootflow */
	ut_assert_nextline("Scanning for bootflows with label 'mmc'");
	ut_assert_nextlinen("Seq");
	ut_assert_nextlinen("---");
	ut_assert_nextline("Hunting with: simple_bus");
	ut_assert_nextline("Found 2 extension board(s).");
	ut_assert_nextline("Hunting with: mmc");
	ut_assert_nextline("Scanning bootdev 'mmc2.bootdev':");
	ut_assert_nextline("Scanning bootdev 'mmc1.bootdev':");
	ut_assert_nextline(
		"  0  extlinux     ready   mmc          1  mmc1.bootdev.part_1       /extlinux/extlinux.conf");
	ut_assert_nextline("Scanning bootdev 'mmc0.bootdev':");
	ut_assert_skip_to_line("(1 bootflow, 1 valid)");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_cmd_hunt_label, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/**
 * check_font() - Check that the font size for an item matches expectations
 *
 * @uts: Unit test state
 * @scn: Scene containing the text object
 * @id: ID of the text object
 * Returns 0 on success, -ve on failure
 */
static int check_font(struct unit_test_state *uts, struct scene *scn, uint id,
		      int font_size)
{
	struct scene_obj_txt *txt;

	txt = scene_obj_find(scn, id, SCENEOBJT_TEXT);
	ut_assertnonnull(txt);

	ut_asserteq(font_size, txt->font_size);

	return 0;
}

/* Check themes work with a bootflow menu */
static int bootflow_menu_theme(struct unit_test_state *uts)
{
	const int font_size = 30;
	struct scene *scn;
	struct expo *exp;
	ofnode node;
	int i;

	ut_assertok(scan_mmc4_bootdev(uts));

	ut_assertok(bootflow_menu_new(&exp));
	node = ofnode_path("/bootstd/theme");
	ut_assert(ofnode_valid(node));
	ut_assertok(bootflow_menu_apply_theme(exp, node));

	scn = expo_lookup_scene_id(exp, MAIN);
	ut_assertnonnull(scn);

	/*
	 * Check that the txt objects have the correct font size from the
	 * device tree node: bootstd/theme
	 *
	 * Check both menu items, since there are two bootflows
	 */
	ut_assertok(check_font(uts, scn, OBJ_PROMPT, font_size));
	ut_assertok(check_font(uts, scn, OBJ_POINTER, font_size));
	for (i = 0; i < 2; i++) {
		ut_assertok(check_font(uts, scn, ITEM_DESC + i, font_size));
		ut_assertok(check_font(uts, scn, ITEM_KEY + i, font_size));
		ut_assertok(check_font(uts, scn, ITEM_LABEL + i, font_size));
	}

	expo_destroy(exp);

	return 0;
}
BOOTSTD_TEST(bootflow_menu_theme, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/**
 * check_arg() - Check both the normal case and the buffer-overflow case
 *
 * @uts: Unit-test state
 * @expect_ret: Expected return value (i.e. buffer length)
 * @expect_str: String expected to be returned
 * @buf: Buffer to use
 * @from: Original cmdline to update
 * @arg: Argument to update (e.g. "console")
 * @val: Value to set (e.g. "ttyS2") or NULL to delete the argument if present,
 * "" to set it to an empty value (e.g. "console=") and BOOTFLOWCL_EMPTY to add
 * it without any value ("initrd")
 */
static int check_arg(struct unit_test_state *uts, int expect_ret,
		     const char *expect_str, char *buf, const char *from,
		     const char *arg, const char *val)
{
	/* check for writing outside the reported bounds */
	buf[expect_ret] = '[';
	ut_asserteq(expect_ret,
		    cmdline_set_arg(buf, expect_ret, from, arg, val, NULL));
	ut_asserteq_str(expect_str, buf);
	ut_asserteq('[', buf[expect_ret]);

	/* do the test again but with one less byte in the buffer */
	ut_asserteq(-E2BIG, cmdline_set_arg(buf, expect_ret - 1, from, arg,
					    val, NULL));

	return 0;
}

/* Test of bootflow_cmdline_set_arg() */
static int test_bootflow_cmdline_set(struct unit_test_state *uts)
{
	char buf[50];
	const int size = sizeof(buf);

	/*
	 * note that buffer-overflow tests are immediately each test case, just
	 * top keep the code together
	 */

	/* add an arg that doesn't already exist, starting from empty */
	ut_asserteq(-ENOENT, cmdline_set_arg(buf, size, NULL, "me", NULL,
					     NULL));

	ut_assertok(check_arg(uts, 3, "me", buf, NULL, "me", BOOTFLOWCL_EMPTY));
	ut_assertok(check_arg(uts, 4, "me=", buf, NULL, "me", ""));
	ut_assertok(check_arg(uts, 8, "me=fred", buf, NULL, "me", "fred"));

	/* add an arg that doesn't already exist, starting from non-empty */
	ut_assertok(check_arg(uts, 11, "arg=123 me", buf, "arg=123", "me",
			      BOOTFLOWCL_EMPTY));
	ut_assertok(check_arg(uts, 12, "arg=123 me=", buf, "arg=123", "me",
			      ""));
	ut_assertok(check_arg(uts, 16, "arg=123 me=fred", buf, "arg=123", "me",
			      "fred"));

	/* update an arg at the start */
	ut_assertok(check_arg(uts, 1, "", buf, "arg=123", "arg", NULL));
	ut_assertok(check_arg(uts, 4, "arg", buf, "arg=123", "arg",
			      BOOTFLOWCL_EMPTY));
	ut_assertok(check_arg(uts, 5, "arg=", buf, "arg=123", "arg", ""));
	ut_assertok(check_arg(uts, 6, "arg=1", buf, "arg=123", "arg", "1"));
	ut_assertok(check_arg(uts, 9, "arg=1234", buf, "arg=123", "arg",
			      "1234"));

	/* update an arg at the end */
	ut_assertok(check_arg(uts, 5, "mary", buf, "mary arg=123", "arg",
			      NULL));
	ut_assertok(check_arg(uts, 9, "mary arg", buf, "mary arg=123", "arg",
			      BOOTFLOWCL_EMPTY));
	ut_assertok(check_arg(uts, 10, "mary arg=", buf, "mary arg=123", "arg",
			      ""));
	ut_assertok(check_arg(uts, 11, "mary arg=1", buf, "mary arg=123", "arg",
			      "1"));
	ut_assertok(check_arg(uts, 14, "mary arg=1234", buf, "mary arg=123",
			      "arg", "1234"));

	/* update an arg in the middle */
	ut_assertok(check_arg(uts, 16, "mary=abc john=2", buf,
			      "mary=abc arg=123 john=2", "arg", NULL));
	ut_assertok(check_arg(uts, 20, "mary=abc arg john=2", buf,
			      "mary=abc arg=123 john=2", "arg",
			      BOOTFLOWCL_EMPTY));
	ut_assertok(check_arg(uts, 21, "mary=abc arg= john=2", buf,
			      "mary=abc arg=123 john=2", "arg", ""));
	ut_assertok(check_arg(uts, 22, "mary=abc arg=1 john=2", buf,
			      "mary=abc arg=123 john=2", "arg", "1"));
	ut_assertok(check_arg(uts, 25, "mary=abc arg=1234 john=2", buf,
			      "mary=abc arg=123 john=2", "arg", "1234"));

	/* handle existing args with quotes */
	ut_assertok(check_arg(uts, 16, "mary=\"abc\" john", buf,
			      "mary=\"abc\" arg=123 john", "arg", NULL));

	/* handle existing args with quoted spaces */
	ut_assertok(check_arg(uts, 20, "mary=\"abc def\" john", buf,
			      "mary=\"abc def\" arg=123 john", "arg", NULL));

	ut_assertok(check_arg(uts, 34, "mary=\"abc def\" arg=123 john def=4",
			      buf, "mary=\"abc def\" arg=123 john", "def",
			      "4"));

	/* quote at the start */
	ut_asserteq(-EBADF, cmdline_set_arg(buf, size,
					    "mary=\"abc def\" arg=\"123 456\"",
					    "arg", "\"4 5 6", NULL));

	/* quote at the end */
	ut_asserteq(-EBADF, cmdline_set_arg(buf, size,
					    "mary=\"abc def\" arg=\"123 456\"",
					    "arg", "4 5 6\"", NULL));

	/* quote in the middle */
	ut_asserteq(-EBADF, cmdline_set_arg(buf, size,
					    "mary=\"abc def\" arg=\"123 456\"",
					    "arg", "\"4 \"5 6\"", NULL));

	/* handle updating a quoted arg */
	ut_assertok(check_arg(uts, 27, "mary=\"abc def\" arg=\"4 5 6\"", buf,
			      "mary=\"abc def\" arg=\"123 456\"", "arg",
			      "4 5 6"));

	/* changing a quoted arg to a non-quoted arg */
	ut_assertok(check_arg(uts, 23, "mary=\"abc def\" arg=789", buf,
			      "mary=\"abc def\" arg=\"123 456\"", "arg",
			      "789"));

	/* changing a non-quoted arg to a quoted arg */
	ut_assertok(check_arg(uts, 29, "mary=\"abc def\" arg=\"456 789\"", buf,
			      "mary=\"abc def\" arg=123", "arg", "456 789"));

	/* handling of spaces */
	ut_assertok(check_arg(uts, 8, "arg=123", buf, " ", "arg", "123"));
	ut_assertok(check_arg(uts, 8, "arg=123", buf, "   ", "arg", "123"));
	ut_assertok(check_arg(uts, 13, "john arg=123", buf, " john  ", "arg",
			      "123"));
	ut_assertok(check_arg(uts, 13, "john arg=123", buf, " john  arg=123  ",
			      "arg", "123"));
	ut_assertok(check_arg(uts, 18, "john arg=123 mary", buf,
			      " john  arg=123 mary ", "arg", "123"));

	/* unchanged arg */
	ut_assertok(check_arg(uts, 3, "me", buf, "me", "me", BOOTFLOWCL_EMPTY));

	/* arg which starts with the same name */
	ut_assertok(check_arg(uts, 28, "mary=abc johnathon=2 john=3", buf,
			      "mary=abc johnathon=2 john=1", "john", "3"));

	return 0;
}
BOOTSTD_TEST(test_bootflow_cmdline_set, 0);

/* Test of bootflow_cmdline_set_arg() */
static int bootflow_set_arg(struct unit_test_state *uts)
{
	struct bootflow s_bflow, *bflow = &s_bflow;
	ulong mem_start;

	ut_assertok(env_set("bootargs", NULL));

	mem_start = ut_check_delta(0);

	/* Do a simple sanity check. Rely on bootflow_cmdline() for the rest */
	bflow->cmdline = NULL;
	ut_assertok(bootflow_cmdline_set_arg(bflow, "fred", "123", false));
	ut_asserteq_str(bflow->cmdline, "fred=123");

	ut_assertok(bootflow_cmdline_set_arg(bflow, "mary", "and here", false));
	ut_asserteq_str(bflow->cmdline, "fred=123 mary=\"and here\"");

	ut_assertok(bootflow_cmdline_set_arg(bflow, "mary", NULL, false));
	ut_asserteq_str(bflow->cmdline, "fred=123");
	ut_assertok(bootflow_cmdline_set_arg(bflow, "fred", NULL, false));
	ut_asserteq_ptr(bflow->cmdline, NULL);

	ut_asserteq(0, ut_check_delta(mem_start));

	ut_assertok(bootflow_cmdline_set_arg(bflow, "mary", "here", true));
	ut_asserteq_str("mary=here", env_get("bootargs"));
	ut_assertok(env_set("bootargs", NULL));

	return 0;
}
BOOTSTD_TEST(bootflow_set_arg, 0);

/* Test of bootflow_cmdline_get_arg() */
static int bootflow_cmdline_get(struct unit_test_state *uts)
{
	int pos;

	/* empty string */
	ut_asserteq(-ENOENT, cmdline_get_arg("", "fred", &pos));

	/* arg with empty value */
	ut_asserteq(0, cmdline_get_arg("fred= mary", "fred", &pos));
	ut_asserteq(5, pos);

	/* arg with a value */
	ut_asserteq(2, cmdline_get_arg("fred=23", "fred", &pos));
	ut_asserteq(5, pos);

	/* arg with a value */
	ut_asserteq(3, cmdline_get_arg("mary=1 fred=234", "fred", &pos));
	ut_asserteq(12, pos);

	/* arg with a value, after quoted arg */
	ut_asserteq(3, cmdline_get_arg("mary=\"1 2\" fred=234", "fred", &pos));
	ut_asserteq(16, pos);

	/* arg in the middle */
	ut_asserteq(0, cmdline_get_arg("mary=\"1 2\" fred john=23", "fred",
				       &pos));
	ut_asserteq(15, pos);

	/* quoted arg */
	ut_asserteq(3, cmdline_get_arg("mary=\"1 2\" fred=\"3 4\" john=23",
				       "fred", &pos));
	ut_asserteq(17, pos);

	/* args starting with the same prefix */
	ut_asserteq(1, cmdline_get_arg("mary=abc johnathon=3 john=1", "john",
				       &pos));
	ut_asserteq(26, pos);

	return 0;
}
BOOTSTD_TEST(bootflow_cmdline_get, 0);

static int bootflow_cmdline(struct unit_test_state *uts)
{
	ut_assertok(run_command("bootflow scan mmc", 0));
	ut_assertok(run_command("bootflow sel 0", 0));
	console_record_reset_enable();

	ut_asserteq(1, run_command("bootflow cmdline get fred", 0));
	ut_assert_nextline("Argument not found");
	ut_assert_console_end();

	ut_asserteq(0, run_command("bootflow cmdline set fred 123", 0));
	ut_asserteq(0, run_command("bootflow cmdline get fred", 0));
	ut_assert_nextline("123");

	ut_asserteq(0, run_command("bootflow cmdline set mary abc", 0));
	ut_asserteq(0, run_command("bootflow cmdline get mary", 0));
	ut_assert_nextline("abc");

	ut_asserteq(0, run_command("bootflow cmdline delete fred", 0));
	ut_asserteq(1, run_command("bootflow cmdline get fred", 0));
	ut_assert_nextline("Argument not found");

	ut_asserteq(0, run_command("bootflow cmdline clear mary", 0));
	ut_asserteq(0, run_command("bootflow cmdline get mary", 0));
	ut_assert_nextline_empty();

	ut_asserteq(0, run_command("bootflow cmdline set mary abc", 0));
	ut_asserteq(0, run_command("bootflow cmdline set mary", 0));
	ut_assert_nextline_empty();

	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_cmdline, 0);

/* test a few special changes to a long command line */
static int bootflow_cmdline_special(struct unit_test_state *uts)
{
	char buf[500];
	int pos;

	/*
	 * check handling of an argument which has an embedded '=', as well as
	 * handling of a argument which partially matches ("ro" and "root")
	 */
	ut_asserteq(32, cmdline_set_arg(
		buf, sizeof(buf),
		"loglevel=7 root=PARTUUID=d68352e3 rootwait ro noinitrd",
		"root", NULL, &pos));
	ut_asserteq_str("loglevel=7 rootwait ro noinitrd", buf);

	return 0;
}
BOOTSTD_TEST(bootflow_cmdline_special, 0);

/* Test ChromiumOS bootmeth */
static int bootflow_cros(struct unit_test_state *uts)
{
	ut_assertok(scan_mmc_bootdev(uts, "mmc5", true));
	ut_assertok(run_command("bootflow list", 0));

	ut_assert_nextlinen("Showing all");
	ut_assert_nextlinen("Seq");
	ut_assert_nextlinen("---");
	ut_assert_nextlinen("  0  extlinux");
	ut_assert_nextlinen("  1  cros         ready   mmc          2  mmc5.bootdev.part_2       ");
	ut_assert_nextlinen("  2  cros         ready   mmc          4  mmc5.bootdev.part_4       ");
	ut_assert_nextlinen("---");
	ut_assert_skip_to_line("(3 bootflows, 3 valid)");

	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootflow_cros, 0);
