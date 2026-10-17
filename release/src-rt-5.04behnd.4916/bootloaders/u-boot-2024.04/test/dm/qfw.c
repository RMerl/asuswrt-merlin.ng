// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#include <common.h>
#include <qfw.h>
#include <dm.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

/*
 * Exercise the device enough to be satisfied the initialisation and DMA
 * interfaces work.
 */

static int dm_test_qfw_cpus(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_QFW, &dev));
	ut_asserteq(5, qfw_online_cpus(dev));

	return 0;
}

DM_TEST(dm_test_qfw_cpus, UT_TESTF_SCAN_PDATA);

static int dm_test_qfw_firmware_list(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct fw_file *file;

	ut_assertok(uclass_first_device_err(UCLASS_QFW, &dev));
	ut_assertok(qfw_read_firmware_list(dev));
	ut_assertok_ptr((file = qfw_find_file(dev, "test-one")));

	return 0;
}

DM_TEST(dm_test_qfw_firmware_list, UT_TESTF_SCAN_PDATA);
