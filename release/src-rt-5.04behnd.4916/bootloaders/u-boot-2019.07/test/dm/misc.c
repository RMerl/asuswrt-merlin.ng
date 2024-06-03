// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <misc.h>
#include <test/ut.h>

static int dm_test_misc(struct unit_test_state *uts)
{
	struct udevice *dev;
	u8 buf[16];
	int id;
	ulong last_ioctl;
	bool enabled;

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "misc-test", &dev));

	/* Read / write tests */
	ut_asserteq(4, misc_write(dev, 0, "TEST", 4));
	ut_asserteq(5, misc_write(dev, 4, "WRITE", 5));
	ut_asserteq(9, misc_read(dev, 0, buf, 9));

	ut_assertok(memcmp(buf, "TESTWRITE", 9));

	/* Call tests */

	id = 0;
	ut_assertok(misc_call(dev, 0, &id, 4, buf, 16));
	ut_assertok(memcmp(buf, "Zero", 4));

	id = 2;
	ut_assertok(misc_call(dev, 0, &id, 4, buf, 16));
	ut_assertok(memcmp(buf, "Two", 3));

	ut_assertok(misc_call(dev, 1, &id, 4, buf, 16));
	ut_assertok(memcmp(buf, "Forty-two", 9));

	id = 1;
	ut_assertok(misc_call(dev, 1, &id, 4, buf, 16));
	ut_assertok(memcmp(buf, "Forty-one", 9));

	/* IOCTL tests */

	ut_assertok(misc_ioctl(dev, 6, NULL));
	/* Read back last issued ioctl */
	ut_assertok(misc_call(dev, 2, NULL, 0, &last_ioctl,
			      sizeof(last_ioctl)));
	ut_asserteq(6, last_ioctl)

	ut_assertok(misc_ioctl(dev, 23, NULL));
	/* Read back last issued ioctl */
	ut_assertok(misc_call(dev, 2, NULL, 0, &last_ioctl,
			      sizeof(last_ioctl)));
	ut_asserteq(23, last_ioctl)

	/* Enable / disable tests */

	/* Read back enable/disable status */
	ut_assertok(misc_call(dev, 3, NULL, 0, &enabled,
			      sizeof(enabled)));
	ut_asserteq(true, enabled);

	ut_assertok(misc_set_enabled(dev, false));
	/* Read back enable/disable status */
	ut_assertok(misc_call(dev, 3, NULL, 0, &enabled,
			      sizeof(enabled)));
	ut_asserteq(false, enabled);

	ut_assertok(misc_set_enabled(dev, true));
	/* Read back enable/disable status */
	ut_assertok(misc_call(dev, 3, NULL, 0, &enabled,
			      sizeof(enabled)));
	ut_asserteq(true, enabled);

	return 0;
}

DM_TEST(dm_test_misc, DM_TESTF_SCAN_FDT);
