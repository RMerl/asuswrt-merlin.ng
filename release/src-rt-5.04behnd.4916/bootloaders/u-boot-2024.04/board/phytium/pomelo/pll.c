// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021
 * lixinde         <lixinde@phytium.com.cn>
 * weichangzheng   <weichangzheng@phytium.com.cn>
 */

#include <stdio.h>
#include <string.h>
#include <asm/io.h>
#include <linux/arm-smccc.h>
#include <init.h>
#include "cpu.h"

struct pll_config {
	u32 magic;
	u32 version;
	u32 size;
	u8 rev1[4];
	u32 core_pll;
	u32 res1;
	u32 lmu_pll;
	u32 res2;
	u32 res3;
	u32 res4;
	u32 res5;
} __attribute((aligned(4)));

struct pll_config const pll_base_info = {
	.magic = PARAMETER_PLL_MAGIC,
	.version = 0x1,
	.size = 0x30,
	.core_pll = 2300,	/*MHz*/
	.lmu_pll = 667,		/*MHz*/
};

u32 get_reset_source(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(CPU_GET_RST_SOURCE, 0, 0, 0, 0, 0, 0, 0, &res);
	return res.a0;
}

void pll_init(void)
{
	u8 buffer[0x100];
	struct arm_smccc_res res;

	memcpy(buffer, &pll_base_info, sizeof(pll_base_info));
	arm_smccc_smc(CPU_INIT_PLL, 0, (u64)buffer, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		panic("PLL init failed :0x%lx\n", res.a0);
}

void check_reset(void)
{
	u32 rst;

	rst = get_reset_source();

	switch (rst) {
	case CPU_RESET_POWER_ON:
		pll_init();
		break;
	case CPU_RESET_PLL:
		break;
	case CPU_RESET_WATCH_DOG:
		break;
	default:
		panic("other reset source\n");
	}
}
