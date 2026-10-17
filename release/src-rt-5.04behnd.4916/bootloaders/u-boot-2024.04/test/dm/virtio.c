// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <virtio_types.h>
#include <virtio.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/test.h>
#include <test/ut.h>

/* Test of the virtio driver that does not have required driver ops */
static int dm_test_virtio_missing_ops(struct unit_test_state *uts)
{
	struct udevice *bus;

	/* find the virtio device */
	ut_assertok(uclass_find_device(UCLASS_VIRTIO, 1, &bus));

	/*
	 * Probe the device should fail with error -ENOENT.
	 * See ops check in virtio_uclass_pre_probe().
	 */
	ut_asserteq(-ENOENT, device_probe(bus));

	return 0;
}
DM_TEST(dm_test_virtio_missing_ops, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
