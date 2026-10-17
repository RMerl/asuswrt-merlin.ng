/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2020 Intel Corporation <www.intel.com>
 *
 */

#ifndef	_SECURE_REG_HELPER_H_
#define	_SECURE_REG_HELPER_H_

#define SOCFPGA_SECURE_REG_SYSMGR_SOC64_SDMMC 1
#define SOCFPGA_SECURE_REG_SYSMGR_SOC64_EMAC0 2
#define SOCFPGA_SECURE_REG_SYSMGR_SOC64_EMAC1 3
#define SOCFPGA_SECURE_REG_SYSMGR_SOC64_EMAC2 4

int socfpga_secure_reg_read32(u32 id, u32 *val);
int socfpga_secure_reg_write32(u32 id, u32 val);
int socfpga_secure_reg_update32(u32 id, u32 mask, u32 val);

#endif /* _SECURE_REG_HELPER_H_ */
