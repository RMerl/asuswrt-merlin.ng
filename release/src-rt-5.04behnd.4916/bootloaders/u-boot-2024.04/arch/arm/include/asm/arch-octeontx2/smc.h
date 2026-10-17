/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __SMC_H__
#define __SMC_H__

#include <asm/arch/smc-id.h>

ssize_t smc_configure_ooo(unsigned int val);
ssize_t smc_dram_size(unsigned int node);
ssize_t smc_disable_rvu_lfs(unsigned int node);
ssize_t smc_flsf_fw_booted(void);

#endif
