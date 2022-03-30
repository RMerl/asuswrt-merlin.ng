// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <mmc.h>
#include <dm/test.h>
#include <test/ut.h>

/*
 * Basic test of the mmc uclass. We could expand this by implementing an MMC
 * stack for sandbox, or at least implementing the basic operation.
 */
static int dm_test_mmc_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_MMC, 0, &dev));

	return 0;
}
DM_TEST(dm_test_mmc_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int dm_test_mmc_blk(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct blk_desc *dev_desc;
	char cmp[1024];

	ut_assertok(uclass_get_device(UCLASS_MMC, 0, &dev));
	ut_assertok(blk_get_device_by_str("mmc", "0", &dev_desc));

	/* Read a few blocks and look for the string we expect */
	ut_asserteq(512, dev_desc->blksz);
	memset(cmp, '\0', sizeof(cmp));
	ut_asserteq(2, blk_dread(dev_desc, 0, 2, cmp));
	ut_assertok(strcmp(cmp, "this is a test"));

	return 0;
}
DM_TEST(dm_test_mmc_blk, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
