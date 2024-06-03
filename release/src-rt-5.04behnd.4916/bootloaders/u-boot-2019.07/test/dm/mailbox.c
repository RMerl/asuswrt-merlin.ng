// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <asm/mbox.h>
#include <test/ut.h>

static int dm_test_mailbox(struct unit_test_state *uts)
{
	struct udevice *dev;
	uint32_t msg;

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "mbox-test", &dev));
	ut_assertok(sandbox_mbox_test_get(dev));

	ut_asserteq(-ETIMEDOUT, sandbox_mbox_test_recv(dev, &msg));
	ut_assertok(sandbox_mbox_test_send(dev, 0xaaff9955UL));
	ut_assertok(sandbox_mbox_test_recv(dev, &msg));
	ut_asserteq(msg, 0xaaff9955UL ^ SANDBOX_MBOX_PING_XOR);
	ut_asserteq(-ETIMEDOUT, sandbox_mbox_test_recv(dev, &msg));

	ut_assertok(sandbox_mbox_test_free(dev));

	return 0;
}
DM_TEST(dm_test_mailbox, DM_TESTF_SCAN_FDT);
