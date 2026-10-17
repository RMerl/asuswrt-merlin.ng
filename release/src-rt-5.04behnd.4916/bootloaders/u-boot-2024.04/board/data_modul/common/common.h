/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#ifndef __EDM_SBC_COMMON_H__
#define __EDM_SBC_COMMON_H__

#include <asm/arch/ddr.h>
#include <asm/mach-imx/iomux-v3.h>

u8 dmo_get_memcfg(void);
void dmo_board_init_f(const iomux_v3_cfg_t wdog_pad,
		      struct dram_timing_info *dram_timing_info[8]);
void dmo_setup_boot_device(void);
void dmo_setup_mac_address(void);

#endif /* __EDM_SBC_COMMON_H__ */
