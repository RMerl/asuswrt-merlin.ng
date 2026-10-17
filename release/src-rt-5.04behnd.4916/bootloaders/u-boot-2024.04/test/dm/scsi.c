// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <part.h>
#include <scsi.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Test that sandbox SCSI works correctly */
static int dm_test_scsi_base(struct unit_test_state *uts)
{
	const struct disk_partition *info;
	const struct disk_part *part;
	struct udevice *dev;

	ut_assertok(scsi_scan(false));

	/*
	 * We expect some sort of partition on the disk image, created by
	 * test_ut_dm_init()
	 */
	ut_assertok(uclass_first_device_err(UCLASS_PARTITION, &dev));

	part = dev_get_uclass_plat(dev);
	ut_asserteq(1, part->partnum);

	info = &part->gpt_part_info;
	ut_asserteq_str("sda1", info->name);
	ut_asserteq_str("U-Boot", info->type);
	ut_asserteq(0x83 /* linux */, info->sys_ind);

	return 0;
}
DM_TEST(dm_test_scsi_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
