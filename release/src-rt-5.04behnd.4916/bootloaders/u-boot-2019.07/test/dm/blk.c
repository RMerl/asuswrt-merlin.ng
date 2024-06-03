// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <asm/state.h>
#include <dm/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Test that block devices can be created */
static int dm_test_blk_base(struct unit_test_state *uts)
{
	struct udevice *blk1, *blk3, *dev;

	/* Make sure there are no block devices */
	ut_asserteq(-ENODEV, uclass_get_device_by_seq(UCLASS_BLK, 0, &dev));

	/* Create two, one the parent of the other */
	ut_assertok(blk_create_device(gd->dm_root, "sandbox_host_blk", "test",
				      IF_TYPE_HOST, 1, 512, 2, &blk1));
	ut_assertok(blk_create_device(blk1, "sandbox_host_blk", "test",
				      IF_TYPE_HOST, 3, 512, 2, &blk3));

	/* Check we can find them */
	ut_asserteq(-ENODEV, blk_get_device(IF_TYPE_HOST, 0, &dev));
	ut_assertok(blk_get_device(IF_TYPE_HOST, 1, &dev));
	ut_asserteq_ptr(blk1, dev);
	ut_assertok(blk_get_device(IF_TYPE_HOST, 3, &dev));
	ut_asserteq_ptr(blk3, dev);

	/* Check we can iterate */
	ut_assertok(blk_first_device(IF_TYPE_HOST, &dev));
	ut_asserteq_ptr(blk1, dev);
	ut_assertok(blk_next_device(&dev));
	ut_asserteq_ptr(blk3, dev);

	return 0;
}
DM_TEST(dm_test_blk_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

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

	/* Get a flash device */
	state_set_skip_delays(true);
	ut_assertok(usb_init());
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 0, &usb_dev));
	ut_assertok(blk_get_device_by_str("usb", "0", &dev_desc));

	/* The parent should be a block device */
	ut_assertok(blk_get_device(IF_TYPE_USB, 0, &dev));
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
DM_TEST(dm_test_blk_usb, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we can find block devices without probing them */
static int dm_test_blk_find(struct unit_test_state *uts)
{
	struct udevice *blk, *dev;

	ut_assertok(blk_create_device(gd->dm_root, "sandbox_host_blk", "test",
				      IF_TYPE_HOST, 1, 512, 2, &blk));
	ut_asserteq(-ENODEV, blk_find_device(IF_TYPE_HOST, 0, &dev));
	ut_assertok(blk_find_device(IF_TYPE_HOST, 1, &dev));
	ut_asserteq_ptr(blk, dev);
	ut_asserteq(false, device_active(dev));

	/* Now activate it */
	ut_assertok(blk_get_device(IF_TYPE_HOST, 1, &dev));
	ut_asserteq_ptr(blk, dev);
	ut_asserteq(true, device_active(dev));

	return 0;
}
DM_TEST(dm_test_blk_find, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that block device numbering works as expected */
static int dm_test_blk_devnum(struct unit_test_state *uts)
{
	struct udevice *dev, *mmc_dev, *parent;
	int i;

	/*
	 * Probe the devices, with the first one being probed last. This is the
	 * one with no alias / sequence numnber.
	 */
	ut_assertok(uclass_get_device(UCLASS_MMC, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_MMC, 2, &dev));
	ut_assertok(uclass_get_device(UCLASS_MMC, 0, &dev));
	for (i = 0; i < 3; i++) {
		struct blk_desc *desc;

		/* Check that the bblock device is attached */
		ut_assertok(uclass_get_device_by_seq(UCLASS_MMC, i, &mmc_dev));
		ut_assertok(blk_find_device(IF_TYPE_MMC, i, &dev));
		parent = dev_get_parent(dev);
		ut_asserteq_ptr(parent, mmc_dev);
		ut_asserteq(trailing_strtol(mmc_dev->name), i);

		/*
		 * Check that the block device devnum matches its parent's
		 * sequence number
		 */
		desc = dev_get_uclass_platdata(dev);
		ut_asserteq(desc->devnum, i);
	}

	return 0;
}
DM_TEST(dm_test_blk_devnum, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we can get a block from its parent */
static int dm_test_blk_get_from_parent(struct unit_test_state *uts)
{
	struct udevice *dev, *blk;

	ut_assertok(uclass_get_device(UCLASS_MMC, 0, &dev));
	ut_assertok(blk_get_from_parent(dev, &blk));

	ut_assertok(uclass_get_device(UCLASS_I2C, 0, &dev));
	ut_asserteq(-ENOTBLK, blk_get_from_parent(dev, &blk));

	ut_assertok(uclass_get_device(UCLASS_GPIO, 0, &dev));
	ut_asserteq(-ENODEV, blk_get_from_parent(dev, &blk));

	return 0;
}
DM_TEST(dm_test_blk_get_from_parent, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
