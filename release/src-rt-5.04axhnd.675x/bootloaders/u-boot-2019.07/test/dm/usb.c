// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/state.h>
#include <asm/test.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/ut.h>

/* Test that sandbox USB works correctly */
static int dm_test_usb_base(struct unit_test_state *uts)
{
	struct udevice *bus;

	ut_asserteq(-ENODEV, uclass_get_device_by_seq(UCLASS_USB, 0, &bus));
	ut_assertok(uclass_get_device(UCLASS_USB, 0, &bus));
	ut_asserteq(-ENODEV, uclass_get_device_by_seq(UCLASS_USB, 2, &bus));

	return 0;
}
DM_TEST(dm_test_usb_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/*
 * Test that we can use the flash stick. This is more of a functional test. It
 * covers scanning the bug, setting up a hub and a flash stick and reading
 * data from the flash stick.
 */
static int dm_test_usb_flash(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct blk_desc *dev_desc;
	char cmp[1024];

	state_set_skip_delays(true);
	ut_assertok(usb_init());
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 0, &dev));
	ut_assertok(blk_get_device_by_str("usb", "0", &dev_desc));

	/* Read a few blocks and look for the string we expect */
	ut_asserteq(512, dev_desc->blksz);
	memset(cmp, '\0', sizeof(cmp));
	ut_asserteq(2, blk_dread(dev_desc, 0, 2, cmp));
	ut_assertok(strcmp(cmp, "this is a test"));
	ut_assertok(usb_stop());

	return 0;
}
DM_TEST(dm_test_usb_flash, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* test that we can handle multiple storage devices */
static int dm_test_usb_multi(struct unit_test_state *uts)
{
	struct udevice *dev;

	state_set_skip_delays(true);
	ut_assertok(usb_init());
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 2, &dev));
	ut_assertok(usb_stop());

	return 0;
}
DM_TEST(dm_test_usb_multi, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int count_usb_devices(void)
{
	struct udevice *hub;
	struct uclass *uc;
	int count = 0;
	int ret;

	ret = uclass_get(UCLASS_USB_HUB, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(hub, uc) {
		struct udevice *dev;

		count++;
		for (device_find_first_child(hub, &dev);
		     dev;
		     device_find_next_child(&dev)) {
			count++;
		}
	}

	return count;
}

/* test that no USB devices are found after we stop the stack */
static int dm_test_usb_stop(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* Scan and check that all devices are present */
	state_set_skip_delays(true);
	ut_assertok(usb_init());
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 2, &dev));
	ut_asserteq(6, count_usb_devices());
	ut_assertok(usb_stop());
	ut_asserteq(0, count_usb_devices());

	return 0;
}
DM_TEST(dm_test_usb_stop, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int dm_test_usb_keyb(struct unit_test_state *uts)
{
	struct udevice *dev;

	state_set_skip_delays(true);
	ut_assertok(usb_init());

	/* Initially there should be no characters */
	ut_asserteq(0, tstc());

	ut_assertok(uclass_get_device_by_name(UCLASS_USB_EMUL, "keyb",
					      &dev));

	/*
	 * Add a string to the USB keyboard buffer - it should appear in
	 * stdin
	 */
	ut_assertok(sandbox_usb_keyb_add_string(dev, "ab"));
	ut_asserteq(1, tstc());
	ut_asserteq('a', getc());
	ut_asserteq(1, tstc());
	ut_asserteq('b', getc());
	ut_asserteq(0, tstc());

	ut_assertok(usb_stop());

	return 0;
}
DM_TEST(dm_test_usb_keyb, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
