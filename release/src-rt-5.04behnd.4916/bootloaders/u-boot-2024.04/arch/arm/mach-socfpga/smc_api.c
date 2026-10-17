// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <linux/intel-smc.h>

int invoke_smc(u32 func_id, u64 *args, int arg_len, u64 *ret_arg, int ret_len)
{
	struct pt_regs regs;

	memset(&regs, 0, sizeof(regs));
	regs.regs[0] = func_id;

	if (args)
		memcpy(&regs.regs[1], args, arg_len * sizeof(*args));

	smc_call(&regs);

	if (ret_arg)
		memcpy(ret_arg, &regs.regs[1], ret_len * sizeof(*ret_arg));

	return regs.regs[0];
}

int smc_send_mailbox(u32 cmd, u32 len, u32 *arg, u8 urgent, u32 *resp_buf_len,
		     u32 *resp_buf)
{
	int ret;
	u64 args[6];
	u64 resp[3];

	args[0] = cmd;
	args[1] = (u64)arg;
	args[2] = len;
	args[3] = urgent;
	args[4] = (u64)resp_buf;
	if (resp_buf_len)
		args[5] = *resp_buf_len;
	else
		args[5] = 0;

	ret = invoke_smc(INTEL_SIP_SMC_MBOX_SEND_CMD, args, ARRAY_SIZE(args),
			 resp, ARRAY_SIZE(resp));

	if (ret == INTEL_SIP_SMC_STATUS_OK && resp_buf && resp_buf_len) {
		if (!resp[0])
			*resp_buf_len = resp[1];
	}

	return (int)resp[0];
}

int smc_get_usercode(u32 *usercode)
{
	int ret;
	u64 resp;

	if (!usercode)
		return -EINVAL;

	ret = invoke_smc(INTEL_SIP_SMC_GET_USERCODE, NULL, 0,
			 &resp, 1);

	if (ret == INTEL_SIP_SMC_STATUS_OK)
		*usercode = (u32)resp;

	return ret;
}
