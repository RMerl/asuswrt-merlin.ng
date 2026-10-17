// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for bootdev functions. All start with 'bootdev'
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootstd.h>
#include <dm.h>
#include <bootdev.h>
#include <bootflow.h>
#include <mapmem.h>
#include <os.h>
#include <test/suites.h>
#include <test/ut.h>
#include "bootstd_common.h"

/* Allow reseting the USB-started flag */
#if defined(CONFIG_USB_HOST) || defined(CONFIG_USB_GADGET)
extern bool usb_started;
#else
#include <usb.h>
#endif

/* Check 'bootdev list' command */
static int bootdev_test_cmd_list(struct unit_test_state *uts)
{
	int probed;

	console_record_reset_enable();
	for (probed = 0; probed < 2; probed++) {
		int probe_ch = probed ? '+' : ' ';

		ut_assertok(run_command(probed ? "bootdev list -p" :
			"bootdev list", 0));
		ut_assert_nextline("Seq  Probed  Status  Uclass    Name");
		ut_assert_nextlinen("---");
		ut_assert_nextline("%3x   [ %c ]  %6s  %-8s  %s", 0, probe_ch, "OK",
				   "mmc", "mmc2.bootdev");
		ut_assert_nextline("%3x   [ %c ]  %6s  %-8s  %s", 1, probe_ch, "OK",
				   "mmc", "mmc1.bootdev");
		ut_assert_nextline("%3x   [ %c ]  %6s  %-8s  %s", 2, probe_ch, "OK",
				   "mmc", "mmc0.bootdev");
		ut_assert_nextlinen("---");
		ut_assert_nextline("(3 bootdevs)");
		ut_assert_console_end();
	}

	return 0;
}
BOOTSTD_TEST(bootdev_test_cmd_list, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootdev select' and 'info' commands */
static int bootdev_test_cmd_select(struct unit_test_state *uts)
{
	struct bootstd_priv *std;

	/* get access to the CLI's cur_bootdev */
	ut_assertok(bootstd_get_priv(&std));

	console_record_reset_enable();
	ut_asserteq(1, run_command("bootdev info", 0));
	ut_assert_nextlinen("Please use");
	ut_assert_console_end();

	/* select by sequence */
	ut_assertok(run_command("bootdev select 0", 0));
	ut_assert_console_end();

	ut_assertok(run_command("bootdev info", 0));
	ut_assert_nextline("Name:      mmc2.bootdev");
	ut_assert_nextline("Sequence:  0");
	ut_assert_nextline("Status:    Probed");
	ut_assert_nextline("Uclass:    mmc");
	ut_assert_nextline("Bootflows: 0 (0 valid)");
	ut_assert_console_end();

	/* select by bootdev name */
	ut_assertok(run_command("bootdev select mmc1.bootdev", 0));
	ut_assert_console_end();
	ut_assertnonnull(std->cur_bootdev);
	ut_asserteq_str("mmc1.bootdev", std->cur_bootdev->name);

	/* select by bootdev label*/
	ut_assertok(run_command("bootdev select mmc1", 0));
	ut_assert_console_end();
	ut_assertnonnull(std->cur_bootdev);
	ut_asserteq_str("mmc1.bootdev", std->cur_bootdev->name);

	/* deselect */
	ut_assertok(run_command("bootdev select", 0));
	ut_assert_console_end();
	ut_assertnull(std->cur_bootdev);

	ut_asserteq(1, run_command("bootdev info", 0));
	ut_assert_nextlinen("Please use");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootdev_test_cmd_select, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check bootdev labels */
static int bootdev_test_labels(struct unit_test_state *uts)
{
	struct udevice *dev, *media;
	int mflags = 0;

	ut_assertok(bootdev_find_by_label("mmc2", &dev, &mflags));
	ut_asserteq(UCLASS_BOOTDEV, device_get_uclass_id(dev));
	ut_asserteq(0, mflags);
	media = dev_get_parent(dev);
	ut_asserteq(UCLASS_MMC, device_get_uclass_id(media));
	ut_asserteq_str("mmc2", media->name);

	/* Check method flags */
	ut_assertok(bootdev_find_by_label("pxe", &dev, &mflags));
	ut_asserteq(BOOTFLOW_METHF_SINGLE_UCLASS | BOOTFLOW_METHF_PXE_ONLY,
		    mflags);
	ut_assertok(bootdev_find_by_label("dhcp", &dev, &mflags));
	ut_asserteq(BOOTFLOW_METHF_SINGLE_UCLASS | BOOTFLOW_METHF_DHCP_ONLY,
		    mflags);

	/* Check invalid uclass */
	ut_asserteq(-EPFNOSUPPORT,
		    bootdev_find_by_label("fred0", &dev, &mflags));

	/* Check unknown sequence number */
	ut_asserteq(-ENOENT, bootdev_find_by_label("mmc6", &dev, &mflags));

	return 0;
}
BOOTSTD_TEST(bootdev_test_labels, UT_TESTF_DM | UT_TESTF_SCAN_FDT |
	     UT_TESTF_ETH_BOOTDEV);

/* Check bootdev_find_by_any() */
static int bootdev_test_any(struct unit_test_state *uts)
{
	struct udevice *dev, *media;
	int mflags;

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
	console_record_reset_enable();
	ut_assertok(bootdev_find_by_any("8", &dev, &mflags));
	ut_asserteq(UCLASS_BOOTDEV, device_get_uclass_id(dev));
	ut_asserteq(BOOTFLOW_METHF_SINGLE_DEV, mflags);
	media = dev_get_parent(dev);
	ut_asserteq(UCLASS_MMC, device_get_uclass_id(media));
	ut_asserteq_str("mmc2", media->name);
	ut_assert_console_end();

	/* there should not be this many bootdevs */
	ut_asserteq(-ENODEV, bootdev_find_by_any("50", &dev, &mflags));
	ut_assert_nextline("Cannot find '50' (err=-19)");
	ut_assert_console_end();

	/* Check method flags */
	ut_assertok(bootdev_find_by_any("pxe", &dev, &mflags));
	ut_asserteq(BOOTFLOW_METHF_SINGLE_UCLASS | BOOTFLOW_METHF_PXE_ONLY,
		    mflags);

	/* Check invalid uclass */
	mflags = 123;
	ut_asserteq(-EPFNOSUPPORT, bootdev_find_by_any("fred0", &dev, &mflags));
	ut_assert_nextline("Cannot find bootdev 'fred0' (err=-96)");
	ut_asserteq(123, mflags);
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootdev_test_any, UT_TESTF_DM | UT_TESTF_SCAN_FDT |
	     UT_TESTF_ETH_BOOTDEV);

/*
 * Check bootdev ordering with the bootdev-order property and boot_targets
 * environment variable
 */
static int bootdev_test_order(struct unit_test_state *uts)
{
	struct bootflow_iter iter;
	struct bootflow bflow;

	test_set_skip_delays(true);

	/* Start up USB which gives us three additional bootdevs */
	usb_started = false;
	ut_assertok(run_command("usb start", 0));

	/*
	 * First try the order set by the bootdev-order property
	 * Like all sandbox unit tests this relies on the devicetree setting up
	 * the required devices:
	 *
	 * mmc0 - nothing connected
	 * mmc1 - connected to mmc1.img file
	 * mmc2 - nothing connected
	 */
	ut_assertok(env_set("boot_targets", NULL));
	ut_assertok(bootflow_scan_first(NULL, NULL, &iter, 0, &bflow));
	ut_asserteq(2, iter.num_devs);
	ut_asserteq_str("mmc2.bootdev", iter.dev_used[0]->name);
	ut_asserteq_str("mmc1.bootdev", iter.dev_used[1]->name);
	bootflow_iter_uninit(&iter);

	/* Use the environment variable to override it */
	ut_assertok(env_set("boot_targets", "mmc1 mmc2 usb"));
	ut_assertok(bootflow_scan_first(NULL, NULL, &iter, 0, &bflow));
	ut_asserteq(-ENODEV, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(5, iter.num_devs);
	ut_asserteq_str("mmc1.bootdev", iter.dev_used[0]->name);
	ut_asserteq_str("mmc2.bootdev", iter.dev_used[1]->name);
	ut_asserteq_str("usb_mass_storage.lun0.bootdev",
			iter.dev_used[2]->name);
	bootflow_iter_uninit(&iter);

	/* Try a single uclass */
	ut_assertok(env_set("boot_targets", NULL));
	ut_assertok(bootflow_scan_first(NULL, "mmc", &iter, 0, &bflow));
	ut_asserteq(2, iter.num_devs);

	/* Now scan past mmc1 and make sure that only mmc0 shows up */
	ut_asserteq(-ENODEV, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(3, iter.num_devs);
	ut_asserteq_str("mmc2.bootdev", iter.dev_used[0]->name);
	ut_asserteq_str("mmc1.bootdev", iter.dev_used[1]->name);
	ut_asserteq_str("mmc0.bootdev", iter.dev_used[2]->name);
	bootflow_iter_uninit(&iter);

	/* Try a single uclass with boot_targets */
	ut_assertok(env_set("boot_targets", "mmc"));
	ut_assertok(bootflow_scan_first(NULL, NULL, &iter, 0, &bflow));
	ut_asserteq(2, iter.num_devs);

	/* Now scan past mmc1 and make sure that only mmc0 shows up */
	ut_asserteq(-ENODEV, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(3, iter.num_devs);
	ut_asserteq_str("mmc2.bootdev", iter.dev_used[0]->name);
	ut_asserteq_str("mmc1.bootdev", iter.dev_used[1]->name);
	ut_asserteq_str("mmc0.bootdev", iter.dev_used[2]->name);
	bootflow_iter_uninit(&iter);

	/* Try a single uclass with boot_targets */
	ut_assertok(env_set("boot_targets", "mmc usb"));
	ut_assertok(bootflow_scan_first(NULL, NULL, &iter, 0, &bflow));
	ut_asserteq(2, iter.num_devs);

	/* Now scan past mmc1 and make sure that the 3 USB devices show up */
	ut_asserteq(-ENODEV, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(6, iter.num_devs);
	ut_asserteq_str("mmc2.bootdev", iter.dev_used[0]->name);
	ut_asserteq_str("mmc1.bootdev", iter.dev_used[1]->name);
	ut_asserteq_str("mmc0.bootdev", iter.dev_used[2]->name);
	ut_asserteq_str("usb_mass_storage.lun0.bootdev",
			iter.dev_used[3]->name);
	bootflow_iter_uninit(&iter);

	return 0;
}
BOOTSTD_TEST(bootdev_test_order, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check default bootdev ordering  */
static int bootdev_test_order_default(struct unit_test_state *uts)
{
	struct bootflow_iter iter;
	struct bootflow bflow;

	/*
	 * Now drop both orderings, to check the default (prioriy/sequence)
	 * ordering
	 */
	ut_assertok(env_set("boot_targets", NULL));
	ut_assertok(bootstd_test_drop_bootdev_order(uts));

	ut_assertok(bootflow_scan_first(NULL, NULL, &iter, 0, &bflow));
	ut_asserteq(2, iter.num_devs);
	ut_asserteq_str("mmc2.bootdev", iter.dev_used[0]->name);
	ut_asserteq_str("mmc1.bootdev", iter.dev_used[1]->name);

	ut_asserteq(-ENODEV, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(3, iter.num_devs);
	ut_asserteq_str("mmc0.bootdev", iter.dev_used[2]->name);
	bootflow_iter_uninit(&iter);

	return 0;
}
BOOTSTD_TEST(bootdev_test_order_default, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check bootdev ordering with the uclass priority */
static int bootdev_test_prio(struct unit_test_state *uts)
{
	struct bootdev_uc_plat *ucp;
	struct bootflow_iter iter;
	struct bootflow bflow;
	struct udevice *blk;

	test_set_skip_delays(true);

	/* disable ethernet since the hunter will run dhcp */
	test_set_eth_enable(false);

	/* Start up USB which gives us three additional bootdevs */
	usb_started = false;
	ut_assertok(run_command("usb start", 0));

	ut_assertok(bootstd_test_drop_bootdev_order(uts));

	/* 3 MMC and 3 USB bootdevs: MMC should come before USB */
	console_record_reset_enable();
	ut_assertok(bootflow_scan_first(NULL, NULL, &iter, 0, &bflow));
	ut_asserteq(-ENODEV, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(6, iter.num_devs);
	ut_asserteq_str("mmc2.bootdev", iter.dev_used[0]->name);
	ut_asserteq_str("usb_mass_storage.lun0.bootdev",
			iter.dev_used[3]->name);

	ut_assertok(bootdev_get_sibling_blk(iter.dev_used[3], &blk));
	ut_asserteq_str("usb_mass_storage.lun0", blk->name);

	/* adjust the priority of the first USB bootdev to the highest */
	ucp = dev_get_uclass_plat(iter.dev_used[3]);
	ucp->prio = BOOTDEVP_1_PRE_SCAN;

	/* try again but enable hunting, which brings in SCSI */
	bootflow_iter_uninit(&iter);
	ut_assertok(bootflow_scan_first(NULL, NULL, &iter, BOOTFLOWIF_HUNT,
					&bflow));
	ut_asserteq(-ENODEV, bootflow_scan_next(&iter, &bflow));
	ut_asserteq(7, iter.num_devs);
	ut_asserteq_str("usb_mass_storage.lun0.bootdev",
			iter.dev_used[0]->name);
	ut_asserteq_str("mmc2.bootdev", iter.dev_used[1]->name);

	return 0;
}
BOOTSTD_TEST(bootdev_test_prio, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check listing hunters */
static int bootdev_test_hunter(struct unit_test_state *uts)
{
	struct bootstd_priv *std;

	usb_started = false;
	test_set_skip_delays(true);

	/* get access to the used hunters */
	ut_assertok(bootstd_get_priv(&std));

	console_record_reset_enable();
	bootdev_list_hunters(std);
	ut_assert_nextline("Prio  Used  Uclass           Hunter");
	ut_assert_nextlinen("----");
	ut_assert_nextline("   6        ethernet         eth_bootdev");
	ut_assert_nextline("   1        simple_bus       (none)");
	ut_assert_nextline("   5        ide              ide_bootdev");
	ut_assert_nextline("   2        mmc              mmc_bootdev");
	ut_assert_nextline("   4        nvme             nvme_bootdev");
	ut_assert_nextline("   4        qfw              qfw_bootdev");
	ut_assert_nextline("   4        scsi             scsi_bootdev");
	ut_assert_nextline("   4        spi_flash        sf_bootdev");
	ut_assert_nextline("   5        usb              usb_bootdev");
	ut_assert_nextline("   4        virtio           virtio_bootdev");
	ut_assert_nextline("(total hunters: 10)");
	ut_assert_console_end();

	ut_assertok(bootdev_hunt("usb1", false));
	ut_assert_nextline(
		"Bus usb@1: scanning bus usb@1 for devices... 5 USB Device(s) found");
	ut_assert_console_end();

	/* USB is 7th in the list, so bit 8 */
	ut_asserteq(BIT(8), std->hunters_used);

	return 0;
}
BOOTSTD_TEST(bootdev_test_hunter, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootdev hunt' command */
static int bootdev_test_cmd_hunt(struct unit_test_state *uts)
{
	struct bootstd_priv *std;

	test_set_skip_delays(true);
	usb_started = false;

	/* get access to the used hunters */
	ut_assertok(bootstd_get_priv(&std));

	console_record_reset_enable();
	ut_assertok(run_command("bootdev hunt -l", 0));
	ut_assert_nextline("Prio  Used  Uclass           Hunter");
	ut_assert_nextlinen("----");
	ut_assert_nextline("   6        ethernet         eth_bootdev");
	ut_assert_skip_to_line("(total hunters: 10)");
	ut_assert_console_end();

	/* Use the MMC hunter and see that it updates */
	ut_assertok(run_command("bootdev hunt mmc", 0));
	ut_assertok(run_command("bootdev hunt -l", 0));
	ut_assert_skip_to_line("   5        ide              ide_bootdev");
	ut_assert_nextline("   2     *  mmc              mmc_bootdev");
	ut_assert_skip_to_line("(total hunters: 10)");
	ut_assert_console_end();

	/* Scan all hunters */
	test_set_eth_enable(false);
	test_set_skip_delays(true);
	ut_assertok(run_command("bootdev hunt", 0));
	ut_assert_nextline("Hunting with: ethernet");

	/* This is the extension feature which has no uclass at present */
	ut_assert_nextline("Hunting with: simple_bus");
	ut_assert_nextline("Found 2 extension board(s).");
	ut_assert_nextline("Hunting with: ide");

	/* mmc hunter has already been used so should not run again */

	ut_assert_nextline("Hunting with: nvme");
	ut_assert_nextline("Hunting with: qfw");
	ut_assert_nextline("Hunting with: scsi");
	ut_assert_nextline("scanning bus for devices...");
	ut_assert_skip_to_line("Hunting with: spi_flash");
	ut_assert_nextline("Hunting with: usb");
	ut_assert_nextline(
		"Bus usb@1: scanning bus usb@1 for devices... 5 USB Device(s) found");
	ut_assert_nextline("Hunting with: virtio");
	ut_assert_console_end();

	/* List available hunters */
	ut_assertok(run_command("bootdev hunt -l", 0));
	ut_assert_nextlinen("Prio");
	ut_assert_nextlinen("----");
	ut_assert_nextline("   6     *  ethernet         eth_bootdev");
	ut_assert_nextline("   1     *  simple_bus       (none)");
	ut_assert_nextline("   5     *  ide              ide_bootdev");
	ut_assert_nextline("   2     *  mmc              mmc_bootdev");
	ut_assert_nextline("   4     *  nvme             nvme_bootdev");
	ut_assert_nextline("   4     *  qfw              qfw_bootdev");
	ut_assert_nextline("   4     *  scsi             scsi_bootdev");
	ut_assert_nextline("   4     *  spi_flash        sf_bootdev");
	ut_assert_nextline("   5     *  usb              usb_bootdev");
	ut_assert_nextline("   4     *  virtio           virtio_bootdev");
	ut_assert_nextline("(total hunters: 10)");
	ut_assert_console_end();

	ut_asserteq(GENMASK(MAX_HUNTER, 0), std->hunters_used);

	return 0;
}
BOOTSTD_TEST(bootdev_test_cmd_hunt, UT_TESTF_DM | UT_TESTF_SCAN_FDT |
	     UT_TESTF_ETH_BOOTDEV);

/* Check searching for bootdevs using the hunters */
static int bootdev_test_hunt_scan(struct unit_test_state *uts)
{
	struct bootflow_iter iter;
	struct bootstd_priv *std;
	struct bootflow bflow;

	/* get access to the used hunters */
	ut_assertok(bootstd_get_priv(&std));

	ut_assertok(bootstd_test_drop_bootdev_order(uts));
	ut_assertok(bootflow_scan_first(NULL, NULL, &iter,
					BOOTFLOWIF_SHOW | BOOTFLOWIF_HUNT |
					BOOTFLOWIF_SKIP_GLOBAL, &bflow));
	ut_asserteq(BIT(MMC_HUNTER) | BIT(1), std->hunters_used);

	return 0;
}
BOOTSTD_TEST(bootdev_test_hunt_scan, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check that only bootable partitions are processed */
static int bootdev_test_bootable(struct unit_test_state *uts)
{
	struct bootflow_iter iter;
	struct bootflow bflow;
	struct udevice *blk;

	memset(&iter, '\0', sizeof(iter));
	memset(&bflow, '\0', sizeof(bflow));
	iter.part = 0;
	ut_assertok(uclass_get_device_by_name(UCLASS_BLK, "mmc1.blk", &blk));
	iter.dev = blk;
	ut_assertok(device_find_next_child(&iter.dev));
	uclass_first_device(UCLASS_BOOTMETH, &bflow.method);

	/*
	 * initially we don't have any knowledge of which partitions are
	 * bootable, but mmc1 has two partitions, with the first one being
	 * bootable
	 */
	iter.part = 2;
	ut_asserteq(-EINVAL, bootdev_find_in_blk(iter.dev, blk, &iter, &bflow));
	ut_asserteq(0, iter.first_bootable);

	/* scan with part == 0 to get the partition info */
	iter.part = 0;
	ut_asserteq(-ENOENT, bootdev_find_in_blk(iter.dev, blk, &iter, &bflow));
	ut_asserteq(1, iter.first_bootable);

	/* now it will refuse to use non-bootable partitions */
	iter.part = 2;
	ut_asserteq(-EINVAL, bootdev_find_in_blk(iter.dev, blk, &iter, &bflow));

	return 0;
}
BOOTSTD_TEST(bootdev_test_bootable, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check hunting for bootdev of a particular priority */
static int bootdev_test_hunt_prio(struct unit_test_state *uts)
{
	usb_started = false;
	test_set_skip_delays(true);

	console_record_reset_enable();
	ut_assertok(bootdev_hunt_prio(BOOTDEVP_4_SCAN_FAST, false));
	ut_assert_nextline("scanning bus for devices...");
	ut_assert_skip_to_line("            Type: Hard Disk");
	ut_assert_nextlinen("            Capacity:");
	ut_assert_console_end();

	/* now try a different priority, verbosely */
	ut_assertok(bootdev_hunt_prio(BOOTDEVP_5_SCAN_SLOW, true));
	ut_assert_nextline("Hunting with: ide");
	ut_assert_nextline("Hunting with: usb");
	ut_assert_nextline(
		"Bus usb@1: scanning bus usb@1 for devices... 5 USB Device(s) found");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootdev_test_hunt_prio, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check hunting for bootdevs with a particular label */
static int bootdev_test_hunt_label(struct unit_test_state *uts)
{
	struct udevice *dev, *old;
	struct bootstd_priv *std;
	int mflags;

	usb_started = false;

	/* get access to the used hunters */
	ut_assertok(bootstd_get_priv(&std));

	/* scan an unknown uclass */
	console_record_reset_enable();
	old = (void *)&mflags;   /* arbitrary pointer to check against dev */
	dev = old;
	mflags = 123;
	ut_asserteq(-EPFNOSUPPORT,
		    bootdev_hunt_and_find_by_label("fred", &dev, &mflags));
	ut_asserteq_ptr(old, dev);
	ut_asserteq(123, mflags);
	ut_assert_console_end();
	ut_asserteq(0, std->hunters_used);

	/* scan an invalid mmc controllers */
	ut_asserteq(-ENOENT,
		    bootdev_hunt_and_find_by_label("mmc4", &dev, &mflags));
	ut_asserteq_ptr(old, dev);
	ut_asserteq(123, mflags);
	ut_assert_console_end();

	ut_assertok(bootstd_test_check_mmc_hunter(uts));

	/* scan for a particular mmc controller */
	ut_assertok(bootdev_hunt_and_find_by_label("mmc1", &dev, &mflags));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc1.bootdev", dev->name);
	ut_asserteq(0, mflags);
	ut_assert_console_end();

	/* scan all of usb */
	test_set_skip_delays(true);
	ut_assertok(bootdev_hunt_and_find_by_label("usb", &dev, &mflags));
	ut_assertnonnull(dev);
	ut_asserteq_str("usb_mass_storage.lun0.bootdev", dev->name);
	ut_asserteq(BOOTFLOW_METHF_SINGLE_UCLASS, mflags);
	ut_assert_nextlinen("Bus usb@1: scanning bus usb@1");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootdev_test_hunt_label, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check iterating to the next label in a list */
static int bootdev_test_next_label(struct unit_test_state *uts)
{
	const char *const labels[] = {"mmc0", "scsi", "dhcp", "pxe", NULL};
	struct bootflow_iter iter;
	struct bootstd_priv *std;
	struct bootflow bflow;
	struct udevice *dev;
	int mflags;

	test_set_eth_enable(false);

	/* get access to the used hunters */
	ut_assertok(bootstd_get_priv(&std));

	memset(&iter, '\0', sizeof(iter));
	memset(&bflow, '\0', sizeof(bflow));
	iter.part = 0;
	uclass_first_device(UCLASS_BOOTMETH, &bflow.method);
	iter.cur_label = -1;
	iter.labels = labels;

	dev = NULL;
	mflags = 123;
	ut_assertok(bootdev_next_label(&iter, &dev, &mflags));
	console_record_reset_enable();
	ut_assert_console_end();
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc0.bootdev", dev->name);
	ut_asserteq(0, mflags);

	ut_assertok(bootstd_test_check_mmc_hunter(uts));

	ut_assertok(bootdev_next_label(&iter, &dev, &mflags));
	ut_assert_nextline("scanning bus for devices...");
	ut_assert_skip_to_line(
		"            Capacity: 1.9 MB = 0.0 GB (4095 x 512)");
	ut_assert_console_end();
	ut_assertnonnull(dev);
	ut_asserteq_str("scsi.id0lun0.bootdev", dev->name);
	ut_asserteq(BOOTFLOW_METHF_SINGLE_UCLASS, mflags);

	/* SCSI is 7th in the list, so bit 6 */
	ut_asserteq(BIT(MMC_HUNTER) | BIT(6), std->hunters_used);

	ut_assertok(bootdev_next_label(&iter, &dev, &mflags));
	ut_assert_console_end();
	ut_assertnonnull(dev);
	ut_asserteq_str("eth@10002000.bootdev", dev->name);
	ut_asserteq(BOOTFLOW_METHF_SINGLE_UCLASS | BOOTFLOW_METHF_DHCP_ONLY,
		    mflags);

	/* dhcp: Ethernet is first so bit 0 */
	ut_asserteq(BIT(MMC_HUNTER) | BIT(6) | BIT(0), std->hunters_used);

	ut_assertok(bootdev_next_label(&iter, &dev, &mflags));
	ut_assert_console_end();
	ut_assertnonnull(dev);
	ut_asserteq_str("eth@10002000.bootdev", dev->name);
	ut_asserteq(BOOTFLOW_METHF_SINGLE_UCLASS | BOOTFLOW_METHF_PXE_ONLY,
		    mflags);

	/* pxe: Ethernet is first so bit 0 */
	ut_asserteq(BIT(MMC_HUNTER) | BIT(6) | BIT(0), std->hunters_used);

	mflags = 123;
	ut_asserteq(-ENODEV, bootdev_next_label(&iter, &dev, &mflags));
	ut_asserteq(123, mflags);
	ut_assert_console_end();

	/* no change */
	ut_asserteq(BIT(MMC_HUNTER) | BIT(6) | BIT(0), std->hunters_used);

	return 0;
}
BOOTSTD_TEST(bootdev_test_next_label, UT_TESTF_DM | UT_TESTF_SCAN_FDT |
	     UT_TESTF_ETH_BOOTDEV | UT_TESTF_SF_BOOTDEV);


/* Check iterating to the next prioirty in a list */
static int bootdev_test_next_prio(struct unit_test_state *uts)
{
	struct bootflow_iter iter;
	struct bootstd_priv *std;
	struct bootflow bflow;
	struct udevice *dev;
	int ret;

	test_set_eth_enable(false);
	test_set_skip_delays(true);

	/* get access to the used hunters */
	ut_assertok(bootstd_get_priv(&std));

	memset(&iter, '\0', sizeof(iter));
	memset(&bflow, '\0', sizeof(bflow));
	iter.part = 0;
	uclass_first_device(UCLASS_BOOTMETH, &bflow.method);
	iter.cur_prio = 0;
	iter.flags = BOOTFLOWIF_SHOW;

	dev = NULL;
	console_record_reset_enable();
	ut_assertok(bootdev_next_prio(&iter, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc2.bootdev", dev->name);

	/* hunt flag not set, so this should not use any hunters */
	ut_asserteq(0, std->hunters_used);
	ut_assert_console_end();

	/* now try again with hunting enabled */
	iter.flags = BOOTFLOWIF_SHOW | BOOTFLOWIF_HUNT;
	iter.cur_prio = 0;
	iter.part = 0;

	ut_assertok(bootdev_next_prio(&iter, &dev));
	ut_asserteq_str("mmc2.bootdev", dev->name);
	ut_assert_nextline("Hunting with: simple_bus");
	ut_assert_nextline("Found 2 extension board(s).");
	ut_assert_nextline("Hunting with: mmc");
	ut_assert_console_end();

	ut_asserteq(BIT(MMC_HUNTER) | BIT(1), std->hunters_used);

	ut_assertok(bootdev_next_prio(&iter, &dev));
	ut_asserteq_str("mmc1.bootdev", dev->name);

	ut_assertok(bootdev_next_prio(&iter, &dev));
	ut_asserteq_str("mmc0.bootdev", dev->name);
	ut_assert_console_end();

	ut_assertok(bootdev_next_prio(&iter, &dev));
	ut_asserteq_str("spi.bin@0.bootdev", dev->name);
	ut_assert_skip_to_line("Hunting with: spi_flash");

	/*
	 * this scans all bootdevs of priority BOOTDEVP_4_SCAN_FAST before it
	 * starts looking at the devices, so we se virtio as well
	 */
	ut_assert_nextline("Hunting with: virtio");
	ut_assert_nextlinen("SF: Detected m25p16");

	ut_assertok(bootdev_next_prio(&iter, &dev));
	ut_asserteq_str("spi.bin@1.bootdev", dev->name);
	ut_assert_nextlinen("SF: Detected m25p16");
	ut_assert_console_end();

	/* keep going until there are no more bootdevs */
	do {
		ret = bootdev_next_prio(&iter, &dev);
	} while (!ret);
	ut_asserteq(-ENODEV, ret);
	ut_assertnull(dev);
	ut_asserteq(GENMASK(MAX_HUNTER, 0), std->hunters_used);

	ut_assert_skip_to_line("Hunting with: ethernet");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootdev_test_next_prio, UT_TESTF_DM | UT_TESTF_SCAN_FDT |
	     UT_TESTF_SF_BOOTDEV);
