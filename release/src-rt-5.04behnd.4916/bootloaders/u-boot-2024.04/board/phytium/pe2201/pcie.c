// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.
 * lixinde          <lixinde@phytium.com.cn>
 * weichangzheng    <weichangzheng@phytium.com.cn>
 */

#include <stdio.h>
#include <string.h>
#include <linux/arm-smccc.h>
#include <init.h>
#include "cpu.h"

struct pcu_ctr {
	u32 base_config[4];
	u32 equalization[4];
	u8 rev[72];
} __attribute((aligned(4)));

struct pcu_config {
	u32 magic;
	u32 version;
	u32 size;
	u8 rev1[4];
	u32 independent_tree;
	u32 base_cfg;
	u8 rev2[16];
	struct pcu_ctr ctr_cfg[2];
} __attribute((aligned(4)));

struct pcu_config const peu_base_info = {
	.magic = PARAMETER_PCIE_MAGIC,
	.version = 0x4,
	.size = 0x100,
	.independent_tree = CFG_INDEPENDENT_TREE,
	.base_cfg = ((PCI_PEU1 | (X1X1X1X1 << 1)) << PEU1_OFFSET | (PCI_PEU0 | (X1X1 << 1))),
	.ctr_cfg[0].base_config[0] = (EP_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
	.ctr_cfg[0].base_config[1] = (EP_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
	.ctr_cfg[0].base_config[2] = (EP_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
	.ctr_cfg[1].base_config[0] = (EP_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
	.ctr_cfg[1].base_config[1] = (EP_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
	.ctr_cfg[1].base_config[2] = (EP_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
	.ctr_cfg[0].equalization[0] = 0x7,
	.ctr_cfg[0].equalization[1] = 0x7,
	.ctr_cfg[0].equalization[2] = 0x7,
	.ctr_cfg[1].equalization[0] = 0x7,
	.ctr_cfg[1].equalization[1] = 0x7,
	.ctr_cfg[1].equalization[2] = 0x7,
};

void pcie_init(void)
{
	u8 buffer[0x100];
	struct arm_smccc_res res;

	memcpy(buffer, &peu_base_info, sizeof(peu_base_info));
	arm_smccc_smc(CPU_INIT_PCIE, 0, (u64)buffer, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		panic("PCIE init failed :0x%lx\n", res.a0);
}
