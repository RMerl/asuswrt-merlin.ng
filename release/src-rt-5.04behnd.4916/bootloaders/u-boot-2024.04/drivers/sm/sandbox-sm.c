// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 SberDevices, Inc.
 *
 * Author: Alexey Romanov <avromanov@salutedevices.com>
 */

#include <common.h>
#include <sm.h>
#include <sm-uclass.h>
#include <sandbox-sm.h>
#include <asm/ptrace.h>
#include <dm/device.h>
#include <linux/sizes.h>

static u8 test_buffer[SZ_4K];

static int sandbox_sm_call(struct udevice *dev, u32 cmd_index, s32 *smc_ret,
			   struct pt_regs *args)
{
	if (cmd_index >= SANDBOX_SMC_CMD_COUNT)
		return -EINVAL;

	if (smc_ret)
		*smc_ret = 0;

	return 0;
}

static int sandbox_sm_call_read(struct udevice *dev, void *buffer, size_t size,
				u32 cmd_index, struct pt_regs *args)
{
	if (cmd_index >= SANDBOX_SMC_CMD_COUNT || !buffer)
		return -EINVAL;

	if (size > sizeof(test_buffer))
		return -EINVAL;

	memcpy(buffer, test_buffer, size);

	return size;
}

static int sandbox_sm_call_write(struct udevice *dev, void *buffer, size_t size,
				 u32 cmd_index, struct pt_regs *args)
{
	if (cmd_index >= SANDBOX_SMC_CMD_COUNT || !buffer)
		return -EINVAL;

	if (size > sizeof(test_buffer))
		return -EINVAL;

	memcpy(test_buffer, buffer, size);

	return size;
}

static const struct udevice_id sandbox_sm_ids[] = {
	{
		.compatible = "sandbox,sm",
	},
	{},
};

static const struct sm_ops sandbox_sm_ops = {
	.sm_call = sandbox_sm_call,
	.sm_call_read = sandbox_sm_call_read,
	.sm_call_write = sandbox_sm_call_write,
};

U_BOOT_DRIVER(sm) = {
	.name = "sm",
	.id = UCLASS_SM,
	.of_match = sandbox_sm_ids,
	.ops = &sandbox_sm_ops,
};
