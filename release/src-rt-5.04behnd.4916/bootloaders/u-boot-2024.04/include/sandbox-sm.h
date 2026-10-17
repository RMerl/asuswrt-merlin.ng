/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 SberDevices, Inc.
 *
 * Author: Alexey Romanov <avromanov@salutedevices.com>
 */

#ifndef __SANDBOX_SM_H__
#define __SANDBOX_SM_H__

enum sandbox_smc_cmd {
	SANDBOX_SMC_CMD_READ_MEM,
	SANDBOX_SMC_CMD_WRITE_MEM,
	SANDBOX_SMC_CMD_COMMON,
	SANDBOX_SMC_CMD_COUNT,
};

#endif
