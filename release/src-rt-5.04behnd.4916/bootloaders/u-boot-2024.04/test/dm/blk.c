// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <part.h>
#include <sandbox_host.h>
#include <usb.h>
#include <asm/global_data.h>
#include <asm/state.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Test that block devices can be created */
static int dm_test_blk_base(struct unit_test_state *uts)
{
	struct udevice *blk0, *blk1, *dev0, *dev1, *dev, *chk0, *chk1;

	/* Create two, one the parent of the other */
	ut_assertok(host_create_device("test0", false, DEFAULT_BLKSZ, &dev0));
	ut_assertok(host_create_device("test1", false, DEFAULT_BLKSZ, &dev1));

	/* Check we can find them */
	ut_assertok(blk_get_device(UCLASS_HOST, 0, &blk0));
	ut_assertok(blk_get_from_parent(dev0, &chk0));
	ut_asserteq_ptr(blk0, chk0);

	ut_assertok(blk_get_device(UCLASS_HOST, 1, &blk1));
	ut_assertok(blk_get_from_parent(dev1, &chk1));
	ut_asserteq_ptr(blk1, chk1);
	ut_asserteq(-ENODEV, blk_get_device(UCLASS_HOST, 2, &dev0));

	/* Check we can iterate */
	ut_assertok(blk_first_device(UCLASS_HOST, &dev));
	ut_asserteq_ptr(blk0, dev);
	ut_assertok(blk_next_device(&dev));
	ut_asserteq_ptr(blk1, dev);

	return 0;
}
DM_TEST(dm_test_blk_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int count_blk_devices(void)
{
	struct udevice *blk;
	struct uclass *uc;
	int count = 0;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(blk, uc)
		count++;

	return count;
}

/* Test that block devices work correctly with USB */
static int dm_test_blk_usb(struct unit_test_state *uts)
{
	struct udevice *usb_dev, *dev;
	struct blk_desc *dev_desc;

	usb_started = false;

	/* Get a flash device */
	state_set_skip_delays(true);
	ut_assertok(usb_stop());
	ut_assertok(usb_init());
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 0, &usb_dev));
	ut_assertok(blk_get_device_by_str("usb", "0", &dev_desc));

	/* The parent should be a block device */
	ut_assertok(blk_get_device(UCLASS_USB, 0, &dev));
	ut_asserteq_ptr(usb_dev, dev_get_parent(dev));

	/* Check we have one block device for each mass storage device */
	ut_asserteq(6, count_blk_devices());

	/* Now go around again, making sure the old devices were unbound */
	ut_assertok(usb_stop());
	ut_assertok(usb_init());
	ut_asserteq(6, count_blk_devices());
	ut_assertok(usb_stop());

	return 0;
}
DM_TEST(dm_test_blk_usb, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that we can find block devices without probing them */
static int dm_test_blk_find(struct unit_test_state *uts)
{
	struct udevice *blk, *chk, *dev;

	ut_assertok(host_create_device("test0", false, DEFAULT_BLKSZ, &dev));

	ut_assertok(blk_find_device(UCLASS_HOST, 0, &chk));
	ut_assertok(device_find_first_child_by_uclass(dev, UCLASS_BLK, &blk));
	ut_asserteq_ptr(chk, blk);
	ut_asserteq(false, device_active(dev));
	ut_asserteq(-ENODEV, blk_find_device(UCLASS_HOST, 1, &dev));

	/* Now activate it */
	ut_assertok(blk_get_device(UCLASS_HOST, 0, &blk));
	ut_asserteq_ptr(chk, blk);
	ut_asserteq(true, device_active(blk));

	return 0;
}
DM_TEST(dm_test_blk_find, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that block device numbering works as expected */
static int dm_test_blk_devnum(struct unit_test_state *uts)
{
	struct udevice *dev, *mmc_dev, *parent;
	int i;

	/*
	 * Probe the devices, with the first one being probed last. This is the
	 * one with no alias / sequence number.
	 */
	ut_assertok(uclass_get_device(UCLASS_MMC, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_MMC, 2, &dev));
	ut_assertok(uclass_get_device(UCLASS_MMC, 0, &dev));
	for (i = 0; i < 3; i++) {
		struct blk_desc *desc;

		/* Check that the bblock device is attached */
		ut_assertok(uclass_get_device_by_seq(UCLASS_MMC, i, &mmc_dev));
		ut_assertok(blk_find_device(UCLASS_MMC, i, &dev));
		parent = dev_get_parent(dev);
		ut_asserteq_ptr(parent, mmc_dev);
		ut_asserteq(trailing_strtol(mmc_dev->name), i);

		/*
		 * Check that the block device devnum matches its parent's
		 * sequence number
		 */
		desc = dev_get_uclass_plat(dev);
		ut_asserteq(desc->devnum, i);
	}

	return 0;
}
DM_TEST(dm_test_blk_devnum, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that we can get a block from its parent */
static int dm_test_blk_get_from_parent(struct unit_test_state *uts)
{
	struct udevice *dev, *blk;

	ut_assertok(uclass_get_device(UCLASS_MMC, 0, &dev));
	ut_assertok(blk_get_from_parent(dev, &blk));

	ut_assertok(uclass_get_device(UCLASS_I2C, 0, &dev));
	ut_asserteq(-ENODEV, blk_get_from_parent(dev, &blk));

	ut_assertok(uclass_get_device(UCLASS_GPIO, 0, &dev));
	ut_asserteq(-ENODEV, blk_get_from_parent(dev, &blk));

	return 0;
}
DM_TEST(dm_test_blk_get_from_parent, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test iteration through block devices */
static int dm_test_blk_iter(struct unit_test_state *uts)
{
	struct udevice *dev;
	int i;

	/*
	 * See sandbox test.dts - it has:
	 *
	 *   mmc0 - removable
	 *   mmc1 - removable
	 *   mmc2 - fixed
	 */
	ut_assertok(blk_first_device_err(BLKF_FIXED, &dev));
	ut_asserteq_str("mmc2.blk", dev->name);
	ut_asserteq(-ENODEV, blk_next_device_err(BLKF_FIXED, &dev));

	ut_assertok(blk_first_device_err(BLKF_REMOVABLE, &dev));
	ut_asserteq_str("mmc1.blk", dev->name);
	ut_assertok(blk_next_device_err(BLKF_REMOVABLE, &dev));
	ut_asserteq_str("mmc0.blk", dev->name);
	ut_asserteq(-ENODEV, blk_next_device_err(BLKF_REMOVABLE, &dev));

	ut_assertok(blk_first_device_err(BLKF_BOTH, &dev));
	ut_asserteq_str("mmc2.blk", dev->name);
	ut_assertok(blk_next_device_err(BLKF_BOTH, &dev));
	ut_asserteq_str("mmc1.blk", dev->name);
	ut_assertok(blk_next_device_err(BLKF_BOTH, &dev));
	ut_asserteq_str("mmc0.blk", dev->name);
	ut_asserteq(-ENODEV, blk_next_device_err(BLKF_FIXED, &dev));

	ut_asserteq(1, blk_count_devices(BLKF_FIXED));
	ut_asserteq(2, blk_count_devices(BLKF_REMOVABLE));
	ut_asserteq(3, blk_count_devices(BLKF_BOTH));

	i = 0;
	blk_foreach_probe(BLKF_FIXED, dev)
		ut_asserteq_str((i++, "mmc2.blk"), dev->name);
	ut_asserteq(1, i);

	i = 0;
	blk_foreach_probe(BLKF_REMOVABLE, dev)
		ut_asserteq_str(i++ ? "mmc0.blk" : "mmc1.blk", dev->name);
	ut_asserteq(2, i);

	i = 0;
	blk_foreach_probe(BLKF_BOTH, dev)
		ut_asserteq_str((++i == 1 ? "mmc2.blk" : i == 2 ?
			"mmc1.blk" : "mmc0.blk"), dev->name);
	ut_asserteq(3, i);

	return 0;
}
DM_TEST(dm_test_blk_iter, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test finding fixed/removable block devices */
static int dm_test_blk_flags(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* Iterate through devices without probing them */
	ut_assertok(blk_find_first(BLKF_BOTH, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc2.blk", dev->name);

	ut_assertok(blk_find_next(BLKF_BOTH, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc1.blk", dev->name);

	ut_assertok(blk_find_next(BLKF_BOTH, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc0.blk", dev->name);

	ut_asserteq(-ENODEV, blk_find_next(BLKF_BOTH, &dev));
	ut_assertnull(dev);

	/* All devices are removable until probed */
	ut_asserteq(-ENODEV, blk_find_first(BLKF_FIXED, &dev));

	ut_assertok(blk_find_first(BLKF_REMOVABLE, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc2.blk", dev->name);

	/* Now probe them and iterate again */
	ut_assertok(blk_first_device_err(BLKF_BOTH, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc2.blk", dev->name);

	ut_assertok(blk_next_device_err(BLKF_BOTH, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc1.blk", dev->name);

	ut_assertok(blk_next_device_err(BLKF_BOTH, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc0.blk", dev->name);

	ut_asserteq(-ENODEV, blk_next_device_err(BLKF_BOTH, &dev));

	/* Look only for fixed devices */
	ut_assertok(blk_first_device_err(BLKF_FIXED, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc2.blk", dev->name);

	ut_asserteq(-ENODEV, blk_next_device_err(BLKF_FIXED, &dev));

	/* Look only for removable devices */
	ut_assertok(blk_first_device_err(BLKF_REMOVABLE, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc1.blk", dev->name);

	ut_assertok(blk_next_device_err(BLKF_REMOVABLE, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("mmc0.blk", dev->name);

	ut_asserteq(-ENODEV, blk_next_device_err(BLKF_REMOVABLE, &dev));

	return 0;
}
DM_TEST(dm_test_blk_flags, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test blk_foreach() and friend */
static int dm_test_blk_foreach(struct unit_test_state *uts)
{
	struct udevice *dev;
	int found;

	/* Test blk_foreach() - use the 3rd bytes of the name (0/1/2) */
	found = 0;
	blk_foreach(BLKF_BOTH, dev)
		found |= 1 << dectoul(&dev->name[3], NULL);
	ut_asserteq(7, found);

	/* All devices are removable until probed */
	found = 0;
	blk_foreach(BLKF_FIXED, dev)
		found |= 1 << dectoul(&dev->name[3], NULL);
	ut_asserteq(0, found);

	found = 0;
	blk_foreach(BLKF_REMOVABLE, dev)
		found |= 1 << dectoul(&dev->name[3], NULL);
	ut_asserteq(7, found);

	/* Now try again with the probing functions */
	found = 0;
	blk_foreach_probe(BLKF_BOTH, dev)
		found |= 1 << dectoul(&dev->name[3], NULL);
	ut_asserteq(7, found);
	ut_asserteq(3, blk_count_devices(BLKF_BOTH));

	found = 0;
	blk_foreach_probe(BLKF_FIXED, dev)
		found |= 1 << dectoul(&dev->name[3], NULL);
	ut_asserteq(4, found);
	ut_asserteq(1, blk_count_devices(BLKF_FIXED));

	found = 0;
	blk_foreach_probe(BLKF_REMOVABLE, dev)
		found |= 1 << dectoul(&dev->name[3], NULL);
	ut_asserteq(3, found);
	ut_asserteq(2, blk_count_devices(BLKF_REMOVABLE));

	return 0;
}
DM_TEST(dm_test_blk_foreach, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
