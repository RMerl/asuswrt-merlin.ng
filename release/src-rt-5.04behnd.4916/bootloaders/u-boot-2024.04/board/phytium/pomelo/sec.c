// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021
 * lixinde         <lixinde@phytium.com.cn>
 * weichangzheng   <weichangzheng@phytium.com.cn>
 */

#include <stdio.h>
#include <string.h>
#include <linux/arm-smccc.h>
#include <init.h>
#include "cpu.h"

struct common_config {
	u32 magic;
	u32 version;
	u32 size;
	u8 rev1[4];
	u64  core_bit_map;
} __attribute((aligned(4)));

struct common_config const common_base_info = {
	.magic = PARAMETER_COMMON_MAGIC,
	.version = 0x1,
	.core_bit_map = 0x3333,
};

void sec_init(void)
{
	u8 buffer[0x100];
	struct arm_smccc_res res;

	memcpy(buffer, &common_base_info, sizeof(common_base_info));
	arm_smccc_smc(CPU_INIT_SEC_SVC, 0, (u64)buffer, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		panic("SEC init failed :0x%lx\n", res.a0);
}
