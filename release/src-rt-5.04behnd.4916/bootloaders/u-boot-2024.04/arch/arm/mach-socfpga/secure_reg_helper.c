// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <hang.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/misc.h>
#include <asm/arch/secure_reg_helper.h>
#include <asm/arch/smc_api.h>
#include <asm/arch/system_manager.h>
#include <linux/errno.h>
#include <linux/intel-smc.h>

int socfpga_secure_convert_reg_id_to_addr(u32 id, phys_addr_t *reg_addr)
{
	switch (id) {
	case SOCFPGA_SECURE_REG_SYSMGR_SOC64_SDMMC:
		*reg_addr = socfpga_get_sysmgr_addr() + SYSMGR_SOC64_SDMMC;
		break;
	case SOCFPGA_SECURE_REG_SYSMGR_SOC64_EMAC0:
		*reg_addr = socfpga_get_sysmgr_addr() + SYSMGR_SOC64_EMAC0;
		break;
	case SOCFPGA_SECURE_REG_SYSMGR_SOC64_EMAC1:
		*reg_addr = socfpga_get_sysmgr_addr() + SYSMGR_SOC64_EMAC1;
		break;
	case SOCFPGA_SECURE_REG_SYSMGR_SOC64_EMAC2:
		*reg_addr = socfpga_get_sysmgr_addr() + SYSMGR_SOC64_EMAC2;
		break;
	default:
		return -EADDRNOTAVAIL;
	}
	return 0;
}

int socfpga_secure_reg_read32(u32 id, u32 *val)
{
	int ret;
	u64 ret_arg;
	u64 args[1];

	phys_addr_t reg_addr;
	ret = socfpga_secure_convert_reg_id_to_addr(id, &reg_addr);
	if (ret)
		return ret;

	args[0] = (u64)reg_addr;
	ret = invoke_smc(INTEL_SIP_SMC_REG_READ, args, 1, &ret_arg, 1);
	if (ret)
		return ret;

	*val = (u32)ret_arg;

	return 0;
}

int socfpga_secure_reg_write32(u32 id, u32 val)
{
	int ret;
	u64 args[2];

	phys_addr_t reg_addr;
	ret = socfpga_secure_convert_reg_id_to_addr(id, &reg_addr);
	if (ret)
		return ret;

	args[0] = (u64)reg_addr;
	args[1] = val;
	return invoke_smc(INTEL_SIP_SMC_REG_WRITE, args, 2, NULL, 0);
}

int socfpga_secure_reg_update32(u32 id, u32 mask, u32 val)
{
	int ret;
	u64 args[3];

	phys_addr_t reg_addr;
	ret = socfpga_secure_convert_reg_id_to_addr(id, &reg_addr);
	if (ret)
		return ret;

	args[0] = (u64)reg_addr;
	args[1] = mask;
	args[2] = val;
	return invoke_smc(INTEL_SIP_SMC_REG_UPDATE, args, 3, NULL, 0);
}
