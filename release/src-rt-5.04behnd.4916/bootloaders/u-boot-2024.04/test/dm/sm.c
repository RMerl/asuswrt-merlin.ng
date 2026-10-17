// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 SberDevices, Inc.
 *
 * Author: Alexey Romanov <avromanov@salutedevices.com>
 */

#include <common.h>
#include <dm.h>
#include <sm.h>
#include <sandbox-sm.h>
#include <asm/ptrace.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <test/ut.h>
#include <linux/sizes.h>

static int dm_test_sm(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct pt_regs regs;
	char buffer[128] = { 0 };
	char test_string[] = "secure-monitor";
	int ret, val;

	ut_assertok(uclass_get_device_by_name(UCLASS_SM,
		"secure-monitor", &dev));

	ret = sm_call(dev, SANDBOX_SMC_CMD_COUNT, NULL, &regs);
	ut_asserteq(ret, -EINVAL);

	ret = sm_call(dev, SANDBOX_SMC_CMD_COMMON, &val, &regs);
	ut_asserteq(ret, 0);
	ut_asserteq(val, 0);

	ret = sm_call_write(dev, buffer, sizeof(buffer),
		SANDBOX_SMC_CMD_COUNT, &regs);
	ut_asserteq(ret, -EINVAL);

	ret = sm_call_write(dev, buffer, SZ_4K + 1,
		SANDBOX_SMC_CMD_WRITE_MEM, &regs);
	ut_asserteq(ret, -EINVAL);

	ret = sm_call_write(dev, buffer, sizeof(buffer),
		SANDBOX_SMC_CMD_COUNT, &regs);
	ut_asserteq(ret, -EINVAL);

	ret = sm_call_write(dev, buffer, SZ_4K + 1,
		SANDBOX_SMC_CMD_READ_MEM, &regs);
	ut_asserteq(ret, -EINVAL);

	ret = sm_call_write(dev, test_string, sizeof(test_string),
		SANDBOX_SMC_CMD_WRITE_MEM, &regs);
	ut_asserteq(ret, sizeof(test_string));

	ret = sm_call_read(dev, buffer, sizeof(buffer),
		SANDBOX_SMC_CMD_READ_MEM, &regs);
	ut_asserteq(ret, sizeof(buffer));

	ut_asserteq_str(buffer, test_string);

	return 0;
}

DM_TEST(dm_test_sm, UT_TESTF_SCAN_FDT);
